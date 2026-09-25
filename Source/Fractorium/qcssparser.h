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
#pragma once

#include "FractoriumPch.h"

/// <summary>
/// The code in this file did not originate in Fractorium.
/// It was taken either in whole or in part from the source code
/// of Qt Creator. Their license applies.
/// </summary>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

//Some of these types *MUST* be named something different than the types of the same name
//in the Qt libraries because they clash
//with a Qt internal class of the same name and causes very strange crash bugs.
//This happens even though we use namespaces to avoid name collisions.
//The reason is that Qt uses a binary mapping of the class names to their internal types and
//when it sees a class with the same name as one of its own, it uses that class instead of its own,
//even if it's in a different namespace.
//This is a bug in Qt and has been reported to them, but they have not fixed it yet.
//What happens is: even if you never use this class, Qt internally uses it whenever
//setStyleSheet() or similar is called. So all of it's internal binary mapping points to this
//class rather than its own, causing memory corruption and then a crash.
//Some of these probably don't need renaming, but we rename them all to be safe and avoid any future problems.

class QIcon;

namespace QCss2
{

enum Property2
{
	UnknownProperty2,
	BackgroundColor2,
	Color2,
	Float2,
	Font2,
	FontFamily2,
	FontSize2,
	FontStyle2,
	FontWeight2,
	Margin2,
	MarginBottom2,
	MarginLeft2,
	MarginRight2,
	MarginTop2,
	QtBlockIndent2,
	QtListIndent2,
	QtParagraphType2,
	QtTableType2,
	QtUserState2,
	TextDecoration2,
	TextIndent2,
	TextUnderlineStyle2,
	VerticalAlignment2,
	Whitespace2,
	QtSelectionForeground2,
	QtSelectionBackground2,
	Border2,
	BorderLeft2,
	BorderRight2,
	BorderTop2,
	BorderBottom2,
	Padding2,
	PaddingLeft2,
	PaddingRight2,
	PaddingTop2,
	PaddingBottom2,
	PageBreakBefore2,
	PageBreakAfter2,
	QtAlternateBackground2,
	BorderLeftStyle2,
	BorderRightStyle2,
	BorderTopStyle2,
	BorderBottomStyle2,
	BorderStyles2,
	BorderLeftColor2,
	BorderRightColor2,
	BorderTopColor2,
	BorderBottomColor2,
	BorderColor2,
	BorderLeftWidth2,
	BorderRightWidth2,
	BorderTopWidth2,
	BorderBottomWidth2,
	BorderWidth2,
	BorderTopLeftRadius2,
	BorderTopRightRadius2,
	BorderBottomLeftRadius2,
	BorderBottomRightRadius2,
	BorderRadius2,
	Background2,
	BackgroundOrigin2,
	BackgroundClip2,
	BackgroundRepeat2,
	BackgroundPosition2,
	BackgroundAttachment2,
	BackgroundImage2,
	BorderImage2,
	QtSpacing2,
	Width2,
	Height2,
	MinimumWidth2,
	MinimumHeight2,
	MaximumWidth2,
	MaximumHeight2,
	QtImage2,
	Left2,
	Right2,
	Top2,
	Bottom2,
	QtOrigin2,
	QtPosition2,
	Position2,
	QtStyleFeatures2,
	QtBackgroundRole2,
	ListStyleType2,
	ListStyle2,
	QtImageAlignment2,
	TextAlignment2,
	Outline2,
	OutlineOffset2,
	OutlineWidth2,
	OutlineColor2,
	OutlineStyle2,
	OutlineRadius2,
	OutlineTopLeftRadius2,
	OutlineTopRightRadius2,
	OutlineBottomLeftRadius2,
	OutlineBottomRightRadius2,
	FontVariant2,
	TextTransform2,
	QtListNumberPrefix2,
	QtListNumberSuffix2,
	LineHeight2,
	NumProperties2
};

enum KnownValue2
{
	UnknownValue2,
	Value_Normal2,
	Value_Pre2,
	Value_NoWrap2,
	Value_PreWrap2,
	Value_Small2,
	Value_Medium2,
	Value_Large2,
	Value_XLarge2,
	Value_XXLarge2,
	Value_Italic2,
	Value_Oblique2,
	Value_Bold2,
	Value_Underline2,
	Value_Overline2,
	Value_LineThrough2,
	Value_Sub2,
	Value_Super2,
	Value_Left2,
	Value_Right2,
	Value_Top2,
	Value_Bottom2,
	Value_Center2,
	Value_Native2,
	Value_Solid2,
	Value_Dotted2,
	Value_Dashed2,
	Value_DotDash2,
	Value_DotDotDash2,
	Value_Double2,
	Value_Groove2,
	Value_Ridge2,
	Value_Inset2,
	Value_Outset2,
	Value_Wave2,
	Value_Middle2,
	Value_Auto2,
	Value_Always2,
	Value_None2,
	Value_Transparent2,
	Value_Disc2,
	Value_Circle2,
	Value_Square2,
	Value_Decimal2,
	Value_LowerAlpha2,
	Value_UpperAlpha2,
	Value_LowerRoman2,
	Value_UpperRoman2,
	Value_SmallCaps2,
	Value_Uppercase2,
	Value_Lowercase2,

