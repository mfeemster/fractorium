/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "FractoriumPch.h"
#include "qcssscanner.h"

/// <summary>
/// The code in this file did not originate in Fractorium.
/// It was taken either in whole or in part from the source code
/// of Qt Creator. Their license applies.
/// </summary>

using namespace QCss2;

struct QCssKnownValue
{
	const char* name;
	quint64 id;
};

static const QCssKnownValue properties[NumProperties2 - 1] =
{
	{ "-qt-background-role", QtBackgroundRole2 },
	{ "-qt-block-indent", QtBlockIndent2 },
	{ "-qt-list-indent", QtListIndent2 },
	{ "-qt-list-number-prefix", QtListNumberPrefix2 },
	{ "-qt-list-number-suffix", QtListNumberSuffix2 },
	{ "-qt-paragraph-type", QtParagraphType2 },
	{ "-qt-style-features", QtStyleFeatures2 },
	{ "-qt-table-type", QtTableType2 },
	{ "-qt-user-state", QtUserState2 },
	{ "alternate-background-color", QtAlternateBackground2 },
	{ "background", Background2 },
	{ "background-attachment", BackgroundAttachment2 },
	{ "background-clip", BackgroundClip2 },
	{ "background-color", BackgroundColor2 },
	{ "background-image", BackgroundImage2 },
	{ "background-origin", BackgroundOrigin2 },
	{ "background-position", BackgroundPosition2 },
	{ "background-repeat", BackgroundRepeat2 },
	{ "border", Border2 },
	{ "border-bottom", BorderBottom2 },
	{ "border-bottom-color", BorderBottomColor2 },
	{ "border-bottom-left-radius", BorderBottomLeftRadius2 },
	{ "border-bottom-right-radius", BorderBottomRightRadius2 },
	{ "border-bottom-style", BorderBottomStyle2 },
	{ "border-bottom-width", BorderBottomWidth2 },
	{ "border-color", BorderColor2 },
	{ "border-image", BorderImage2 },
	{ "border-left", BorderLeft2 },
	{ "border-left-color", BorderLeftColor2 },
	{ "border-left-style", BorderLeftStyle2 },
	{ "border-left-width", BorderLeftWidth2 },
	{ "border-radius", BorderRadius2 },
	{ "border-right", BorderRight2 },
	{ "border-right-color", BorderRightColor2 },
	{ "border-right-style", BorderRightStyle2 },
	{ "border-right-width", BorderRightWidth2 },
	{ "border-style", BorderStyles2 },
	{ "border-top", BorderTop2 },
	{ "border-top-color", BorderTopColor2 },
	{ "border-top-left-radius", BorderTopLeftRadius2 },
	{ "border-top-right-radius", BorderTopRightRadius2 },
	{ "border-top-style", BorderTopStyle2 },
	{ "border-top-width", BorderTopWidth2 },
	{ "border-width", BorderWidth2 },
	{ "bottom", Bottom2 },
	{ "color", Property2::Color2 },
	{ "float", Float2 },
	{ "font", QCss2::Font2 },
	{ "font-family", FontFamily2 },
	{ "font-size", FontSize2 },
	{ "font-style", FontStyle2 },
	{ "font-variant", FontVariant2 },
	{ "font-weight", FontWeight2 },
	{ "height", Height2 },
	{ "image", QtImage2 },
	{ "image-position", QtImageAlignment2 },
	{ "left", Left2 },
	{ "line-height", LineHeight2 },
	{ "list-style", ListStyle2 },
	{ "list-style-type", ListStyleType2 },
	{ "margin", Margin2 },
	{ "margin-bottom", MarginBottom2 },
	{ "margin-left", MarginLeft2 },
	{ "margin-right", MarginRight2 },
	{ "margin-top", MarginTop2 },
	{ "max-height", MaximumHeight2 },
	{ "max-width", MaximumWidth2 },
	{ "min-height", MinimumHeight2 },
	{ "min-width", MinimumWidth2 },
	{ "outline", Outline2 },
	{ "outline-bottom-left-radius", OutlineBottomLeftRadius2 },
	{ "outline-bottom-right-radius", OutlineBottomRightRadius2 },
	{ "outline-color", OutlineColor2 },
	{ "outline-offset", OutlineOffset2 },
	{ "outline-radius", OutlineRadius2 },
	{ "outline-style", OutlineStyle2 },
	{ "outline-top-left-radius", OutlineTopLeftRadius2 },
	{ "outline-top-right-radius", OutlineTopRightRadius2 },
	{ "outline-width", OutlineWidth2 },
	{ "padding", Padding2 },
	{ "padding-bottom", PaddingBottom2 },
	{ "padding-left", PaddingLeft2 },
	{ "padding-right", PaddingRight2 },
	{ "padding-top", PaddingTop2 },
	{ "page-break-after", PageBreakAfter2 },
	{ "page-break-before", PageBreakBefore2 },
	{ "position", Position2 },
	{ "right", Right2 },
	{ "selection-background-color", QtSelectionBackground2 },
	{ "selection-color", QtSelectionForeground2 },
	{ "spacing", QtSpacing2 },
	{ "subcontrol-origin", QtOrigin2 },
	{ "subcontrol-position", QtPosition2 },
	{ "text-align", TextAlignment2 },
	{ "text-decoration", TextDecoration2 },
	{ "text-indent", TextIndent2 },
	{ "text-transform", TextTransform2 },
	{ "text-underline-style", TextUnderlineStyle2 },
	{ "top", Top2 },
	{ "vertical-align", VerticalAlignment2 },
	{ "white-space", Whitespace2 },
	{ "width", Width2 }
};

static const QCssKnownValue values[NumKnownValues2 - 1] =
{
	{ "active", Value_Active2 },
	{ "alternate-base", Value_AlternateBase2 },
	{ "always", Value_Always2 },
	{ "auto", Value_Auto2 },
	{ "base", Value_Base2 },
	{ "bold", Value_Bold2 },
	{ "bottom", Value_Bottom2 },
	{ "bright-text", Value_BrightText2 },
	{ "button", Value_Button2 },
	{ "button-text", Value_ButtonText2 },
	{ "center", Value_Center2 },
	{ "circle", Value_Circle2 },
	{ "dark", Value_Dark2 },
	{ "dashed", Value_Dashed2 },
	{ "decimal", Value_Decimal2 },
	{ "disabled", Value_Disabled2 },
	{ "disc", Value_Disc2 },
	{ "dot-dash", Value_DotDash2 },
	{ "dot-dot-dash", Value_DotDotDash2 },
	{ "dotted", Value_Dotted2 },
	{ "double", Value_Double2 },
	{ "groove", Value_Groove2 },
	{ "highlight", Value_Highlight2 },
	{ "highlighted-text", Value_HighlightedText2 },
	{ "inset", Value_Inset2 },
	{ "italic", Value_Italic2 },
	{ "large", Value_Large2 },
	{ "left", Value_Left2 },
	{ "light", Value_Light2 },
	{ "line-through", Value_LineThrough2 },
	{ "link", Value_Link2 },
	{ "link-visited", Value_LinkVisited2 },
	{ "lower-alpha", Value_LowerAlpha2 },
	{ "lower-roman", Value_LowerRoman2 },
	{ "lowercase", Value_Lowercase2 },
	{ "medium", Value_Medium2 },
	{ "mid", Value_Mid2 },
	{ "middle", Value_Middle2 },
	{ "midlight", Value_Midlight2 },
	{ "native", Value_Native2 },
	{ "none", Value_None2 },
	{ "normal", Value_Normal2 },
	{ "nowrap", Value_NoWrap2 },
	{ "oblique", Value_Oblique2 },
	{ "off", Value_Off2 },
	{ "on", Value_On2 },
	{ "outset", Value_Outset2 },
	{ "overline", Value_Overline2 },
	{ "pre", Value_Pre2 },
	{ "pre-wrap", Value_PreWrap2 },
	{ "ridge", Value_Ridge2 },
	{ "right", Value_Right2 },
	{ "selected", Value_Selected2 },
	{ "shadow", Value_Shadow2 },
	{ "small", Value_Small2 },
	{ "small-caps", Value_SmallCaps2 },
	{ "solid", Value_Solid2 },
	{ "square", Value_Square2 },
	{ "sub", Value_Sub2 },
	{ "super", Value_Super2 },
	{ "text", Value_Text2 },
	{ "top", Value_Top2 },
	{ "transparent", Value_Transparent2 },
	{ "underline", Value_Underline2 },
	{ "upper-alpha", Value_UpperAlpha2 },
	{ "upper-roman", Value_UpperRoman2 },
	{ "uppercase", Value_Uppercase2 },
	{ "wave", Value_Wave2 },
	{ "window", Value_Window2 },
	{ "window-text", Value_WindowText2 },
	{ "x-large", Value_XLarge2 },
	{ "xx-large", Value_XXLarge2 }
};

//Map id to strings as they appears in the 'values' array above
static const short indexOfId[NumKnownValues2] = { 0, 41, 48, 42, 49, 54, 35, 26, 70, 71, 25, 43, 5, 63, 47,
												 29, 58, 59, 27, 51, 61, 6, 10, 39, 56, 19, 13, 17, 18, 20, 21, 50, 24, 46, 67, 37, 3, 2, 40, 62, 16,
												 11, 57, 14, 32, 64, 33, 65, 55, 66, 34, 69, 8, 28, 38, 12, 36, 60, 7, 9, 4, 68, 53, 22, 23, 30, 31,
												 1, 15, 0, 52, 45, 44
											   };

QString Value2::toString() const
{
	if (type == KnownIdentifier2)
	{
		return QLatin1String(values[indexOfId[variant.toInt()]].name);
	}
	else
	{
		return variant.toString();
	}
}

static const QCssKnownValue pseudos[NumPseudos - 1] =
{
	{ "active", PseudoClass_Active },
	{ "adjoins-item", PseudoClass_Item },
	{ "alternate", PseudoClass_Alternate },
	{ "bottom", PseudoClass_Bottom },
	{ "checked", PseudoClass_Checked },
	{ "closable", PseudoClass_Closable },
	{ "closed", PseudoClass_Closed },
	{ "default", PseudoClass_Default },
	{ "disabled", PseudoClass_Disabled },
	{ "edit-focus", PseudoClass_EditFocus },
	{ "editable", PseudoClass_Editable },
	{ "enabled", PseudoClass_Enabled },
	{ "exclusive", PseudoClass_Exclusive },
	{ "first", PseudoClass_First },
	{ "flat", PseudoClass_Flat },
	{ "floatable", PseudoClass_Floatable },
	{ "focus", PseudoClass_Focus },
	{ "has-children", PseudoClass_Children },
	{ "has-siblings", PseudoClass_Sibling },
	{ "horizontal", PseudoClass_Horizontal },
	{ "hover", PseudoClass_Hover },
	{ "indeterminate", PseudoClass_Indeterminate },
	{ "last", PseudoClass_Last },
	{ "left", PseudoClass_Left },
	{ "maximized", PseudoClass_Maximized },
	{ "middle", PseudoClass_Middle },
	{ "minimized", PseudoClass_Minimized },
	{ "movable", PseudoClass_Movable },
	{ "next-selected", PseudoClass_NextSelected },
	{ "no-frame", PseudoClass_Frameless },
	{ "non-exclusive", PseudoClass_NonExclusive },
	{ "off", PseudoClass_Unchecked },
	{ "on", PseudoClass_Checked },
	{ "only-one", PseudoClass_OnlyOne },
	{ "open", PseudoClass_Open },
	{ "pressed", PseudoClass_Pressed },
	{ "previous-selected", PseudoClass_PreviousSelected },
	{ "read-only", PseudoClass_ReadOnly },
	{ "right", PseudoClass_Right },
	{ "selected", PseudoClass_Selected },
	{ "top", PseudoClass_Top },
	{ "unchecked", PseudoClass_Unchecked },
	{ "vertical", PseudoClass_Vertical },
	{ "window", PseudoClass_Window }
};

static const QCssKnownValue origins[Origin2::NumKnownOrigins2 - 1] =
{
	{ "border", Origin2::Origin_Border2 },
	{ "content", Origin2::Origin_Content2 },
	{ "margin", Origin2::Origin_Margin2 }, // not in css
	{ "padding", Origin2::Origin_Padding2 }
};

static const QCssKnownValue repeats[Repeat2::NumKnownRepeats2 - 1] =
{
	{ "no-repeat", Repeat2::Repeat_None2 },
	{ "repeat-x", Repeat2::Repeat_X2 },
	{ "repeat-xy", Repeat2::Repeat_XY2 },
	{ "repeat-y", Repeat2::Repeat_Y2 }
};

static const QCssKnownValue tileModes[TileMode2::NumKnownTileModes2 - 1] =
{
	{ "repeat", TileMode2::TileMode_Repeat2 },
	{ "round", TileMode2::TileMode_Round2 },
	{ "stretch", TileMode2::TileMode_Stretch2 },
};

static const QCssKnownValue positions[PositionMode2::NumKnownPositionModes2 - 1] =
{
	{ "absolute", PositionMode2::PositionMode_Absolute2 },
	{ "fixed", PositionMode2::PositionMode_Fixed2 },
	{ "relative", PositionMode2::PositionMode_Relative2 },
	{ "static", PositionMode2::PositionMode_Static2	}
};

static const QCssKnownValue attachments[Attachment2::NumKnownAttachments2 - 1] =
{
	{ "fixed", Attachment2::Attachment_Fixed2 },
	{ "scroll", Attachment2::Attachment_Scroll2 }
};

static const QCssKnownValue styleFeatures[StyleFeature2::NumKnownStyleFeatures2 - 1] =
{
	{ "background-color", StyleFeature2::StyleFeature_BackgroundColor2 },
	{ "background-gradient", StyleFeature2::StyleFeature_BackgroundGradient2 },
	{ "none", StyleFeature2::StyleFeature_None2 }
};

#if defined(Q_CC_MSVC) && _MSC_VER < 1600
Q_STATIC_GLOBAL_OPERATOR bool operator<(const QCssKnownValue& prop1, const QCssKnownValue& prop2)
{
	return QString::compare(QString::fromLatin1(prop1.name), QLatin1String(prop2.name), Qt::CaseSensitivity::CaseInsensitive) < 0;
}
#endif

Q_STATIC_GLOBAL_OPERATOR bool operator<(const QString& name, const QCssKnownValue& prop) noexcept
{
	return QString::compare(name, QLatin1String(prop.name), Qt::CaseSensitivity::CaseInsensitive) < 0;
}

Q_STATIC_GLOBAL_OPERATOR bool operator<(const QCssKnownValue& prop, const QString& name) noexcept
{
	return QString::compare(QLatin1String(prop.name), name, Qt::CaseSensitivity::CaseInsensitive) < 0;
}

