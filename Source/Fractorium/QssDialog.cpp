#include "FractoriumPch.h"
#include "QssDialog.h"
#include "ui_QssDialog.h"
#include "qcssscanner.h"

/// <summary>
/// The code in this file did not originate in Fractorium.
/// It was taken either in whole or in part from the source code
/// of Qt Creator. Their license applies.
/// </summary>

/// <summary>
/// Comparison for sorting object names.
/// Strings not starting with the letter 'Q' take precedence.
/// This has the effect of putting custom derived classes first before
/// all Q* classes.
/// </summary>
/// <param name="s1">The first string to compare</param>
/// <param name="s2">The second string to compare</param>
/// <returns>True if s1 < s2 with special rules for 'Q' taken into account.</returns>
bool CaseInsensitiveLessThanQ(const QString& s1, const QString& s2)
{
	if (s1.length() && s2.length())
	{
		if (s1[0] == 'Q' && s2[0] == 'Q')
			return s1.toLower() < s2.toLower();
		else if (s1[0] == 'Q')
			return false;
		else if (s2[0] == 'Q')
			return true;
		else
			return s1.toLower() < s2.toLower();
	}

	return false;
}

/// <summary>
/// Construct a QssDialog.
/// This manually constructs much of the menu GUI via code rather
/// than in the designer.
/// </summary>
/// <param name="parent">The main Fractorium window.</param>
QssDialog::QssDialog(Fractorium* parent) :
	QDialog(parent),
    m_AddColorAction(new QAction(tr("Add Color"), this)),
    m_AddGeomAction(new QAction(tr("Add Geometry"), this)),
    m_AddBorderAction(new QAction(tr("Add Border"), this)),
    m_AddFontAction(new QAction(tr("Add Font..."), this)),
    m_AddStyleAction(new QAction(tr("Set Theme"), this)),
    m_Parent(parent),
    ui(new Ui::QssDialog)
{
	ui->setupUi(this);
	m_LastStyle = m_Parent->styleSheet();
	setWindowTitle("QSS Editor - default.qss");
	connect(ui->QssEdit, SIGNAL(textChanged()), this, SLOT(SlotTextChanged()));
	QToolBar* toolBar = new QToolBar(this);
	QMenu* colorActionMenu = new QMenu(this);
	QMenu* geomActionMenu = new QMenu(this);
	QMenu* borderActionMenu = new QMenu(this);
	QMenu* styleActionMenu = new QMenu(this);
	(m_ColorActionMapper = new QSignalMapper(this))->setMapping(m_AddColorAction, QString());
	(m_GeomActionMapper = new QSignalMapper(this))->setMapping(m_AddGeomAction, QString());
	(m_BorderActionMapper = new QSignalMapper(this))->setMapping(m_AddBorderAction, QString());
	(m_StyleActionMapper = new QSignalMapper(this))->setMapping(m_AddStyleAction, QString());
	connect(ui->QssLoadButton, SIGNAL(clicked()), this, SLOT(LoadButton_clicked()), Qt::QueuedConnection);
	connect(ui->QssSaveButton, SIGNAL(clicked()), this, SLOT(SaveButton_clicked()), Qt::QueuedConnection);
	connect(ui->QssBasicButton, SIGNAL(clicked()), this, SLOT(BasicButton_clicked()), Qt::QueuedConnection);
	connect(ui->QssMediumButton, SIGNAL(clicked()), this, SLOT(MediumButton_clicked()), Qt::QueuedConnection);
	connect(ui->QssAdvancedButton, SIGNAL(clicked()), this, SLOT(AdvancedButton_clicked()), Qt::QueuedConnection);
	connect(m_AddFontAction, SIGNAL(triggered()), this, SLOT(SlotAddFont()));
	QVector<QPair<QString, QString>> colorVec;
	colorVec.reserve(12);
	colorVec.push_back(QPair<QString, QString>("color", ""));
	colorVec.push_back(QPair<QString, QString>("background-color", ""));
	colorVec.push_back(QPair<QString, QString>("alternate-background-color", ""));
	colorVec.push_back(QPair<QString, QString>("border-color", ""));
	colorVec.push_back(QPair<QString, QString>("border-top-color", ""));
	colorVec.push_back(QPair<QString, QString>("border-right-color", ""));
	colorVec.push_back(QPair<QString, QString>("border-bottom-color", ""));
	colorVec.push_back(QPair<QString, QString>("border-left-color", ""));
	colorVec.push_back(QPair<QString, QString>("gridline-color", ""));
	colorVec.push_back(QPair<QString, QString>("selection-color", ""));
	colorVec.push_back(QPair<QString, QString>("selection-background-color", ""));

	for (auto& c : colorVec)
	{
		auto colorAction = colorActionMenu->addAction(c.first);
		m_ColorMap[c.first] = c.second;
		connect(colorAction, SIGNAL(triggered()), m_ColorActionMapper, SLOT(map()));
		m_ColorActionMapper->setMapping(colorAction, c.first);
	}

	QVector<QPair<QString, QString>> geomVec;
	geomVec.reserve(12);
	geomVec.push_back(QPair<QString, QString>("width", "100px"));
	geomVec.push_back(QPair<QString, QString>("height", "50px"));
	geomVec.push_back(QPair<QString, QString>("spacing", "10"));
	geomVec.push_back(QPair<QString, QString>("padding", "3px"));
	geomVec.push_back(QPair<QString, QString>("padding-top", "3px"));
	geomVec.push_back(QPair<QString, QString>("padding-right", "3px"));
	geomVec.push_back(QPair<QString, QString>("padding-bottom", "3px"));
	geomVec.push_back(QPair<QString, QString>("padding-left", "3px"));
	geomVec.push_back(QPair<QString, QString>("margin", "3px"));
	geomVec.push_back(QPair<QString, QString>("margin-top", "3px"));
	geomVec.push_back(QPair<QString, QString>("margin-right", "3px"));
	geomVec.push_back(QPair<QString, QString>("margin-bottom", "3px"));
	geomVec.push_back(QPair<QString, QString>("margin-left", "3px"));

	for (auto& g : geomVec)
	{
		auto geomAction = geomActionMenu->addAction(g.first);
		m_GeomMap[g.first] = g.second;
		connect(geomAction, SIGNAL(triggered()), m_GeomActionMapper, SLOT(map()));
		m_GeomActionMapper->setMapping(geomAction, g.first);
	}

	QVector<QPair<QString, QString>> borderVec;
	borderVec.reserve(8);
	borderVec.push_back(QPair<QString, QString>("border", "1px solid black"));
	borderVec.push_back(QPair<QString, QString>("border-top", "1px inset black"));
	borderVec.push_back(QPair<QString, QString>("border-right", "1px outset black"));
	borderVec.push_back(QPair<QString, QString>("border-bottom", "1px ridge black"));
	borderVec.push_back(QPair<QString, QString>("border-left", "1px groove black"));
	borderVec.push_back(QPair<QString, QString>("border-style", "double"));
	borderVec.push_back(QPair<QString, QString>("border-width", "1px"));
	borderVec.push_back(QPair<QString, QString>("border-radius", "10px"));

	for (auto& b : borderVec)
	{
		auto borderAction = borderActionMenu->addAction(b.first);
		m_BorderMap[b.first] = b.second;
		connect(borderAction, SIGNAL(triggered()), m_BorderActionMapper, SLOT(map()));
		m_BorderActionMapper->setMapping(borderAction, b.first);
	}

	auto styles = QStyleFactory::keys();

	for (auto& s : styles)
	{
		auto styleAction = styleActionMenu->addAction(s);
		m_StyleMap[s] = s;
		connect(styleAction, SIGNAL(triggered()), m_StyleActionMapper, SLOT(map()));
		m_StyleActionMapper->setMapping(styleAction, s);
	}

	connect(m_ColorActionMapper, SIGNAL(mappedString(const QString&)), this, SLOT(SlotAddColor(const QString&)));
	connect(m_GeomActionMapper, SIGNAL(mappedString(const QString&)), this, SLOT(SlotAddGeom(const QString&)));
	connect(m_BorderActionMapper, SIGNAL(mappedString(const QString&)), this, SLOT(SlotAddBorder(const QString&)));
	connect(m_StyleActionMapper, SIGNAL(mappedString(const QString&)), this, SLOT(SlotSetTheme(const QString&)));
	m_AddColorAction->setMenu(colorActionMenu);
	m_AddGeomAction->setMenu(geomActionMenu);
	m_AddBorderAction->setMenu(borderActionMenu);
	m_AddStyleAction->setMenu(styleActionMenu);
	toolBar->addAction(m_AddColorAction);
	toolBar->addAction(m_AddGeomAction);
	toolBar->addAction(m_AddBorderAction);
	toolBar->addAction(m_AddFontAction);
	toolBar->addAction(m_AddStyleAction);
	ui->verticalLayout->insertWidget(0, toolBar);
	ui->QssEdit->setFocus();
	m_ApplyTimer = new QTimer(this);
	m_ApplyTimer->setSingleShot(true);
	m_ApplyTimer->setInterval(1000);
	connect(m_ApplyTimer, SIGNAL(timeout()), this, SLOT(SlotApplyCss()));
}