	/* keep these in same order as QPalette::ColorRole */
	Value_FirstColorRole2,
	Value_WindowText2 = Value_FirstColorRole2,
	Value_Button2,
	Value_Light2,
	Value_Midlight2,
	Value_Dark2,
	Value_Mid2,
	Value_Text2,
	Value_BrightText2,
	Value_ButtonText2,
	Value_Base2,
	Value_Window2,
	Value_Shadow2,
	Value_Highlight2,
	Value_HighlightedText2,
	Value_Link2,
	Value_LinkVisited2,
	Value_AlternateBase2,
	Value_LastColorRole2 = Value_AlternateBase2,

	Value_Disabled2,
	Value_Active2,
	Value_Selected2,
	Value_On2,
	Value_Off2,

	NumKnownValues2
};

enum BorderStyle2
{
	BorderStyle_Unknown2,
	BorderStyle_None2,
	BorderStyle_Dotted2,
	BorderStyle_Dashed2,
	BorderStyle_Solid2,
	BorderStyle_Double2,
	BorderStyle_DotDash2,
	BorderStyle_DotDotDash2,
	BorderStyle_Groove2,
	BorderStyle_Ridge2,
	BorderStyle_Inset2,
	BorderStyle_Outset2,
	BorderStyle_Native2,
	NumKnownBorderStyles2
};

enum Edge2
{
	TopEdge2,
	RightEdge2,
	BottomEdge2,
	LeftEdge2,
	NumEdges2
};

enum Corner2
{
	TopLeftCorner2,
	TopRightCorner2,
	BottomLeftCorner2,
	BottomRightCorner2
};

enum TileMode2
{
	TileMode_Unknown2,
	TileMode_Round2,
	TileMode_Stretch2,
	TileMode_Repeat2,
	NumKnownTileModes2
};

enum Repeat2
{
	Repeat_Unknown2,
	Repeat_None2,
	Repeat_X2,
	Repeat_Y2,
	Repeat_XY2,
	NumKnownRepeats2
};

enum Origin2
{
	Origin_Unknown2,
	Origin_Padding2,
	Origin_Border2,
	Origin_Content2,
	Origin_Margin2,
	NumKnownOrigins2
};

enum PositionMode2
{
	PositionMode_Unknown2,
	PositionMode_Static2,
	PositionMode_Relative2,
	PositionMode_Absolute2,
	PositionMode_Fixed2,
	NumKnownPositionModes2
};

enum Attachment2
{
	Attachment_Unknown2,
	Attachment_Fixed2,
	Attachment_Scroll2,
	NumKnownAttachments2
};

enum StyleFeature2
{
	StyleFeature_None2 = 0,
	StyleFeature_BackgroundColor2 = 1,
	StyleFeature_BackgroundGradient2 = 2,
	NumKnownStyleFeatures2 = 4
};

static inline bool isHexDigit2(const char c) noexcept
{
	return (c >= '0' && c <= '9')
		   || (c >= 'a' && c <= 'f')
		   || (c >= 'A' && c <= 'F')
		   ;
}

struct Value2
{
	enum Type2
	{
		Unknown2,
		Number2,
		Percentage2,
		Length2,
		String2,
		Identifier2,
		KnownIdentifier2,
		Uri2,
		Color2,
		Function2,
		TermOperatorSlash2,
		TermOperatorComma2
	};
	inline Value2() noexcept : type(Unknown2) { }
	Type2 type;
	QVariant variant;
	QString toString() const;
};

struct ColorData2
{
	ColorData2() noexcept : role(QPalette::NoRole), type(Invalid) {}
	ColorData2(const QColor& col) noexcept : color(col), role(QPalette::NoRole), type(Color) {}
	ColorData2(QPalette::ColorRole r) noexcept : role(r), type(Role) {}
	QColor color;
	QPalette::ColorRole role;
	enum { Invalid, Color, Role} type;
};

struct BrushData2
{
	BrushData2() noexcept : role(QPalette::NoRole), type(Invalid) {}
	BrushData2(const QBrush& br) : brush(br), role(QPalette::NoRole), type(Brush) {}
	BrushData2(QPalette::ColorRole r) : role(r), type(Role) {}
	QBrush brush;
	QPalette::ColorRole role;
	enum { Invalid, Brush, Role, DependsOnThePalette } type;
};

struct BackgroundData2
{
	BrushData2 brush;
	QString image;
	Repeat2 repeat;
	Qt::Alignment alignment;
};

struct LengthData2
{
	qreal number;
	enum { NONE, Px, Ex, Em } unit;
};

struct BorderData2
{
	LengthData2 width;
	BorderStyle2 style;
	BrushData2 color;
};


// 1. StyleRule - x:hover, y:clicked > z:checked { prop1: value1; prop2: value2; }
// 2. QVector<Selector> - x:hover, y:clicked z:checked
// 3. QVector<BasicSelector> - y:clicked z:checked
// 4. QVector<Declaration> - { prop1: value1; prop2: value2; }
// 5. Declaration - prop1: value1;

struct Declaration2
{
	struct DeclarationData2 : public QSharedData
	{
		inline DeclarationData2() : propertyId(UnknownProperty2), important(false) {}
		QString property;
		Property2 propertyId;
		QVector<Value2> values;
		QVariant parsed;
		bool important;
	};
	QExplicitlySharedDataPointer<DeclarationData2> d;
	inline Declaration2() : d(new DeclarationData2()) {}
	inline bool isEmpty() const { return d->property.isEmpty() && d->propertyId == UnknownProperty2; }

