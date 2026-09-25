#include "FractoriumPch.h"
#include "Fractorium.h"
#include "QssDialog.h"

// X11 headers on Linux define this, causing build errors.
#ifdef KeyRelease
	#undef KeyRelease
#endif

/// <summary>
/// Constructor that initializes the entire program.
/// The setup process is very lengthy because it requires many custom modifications
/// to the GUI widgets that are not possible to do through the designer. So if something
/// is present here, it's safe to assume it can't be done in the designer.
/// </summary>
/// <param name="p">The parent widget of this item</param>
Fractorium::Fractorium(QWidget* p)
	: QMainWindow(p)
{
	const auto iconSize_ = 9;
	size_t i = 0;
	string s;
	Timing t;
	ui.setupUi(this);
	m_Info = OpenCLInfo::Instance();
	qRegisterMetaType<size_t>("size_t");
	qRegisterMetaType<QVector<int>>("QVector<int>");//For previews.
	qRegisterMetaType<vector<byte>>("vector<byte>");
	qRegisterMetaType<vv4F>("vv4F");
	qRegisterMetaType<Qt::CheckState>("CheckState");
	qRegisterMetaType<EmberTreeWidgetItemBase*>("EmberTreeWidgetItemBase*");
	tabifyDockWidget(ui.LibraryDockWidget, ui.FlameDockWidget);
	tabifyDockWidget(ui.FlameDockWidget, ui.XformsDockWidget);
	tabifyDockWidget(ui.XformsDockWidget, ui.XaosDockWidget);
	tabifyDockWidget(ui.XaosDockWidget, ui.PaletteDockWidget);
	tabifyDockWidget(ui.PaletteDockWidget, ui.InfoDockWidget);
	setTabPosition(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea, QTabWidget::TabPosition::North);
	//setTabShape(QTabWidget::TabShape::Rounded);
	m_Docks.reserve(8);
	m_Docks.push_back(ui.LibraryDockWidget);
	m_Docks.push_back(ui.FlameDockWidget);
	m_Docks.push_back(ui.XformsDockWidget);
	m_Docks.push_back(ui.XaosDockWidget);
	m_Docks.push_back(ui.PaletteDockWidget);
	m_Docks.push_back(ui.InfoDockWidget);

	for (auto dock : m_Docks)//Prevents a dock from ever getting accidentally hidden.
	{
		dock->setWindowFlags(dock->windowFlags() & Qt::WindowType::WindowStaysOnTopHint);
		dock->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea
//#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
							  | Qt::DockWidgetArea::RightDockWidgetArea
//#endif
							 );
	}

	m_Urls << QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first())
		   << QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first())
		   << QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first())
		   << QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first())
		   << QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first())
		   ;
	m_FontSize = 9;
	m_VarSortMode = 1;//Sort by weight by default.
	m_PaletteSortMode = 0;//Sort by palette ascending by default.
	m_ColorDialog = new QColorDialog(this);
	m_Settings = FractoriumSettings::Instance();
	m_QssDialog = new QssDialog(this);
	m_OptionsDialog = new FractoriumOptionsDialog(this);
	m_VarDialog = new FractoriumVariationsDialog(this);
	m_AboutDialog = new FractoriumAboutDialog(this);
	//Put the about dialog in the screen center.
	m_AboutDialogCentered = false;
	connect(m_ColorDialog, SIGNAL(colorSelected(const QColor&)), this, SLOT(OnColorSelected(const QColor&)), Qt::ConnectionType::QueuedConnection);
	m_XformComboColors[i++] = QColor(0XFF, 0X00, 0X00);
	m_XformComboColors[i++] = QColor(0XCC, 0XCC, 0X00);
	m_XformComboColors[i++] = QColor(0X00, 0XCC, 0X00);
	m_XformComboColors[i++] = QColor(0X00, 0XCC, 0XCC);
	m_XformComboColors[i++] = QColor(0X40, 0X40, 0XFF);
	m_XformComboColors[i++] = QColor(0XCC, 0X00, 0XCC);
	m_XformComboColors[i++] = QColor(0XCC, 0X80, 0X00);
	m_XformComboColors[i++] = QColor(0X80, 0X00, 0X4F);
	m_XformComboColors[i++] = QColor(0X80, 0X80, 0X22);
	m_XformComboColors[i++] = QColor(0X60, 0X80, 0X60);
	m_XformComboColors[i++] = QColor(0X50, 0X80, 0X80);
	m_XformComboColors[i++] = QColor(0X4F, 0X4F, 0X80);
	m_XformComboColors[i++] = QColor(0X80, 0X50, 0X80);
	m_XformComboColors[i++] = QColor(0X80, 0X60, 0X22);
	m_FinalXformComboColor  = QColor(0x7F, 0x7F, 0x7F);

	for (i = 0; i < XFORM_COLOR_COUNT; i++)
	{
		QPixmap pixmap(iconSize_, iconSize_);
		pixmap.fill(m_XformComboColors[i]);
		m_XformComboIcons[i] = QIcon(pixmap);
	}

	QPixmap pixmap(iconSize_, iconSize_);
	pixmap.fill(m_FinalXformComboColor);
	m_FinalXformComboIcon = QIcon(pixmap);
	InitToolbarUI();
	InitParamsUI();
	InitXformsUI();
	InitXformsColorUI();
	InitXformsAffineUI();
	InitXformsVariationsUI();
	InitXformsSelectUI();
	InitXaosUI();
	InitPaletteUI();
	InitLibraryUI();
	InitInfoUI();
	InitMenusUI();
	//This will init the controller and fill in the variations and palette tables with template specific instances
	//of their respective objects.
#ifdef DO_DOUBLE

	if (m_Settings->Double())
		m_Controller = unique_ptr<FractoriumEmberControllerBase>(new FractoriumEmberController<double>(this));
	else
