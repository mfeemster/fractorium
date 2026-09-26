#include "FractoriumPch.h"
#include "CurvesGraphicsView.h"

/// <summary>
/// Construct the scene which will have a fixed rect.
/// Construct all points, pens and axes.
/// </summary>
/// <param name="parent">Pass to the parent</param>
CurvesGraphicsView::CurvesGraphicsView(QWidget* parent)
	: QGraphicsView(parent)
{
	m_Scene.setSceneRect(0, 0, 245, 245);
	m_AxisPen = QPen(Qt::GlobalColor::white);
	m_APen = QPen(Qt::GlobalColor::black); m_Pens[0] = &m_APen;
	m_RPen = QPen(Qt::GlobalColor::red);   m_Pens[1] = &m_RPen;
	m_GPen = QPen(Qt::GlobalColor::green); m_Pens[2] = &m_GPen;
	m_BPen = QPen(Qt::GlobalColor::blue);  m_Pens[3] = &m_BPen;
	m_APen.setWidth(2);
	m_RPen.setWidth(2);
	m_GPen.setWidth(2);
	m_BPen.setWidth(2);
	setScene(&m_Scene);
	//qDebug() << "Original scene rect before setting anything is: " << sceneRect();
	m_OriginalRect = sceneRect();
	Curves<float> curves(true);
	Set(curves);
	show();
	//qDebug() << "Original scene rect is: " << m_OriginalRect;
}

/// <summary>
/// Called when an underlying point has had its position changed, so emit a signal so that a listener can take action.
/// </summary>
/// <param name="curveIndex">The curve whose point value was changed, 0-3.</param>
/// <param name="pointIndex">The point within the curve whose point value was changed.</param>
/// <param name="point">The position of the point. X,Y will each be within 0-1.</param>
void CurvesGraphicsView::PointChanged(int curveIndex, int pointIndex, const QPointF& point)
{
    if (static_cast<size_t>(curveIndex) == m_Index)
	{
		const auto x = point.x() / width();
		const auto y = (height() - point.y()) / height();
		emit PointChangedSignal(curveIndex, pointIndex, QPointF(x, y));
	}
}

/// <summary>
/// Get the position of a given point within a given curve.
/// </summary>
/// <param name="curveIndex">The curve whose point value will be retrieved, 0-3.</param>
/// <param name="pointIndex">The point within the curve whose value will be retrieved, 0-3.</param>
/// <returns>The position of the point. X,Y will each be within 0-1.</returns>
QPointF CurvesGraphicsView::Get(int curveIndex, int pointIndex)
{
    if (curveIndex < 4 && static_cast<size_t>(pointIndex) < m_Points[curveIndex].size())
	{
		if (EllipseItem* item = m_Points[curveIndex][pointIndex])
			return QPointF(item->pos().x() / width(), (height() - item->pos().y()) / height());
	}

	return QPointF();
}

/// <summary>
/// Set the position of a given point within a given curve.
/// </summary>
/// <param name="curveIndex">The curve whose point will be set, 0-3.</param>
/// <param name="pointIndex">The point within the curve which will be set, 0-3</param>
/// <param name="point">The position to set the point to. X,Y will each be within 0-1.</param>
void CurvesGraphicsView::Set(int curveIndex, int pointIndex, const QPointF& point)
{
    if (curveIndex < 4 && static_cast<size_t>(pointIndex) < m_Points[curveIndex].size())
	{
		m_Points[curveIndex][pointIndex]->setPos(point.x() * width(), (1.0 - point.y()) * height());//Scale to scene dimensions, Y axis is flipped.
	}
}

void CurvesGraphicsView::Set(Curves<float>& curves)
{
	m_Scene.clear();
	m_XLine = new QGraphicsLineItem();
	m_XLine->setPen(m_AxisPen);
	m_XLine->setZValue(0);
	m_YLine = new QGraphicsLineItem();
	m_YLine->setPen(m_AxisPen);
	m_YLine->setZValue(0);
	m_Scene.addItem(m_XLine);
	m_Scene.addItem(m_YLine);
	auto createpoints = [&](int index, vector<EllipseItem*>& items, Qt::GlobalColor col, int zval)
	{
		items.clear();
		m_Points[index].clear();

        for (int i = 0; i < int(curves.m_Points[index].size()); i++)
		{
			auto item = new EllipseItem(QRectF(-5, -5, 10, 10), index, i, this);
			items.push_back(item);
			item->setBrush(QBrush(col));
			m_Scene.addItem(item);
			m_Points[index].push_back(item);
			item->setZValue(zval);
			const QPointF point(curves.m_Points[index][i].x, curves.m_Points[index][i].y);
			Set(index, i, point);
		}
	};
	createpoints(0, m_AllP, Qt::GlobalColor::black, 2);
	createpoints(1, m_RedP, Qt::GlobalColor::red, 1);
	createpoints(2, m_GrnP, Qt::GlobalColor::green, 1);
	createpoints(3, m_BluP, Qt::GlobalColor::blue, 1);
	SetTop(CurveIndex(m_Index));
}