/// <summary>
/// Destructor that stops the apply timer and deletes the ui.
/// </summary>
QssDialog::~QssDialog()
{
	m_ApplyTimer->stop();
	delete ui;
}

/// <summary>
/// Thin wrapper around getting the text from the main text box as plain text.
/// </summary>
/// <returns>The plain text of the main text box</returns>
QString QssDialog::Text() const
{
	return ui->QssEdit->toPlainText();
}

/// <summary>
/// Thin wrapper around setting the text of the main text box.
/// </summary>
/// <param name="t">The text to set</param>
void QssDialog::SetText(const QString& t)
{
	ui->QssEdit->setText(t);
}

/// <summary>
/// Get the class names of all objects in the application.
/// This only makes one entry for each class type.
/// It will also optionally return the object names as well for advanced QSS editing.
/// </summary>
/// <param name="includeObjectNames">Whether to get the individual object names as well</param>
/// <returns>A list of all class names with optional entries for each individual object</returns>
QList<QString> QssDialog::GetClassNames(bool includeObjectNames)
{
	QSet<QString> classNames;
	QList<QList<QString>> dialogClassNames;
	auto widgetList = m_Parent->findChildren<QWidget*>();

	for (int i = 0; i < widgetList.size(); i++)
	{
		auto classAndName = QString(widgetList[i]->metaObject()->className());

		if (!includeObjectNames)
		{
			classNames.insert(classAndName);
		}
		else
		{
			auto dlg = qobject_cast<QDialog*>(widgetList[i]);

			if (dlg)//Dialogs only nest one level deep, so no need for generalized recursion.
			{
				QSet<QString> dlgSet;
				auto dlgWidgetList = dlg->findChildren<QWidget*>();//Find all children of the dialog.
				dlgSet.insert(classAndName);//Add the basic dialog class name, opening curly brace will be added later.
				classAndName += " ";

                for (int j = 0; j < dlgWidgetList.size(); j++)
				{
                    auto dlgClassAndName = classAndName + QString(dlgWidgetList[j]->metaObject()->className());
					dlgSet.insert(dlgClassAndName);

                    if (!dlgWidgetList[j]->objectName().isEmpty())//Add the class with object name for individual control customization.
					{
                        dlgClassAndName += "#" + dlgWidgetList[j]->objectName();
						dlgSet.insert(dlgClassAndName);
					}
				}

				QList<QString> dlgList(dlgSet.begin(), dlgSet.end());//Convert set to list and sort.
				std::sort(dlgList.begin(), dlgList.end(), CaseInsensitiveLessThanQ);
				dialogClassNames.push_back(dlgList);//Add this to the full list after sorting at the end.
			}
			else if (GetAllParents<QDialog*>(widgetList[i]).empty())//Skip widgets on dialogs, they are added above.
			{
				classNames.insert(classAndName);//Add the basic class name.

				if (!widgetList[i]->objectName().isEmpty())//Add the class with object name for individual control customization.
				{
					classAndName += "#" + widgetList[i]->objectName();
					classNames.insert(classAndName);
				}
			}
		}
	}

	QList<QString> l(classNames.begin(), classNames.end());//Convert set to list and sort.
	std::sort(l.begin(), l.end(), CaseInsensitiveLessThanQ);

	for (auto& d : dialogClassNames)
		l.append(d);

	return l;
}