#endif
		m_Controller = unique_ptr<FractoriumEmberControllerBase>(new FractoriumEmberController<float>(this));

	m_Controller->SetupVariationsTree();
	m_Controller->FilteredVariations();

	if (m_Info->Ok() && m_Settings->OpenCL() && m_QualitySpin->value() < (m_Settings->OpenClQuality() * m_Settings->Devices().size()))
		m_QualitySpin->setValue(m_Settings->OpenClQuality() * m_Settings->Devices().size());

	const auto statusBarHeight = 20;// *devicePixelRatio();
	ui.StatusBar->setMinimumHeight(statusBarHeight);
	ui.StatusBar->setMaximumHeight(statusBarHeight);
	m_RenderStatusLabel = new QLabel(this);
	m_RenderStatusLabel->setMinimumWidth(200);
	m_RenderStatusLabel->setAlignment(Qt::AlignmentFlag::AlignRight);
	ui.StatusBar->addPermanentWidget(m_RenderStatusLabel);
	m_CoordinateStatusLabel = new QLabel(this);
	m_CoordinateStatusLabel->setMinimumWidth(300);
	m_CoordinateStatusLabel->setMaximumWidth(300);
	m_CoordinateStatusLabel->setAlignment(Qt::AlignmentFlag::AlignLeft);
	ui.StatusBar->addWidget(m_CoordinateStatusLabel);
	const auto progressBarHeight = 15;
	const auto progressBarWidth = 300;
	m_ProgressBar = new QProgressBar(this);
	m_ProgressBar->setRange(0, 100);
	m_ProgressBar->setValue(0);
	m_ProgressBar->setMinimumHeight(progressBarHeight);
	m_ProgressBar->setMaximumHeight(progressBarHeight);
	m_ProgressBar->setMinimumWidth(progressBarWidth);
	m_ProgressBar->setMaximumWidth(progressBarWidth);
	m_ProgressBar->setAlignment(Qt::AlignmentFlag::AlignCenter);
	ui.StatusBar->addPermanentWidget(m_ProgressBar);
	//Setup pointer in the GL window to point back to here.
	ui.GLDisplay->SetMainWindow(this);
	bool restored = restoreState(m_Settings->value("windowState").toByteArray());
	showMaximized();//This won't fully set things up and show them until after this constructor exits.
	connect(ui.LibraryDockWidget, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(dockLocationChanged(Qt::DockWidgetArea)));
	connect(ui.LibraryDockWidget, SIGNAL(topLevelChanged(bool)),                   this, SLOT(OnDockTopLevelChanged(bool)));

	//Always ensure the library tab is selected if not restoring, which will show preview renders.
	if (!restored)
	{
		ui.LibraryDockWidget->raise();
		ui.LibraryDockWidget->show();
		ui.XformsTabWidget->setCurrentIndex(2);//Make variations tab the currently selected one under the Xforms tab.
	}

	m_PreviousPaletteRow = -1;//Force click handler the first time through.
	SetCoordinateStatus(0, 0, 0, 0);
	SetTabOrders();
	m_SettingsPath = QFileInfo(m_Settings->fileName()).absoluteDir().absolutePath();
	ifstream ifs((m_SettingsPath + "/default.qss").toStdString().c_str(), ifstream::in);

	if (ifs.is_open())
	{
		string total, qs;
		total.reserve(20 * 1024);

		while (std::getline(ifs, qs))
			total += qs + "\n";

		m_Style = QString::fromStdString(total);
	}
	else
		m_Style = BaseStyle();

	setStyleSheet(m_Style);

	if (!m_Settings->Theme().isEmpty())
	{
		if (auto theme = QStyleFactory::create(m_Settings->Theme()))
		{
			m_Theme = theme;
			setStyle(m_Theme);
		}
	}
	else
	{
		if (!QStyleFactory::keys().empty())
		{
			auto foundFusion = false;

			for (auto& key : QStyleFactory::keys())
			{
				if (key.compare("fusion", Qt::CaseSensitivity::CaseInsensitive) == 0)//Default to fusion if it exists and the style has not been set yet.
				{
					m_Theme = QStyleFactory::create(key);
					setStyle(m_Theme);
					foundFusion = true;
					break;
				}
			}

			if (!foundFusion)
			{
				m_Theme = QStyleFactory::create(qApp->style()->objectName());
				setStyle(m_Theme);
			}
		}
	}

#ifdef __APPLE__

	for (auto dock : m_Docks)//Fixes focus problem on OSX.
	{
		if (!dock->isHidden())
		{
			dock->setFloating(!dock->isFloating());
			dock->setFloating(!dock->isFloating());
			break;
		}
	}

#endif
	//At this point, everything has been setup except the renderer. Shortly after
	//this constructor exits, GLWidget::InitGL() will create the initial flock and start the rendering timer
	//which executes whenever the program is idle. Upon starting the timer, the renderer
	//will be initialized.
	//auto cdc = wglGetCurrentDC();
	//auto cc = wglGetCurrentContext();
	//qDebug() << "Fractorium::Fractorium():";
	//qDebug() << "Current DC: " << cdc;
	//qDebug() << "Current Context: " << cc;
	QTimer::singleShot(1000, [&]() { ui.GLDisplay->InitGL(); });
}

/// <summary>
/// Destructor which saves out the settings file.
/// All other memory is cleared automatically through the use of STL.
/// </summary>
Fractorium::~Fractorium()
{
	SyncSequenceSettings();
	m_VarDialog->SyncSettings();
	m_Settings->ShowXforms(ui.ActionDrawPreAffines->isChecked() || ui.ActionDrawPostAffines->isChecked());
	m_Settings->ShowGrid(ui.ActionDrawGrid->isChecked());
	m_Settings->setValue("windowState", saveState());
	m_Settings->sync();

	if (m_Settings->LoadLast())
		m_Controller->SaveCurrentFileOnShutdown();
}

/// <summary>
/// Return the URLs used to determine the icons that show up in the location bar in all file/folder dialogs.
/// </summary>
QList<QUrl> Fractorium::Urls()
{
	return m_Urls;
}

/// <summary>
/// Set the coordinate text in the status bar.
/// </summary>
/// <param name="rasX">The raster x coordinate</param>
/// <param name="rasY">The raster y coordinate</param>
/// <param name="worldX">The cartesian world x coordinate</param>
/// <param name="worldY">The cartesian world y coordinate</param>
void Fractorium::SetCoordinateStatus(int rasX, int rasY, float worldX, float worldY)
{
	static QString coords;
	auto str = coords.asprintf("Window: %4d, %4d World: %2.2f, %2.2f", rasX, rasY, worldX, worldY);
	m_CoordinateStatusLabel->setText(str);
}

/// <summary>
/// Center the scroll area.
/// Called in response to a resizing, or setting of new ember.
/// </summary>
void Fractorium::CenterScrollbars()
{
	const auto w = ui.GLParentScrollArea->horizontalScrollBar();
	const auto h = ui.GLParentScrollArea->verticalScrollBar();

	if (w && h)
	{
		w->setValue(w->maximum() / 2);
		h->setValue(h->maximum() / 2);
	}
}

/// <summary>
/// Apply the settings for saving an ember to an Xml file to an ember (presumably about to be saved).
/// </summary>
/// <param name="ember">The ember to apply the settings to</param>
template <typename T>
void FractoriumEmberController<T>::ApplyXmlSavingTemplate(Ember<T>& ember)
{
	ember.m_Quality         = m_Fractorium->m_Settings->XmlQuality();
	ember.m_Supersample     = m_Fractorium->m_Settings->XmlSupersample();
	ember.m_TemporalSamples = m_Fractorium->m_Settings->XmlTemporalSamples();
}

/// <summary>
/// Return whether the current ember contains a final xform and the GUI is aware of it.
/// Note this can be true even if the final is empty, as long as they've added one and have
/// not explicitly deleted it.
/// </summary>
/// <returns>True if the current ember contains a final xform, else false.</returns>
bool Fractorium::HaveFinal()
{
	const auto combo = ui.CurrentXformCombo;
	return (combo->count() > 0 && combo->itemText(combo->count() - 1) == "Final");
}

/// <summary>
/// Slots.
/// </summary>

/// <summary>
/// Empty placeholder for now.
/// Qt has a severe bug where the dock gets hidden behind the window.
/// Perhaps this will be used in the future if Qt ever fixes that bug.
/// Called when the top level dock is changed.
/// </summary>
/// <param name="topLevel">True if top level, else false.</param>
void Fractorium::OnDockTopLevelChanged(bool topLevel)
{
	//setTabPosition(Qt::DockWidgetArea::AllDockWidgetAreas, QTabWidget::TabPosition::North);
	//if (topLevel)
	//{
	//	if (ui.DockWidget->y() <= 0)
	//		ui.DockWidget->setGeometry(ui.DockWidget->x(), ui.DockWidget->y() + 100, ui.DockWidget->width(), ui.DockWidget->height());
	//
	//	ui.DockWidget->setFloating(true);
	//}
	//else
	//	ui.DockWidget->setFloating(false);
}

