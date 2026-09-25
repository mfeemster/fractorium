/****************************************************************************/
// This file is part of the gradLib library originally made by Stian Broen
//
// For more free libraries, please visit <http://broentech.no>
//
// gradLib is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this library.  If not, see <http://www.gnu.org/licenses/>
/****************************************************************************/

#include "FractoriumPch.h"
#include "GradientColorsView.h"

/// <summary>
/// Constructor which passes parent widget to the base and sets various size constraints.
/// </summary>
/// <param name="p">The parent widget</param>
GradientColorsView::GradientColorsView(QWidget* p)
	: QWidget(p)
{
	m_ViewRect = QRect(QPoint(0, 0), QPoint(0, 0));
	qRegisterMetaType<GradientArrow>("GradientArrow");
	setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
	setMinimumSize(p->width() - 10, p->height() - 10);
	setMouseTracking(true);
	ResetToDefault();
}

/// <summary>
/// Get whether to interpolate color keys.
/// </summary>
/// <returns>True to interpolate (blend), false to do hard cuts.</returns>
bool GradientColorsView::Blend()
{
	return m_Blend;
}

/// <summary>
/// Set whether to interpolate color keys.
/// </summary>
/// <param name="blend">rue to interpolate (blend), false to do hard cuts.</param>
void GradientColorsView::Blend(bool blend)
{
	m_Blend = blend;
}

/// <summary>
/// Set the focus to the arrow at the given normalized position.
/// </summary>
/// <param name="position">The normalized position of the arrow to focus</param>
void GradientColorsView::SetFocus(float position)
{
	bool focused = false;

	for (auto& it : m_Arrows)
	{
		focused |= position == it.first;
		it.second.Focus(position == it.first);
	}

	if (!focused)
		m_Arrows.begin()->second.Focus(true);

	update();
}

/// <summary>
/// Set the focus to the arrow at the given index.
/// </summary>
/// <param name="position">The index of the arrow to focus</param>
void GradientColorsView::SetFocus(size_t position)
{
	bool focused = false;
	size_t index = 0;
	position = std::min(m_Arrows.size() - 1, position);

	for (auto& it : m_Arrows)
	{
		bool b = position == index++;
		focused |= b;
		it.second.Focus(b);
	}

	if (!focused && !m_Arrows.empty())
		m_Arrows.begin()->second.Focus(true);

	update();
}

/// <summary>
/// Set the color of the currently focused arrow to the passed in color.
/// </summary>
/// <param name="color">The color to set the focused arrow to</param>
void GradientColorsView::SetFocusColor(const QColor& color)
{
	for (auto& it : m_Arrows)
	{
		auto& anArrow = it.second;

		if (anArrow.Focus())
		{
			anArrow.Color(color);
			update();
			break;
		}
	}
}

/// <summary>
/// Add an arrow whose color will be assigned the passed in color.
/// This should never be called on a fixed palette.
/// </summary>
/// <param name="color">The color to assign to the new arrow</param>
void GradientColorsView::AddArrow(const QColor& color)
{
	float position = 0.5f;

	if (m_Arrows.size() >= 256)
		return;

	if (m_Arrows.empty())
	{
		position = 0;
	}
	else if (m_Arrows.size() == 1)
	{
		position = (m_Arrows.begin()->first < 1) ? 1 : 0;
	}
	else if (m_Arrows.size() == 2)
	{
		const auto b = m_Arrows.begin();
		const auto rb = m_Arrows.rbegin();
		position = std::abs((rb->first + b->first) / 2.0);

		if (position == b->first)
			position = b->first / 2.0;
		else if (position == rb->first)
			position = (1.0 + rb->first) / 2.0;
	}
	else
	{
		bool set = false;
		auto it = m_Arrows.begin();
		const auto oneBeforeLast = Advance(m_Arrows.begin(), m_Arrows.size() - 1);

		for (; it != oneBeforeLast; ++it)
		{
			if (it->second.Focus())
			{
				auto next = Advance(it, 1);
				position = std::abs((next->first + it->first) / 2.0);
				set = true;
				break;
			}
		}

		if (!set)
		{
			it = m_Arrows.begin();
			position = std::abs((Advance(it, 1)->first + it->first) / 2.0);
		}
	}

	AddArrow(position, color);
}

