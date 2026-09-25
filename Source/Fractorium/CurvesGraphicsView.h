#pragma once
#include "FractoriumPch.h"

/// <summary>
/// CurvesGraphicsView and EllipseItem classes.
/// </summary>

class EllipseItem;

/// <summary>
/// Enumeration used for setting values on a specific curve.
/// </summary>
enum class CurveIndex : et
{
	ALL,
	RED,
	GREEN,
	BLUE
};

/// <summary>
/// Derivation to display points on bezier cuves for the user to drag.
/// Selection logic and updating is handled by the base class.
/// Changes here will affect the current ember and vice versa.
/// The points, axis lines and pens are kept as members and the curves
/// themselves are drawn on the fly during each paint event.
/// Pointers to these are kept in arrays to make manipulating them in loops easier.
/// Note that this must work off a fixed rect size, hence why the control is not resizeable.
/// </summary>
class CurvesGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
	CurvesGraphicsView(QWidget* parent = nullptr);

	void PointChanged(int curveIndex, int pointIndex, const QPointF& point);
	QPointF Get(int curveIndex, int pointIndex);
	void Set(int curveIndex, int pointIndex, const QPointF& point);
	void Set(Curves<float>& curves);
	void SetTop(CurveIndex curveIndex);
	size_t SelectedCurveIndex() const noexcept { return m_Index; }

Q_SIGNALS:
	void PointChangedSignal(int curveIndex, int pointIndex, const QPointF& point);
	void PointAddedSignal(size_t curveIndex, const QPointF& point);
	void PointRemovedSignal(size_t curveIndex, int pointIndex);

protected:
	void paintEvent(QPaintEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;

	size_t m_Index = 0;
	QPen m_APen;
	QPen m_RPen;
	QPen m_GPen;
	QPen m_BPen;
	QPen m_AxisPen;
	std::vector<EllipseItem*> m_AllP;
	std::vector<EllipseItem*> m_RedP;
	std::vector<EllipseItem*> m_GrnP;
	std::vector<EllipseItem*> m_BluP;
	QGraphicsLineItem* m_XLine;
	QGraphicsLineItem* m_YLine;
	std::array<QPen*, 4> m_Pens;
	QGraphicsScene m_Scene;
	QRectF m_OriginalRect;
	std::array<std::vector<EllipseItem*>, 4> m_Points;
};


/// <summary>
/// Derivation for draggable points needed to trigger an event whenever the item is changed.
/// Custom drawing is also done to omit drawing a selection rectangle.
/// </summary>
class EllipseItem : public QGraphicsEllipseItem
{
public:
	/// <summary>
	/// Construct the point and specify the curve index it's part of, as well as the
	/// point index within the curve.
	/// </summary>
	/// <param name="rect">Pass to the parent</param>
	/// <param name="curveIndex">The curve's index this point is a part of, 0-3.</param>
	/// <param name="pointIndex">The point index within the curve</param>
	/// <param name="viewParent">The graphics view this point is displayed on</param>
	/// <param name="p">The parent widget of this item</param>
	EllipseItem(const QRectF& rect, int curveIndex, int pointIndex, CurvesGraphicsView* viewParent, QGraphicsItem* parent = nullptr)
		: QGraphicsEllipseItem(rect, parent)
	{
		m_CurveIndex = curveIndex;
		m_PointIndex = pointIndex;
		m_ViewParent = viewParent;
		setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
		setFlag(QGraphicsItem::ItemIsSelectable);
		setFlag(QGraphicsItem::ItemIsMovable);
		setPen(Qt::PenStyle::NoPen);
	}

	/// <summary>
	/// Set whether this item is selectable, which means this curve is the current one.
	/// </summary>
	/// <param name="b">True if selected, else false.</param>
	void SetCurrent(bool b)
	{
		setFlag(QGraphicsItem::ItemIsMovable, b);
		setZValue(b ? 2 : 1);
	}

	/// <summary>
	/// Index properties, getters only.
	/// </summary>
	int CurveIndex() const noexcept { return m_CurveIndex; }
	int PointIndex() const noexcept { return m_PointIndex; }

protected:
	/// <summary>
	/// Overridden paint event to disable the selection rectangle.
	/// </summary>
	/// <param name="painter">Unused and just passed to QGraphicsEllipseItem::paint()</param>
	/// <param name="option">Drawing options used which will have the QStyle::State_Selected flag unset</param>
	/// <param name="widget">Unused and just passed to QGraphicsEllipseItem::paint()</param>
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
	{
		if (option != nullptr && widget != nullptr)
		{
			QStyleOptionGraphicsItem myOption(*option);
			myOption.state &= ~QStyle::State_Selected;
			QGraphicsEllipseItem::paint(painter, &myOption, widget);
		}
	}

	/// <summary>
	/// Overridden itemChange event to notify the parent control that it has moved.
	/// Movement is also restricted to the scene rect.
	/// </summary>
	/// <param name="change">Action is only taken if this value equals ItemPositionChange</param>
	/// <param name="value">The new position. This will be clamped to the scene rect.</param>
	/// <returns>The new position</returns>
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override
	{
		if ((change == ItemPositionChange) && scene())
		{
			//Value is the new position.
			auto newPos = value.toPointF();
			const auto rect = scene()->sceneRect();

			if (!rect.contains(newPos))
			{
				//Keep the item inside the scene rect.
				newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
				newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
			}

			m_ViewParent->PointChanged(m_CurveIndex, m_PointIndex, newPos);
			return newPos;
		}

		return QGraphicsEllipseItem::itemChange(change, value);
	}

	int m_CurveIndex;
	int m_PointIndex;
	CurvesGraphicsView* m_ViewParent;
};