	// helper functions
	QColor colorValue(const QPalette& = QPalette()) const;
	void colorValues(QColor* c, const QPalette& = QPalette()) const;
	QBrush brushValue(const QPalette& = QPalette()) const;
	void brushValues(QBrush* c, const QPalette& = QPalette()) const;

	BorderStyle2 styleValue() const;
	void styleValues(BorderStyle2* s) const;

	Origin2 originValue() const;
	Repeat2 repeatValue() const;
	Qt::Alignment alignmentValue() const;
	PositionMode2 positionValue() const;
	Attachment2 attachmentValue() const;
	int  styleFeaturesValue() const;

	bool intValue(int* i, const char* unit = nullptr) const;
	bool realValue(qreal* r, const char* unit = nullptr) const;

	QSize sizeValue() const;
	QRect rectValue() const;
	QString uriValue() const;
	QIcon iconValue() const;

	void borderImageValue(QString* image, int* cuts, TileMode2* h, TileMode2* v) const;
};

const quint64 PseudoClass_Unknown          = Q_UINT64_C(0x0000000000000000);
const quint64 PseudoClass_Enabled          = Q_UINT64_C(0x0000000000000001);
const quint64 PseudoClass_Disabled         = Q_UINT64_C(0x0000000000000002);
const quint64 PseudoClass_Pressed          = Q_UINT64_C(0x0000000000000004);
const quint64 PseudoClass_Focus            = Q_UINT64_C(0x0000000000000008);
const quint64 PseudoClass_Hover            = Q_UINT64_C(0x0000000000000010);
const quint64 PseudoClass_Checked          = Q_UINT64_C(0x0000000000000020);
const quint64 PseudoClass_Unchecked        = Q_UINT64_C(0x0000000000000040);
const quint64 PseudoClass_Indeterminate    = Q_UINT64_C(0x0000000000000080);
const quint64 PseudoClass_Unspecified      = Q_UINT64_C(0x0000000000000100);
const quint64 PseudoClass_Selected         = Q_UINT64_C(0x0000000000000200);
const quint64 PseudoClass_Horizontal       = Q_UINT64_C(0x0000000000000400);
const quint64 PseudoClass_Vertical         = Q_UINT64_C(0x0000000000000800);
const quint64 PseudoClass_Window           = Q_UINT64_C(0x0000000000001000);
const quint64 PseudoClass_Children         = Q_UINT64_C(0x0000000000002000);
const quint64 PseudoClass_Sibling          = Q_UINT64_C(0x0000000000004000);
const quint64 PseudoClass_Default          = Q_UINT64_C(0x0000000000008000);
const quint64 PseudoClass_First            = Q_UINT64_C(0x0000000000010000);
const quint64 PseudoClass_Last             = Q_UINT64_C(0x0000000000020000);
const quint64 PseudoClass_Middle           = Q_UINT64_C(0x0000000000040000);
const quint64 PseudoClass_OnlyOne          = Q_UINT64_C(0x0000000000080000);
const quint64 PseudoClass_PreviousSelected = Q_UINT64_C(0x0000000000100000);
const quint64 PseudoClass_NextSelected     = Q_UINT64_C(0x0000000000200000);
const quint64 PseudoClass_Flat             = Q_UINT64_C(0x0000000000400000);
const quint64 PseudoClass_Left             = Q_UINT64_C(0x0000000000800000);
const quint64 PseudoClass_Right            = Q_UINT64_C(0x0000000001000000);
const quint64 PseudoClass_Top              = Q_UINT64_C(0x0000000002000000);
const quint64 PseudoClass_Bottom           = Q_UINT64_C(0x0000000004000000);
const quint64 PseudoClass_Exclusive        = Q_UINT64_C(0x0000000008000000);
const quint64 PseudoClass_NonExclusive     = Q_UINT64_C(0x0000000010000000);
const quint64 PseudoClass_Frameless        = Q_UINT64_C(0x0000000020000000);
const quint64 PseudoClass_ReadOnly         = Q_UINT64_C(0x0000000040000000);
const quint64 PseudoClass_Active           = Q_UINT64_C(0x0000000080000000);
const quint64 PseudoClass_Closable         = Q_UINT64_C(0x0000000100000000);
const quint64 PseudoClass_Movable          = Q_UINT64_C(0x0000000200000000);
const quint64 PseudoClass_Floatable        = Q_UINT64_C(0x0000000400000000);
const quint64 PseudoClass_Minimized        = Q_UINT64_C(0x0000000800000000);
const quint64 PseudoClass_Maximized        = Q_UINT64_C(0x0000001000000000);
const quint64 PseudoClass_On               = Q_UINT64_C(0x0000002000000000);
const quint64 PseudoClass_Off              = Q_UINT64_C(0x0000004000000000);
const quint64 PseudoClass_Editable         = Q_UINT64_C(0x0000008000000000);
const quint64 PseudoClass_Item             = Q_UINT64_C(0x0000010000000000);
const quint64 PseudoClass_Closed           = Q_UINT64_C(0x0000020000000000);
const quint64 PseudoClass_Open             = Q_UINT64_C(0x0000040000000000);
const quint64 PseudoClass_EditFocus        = Q_UINT64_C(0x0000080000000000);
const quint64 PseudoClass_Alternate        = Q_UINT64_C(0x0000100000000000);
// The Any specifier is never generated, but can be used as a wildcard in searches.
const quint64 PseudoClass_Any              = Q_UINT64_C(0x0000ffffffffffff);
const int NumPseudos = 45;

struct Pseudo2
{
	Pseudo2() : type(0), negated(false) { }
	quint64 type;
	QString name;
	QString function;
	bool negated;
};

struct AttributeSelector2
{
	enum ValueMatchType2
	{
		NoMatch2,
		MatchEqual2,
		MatchContains2,
		MatchBeginsWith2
	};
	inline AttributeSelector2() : valueMatchCriterium(NoMatch2) {}