/// <summary>
/// Empty placeholder for now.
/// Qt has a severe bug where the dock gets hidden behind the window.
/// Perhaps this will be used in the future if Qt ever fixes that bug.
/// Called when the dock location is changed.
/// </summary>
/// <param name="area">The dock widget area</param>
void Fractorium::dockLocationChanged(Qt::DockWidgetArea area)
{
	//setTabPosition(Qt::DockWidgetArea::AllDockWidgetAreas, QTabWidget::TabPosition::North);
	//ui.DockWidget->resize(500, ui.DockWidget->height());
	//ui.DockWidget->update();
	//ui.dockWidget->setFloating(true);
	//ui.dockWidget->setFloating(false);
}

/// <summary>
/// Virtual event overrides.
/// </summary>

/// <summary>
/// Event filter for taking special action on:
/// Dock widget resize events, which in turn trigger GLParentScrollArea events.
/// Library tree key events, specifically delete.
/// Library tree drag n drop events.
/// </summary>
/// <param name="o">The object</param>
/// <param name="e">The eevent</param>
/// <returns>false</returns>
bool Fractorium::eventFilter(QObject* o, QEvent* e)
{
	static int fcount = 0;//Qt seems to deliver three events for every key press. So a count must be kept to only respond to the third event.
	static int xfupcount = 0;
	static int xfdncount = 0;
	static int wcount = 0;
	static int scount = 0;
	static int acount = 0;
	static int dcount = 0;
	static int qcount = 0;
	static int ecount = 0;
	static int gcount = 0;
	static int hcount = 0;
	static int pcount = 0;
	static int commacount = 0;
	static int periodcount = 0;
	static int lcount = 0;
	static int ctrlgcount = 0;
	const bool shift = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier);
	const bool ctrl = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier);

	if (o == ui.GLParentScrollArea && e->type() == QEvent::Resize)
	{
		m_WidthSpin->DoubleClickNonZero(ui.GLParentScrollArea->width() * ui.GLDisplay->devicePixelRatioF());
		m_HeightSpin->DoubleClickNonZero(ui.GLParentScrollArea->height() * ui.GLDisplay->devicePixelRatioF());
	}
	else if (const auto ke = dynamic_cast<QKeyEvent*>(e))
	{
		auto combo = ui.CurrentXformCombo;
		const auto times = 3;
		const auto ftimes = 2;

		if (ke->key() >= Qt::Key::Key_F1 && ke->key() <= Qt::Key::Key_F32)
		{
			fcount++;

			if (fcount >= ftimes)
			{
				const auto val = ke->key() - (int)Qt::Key::Key_F1;

				if (shift)
				{
					if (val < ui.LibraryTree->topLevelItem(0)->childCount())
						m_Controller->SetEmber(val, true);
				}
				else if (val < combo->count())
					combo->setCurrentIndex(val);

				fcount = 0;
				//qDebug() << "global function key press: " << ke->key() << " " << o->metaObject()->className() << " " << o->objectName();
			}

			return true;
		}
		else if (o == ui.LibraryTree)
		{
			//Require shift for deleting to prevent it from triggering when the user enters delete in the edit box.
			if (ke->key() == Qt::Key::Key_Delete && e->type() == QEvent::Type::KeyRelease && shift)
			{
				auto v = GetCurrentEmberIndex(false);

				if (ui.LibraryTree->topLevelItem(0)->childCount() > 1)
					OnDelete(v);

				return true;
			}
		}
		else if (o == this)
		{
			const auto focusedctrlEdit = dynamic_cast<QLineEdit*>(this->focusWidget());
			const auto focusedctrlSpin = dynamic_cast<QSpinBox*>(this->focusWidget());
			const auto focusedctrlDblSpin = dynamic_cast<QDoubleSpinBox*>(this->focusWidget());
			const auto focusedctrlCombo = dynamic_cast<QComboBox*>(this->focusWidget());

			if (ctrl && (ke->key() == Qt::Key_G))
			{
				ctrlgcount++;

				if (ctrlgcount >= ftimes)
				{
					OnSequenceGenerateButtonClicked(true);
					ctrlgcount = 0;
					return true;
				}
			}
			else if (!focusedctrlEdit &&
					 !focusedctrlSpin &&
					 !focusedctrlDblSpin &&
					 !focusedctrlCombo &&
					 !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::AltModifier))//Must exclude these because otherwise, typing a minus key in any of the spinners will switch the xform. Also exclude alt.
			{
				size_t index = 0;
				double vdist = 0.01;
				double hdist = 0.01;
				double zoom = 1;
				double rot = 1;
				double grow = 0.01;
				bool pre = true;

				if (shift)
				{
					auto v = GetCurrentEmberIndex(true);

					if (v.size() > 0)
						index = v[0].first;
					else
						index = 0;
				}
				else
					index = combo->currentIndex();

				if (const auto r = m_Controller->Renderer())
				{
					hdist = std::abs(r->UpperRightX() - r->LowerLeftX()) * 0.01 * m_Controller->AffineScaleLockedToCurrent();
					vdist = std::abs(r->UpperRightY() - r->LowerLeftY()) * 0.01 * m_Controller->AffineScaleLockedToCurrent();

					if (shift)
					{
						hdist *= 0.1;
						vdist *= 0.1;
						rot *= 0.1;
						grow *= 0.1;
						zoom *= 0.1;
					}
					else if (ctrl)
					{
						hdist *= 10;
						vdist *= 10;
						rot *= 10;
						grow *= 10;
						zoom *= 10;
					}
				}

				if (m_Controller.get() && m_Controller->GLController())
					pre = m_Controller->GLController()->AffineType() == eAffineType::AffinePre;

				if (ke->key() == Qt::Key::Key_Plus || ke->key() == Qt::Key::Key_Equal)
				{
					xfupcount++;

					if (xfupcount >= times)
					{
						xfupcount = 0;

						if (shift)
							m_Controller->SetEmber((index + 1) % ui.LibraryTree->topLevelItem(0)->childCount(), true);
						else
							combo->setCurrentIndex((index + 1) % combo->count());

						//qDebug() << "global arrow plus key press: " << ke->key() << " " << o->metaObject()->className() << " " << o->objectName();
					}

					return true;
				}
				else if (ke->key() == Qt::Key::Key_Minus || ke->key() == Qt::Key::Key_Underscore)
				{
					xfdncount++;

					if (xfdncount >= times)
					{
						xfdncount = 0;

						if (shift)
						{
							if (index == 0)
								index = ui.LibraryTree->topLevelItem(0)->childCount();

							m_Controller->SetEmber((index - 1) % ui.LibraryTree->topLevelItem(0)->childCount(), true);
						}
						else
						{
							if (index == 0)
								index = combo->count();

							combo->setCurrentIndex((index - 1) % combo->count());
						}

						//qDebug() << "global arrow minus key press: " << ke->key() << " " << o->metaObject()->className() << " " << o->objectName();
					}

					return true;
				}
				else if (ke->key() == Qt::Key::Key_P)
				{
					if (!ctrl)
					{
						pcount++;

						if (pcount >= times)
						{
							pcount = 0;

							if (!shift)
							{
								ui.ActionDrawPreAffines->setChecked(!ui.ActionDrawPreAffines->isChecked());
								OnActionDrawAffines(ui.ActionDrawPreAffines->isChecked());
							}
							else
							{
								ui.ActionDrawPostAffines->setChecked(!ui.ActionDrawPostAffines->isChecked());
								OnActionDrawAffines(ui.ActionDrawPostAffines->isChecked());
							}
						}

						return true;
					}
				}
				else if (ke->key() == Qt::Key::Key_L)
				{
					if (!ctrl)
					{
						lcount++;

						if (lcount >= times)
						{
							lcount = 0;

							if (!shift)
							{
								if (DrawPreAffines())
								{
									ui.ActionDrawAllPreAffines->setChecked(!ui.ActionDrawAllPreAffines->isChecked());
									OnActionDrawAllAffines(ui.ActionDrawAllPreAffines->isChecked());
								}
							}
							else if (DrawPostAffines())
							{
								ui.ActionDrawAllPostAffines->setChecked(!ui.ActionDrawAllPostAffines->isChecked());
								OnActionDrawAllAffines(ui.ActionDrawAllPostAffines->isChecked());
							}
						}

						return true;
					}
				}
				else if (ke->key() == Qt::Key::Key_Comma || ke->key() == Qt::Key::Key_Less)
				{
					commacount++;

					if (commacount >= times)
					{
						commacount = 0;
						m_ScaleSpin->setValue(m_ScaleSpin->value() - zoom);
					}

					return true;
				}
				else if (ke->key() == Qt::Key::Key_Period || ke->key() == Qt::Key::Key_Greater)
				{
					periodcount++;

					if (periodcount >= times)
					{
						periodcount = 0;
						m_ScaleSpin->setValue(m_ScaleSpin->value() + zoom);
					}

					return true;
				}
				else if ((!DrawPreAffines() && pre) || (!DrawPostAffines() && !pre))//Everything below this must be for editing xforms via key press.
				{
					return true;
				}
				else if (ke->key() == Qt::Key::Key_W)
				{
					wcount++;

					if (wcount >= times)
					{
						wcount = 0;
						m_Controller->MoveXforms(0, vdist, pre);
					}

					return true;
				}
				else if (ke->key() == Qt::Key::Key_S)
				{
					scount++;

					if (scount >= times)
					{
						scount = 0;
						m_Controller->MoveXforms(0, -vdist, pre);
					}

					return true;
				}
				else if (ke->key() == Qt::Key::Key_A)
				{
					acount++;

					if (acount >= times)
					{
						acount = 0;
						m_Controller->MoveXforms(-hdist, 0, pre);
					}

					return true;
				}
				else if (ke->key() == Qt::Key::Key_D)
				{
					dcount++;

					if (dcount >= times)
					{
						dcount = 0;
						m_Controller->MoveXforms(hdist, 0, pre);
					}

					return true;
				}
				else if (ke->key() == Qt::Key::Key_Q)
				{
					qcount++;

					if (qcount >= times)
					{
						qcount = 0;
						m_Controller->RotateXformsByAngle(-rot, pre);
					}

					return true;
				}
				else if (ke->key() == Qt::Key::Key_E)
				{
					ecount++;

					if (ecount >= times)
					{
						ecount = 0;
						m_Controller->RotateXformsByAngle(rot, pre);
					}

					return true;
				}
				else if (ke->key() == Qt::Key::Key_G)
				{
					gcount++;

					if (gcount >= times)
					{
						gcount = 0;
						m_Controller->ScaleXforms(1 - grow, pre);
					}

					return true;
				}
				else if (ke->key() == Qt::Key::Key_H)
				{
					hcount++;

					if (hcount >= times)
					{
						hcount = 0;
						m_Controller->ScaleXforms(1 + grow, pre);
					}

					return true;
				}
			}
		}
	}

	return QMainWindow::eventFilter(o, e);
}

