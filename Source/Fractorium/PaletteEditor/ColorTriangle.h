/****************************************************************************
**
** This file is part of a Qt Solutions component.
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact:  Qt Software Information (qt-info@nokia.com)
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/
#pragma once

#include "FractoriumPch.h"

/// <summary>
/// DoubleColor, Vertex and ColorTriangle classes.
/// </summary>

/// <summary>
/// Used to store color values in the range 0..255 as doubles.
/// </summary>
struct DoubleColor
{
	double r, g, b;

	DoubleColor() : r(0.0), g(0.0), b(0.0) {}
	DoubleColor(double red, double green, double blue) : r(red), g(green), b(blue) {}
	DoubleColor(const DoubleColor& c) : r(c.r), g(c.g), b(c.b) {}
    DoubleColor& operator = (const DoubleColor& carToRas) = default;
};

/// <summary>
/// Used to store pairs of DoubleColor and DoublePoint in one structure.
/// </summary>
struct Vertex
{
	DoubleColor color;
	QPointF point;

	Vertex(const DoubleColor& c, const QPointF& p) : color(c), point(p) {}
	Vertex(const QColor& c, const QPointF& p)
		: color(DoubleColor(static_cast<double>(c.red()), static_cast<double>(c.green()),
							static_cast<double>(c.blue()))), point(p) {}
};

/// <summary>
/// Widget for drawing a color triangle which allows users to select colors
/// in a manner more intuitive than the usual color picker dialog.
/// This class was taken from an open source project named Chaos Helper, which took
/// it from the Qt examples, so it mostly remains as-is.
/// </summary>
class ColorTriangle : public QWidget
{
	Q_OBJECT

public:
	ColorTriangle(QWidget* parent = nullptr);

	void Polish();
	QColor Color() const;
	void Color(const QColor& col);

	int heightForWidth(int w) const override;
	QSize sizeHint() const override;

Q_SIGNALS:
	void ColorChanged(const QColor& col);

protected:
	void paintEvent(QPaintEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void keyPressEvent(QKeyEvent* e) override;
	void resizeEvent(QResizeEvent*) override;

private:
	void InnerPolish();
	void GenBackground();
	void DrawTrigon(QImage* p, const QPointF& a, const QPointF& b, const QPointF& c, const QColor& color);
	double CalcOuterRadius() const;
	double RadiusAt(const QPointF& pos, const QRect& rect) const;
	double AngleAt(const QPointF& pos, const QRect& rect) const;
	QColor ColorFromPoint(const QPointF& p) const;
	QPointF PointFromColor(const QColor& col) const;
	QPointF MovePointToTriangle(double x, double y, const Vertex& a, const Vertex& b, const Vertex& c) const;

	bool mustGenerateBackground;
	int curHue;
	int penWidth;
	int ellipseSize;
	int outerRadius;
    double _a, _b, _c;
	QImage bg;
	QColor curColor;
    QPointF _pa, _pb, _pc, _pd;
	QPointF selectorPos;

	enum SelectionMode
	{
		Idle,
		SelectingHue,
		SelectingSatValue
	} selMode;
};