/// <summary>
/// Add an arrow whose position and color will be assigned the values passed in.
/// If an arrow exists at the specified position, it is overwritten.
/// </summary>
/// <param name="position">The position to place the new arrow in</param>
/// <param name="color">The color to assign to the new arrow</param>
void GradientColorsView::AddArrow(float position, const QColor& color)
{
	GradientArrow arrow;
	arrow.Focus(true);
	arrow.Color(color);
	m_Arrows[position] = arrow;
	SetFocus(position);
	update();
}

/// <summary>
/// Delete the currently focused arrow if there are more than 2 arrows.
/// Set the focus to the arrow whose index is one greater than the one deleted.
/// </summary>
void GradientColorsView::DeleteFocusedArrow()
{
	if (m_Arrows.size() <= 2)
		return;

	size_t index = 0;

	for (auto it = m_Arrows.begin(); it != m_Arrows.end(); ++it)
	{
		if (it->second.Focus())
		{
			m_Arrows.erase(it);
			break;
		}

		index++;
	}

	SetFocus(index);
	update();
}

/// <summary>
/// Invert the values of all colors by subtracting each component from 255.
/// </summary>
void GradientColorsView::InvertColors()
{
	if (!m_Arrows.empty())
	{
		for (auto& it : m_Arrows)
		{
			auto& arrow = it.second;
			const auto col = arrow.Color();
			arrow.Color(QColor(255 - col.red(), 255 - col.green(), 255 - col.blue()));

			if (arrow.Focus())
				emit ArrowDoubleClicked(arrow);
		}
	}
	else
	{
		for (auto& col : m_Palette.m_Entries)
		{
			col.r = 1 - col.r;
			col.g = 1 - col.g;
			col.b = 1 - col.b;
		}

		SetPalette(m_Palette);//Reset associated pixmap etc...
	}

	update();
}

/// <summary>
/// Set each component of each color to a random value between 0 and 255 inclusive.
/// </summary>
void GradientColorsView::RandomColors()
{
	for (auto& it : m_Arrows)
		it.second.Color(
	{
		static_cast<int>(QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(256)),
		static_cast<int>(QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(256)),
		static_cast<int>(QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(256))
	});
	update();
}

/// <summary>
/// Set the distance between each arrow to be equal.
/// </summary>
void GradientColorsView::DistributeColors()
{
	if (!m_Arrows.empty())
	{
		map<float, GradientArrow> arrows;
		float index = 0;
		const auto inc = 1.0f / std::max<size_t>(size_t(1), m_Arrows.size() - 1);

		for (auto it : m_Arrows)
		{
			arrows[index] = it.second;
			index = std::min(1.0f, index + inc);
		}

		m_Arrows = std::move(arrows);
	}

	update();
}

/// <summary>
/// Delete all arrows and add a white arrow at index 0, and a black
/// arrow at index 1.
/// </summary>
void GradientColorsView::ResetToDefault()
{
	ClearArrows();
	AddArrow(0.0, Qt::GlobalColor::white);
	AddArrow(1.0, Qt::GlobalColor::black);
}

/// <summary>
/// Clear all arrows.
/// </summary>
void GradientColorsView::ClearArrows()
{
	m_Arrows.clear();
	update();
}

/// <summary>
/// Set the arrow at the specified index to the specified color, and also
/// focus it.
/// </summary>
/// <param name="color">The color to assign to the arrow at the specified index</param>
/// <param name="index">The index of the arrow to assign the color to and focus</param>
void GradientColorsView::NewFocusColor(const QColor& color, int index)
{
	int i = 0;

	for (auto& kv : m_Arrows)
	{
		auto& arrow = kv.second;

		if (i == index)
		{
			arrow.Color(color);
			arrow.Focus(true);
			update();
		}
		else
			arrow.Focus(false);

		kv.second = arrow;
		i++;
	}
}

/// <summary>
/// Set the arrow map to the passed in one.
/// </summary>
/// <param name="newArrows">The new arrows to assign to the internal m_Arrows member</param>
void GradientColorsView::SetArrows(map<float, GradientArrow>& newArrows)
{
	m_Arrows = newArrows;
	update();
}

/// <summary>
/// Get the number of arrows in the map.
/// </summary>
/// <returns>int</returns>
int GradientColorsView::ArrowCount()
{
	return int(m_Arrows.size());
}