	QString name;
	QString value;
	ValueMatchType2 valueMatchCriterium;
};

struct BasicSelector2
{
	inline BasicSelector2() : relationToNext(NoRelation2) {}

	enum Relation2
	{
		NoRelation2,
		MatchNextSelectorIfAncestor2,
		MatchNextSelectorIfParent2,
		MatchNextSelectorIfPreceeds2
	};

	QString elementName;

	QStringList ids;
	QVector<Pseudo2> pseudos;
	QVector<AttributeSelector2> attributeSelectors;

	Relation2 relationToNext;
};

struct Selector2
{
	QVector<BasicSelector2> basicSelectors;
	int specificity() const;
	quint64 pseudoClass(quint64* negated = nullptr) const;
	QString pseudoElement() const;
};

struct StyleRule2;
struct MediaRule2;
struct PageRule2;
struct ImportRule2;

struct ValueExtractor2
{
	ValueExtractor2(const QVector<Declaration2>& declarations, const QPalette& = QPalette());

	bool extractFont(QFont* font, int* fontSizeAdjustment);
	bool extractBackground(QBrush*, QString*, Repeat2*, Qt::Alignment*, QCss2::Origin2*, QCss2::Attachment2*,
						   QCss2::Origin2*);
	bool extractGeometry(int* w, int* h, int* minw, int* minh, int* maxw, int* maxh);
	bool extractPosition(int* l, int* t, int* r, int* b, QCss2::Origin2*, Qt::Alignment*,
						 QCss2::PositionMode2*, Qt::Alignment*);
	bool extractBox(int* margins, int* paddings, int* spacing = nullptr);
	bool extractBorder(int* borders, QBrush* colors, BorderStyle2* Styles, QSize* radii);
	bool extractOutline(int* borders, QBrush* colors, BorderStyle2* Styles, QSize* radii, int* offsets);
	bool extractPalette(QBrush* fg, QBrush* sfg, QBrush* sbg, QBrush* abg);
	int  extractStyleFeatures();
	bool extractImage(QIcon* icon, Qt::Alignment* a, QSize* size);