/// <summary>
/// Respond to a resize event which will set the double click default value
/// in the width and height spinners.
/// Note, this does not change the size of the ember being rendered or
/// the OpenGL texture it's being drawn on.
/// <param name="e">The event</param>
void Fractorium::resizeEvent(QResizeEvent* e)
{
	m_WidthSpin->DoubleClickNonZero(ui.GLParentScrollArea->width() * ui.GLDisplay->devicePixelRatioF());
	m_HeightSpin->DoubleClickNonZero(ui.GLParentScrollArea->height() * ui.GLDisplay->devicePixelRatioF());
	QMainWindow::resizeEvent(e);
}

/// <summary>
/// Respond to a show event to ensure Qt updates the native menubar.
/// On first create, Qt can fail to create the native menu bar properly,
/// but telling it that this window has become the focus window forces
/// it to refresh this.
/// <param name="e">The event</param>
void Fractorium::showEvent(QShowEvent* e)
{
	//Tell Qt to refresh the native menubar from this widget.
	emit qGuiApp->focusWindowChanged(windowHandle());
	QMainWindow::showEvent(e);

	if (!m_AboutDialogCentered)
	{
		m_AboutDialogCentered = true;
		QTimer::singleShot(100, this, SLOT(WindowShown()));
	}
}

/// <summary>
/// Center About Dialog window.
/// Called on program starts after show is completed.
/// See https://doc.qt.io/qt-6/application-windows.html#window-geometry for more information
/// </summary>
void Fractorium::WindowShown()
{
	//Put the about dialog in the screen center.
	auto screen = QGuiApplication::screenAt(pos());
	auto geom = screen->availableGeometry();
	m_AboutDialog->move(geom.center() - m_AboutDialog->rect().center());
}

/// <summary>
/// Stop rendering and block before exiting.
/// Called on program exit.
/// </summary>
/// <param name="e">The event</param>
void Fractorium::closeEvent(QCloseEvent* e)
{
	if (m_Controller.get())
	{
		m_Controller->StopRenderTimer(true);//Will wait until fully exited and stopped.
		m_Controller->StopAllPreviewRenderers();
	}

	if (e)
		e->accept();
}

/// <summary>
/// Examine the files dragged when it first enters the window area.
/// Ok if at least one file is .flam3, .flam3 or .xml, else not ok.
/// Also traverse folders recursively if a folder is included in the list of dragged items.
/// Called when the user first drags files in.
/// </summary>
/// <param name="e">The event</param>
void Fractorium::dragEnterEvent(QDragEnterEvent* e)
{
	if (e->mimeData()->hasUrls())
	{
		auto urls = e->mimeData()->urls();

		for (auto& url : urls)
		{
			auto localFile = url.toLocalFile();

			if (QDir(localFile).exists())
			{
				QDirIterator it(localFile, QDirIterator::Subdirectories);

				while (it.hasNext())
				{
					auto next = it.next();
					qDebug() << next;
					QFileInfo fileInfo(next);
					auto suf = fileInfo.suffix();

					if (suf == "flam3" || suf == "flame" || suf == "xml" || suf == "chaos")
					{
						e->accept();
						return;
					}
				}
			}
			else
			{
				QFileInfo fileInfo(localFile);
				auto suf = fileInfo.suffix();

				if (suf == "flam3" || suf == "flame" || suf == "xml" || suf == "chaos")
				{
					e->accept();
					break;
				}
			}
		}
	}
}

/// <summary>
/// Always accept drag when moving, so that the drop event will correctly be called.
/// </summary>
/// <param name="e">The event</param>
void Fractorium::dragMoveEvent(QDragMoveEvent* e)
{
	e->accept();
}

/// <summary>
/// Examine and open the dropped files and/or folders.
/// Called when the user drops a file or folder in.
/// </summary>
/// <param name="e">The event</param>
void Fractorium::dropEvent(QDropEvent* e)
{
	QStringList filenames;
	const auto mod = e->modifiers();
	const auto append = mod.testFlag(Qt::KeyboardModifier::ControlModifier) ? false : true;

	if (e->mimeData()->hasUrls())
	{
		auto urls = e->mimeData()->urls();

		for (auto& url : urls)
		{
			auto localFile = url.toLocalFile();

			if (QDir(localFile).exists())
			{
				QDirIterator it(localFile, QDirIterator::IteratorFlag::Subdirectories);

				while (it.hasNext())
				{
					auto next = it.next();
					qDebug() << next;
					QFileInfo fileInfo(next);

					if (fileInfo.isFile() && fileInfo.exists())
					{
						auto suf = fileInfo.suffix();

						if (suf == "flam3" || suf == "flame" || suf == "xml" || suf == "chaos")
							filenames << next;
					}
				}
			}
			else
			{
				QFileInfo fileInfo(localFile);
				auto suf = fileInfo.suffix();

				if (suf == "flam3" || suf == "flame" || suf == "xml" || suf == "chaos")
					filenames << localFile;
			}
		}
	}

	if (!filenames.empty())
		m_Controller->OpenAndPrepFiles(filenames, append);
}