/// <summary>
/// Get the index of the focused arrow.
/// Return 0 if none are focused.
/// </summary>
/// <returns>The focused index if at least one arrow is focused, else 0.</returns>
int GradientColorsView::GetFocusedIndex()
{
	int index = 0;

	for (auto& kv : m_Arrows)
	{
		if (kv.second.Focus())
			break;

		index++;
	}

	return index;
}

/// <summary>
/// Return a reference to the arrows map.
/// Be very careful what you do with this.
/// </summary>
/// <returns>A reference to the internal map containing the arrows</returns>
map<float, GradientArrow>& GradientColorsView::GetArrows()
{
	return m_Arrows;
}

/// <summary>
/// Populate the palette member with the specified number of elements based on
/// interpolating the values in the arrows and return a reference to it.
/// </summary>
/// <param name="size">The number of elements the palette will have</param>
/// <returns>A reference to the internal map containing the arrows</returns>
Palette<float>& GradientColorsView::GetPalette(int size)
{
	if (!m_Arrows.empty())
	{
		const QSize imageSize(size, 1);
		QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
		m_Palette.m_SourceColors.clear();
		QPainter painter(&image);
		float start = 0;

		if (Blend())
		{
			QLinearGradient grad(QPoint(0, 0), QPoint(imageSize.width(), imageSize.height()));

			for (auto& it : m_Arrows)
			{
				auto col = it.second.Color();
				m_Palette.m_SourceColors[it.first] = v4F(col.red() / 255.0f, col.green() / 255.0f, col.blue() / 255.0f, 1.0f);
				grad.setColorAt(it.first, col);
			}

			painter.fillRect(image.rect(), grad);
		}
		else
		{
			for (auto& it : m_Arrows)
			{
				auto col = it.second.Color();
				m_Palette.m_SourceColors[it.first] = v4F(col.red() / 255.0f, col.green() / 255.0f, col.blue() / 255.0f, 1.0f);
				painter.setBrush(col);
				painter.fillRect(start, 0, imageSize.width(), imageSize.height(), col);
				start = std::ceil(it.first * imageSize.width());
			}
		}

		m_Palette.m_Entries.reserve(image.width());

		for (int i = 0; i < image.width(); i++)
		{
			QColor col(image.pixel(i, 0));
			m_Palette[i].r = col.red() / 255.0f;
			m_Palette[i].g = col.green() / 255.0f;
			m_Palette[i].b = col.blue() / 255.0f;
		}
	}

	return m_Palette;//If fixed palette, just return verbatim.
}

/// <summary>
/// Assign the values of the m_SourceColors member of the palette to the
/// internal map of arrows. Note this assignment will only take place if
/// the number of source colors is 2 or more.
/// This will only be the case if it was a user created palette made here.
/// All palettes gotten from elsewhere are not assignable.
/// </summary>
/// <param name="palette">The palette whose source colors will be assigned to the arrow map</param>
void GradientColorsView::SetPalette(const Palette<float>& palette)
{
	m_Arrows.clear();
	m_Palette = palette;

	if (palette.m_SourceColors.size() > 1)
	{
		for (auto& col : m_Palette.m_SourceColors)
		{
			auto& rgb = col.second;
			m_Arrows[col.first] = GradientArrow(QColor(rgb.r * 255, rgb.g * 255, rgb.b * 255), false);
		}

		SetFocus(size_t(0));
	}
	else
	{
		auto v = m_Palette.MakeRgbPaletteBlock(m_ViewRect.height());//Make the palette repeat for PALETTE_CELL_HEIGHT rows.
		auto image = QImage(int(m_Palette.Size()), m_ViewRect.height(), QImage::Format_RGB888);//Create a QImage out of it.
		memcpy(image.scanLine(0), v.data(), v.size() * sizeof(v[0]));//Memcpy the data in.
		m_FinalFixedPixmap = QPixmap(QPixmap::fromImage(image));//Create a QPixmap out of the QImage.
	}

	update();
}

/// <summary>
/// Return a temporary copy of the xform color indices as a map.
/// The keys are the xform indices, and the values are the color indices.
/// </summary>
/// <param name="palette">The color indices</param>
map<size_t, float> GradientColorsView::GetColorIndices() const
{
	map<size_t, float> indices;

	for (auto& it : m_ColorIndicesArrows)
		indices[it.first] = it.second.first;

	return indices;
}