	int lengthValue(const Declaration2& decl);

	private:
	void extractFont();
	void borderValue(const Declaration2& decl, int* width, QCss2::BorderStyle2* style, QBrush* color);
	LengthData2 lengthValue(const Value2& v);
	void lengthValues(const Declaration2& decl, int* m);
	QSize sizeValue(const Declaration2& decl);
	void sizeValues(const Declaration2& decl, QSize* radii);

	QVector<Declaration2> declarations;
	QFont f;
	int adjustment;
	int fontExtracted;
	QPalette pal;
};

struct StyleRule2
{
	StyleRule2() : order(0) { }
	QVector<Selector2> selectors;
	QVector<Declaration2> declarations;
	int order;
};

struct MediaRule2
{
	QStringList media;
	QVector<StyleRule2> styleRules;
};

struct PageRule2
{
	QString selector;
	QVector<Declaration2> declarations;
};

struct ImportRule2
{
	QString href;
	QStringList media;
};

enum StyleSheetOrigin2
{
	StyleSheetOrigin_Unspecified,
	StyleSheetOrigin_UserAgent,
	StyleSheetOrigin_User,
	StyleSheetOrigin_Author,
	StyleSheetOrigin_Inline
};

struct StyleSheet2
{
	StyleSheet2() : origin(StyleSheetOrigin_Unspecified), depth(0) { }
	QVector<StyleRule2> styleRules;  //only contains rules that are not indexed
	QVector<MediaRule2> mediaRules;
	QVector<PageRule2> pageRules;
	QVector<ImportRule2> importRules;
	StyleSheetOrigin2 origin;
	int depth; // applicable only for inline style sheets
	QMultiHash<QString, StyleRule2> nameIndex;
	QMultiHash<QString, StyleRule2> idIndex;
	void buildIndexes(Qt::CaseSensitivity nameCaseSensitivity = Qt::CaseSensitivity::CaseSensitive);
};

class StyleSelector2
{
public:
	StyleSelector2() : nameCaseSensitivity(Qt::CaseSensitivity::CaseSensitive)  {}
	virtual ~StyleSelector2();