/// <summary>
/// Setup a combo box to be placed in a table cell.
/// </summary>
/// <param name="table">The table the combo box belongs to</param>
/// <param name="receiver">The receiver object</param>
/// <param name="row">The row in the table where this combo box resides</param>
/// <param name="col">The col in the table where this combo box resides</param>
/// <param name="comboBox">Double pointer to combo box which will hold the spinner upon exit</param>
/// <param name="vals">The string values to populate the combo box with</param>
/// <param name="signal">The signal the combo box emits</param>
/// <param name="slot">The slot to receive the signal</param>
/// <param name="connectionType">Type of the connection. Default: Qt::ConnectionType::QueuedConnection.</param>
void Fractorium::SetupCombo(QTableWidget* table, const QObject* receiver, int& row, int col, StealthComboBox*& comboBox, const vector<string>& vals, const char* signal, const char* slot, Qt::ConnectionType connectionType)
{
	comboBox = new StealthComboBox(table);

	for (auto& s : vals) comboBox->addItem(s.c_str());

	table->setCellWidget(row, col, comboBox);
	connect(comboBox, signal, receiver, slot, connectionType);
	row++;
}

/// <summary>
/// Set the header of a table to be fixed.
/// </summary>
/// <param name="header">The header to set</param>
/// <param name="mode">The resizing mode to use. Default: QHeaderView::Fixed.</param>
void Fractorium::SetFixedTableHeader(QHeaderView* header, QHeaderView::ResizeMode mode)
{
	header->setVisible(true);//For some reason, the designer keeps clobbering this value, so force it here.
	header->setSectionsClickable(false);
	header->setSectionResizeMode(mode);
}

/// <summary>
/// Setup and show the open XML dialog.
/// This will perform lazy instantiation.
/// </summary>
/// <param name="openExamples">true to open files in examples folder</param>
/// <returns>The list of filenames selected</returns>
QStringList Fractorium::SetupOpenXmlDialog(bool openExamples)
{
#ifndef __APPLE__

	//Lazy instantiate since it takes a long time.
	if (!m_OpenFileDialog.get())
	{
		m_OpenFileDialog = std::make_unique<QFileDialog>(this);
		m_OpenFileDialog->setViewMode(QFileDialog::List);
		m_OpenFileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
		connect(m_OpenFileDialog.get(), &QFileDialog::filterSelected, [&](const QString & filter) { m_Settings->OpenXmlExt(filter); });
		m_OpenFileDialog->setFileMode(QFileDialog::ExistingFiles);
		m_OpenFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
		m_OpenFileDialog->setNameFilter("flam3 (*.flam3);;flame (*.flame);;xml (*.xml);;chaos (*.chaos)");
		m_OpenFileDialog->setWindowTitle("Open Flame");
		m_OpenFileDialog->setSidebarUrls(m_Urls);
	}

	QStringList filenames;

	if (openExamples)
	{
		m_OpenFileDialog->selectFile("*");
#ifndef _WIN32

		if (QDir("/usr/share/fractorium/examples").exists())
			m_OpenFileDialog->setDirectory("/usr/share/fractorium/examples");
		else
			m_OpenFileDialog->setDirectory(QCoreApplication::applicationDirPath() + "/examples");

#else
		m_OpenFileDialog->setDirectory(QCoreApplication::applicationDirPath() + "/examples");
#endif
		m_OpenFileDialog->selectNameFilter("flame (*.flame)");
	}
	else
	{
		m_OpenFileDialog->setDirectory(m_Settings->OpenFolder());
		m_OpenFileDialog->selectNameFilter(m_Settings->OpenXmlExt());
	}

	if (m_OpenFileDialog->exec() == QDialog::Accepted)
	{
		filenames = m_OpenFileDialog->selectedFiles();

		if (!openExamples && !filenames.empty())
			m_Settings->OpenFolder(QFileInfo(filenames[0]).canonicalPath());
		else
			m_OpenFileDialog->selectFile("*");
	}

#else
	QString defaultFilter;
	QStringList filenames;

	if (openExamples)
	{
		defaultFilter = "flame (*.flame)";
		filenames = QFileDialog::getOpenFileNames(this, tr("Open Flame"), QCoreApplication::applicationDirPath() + "/Examples", tr("flame(*.flame)"), &defaultFilter);
	}
	else
	{
		defaultFilter = m_Settings->OpenXmlExt();
		filenames = QFileDialog::getOpenFileNames(this, tr("Open Flame"), m_Settings->OpenFolder(), tr("flam3(*.flam3);; flame(*.flame);; xml(*.xml);; chaos (*.chaos)"), &defaultFilter);
		m_Settings->OpenXmlExt(defaultFilter);

		if (!filenames.empty())
			m_Settings->OpenFolder(QFileInfo(filenames[0]).canonicalPath());
	}

#endif
	return filenames;
}

/// <summary>
/// Setup and show the save XML dialog.
/// This will perform lazy instantiation.
/// </summary>
/// <param name="defaultFilename">The default filename to populate the text box with</param>
/// <returns>The filename selected</returns>
QString Fractorium::SetupSaveXmlDialog(const QString& defaultFilename)
{
#ifndef __APPLE__

	//Lazy instantiate since it takes a long time.
	//QS
	if (!m_SaveFileDialog.get())
	{
		m_SaveFileDialog = std::make_unique<QFileDialog>(this);
		m_SaveFileDialog->setViewMode(QFileDialog::List);
		m_SaveFileDialog->setFileMode(QFileDialog::FileMode::AnyFile);
		m_SaveFileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
		//This must be done once here because clears various internal states which allow the file text to be properly set.
		//This is most likely a bug in QFileDialog.
		m_SaveFileDialog->setAcceptMode(QFileDialog::AcceptSave);
		connect(m_SaveFileDialog.get(), &QFileDialog::filterSelected, [&](const QString & filter)
		{
			m_Settings->SaveXmlExt(filter);
			m_SaveFileDialog->setDefaultSuffix(filter);
		});
		m_SaveFileDialog->setNameFilter("flam3 (*.flam3);;flame (*.flame);;xml (*.xml)");
		m_SaveFileDialog->setWindowTitle("Save flame as xml");
		m_SaveFileDialog->setSidebarUrls(m_Urls);
	}

	QString filename;
	m_SaveFileDialog->selectFile(defaultFilename);
	m_SaveFileDialog->setDirectory(m_Settings->SaveFolder());
	m_SaveFileDialog->selectNameFilter(m_Settings->SaveXmlExt());
	m_SaveFileDialog->setDefaultSuffix(m_Settings->SaveXmlExt());

	if (m_SaveFileDialog->exec() == QDialog::Accepted)
	{
		filename = m_SaveFileDialog->selectedFiles().value(0);
		auto filenames = filename.split(" (*");//This is a total hack, but Qt has the unfortunate behavior of including the description with the extension. It's probably a bug.
		filename = filenames[0];
		m_Settings->SaveXmlExt(m_SaveFileDialog->selectedNameFilter());
	}

#else
	auto defaultFilter(m_Settings->SaveXmlExt());
	auto filename = QFileDialog::getSaveFileName(this, tr("Save flame as xml"), m_Settings->SaveFolder() + "/" + defaultFilename, tr("flam3 (*.flam3);;flame (*.flame);;xml (*.xml)"), &defaultFilter);
	m_Settings->SaveXmlExt(defaultFilter);
#endif
	return filename;
}