static quint64 findKnownValue(const QString& name, const QCssKnownValue* start, int numValues)
{
	if (start != nullptr)
	{
		const QCssKnownValue* end = &start[numValues - 1];
		const QCssKnownValue* prop = std::lower_bound(start, end, name);

		if (prop != nullptr)
			if ((prop == end) || (name < *prop))
				return 0;

		return prop->id;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Value Extractor
ValueExtractor2::ValueExtractor2(const QVector<Declaration2>& decls, const QPalette& palette)
	: declarations(decls), adjustment(0), fontExtracted(false), pal(palette)
{
}

LengthData2 ValueExtractor2::lengthValue(const Value2& v)
{
	QString s = v.variant.toString();
	s.reserve(s.length());
	LengthData2 data;
	data.unit = LengthData2::NONE;

	if (s.endsWith(QLatin1String("px"), Qt::CaseSensitivity::CaseInsensitive))
		data.unit = LengthData2::Px;
	else if (s.endsWith(QLatin1String("ex"), Qt::CaseSensitivity::CaseInsensitive))
		data.unit = LengthData2::Ex;
	else if (s.endsWith(QLatin1String("em"), Qt::CaseSensitivity::CaseInsensitive))
		data.unit = LengthData2::Em;

	if (data.unit != LengthData2::NONE)
		s.chop(2);

	data.number = s.toDouble();
	return data;
}

static int lengthValueFromData(const LengthData2& data, const QFont& f)
{
	if (data.unit == LengthData2::Ex)
		return qRound(QFontMetrics(f).xHeight() * data.number);
	else if (data.unit == LengthData2::Em)
		return qRound(QFontMetrics(f).height() * data.number);

	return qRound(data.number);
}

int ValueExtractor2::lengthValue(const Declaration2& decl)
{
	if (decl.d->parsed.isValid())
		return  lengthValueFromData(qvariant_cast<LengthData2>(decl.d->parsed), f);

	if (decl.d->values.count() < 1)
		return 0;

	LengthData2 const data = lengthValue(decl.d->values.at(0));
	decl.d->parsed = QVariant::fromValue<LengthData2>(data);
	return lengthValueFromData(data, f);
}

void ValueExtractor2::lengthValues(const Declaration2& decl, int* m)
{
	if (m != nullptr)
	{
		if (decl.d->parsed.isValid())
		{
			QList<QVariant> v = decl.d->parsed.toList();

			for (int i = 0; i < 4; i++)
				m[i] = lengthValueFromData(qvariant_cast<LengthData2>(v.at(i)), f);

			return;
		}

		LengthData2 datas[4];
		int i;

		for (i = 0; i < qMin(decl.d->values.count(), 4); i++)
			datas[i] = lengthValue(decl.d->values[i]);

		if (i == 0)
		{
			LengthData2 zero = { 0.0, LengthData2::NONE };
			datas[0] = datas[1] = datas[2] = datas[3] = zero;
		}
		else if (i == 1)
		{
			datas[3] = datas[2] = datas[1] = datas[0];
		}
		else if (i == 2)
		{
			datas[2] = datas[0];
			datas[3] = datas[1];
		}
		else if (i == 3)
		{
			datas[3] = datas[1];
		}

		QList<QVariant> v;

		for (i = 0; i < 4; i++)
		{
			v += QVariant::fromValue<LengthData2>(datas[i]);
			m[i] = lengthValueFromData(datas[i], f);
		}

		decl.d->parsed = v;
	}
}

bool ValueExtractor2::extractGeometry(int* w, int* h, int* minw, int* minh, int* maxw, int* maxh)
{
	extractFont();
	bool hit = false;

	for (int i = 0; i < declarations.count(); i++)
	{
		const Declaration2& decl = declarations.at(i);

		switch (decl.d->propertyId)
		{
			case Width2: if (w != nullptr) *w = lengthValue(decl); break;

			case Height2: if (h != nullptr)  *h = lengthValue(decl); break;

			case MinimumWidth2: if (minw != nullptr)  *minw = lengthValue(decl); break;

			case MinimumHeight2: if (minh != nullptr) *minh = lengthValue(decl); break;

			case MaximumWidth2: if (maxw != nullptr) *maxw = lengthValue(decl); break;

			case MaximumHeight2: if (maxh != nullptr) *maxh = lengthValue(decl); break;

			case BackgroundColor2:
			case UnknownProperty2:
			case Color2:
			case Float2:
			case Font2:
			case FontFamily2:
			case FontSize2:
			case FontStyle2:
			case FontWeight2:
			case Margin2:
			case MarginBottom2:
			case MarginLeft2:
			case MarginRight2:
			case MarginTop2:
			case QtBlockIndent2:
			case QtListIndent2:
			case QtParagraphType2:
			case QtTableType2:
			case QtUserState2:
			case TextDecoration2:
			case TextIndent2:
			case TextUnderlineStyle2:
			case VerticalAlignment2:
			case Whitespace2:
			case QtSelectionForeground2:
			case QtSelectionBackground2:
			case Border2:
			case BorderLeft2:
			case BorderRight2:
			case BorderTop2:
			case BorderBottom2:
			case Padding2:
			case PaddingLeft2:
			case PaddingRight2:
			case PaddingTop2:
			case PaddingBottom2:
			case PageBreakBefore2:
			case PageBreakAfter2:
			case QtAlternateBackground2:
			case BorderLeftStyle2:
			case BorderRightStyle2:
			case BorderTopStyle2:
			case BorderBottomStyle2:
			case BorderStyles2:
			case BorderLeftColor2:
			case BorderRightColor2:
			case BorderTopColor2:
			case BorderBottomColor2:
			case BorderColor2:
			case BorderLeftWidth2:
			case BorderRightWidth2:
			case BorderTopWidth2:
			case BorderBottomWidth2:
			case BorderWidth2:
			case BorderTopLeftRadius2:
			case BorderTopRightRadius2:
			case BorderBottomLeftRadius2:
			case BorderBottomRightRadius2:
			case BorderRadius2:
			case Background2:
			case BackgroundOrigin2:
			case BackgroundClip2:
			case BackgroundRepeat2:
			case BackgroundPosition2:
			case BackgroundAttachment2:
			case BackgroundImage2:
			case BorderImage2:
			case QtSpacing2:
			case QtImage2:
			case Left2:
			case Right2:
			case Top2:
			case Bottom2:
			case QtOrigin2:
			case QtPosition2:
			case Position2:
			case QtStyleFeatures2:
			case QtBackgroundRole2:
			case ListStyleType2:
			case ListStyle2:
			case QtImageAlignment2:
			case TextAlignment2:
			case Outline2:
			case OutlineOffset2:
			case OutlineWidth2:
			case OutlineColor2:
			case OutlineStyle2:
			case OutlineRadius2:
			case OutlineTopLeftRadius2:
			case OutlineTopRightRadius2:
			case OutlineBottomLeftRadius2:
			case OutlineBottomRightRadius2:
			case FontVariant2:
			case TextTransform2:
			case QtListNumberPrefix2:
			case QtListNumberSuffix2:
			case LineHeight2:
			case NumProperties2:
			default: continue;
		}

		hit = true;
	}

	return hit;
}

bool ValueExtractor2::extractPosition(int* left, int* top, int* right, int* bottom, QCss2::Origin2* origin,
									 Qt::Alignment* position, QCss2::PositionMode2* mode, Qt::Alignment* textAlignment)
{
	extractFont();
	bool hit = false;

	for (int i = 0; i < declarations.count(); i++)
	{
		const Declaration2& decl = declarations.at(i);

		switch (decl.d->propertyId)
		{
			case Left2: if (left != nullptr)  *left = lengthValue(decl); break;

			case Top2: if (top != nullptr) *top = lengthValue(decl); break;

			case Right2: if (right != nullptr) *right = lengthValue(decl); break;

			case Bottom2: if (bottom != nullptr) *bottom = lengthValue(decl); break;

			case QtOrigin2: if (origin != nullptr) *origin = decl.originValue(); break;

			case QtPosition2: if (position != nullptr) *position = decl.alignmentValue(); break;

			case TextAlignment2: if (textAlignment != nullptr) *textAlignment = decl.alignmentValue(); break;

			case Position2: if (mode != nullptr) *mode = decl.positionValue(); break;

			case BackgroundColor2:
			case UnknownProperty2:
			case Color2:
			case Float2:
			case Font2:
			case FontFamily2:
			case FontSize2:
			case FontStyle2:
			case FontWeight2:
			case Margin2:
			case MarginBottom2:
			case MarginLeft2:
			case MarginRight2:
			case MarginTop2:
			case QtBlockIndent2:
			case QtListIndent2:
			case QtParagraphType2:
			case QtTableType2:
			case QtUserState2:
			case TextDecoration2:
			case TextIndent2:
			case TextUnderlineStyle2:
			case VerticalAlignment2:
			case Whitespace2:
			case QtSelectionForeground2:
			case QtSelectionBackground2:
			case Border2:
			case BorderLeft2:
			case BorderRight2:
			case BorderTop2:
			case BorderBottom2:
			case Padding2:
			case PaddingLeft2:
			case PaddingRight2:
			case PaddingTop2:
			case PaddingBottom2:
			case PageBreakBefore2:
			case PageBreakAfter2:
			case QtAlternateBackground2:
			case BorderLeftStyle2:
			case BorderRightStyle2:
			case BorderTopStyle2:
			case BorderBottomStyle2:
			case BorderStyles2:
			case BorderLeftColor2:
			case BorderRightColor2:
			case BorderTopColor2:
			case BorderBottomColor2:
			case BorderColor2:
			case BorderLeftWidth2:
			case BorderRightWidth2:
			case BorderTopWidth2:
			case BorderBottomWidth2:
			case BorderWidth2:
			case BorderTopLeftRadius2:
			case BorderTopRightRadius2:
			case BorderBottomLeftRadius2:
			case BorderBottomRightRadius2:
			case BorderRadius2:
			case Background2:
			case BackgroundOrigin2:
			case BackgroundClip2:
			case BackgroundRepeat2:
			case BackgroundPosition2:
			case BackgroundAttachment2:
			case BackgroundImage2:
			case BorderImage2:
			case QtSpacing2:
			case Width2:
			case Height2:
			case MinimumWidth2:
			case MinimumHeight2:
			case MaximumWidth2:
			case MaximumHeight2:
			case QtImage2:
			case QtStyleFeatures2:
			case QtBackgroundRole2:
			case ListStyleType2:
			case ListStyle2:
			case QtImageAlignment2:
			case Outline2:
			case OutlineOffset2:
			case OutlineWidth2:
			case OutlineColor2:
			case OutlineStyle2:
			case OutlineRadius2:
			case OutlineTopLeftRadius2:
			case OutlineTopRightRadius2:
			case OutlineBottomLeftRadius2:
			case OutlineBottomRightRadius2:
			case FontVariant2:
			case TextTransform2:
			case QtListNumberPrefix2:
			case QtListNumberSuffix2:
			case LineHeight2:
			case NumProperties2:
			default: continue;
		}

		hit = true;
	}

	return hit;
}

bool ValueExtractor2::extractBox(int* margins, int* paddings, int* spacing)
{
	extractFont();
	bool hit = false;

	for (int i = 0; i < declarations.count(); i++)
	{
		const Declaration2& decl = declarations.at(i);

		switch (decl.d->propertyId)
		{
			case PaddingLeft2: if (paddings != nullptr) paddings[Edge2::LeftEdge2] = lengthValue(decl); break;

			case PaddingRight2: if (paddings != nullptr) paddings[Edge2::RightEdge2] = lengthValue(decl); break;

			case PaddingTop2: if (paddings != nullptr) paddings[Edge2::TopEdge2] = lengthValue(decl); break;

			case PaddingBottom2: if (paddings != nullptr) paddings[Edge2::BottomEdge2] = lengthValue(decl); break;

			case Padding2:  if (paddings != nullptr) lengthValues(decl, paddings); break;

			case MarginLeft2: if (margins != nullptr) margins[Edge2::LeftEdge2] = lengthValue(decl); break;

			case MarginRight2: if (margins != nullptr) margins[Edge2::RightEdge2] = lengthValue(decl); break;

			case MarginTop2: if (margins != nullptr) margins[Edge2::TopEdge2] = lengthValue(decl); break;

			case MarginBottom2: if (margins != nullptr) margins[Edge2::BottomEdge2] = lengthValue(decl); break;

			case Margin2: if (margins != nullptr) lengthValues(decl, margins); break;

			case QtSpacing2: if (spacing) *spacing = lengthValue(decl); break;

			case UnknownProperty2:
			case BackgroundColor2:
			case Color2:
			case Float2:
			case Font2:
			case FontFamily2:
			case FontSize2:
			case FontStyle2:
			case FontWeight2:
			case QtBlockIndent2:
			case QtListIndent2:
			case QtParagraphType2:
			case QtTableType2:
			case QtUserState2:
			case TextDecoration2:
			case TextIndent2:
			case TextUnderlineStyle2:
			case VerticalAlignment2:
			case Whitespace2:
			case QtSelectionForeground2:
			case QtSelectionBackground2:
			case Border2:
			case BorderLeft2:
			case BorderRight2:
			case BorderTop2:
			case BorderBottom2:
			case PageBreakBefore2:
			case PageBreakAfter2:
			case QtAlternateBackground2:
			case BorderLeftStyle2:
			case BorderRightStyle2:
			case BorderTopStyle2:
			case BorderBottomStyle2:
			case BorderStyles2:
			case BorderLeftColor2:
			case BorderRightColor2:
			case BorderTopColor2:
			case BorderBottomColor2:
			case BorderColor2:
			case BorderLeftWidth2:
			case BorderRightWidth2:
			case BorderTopWidth2:
			case BorderBottomWidth2:
			case BorderWidth2:
			case BorderTopLeftRadius2:
			case BorderTopRightRadius2:
			case BorderBottomLeftRadius2:
			case BorderBottomRightRadius2:
			case BorderRadius2:
			case Background2:
			case BackgroundOrigin2:
			case BackgroundClip2:
			case BackgroundRepeat2:
			case BackgroundPosition2:
			case BackgroundAttachment2:
			case BackgroundImage2:
			case BorderImage2:
			case Width2:
			case Height2:
			case MinimumWidth2:
			case MinimumHeight2:
			case MaximumWidth2:
			case MaximumHeight2:
			case QtImage2:
			case Left2:
			case Right2:
			case Top2:
			case Bottom2:
			case QtOrigin2:
			case QtPosition2:
			case Position2:
			case QtStyleFeatures2:
			case QtBackgroundRole2:
			case ListStyleType2:
			case ListStyle2:
			case QtImageAlignment2:
			case TextAlignment2:
			case Outline2:
			case OutlineOffset2:
			case OutlineWidth2:
			case OutlineColor2:
			case OutlineStyle2:
			case OutlineRadius2:
			case OutlineTopLeftRadius2:
			case OutlineTopRightRadius2:
			case OutlineBottomLeftRadius2:
			case OutlineBottomRightRadius2:
			case FontVariant2:
			case TextTransform2:
			case QtListNumberPrefix2:
			case QtListNumberSuffix2:
			case LineHeight2:
			case NumProperties2:

			default: continue;
		}

		hit = true;
	}

	return hit;
}

int ValueExtractor2::extractStyleFeatures()
{
	int features = StyleFeature2::StyleFeature_None2;

	for (int i = 0; i < declarations.count(); i++)
	{
		const Declaration2& decl = declarations.at(i);

		if (decl.d->propertyId == Property2::QtStyleFeatures2)
			features = decl.styleFeaturesValue();
	}

	return features;
}

QSize ValueExtractor2::sizeValue(const Declaration2& decl)
{
	if (decl.d->parsed.isValid())
	{
		QList<QVariant> v = decl.d->parsed.toList();
		return QSize(lengthValueFromData(qvariant_cast<LengthData2>(v.at(0)), f),
					 lengthValueFromData(qvariant_cast<LengthData2>(v.at(1)), f));
	}

	LengthData2 x[2] = { {0, LengthData2::NONE }, {0, LengthData2::NONE} };

	if (decl.d->values.count() > 0)
		x[0] = lengthValue(decl.d->values.at(0));

	if (decl.d->values.count() > 1)
		x[1] = lengthValue(decl.d->values.at(1));
	else
		x[1] = x[0];

	QList<QVariant> v;
	v << QVariant::fromValue<LengthData2>(x[0]) << QVariant::fromValue<LengthData2>(x[1]);
	decl.d->parsed = v;
	return QSize(lengthValueFromData(x[0], f), lengthValueFromData(x[1], f));
}

void ValueExtractor2::sizeValues(const Declaration2& decl, QSize* radii)
{
	if (radii != nullptr)
	{
		radii[0] = sizeValue(decl);

		for (int i = 1; i < 4; i++)
			radii[i] = radii[0];
	}
}

bool ValueExtractor2::extractBorder(int* borders, QBrush* colors, BorderStyle2* styles,
								   QSize* radii)
{
	extractFont();
	bool hit = false;

	for (int i = 0; i < declarations.count(); i++)
	{
		const Declaration2& decl = declarations.at(i);

		switch (decl.d->propertyId)
		{
			case BorderLeftWidth2: if (borders != nullptr) borders[Edge2::LeftEdge2] = lengthValue(decl); break;

			case BorderRightWidth2: if (borders != nullptr) borders[Edge2::RightEdge2] = lengthValue(decl); break;

			case BorderTopWidth2: if (borders != nullptr) borders[Edge2::TopEdge2] = lengthValue(decl); break;

			case BorderBottomWidth2: if (borders != nullptr) borders[Edge2::BottomEdge2] = lengthValue(decl); break;

			case BorderWidth2: if (borders != nullptr) lengthValues(decl, borders); break;

			case BorderLeftColor2: if (colors != nullptr) colors[Edge2::LeftEdge2] = decl.brushValue(pal); break;

			case BorderRightColor2: if (colors != nullptr) colors[Edge2::RightEdge2] = decl.brushValue(pal); break;

			case BorderTopColor2: if (colors != nullptr) colors[Edge2::TopEdge2] = decl.brushValue(pal); break;

			case BorderBottomColor2: if (colors != nullptr) colors[Edge2::BottomEdge2] = decl.brushValue(pal); break;

			case BorderColor2: if (colors != nullptr) decl.brushValues(colors, pal); break;

			case BorderTopStyle2: if (styles != nullptr) styles[Edge2::TopEdge2] = decl.styleValue(); break;

			case BorderBottomStyle2: if (styles != nullptr) styles[Edge2::BottomEdge2] = decl.styleValue(); break;

			case BorderLeftStyle2: if (styles != nullptr) styles[Edge2::LeftEdge2] = decl.styleValue(); break;

			case BorderRightStyle2: if (styles != nullptr) styles[Edge2::RightEdge2] = decl.styleValue(); break;

			case BorderStyles2:  if (styles != nullptr) decl.styleValues(styles); break;
#ifndef QT_OS_ANDROID_GCC_48_WORKAROUND

			case BorderTopLeftRadius2: if (radii != nullptr) radii[0] = sizeValue(decl); break;
#else

			case BorderTopLeftRadius2: new (radii)QSize(sizeValue(decl)); break;
#endif

			case BorderTopRightRadius2: if (radii != nullptr) radii[1] = sizeValue(decl); break;

			case BorderBottomLeftRadius2: if (radii != nullptr) radii[2] = sizeValue(decl); break;

			case BorderBottomRightRadius2: if (radii != nullptr) radii[3] = sizeValue(decl); break;

			case BorderRadius2: if (radii != nullptr) sizeValues(decl, radii); break;

			case BorderLeft2:
				if (borders != nullptr && styles != nullptr && colors != nullptr)
					borderValue(decl, &borders[Edge2::LeftEdge2], &styles[Edge2::LeftEdge2], &colors[Edge2::LeftEdge2]);

				break;

			case BorderTop2:
				if (borders != nullptr && styles != nullptr && colors != nullptr)
					borderValue(decl, &borders[Edge2::TopEdge2], &styles[Edge2::TopEdge2], &colors[Edge2::TopEdge2]);

				break;

			case BorderRight2:
				if (borders != nullptr && styles != nullptr && colors != nullptr)
					borderValue(decl, &borders[Edge2::RightEdge2], &styles[Edge2::RightEdge2], &colors[Edge2::RightEdge2]);

				break;

			case BorderBottom2:
				if (borders != nullptr && styles != nullptr && colors != nullptr)
					borderValue(decl, &borders[Edge2::BottomEdge2], &styles[Edge2::BottomEdge2], &colors[Edge2::BottomEdge2]);

				break;

			case Border2:
				if (borders != nullptr && styles != nullptr && colors != nullptr)
				{
					borderValue(decl, &borders[Edge2::LeftEdge2], &styles[Edge2::LeftEdge2], &colors[Edge2::LeftEdge2]);
					borders[Edge2::TopEdge2] = borders[Edge2::RightEdge2] = borders[Edge2::BottomEdge2] = borders[Edge2::LeftEdge2];
					styles[Edge2::TopEdge2] = styles[Edge2::RightEdge2] = styles[Edge2::BottomEdge2] = styles[Edge2::LeftEdge2];
					colors[Edge2::TopEdge2] = colors[Edge2::RightEdge2] = colors[Edge2::BottomEdge2] = colors[Edge2::LeftEdge2];
				}

				break;

			case UnknownProperty2:
			case BackgroundColor2:
			case Color2:
			case Float2:
			case Font2:
			case FontFamily2:
			case FontSize2:
			case FontStyle2:
			case FontWeight2:
			case Margin2:
			case MarginBottom2:
			case MarginLeft2:
			case MarginRight2:
			case MarginTop2:
			case QtBlockIndent2:
			case QtListIndent2:
			case QtParagraphType2:
			case QtTableType2:
			case QtUserState2:
			case TextDecoration2:
			case TextIndent2:
			case TextUnderlineStyle2:
			case VerticalAlignment2:
			case Whitespace2:
			case QtSelectionForeground2:
			case QtSelectionBackground2:
			case Padding2:
			case PaddingLeft2:
			case PaddingRight2:
			case PaddingTop2:
			case PaddingBottom2:
			case PageBreakBefore2:
			case PageBreakAfter2:
			case QtAlternateBackground2:
			case Background2:
			case BackgroundOrigin2:
			case BackgroundClip2:
			case BackgroundRepeat2:
			case BackgroundPosition2:
			case BackgroundAttachment2:
			case BackgroundImage2:
			case BorderImage2:
			case QtSpacing2:
			case Width2:
			case Height2:
			case MinimumWidth2:
			case MinimumHeight2:
			case MaximumWidth2:
			case MaximumHeight2:
			case QtImage2:
			case Left2:
			case Right2:
			case Top2:
			case Bottom2:
			case QtOrigin2:
			case QtPosition2:
			case Position2:
			case QtStyleFeatures2:
			case QtBackgroundRole2:
			case ListStyleType2:
			case ListStyle2:
			case QtImageAlignment2:
			case TextAlignment2:
			case Outline2:
			case OutlineOffset2:
			case OutlineWidth2:
			case OutlineColor2:
			case OutlineStyle2:
			case OutlineRadius2:
			case OutlineTopLeftRadius2:
			case OutlineTopRightRadius2:
			case OutlineBottomLeftRadius2:
			case OutlineBottomRightRadius2:
			case FontVariant2:
			case TextTransform2:
			case QtListNumberPrefix2:
			case QtListNumberSuffix2:
			case LineHeight2:
			case NumProperties2:

			default: continue;
		}

		hit = true;
	}

	return hit;
}

bool ValueExtractor2::extractOutline(int* borders, QBrush* colors, BorderStyle2* styles,
									QSize* radii, int* offsets)
{
	extractFont();
	bool hit = false;

	for (int i = 0; i < declarations.count(); i++)
	{
		const Declaration2& decl = declarations.at(i);

		switch (decl.d->propertyId)
		{
			case OutlineWidth2: if (borders != nullptr) lengthValues(decl, borders); break;

			case OutlineColor2: if (colors != nullptr) decl.brushValues(colors, pal); break;

			case OutlineStyle2:  if (styles != nullptr) decl.styleValues(styles); break;

			case OutlineTopLeftRadius2: if (radii != nullptr) radii[0] = sizeValue(decl); break;

			case OutlineTopRightRadius2: if (radii != nullptr) radii[1] = sizeValue(decl); break;

			case OutlineBottomLeftRadius2: if (radii != nullptr) radii[2] = sizeValue(decl); break;

			case OutlineBottomRightRadius2: if (radii != nullptr) radii[3] = sizeValue(decl); break;

			case OutlineRadius2: if (radii != nullptr) sizeValues(decl, radii); break;

			case OutlineOffset2: if (offsets != nullptr) lengthValues(decl, offsets); break;

			case Outline2:
				if (borders != nullptr && styles != nullptr && colors != nullptr)
				{
					borderValue(decl, &borders[Edge2::LeftEdge2], &styles[Edge2::LeftEdge2], &colors[Edge2::LeftEdge2]);
					borders[Edge2::TopEdge2] = borders[Edge2::RightEdge2] = borders[Edge2::BottomEdge2] = borders[Edge2::LeftEdge2];
					styles[Edge2::TopEdge2] = styles[Edge2::RightEdge2] = styles[Edge2::BottomEdge2] = styles[Edge2::LeftEdge2];
					colors[Edge2::TopEdge2] = colors[Edge2::RightEdge2] = colors[Edge2::BottomEdge2] = colors[Edge2::LeftEdge2];
				}

				break;

			case UnknownProperty2:
			case BackgroundColor2:
			case Color2:
			case Float2:
			case Font2:
			case FontFamily2:
			case FontSize2:
			case FontStyle2:
			case FontWeight2:
			case Margin2:
			case MarginBottom2:
			case MarginLeft2:
			case MarginRight2:
			case MarginTop2:
			case QtBlockIndent2:
			case QtListIndent2:
			case QtParagraphType2:
			case QtTableType2:
			case QtUserState2:
			case TextDecoration2:
			case TextIndent2:
			case TextUnderlineStyle2:
			case VerticalAlignment2:
			case Whitespace2:
			case QtSelectionForeground2:
			case QtSelectionBackground2:
			case Border2:
			case BorderLeft2:
			case BorderRight2:
			case BorderTop2:
			case BorderBottom2:
			case Padding2:
			case PaddingLeft2:
			case PaddingRight2:
			case PaddingTop2:
			case PaddingBottom2:
			case PageBreakBefore2:
			case PageBreakAfter2:
			case QtAlternateBackground2:
			case BorderLeftStyle2:
			case BorderRightStyle2:
			case BorderTopStyle2:
			case BorderBottomStyle2:
			case BorderStyles2:
			case BorderLeftColor2:
			case BorderRightColor2:
			case BorderTopColor2:
			case BorderBottomColor2:
			case BorderColor2:
			case BorderLeftWidth2:
			case BorderRightWidth2:
			case BorderTopWidth2:
			case BorderBottomWidth2:
			case BorderWidth2:
			case BorderTopLeftRadius2:
			case BorderTopRightRadius2:
			case BorderBottomLeftRadius2:
			case BorderBottomRightRadius2:
			case BorderRadius2:
			case Background2:
			case BackgroundOrigin2:
			case BackgroundClip2:
			case BackgroundRepeat2:
			case BackgroundPosition2:
			case BackgroundAttachment2:
			case BackgroundImage2:
			case BorderImage2:
			case QtSpacing2:
			case Width2:
			case Height2:
			case MinimumWidth2:
			case MinimumHeight2:
			case MaximumWidth2:
			case MaximumHeight2:
			case QtImage2:
			case Left2:
			case Right2:
			case Top2:
			case Bottom2:
			case QtOrigin2:
			case QtPosition2:
			case Position2:
			case QtStyleFeatures2:
			case QtBackgroundRole2:
			case ListStyleType2:
			case ListStyle2:
			case QtImageAlignment2:
			case TextAlignment2:
			case FontVariant2:
			case TextTransform2:
			case QtListNumberPrefix2:
			case QtListNumberSuffix2:
			case LineHeight2:
			case NumProperties2:

			default: continue;
		}

		hit = true;
	}

	return hit;
}

static Qt::Alignment parseAlignment(const QCss2::Value2* vals, int count)
{
	Qt::Alignment a[2] = { Qt::AlignmentFlag::AlignLeft, Qt::AlignmentFlag::AlignLeft };

	if (vals != nullptr)
	{
		for (int i = 0; i < qMin(2, count); i++)
		{
			if (vals[i].type != QCss2::Value2::Type2::KnownIdentifier2)
				break;

			switch (vals[i].variant.toInt())
			{
				case QCss2::KnownValue2::Value_Left2: a[i] = Qt::AlignmentFlag::AlignLeft; break;

				case QCss2::KnownValue2::Value_Right2: a[i] = Qt::AlignmentFlag::AlignRight; break;

				case QCss2::KnownValue2::Value_Top2: a[i] = Qt::AlignmentFlag::AlignTop; break;

				case QCss2::KnownValue2::Value_Bottom2: a[i] = Qt::AlignmentFlag::AlignBottom; break;

				case QCss2::KnownValue2::Value_Center2: a[i] = Qt::AlignmentFlag::AlignCenter; break;

				default: break;
			}
		}
	}

	if (a[0] == Qt::AlignmentFlag::AlignCenter && a[1] != 0 && a[1] != Qt::AlignmentFlag::AlignCenter)
		a[0] = (a[1] == Qt::AlignmentFlag::AlignLeft || a[1] == Qt::AlignmentFlag::AlignRight) ? Qt::AlignmentFlag::AlignVCenter : Qt::AlignmentFlag::AlignHCenter;

	if ((a[1] == 0 || a[1] == Qt::AlignmentFlag::AlignCenter) && a[0] != Qt::AlignmentFlag::AlignCenter)
		a[1] = (a[0] == Qt::AlignmentFlag::AlignLeft || a[0] == Qt::AlignmentFlag::AlignRight) ? Qt::AlignmentFlag::AlignVCenter : Qt::AlignmentFlag::AlignHCenter;

	return a[0] | a[1];
}

static ColorData2 parseColorValue(QCss2::Value2 v)
{
	if (v.type == Value2::Identifier2 || v.type == Value2::String2)
	{
		v.variant.convert(QMetaType(QMetaType::QColor));
		v.type = Value2::Color2;
	}

	if (v.type == Value2::Color2)
		return qvariant_cast<QColor>(v.variant);

	if (v.type == Value2::KnownIdentifier2 && v.variant.toInt() == KnownValue2::Value_Transparent2)
		return QColor(Qt::GlobalColor::transparent);

	if (v.type != Value2::Function2)
		return ColorData2();

	QStringList lst = v.variant.toStringList();

	if (lst.count() != 2)
		return ColorData2();

	if ((lst.at(0).compare(QLatin1String("palette"), Qt::CaseSensitivity::CaseInsensitive)) == 0)
	{
		const auto role = findKnownValue(lst.at(1).trimmed(), values, NumKnownValues2);

		if (role >= Value_FirstColorRole2 && role <= Value_LastColorRole2)
			return (QPalette::ColorRole)(role - Value_FirstColorRole2);

		return ColorData2();
	}

	const auto rgb = lst.at(0).startsWith(QLatin1String("rgb"));
	const auto rgba = lst.at(0).startsWith(QLatin1String("rgba"));
	Parser2 p(lst.at(1));

	if (!p.testExpr())
		return ColorData2();

	QVector<QCss2::Value2> colorDigits;

	if (!p.parseExpr(&colorDigits))
		return ColorData2();

	for (int i = 0; i < qMin(colorDigits.count(), 7); i += 2)
	{
		if (colorDigits.at(i).type == Value2::Percentage2)
		{
			colorDigits[i].variant = colorDigits.at(i).variant.toReal() * (255. / 100.);
			colorDigits[i].type = Value2::Number2;
		}
		else if (colorDigits.at(i).type != Value2::Number2)
		{
			return ColorData2();
		}
	}

	const auto v1 = colorDigits.at(0).variant.toInt();
	const auto v2 = colorDigits.at(2).variant.toInt();
	const auto v3 = colorDigits.at(4).variant.toInt();
	auto alpha = 255;

	if (colorDigits.count() >= 7)
	{
		int alphaValue = colorDigits.at(6).variant.toInt();

		if (rgba && alphaValue <= 1)
			alpha = colorDigits.at(6).variant.toReal() * 255.;
		else
			alpha = alphaValue;
	}

	return rgb ? QColor::fromRgb(v1, v2, v3, alpha)
		   : QColor::fromHsv(v1, v2, v3, alpha);
}

static QColor colorFromData(const ColorData2& c, const QPalette& pal)
{
	if (c.type == ColorData2::Color)
	{
		return c.color;
	}
	else if (c.type == ColorData2::Role)
	{
		return pal.color(c.role);
	}

	return QColor();
}

static BrushData2 parseBrushValue(const QCss2::Value2& v, const QPalette& pal)
{
	const auto c = parseColorValue(v);

	if (c.type == ColorData2::Color)
	{
		return QBrush(c.color);
	}
	else if (c.type == ColorData2::Role)
	{
		return c.role;
	}

	if (v.type != Value2::Function2)
		return BrushData2();

	QStringList lst = v.variant.toStringList();

	if (lst.count() != 2)
		return BrushData2();

	QStringList gradFuncs;
	gradFuncs << QLatin1String("qlineargradient") << QLatin1String("qradialgradient") << QLatin1String("qconicalgradient") << QLatin1String("qgradient");
	int gradType = -1;

	if ((gradType = gradFuncs.indexOf(lst.at(0).toLower())) == -1)
		return BrushData2();

	QHash<QString, qreal> vars;
	QVector<QGradientStop> stops;
	int spread = -1;
	QStringList spreads;
	spreads << QLatin1String("pad") << QLatin1String("reflect") << QLatin1String("repeat");
	bool dependsOnThePalette = false;
	Parser2 parser(lst.at(1));

	while (parser.hasNext())
	{
		parser.skipSpace();

		if (!parser.test(TokenType2::IDENT))
			return BrushData2();

		QString attr = parser.lexem();
		parser.skipSpace();

		if (!parser.test(TokenType2::COLON))
			return BrushData2();

		parser.skipSpace();

		if (attr.compare(QLatin1String("stop"), Qt::CaseSensitivity::CaseInsensitive) == 0)
		{
			QCss2::Value2 stop, color;
			parser.next();

			if (!parser.parseTerm(&stop)) return BrushData2();

			parser.skipSpace();
			parser.next();

			if (!parser.parseTerm(&color)) return BrushData2();

			const auto cd = parseColorValue(color);

			if (cd.type == ColorData2::Role)
				dependsOnThePalette = true;

			stops.append(QGradientStop(stop.variant.toReal(), colorFromData(cd, pal)));
		}
		else
		{
			parser.next();
			QCss2::Value2 value;
			std::ignore = parser.parseTerm(&value);

			if (attr.compare(QLatin1String("spread"), Qt::CaseSensitivity::CaseInsensitive) == 0)
			{
				spread = spreads.indexOf(value.variant.toString());
			}
			else
			{
				vars[attr] = value.variant.toReal();
			}
		}

		parser.skipSpace();
		std::ignore = parser.test(TokenType2::COMMA);
	}

	if (gradType == 0)
	{
		QLinearGradient lg(vars.value(QLatin1String("x1")), vars.value(QLatin1String("y1")),
						   vars.value(QLatin1String("x2")), vars.value(QLatin1String("y2")));
		lg.setCoordinateMode(QGradient::ObjectBoundingMode);
		lg.setStops(stops);

		if (spread != -1)
			lg.setSpread(QGradient::Spread(spread));

		BrushData2 bd = QBrush(lg);

		if (dependsOnThePalette)
			bd.type = BrushData2::DependsOnThePalette;

		return bd;
	}

	if (gradType == 1)
	{
		QRadialGradient rg(vars.value(QLatin1String("cx")), vars.value(QLatin1String("cy")),
						   vars.value(QLatin1String("radius")), vars.value(QLatin1String("fx")),
						   vars.value(QLatin1String("fy")));
		rg.setCoordinateMode(QGradient::ObjectBoundingMode);
		rg.setStops(stops);

		if (spread != -1)
			rg.setSpread(QGradient::Spread(spread));

		BrushData2 bd = QBrush(rg);

		if (dependsOnThePalette)
			bd.type = BrushData2::DependsOnThePalette;

		return bd;
	}

	if (gradType == 2)
	{
		QConicalGradient cg(vars.value(QLatin1String("cx")), vars.value(QLatin1String("cy")),
							vars.value(QLatin1String("angle")));
		cg.setCoordinateMode(QGradient::ObjectBoundingMode);
		cg.setStops(stops);

		if (spread != -1)
			cg.setSpread(QGradient::Spread(spread));

		BrushData2 bd = QBrush(cg);

		if (dependsOnThePalette)
			bd.type = BrushData2::DependsOnThePalette;

		return bd;
	}

	return BrushData2();
}

static QBrush brushFromData(const BrushData2& c, const QPalette& pal)
{
	if (c.type == BrushData2::Role)
	{
		return pal.color(c.role);
	}
	else
	{
		return c.brush;
	}
}

static BorderStyle2 parseStyleValue(QCss2::Value2 v)
{
	if (v.type == Value2::KnownIdentifier2)
	{
		switch (v.variant.toInt())
		{
			case Value_None2:
				return BorderStyle2::BorderStyle_None2;

			case Value_Dotted2:
				return BorderStyle2::BorderStyle_Dotted2;

			case Value_Dashed2:
				return BorderStyle2::BorderStyle_Dashed2;

			case Value_Solid2:
				return BorderStyle2::BorderStyle_Solid2;

			case Value_Double2:
				return BorderStyle2::BorderStyle_Double2;

			case Value_DotDash2:
				return BorderStyle2::BorderStyle_DotDash2;

			case Value_DotDotDash2:
				return BorderStyle2::BorderStyle_DotDotDash2;

			case Value_Groove2:
				return BorderStyle2::BorderStyle_Groove2;

			case Value_Ridge2:
				return BorderStyle2::BorderStyle_Ridge2;

			case Value_Inset2:
				return BorderStyle2::BorderStyle_Inset2;

			case Value_Outset2:
				return BorderStyle2::BorderStyle_Outset2;

			case Value_Native2:
				return BorderStyle2::BorderStyle_Native2;

			default:
				break;
		}
	}

	return BorderStyle2::BorderStyle_Unknown2;
}

void ValueExtractor2::borderValue(const Declaration2& decl, int* width, QCss2::BorderStyle2* style, QBrush* color)
{
	if (width != nullptr && style != nullptr && color != nullptr)
	{
		if (decl.d->parsed.isValid())
		{
			BorderData2 data = qvariant_cast<BorderData2>(decl.d->parsed);
			*width = lengthValueFromData(data.width, f);
			*style = data.style;
			*color = data.color.type != BrushData2::Invalid ? brushFromData(data.color, pal) : QBrush(QColor());
			return;
		}

		*width = 0;
		*style = BorderStyle2::BorderStyle_None2;
		*color = QColor();

		if (decl.d->values.isEmpty())
			return;

		BorderData2 data;
		data.width.number = 0;
		data.width.unit = LengthData2::NONE;
		data.style = BorderStyle2::BorderStyle_None2;
		int i = 0;

		if (decl.d->values.at(i).type == Value2::Length2 || decl.d->values.at(i).type == Value2::Number2)
		{
			data.width = lengthValue(decl.d->values.at(i));
			*width = lengthValueFromData(data.width, f);

			if (++i >= decl.d->values.count())
			{
				decl.d->parsed = QVariant::fromValue<BorderData2>(data);
				return;
			}
		}

		data.style = parseStyleValue(decl.d->values.at(i));

		if (data.style != BorderStyle2::BorderStyle_Unknown2)
		{
			*style = data.style;

			if (++i >= decl.d->values.count())
			{
				decl.d->parsed = QVariant::fromValue<BorderData2>(data);
				return;
			}
		}
		else
		{
			data.style = BorderStyle2::BorderStyle_None2;
		}

		data.color = parseBrushValue(decl.d->values.at(i), pal);
		*color = brushFromData(data.color, pal);

		if (data.color.type != BrushData2::DependsOnThePalette)
			decl.d->parsed = QVariant::fromValue<BorderData2>(data);
	}
}

static void parseShorthandBackgroundProperty(const QVector<QCss2::Value2>& vals, BrushData2* brush, QString* image, Repeat2* repeat, Qt::Alignment* alignment, const QPalette& pal)
{
	if (brush != nullptr && image != nullptr && repeat != nullptr && alignment != nullptr)
	{
		*brush = BrushData2();
		*image = QString();
		*repeat = Repeat2::Repeat_XY2;
		*alignment = Qt::AlignmentFlag::AlignTop | Qt::AlignmentFlag::AlignLeft;

		for (int i = 0; i < vals.count(); ++i)
		{
			const QCss2::Value2& v = vals.at(i);

			if (v.type == Value2::Uri2)
			{
				*image = v.variant.toString();
				continue;
			}
			else if (v.type == Value2::KnownIdentifier2 && v.variant.toInt() == KnownValue2::Value_None2)
			{
				*image = QString();
				continue;
			}
			else if (v.type == Value2::KnownIdentifier2 && v.variant.toInt() == KnownValue2::Value_Transparent2)
			{
				*brush = QBrush(Qt::GlobalColor::transparent);
			}

			Repeat2 repeatAttempt = static_cast<Repeat2>(findKnownValue(v.variant.toString(),
								   repeats, Repeat2::NumKnownRepeats2));

			if (repeatAttempt != Repeat2::Repeat_Unknown2)
			{
				*repeat = repeatAttempt;
				continue;
			}

			if (v.type == Value2::KnownIdentifier2)
			{
				const int start = i;
				int count = 1;

				if (i < vals.count() - 1
						&& vals.at(i + 1).type == Value2::KnownIdentifier2)
				{
					++i;
					++count;
				}

				Qt::Alignment a = parseAlignment(vals.constData() + start, count);

				if (int(a) != 0)
				{
					*alignment = a;
					continue;
				}

				i -= count - 1;
			}

			*brush = parseBrushValue(v, pal);
		}
	}
}

bool ValueExtractor2::extractBackground(QBrush* brush, QString* image, Repeat2* repeat,
									   Qt::Alignment* alignment, Origin2* origin, Attachment2* attachment,
									   Origin2* clip)
{
	bool hit = false;

	if (brush != nullptr && image != nullptr && repeat != nullptr && alignment != nullptr && origin != nullptr && attachment != nullptr && clip != nullptr)
	{
		for (int i = 0; i < declarations.count(); ++i)
		{
			const Declaration2& decl = declarations.at(i);

			if (decl.d->values.isEmpty())
				continue;

			const QCss2::Value2& val = decl.d->values.at(0);

			switch (decl.d->propertyId)
			{
				case BackgroundColor2:
					*brush = decl.brushValue();
					break;

				case BackgroundImage2:
					if (val.type == Value2::Uri2)
						*image = val.variant.toString();

					break;

				case BackgroundRepeat2:
					if (decl.d->parsed.isValid())
					{
						*repeat = static_cast<Repeat2>(decl.d->parsed.toInt());
					}
					else
					{
						*repeat = static_cast<Repeat2>(findKnownValue(val.variant.toString(),
													  repeats, Repeat2::NumKnownRepeats2));
						decl.d->parsed = *repeat;
					}

					break;

				case BackgroundPosition2:
					*alignment = decl.alignmentValue();
					break;

				case BackgroundOrigin2:
					*origin = decl.originValue();
					break;

				case BackgroundClip2:
					*clip = decl.originValue();
					break;

				case Background2:
					if (decl.d->parsed.isValid())
					{
						BackgroundData2 data = qvariant_cast<BackgroundData2>(decl.d->parsed);
						*brush = brushFromData(data.brush, pal);
						*image = data.image;
						*repeat = data.repeat;
						*alignment = data.alignment;
					}
					else
					{
						BrushData2 brushData;
						parseShorthandBackgroundProperty(decl.d->values, &brushData, image, repeat, alignment, pal);
						*brush = brushFromData(brushData, pal);

						if (brushData.type != BrushData2::DependsOnThePalette)
						{
							BackgroundData2 data = { brushData, *image, *repeat, *alignment };
							decl.d->parsed = QVariant::fromValue<BackgroundData2>(data);
						}
					}

					break;

				case BackgroundAttachment2:
					*attachment = decl.attachmentValue();
					break;

				case UnknownProperty2:
				case Color2:
				case Float2:
				case Font2:
				case FontFamily2:
				case FontSize2:
				case FontStyle2:
				case FontWeight2:
				case Margin2:
				case MarginBottom2:
				case MarginLeft2:
				case MarginRight2:
				case MarginTop2:
				case QtBlockIndent2:
				case QtListIndent2:
				case QtParagraphType2:
				case QtTableType2:
				case QtUserState2:
				case TextDecoration2:
				case TextIndent2:
				case TextUnderlineStyle2:
				case VerticalAlignment2:
				case Whitespace2:
				case QtSelectionForeground2:
				case QtSelectionBackground2:
				case Border2:
				case BorderLeft2:
				case BorderRight2:
				case BorderTop2:
				case BorderBottom2:
				case Padding2:
				case PaddingLeft2:
				case PaddingRight2:
				case PaddingTop2:
				case PaddingBottom2:
				case PageBreakBefore2:
				case PageBreakAfter2:
				case QtAlternateBackground2:
				case BorderLeftStyle2:
				case BorderRightStyle2:
				case BorderTopStyle2:
				case BorderBottomStyle2:
				case BorderStyles2:
				case BorderLeftColor2:
				case BorderRightColor2:
				case BorderTopColor2:
				case BorderBottomColor2:
				case BorderColor2:
				case BorderLeftWidth2:
				case BorderRightWidth2:
				case BorderTopWidth2:
				case BorderBottomWidth2:
				case BorderWidth2:
				case BorderTopLeftRadius2:
				case BorderTopRightRadius2:
				case BorderBottomLeftRadius2:
				case BorderBottomRightRadius2:
				case BorderRadius2:
				case BorderImage2:
				case QtSpacing2:
				case Width2:
				case Height2:
				case MinimumWidth2:
				case MinimumHeight2:
				case MaximumWidth2:
				case MaximumHeight2:
				case QtImage2:
				case Left2:
				case Right2:
				case Top2:
				case Bottom2:
				case QtOrigin2:
				case QtPosition2:
				case Position2:
				case QtStyleFeatures2:
				case QtBackgroundRole2:
				case ListStyleType2:
				case ListStyle2:
				case QtImageAlignment2:
				case TextAlignment2:
				case Outline2:
				case OutlineOffset2:
				case OutlineWidth2:
				case OutlineColor2:
				case OutlineStyle2:
				case OutlineRadius2:
				case OutlineTopLeftRadius2:
				case OutlineTopRightRadius2:
				case OutlineBottomLeftRadius2:
				case OutlineBottomRightRadius2:
				case FontVariant2:
				case TextTransform2:
				case QtListNumberPrefix2:
				case QtListNumberSuffix2:
				case LineHeight2:
				case NumProperties2:

				default: continue;
			}

			hit = true;
		}
	}

	return hit;
}

static bool setFontSizeFromValue(QCss2::Value2 value, QFont* font, int* fontSizeAdjustment)
{
	if (font != nullptr && fontSizeAdjustment != nullptr)
	{
		if (value.type == Value2::KnownIdentifier2)
		{
			bool valid = true;

			switch (value.variant.toInt())
			{
				case KnownValue2::Value_Small2: *fontSizeAdjustment = -1; break;

				case KnownValue2::Value_Medium2: *fontSizeAdjustment = 0; break;

				case KnownValue2::Value_Large2: *fontSizeAdjustment = 1; break;

				case KnownValue2::Value_XLarge2: *fontSizeAdjustment = 2; break;

				case KnownValue2::Value_XXLarge2: *fontSizeAdjustment = 3; break;

				default: valid = false; break;
			}

			return valid;
		}

		if (value.type != Value2::Length2)
			return false;

		bool valid = false;
		QString s = value.variant.toString();

		if (s.endsWith(QLatin1String("pt"), Qt::CaseSensitivity::CaseInsensitive))
		{
			s.chop(2);
			value.variant = s;

			if (value.variant.convert(QMetaType((QMetaType::Type)qMetaTypeId<qreal>())))
			{
				font->setPointSizeF(value.variant.toReal());
				valid = true;
			}
		}
		else if (s.endsWith(QLatin1String("px"), Qt::CaseSensitivity::CaseInsensitive))
		{
			s.chop(2);
			value.variant = s;

			if (value.variant.convert(QMetaType(QMetaType::Int)))
			{
				font->setPixelSize(value.variant.toInt());
				valid = true;
			}
		}

		return valid;
	}

	return false;
}

static bool setFontStyleFromValue(const QCss2::Value2& value, QFont* font)
{
	if (value.type != Value2::KnownIdentifier2)
		return false ;

	if (font != nullptr)
	{
		switch (value.variant.toInt())
		{
			case KnownValue2::Value_Normal2: font->setStyle(QFont::StyleNormal); return true;

			case KnownValue2::Value_Italic2: font->setStyle(QFont::StyleItalic); return true;

			case KnownValue2::Value_Oblique2: font->setStyle(QFont::StyleOblique); return true;

			default: break;
		}
	}

	return false;
}

static QFont::Weight IntToFontWeight(int i) noexcept
{
	if (i <= QFont::Weight::Thin)
		return QFont::Weight::Thin;
	else if (i <= QFont::Weight::ExtraLight)
		return QFont::Weight::ExtraLight;
	else if (i <= QFont::Weight::Light)
		return QFont::Weight::Light;
	else if (i <= QFont::Weight::Normal)
		return QFont::Weight::Normal;
	else if (i <= QFont::Weight::Medium)
		return	QFont::Weight::Medium;
	else if (i <= QFont::Weight::DemiBold)
		return QFont::Weight::DemiBold;
	else if (i <= QFont::Weight::Bold)
		return QFont::Weight::Bold;
	else if (i <= QFont::Weight::ExtraBold)
		return QFont::Weight::ExtraBold;

	return QFont::Weight::Black;
}

static bool setFontWeightFromValue(const QCss2::Value2& value, QFont* font)
{
	if (font != nullptr)
	{
		if (value.type == Value2::KnownIdentifier2)
		{
			switch (value.variant.toInt())
			{
				case KnownValue2::Value_Normal2: font->setWeight(QFont::Normal); return true;

				case KnownValue2::Value_Bold2: font->setWeight(QFont::Bold); return true;

				default: break;
			}

			return false;
		}

		if (value.type != Value2::Number2)
			return false;

		font->setWeight(IntToFontWeight(value.variant.toInt()));
	}

	return true;
}

/** \internal
	parse the font family from the values (starting from index \a start)
	and set it the \a font
	The function returns \c true if a family was extracted.
*/
static bool setFontFamilyFromValues(const QVector<QCss2::Value2>& vals, QFont* font, int start = 0)
{
	QString family;
	bool shouldAddSpace = false;

	for (int i = start; i < vals.count(); ++i)
	{
		const QCss2::Value2& v = vals.at(i);

		if (v.type == Value2::TermOperatorComma2)
		{
			family += QLatin1Char(',');
			shouldAddSpace = false;
			continue;
		}

		const QString str = v.variant.toString();

		if (str.isEmpty())
			break;

		if (shouldAddSpace)
			family += QLatin1Char(' ');

		family += str;
		shouldAddSpace = true;
	}

	if (family.isEmpty())
		return false;

	if (font != nullptr)
		font->setFamily(family);

	return true;
}

static void setTextDecorationFromValues(const QVector<QCss2::Value2>& vals, QFont* font)
{
	if (font != nullptr)
	{
		for (int i = 0; i < vals.count(); ++i)
		{
			if (vals.at(i).type != Value2::KnownIdentifier2)
				continue;

			switch (vals.at(i).variant.toInt())
			{
				case KnownValue2::Value_Underline2: font->setUnderline(true); break;

				case KnownValue2::Value_Overline2: font->setOverline(true); break;

				case KnownValue2::Value_LineThrough2: font->setStrikeOut(true); break;

				case KnownValue2::Value_None2:
					font->setUnderline(false);
					font->setOverline(false);
					font->setStrikeOut(false);
					break;

				default: break;
			}
		}
	}
}

static void parseShorthandFontProperty(const QVector<QCss2::Value2>& vals, QFont* font, int* fontSizeAdjustment)
{
	if (font != nullptr)
	{
		font->setStyle(QFont::StyleNormal);
		font->setWeight(QFont::Normal);
		*fontSizeAdjustment = -255;
		int i = 0;

		while (i < vals.count())
		{
			if (setFontStyleFromValue(vals.at(i), font)
					|| setFontWeightFromValue(vals.at(i), font))
				++i;
			else
				break;
		}

		if (i < vals.count())
		{
			setFontSizeFromValue(vals.at(i), font, fontSizeAdjustment);
			++i;
		}

		if (i < vals.count())
		{
			setFontFamilyFromValues(vals, font, i);
		}
	}
}

static void setFontVariantFromValue(const QCss2::Value2& value, QFont* font)
{
	if (font != nullptr)
	{
		if (value.type == Value2::KnownIdentifier2)
		{
			switch (value.variant.toInt())
			{
				case KnownValue2::Value_Normal2: font->setCapitalization(QFont::MixedCase); break;

				case KnownValue2::Value_SmallCaps2: font->setCapitalization(QFont::SmallCaps); break;

				default: break;
			}
		}
	}
}

static void setTextTransformFromValue(const QCss2::Value2& value, QFont* font)
{
	if (font != nullptr)
	{
		if (value.type == Value2::KnownIdentifier2)
		{
			switch (value.variant.toInt())
			{
				case KnownValue2::Value_None2: font->setCapitalization(QFont::MixedCase); break;

				case KnownValue2::Value_Uppercase2: font->setCapitalization(QFont::AllUppercase); break;

				case KnownValue2::Value_Lowercase2: font->setCapitalization(QFont::AllLowercase); break;

				default: break;
			}
		}
	}
}

bool ValueExtractor2::extractFont(QFont* font, int* fontSizeAdjustment)
{
	if (font != nullptr)
	{
		if (fontExtracted)
		{
			*font = f;
			*fontSizeAdjustment = adjustment;
			return fontExtracted == 1;
		}

		bool hit = false;

		for (int i = 0; i < declarations.count(); ++i)
		{
			const Declaration2& decl = declarations.at(i);

			if (decl.d->values.isEmpty())
				continue;

			const QCss2::Value2& val = decl.d->values.at(0);

			switch (decl.d->propertyId)
			{
				case FontSize2: setFontSizeFromValue(val, font, fontSizeAdjustment); break;

				case FontStyle2: setFontStyleFromValue(val, font); break;

				case FontWeight2: setFontWeightFromValue(val, font); break;

				case FontFamily2: setFontFamilyFromValues(decl.d->values, font); break;

				case TextDecoration2: setTextDecorationFromValues(decl.d->values, font); break;

				case Font2: parseShorthandFontProperty(decl.d->values, font, fontSizeAdjustment); break;

				case FontVariant2: setFontVariantFromValue(val, font); break;

				case TextTransform2: setTextTransformFromValue(val, font); break;

				case UnknownProperty2:
				case BackgroundColor2:
				case Color2:
				case Float2:
				case Margin2:
				case MarginBottom2:
				case MarginLeft2:
				case MarginRight2:
				case MarginTop2:
				case QtBlockIndent2:
				case QtListIndent2:
				case QtParagraphType2:
				case QtTableType2:
				case QtUserState2:
				case TextIndent2:
				case TextUnderlineStyle2:
				case VerticalAlignment2:
				case Whitespace2:
				case QtSelectionForeground2:
				case QtSelectionBackground2:
				case Border2:
				case BorderLeft2:
				case BorderRight2:
				case BorderTop2:
				case BorderBottom2:
				case Padding2:
				case PaddingLeft2:
				case PaddingRight2:
				case PaddingTop2:
				case PaddingBottom2:
				case PageBreakBefore2:
				case PageBreakAfter2:
				case QtAlternateBackground2:
				case BorderLeftStyle2:
				case BorderRightStyle2:
				case BorderTopStyle2:
				case BorderBottomStyle2:
				case BorderStyles2:
				case BorderLeftColor2:
				case BorderRightColor2:
				case BorderTopColor2:
				case BorderBottomColor2:
				case BorderColor2:
				case BorderLeftWidth2:
				case BorderRightWidth2:
				case BorderTopWidth2:
				case BorderBottomWidth2:
				case BorderWidth2:
				case BorderTopLeftRadius2:
				case BorderTopRightRadius2:
				case BorderBottomLeftRadius2:
				case BorderBottomRightRadius2:
				case BorderRadius2:
				case Background2:
				case BackgroundOrigin2:
				case BackgroundClip2:
				case BackgroundRepeat2:
				case BackgroundPosition2:
				case BackgroundAttachment2:
				case BackgroundImage2:
				case BorderImage2:
				case QtSpacing2:
				case Width2:
				case Height2:
				case MinimumWidth2:
				case MinimumHeight2:
				case MaximumWidth2:
				case MaximumHeight2:
				case QtImage2:
				case Left2:
				case Right2:
				case Top2:
				case Bottom2:
				case QtOrigin2:
				case QtPosition2:
				case Position2:
				case QtStyleFeatures2:
				case QtBackgroundRole2:
				case ListStyleType2:
				case ListStyle2:
				case QtImageAlignment2:
				case TextAlignment2:
				case Outline2:
				case OutlineOffset2:
				case OutlineWidth2:
				case OutlineColor2:
				case OutlineStyle2:
				case OutlineRadius2:
				case OutlineTopLeftRadius2:
				case OutlineTopRightRadius2:
				case OutlineBottomLeftRadius2:
				case OutlineBottomRightRadius2:
				case QtListNumberPrefix2:
				case QtListNumberSuffix2:
				case LineHeight2:
				case NumProperties2:

				default: continue;
			}

			hit = true;
		}

		f = *font;
		adjustment = *fontSizeAdjustment;
		fontExtracted = hit ? 1 : 2;
		return hit;
	}

	return false;
}

bool ValueExtractor2::extractPalette(QBrush* fg, QBrush* sfg, QBrush* sbg, QBrush* abg)
{
	bool hit = false;

	if (fg != nullptr && sfg != nullptr && sbg != nullptr && abg != nullptr)
	{
		for (int i = 0; i < declarations.count(); ++i)
		{
			const Declaration2& decl = declarations.at(i);

			switch (decl.d->propertyId)
			{
				case Color2: *fg = decl.brushValue(pal); break;

				case QtSelectionForeground2: *sfg = decl.brushValue(pal); break;

				case QtSelectionBackground2: *sbg = decl.brushValue(pal); break;

				case QtAlternateBackground2: *abg = decl.brushValue(pal); break;

				case UnknownProperty2:
				case BackgroundColor2:
				case Float2:
				case Font2:
				case FontFamily2:
				case FontSize2:
				case FontStyle2:
				case FontWeight2:
				case Margin2:
				case MarginBottom2:
				case MarginLeft2:
				case MarginRight2:
				case MarginTop2:
				case QtBlockIndent2:
				case QtListIndent2:
				case QtParagraphType2:
				case QtTableType2:
				case QtUserState2:
				case TextDecoration2:
				case TextIndent2:
				case TextUnderlineStyle2:
				case VerticalAlignment2:
				case Whitespace2:
				case Border2:
				case BorderLeft2:
				case BorderRight2:
				case BorderTop2:
				case BorderBottom2:
				case Padding2:
				case PaddingLeft2:
				case PaddingRight2:
				case PaddingTop2:
				case PaddingBottom2:
				case PageBreakBefore2:
				case PageBreakAfter2:
				case BorderLeftStyle2:
				case BorderRightStyle2:
				case BorderTopStyle2:
				case BorderBottomStyle2:
				case BorderStyles2:
				case BorderLeftColor2:
				case BorderRightColor2:
				case BorderTopColor2:
				case BorderBottomColor2:
				case BorderColor2:
				case BorderLeftWidth2:
				case BorderRightWidth2:
				case BorderTopWidth2:
				case BorderBottomWidth2:
				case BorderWidth2:
				case BorderTopLeftRadius2:
				case BorderTopRightRadius2:
				case BorderBottomLeftRadius2:
				case BorderBottomRightRadius2:
				case BorderRadius2:
				case Background2:
				case BackgroundOrigin2:
				case BackgroundClip2:
				case BackgroundRepeat2:
				case BackgroundPosition2:
				case BackgroundAttachment2:
				case BackgroundImage2:
				case BorderImage2:
				case QtSpacing2:
				case Width2:
				case Height2:
				case MinimumWidth2:
				case MinimumHeight2:
				case MaximumWidth2:
				case MaximumHeight2:
				case QtImage2:
				case Left2:
				case Right2:
				case Top2:
				case Bottom2:
				case QtOrigin2:
				case QtPosition2:
				case Position2:
				case QtStyleFeatures2:
				case QtBackgroundRole2:
				case ListStyleType2:
				case ListStyle2:
				case QtImageAlignment2:
				case TextAlignment2:
				case Outline2:
				case OutlineOffset2:
				case OutlineWidth2:
				case OutlineColor2:
				case OutlineStyle2:
				case OutlineRadius2:
				case OutlineTopLeftRadius2:
				case OutlineTopRightRadius2:
				case OutlineBottomLeftRadius2:
				case OutlineBottomRightRadius2:
				case FontVariant2:
				case TextTransform2:
				case QtListNumberPrefix2:
				case QtListNumberSuffix2:
				case LineHeight2:
				case NumProperties2:

				default: continue;
			}

			hit = true;
		}
	}

	return hit;
}

void ValueExtractor2::extractFont()
{
	if (fontExtracted)
		return;

	int dummy = -255;
	extractFont(&f, &dummy);
}

bool ValueExtractor2::extractImage(QIcon* icon, Qt::Alignment* a, QSize* size)
{
	bool hit = false;

	if (icon != nullptr && a != nullptr && size != nullptr)
	{
		for (int i = 0; i < declarations.count(); ++i)
		{
			const Declaration2& decl = declarations.at(i);

			switch (decl.d->propertyId)
			{
				case QtImage2:
					*icon = decl.iconValue();

					if (decl.d->values.count() > 0 && decl.d->values.at(0).type == Value2::Uri2)
					{
						// try to pull just the size from the image...
						QImageReader imageReader(decl.d->values.at(0).variant.toString());

						if ((*size = imageReader.size()).isNull())
						{
							// but we'll have to load the whole image if the
							// format doesn't support just reading the size
							*size = imageReader.read().size();
						}
					}

					break;

				case QtImageAlignment2: *a = decl.alignmentValue();  break;

				case UnknownProperty2:
				case BackgroundColor2:
				case Color2:
				case Float2:
				case Font2:
				case FontFamily2:
				case FontSize2:
				case FontStyle2:
				case FontWeight2:
				case Margin2:
				case MarginBottom2:
				case MarginLeft2:
				case MarginRight2:
				case MarginTop2:
				case QtBlockIndent2:
				case QtListIndent2:
				case QtParagraphType2:
				case QtTableType2:
				case QtUserState2:
				case TextDecoration2:
				case TextIndent2:
				case TextUnderlineStyle2:
				case VerticalAlignment2:
				case Whitespace2:
				case QtSelectionForeground2:
				case QtSelectionBackground2:
				case Border2:
				case BorderLeft2:
				case BorderRight2:
				case BorderTop2:
				case BorderBottom2:
				case Padding2:
				case PaddingLeft2:
				case PaddingRight2:
				case PaddingTop2:
				case PaddingBottom2:
				case PageBreakBefore2:
				case PageBreakAfter2:
				case QtAlternateBackground2:
				case BorderLeftStyle2:
				case BorderRightStyle2:
				case BorderTopStyle2:
				case BorderBottomStyle2:
				case BorderStyles2:
				case BorderLeftColor2:
				case BorderRightColor2:
				case BorderTopColor2:
				case BorderBottomColor2:
				case BorderColor2:
				case BorderLeftWidth2:
				case BorderRightWidth2:
				case BorderTopWidth2:
				case BorderBottomWidth2:
				case BorderWidth2:
				case BorderTopLeftRadius2:
				case BorderTopRightRadius2:
				case BorderBottomLeftRadius2:
				case BorderBottomRightRadius2:
				case BorderRadius2:
				case Background2:
				case BackgroundOrigin2:
				case BackgroundClip2:
				case BackgroundRepeat2:
				case BackgroundPosition2:
				case BackgroundAttachment2:
				case BackgroundImage2:
				case BorderImage2:
				case QtSpacing2:
				case Width2:
				case Height2:
				case MinimumWidth2:
				case MinimumHeight2:
				case MaximumWidth2:
				case MaximumHeight2:
				case Left2:
				case Right2:
				case Top2:
				case Bottom2:
				case QtOrigin2:
				case QtPosition2:
				case Position2:
				case QtStyleFeatures2:
				case QtBackgroundRole2:
				case ListStyleType2:
				case ListStyle2:
				case TextAlignment2:
				case Outline2:
				case OutlineOffset2:
				case OutlineWidth2:
				case OutlineColor2:
				case OutlineStyle2:
				case OutlineRadius2:
				case OutlineTopLeftRadius2:
				case OutlineTopRightRadius2:
				case OutlineBottomLeftRadius2:
				case OutlineBottomRightRadius2:
				case FontVariant2:
				case TextTransform2:
				case QtListNumberPrefix2:
				case QtListNumberSuffix2:
				case LineHeight2:
				case NumProperties2:

				default: continue;
			}

			hit = true;
		}
	}

	return hit;
}

///////////////////////////////////////////////////////////////////////////////
// Declaration
QColor Declaration2::colorValue(const QPalette& pal) const
{
	if (d->values.count() != 1)
		return QColor();

	if (d->parsed.isValid())
	{
		if (d->parsed.typeId() == QMetaType::QColor)
			return qvariant_cast<QColor>(d->parsed);

		if (d->parsed.typeId() == QMetaType::Int)
			return pal.color((QPalette::ColorRole)(d->parsed.toInt()));
	}

	ColorData2 color = parseColorValue(d->values.at(0));

	if (color.type == ColorData2::Role)
	{
		d->parsed = QVariant::fromValue<int>(color.role);
		return pal.color((QPalette::ColorRole)(color.role));
	}
	else
	{
		d->parsed = QVariant::fromValue<QColor>(color.color);
		return color.color;
	}
}

QBrush Declaration2::brushValue(const QPalette& pal) const
{
	if (d->values.count() != 1)
		return QBrush();

	if (d->parsed.isValid())
	{
		if (d->parsed.typeId() == QMetaType::QBrush)
			return qvariant_cast<QBrush>(d->parsed);

		if (d->parsed.typeId() == QMetaType::Int)
			return pal.color((QPalette::ColorRole)(d->parsed.toInt()));
	}

	BrushData2 data = parseBrushValue(d->values.at(0), pal);

	if (data.type == BrushData2::Role)
	{
		d->parsed = QVariant::fromValue<int>(data.role);
		return pal.color((QPalette::ColorRole)(data.role));
	}
	else
	{
		if (data.type != BrushData2::DependsOnThePalette)
			d->parsed = QVariant::fromValue<QBrush>(data.brush);

		return data.brush;
	}
}

void Declaration2::brushValues(QBrush* c, const QPalette& pal) const
{
	int needParse = 0x1f; // bits 0..3 say if we should parse the corresponding value.
	// the bit 4 say we need to update d->parsed
	int i = 0;

	if (c != nullptr)
	{
		if (d->parsed.isValid())
		{
			needParse = 0;
			QList<QVariant> v = d->parsed.toList();

			for (i = 0; i < qMin(v.count(), 4); i++)
			{
				if (v.at(i).typeId() == QMetaType::QBrush)
				{
					c[i] = qvariant_cast<QBrush>(v.at(i));
				}
				else if (v.at(i).typeId() == QMetaType::Int)
				{
					c[i] = pal.color((QPalette::ColorRole)(v.at(i).toInt()));
				}
				else
				{
					needParse |= (1 << i);
				}
			}
		}

		if (needParse != 0)
		{
			QList<QVariant> v;

			for (i = 0; i < qMin(d->values.count(), 4); i++)
			{
				if (!(needParse & (1 << i)))
					continue;

				BrushData2 data = parseBrushValue(d->values.at(i), pal);

				if (data.type == BrushData2::Role)
				{
					v += QVariant::fromValue<int>(data.role);
					c[i] = pal.color((QPalette::ColorRole)(data.role));
				}
				else
				{
					if (data.type != BrushData2::DependsOnThePalette)
					{
						v += QVariant::fromValue<QBrush>(data.brush);
					}
					else
					{
						v += QVariant();
					}

					c[i] = data.brush;
				}
			}

			if (needParse & 0x10)
				d->parsed = v;
		}

		if (i == 0) c[0] = c[1] = c[2] = c[3] = QBrush();
		else if (i == 1) c[3] = c[2] = c[1] = c[0];
		else if (i == 2) c[2] = c[0], c[3] = c[1];
		else if (i == 3) c[3] = c[1];
	}
}

bool Declaration2::realValue(qreal* real, const char* unit) const
{
	if (d->values.count() != 1)
		return false;

	const Value2& v = d->values.at(0);

	if (unit != nullptr && v.type != Value2::Length2)
		return false;

	QString s = v.variant.toString();

	if (unit != nullptr)
	{
		if (!s.endsWith(QLatin1String(unit), Qt::CaseSensitivity::CaseInsensitive))
			return false;

		s.chop(qstrlen(unit));
	}

	bool ok = false;
	qreal val = s.toDouble(&ok);

	if (ok && real != nullptr)
		*real = val;

	return ok;
}

static bool intValueHelper(const QCss2::Value2& v, int* i, const char* unit)
{
	if (unit && v.type != Value2::Length2)
		return false;

	QString s = v.variant.toString();

	if (unit)
	{
		if (!s.endsWith(QLatin1String(unit), Qt::CaseSensitivity::CaseInsensitive))
			return false;

		s.chop(qstrlen(unit));
	}

	bool ok = false;
	int val = s.toInt(&ok);

	if (ok && i != nullptr)
		*i = val;

	return ok;
}

bool Declaration2::intValue(int* i, const char* unit) const
{
	if (d->values.count() != 1)
		return false;

	return intValueHelper(d->values.at(0), i, unit);
}

QSize Declaration2::sizeValue() const
{
	if (d->parsed.isValid())
		return qvariant_cast<QSize>(d->parsed);

	int x[2] = { 0, 0 };

	if (d->values.count() > 0)
		intValueHelper(d->values.at(0), &x[0], "px");

	if (d->values.count() > 1)
		intValueHelper(d->values.at(1), &x[1], "px");
	else
		x[1] = x[0];

	QSize size(x[0], x[1]);
	d->parsed = QVariant::fromValue<QSize>(size);
	return size;
}

QRect Declaration2::rectValue() const
{
	if (d->values.count() != 1)
		return QRect();

	if (d->parsed.isValid())
		return qvariant_cast<QRect>(d->parsed);

	const QCss2::Value2& v = d->values.at(0);

	if (v.type != Value2::Function2)
		return QRect();

	QStringList func = v.variant.toStringList();

	if (func.count() != 2 || func.at(0).compare(QLatin1String("rect")) != 0)
		return QRect();

	QStringList args = func[1].split(QLatin1Char(' '), Qt::SplitBehaviorFlags::SkipEmptyParts);

	if (args.count() != 4)
		return QRect();

	QRect rect(args[0].toInt(), args[1].toInt(), args[2].toInt(), args[3].toInt());
	d->parsed = QVariant::fromValue<QRect>(rect);
	return rect;
}

void Declaration2::colorValues(QColor* c, const QPalette& pal) const
{
	int i;

	if (c != nullptr)
	{
		if (d->parsed.isValid())
		{
			QList<QVariant> v = d->parsed.toList();

			for (i = 0; i < qMin(d->values.count(), 4); i++)
			{
				if (v.at(i).typeId() == QMetaType::QColor)
				{
					c[i] = qvariant_cast<QColor>(v.at(i));
				}
				else
				{
					c[i] = pal.color((QPalette::ColorRole)(v.at(i).toInt()));
				}
			}
		}
		else
		{
			QList<QVariant> v;

			for (i = 0; i < qMin(d->values.count(), 4); i++)
			{
				ColorData2 color = parseColorValue(d->values.at(i));

				if (color.type == ColorData2::Role)
				{
					v += QVariant::fromValue<int>(color.role);
					c[i] = pal.color((QPalette::ColorRole)(color.role));
				}
				else
				{
					v += QVariant::fromValue<QColor>(color.color);
					c[i] = color.color;
				}
			}

			d->parsed = v;
		}

		if (i == 0) c[0] = c[1] = c[2] = c[3] = QColor();
		else if (i == 1) c[3] = c[2] = c[1] = c[0];
		else if (i == 2) c[2] = c[0], c[3] = c[1];
		else if (i == 3) c[3] = c[1];
	}
}

BorderStyle2 Declaration2::styleValue() const
{
	if (d->values.count() != 1)
		return BorderStyle2::BorderStyle_None2;

	return parseStyleValue(d->values.at(0));
}

void Declaration2::styleValues(BorderStyle2* s) const
{
	if (s != nullptr)
	{
		int i;

		for (i = 0; i < qMin(d->values.count(), 4); i++)
			s[i] = parseStyleValue(d->values.at(i));

		if (i == 0) s[0] = s[1] = s[2] = s[3] = BorderStyle2::BorderStyle_None2;
		else if (i == 1) s[3] = s[2] = s[1] = s[0];
		else if (i == 2) s[2] = s[0], s[3] = s[1];
		else if (i == 3) s[3] = s[1];
	}
}

Repeat2 Declaration2::repeatValue() const
{
	if (d->parsed.isValid())
		return static_cast<Repeat2>(d->parsed.toInt());

	if (d->values.count() != 1)
		return Repeat2::Repeat_Unknown2;

	const auto v = findKnownValue(d->values.at(0).variant.toString(),
								  repeats, Repeat2::NumKnownRepeats2);
	d->parsed = v;
	return static_cast<Repeat2>(v);
}

Origin2 Declaration2::originValue() const
{
	if (d->parsed.isValid())
		return static_cast<Origin2>(d->parsed.toInt());

	if (d->values.count() != 1)
		return Origin2::Origin_Unknown2;

	const auto v = findKnownValue(d->values.at(0).variant.toString(),
								  origins, Origin2::NumKnownOrigins2);
	d->parsed = v;
	return static_cast<Origin2>(v);
}

PositionMode2 Declaration2::positionValue() const
{
	if (d->parsed.isValid())
		return static_cast<PositionMode2>(d->parsed.toInt());

	if (d->values.count() != 1)
		return PositionMode2::PositionMode_Unknown2;

	const auto v = findKnownValue(d->values.at(0).variant.toString(),
								  positions, PositionMode2::NumKnownPositionModes2);
	d->parsed = v;
	return static_cast<PositionMode2>(v);
}

Attachment2 Declaration2::attachmentValue() const
{
	if (d->parsed.isValid())
		return static_cast<Attachment2>(d->parsed.toInt());

	if (d->values.count() != 1)
		return Attachment2::Attachment_Unknown2;

	const auto v = findKnownValue(d->values.at(0).variant.toString(),
								  attachments, Attachment2::NumKnownAttachments2);
	d->parsed = v;
	return static_cast<Attachment2>(v);
}

int Declaration2::styleFeaturesValue() const
{
	Q_ASSERT(d->propertyId == QtStyleFeatures2);

	if (d->parsed.isValid())
		return d->parsed.toInt();

	int features = StyleFeature2::StyleFeature_None2;

	for (int i = 0; i < d->values.count(); i++)
	{
		features |= static_cast<int>(findKnownValue(d->values.value(i).variant.toString(),
									 styleFeatures, StyleFeature2::NumKnownStyleFeatures2));
	}

	d->parsed = features;
	return features;
}

QString Declaration2::uriValue() const
{
	if (d->values.isEmpty() || d->values.at(0).type != Value2::Uri2)
		return QString();

	return d->values.at(0).variant.toString();
}

Qt::Alignment Declaration2::alignmentValue() const
{
	if (d->parsed.isValid())
		return Qt::Alignment(d->parsed.toInt());

	if (d->values.isEmpty() || d->values.count() > 2)
		return Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop;

	Qt::Alignment v = parseAlignment(d->values.constData(), d->values.count());
	d->parsed = int(v);
	return v;
}

void Declaration2::borderImageValue(QString* image, int* cuts,
								   TileMode2* h, TileMode2* v) const
{
	if (image != nullptr && cuts != nullptr && h != nullptr && v != nullptr)
	{
		*image = uriValue();

		for (int i = 0; i < 4; i++)
			cuts[i] = -1;

		*h = *v = TileMode2::TileMode_Stretch2;

		if (d->values.count() < 2)
			return;

		if (d->values.at(1).type == Value2::Number2)   // cuts!
		{
			int i;

			for (i = 0; i < qMin(d->values.count() - 1, 4); i++)
			{
				const Value2& vv = d->values.at(i + 1);

				if (vv.type != Value2::Number2)
					break;

				cuts[i] = vv.variant.toString().toInt();
			}

			if (i == 0) cuts[0] = cuts[1] = cuts[2] = cuts[3] = 0;
			else if (i == 1) cuts[3] = cuts[2] = cuts[1] = cuts[0];
			else if (i == 2) cuts[2] = cuts[0], cuts[3] = cuts[1];
			else if (i == 3) cuts[3] = cuts[1];
		}

		if (d->values.last().type == Value2::Identifier2)
		{
			*v = static_cast<TileMode2>(findKnownValue(d->values.last().variant.toString(),
									   tileModes, TileMode2::NumKnownTileModes2));
		}

		if (d->values[d->values.count() - 2].type == Value2::Identifier2)
		{
			*h = static_cast<TileMode2>
				 (findKnownValue(d->values[d->values.count() - 2].variant.toString(),
								 tileModes, TileMode2::NumKnownTileModes2));
		}
		else
			*h = *v;
	}
}

QIcon Declaration2::iconValue() const
{
	if (d->parsed.isValid())
		return qvariant_cast<QIcon>(d->parsed);

	QIcon icon;

	for (int i = 0; i < d->values.count();)
	{
		const Value2& value = d->values.at(i++);

		if (value.type != Value2::Uri2)
			break;

		QString uri = value.variant.toString();
		QIcon::Mode mode = QIcon::Normal;
		QIcon::State state = QIcon::Off;

		for (int j = 0; j < 2; j++)
		{
			if (i != d->values.count() && d->values.at(i).type == Value2::KnownIdentifier2)
			{
				switch (d->values.at(i).variant.toInt())
				{
					case KnownValue2::Value_Disabled2: mode = QIcon::Disabled; break;

					case KnownValue2::Value_Active2: mode = QIcon::Active; break;

					case KnownValue2::Value_Selected2: mode = QIcon::Selected; break;

					case KnownValue2::Value_Normal2: mode = QIcon::Normal; break;

					case KnownValue2::Value_On2: state = QIcon::On; break;

					case KnownValue2::Value_Off2: state = QIcon::Off; break;

					default: break;
				}

				++i;
			}
			else
			{
				break;
			}
		}

		// QIcon is soo broken
		if (icon.isNull())
			icon = QIcon(uri);
		else
			icon.addPixmap(uri, mode, state);

		if (i == d->values.count())
			break;

		if (d->values.at(i).type == Value2::TermOperatorComma2)
			i++;
	}

	d->parsed = QVariant::fromValue<QIcon>(icon);
	return icon;
}

///////////////////////////////////////////////////////////////////////////////
// Selector
int Selector2::specificity() const
{
	int val = 0;

	for (int i = 0; i < basicSelectors.count(); ++i)
	{
		const BasicSelector2& sel = basicSelectors.at(i);

		if (!sel.elementName.isEmpty())
			val += 1;

		val += (sel.pseudos.count() + sel.attributeSelectors.count()) * 0x10;
		val += sel.ids.count() * 0x100;
	}

	return val;
}

QString Selector2::pseudoElement() const
{
	const BasicSelector2& bs = basicSelectors.last();

	if (!bs.pseudos.isEmpty() && bs.pseudos.at(0).type == PseudoClass_Unknown)
		return bs.pseudos.at(0).name;

	return QString();
}

quint64 Selector2::pseudoClass(quint64* negated) const
{
	const BasicSelector2& bs = basicSelectors.last();

	if (bs.pseudos.isEmpty())
		return PseudoClass_Unspecified;

	quint64 pc = PseudoClass_Unknown;

	for (int i = !pseudoElement().isEmpty(); i < bs.pseudos.count(); i++)
	{
		const Pseudo2& pseudo = bs.pseudos.at(i);

		if (pseudo.type == PseudoClass_Unknown)
			return PseudoClass_Unknown;

		if (!pseudo.negated)
			pc |= pseudo.type;
		else if (negated)
			*negated |= pseudo.type;
	}

	return pc;
}

///////////////////////////////////////////////////////////////////////////////
// StyleSheet
void StyleSheet2::buildIndexes(Qt::CaseSensitivity nameCaseSensitivity)
{
	QVector<StyleRule2> universals;

	for (int i = 0; i < styleRules.count(); ++i)
	{
		const StyleRule2& rule = styleRules.at(i);
		QVector<Selector2> universalsSelectors;

		for (int j = 0; j < rule.selectors.count(); ++j)
		{
			const Selector2& selector = rule.selectors.at(j);

			if (selector.basicSelectors.isEmpty())
				continue;

			if (selector.basicSelectors.at(0).relationToNext == BasicSelector2::NoRelation2)
			{
				if (selector.basicSelectors.count() != 1)
					continue;
			}
			else if (selector.basicSelectors.count() <= 1)
			{
				continue;
			}

			const BasicSelector2& sel = selector.basicSelectors.at(selector.basicSelectors.count() - 1);

			if (!sel.ids.isEmpty())
			{
				StyleRule2 nr;
				nr.selectors += selector;
				nr.declarations = rule.declarations;
				nr.order = i;
				idIndex.insert(sel.ids.at(0), nr);
			}
			else if (!sel.elementName.isEmpty())
			{
				StyleRule2 nr;
				nr.selectors += selector;
				nr.declarations = rule.declarations;
				nr.order = i;
				QString name = sel.elementName;

				if (nameCaseSensitivity == Qt::CaseSensitivity::CaseInsensitive)
					name = name.toLower();

				nameIndex.insert(name, nr);
			}
			else
			{
				universalsSelectors += selector;
			}
		}

		if (!universalsSelectors.isEmpty())
		{
			StyleRule2 nr;
			nr.selectors = universalsSelectors;
			nr.declarations = rule.declarations;
			nr.order = i;
			universals << nr;
		}
	}

	styleRules = universals;
}

///////////////////////////////////////////////////////////////////////////////
// StyleSelector
StyleSelector2::~StyleSelector2()
{
}

bool StyleSelector2::nodeNameEquals(NodePtr2 node, const QString& nodeName) const
{
	return nodeNames(node).contains(nodeName, nameCaseSensitivity);
}

QStringList StyleSelector2::nodeIds(NodePtr2 node) const
{
	return QStringList(attribute(node, QLatin1String("id")));
}

bool StyleSelector2::selectorMatches(const Selector2& selector, NodePtr2 node)
{
	if (selector.basicSelectors.isEmpty())
		return false;

	if (selector.basicSelectors.at(0).relationToNext == BasicSelector2::NoRelation2)
	{
		if (selector.basicSelectors.count() != 1)
			return false;

		return basicSelectorMatches(selector.basicSelectors.at(0), node);
	}

	if (selector.basicSelectors.count() <= 1)
		return false;

	int i = selector.basicSelectors.count() - 1;
	node = duplicateNode(node);
	bool match = true;
	BasicSelector2 sel = selector.basicSelectors.at(i);

	do
	{
		match = basicSelectorMatches(sel, node);

		if (!match)
		{
			if (sel.relationToNext == BasicSelector2::MatchNextSelectorIfParent2
					|| i == selector.basicSelectors.count() - 1) // first element must always match!
				break;
		}

		if (match || sel.relationToNext != BasicSelector2::MatchNextSelectorIfAncestor2)
			--i;

		if (i < 0)
			break;

		sel = selector.basicSelectors.at(i);

		if (sel.relationToNext == BasicSelector2::MatchNextSelectorIfAncestor2
				|| sel.relationToNext == BasicSelector2::MatchNextSelectorIfParent2)
		{
			const auto nextParent = parentNode(node);
			freeNode(node);
			node = nextParent;
		}
		else if (sel.relationToNext == BasicSelector2::MatchNextSelectorIfPreceeds2)
		{
			const auto previousSibling = previousSiblingNode(node);
			freeNode(node);
			node = previousSibling;
		}

		if (isNullNode(node))
		{
			match = false;
			break;
		}
	}
	while (i >= 0 && (match || sel.relationToNext == BasicSelector2::MatchNextSelectorIfAncestor2));

	freeNode(node);
	return match;
}

bool StyleSelector2::basicSelectorMatches(const BasicSelector2& sel, NodePtr2 node)
{
	if (!sel.attributeSelectors.isEmpty())
	{
		if (!hasAttributes(node))
			return false;

		for (int i = 0; i < sel.attributeSelectors.count(); ++i)
		{
			const QCss2::AttributeSelector2& a = sel.attributeSelectors.at(i);
			const QString attrValue = attribute(node, a.name);

			if (attrValue.isNull())
				return false;

			if (a.valueMatchCriterium == QCss2::AttributeSelector2::MatchContains2)
			{
				QStringList lst = attrValue.split(QLatin1Char(' '));

				if (!lst.contains(a.value))
					return false;
			}
			else if (
				(a.valueMatchCriterium == QCss2::AttributeSelector2::MatchEqual2
				 && attrValue != a.value)
				||
				(a.valueMatchCriterium == QCss2::AttributeSelector2::MatchBeginsWith2
				 && !attrValue.startsWith(a.value))
			)
				return false;
		}
	}

	if (!sel.elementName.isEmpty()
			&& !nodeNameEquals(node, sel.elementName))
		return false;

	if (!sel.ids.isEmpty()
			&& sel.ids != nodeIds(node))
		return false;

	return true;
}

void StyleSelector2::matchRule(NodePtr2 node, const StyleRule2& rule, StyleSheetOrigin2 origin,
							  int depth, QMap<uint, StyleRule2>* weightedRules)
{
	for (int j = 0; j < rule.selectors.count(); ++j)
	{
		const Selector2& selector = rule.selectors.at(j);

		if (selectorMatches(selector, node))
		{
			uint const weight = rule.order
								+ selector.specificity() * 0x100
								+ (uint(origin) + depth) * 0x100000;
			StyleRule2 newRule = rule;

			if (rule.selectors.count() > 1)
			{
				newRule.selectors.resize(1);
				newRule.selectors[0] = selector;
			}

			//We might have rules with the same weight if they came from a rule with several selectors
			if (weightedRules != nullptr)
				weightedRules->insert(weight, newRule);
		}
	}
}

// Returns style rules that are in ascending order of specificity
// Each of the StyleRule returned will contain exactly one Selector
QVector<StyleRule2> StyleSelector2::styleRulesForNode(NodePtr2 node)
{
	QVector<StyleRule2> rules;

	if (styleSheets.isEmpty())
		return rules;

	QMap<uint, StyleRule2> weightedRules; // (spec, rule) that will be sorted below

	//prune using indexed stylesheet
	for (int sheetIdx = 0; sheetIdx < styleSheets.count(); ++sheetIdx)
	{
		const StyleSheet2& styleSheet = styleSheets.at(sheetIdx);

		for (int i = 0; i < styleSheet.styleRules.count(); ++i)
		{
			matchRule(node, styleSheet.styleRules.at(i), styleSheet.origin, styleSheet.depth, &weightedRules);
		}

		if (!styleSheet.idIndex.isEmpty())
		{
			QStringList ids = nodeIds(node);

			for (int i = 0; i < ids.count(); i++)
			{
				const QString& key = ids.at(i);
				QMultiHash<QString, StyleRule2>::const_iterator it = styleSheet.idIndex.constFind(key);

				while (it != styleSheet.idIndex.constEnd() && it.key() == key)
				{
					matchRule(node, it.value(), styleSheet.origin, styleSheet.depth, &weightedRules);
					++it;
				}
			}
		}

		if (!styleSheet.nameIndex.isEmpty())
		{
			QStringList names = nodeNames(node);

			for (int i = 0; i < names.count(); i++)
			{
				QString name = names.at(i);

				if (nameCaseSensitivity == Qt::CaseSensitivity::CaseInsensitive)
					name = name.toLower();

				QMultiHash<QString, StyleRule2>::const_iterator it = styleSheet.nameIndex.constFind(name);

				while (it != styleSheet.nameIndex.constEnd() && it.key() == name)
				{
					matchRule(node, it.value(), styleSheet.origin, styleSheet.depth, &weightedRules);
					++it;
				}
			}
		}

		if (!medium.isEmpty())
		{
			for (int i = 0; i < styleSheet.mediaRules.count(); ++i)
			{
				if (styleSheet.mediaRules.at(i).media.contains(medium, Qt::CaseSensitivity::CaseInsensitive))
				{
					for (int j = 0; j < styleSheet.mediaRules.at(i).styleRules.count(); ++j)
					{
						matchRule(node, styleSheet.mediaRules.at(i).styleRules.at(j), styleSheet.origin,
								  styleSheet.depth, &weightedRules);
					}
				}
			}
		}
	}

	rules.reserve(weightedRules.count());
	QMap<uint, StyleRule2>::const_iterator it = weightedRules.constBegin();

	for ( ; it != weightedRules.constEnd() ; ++it)
		rules += *it;

	return rules;
}

// for qtexthtmlparser which requires just the declarations with Enabled state
// and without pseudo elements
QVector<Declaration2> StyleSelector2::declarationsForNode(NodePtr2 node, const char* extraPseudo)
{
	QVector<Declaration2> decls;
	QVector<StyleRule2> rules = styleRulesForNode(node);

	for (int i = 0; i < rules.count(); i++)
	{
		const Selector2& selector = rules.at(i).selectors.at(0);
		const QString pseudoElement = selector.pseudoElement();

		if (extraPseudo && pseudoElement == QLatin1String(extraPseudo))
		{
			decls += rules.at(i).declarations;
			continue;
		}

		if (!pseudoElement.isEmpty()) // skip rules with pseudo elements
			continue;

		quint64 const pseudoClass = selector.pseudoClass();

		if (pseudoClass == PseudoClass_Enabled || pseudoClass == PseudoClass_Unspecified)
			decls += rules.at(i).declarations;
	}

	return decls;
}

QString Symbol2::lexem() const
{
	QString result;

	if (len > 0)
		result.reserve(len);

	for (int i = 0; i < len; ++i)
	{
		if (text.at(start + i) == QLatin1Char('\\') && i < len - 1)
			++i;

		result += text.at(start + i);
	}

	return result;
}

Parser2::Parser2(const QString& css, bool isFile)
{
	init(css, isFile);
}

Parser2::Parser2() noexcept
{
	index = 0;
	errorIndex = -1;
	hasEscapeSequences = false;
}

void Parser2::init(const QString& css, bool isFile)
{
	QString styleSheet = css;

	if (isFile)
	{
		QFile file(css);

		if (file.open(QFile::ReadOnly))
		{
			sourcePath = QFileInfo(styleSheet).absolutePath() + QLatin1Char('/');
			QTextStream stream(&file);
			styleSheet = stream.readAll();
		}
		else
		{
			qWarning() << "QCss::Parser - Failed to load file " << css;
			styleSheet.clear();
		}
	}
	else
	{
		sourcePath.clear();
	}

	hasEscapeSequences = false;
	symbols.resize(0);
	symbols.reserve(8);
	Scanner2::scan(Scanner2::preprocess(styleSheet, &hasEscapeSequences), &symbols);
	index = 0;
	errorIndex = -1;
}

bool Parser2::parse(StyleSheet2* styleSheet, Qt::CaseSensitivity nameCaseSensitivity)
{
	if (testTokenAndEndsWith(TokenType2::ATKEYWORD_SYM, QLatin1String("charset")))
	{
		if (!next(TokenType2::STRING)) return false;

		if (!next(TokenType2::SEMICOLON)) return false;
	}

	while (test(TokenType2::S) || test(TokenType2::CDO) || test(TokenType2::CDC)) {}

	if (styleSheet != nullptr)
	{
		while (testImport())
		{
			ImportRule2 rule;

			if (!parseImport(&rule)) return false;

			styleSheet->importRules.append(rule);

			while (test(TokenType2::S) || test(TokenType2::CDO) || test(TokenType2::CDC)) {}
		}

		do
		{
			if (testMedia())
			{
				MediaRule2 rule;

				if (!parseMedia(&rule)) return false;

				styleSheet->mediaRules.append(rule);
			}
			else if (testPage())
			{
				PageRule2 rule;

				if (!parsePage(&rule)) return false;

				styleSheet->pageRules.append(rule);
			}
			else if (testRuleset())
			{
				StyleRule2 rule;

				if (!parseRuleset(&rule)) return false;

				styleSheet->styleRules.append(rule);
			}
			else if (test(TokenType2::ATKEYWORD_SYM))
			{
				if (!until(TokenType2::RBRACE)) return false;
			}
			else if (hasNext())
			{
				return false;
			}

			while (test(TokenType2::S) || test(TokenType2::CDO) || test(TokenType2::CDC)) {}
		}
		while (hasNext());

		styleSheet->buildIndexes(nameCaseSensitivity);
	}

	return true;
}

Symbol2 Parser2::errorSymbol()
{
	if (errorIndex == -1) return Symbol2();

	return symbols.at(errorIndex);
}

static inline void removeOptionalQuotes(QString* str)
{
	if (str != nullptr)
	{
		if (!str->startsWith(QLatin1Char('\''))
				&& !str->startsWith(QLatin1Char('\"')))
			return;

		str->remove(0, 1);
		str->chop(1);
	}
}

bool Parser2::parseImport(ImportRule2* importRule)
{
	if (importRule != nullptr)
	{
		skipSpace();

		if (test(TokenType2::STRING))
		{
			importRule->href = lexem();
		}
		else
		{
			if (!testAndParseUri(&importRule->href)) return false;
		}

		removeOptionalQuotes(&importRule->href);
		skipSpace();

		if (testMedium())
		{
			if (!parseMedium(&importRule->media)) return false;

			while (test(TokenType2::COMMA))
			{
				skipSpace();

				if (!parseNextMedium(&importRule->media)) return false;
			}
		}

		if (!next(TokenType2::SEMICOLON)) return false;

		skipSpace();
	}

	return true;
}

bool Parser2::parseMedia(MediaRule2* mediaRule)
{
	if (mediaRule != nullptr)
	{
		do
		{
			skipSpace();

			if (!parseNextMedium(&mediaRule->media)) return false;
		}
		while (test(TokenType2::COMMA));

		if (!next(TokenType2::LBRACE)) return false;

		skipSpace();

		while (testRuleset())
		{
			StyleRule2 rule;

			if (!parseRuleset(&rule)) return false;

			mediaRule->styleRules.append(rule);
		}

		if (!next(TokenType2::RBRACE)) return false;

		skipSpace();
	}

	return true;
}

bool Parser2::parseMedium(QStringList* media)
{
	if (media != nullptr)
	{
		media->append(lexem());
		skipSpace();
	}

	return true;
}

bool Parser2::parsePage(PageRule2* pageRule)
{
	if (pageRule != nullptr)
	{
		skipSpace();

		if (testPseudoPage())
			if (!parsePseudoPage(&pageRule->selector)) return false;

		skipSpace();

		if (!next(TokenType2::LBRACE)) return false;

		do
		{
			skipSpace();
			Declaration2 decl;

			if (!parseNextDeclaration(&decl)) return false;

			if (!decl.isEmpty())
				pageRule->declarations.append(decl);
		}
		while (test(TokenType2::SEMICOLON));

		if (!next(TokenType2::RBRACE)) return false;

		skipSpace();
	}

	return true;
}

bool Parser2::parsePseudoPage(QString* selector)
{
	if (!next(TokenType2::IDENT)) return false;

	*selector = lexem();
	return true;
}

bool Parser2::parseNextOperator(Value2* value)
{
	if (!hasNext()) return true;

	if (value != nullptr)
	{
		switch (next())
		{
			case TokenType2::SLASH: value->type = Value2::TermOperatorSlash2; skipSpace(); break;
			case TokenType2::COMMA: value->type = Value2::TermOperatorComma2; skipSpace(); break;
			case TokenType2::NONE:
			case TokenType2::S:
			case TokenType2::CDO:
			case TokenType2::CDC:
			case TokenType2::INCLUDES:
			case TokenType2::DASHMATCH:
			case TokenType2::LBRACE:
			case TokenType2::PLUS:
			case TokenType2::GREATER:
			case TokenType2::STRING:
			case TokenType2::INVALID:
			case TokenType2::IDENT:
			case TokenType2::HASH:
			case TokenType2::ATKEYWORD_SYM:
			case TokenType2::EXCLAMATION_SYM:
			case TokenType2::LENGTH:
			case TokenType2::PERCENTAGE:
			case TokenType2::NUMBER:
			case TokenType2::FUNCTION:
			case TokenType2::COLON:
			case TokenType2::SEMICOLON:
			case TokenType2::RBRACE:
			case TokenType2::MINUS:
			case TokenType2::DOT:
			case TokenType2::STAR:
			case TokenType2::LBRACKET:
			case TokenType2::RBRACKET:
			case TokenType2::EQUAL:
			case TokenType2::LPAREN:
			case TokenType2::RPAREN:
			case TokenType2::OR:

			default: prev(); break;
		}
	}

	return true;
}

bool Parser2::parseCombinator(BasicSelector2::Relation2* relation)
{
	if (relation != nullptr)
	{
		*relation = BasicSelector2::NoRelation2;

		if (lookup() == TokenType2::S)
		{
			*relation = BasicSelector2::MatchNextSelectorIfAncestor2;
			skipSpace();
		}
		else
		{
			prev();
		}

		if (test(TokenType2::PLUS))
		{
			*relation = BasicSelector2::MatchNextSelectorIfPreceeds2;
		}
		else if (test(TokenType2::GREATER))
		{
			*relation = BasicSelector2::MatchNextSelectorIfParent2;
		}

		skipSpace();
	}

	return true;
}

bool Parser2::parseProperty(Declaration2* decl)
{
	if (decl != nullptr)
	{
		decl->d->property = lexem();
		decl->d->propertyId = static_cast<Property2>(findKnownValue(decl->d->property, properties, NumProperties2));
		skipSpace();
	}

	return true;
}

bool Parser2::parseRuleset(StyleRule2* styleRule)
{
	if (styleRule != nullptr)
	{
		Selector2 sel;

		if (!parseSelector(&sel)) return false;

		styleRule->selectors.append(sel);

		while (test(TokenType2::COMMA))
		{
			skipSpace();
			Selector2 sel2;

			if (!parseNextSelector(&sel2)) return false;

			styleRule->selectors.append(sel2);
		}

		skipSpace();

		if (!next(TokenType2::LBRACE)) return false;

		const int declarationStart = index;

		do
		{
			skipSpace();
			Declaration2 decl;
			const int rewind = index;

			if (!parseNextDeclaration(&decl))
			{
				index = rewind;
				const bool foundSemicolon = until(TokenType2::SEMICOLON);
				const int semicolonIndex = index;
				index = declarationStart;
				const bool foundRBrace = until(TokenType2::RBRACE);

				if (foundSemicolon && semicolonIndex < index)
				{
					decl = Declaration2();
					index = semicolonIndex - 1;
				}
				else
				{
					skipSpace();
					return foundRBrace;
				}
			}

			if (!decl.isEmpty())
				styleRule->declarations.append(decl);
		}
		while (test(TokenType2::SEMICOLON));

		if (!next(TokenType2::RBRACE)) return false;

		skipSpace();
	}

	return true;
}

bool Parser2::parseSelector(Selector2* sel)
{
	if (sel != nullptr)
	{
		BasicSelector2 basicSel;

		if (!parseSimpleSelector(&basicSel)) return false;

		while (testCombinator())
		{
			if (!parseCombinator(&basicSel.relationToNext)) return false;

			if (!testSimpleSelector()) break;

			sel->basicSelectors.append(basicSel);
			basicSel = BasicSelector2();

			if (!parseSimpleSelector(&basicSel)) return false;
		}

		sel->basicSelectors.append(basicSel);
	}

	return true;
}

bool Parser2::parseSimpleSelector(BasicSelector2* basicSel)
{
	if (basicSel == nullptr)
		return false;

	int minCount = 0;

	if (lookupElementName())
	{
		if (!parseElementName(&basicSel->elementName)) return false;
	}
	else
	{
		prev();
		minCount = 1;
	}

	bool onceMore;
	int count = 0;

	do
	{
		onceMore = false;

		if (test(TokenType2::HASH))
		{
			QString theid = lexem();
			// chop off leading #
			theid.remove(0, 1);
			basicSel->ids.append(theid);
			onceMore = true;
		}
		else if (testClass())
		{
			onceMore = true;
			AttributeSelector2 a;
			a.name = QLatin1String("class");
			a.valueMatchCriterium = AttributeSelector2::MatchContains2;

			if (!parseClass(&a.value)) return false;

			basicSel->attributeSelectors.append(a);
		}
		else if (testAttrib())
		{
			onceMore = true;
			AttributeSelector2 a;

			if (!parseAttrib(&a)) return false;

			basicSel->attributeSelectors.append(a);
		}
		else if (testPseudo())
		{
			onceMore = true;
			Pseudo2 ps;

			if (!parsePseudo(&ps)) return false;

			basicSel->pseudos.append(ps);
		}

		if (onceMore) ++count;
	}
	while (onceMore);

	return count >= minCount;
}

bool Parser2::parseClass(QString* name)
{
	if (!next(TokenType2::IDENT)) return false;

	if (name != nullptr)
		*name = lexem();

	return true;
}

bool Parser2::parseElementName(QString* name)
{
	if (name != nullptr)
	{
		switch (lookup())
		{
			case TokenType2::STAR: name->clear(); break;
			case TokenType2::IDENT: *name = lexem(); break;
			case TokenType2::NONE:
			case TokenType2::S:
			case TokenType2::CDO:
			case TokenType2::CDC:
			case TokenType2::INCLUDES:
			case TokenType2::DASHMATCH:
			case TokenType2::LBRACE:
			case TokenType2::PLUS:
			case TokenType2::GREATER:
			case TokenType2::COMMA:
			case TokenType2::STRING:
			case TokenType2::INVALID:
			case TokenType2::HASH:
			case TokenType2::ATKEYWORD_SYM:
			case TokenType2::EXCLAMATION_SYM:
			case TokenType2::LENGTH:
			case TokenType2::PERCENTAGE:
			case TokenType2::NUMBER:
			case TokenType2::FUNCTION:
			case TokenType2::COLON:
			case TokenType2::SEMICOLON:
			case TokenType2::RBRACE:
			case TokenType2::SLASH:
			case TokenType2::MINUS:
			case TokenType2::DOT:
			case TokenType2::LBRACKET:
			case TokenType2::RBRACKET:
			case TokenType2::EQUAL:
			case TokenType2::LPAREN:
			case TokenType2::RPAREN:
			case TokenType2::OR:

			default: return false;
		}
	}

	return true;
}

bool Parser2::parseAttrib(AttributeSelector2* attr)
{
	skipSpace();

	if (!next(TokenType2::IDENT)) return false;

	if (attr != nullptr)
	{
		attr->name = lexem();
		skipSpace();

		if (test(TokenType2::EQUAL))
		{
			attr->valueMatchCriterium = AttributeSelector2::MatchEqual2;
		}
		else if (test(TokenType2::INCLUDES))
		{
			attr->valueMatchCriterium = AttributeSelector2::MatchContains2;
		}
		else if (test(TokenType2::DASHMATCH))
		{
			attr->valueMatchCriterium = AttributeSelector2::MatchBeginsWith2;
		}
		else
		{
			return next(TokenType2::RBRACKET);
		}

		skipSpace();

		if (!test(TokenType2::IDENT) && !test(TokenType2::STRING)) return false;

		attr->value = unquotedLexem();
		skipSpace();
	}

	return next(TokenType2::RBRACKET);
}

bool Parser2::parsePseudo(Pseudo2* pseudo)
{
	std::ignore = test(TokenType2::COLON);

	if (pseudo != nullptr)
	{
		pseudo->negated = test(TokenType2::EXCLAMATION_SYM);

		if (test(TokenType2::IDENT))
		{
			pseudo->name = lexem();
			pseudo->type = static_cast<quint64>(findKnownValue(pseudo->name, pseudos, NumPseudos));
			return true;
		}

		if (!next(TokenType2::FUNCTION)) return false;

		pseudo->function = lexem();
		// chop off trailing parenthesis
		pseudo->function.chop(1);
		skipSpace();

		if (!test(TokenType2::IDENT)) return false;

		pseudo->name = lexem();
		skipSpace();
	}

	return next(TokenType2::RPAREN);
}

bool Parser2::parseNextDeclaration(Declaration2* decl)
{
	if (!testProperty())
		return true; // not an error!

	if (decl != nullptr)
	{
		if (!parseProperty(decl)) return false;

		if (!next(TokenType2::COLON)) return false;

		skipSpace();

		if (!parseNextExpr(&decl->d->values)) return false;

		if (testPrio())
			if (!parsePrio(decl)) return false;
	}

	return true;
}

bool Parser2::testPrio()
{
	const int rewind = index;

	if (!test(TokenType2::EXCLAMATION_SYM)) return false;

	skipSpace();

	if (!test(TokenType2::IDENT))
	{
		index = rewind;
		return false;
	}

	if (lexem().compare(QLatin1String("important"), Qt::CaseSensitivity::CaseInsensitive) != 0)
	{
		index = rewind;
		return false;
	}

	return true;
}

bool Parser2::parsePrio(Declaration2* declaration)
{
	if (declaration != nullptr)
		declaration->d->important = true;

	skipSpace();
	return true;
}

bool Parser2::parseExpr(QVector<Value2>* vals)
{
	Value2 val;

	if (!parseTerm(&val)) return false;

	if (vals != nullptr)
	{
		vals->append(val);
		bool onceMore;

		do
		{
			onceMore = false;
			val = Value2();

			if (!parseNextOperator(&val)) return false;

			if (val.type != QCss2::Value2::Unknown2)
				vals->append(val);

			if (testTerm())
			{
				onceMore = true;
				val = Value2();

				if (!parseTerm(&val)) return false;

				vals->append(val);
			}
		}
		while (onceMore);
	}

	return true;
}

bool Parser2::testTerm()
{
	return test(TokenType2::PLUS) || test(TokenType2::MINUS)
		   || test(TokenType2::NUMBER)
		   || test(TokenType2::PERCENTAGE)
		   || test(TokenType2::LENGTH)
		   || test(TokenType2::STRING)
		   || test(TokenType2::IDENT)
		   || testHexColor()
		   || testFunction();
}

bool Parser2::parseTerm(Value2* value)
{
	QString str = lexem();
	bool haveUnary = false;

	if (lookup() == TokenType2::PLUS || lookup() == TokenType2::MINUS)
	{
		haveUnary = true;

		if (!hasNext()) return false;

		next();
		str += lexem();
	}

	if (value != nullptr)
	{
		value->variant = str;
		value->type = QCss2::Value2::String2;

		switch (lookup())
		{
			case TokenType2::NUMBER:
				value->type = Value2::Number2;
				value->variant.convert(QMetaType(QMetaType::Double));
				break;

			case TokenType2::PERCENTAGE:
				value->type = Value2::Percentage2;
				str.chop(1); // strip off %
				value->variant = str;
				break;

			case TokenType2::LENGTH:
				value->type = Value2::Length2;
				break;

			case TokenType2::STRING:
				if (haveUnary) return false;

				value->type = Value2::String2;
				str.chop(1);
				str.remove(0, 1);
				value->variant = str;
				break;

			case TokenType2::IDENT:
			{
				if (haveUnary) return false;

				value->type = Value2::Identifier2;
				const int theid = findKnownValue(str, values, NumKnownValues2);

				if (theid != 0)
				{
					value->type = Value2::KnownIdentifier2;
					value->variant = theid;
				}

				break;
			}

			case TokenType2::NONE:
			case TokenType2::S:
			case TokenType2::CDO:
			case TokenType2::CDC:
			case TokenType2::INCLUDES:
			case TokenType2::DASHMATCH:
			case TokenType2::LBRACE:
			case TokenType2::PLUS:
			case TokenType2::GREATER:
			case TokenType2::COMMA:
			case TokenType2::INVALID:
			case TokenType2::HASH:
			case TokenType2::ATKEYWORD_SYM:
			case TokenType2::EXCLAMATION_SYM:
			case TokenType2::FUNCTION:
			case TokenType2::COLON:
			case TokenType2::SEMICOLON:
			case TokenType2::RBRACE:
			case TokenType2::SLASH:
			case TokenType2::MINUS:
			case TokenType2::DOT:
			case TokenType2::STAR:
			case TokenType2::LBRACKET:
			case TokenType2::RBRACKET:
			case TokenType2::EQUAL:
			case TokenType2::LPAREN:
			case TokenType2::RPAREN:
			case TokenType2::OR:

			default:
			{
				if (haveUnary) return false;

				prev();

				if (testHexColor())
				{
					QColor col;

					if (!parseHexColor(&col)) return false;

					value->type = Value2::Color2;
					value->variant = col;
				}
				else if (testFunction())
				{
					QString name, args;

					if (!parseFunction(&name, &args)) return false;

					if (name == QLatin1String("url"))
					{
						value->type = Value2::Uri2;
						removeOptionalQuotes(&args);

						if (QFileInfo(args).isRelative() && !sourcePath.isEmpty())
						{
							args.prepend(sourcePath);
						}

						value->variant = args;
					}
					else
					{
						value->type = Value2::Function2;
						value->variant = QStringList() << name << args;
					}
				}
				else
				{
					return recordError();
				}

				return true;
			}
		}

		skipSpace();
	}

	return true;
}

bool Parser2::parseFunction(QString* name, QString* args)
{
	if (name != nullptr && args != nullptr)
	{
		*name = lexem();
		name->chop(1);
		skipSpace();
		const int start = index;

		if (!until(TokenType2::RPAREN)) return false;

		for (int i = start; i < index - 1; ++i)
			args->append(symbols.at(i).lexem());

		/*
			if (!nextExpr(&arguments)) return false;
			if (!next(RPAREN)) return false;
		*/
		skipSpace();
	}

	return true;
}

bool Parser2::parseHexColor(QColor* col)
{
	if (col != nullptr)
	{
		col->fromString(lexem());

		if (!col->isValid())
		{
			//qWarning("QCssParser::parseHexColor: Unknown color name '%s'", lexem().toLatin1().constData());
			return false;
		}

		skipSpace();
	}

	return true;
}

bool Parser2::testAndParseUri(QString* uri)
{
	const int rewind = index;

	if (!testFunction()) return false;

	QString name, args;

	if (!parseFunction(&name, &args))
	{
		index = rewind;
		return false;
	}

	if (name.toLower() != QLatin1String("url"))
	{
		index = rewind;
		return false;
	}

	if (uri != nullptr)
	{
		*uri = args;
		removeOptionalQuotes(uri);
	}

	return true;
}

bool Parser2::testSimpleSelector()
{
	return testElementName()
		   || (test(TokenType2::HASH))
		   || testClass()
		   || testAttrib()
		   || testPseudo();
}

bool Parser2::next(QCss2::TokenType2 t)
{
	if (hasNext() && next() == t)
		return true;

	return recordError();
}

bool Parser2::test(QCss2::TokenType2 t) noexcept
{
	if (index >= symbols.count())
		return false;

	if (symbols.at(index).token == t)
	{
		++index;
		return true;
	}

	return false;
}

QString Parser2::unquotedLexem() const
{
	QString s = lexem();

	if (lookup() == TokenType2::STRING)
	{
		s.chop(1);
		s.remove(0, 1);
	}

	return s;
}

QString Parser2::lexemUntil(QCss2::TokenType2 t)
{
	QString lexem;

	while (hasNext() && next() != t)
		lexem += symbol().lexem();

	return lexem;
}

bool Parser2::until(QCss2::TokenType2 target, QCss2::TokenType2 target2) noexcept
{
	int braceCount = 0;
	int brackCount = 0;
	int parenCount = 0;

	if (index)
	{
		switch (symbols.at(index - 1).token)
		{
			case TokenType2::LBRACE: ++braceCount; break;
			case TokenType2::LBRACKET: ++brackCount; break;
			case TokenType2::FUNCTION:
			case TokenType2::LPAREN: ++parenCount; break;
			case TokenType2::NONE:
			case TokenType2::S:
			case TokenType2::CDO:
			case TokenType2::CDC:
			case TokenType2::INCLUDES:
			case TokenType2::DASHMATCH:
			case TokenType2::PLUS:
			case TokenType2::GREATER:
			case TokenType2::COMMA:
			case TokenType2::STRING:
			case TokenType2::INVALID:
			case TokenType2::IDENT:
			case TokenType2::HASH:
			case TokenType2::ATKEYWORD_SYM:
			case TokenType2::EXCLAMATION_SYM:
			case TokenType2::LENGTH:
			case TokenType2::PERCENTAGE:
			case TokenType2::NUMBER:
			case TokenType2::COLON:
			case TokenType2::SEMICOLON:
			case TokenType2::RBRACE:
			case TokenType2::SLASH:
			case TokenType2::MINUS:
			case TokenType2::DOT:
			case TokenType2::STAR:
			case TokenType2::RBRACKET:
			case TokenType2::EQUAL:
			case TokenType2::RPAREN:
			case TokenType2::OR:

			default: ;
		}
	}

	while (index < symbols.size())
	{
		QCss2::TokenType2 const t = symbols.at(index++).token;

		switch (t)
		{
			case TokenType2::LBRACE: ++braceCount; break;
			case TokenType2::RBRACE: --braceCount; break;
			case TokenType2::LBRACKET: ++brackCount; break;
			case TokenType2::RBRACKET: --brackCount; break;
			case TokenType2::FUNCTION:
			case TokenType2::LPAREN: ++parenCount; break;
			case TokenType2::RPAREN: --parenCount; break;
			case TokenType2::NONE:
			case TokenType2::S:
			case TokenType2::CDO:
			case TokenType2::CDC:
			case TokenType2::INCLUDES:
			case TokenType2::DASHMATCH:
			case TokenType2::PLUS:
			case TokenType2::GREATER:
			case TokenType2::COMMA:
			case TokenType2::STRING:
			case TokenType2::INVALID:
			case TokenType2::IDENT:
			case TokenType2::HASH:
			case TokenType2::ATKEYWORD_SYM:
			case TokenType2::EXCLAMATION_SYM:
			case TokenType2::LENGTH:
			case TokenType2::PERCENTAGE:
			case TokenType2::NUMBER:
			case TokenType2::COLON:
			case TokenType2::SEMICOLON:
			case TokenType2::SLASH:
			case TokenType2::MINUS:
			case TokenType2::DOT:
			case TokenType2::STAR:
			case TokenType2::EQUAL:
			case TokenType2::OR:

			default: break;
		}

		if ((t == target || (target2 != TokenType2::NONE && t == target2))
				&& braceCount <= 0
				&& brackCount <= 0
				&& parenCount <= 0)
			return true;

		if (braceCount < 0 || brackCount < 0 || parenCount < 0)
		{
			--index;
			break;
		}
	}

	return false;
}

bool Parser2::testTokenAndEndsWith(QCss2::TokenType2 t, QLatin1String str)
{
	if (!test(t)) return false;

	if (!lexem().endsWith(str, Qt::CaseSensitivity::CaseInsensitive))
	{
		prev();
		return false;
	}

	return true;
}