	union NodePtr2
	{
		void* ptr;
		int id;
	};

	QVector<StyleRule2> styleRulesForNode(NodePtr2 node);
	QVector<Declaration2> declarationsForNode(NodePtr2 node, const char* extraPseudo = nullptr);

	virtual bool nodeNameEquals(NodePtr2 node, const QString& nodeName) const;
	virtual QString attribute(NodePtr2 node, const QString& name) const = 0;
	virtual bool hasAttributes(NodePtr2 node) const = 0;
	virtual QStringList nodeIds(NodePtr2 node) const;
	virtual QStringList nodeNames(NodePtr2 node) const = 0;
	virtual bool isNullNode(NodePtr2 node) const = 0;
	virtual NodePtr2 parentNode(NodePtr2 node) const = 0;
	virtual NodePtr2 previousSiblingNode(NodePtr2 node) const = 0;
	virtual NodePtr2 duplicateNode(NodePtr2 node) const = 0;
	virtual void freeNode(NodePtr2 node) const = 0;

	QVector<StyleSheet2> styleSheets;
	QString medium;
	Qt::CaseSensitivity nameCaseSensitivity;
private:
	void matchRule(NodePtr2 node, const StyleRule2& rules, StyleSheetOrigin2 origin,
				   int depth, QMap<uint, StyleRule2>* weightedRules);
	bool selectorMatches(const Selector2& rule, NodePtr2 node);
	bool basicSelectorMatches(const BasicSelector2& rule, NodePtr2 node);
};

enum TokenType2
{
	NONE,
	S,
	CDO,
	CDC,
	INCLUDES,
	DASHMATCH,
	LBRACE,
	PLUS,
	GREATER,
	COMMA,
	STRING,
	INVALID,
	IDENT,
	HASH,
	ATKEYWORD_SYM,
	EXCLAMATION_SYM,
	LENGTH,
	PERCENTAGE,
	NUMBER,
	FUNCTION,
	COLON,
	SEMICOLON,
	RBRACE,
	SLASH,
	MINUS,
	DOT,
	STAR,
	LBRACKET,
	RBRACKET,
	EQUAL,
	LPAREN,
	RPAREN,
	OR
};

struct Symbol2
{
	inline Symbol2() : token(TokenType2::NONE), start(0), len(-1) {}
	TokenType2 token;
	QString text;
	int start, len;
	QString lexem() const;
};

class Scanner2
{
public:
	static QString preprocess(const QString& input, bool* hasEscapeSequences = nullptr);
	static void scan(const QString& preprocessedInput, QVector<Symbol2>* symbols);
};

class Parser2
{
public:
	Parser2() noexcept;
	explicit Parser2(const QString& css, bool file = false);

	void init(const QString& css, bool file = false);
	bool parse(StyleSheet2* styleSheet, Qt::CaseSensitivity nameCaseSensitivity = Qt::CaseSensitivity::CaseSensitive);
	Symbol2 errorSymbol();

	bool parseImport(ImportRule2* importRule);
	bool parseMedia(MediaRule2* mediaRule);
	bool parseMedium(QStringList* media);
	bool parsePage(PageRule2* pageRule);
	bool parsePseudoPage(QString* selector);
	bool parseNextOperator(Value2* value);
	bool parseCombinator(BasicSelector2::Relation2* relation);
	bool parseProperty(Declaration2* decl);
	bool parseRuleset(StyleRule2* styleRule);
	bool parseSelector(Selector2* sel);
	bool parseSimpleSelector(BasicSelector2* basicSel);
	bool parseClass(QString* name);
	bool parseElementName(QString* name);
	bool parseAttrib(AttributeSelector2* attr);
	bool parsePseudo(Pseudo2* pseudo);
	bool parseNextDeclaration(Declaration2* declaration);
	bool parsePrio(Declaration2* declaration);
	bool parseExpr(QVector<Value2>* values);
	bool parseTerm(Value2* value);
	bool parseFunction(QString* name, QString* args);
	bool parseHexColor(QColor* col);
	bool testAndParseUri(QString* uri);