/// <summary>
/// Determines whether the passed in stylesheet text is valid.
/// If the initial parse fails, a second attempt is made by wrapping the entire
/// text in curly braces.
/// </summary>
/// <param name="styleSheet">The stylesheet text to analyze.</param>
/// <returns>True if valid, else false.</returns>
bool QssDialog::IsStyleSheetValid(const QString& styleSheet)
{
	QCss::Parser parser(styleSheet);
	QCss::StyleSheet sheet;

	if (parser.parse(&sheet))
		return true;

	QString fullSheet = QStringLiteral("* { ");
	fullSheet += styleSheet;
	fullSheet += QLatin1Char('}');
	QCss::Parser parser2(fullSheet);
	return parser2.parse(&sheet);
}

/// <summary>
/// Save the current stylesheet text to default.qss.
/// Also save the selected theme to the settings.
/// Called when the user clicks ok.
/// Not called if cancelled or closed with the X.
/// </summary>
void QssDialog::accept()
{
	if (m_Theme)
		m_Parent->m_Settings->Theme(m_Theme->objectName());

	SaveAsDefault();
	QDialog::accept();
}

/// <summary>
/// Restore the stylesheet and theme to what it was when the dialog was opened.
/// Called when the user clicks cancel or closes with the X.
/// </summary>
void QssDialog::reject()
{
	if (!m_LastStyle.isEmpty())
		m_Parent->setStyleSheet(m_LastStyle);

	if (m_LastTheme)
	{
		m_Parent->setStyle(m_LastTheme);
		m_Parent->m_Settings->Theme(m_LastTheme->objectName());
	}

	m_Parent->m_Controller->FillVariationTreeWithCurrentXform();
	QDialog::reject();
}