/// <summary>
/// Setup and show the save image dialog.
/// This will perform lazy instantiation.
/// </summary>
/// <param name="defaultFilename">The default filename to populate the text box with</param>
/// <returns>The filename selected</returns>
QString Fractorium::SetupSaveImageDialog(const QString& defaultFilename)
{
#ifndef __APPLE__

	//Lazy instantiate since it takes a long time.
	if (!m_SaveImageDialog.get())
	{
		m_SaveImageDialog = std::make_unique<QFileDialog>(this);
		m_SaveImageDialog->setViewMode(QFileDialog::List);
		m_SaveImageDialog->setFileMode(QFileDialog::FileMode::AnyFile);
		m_SaveImageDialog->setOption(QFileDialog::DontUseNativeDialog, true);
		//This must be done once here because clears various internal states which allow the file text to be properly set.
		//This is most likely a bug in QFileDialog.
		m_SaveImageDialog->setAcceptMode(QFileDialog::AcceptSave);
		connect(m_SaveImageDialog.get(), &QFileDialog::filterSelected, [&](const QString & filter)
		{
			m_Settings->SaveImageExt(filter);
			m_SaveImageDialog->setDefaultSuffix(filter);
		});
#ifdef _WIN32
		m_SaveImageDialog->setNameFilter(".bmp;;.jpg;;.png;;.exr");
#else
		m_SaveImageDialog->setNameFilter(".jpg;;.png;;.exr");
#endif
		m_SaveImageDialog->setWindowTitle("Save image");
		m_SaveImageDialog->setSidebarUrls(m_Urls);
	}

	QString filename;
	m_SaveImageDialog->selectFile(defaultFilename);
	m_SaveImageDialog->setDirectory(m_Settings->SaveFolder());
	m_SaveImageDialog->selectNameFilter(m_Settings->SaveImageExt());
	m_SaveImageDialog->setDefaultSuffix(m_Settings->SaveImageExt());

	if (m_SaveImageDialog->exec() == QDialog::Accepted)
		filename = m_SaveImageDialog->selectedFiles().value(0);

#else
	auto defaultFilter(m_Settings->SaveImageExt());
	auto filename = QFileDialog::getSaveFileName(this, tr("Save image"), m_Settings->SaveFolder() + "/" + defaultFilename, tr("Jpg (*.jpg);;Png (*.png);;Exr (*.exr)"), &defaultFilter);
	m_Settings->SaveImageExt(defaultFilter);
#endif
	return filename;
}