	inline bool testRuleset() { return testSelector(); }
	inline bool testSelector() { return testSimpleSelector(); }
	inline bool parseNextSelector(Selector2* sel) { if (!testSelector()) return recordError(); return parseSelector(sel); }
	bool testSimpleSelector();
	inline bool parseNextSimpleSelector(BasicSelector2* basicSel) { if (!testSimpleSelector()) return recordError(); return parseSimpleSelector(basicSel); }
	inline bool testElementName() { return test(TokenType2::IDENT) || test(TokenType2::STAR); }
	inline bool testClass() { return test(TokenType2::DOT); }
	inline bool testAttrib() { return test(TokenType2::LBRACKET); }
	inline bool testPseudo() { return test(TokenType2::COLON); }
	inline bool testMedium() { return test(TokenType2::IDENT); }
	inline bool parseNextMedium(QStringList* media) { if (!testMedium()) return recordError(); return parseMedium(media); }
	inline bool testPseudoPage() { return test(TokenType2::COLON); }
	inline bool testImport() { return testTokenAndEndsWith(TokenType2::ATKEYWORD_SYM, QLatin1String("import")); }
	inline bool testMedia() { return testTokenAndEndsWith(TokenType2::ATKEYWORD_SYM, QLatin1String("media")); }
	inline bool testPage() { return testTokenAndEndsWith(TokenType2::ATKEYWORD_SYM, QLatin1String("page")); }
	inline bool testCombinator() { return test(TokenType2::PLUS) || test(TokenType2::GREATER) || test(TokenType2::S); }
	inline bool testProperty() { return test(TokenType2::IDENT); }
	bool testTerm();
	inline bool testExpr() { return testTerm(); }
	inline bool parseNextExpr(QVector<Value2>* values) { if (!testExpr()) return recordError(); return parseExpr(values); }
	bool testPrio();
	inline bool testHexColor() { return test(TokenType2::HASH); }
	inline bool testFunction() { return test(TokenType2::FUNCTION); }
	inline bool parseNextFunction(QString* name, QString* args) { if (!testFunction()) return recordError(); return parseFunction(name, args); }

	inline bool lookupElementName() const { return lookup() == TokenType2::IDENT || lookup() == TokenType2::STAR; }

	inline void skipSpace() { while (test(TokenType2::S)) {}; }

	inline bool hasNext() const { return index < symbols.count(); }
	inline TokenType2 next() { return symbols.at(index++).token; }
	bool next(TokenType2 t);
	bool test(TokenType2 t) noexcept;
	inline void prev() { index--; }
	inline const Symbol2& symbol() const { return symbols.at(index - 1); }
	inline QString lexem() const { return symbol().lexem(); }
	QString unquotedLexem() const;
	QString lexemUntil(TokenType2 t);
	bool until(TokenType2 target, TokenType2 target2 = TokenType2::NONE) noexcept;
	inline TokenType2 lookup() const
	{
		return (index - 1) < symbols.count() ? symbols.at(index - 1).token : TokenType2::NONE;
	}

	bool testTokenAndEndsWith(TokenType2 t, QLatin1String str);

	inline bool recordError() { errorIndex = index; return false; }

	QVector<Symbol2> symbols;
	int index;
	int errorIndex;
	bool hasEscapeSequences;
	QString sourcePath;
};

} // namespace QCss2

Q_DECLARE_METATYPE( QCss2::BackgroundData2 )
Q_DECLARE_METATYPE( QCss2::LengthData2 )
Q_DECLARE_METATYPE( QCss2::BorderData2 )