/// <summary>
/// Shows the event.
/// </summary>
/// <param name="e">The e.</param>
void QssDialog::showEvent(QShowEvent* e)
{
	if (m_Parent)
	{
		m_LastStyle = m_Parent->styleSheet();
		m_LastTheme = m_Parent->m_Theme;//The style() member cannot be relied upon, it is *not* the same object passed to setStyle();
		SetText(m_LastStyle);
	}

	QDialog::showEvent(e);
}

/// <summary>
/// Start the timer which will analyze and apply the current stylesheet text.
/// Each successive keystroke will reset the timer if it has not timed out yet.
/// This is only called when the dialog is visible because it seems to be spurriously
/// called on startup.
/// Called when the user changes the text in main text box.
/// </summary>
void QssDialog::SlotTextChanged()
{
	if (isVisible())//Sometimes this fires even though the window is not shown yet.
		m_ApplyTimer->start();
}

/// <summary>
///	Add a color string to the stylesheet text.
/// Called when the user clicks the add color menu.
/// </summary>
/// <param name="s">The color string selector to add</param>
void QssDialog::SlotAddColor(const QString& s)
{
	const QColor color = QColorDialog::getColor(0xffffffff, this, QString(), QColorDialog::ShowAlphaChannel);

	if (!color.isValid())
		return;

	QString colorStr;

	if (color.alpha() == 255)
	{
		colorStr = QString(QStringLiteral("rgb(%1, %2, %3)")).arg(
					   color.red()).arg(color.green()).arg(color.blue());
	}
	else
	{
		colorStr = QString(QStringLiteral("rgba(%1, %2, %3, %4)")).arg(
					   color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
	}

	InsertCssProperty(s, colorStr);
}

/// <summary>
/// Adds a geometry string to the stylesheet text.
/// </summary>
/// <param name="s">The geometry string to add</param>
void QssDialog::SlotAddGeom(const QString& s)
{
	auto val = m_GeomMap[s];
	InsertCssProperty(s, val);
}

/// <summary>
/// Adds a border string to the stylesheet text.
/// </summary>
/// <param name="s">The border string to add</param>
void QssDialog::SlotAddBorder(const QString& s)
{
	auto val = m_BorderMap[s];
	InsertCssProperty(s, val);
}

/// <summary>
/// Set the theme to the user selection.
/// Called when the user selects an item on the theme combo box.
/// </summary>
/// <param name="s">The s.</param>
void QssDialog::SlotSetTheme(const QString& s)
{
	if (auto theme = QStyleFactory::create(s))
	{
		m_Theme = theme;
		m_Parent->setStyle(m_Theme);
	}
}

/// <summary>
/// Add a font string.
/// Called when the user clicks the add font menu button.
/// </summary>
void QssDialog::SlotAddFont()
{
	bool ok;
	auto font = QFontDialog::getFont(&ok, this);

	if (ok)
	{
		QString fontStr;

		if (font.weight() != QFont::Normal)
		{
			fontStr += QString::number(font.weight());
			fontStr += QLatin1Char(' ');
		}

		switch (font.style())
		{
			case QFont::StyleItalic:
				fontStr += QStringLiteral("italic ");
				break;

			case QFont::StyleOblique:
				fontStr += QStringLiteral("oblique ");
				break;

            case QFont::StyleNormal:
			default:
				break;
		}

		fontStr += QString::number(font.pointSize());
		fontStr += QStringLiteral("pt \"");
		fontStr += font.family();
		fontStr += QLatin1Char('"');
		InsertCssProperty(QStringLiteral("font"), fontStr);
	}
}

/// <summary>
/// Check if the current stylesheet is valid and apply it if so.
/// Also indicate via label whether it was valid.
/// </summary>
void QssDialog::SlotApplyCss()
{
	auto label = ui->QssValidityLabel;
	auto style = Text();
	const bool valid = IsStyleSheetValid(style);
	ui->QssButtonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);

	if (valid)
	{
		label->setText(tr("Valid Style Sheet"));
		label->setStyleSheet(QStringLiteral("color: green"));
		m_Parent->setStyleSheet(style);
		m_Parent->m_Controller->FillVariationTreeWithCurrentXform();
	}
	else
	{
		label->setText(tr("Invalid Style Sheet"));
		label->setStyleSheet(QStringLiteral("color: red"));
	}
}

