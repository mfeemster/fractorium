/*

	Copyright (C) 2009, Etienne Moutot <e.moutot@gmail.com>

	This file is part of colorPickerWidget.

	colorPickerWidget is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	colorPickerWidget is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "FractoriumPch.h"
#include "ColorPickerWidget.h"

/// <summary>
/// Constructor which passes parent widget to the base and initializes the member controls
/// on a grid layout.
/// </summary>
/// <param name="p">The parent widget</param>
ColorPickerWidget::ColorPickerWidget(QWidget* p)
	: QWidget(p)
{
	m_ColorTriangle = new ColorTriangle(this);
	m_ColorPanel = new ColorPanel(this);
	m_ColorDialog = new QColorDialog(this);
	m_ColorPanel->Color(m_ColorTriangle->Color());
	connect(m_ColorTriangle, SIGNAL(ColorChanged(const QColor&)), this, SLOT(OnTriangleColorChanged(const QColor&)));
	connect(m_ColorPanel,    SIGNAL(clicked()),                   this, SLOT(OnColorViewerClicked()));
	auto layout = new QGridLayout(this);
	layout->setContentsMargins(4, 4, 4, 4);
	layout->addWidget(m_ColorTriangle, 0, 0, 3, 1);
	layout->addWidget(m_ColorPanel,    0, 1, 3, 1);
	setLayout(layout);
}

/// <summary>
/// Get the color used to paint the color panel.
/// </summary>
/// <returns>QColor</returns>
QColor ColorPickerWidget::Color() const
{
	return m_ColorPanel->Color();
}

/// <summary>
/// Set the current color for the triangle.
/// </summary>
/// <param name="col">The parent widget</param>
void ColorPickerWidget::SetColorPanelColor(const QColor& col)
{
	if (col.isValid())
		m_ColorTriangle->Color(col);//Internally emits ColorChanged() which will call OnTriangleColorChanged(), which will call m_ColorPanel->Color().
}

/// <summary>
/// Overridden resize event to set the color panel height slightly
/// smaller than the container it's in.
/// </summary>
/// <param name="col">The resize event</param>
void ColorPickerWidget::resizeEvent(QResizeEvent* event)
{
	m_ColorPanel->setMinimumHeight(event->size().height() - 22);
	m_ColorPanel->setMaximumHeight(event->size().height() - 22);
}

/// <summary>
/// Slot called when the color panel is clicked, which will show the color
/// picker dialog.
/// </summary>
void ColorPickerWidget::OnColorViewerClicked()
{
	m_ColorDialog->setCurrentColor(m_ColorPanel->Color());
	const auto newColor = m_ColorDialog->getColor(m_ColorPanel->Color(), this);
	SetColorPanelColor(newColor);
}

/// <summary>
/// Slot called when the color on the triangle changes for any reason,
/// either user initiated or programatically called.
/// </summary>
/// <param name="col">The new color on the triangle</param>
void ColorPickerWidget::OnTriangleColorChanged(const QColor& col)
{
	if (col.isValid())
	{
		m_ColorPanel->Color(col);
		emit ColorChanged(col);
	}
}