/// <summary>
/// Setup and show the save folder dialog.
/// This will perform lazy instantiation.
/// </summary>
/// <returns>The folder selected, with '/' appended to the end</returns>
QString Fractorium::SetupSaveFolderDialog()
{
#ifndef __APPLE__

	//Lazy instantiate since it takes a long time.
	if (!m_FolderDialog.get())
	{
		m_FolderDialog = std::make_unique<QFileDialog>(this);
		m_FolderDialog->setViewMode(QFileDialog::List);
		m_FolderDialog->setOption(QFileDialog::DontUseNativeDialog, true);
		//This must come first because it clears various internal states which allow the file text to be properly set.
		//This is most likely a bug in QFileDialog.
		m_FolderDialog->setAcceptMode(QFileDialog::AcceptSave);
		m_FolderDialog->setFileMode(QFileDialog::Directory);
		m_FolderDialog->setOption(QFileDialog::ShowDirsOnly, true);
		m_FolderDialog->setWindowTitle("Save to folder");
		m_FolderDialog->setSidebarUrls(m_Urls);
	}

	QString filename;
	m_FolderDialog->selectFile("");
	m_FolderDialog->setDirectory(m_Settings->SaveFolder());

	if (m_FolderDialog->exec() == QDialog::Accepted)
	{
		filename = MakeEnd(m_FolderDialog->selectedFiles().value(0), '/');
	}

#else
	auto filename = QFileDialog::getExistingDirectory(this, tr("Save to folder"),
					m_Settings->SaveFolder(),
					QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (filename.size() > 0)
		filename = MakeEnd(filename, '/');

#endif
	return filename;
}

/// <summary>
/// Setup the final render dialog.
/// Note this deletes the existing instance before creating the new one.
/// This must be called every time the final render dialog is shown because
/// there are problems with reusing it.
/// </summary>
/// <returns>True if created successfully, else false</returns>
bool Fractorium::SetupFinalRenderDialog()
{
	if ((m_FinalRenderDialog = std::make_unique<FractoriumFinalRenderDialog>(this)))
	{
		connect(m_FinalRenderDialog.get(), SIGNAL(finished(int)), this, SLOT(OnFinalRenderClose(int)), Qt::ConnectionType::QueuedConnection);
		return true;
	}

	return false;
}

/// <summary>
/// Thin wrapper around QMessageBox::critical() to allow it to be invoked from another thread.
/// </summary>
/// <param name="title">The title of the message box</param>
/// <param name="text">The text displayed on the message box</param>
/// <param name="invokeRequired">True if running on another thread, else false. Default: false.</param>
void Fractorium::ShowCritical(const QString& title, const QString& text, bool invokeRequired)
{
	if (!invokeRequired)
		QMessageBox::critical(this, title, text);
	else
		QMetaObject::invokeMethod(this, "ShowCritical", Qt::ConnectionType::QueuedConnection, Q_ARG(const QString&, title), Q_ARG(const QString&, text), Q_ARG(bool, false));
}

/// <summary>
/// Explicitly set the tab orders for the entire program.
/// Qt has a facility to do this, but it fails when using custom widgets in
/// tables, so it must be done manually here.
/// This list must be kept in sync with any UI changes.
/// </summary>
void Fractorium::SetTabOrders()
{
	QWidget* w = SetTabOrder(this, ui.ColorTable, m_BrightnessSpin);//Flame color.
	w = SetTabOrder(this, w, m_GammaSpin);
	w = SetTabOrder(this, w, m_GammaThresholdSpin);
	w = SetTabOrder(this, w, m_VibrancySpin);
	w = SetTabOrder(this, w, m_HighlightSpin);
	w = SetTabOrder(this, w, m_BackgroundColorButton);
	w = SetTabOrder(this, w, m_PaletteModeCombo);
	w = SetTabOrder(this, w, m_WidthSpin);//Flame geometry.
	w = SetTabOrder(this, w, m_HeightSpin);
	w = SetTabOrder(this, w, m_CenterXSpin);
	w = SetTabOrder(this, w, m_CenterYSpin);
	w = SetTabOrder(this, w, m_ScaleSpin);
	w = SetTabOrder(this, w, m_ZoomSpin);
	w = SetTabOrder(this, w, m_RotateSpin);
	w = SetTabOrder(this, w, m_ZPosSpin);
	w = SetTabOrder(this, w, m_PerspectiveSpin);
	w = SetTabOrder(this, w, m_PitchSpin);
	w = SetTabOrder(this, w, m_YawSpin);
	w = SetTabOrder(this, w, m_DepthBlurSpin);
	w = SetTabOrder(this, w, m_BlurCurveSpin);
	w = SetTabOrder(this, w, m_SpatialFilterWidthSpin);//Flame filter.
	w = SetTabOrder(this, w, m_SpatialFilterTypeCombo);
	w = SetTabOrder(this, w, m_DEFilterMinRadiusSpin);
	w = SetTabOrder(this, w, m_DEFilterMaxRadiusSpin);
	w = SetTabOrder(this, w, m_DECurveSpin);
	w = SetTabOrder(this, w, m_SbsSpin);//Flame iteration.
	w = SetTabOrder(this, w, m_FuseSpin);
	w = SetTabOrder(this, w, m_RandRangeSpin);
	w = SetTabOrder(this, w, m_QualitySpin);
	w = SetTabOrder(this, w, m_SupersampleSpin);
	w = SetTabOrder(this, w, m_InterpTypeCombo);//Flame animation.
	w = SetTabOrder(this, w, m_AffineInterpTypeCombo);
	w = SetTabOrder(this, w, m_RotationsSpin);
	w = SetTabOrder(this, w, m_SecondsPerRotationSpin);
	w = SetTabOrder(this, w, m_RotateXformsDirCombo);
	w = SetTabOrder(this, w, m_BlendSecondsSpin);
	w = SetTabOrder(this, w, m_RotationsPerBlendSpin);
	w = SetTabOrder(this, w, m_BlendXformsRotateDirCombo);
	w = SetTabOrder(this, w, m_BlendInterpTypeCombo);
	w = SetTabOrder(this, w, m_StaggerSpin);
	w = SetTabOrder(this, w, m_TemporalFilterWidthSpin);
	w = SetTabOrder(this, w, m_TemporalFilterTypeCombo);
	/*w = */SetTabOrder(this, w, m_TemporalFilterExpSpin);
	w = SetTabOrder(this, ui.LibraryTree, ui.SequenceStartCountSpinBox);//Library.
	w = SetTabOrder(this, w, ui.SequenceStartPreviewsButton);
	w = SetTabOrder(this, w, ui.SequenceStopPreviewsButton);
	w = SetTabOrder(this, w, ui.SequenceStartFlameSpinBox);
	w = SetTabOrder(this, w, ui.SequenceStopFlameSpinBox);
	w = SetTabOrder(this, w, ui.SequenceAllButton);
	w = SetTabOrder(this, w, ui.SequenceAnimationFpsSpinBox);
	w = SetTabOrder(this, w, ui.SequenceGenerateButton);
	w = SetTabOrder(this, w, ui.SequenceRenderButton);
	w = SetTabOrder(this, w, ui.SequenceSaveButton);
	w = SetTabOrder(this, w, ui.SequenceOpenButton);
	w = SetTabOrder(this, w, ui.SequenceAnimateButton);
	w = SetTabOrder(this, w, ui.SequenceClearButton);
	/*w = */SetTabOrder(this, w, ui.SequenceTree);
	w = SetTabOrder(this, ui.CurrentXformCombo, ui.AddXformButton);//Xforms.
	w = SetTabOrder(this, w, ui.AddLinkedXformButton);
	w = SetTabOrder(this, w, ui.DuplicateXformButton);
	w = SetTabOrder(this, w, ui.ClearXformButton);
	w = SetTabOrder(this, w, ui.DeleteXformButton);
	w = SetTabOrder(this, w, ui.AddFinalXformButton);
	w = SetTabOrder(this, w, m_XformWeightSpin);
	w = SetTabOrder(this, w, m_XformWeightSpinnerButtonWidget->m_Button);
	/*w = */SetTabOrder(this, w, m_XformNameEdit);
	w = SetTabOrder(this, m_XformColorIndexSpin, ui.XformColorScroll);//Xforms color.
	w = SetTabOrder(this, w, ui.RandomColorIndicesButton);
	w = SetTabOrder(this, w, ui.ToggleColorIndicesButton);
	w = SetTabOrder(this, w, m_XformColorSpeedSpin);
	w = SetTabOrder(this, w, m_XformOpacitySpin);
	w = SetTabOrder(this, w, m_XformDirectColorSpin);
	w = SetTabOrder(this, w, ui.SoloXformCheckBox);
	w = SetTabOrder(this, w, m_PreX1Spin);
	w = SetTabOrder(this, w, m_PreX2Spin);
	w = SetTabOrder(this, w, m_PreY1Spin);
	w = SetTabOrder(this, w, m_PreY2Spin);
	w = SetTabOrder(this, w, m_PreO1Spin);
	w = SetTabOrder(this, w, m_PreO2Spin);
	w = SetTabOrder(this, w, ui.PreCopyButton);
	w = SetTabOrder(this, w, ui.PreFlipVerticalButton);
	w = SetTabOrder(this, w, ui.PreResetButton);
	w = SetTabOrder(this, w, ui.PreFlipHorizontalButton);
	w = SetTabOrder(this, w, ui.PrePasteButton);
	w = SetTabOrder(this, w, ui.PreRotate90CcButton);
	w = SetTabOrder(this, w, ui.PreRotateCcButton);
	w = SetTabOrder(this, w, ui.PreRotateCombo);
	w = SetTabOrder(this, w, ui.PreRotateCButton);
	w = SetTabOrder(this, w, ui.PreRotate90CButton);
	w = SetTabOrder(this, w, ui.PreMoveUpButton);
	w = SetTabOrder(this, w, ui.PreMoveDownButton);
	w = SetTabOrder(this, w, ui.PreMoveCombo);
	w = SetTabOrder(this, w, ui.PreMoveLeftButton);
	w = SetTabOrder(this, w, ui.PreMoveRightButton);
	w = SetTabOrder(this, w, ui.PreScaleDownButton);
	w = SetTabOrder(this, w, ui.PreScaleCombo);
	w = SetTabOrder(this, w, ui.PreScaleUpButton);
	w = SetTabOrder(this, w, ui.PreRandomButton);
	w = SetTabOrder(this, w, ui.SwapAffinesButton);
	w = SetTabOrder(this, w, ui.PostAffineGroupBox);
	w = SetTabOrder(this, w, m_PostX1Spin);
	w = SetTabOrder(this, w, m_PostX2Spin);
	w = SetTabOrder(this, w, m_PostY1Spin);
	w = SetTabOrder(this, w, m_PostY2Spin);
	w = SetTabOrder(this, w, m_PostO1Spin);
	w = SetTabOrder(this, w, m_PostO2Spin);
	w = SetTabOrder(this, w, ui.PostCopyButton);
	w = SetTabOrder(this, w, ui.PostFlipVerticalButton);
	w = SetTabOrder(this, w, ui.PostResetButton);
	w = SetTabOrder(this, w, ui.PostFlipHorizontalButton);
	w = SetTabOrder(this, w, ui.PostPasteButton);
	w = SetTabOrder(this, w, ui.PostRotate90CcButton);
	w = SetTabOrder(this, w, ui.PostRotateCcButton);
	w = SetTabOrder(this, w, ui.PostRotateCombo);
	w = SetTabOrder(this, w, ui.PostRotateCButton);
	w = SetTabOrder(this, w, ui.PostRotate90CButton);
	w = SetTabOrder(this, w, ui.PostMoveUpButton);
	w = SetTabOrder(this, w, ui.PostMoveDownButton);
	w = SetTabOrder(this, w, ui.PostMoveCombo);
	w = SetTabOrder(this, w, ui.PostMoveLeftButton);
	w = SetTabOrder(this, w, ui.PostMoveRightButton);
	w = SetTabOrder(this, w, ui.PostScaleDownButton);
	w = SetTabOrder(this, w, ui.PostScaleCombo);
	w = SetTabOrder(this, w, ui.PostScaleUpButton);
	w = SetTabOrder(this, w, ui.PostRandomButton);
	w = SetTabOrder(this, w, ui.PolarAffineCheckBox);
	w = SetTabOrder(this, w, ui.LocalPivotRadio);
	/*w = */SetTabOrder(this, w, ui.WorldPivotRadio);
	w = SetTabOrder(this, ui.VariationsFilterLineEdit, ui.VariationsFilterClearButton);//Xforms variation.
	w = SetTabOrder(this, w, ui.VariationsTree);
	w = SetTabOrder(this, w, ui.ClearXaosButton);
	w = SetTabOrder(this, w, ui.RandomXaosButton);
	w = SetTabOrder(this, w, ui.AddLayerButton);
	w = SetTabOrder(this, w, ui.AddLayerSpinBox);
	/*w = */SetTabOrder(this, w, ui.TransposeXaosButton);
	//Xforms xaos is done dynamically every time.
	w = SetTabOrder(this, ui.PaletteFilenameCombo, m_PaletteHueSpin);//Palette.
	w = SetTabOrder(this, w, m_PaletteContrastSpin);
	w = SetTabOrder(this, w, m_PaletteSaturationSpin);
	w = SetTabOrder(this, w, m_PaletteBlurSpin);
	w = SetTabOrder(this, w, m_PaletteBrightnessSpin);
	w = SetTabOrder(this, w, m_PaletteFrequencySpin);
	w = SetTabOrder(this, w, ui.PaletteRandomSelectButton);
	w = SetTabOrder(this, w, ui.PaletteRandomAdjustButton);
	w = SetTabOrder(this, w, ui.PaletteEditorButton);
	w = SetTabOrder(this, w, ui.PaletteFilterLineEdit);
	w = SetTabOrder(this, w, ui.PaletteFilterClearButton);
	w = SetTabOrder(this, w, ui.PaletteListTable);
	w = SetTabOrder(this, w, ui.ResetCurvesButton);//Palette curves.
	w = SetTabOrder(this, w, ui.CurvesView);
	w = SetTabOrder(this, w, ui.CurvesGroupBox);
	w = SetTabOrder(this, w, ui.CurvesAllRadio);
	w = SetTabOrder(this, w, ui.CurvesRedRadio);
	w = SetTabOrder(this, w, ui.CurvesGreenRadio);
	/*w = */SetTabOrder(this, w, ui.CurvesBlueRadio);
	/*w = */SetTabOrder(this, ui.SummaryTable, ui.SummaryTree);//Info summary.
	w = SetTabOrder(this, ui.InfoBoundsGroupBox, ui.InfoBoundsFrame);//Info bounds.
	w = SetTabOrder(this, w, ui.InfoBoundsTable);
	w = SetTabOrder(this, w, ui.InfoFileOpeningGroupBox);
	w = SetTabOrder(this, w, ui.InfoFileOpeningTextEdit);
	w = SetTabOrder(this, w, ui.InfoRenderingGroupBox);
	/*w = */SetTabOrder(this, w, ui.InfoRenderingTextEdit);
}

/// <summary>
/// Toggle all table spinner values in one row.
/// The logic is:
///		If any cell in the row is non zero, set all cells to zero, else 1.
///		If shift is held down, reverse the logic.
///		If ctrl is held down, set each cell to a random 0 or 1.
/// Resets the rendering process.
/// </summary>
/// <param name="table">The QTableWidget or QTableView whose row will be toggled</param>
/// <param name="logicalIndex">The index of the row that was double clicked</param>
void Fractorium::ToggleTableRow(QTableView* table, int logicalIndex)
{
	auto allZero = true;
	const auto model = table->model();
	const auto cols = model->columnCount();
	const auto shift = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier);
	const auto ctrl = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier);
	const auto tableWidget = qobject_cast<QTableWidget*>(table);

	if (tableWidget)
	{
		for (int i = 0; i < cols; i++)
		{
			if (auto spinBox = qobject_cast<DoubleSpinBox*>(tableWidget->cellWidget(logicalIndex, i)))
			{
				if (!IsNearZero(spinBox->value()))
				{
					allZero = false;
					break;
				}
			}
		}

		if (shift)
			allZero = !allZero;

		const auto val = allZero ? 1.0 : 0.0;

		for (int i = 0; i < cols; i++)
		{
			if (auto spinBox = qobject_cast<DoubleSpinBox*>(tableWidget->cellWidget(logicalIndex, i)))
			{
				if (ctrl)
					spinBox->setValue(static_cast<double>(QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRandBit()));
				else
					spinBox->setValue(val);
			}
		}
	}
	else
	{
		for (int i = 0; i < cols; i++)
		{
			if (!IsNearZero(model->data(model->index(logicalIndex, i, QModelIndex())).toDouble()))
			{
				allZero = false;
				break;
			}
		}

		if (shift)
			allZero = !allZero;

		const auto val = allZero ? 1.0 : 0.0;

		for (int i = 0; i < cols; i++)
		{
			if (ctrl)
				model->setData(model->index(logicalIndex, i), double(QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRandBit()), Qt::ItemDataRole::EditRole);
			else
				model->setData(model->index(logicalIndex, i), val, Qt::ItemDataRole::EditRole);
		}
	}
}