/// <summary>
/// Assign the values of the xform color indices to the arrows.
/// This will clear out any existing values first.
/// </summary>
/// <param name="palette">The color indices to assign</param>
void GradientColorsView::SetColorIndices(const map<size_t, float>& indices)
{
	QPainter painter(this);
	m_ColorIndicesArrows.clear();

	for (auto& it : indices)
	{
		auto text = QString::number(it.first + 1);//Get text width, which is used to adjust the size of the arrow.
		QFontMetrics fm = painter.fontMetrics();
		m_ColorIndicesArrows[it.first] = make_pair(it.second, TopArrow(fm.horizontalAdvance(text) + 5, it.first));
	}
}

/// <summary>
/// Custom paint event to draw the palette and arrows.
/// </summary>
void GradientColorsView::paintEvent(QPaintEvent*)
{
	if (m_ViewRect.size().isNull() ||
			m_ViewRect.size().isEmpty() ||
			m_ViewRect.topLeft() == m_ViewRect.bottomRight())
	{
		resizeEvent(nullptr);//Setup rects.
	}

	QPainter painter(this);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.setRenderHint(QPainter::Antialiasing);

	if (!m_Arrows.empty())
	{
		const QPoint gradStart = QPoint(m_ViewRect.topLeft().x(), m_ViewRect.bottomLeft().y() / 2);
		const QPoint gradStop = QPoint(m_ViewRect.topRight().x(), m_ViewRect.bottomRight().y() / 2);
		QLinearGradient grad(gradStart, gradStop);
		float start = m_ViewRect.x();

		for (auto& it : m_Arrows)
		{
			GradientArrow& arrow = it.second;
			const auto offset = std::ceil(it.first * RectWidth());

			if (Blend())
			{
				grad.setColorAt(it.first, arrow.Color());
			}
			else
			{
				painter.fillRect(start, m_ViewRect.y(), m_ViewRect.right() - start, m_ViewRect.height(), arrow.Color());
				start = m_ViewRect.x() + offset;
			}

			QPolygon arrowPolygon = arrow.Area();
			const auto iPosX = offset;
			const auto iPosY = m_ViewRect.height() + m_ViewRect.top() + 3;
			arrowPolygon.translate(iPosX, iPosY);
			QPainterPath paintPath;
			paintPath.addPolygon(arrowPolygon);
			painter.setBrush(QBrush(arrow.Color()));

			if (arrow.Focus())
				paintPath.addRect(iPosX + 5, iPosY + 20, 10, 5);

			painter.drawPath(paintPath);
			painter.setBrush(QBrush(Qt::BrushStyle::NoBrush));
		}

		if (Blend())
		{
			painter.fillRect(m_ViewRect, grad);
		}

		painter.drawRect(m_ViewRect);
	}
	else
	{
		painter.drawPixmap(m_ViewRect, m_FinalFixedPixmap);
	}

	for (auto& it : m_ColorIndicesArrows)
	{
		QPainterPath topArrowPaintPath;
		auto& topArrow = it.second.second;
		auto topArrowPolygon = topArrow.Area();
		topArrowPolygon.translate(it.second.first * RectWidth(), 0);
		const auto topArrowRect = topArrowPolygon.boundingRect();
		topArrowPaintPath.addPolygon(topArrowPolygon);
		painter.drawPath(topArrowPaintPath);//When using a separate painter, the sides aren't as thick.
		//Draw text inside of the arrow.
		painter.drawText(topArrowRect.x() + (topArrowRect.width() - (topArrow.Width() - 5)) / 2.0, topArrowRect.y() + (topArrowRect.height() - 5), topArrow.Text());
	}
}

/// <summary>
/// Event for detecting when the mouse is pressed on an arrow to begin dragging.
/// </summary>
/// <param name="e">The mouse event</param>
void GradientColorsView::mousePressEvent(QMouseEvent* e)
{
	m_DragStart = e->pos();

	for (auto& it : m_Arrows)
	{
		auto& arrow = it.second;
		QPolygon poly = arrow.Area();
		poly.translate(it.first * m_ViewRectSize.x(), m_ViewRectSize.y());

		if (poly.containsPoint(m_DragStart, Qt::FillRule::OddEvenFill))
		{
			m_ArrowMoving = true;
			arrow.Focus(true);
		}
		else
			arrow.Focus(false);
	}

	for (auto& it : m_ColorIndicesArrows)
	{
		auto& arrow = it.second.second;
		QPolygon poly = arrow.Area();
		poly.translate(it.second.first * m_ViewRectSize.x(), 0);

		if (poly.containsPoint(m_DragStart, Qt::FillRule::OddEvenFill))
		{
			m_ColorIndexArrowMoving = true;
			arrow.Focus(true);
		}
		else
			arrow.Focus(false);
	}

	update();
}