/// <summary>
/// Load a stylesheet from disk.
/// Called when the user clicks the load button.
/// </summary>
void QssDialog::LoadButton_clicked()
{
	string s;
	auto f = OpenFile();

	if (!f.isEmpty() && ReadFile(f.toStdString().c_str(), s) && !s.empty())
		SetText(QString::fromStdString(s));

	setWindowTitle("QSS Editor - " + f);
}

/// <summary>
/// Save the stylesheet to disk.
/// Called when the user clicks the save button.
/// The user cannot save to default.qss, as it's a special placeholder.
/// When they exit the dialog by clicking OK, the currently displayed stylesheet
/// will be saved to default.qss.
/// </summary>
void QssDialog::SaveButton_clicked()
{
	auto path = SaveFile();

	if (path.toLower().endsWith("default.qss"))
	{
		QMessageBox::critical(this, "File save error", "Stylesheet cannot be saved to default.qss. Save it to a different file name, then exit the dialog by clicking OK which will set it as the default.");
		return;
	}

	if (!path.isEmpty())
	{
		ofstream of(path.toStdString());
		string s = Text().toStdString();

		if (of.is_open())
			of << s;
		else
			QMessageBox::critical(this, "File save error", "Failed to save " + path + ", style will not be set as default");
	}
}

/// <summary>
/// Save the stylesheet to the default.qss on disk.
/// This will be loaded the next time Fractorium runs.
/// Called when the user clicks ok.
/// </summary>
void QssDialog::SaveAsDefault()
{
	auto path = m_Parent->m_SettingsPath + "/default.qss";
	ofstream of(path.toStdString());
	auto s = Text().toStdString();

	if (of.is_open())
		of << s;
	else
		QMessageBox::critical(this, "File save error", "Failed to save " + path + ", style will not be set as default");
}

/// <summary>
/// Fill the main text box with the most basic style.
/// Called when the Basic button is clicked.
/// </summary>
void QssDialog::BasicButton_clicked()
{
	SetText(BaseStyle());
	setWindowTitle("QSS Editor");
}

/// <summary>
/// Fill the main text box with a medium specificity style.
/// This will expose all control types in the application.
/// Called when the Medium button is clicked.
/// </summary>
void QssDialog::MediumButton_clicked()
{
	QString str = BaseStyle();
	auto names = GetClassNames(false);

	for (auto& it : names)
		str += it + QString("\n{\n\t\n}\n\n");

	SetText(str);
	setWindowTitle("QSS Editor");
}

/// <summary>
/// Fill the main text box with the most advanced style.
/// This will expose all control types in the application as well as their named instances.
/// Called when the Advanced button is clicked.
/// </summary>
void QssDialog::AdvancedButton_clicked()
{
	QString str = BaseStyle();
	auto names = GetClassNames(true);

	for (auto& it : names)
		str += it + QString("\n{\n\t\n}\n\n");

	SetText(str);
	setWindowTitle("QSS Editor");
}