/// <summary>
/// Set the topmost curve but setting its Z value.
/// All other curves will get a value one less.
/// </summary>
/// <param name="curveIndex">The curve to set</param>
void CurvesGraphicsView::SetTop(CurveIndex curveIndex)
{
	switch (curveIndex)
	{
		case CurveIndex::ALL:
			m_Index = 0;
			break;

		case CurveIndex::RED:
			m_Index = 1;
			break;

		case CurveIndex::GREEN:
			m_Index = 2;
			break;

		case CurveIndex::BLUE:
		default:
			m_Index = 3;
	}

	for (size_t i = 0; i < 4; i++)
	{
		const auto b = (i == m_Index);

		for (auto& p : m_Points[i])
			p->SetCurrent(b);
	}
}

/// <summary>
/// Overridden paint even which draws the points, axes and curves.
/// </summary>
/// <param name="e">Ignored</param>
void CurvesGraphicsView::paintEvent(QPaintEvent* e)
{
	QGraphicsView::paintEvent(e);
	int i;
	const QRectF rect = scene()->sceneRect();
	const double w2 = width() / 2;
	const double h2 = height() / 2;
	//Draw axis lines.
	m_XLine->setLine(QLineF(0, h2, width(), h2));
	m_YLine->setLine(QLineF(w2, 0, w2, height()));
	//This must be constructed every time and cannot be a member.
	QPainter painter(viewport());
	painter.setClipRect(rect);
	painter.setRenderHint(QPainter::Antialiasing);
	auto points = m_Points;

	for (auto& p : points)
	{
		if (p.size() < 2)
			return;

		std::sort(p.begin(), p.end(), [&](auto & lhs, auto & rhs) { return lhs->pos().x() < rhs->pos().x(); });
	}

	//Create 4 new paths. These must be constructed every time and cannot be members.
	//Need to sort the points here first based on their x coordinate.
	QPainterPath paths[4] =
	{
		QPainterPath(points[0][0]->pos()),
		QPainterPath(points[1][0]->pos()),
		QPainterPath(points[2][0]->pos()),
		QPainterPath(points[3][0]->pos())
	};
	int topmost = 0;

	//Construct paths or all curves, and draw them for all but the topmost curve.
	for (i = 0; i < 4; i++)
	{
		vector<v2F> vals;
		vals.reserve(points[i].size());

		for (auto& p : points[i])
			vals.push_back({ p->pos().x(), p->pos().y() });
		Spline<float> spline(vals);

		for (int j = 0; j < rect.width(); j++)
		{
			const auto x = j;
			const auto y = spline.Interpolate(x);
			paths[i].lineTo(QPointF(x, y));
		}

		if (points[i][0]->zValue() == 1)
		{
			painter.setPen(*m_Pens[i]);
			painter.drawPath(paths[i]);
		}
		else
			topmost = i;
	}

	//Draw the topmost curve.
	painter.setPen(*m_Pens[topmost]);
	painter.drawPath(paths[topmost]);
}

void CurvesGraphicsView::mousePressEvent(QMouseEvent* e)
{
	QGraphicsView::mousePressEvent(e);

	if (e == nullptr)
		return;

	const auto thresh = devicePixelRatioF() * 4;
    auto findpoint = [&](int x, int y, double threshParam) -> int
	{
        for (size_t i = 0; i < m_Points[m_Index].size(); i++)
		{
			if (auto item = m_Points[m_Index][i])
			{
				const auto xdist = std::abs(item->pos().x() - x);
				const auto ydist = std::abs(item->pos().y() - y);
                const auto threshAgain = threshParam;

				if (xdist < threshAgain && ydist < threshAgain)
					return i;
			}
		}

		return -1;
	};

	if (e->button() == Qt::MouseButton::RightButton)
	{
		const auto i = findpoint(e->pos().x(), e->pos().y(), thresh);

		if (i != -1)
			emit PointRemovedSignal(m_Index, i);
	}
	else if (findpoint(e->pos().x(), e->pos().y(), thresh * 2) == -1)
	{
		const auto rect = scene()->sceneRect();
		auto points = m_Points[m_Index];

		if (points.size() < 2)
			return;

		std::sort(points.begin(), points.end(), [&](auto & lhs, auto & rhs) { return lhs->pos().x() < rhs->pos().x(); });
		vector<v2F> vals;
		vals.reserve(points.size());

		for (auto& p : points)
			vals.push_back({ p->pos().x(), p->pos().y() });
		Spline<float> spline(vals);

		for (int j = 0; j < rect.width(); j++)
		{
			auto y = spline.Interpolate(j);
			const auto xdist = std::abs(j - e->pos().x());
			const auto ydist = std::abs(y - e->pos().y());

			if (xdist < thresh && ydist < thresh)
			{
                const auto x = e->pos().x() / double(width());
                const auto y2 = (height() - e->pos().y()) / double(height());
				emit PointAddedSignal(m_Index, QPointF(x, y2));
				break;
			}
		}
	}
}