/// <summary>
/// Event for detecting when the mouse is pressed on an arrow to begin dragging.
/// </summary>
/// <param name="event">The mouse event</param>
void GradientColorsView::mouseDoubleClickEvent(QMouseEvent* e)
{
	for (auto& it : m_Arrows)
	{
		auto& arrow = it.second;
		QPolygon poly = arrow.Area();
		poly.translate(it.first * m_ViewRectSize.x(), m_ViewRectSize.y());

		if (poly.containsPoint(e->pos(), Qt::FillRule::OddEvenFill))
		{
			arrow.Focus(true);
			emit ArrowDoubleClicked(arrow);
		}
		else
			arrow.Focus(false);
	}
}

/// <summary>
/// Event for detecting when the mouse is moving during dragging.
/// </summary>
/// <param name="event">The mouse event</param>
void GradientColorsView::mouseMoveEvent(QMouseEvent* e)
{
	if (!m_ArrowMoving && !m_ColorIndexArrowMoving) return;

	size_t index = 0;
	const qreal maxMove = 11.5 / RectWidth();

	if (m_ArrowMoving)
	{
		for (auto it = m_Arrows.begin(); it != m_Arrows.end(); ++it)
		{
			auto& arrow = it->second;

			if (arrow.Focus())
			{
				const qreal lastPos = it->first;
				const qreal start = m_DragStart.x();
				const qreal end = RectWidth();
				const qreal dPos = ((qreal)e->pos().x() - start) / end;
				const qreal newPos = lastPos + dPos;

				if ((lastPos + dPos > 1) || (lastPos + dPos < 0))
					return;

				if (dPos < 0 && index > 0)
				{
					const qreal posBefore = std::prev(it)->first;

					if ((lastPos - maxMove + dPos) <= posBefore)
						return;
				}

				if ((dPos > 0) && (index < (m_Arrows.size() - 1)))
				{
					const qreal posAfter = std::next(it)->first;

					if ((lastPos + maxMove + dPos) >= posAfter)
						return;
				}

				GradientArrow arrowCopy(it->second);
				m_Arrows.erase(lastPos);
				m_Arrows[newPos] = arrowCopy;
				emit ArrowMove(lastPos, arrow);
				break;
			}

			index++;
		}
	}
	else if (m_ColorIndexArrowMoving)
	{
		for (auto& it : m_ColorIndicesArrows)
		{
			auto& arrow = it.second.second;

			if (arrow.Focus())
			{
				const qreal lastPos = it.second.first;
				const qreal start = m_DragStart.x();
				const qreal end = RectWidth();
				const qreal dPos = ((qreal)e->pos().x() - start) / end;
				const qreal newPos = lastPos + dPos;

				if ((lastPos + dPos > 1) || (lastPos + dPos < 0))
					return;

				it.second.first = newPos;
				emit ColorIndexMove(it.first, it.second.first);
				break;
			}
		}
	}

	m_DragStart = e->pos();
	update();
}

/// <summary>
/// Event for detecting when the mouse is released during dragging.
/// </summary>
void GradientColorsView::mouseReleaseEvent(QMouseEvent*)
{
	m_ArrowMoving = false;
	m_ColorIndexArrowMoving = false;
}

/// <summary>
/// Event for custom drawing the viewable area when its resized.
/// </summary>
void GradientColorsView::resizeEvent(QResizeEvent*)
{
	m_ViewRectSize = QPoint(RectWidth(), RectHeight());
	m_ViewRect = QRect(m_ViewRectOffset, QPoint(m_ViewRectSize.x() + 5, m_ViewRectSize.y() - 10));
	m_ViewRect.translate(m_ViewRectTranslate);
}

/// <summary>
/// Return the width used to draw the gradient area.
/// </summary>
/// <returns>The width</returns>
int GradientColorsView::RectWidth()
{
	return width() - 20;
}

/// <summary>
/// Return the height used to draw the gradient area.
/// </summary>
/// <returns>The height</returns>
int GradientColorsView::RectHeight()
{
	return height() / 3 * 2;
}