/// <summary>
/// Toggle all table spinner values in one column.
/// The logic is:
///		If any cell in the column is non zero, set all cells to zero, else 1.
///		If shift is held down, reverse the logic.
///		If ctrl is held down, set each cell to a random 0 or 1.
/// Resets the rendering process.
/// </summary>
/// <param name="table">The QTableWidget or QTableView whose column will be toggled</param>
/// <param name="logicalIndex">The index of the column that was double clicked</param>
void Fractorium::ToggleTableCol(QTableView* table, int logicalIndex)
{
	auto allZero = true;
	const auto model = table->model();
	const auto rows = model->rowCount();
	const auto shift = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier);
	const auto ctrl = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier);
	const auto tableWidget = qobject_cast<QTableWidget*>(table);

	if (tableWidget)
	{
		for (int i = 0; i < rows; i++)
		{
			if (auto spinBox = qobject_cast<DoubleSpinBox*>(tableWidget->cellWidget(i, logicalIndex)))
			{
				if (!IsNearZero(spinBox->value()))
				{
					allZero = false;
					break;
				}
			}
		}

		if (shift)
			allZero = !allZero;

		const auto val = allZero ? 1.0 : 0.0;

		for (int i = 0; i < rows; i++)
		{
			if (auto spinBox = qobject_cast<DoubleSpinBox*>(tableWidget->cellWidget(i, logicalIndex)))
			{
				if (ctrl)
					spinBox->setValue(static_cast<double>(QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRandBit()));
				else
					spinBox->setValue(val);
			}
		}
	}
	else
	{
		for (int i = 0; i < rows; i++)
		{
			if (!IsNearZero(model->data(model->index(i, logicalIndex, QModelIndex())).toDouble()))
			{
				allZero = false;
				break;
			}
		}

		if (shift)
			allZero = !allZero;

		const auto val = allZero ? 1.0 : 0.0;

		for (int i = 0; i < rows; i++)
		{
			if (ctrl)
				model->setData(model->index(i, logicalIndex), double(QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRandBit()), Qt::ItemDataRole::EditRole);
			else
				model->setData(model->index(i, logicalIndex), val, Qt::ItemDataRole::EditRole);
		}
	}
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