/// <summary>
/// Insert a CSS property.
/// This is called whenever the user inserts a value via the menus.
/// </summary>
/// <param name="name">The name of the property to insert</param>
/// <param name="value">The value of the property to insert</param>
void QssDialog::InsertCssProperty(const QString& name, const QString& value)
{
	auto editor = ui->QssEdit;
	auto cursor = editor->textCursor();

	if (!name.isEmpty())
	{
		cursor.beginEditBlock();
		cursor.removeSelectedText();
		cursor.movePosition(QTextCursor::EndOfLine);
		//Simple check to see if we're in a selector scope.
		const QTextDocument* doc = editor->document();
		const QTextCursor closing = doc->find(QStringLiteral("}"), cursor, QTextDocument::FindBackward);
		const QTextCursor opening = doc->find(QStringLiteral("{"), cursor, QTextDocument::FindBackward);
		const bool inSelector = !opening.isNull() && (closing.isNull() ||
								closing.position() < opening.position());
		QString insertion;

		//Reasonable attempt at positioning things correctly. This can and often is wrong, but is sufficient for our purposes.
		if (editor->textCursor().block().length() != 1 && !editor->textCursor().block().text().isEmpty())
			insertion += QLatin1Char('\n');

		if (inSelector && editor->textCursor().block().text() != "\t")
			insertion += QLatin1Char('\t');

		insertion += name;
		insertion += QStringLiteral(": ");
		insertion += value;
		insertion += QLatin1Char(';');
		cursor.insertText(insertion);
		cursor.endEditBlock();
	}
	else
	{
		cursor.insertText(value);
	}
}

/// <summary>
/// Initial file dialog creation.
/// This will perform lazy instantiation since it takes a long time.
/// </summary>
void QssDialog::SetupFileDialog()
{
#ifndef __APPLE__

	if (!m_FileDialog)
	{
		m_FileDialog = new QFileDialog(this);
		m_FileDialog->setViewMode(QFileDialog::List);
		//m_FileDialog->setDirectory(m_Parent->m_SettingsPath);
		m_FileDialog->setDirectory("./");
		m_FileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
		m_FileDialog->setSidebarUrls(dynamic_cast<Fractorium*>(parent())->Urls());
	}

#endif
}

/// <summary>
/// Present a file open dialog and retun the file selected.
/// </summary>
/// <returns>The file selected if any, else empty string.</returns>
QString QssDialog::OpenFile()
{
#ifndef __APPLE__
	QStringList filenames;
	SetupFileDialog();
	m_FileDialog->setFileMode(QFileDialog::ExistingFile);
	m_FileDialog->setAcceptMode(QFileDialog::AcceptOpen);
	m_FileDialog->setNameFilter("Qss (*.qss)");
	m_FileDialog->setWindowTitle("Open Stylesheet");
	m_FileDialog->selectNameFilter("*.qss");

	if (m_FileDialog->exec() == QDialog::Accepted)
		filenames = m_FileDialog->selectedFiles();

	return !filenames.empty() ? filenames[0] : "";
#else
    auto filename = QFileDialog::getOpenFileName(this, tr("Open Stylesheet"), m_Parent->m_SettingsPath, tr("Qss (*.qss)"));
	return filename.size() > 0 ? filename : "";
#endif
}

/// <summary>
/// Present a file save dialog and retun the file selected.
/// </summary>
/// <returns>The file selected for saving if any, else empty string.</returns>
QString QssDialog::SaveFile()
{
#ifndef __APPLE__
	QStringList filenames;
	SetupFileDialog();
	m_FileDialog->setFileMode(QFileDialog::AnyFile);
	m_FileDialog->setAcceptMode(QFileDialog::AcceptSave);
	m_FileDialog->setNameFilter("Qss (*.qss)");
	m_FileDialog->setWindowTitle("Save Stylesheet");
	m_FileDialog->selectNameFilter("*.qss");

	if (m_FileDialog->exec() == QDialog::Accepted)
		filenames = m_FileDialog->selectedFiles();

	return !filenames.empty() ? filenames[0] : "";
#else
	auto filename = QFileDialog::getSaveFileName(this, tr("Save Stylesheet"), m_Parent->m_SettingsPath, tr("Qss (*.qss)"));
	return filename.size() > 0 ? filename : "";
#endif
}
