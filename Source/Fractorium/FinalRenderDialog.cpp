#include "FractoriumPch.h"
#include "FinalRenderDialog.h"
#include "Fractorium.h"

QString FractoriumFinalRenderDialog::m_Prefix = "";
QString FractoriumFinalRenderDialog::m_Suffix = "";


/// <summary>
/// Constructor which sets up the GUI for the final rendering dialog.
/// Settings used to populate widgets with initial values.
/// This function contains the render function as a lambda.
/// </summary>
/// <param name="settings">Pointer to the global settings object to use</param>
/// <param name="p">The parent widget</param>
/// <param name="f">The window flags. Default: 0.</param>
FractoriumFinalRenderDialog::FractoriumFinalRenderDialog(QWidget* p, Qt::WindowFlags f)
	: QDialog(p, f)
{
	ui.setupUi(this);
	int row = 0;
	const auto spinHeight = 20;
	const auto dmax = numeric_limits<double>::max();
	QTableWidget* table = ui.FinalRenderParamsTable;
	m_Info = OpenCLInfo::Instance();
	m_Fractorium = qobject_cast<Fractorium*>(p);
	m_Settings = FractoriumSettings::DefInstance();
	ui.FinalRenderIterationProgress->setAlignment(Qt::AlignmentFlag::AlignCenter);
	ui.FinalRenderFilteringProgress->setAlignment(Qt::AlignmentFlag::AlignCenter);
	ui.FinalRenderAccumProgress->setAlignment(Qt::AlignmentFlag::AlignCenter);
	ui.FinalRenderTotalProgress->setAlignment(Qt::AlignmentFlag::AlignCenter);
	ui.FinalRenderThreadCountSpin->setRange(1, Timing::ProcessorCount());
	connect(ui.FinalRenderEarlyClipCheckBox,       SIGNAL(checkStateChanged(Qt::CheckState)),   this, SLOT(OnEarlyClipCheckBoxStateChanged(Qt::CheckState)),       Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderYAxisUpCheckBox,         SIGNAL(checkStateChanged(Qt::CheckState)),   this, SLOT(OnYAxisUpCheckBoxStateChanged(Qt::CheckState)),         Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderTransparencyCheckBox,    SIGNAL(checkStateChanged(Qt::CheckState)),   this, SLOT(OnTransparencyCheckBoxStateChanged(Qt::CheckState)),    Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderOpenCLCheckBox,          SIGNAL(checkStateChanged(Qt::CheckState)),   this, SLOT(OnOpenCLCheckBoxStateChanged(Qt::CheckState)),          Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderDoublePrecisionCheckBox, SIGNAL(checkStateChanged(Qt::CheckState)),   this, SLOT(OnDoublePrecisionCheckBoxStateChanged(Qt::CheckState)), Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderDoAllCheckBox,           SIGNAL(checkStateChanged(Qt::CheckState)),   this, SLOT(OnDoAllCheckBoxStateChanged(Qt::CheckState)),           Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderDoSequenceCheckBox,      SIGNAL(checkStateChanged(Qt::CheckState)),   this, SLOT(OnDoSequenceCheckBoxStateChanged(Qt::CheckState)),      Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderCurrentSpin,             SIGNAL(valueChanged(int)),                   this, SLOT(OnCurrentSpinChanged(int)),                             Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderApplyToAllCheckBox,      SIGNAL(checkStateChanged(Qt::CheckState)),   this, SLOT(OnApplyAllCheckBoxStateChanged(Qt::CheckState)),        Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderKeepAspectCheckBox,      SIGNAL(checkStateChanged(Qt::CheckState)),   this, SLOT(OnKeepAspectCheckBoxStateChanged(Qt::CheckState)),      Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderScaleNoneRadioButton,    SIGNAL(toggled(bool)),                       this, SLOT(OnScaleRadioButtonChanged(bool)),                       Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderScaleWidthRadioButton,   SIGNAL(toggled(bool)),                       this, SLOT(OnScaleRadioButtonChanged(bool)),                       Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderScaleHeightRadioButton,  SIGNAL(toggled(bool)),                       this, SLOT(OnScaleRadioButtonChanged(bool)),                       Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderDeviceTable,             SIGNAL(cellChanged(int, int)),               this, SLOT(OnDeviceTableCellChanged(int, int)),                    Qt::ConnectionType::QueuedConnection);
	SetupSpinner<DoubleSpinBox, double>(ui.FinalRenderSizeTable, this, row, -1, m_WidthScaleSpin,  spinHeight, 0.001,                           99.99, 0.1, SIGNAL(valueChanged(double)), SLOT(OnWidthScaleChanged(double)),  true,  1.0,  1.0,  1.0);
	SetupSpinner<SpinBox, int>         (ui.FinalRenderSizeTable, this, row, -1, m_WidthSpin,       spinHeight,    10, std::numeric_limits<int>::max(),  10, SIGNAL(valueChanged(int)),    SLOT(OnWidthChanged(int)),          true, 1920, 1920, 1920);
	SetupSpinner<DoubleSpinBox, double>(ui.FinalRenderSizeTable, this, row, -1, m_HeightScaleSpin, spinHeight, 0.001,                           99.99, 0.1, SIGNAL(valueChanged(double)), SLOT(OnHeightScaleChanged(double)), true,  1.0,  1.0,  1.0);
	SetupSpinner<SpinBox, int>         (ui.FinalRenderSizeTable, this, row, -1, m_HeightSpin,      spinHeight,    10, std::numeric_limits<int>::max(),  10, SIGNAL(valueChanged(int)),    SLOT(OnHeightChanged(int)),         true, 1080, 1080, 1080);
	m_SubBatchPctSpin = ui.FinalRenderOpenCLSubBatchPctSpin;
	m_SubBatchPctSpin->DoubleClick(true);
	m_SubBatchPctSpin->DoubleClickZero(0.025);
	m_SubBatchPctSpin->DoubleClickNonZero(0.025);
	int spinsize = 120;
	m_WidthScaleSpin->setDecimals(4);
	m_HeightScaleSpin->setDecimals(4);
	m_WidthScaleSpin->setFixedWidth(spinsize);
	m_HeightScaleSpin->setFixedWidth(spinsize);
	m_WidthScaleSpin->SmallStep(0.0001);
	m_HeightScaleSpin->SmallStep(0.0001);
	m_WidthSpin->setFixedWidth(spinsize);
	m_HeightSpin->setFixedWidth(spinsize);
	m_WidthSpinnerWidget = new DoubleIntSpinnerWidget(m_WidthScaleSpin, m_WidthSpin, ui.FinalRenderSizeTable);
	m_WidthSpinnerWidget->m_DoubleSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_WidthSpinnerWidget->m_SpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_WidthSpinnerWidget->setMinimumWidth(spinsize);
	m_WidthSpinnerWidget->setMaximumWidth(spinsize);
	ui.FinalRenderSizeTable->setCellWidget(0, 1, m_WidthSpinnerWidget);
	//
	m_HeightSpinnerWidget = new DoubleIntSpinnerWidget(m_HeightScaleSpin, m_HeightSpin, ui.FinalRenderSizeTable);
	m_HeightSpinnerWidget->m_DoubleSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_HeightSpinnerWidget->m_SpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_HeightSpinnerWidget->setMinimumWidth(spinsize);
	m_HeightSpinnerWidget->setMaximumWidth(spinsize);
	ui.FinalRenderSizeTable->setCellWidget(1, 1, m_HeightSpinnerWidget);
	row = 0;
 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_QualitySpin,         spinHeight,  1, dmax, 50, SIGNAL(valueChanged(double)), SLOT(OnQualityChanged(double)),      true, 1000, 1000, 1000);
 SetupSpinner<SpinBox, int>         (table, this, row, 1, m_TemporalSamplesSpin, spinHeight,  1, 5000, 50, SIGNAL(valueChanged(int)),    SLOT(OnTemporalSamplesChanged(int)), true, 1000, 1000, 1000);
 SetupSpinner<SpinBox, int>         (table, this, row, 1, m_SupersampleSpin,     spinHeight,  1,    4,  1, SIGNAL(valueChanged(int)),    SLOT(OnSupersampleChanged(int)),     true,    2,    1,    1);
 SetupSpinner<SpinBox, int>         (table, this, row, 1, m_StripsSpin,          spinHeight,  1,   64,  1, SIGNAL(valueChanged(int)),    SLOT(OnStripsChanged(int)),          true,    1,    1,    1);
	m_MemoryCellIndex = row++;//Memory usage.
	m_ItersCellIndex = row++;//Iters.
	m_PathCellIndex = row;
	QStringList comboList;
#ifdef _WIN32
	comboList.append("bmp");
#endif
	comboList.append("jpg");
	comboList.append("png");
	comboList.append("exr");
	m_Tbcw = new TwoButtonComboWidget("...", "Open", comboList, 22, 40, 22, table);
	table->setCellWidget(row, 1, m_Tbcw);
	table->item(row++, 1)->setTextAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
	connect(m_Tbcw->m_Button1, SIGNAL(clicked(bool)),            this, SLOT(OnFileButtonClicked(bool)),       Qt::ConnectionType::QueuedConnection);
	connect(m_Tbcw->m_Button2, SIGNAL(clicked(bool)),            this, SLOT(OnShowFolderButtonClicked(bool)), Qt::ConnectionType::QueuedConnection);
	connect(m_Tbcw->m_Combo,   SIGNAL(currentIndexChanged(int)), this, SLOT(OnExtIndexChanged(int)),          Qt::ConnectionType::QueuedConnection);
	m_PrefixEdit = new QLineEdit(table);
	m_PrefixEdit->setText(m_Prefix);
	table->setCellWidget(row++, 1, m_PrefixEdit);
	m_SuffixEdit = new QLineEdit(table);
	m_SuffixEdit->setText(m_Suffix);
	table->setCellWidget(row++, 1, m_SuffixEdit);
	connect(m_PrefixEdit, SIGNAL(textChanged(const QString&)), this, SLOT(OnPrefixChanged(const QString&)), Qt::ConnectionType::QueuedConnection);
	connect(m_SuffixEdit, SIGNAL(textChanged(const QString&)), this, SLOT(OnSuffixChanged(const QString&)), Qt::ConnectionType::QueuedConnection);
	ui.FinalRenderStartButton->disconnect(SIGNAL(clicked(bool)));
	connect(ui.FinalRenderStartButton, SIGNAL(clicked(bool)), this, SLOT(OnRenderClicked(bool)),       Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderPauseButton, SIGNAL(clicked(bool)), this, SLOT(OnPauseClicked(bool)),        Qt::ConnectionType::QueuedConnection);
	connect(ui.FinalRenderStopButton,  SIGNAL(clicked(bool)), this, SLOT(OnCancelRenderClicked(bool)), Qt::ConnectionType::QueuedConnection);
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.FinalRenderSizeTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	table = ui.FinalRenderDeviceTable;
	table->clearContents();
	table->setRowCount(0);

	if (m_Info->Ok() && !m_Info->Devices().empty())
	{
		//This is an extra attempt to prevent crashes on unsupported hardware:
		//Hardware which does not have an OpenCL 1.2 driver installed will hard crash the program upon initializing a renderer. There is no way around this.
		//When the user selects the OpenCL settings in the main options dialog, they can select the desired hardware before an attempt is made to create the renderer.
		//However, in the final render dialog, the renderer is created and its settings changed for every applicable UI change made by the user so that it can be
		//applied to the preview thumbnail.
		//When they click Use OpenCL, it will immediately try to create the renderer with whatever devices are present in the device table.
		//Since the first entry is always selected by default, the renderer will be created using it.
		//If that entry is a supported device, then all is fine.
		//However, if it's not a supported device, it will immediately crash the program.
		//In such a scenario, the user will never be able to use OpenCL for the final render, even though they were able to use it for the interactive render.
		//This workaround is to detect if the user has successfully specified and used any device other than the default on the main window in the options dialog.
		//If so, use that instead.
		//Since this code only runs once upon startup, they must close the program and re-run it after properly configuring an OpenCL device on the main window.
		//At that point, when they open the final render dialog, it will use the correct device.
		if (m_Settings->FinalDevices().isEmpty() &&//Is it the first run?
				!m_Settings->Devices().empty() &&//They've successfully used OpenCL in the main window?
				!(m_Settings->Devices().size() == 1 && m_Settings->Devices()[0].toInt() == 0))//Extra check it wasn't just the default, in which case it'd offer no value here since the default here is also 0.
			m_Settings->FinalDevices(m_Settings->Devices());//Assign what was used in the main window here.

		SetupDeviceTable(table, m_Settings->FinalDevices());

		for (int i = 0; i < table->rowCount(); i++)
			if (auto radio = qobject_cast<QRadioButton*>(table->cellWidget(i, 1)))
				connect(radio, SIGNAL(toggled(bool)), this, SLOT(OnDeviceTableRadioToggled(bool)), Qt::ConnectionType::QueuedConnection);

		ui.FinalRenderOpenCLCheckBox->setChecked(m_Settings->FinalOpenCL());
	}
	else
	{
		table->setEnabled(false);
		ui.FinalRenderOpenCLCheckBox->setChecked(false);
		ui.FinalRenderOpenCLCheckBox->setEnabled(false);
	}

	 ui.FinalRenderEarlyClipCheckBox->setChecked(         m_Settings->FinalEarlyClip());
	 ui.FinalRenderYAxisUpCheckBox->setChecked(           m_Settings->FinalYAxisUp());
	 ui.FinalRenderTransparencyCheckBox->setChecked(      m_Settings->FinalTransparency());
	 ui.FinalRenderDoublePrecisionCheckBox->setChecked(   m_Settings->FinalDouble());
	 ui.FinalRenderSaveXmlCheckBox->setChecked(           m_Settings->FinalSaveXml());
	 ui.FinalRenderDoAllCheckBox->setChecked(             m_Settings->FinalDoAll());
	 ui.FinalRenderDoSequenceCheckBox->setChecked(        m_Settings->FinalDoSequence());
	 ui.FinalRenderPng16BitCheckBox->setChecked(          m_Settings->FinalPng16Bit());
	 ui.FinalRenderKeepAspectCheckBox->setChecked(        m_Settings->FinalKeepAspect());
	 ui.FinalRenderThreadCountSpin->setValue(             m_Settings->FinalThreadCount());
#ifdef _WIN32
	ui.FinalRenderThreadPriorityComboBox->setCurrentIndex(m_Settings->FinalThreadPriority() + 2);
#else
	auto tpc = ui.FinalRenderThreadPriorityComboBox->count() - 1;

	if (m_Settings->FinalThreadPriority() == THREAD_PRIORITY_LOWEST)
		ui.FinalRenderThreadPriorityComboBox->setCurrentIndex(0);
	else if (m_Settings->FinalThreadPriority() == THREAD_PRIORITY_HIGHEST)
		ui.FinalRenderThreadPriorityComboBox->setCurrentIndex(tpc);
	else
		ui.FinalRenderThreadPriorityComboBox->setCurrentIndex(Clamp<int>(m_Settings->FinalThreadPriority() / 25, 0, tpc));

#endif
	ui.FinalRenderOpenCLSubBatchPctSpin->setValue(m_Settings->FinalOpenCLSubBatchPct());
	m_QualitySpin->setValue(m_Settings->FinalQuality());
	m_TemporalSamplesSpin->setValue(m_Settings->FinalTemporalSamples());
	m_SupersampleSpin->setValue(m_Settings->FinalSupersample());
	m_StripsSpin->setValue(int(m_Settings->FinalStrips()));
	Scale(eScaleType(m_Settings->FinalScale()));
	const auto bumpmenu = new QMenu(this);
	const auto add10 = new QAction("Add 10% Quality", this); add10->setProperty("tag", QVariant(0.10));
	const auto add25 = new QAction("Add 25% Quality", this); add25->setProperty("tag", QVariant(0.25));
	const auto add50 = new QAction("Add 50% Quality", this); add50->setProperty("tag", QVariant(0.50));
	const auto add100 = new QAction("Add 100% Quality", this); add100->setProperty("tag", QVariant(1.0));
	const auto add200 = new QAction("Add 200% Quality", this); add200->setProperty("tag", QVariant(2.0));
	bumpmenu->addAction(add10);
	bumpmenu->addAction(add25);
	bumpmenu->addAction(add50);
	bumpmenu->addAction(add100);
	bumpmenu->addAction(add200);
	ui.FinalRenderBumpQualityStartButton->setMenu(bumpmenu);
	ui.FinalRenderBumpQualityStartButton->setProperty("tag", add25->property("tag"));
	ui.FinalRenderBumpQualityStartButton->setText(add25->text());
	ui.FinalRenderBumpQualityStartButton->setEnabled(false);
	const auto saamenu = new QMenu(this);
#ifdef _WIN32
	const auto saabmp = new QAction("bmp", this);
	saamenu->addAction(saabmp);
	connect(saabmp, SIGNAL(triggered()), this, SLOT(OnSaveAgainAsClicked()));
#endif
	const auto saajpg = new QAction("jpg", this);
	const auto saapng = new QAction("png", this);
	const auto saaexr = new QAction("exr", this);
	saamenu->addAction(saajpg);
	saamenu->addAction(saapng);
	saamenu->addAction(saaexr);
	ui.FinalRenderSaveAgainAsButton->setMenu(saamenu);
	ui.FinalRenderSaveAgainAsButton->setProperty("tag", "jpg");
	ui.FinalRenderSaveAgainAsButton->setText("Save Again as jpg");
	ui.FinalRenderSaveAgainAsButton->setEnabled(false);
	connect(ui.FinalRenderSaveAgainAsButton, SIGNAL(clicked()), this, SLOT(OnSaveAgainAsClicked()));
	connect(ui.FinalRenderBumpQualityStartButton, SIGNAL(clicked()), this, SLOT(OnQualityBumpClicked()));
	connect(saajpg, SIGNAL(triggered()), this, SLOT(OnSaveAgainAsClicked()));
	connect(saapng, SIGNAL(triggered()), this, SLOT(OnSaveAgainAsClicked()));
	connect(saaexr, SIGNAL(triggered()), this, SLOT(OnSaveAgainAsClicked()));
	connect(add10,  SIGNAL(triggered()), this, SLOT(OnQualityBumpClicked()));
	connect(add25,  SIGNAL(triggered()), this, SLOT(OnQualityBumpClicked()));
	connect(add50,  SIGNAL(triggered()), this, SLOT(OnQualityBumpClicked()));
	connect(add100, SIGNAL(triggered()), this, SLOT(OnQualityBumpClicked()));
	connect(add200, SIGNAL(triggered()), this, SLOT(OnQualityBumpClicked()));
	int index = 0;
#ifdef _WIN32

	if (m_Settings->FinalExt().endsWith("bmp", Qt::CaseSensitivity::CaseInsensitive))
		m_Tbcw->m_Combo->setCurrentIndex(index);

	index++;
#endif

	if (m_Settings->FinalExt().endsWith("jpg", Qt::CaseSensitivity::CaseInsensitive))
		m_Tbcw->m_Combo->setCurrentIndex(index);

	index++;

	if (m_Settings->FinalExt().endsWith("png", Qt::CaseSensitivity::CaseInsensitive))
		m_Tbcw->m_Combo->setCurrentIndex(index);

	index++;

	if (m_Settings->FinalExt().endsWith("exr", Qt::CaseSensitivity::CaseInsensitive))
		m_Tbcw->m_Combo->setCurrentIndex(index);

	//Explicitly call these to enable/disable the appropriate controls.
	OnOpenCLCheckBoxStateChanged(ui.FinalRenderOpenCLCheckBox->isChecked() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
	OnDoAllCheckBoxStateChanged(ui.FinalRenderDoAllCheckBox->isChecked() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
	OnDoSequenceCheckBoxStateChanged(ui.FinalRenderDoSequenceCheckBox->isChecked() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
	auto s = size();
	auto screen = QGuiApplication::screenAt(pos());
	auto geom = screen->availableGeometry();
	const auto desktopHeight = geom.height();
	s.setHeight(std::min(s.height(), int(double(desktopHeight * 0.90))));
	setGeometry(QStyle::alignedRect(Qt::LayoutDirection::LeftToRight, Qt::AlignmentFlag::AlignCenter, s, geom));
	//Update these with new controls.
	auto w = SetTabOrder(this, ui.FinalRenderEarlyClipCheckBox, ui.FinalRenderYAxisUpCheckBox);
	w = SetTabOrder(this, w, ui.FinalRenderTransparencyCheckBox);
	w = SetTabOrder(this, w, ui.FinalRenderOpenCLCheckBox);
	w = SetTabOrder(this, w, ui.FinalRenderPng16BitCheckBox);
	w = SetTabOrder(this, w, ui.FinalRenderDoublePrecisionCheckBox);
	w = SetTabOrder(this, w, ui.FinalRenderSaveXmlCheckBox);
	w = SetTabOrder(this, w, ui.FinalRenderDoAllCheckBox);
	w = SetTabOrder(this, w, ui.FinalRenderDoSequenceCheckBox);
	w = SetTabOrder(this, w, ui.FinalRenderUseNumbersForNames);
	w = SetTabOrder(this, w, ui.FinalRenderStartAtCheckBox);
	w = SetTabOrder(this, w, ui.FinalRenderCurrentSpin);
	w = SetTabOrder(this, w, ui.FinalRenderApplyToAllCheckBox);
	w = SetTabOrder(this, w, ui.FinalRenderDeviceTable);
	w = SetTabOrder(this, w, ui.FinalRenderThreadCountSpin);
	w = SetTabOrder(this, w, ui.FinalRenderThreadPriorityComboBox);
	w = SetTabOrder(this, w, ui.FinalRenderOpenCLSubBatchPctSpin);
	w = SetTabOrder(this, w, m_WidthScaleSpin);
	w = SetTabOrder(this, w, m_WidthSpin);
	w = SetTabOrder(this, w, m_HeightScaleSpin);
	w = SetTabOrder(this, w, m_HeightSpin);
	w = SetTabOrder(this, w, ui.FinalRenderScaleNoneRadioButton);
	w = SetTabOrder(this, w, ui.FinalRenderScaleWidthRadioButton);
	w = SetTabOrder(this, w, ui.FinalRenderScaleHeightRadioButton);
	w = SetTabOrder(this, w, ui.FinalRenderKeepAspectCheckBox);
	w = SetTabOrder(this, w, m_QualitySpin);
	w = SetTabOrder(this, w, m_TemporalSamplesSpin);
	w = SetTabOrder(this, w, m_SupersampleSpin);
	w = SetTabOrder(this, w, m_StripsSpin);
	w = SetTabOrder(this, w, m_Tbcw);
	w = SetTabOrder(this, w, m_Tbcw->m_Combo);
	w = SetTabOrder(this, w, m_Tbcw->m_Button1);
	w = SetTabOrder(this, w, m_Tbcw->m_Button2);
	w = SetTabOrder(this, w, m_PrefixEdit);
	w = SetTabOrder(this, w, m_SuffixEdit);
	w = SetTabOrder(this, w, ui.FinalRenderTextOutput);
	w = SetTabOrder(this, w, ui.FinalRenderSaveAgainAsButton);
	w = SetTabOrder(this, w, ui.FinalRenderBumpQualityStartButton);
	w = SetTabOrder(this, w, ui.FinalRenderStartButton);
	w = SetTabOrder(this, w, ui.FinalRenderPauseButton);
	w = SetTabOrder(this, w, ui.FinalRenderStopButton);
	w = SetTabOrder(this, w, ui.FinalRenderCloseButton);
}

FractoriumFinalRenderDialog::~FractoriumFinalRenderDialog()
{
	m_Prefix = m_PrefixEdit->text();
	m_Suffix = m_SuffixEdit->text();
}

/// <summary>
/// Show the final render dialog and specify whether it was called from the toolbar or the sequence render button.
/// </summary>
/// <param name="fromSequence">True if this is called from the sequence render button, else false.</param>
void FractoriumFinalRenderDialog::Show(bool fromSequence)
{
	m_FromSequence = fromSequence;
#ifdef __APPLE__
	exec();
#else
	show();
#endif
}

/// <summary>
/// GUI settings wrapper functions, getters only.
/// </summary>
bool FractoriumFinalRenderDialog::EarlyClip() { return ui.FinalRenderEarlyClipCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::YAxisUp() { return ui.FinalRenderYAxisUpCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::Transparency() { return ui.FinalRenderTransparencyCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::OpenCL() { return ui.FinalRenderOpenCLCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::Double() { return ui.FinalRenderDoublePrecisionCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::SaveXml() { return ui.FinalRenderSaveXmlCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::DoSequence() { return ui.FinalRenderDoSequenceCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::DoAll() { return ui.FinalRenderDoAllCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::Png16Bit() { return ui.FinalRenderPng16BitCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::UseNumbers() { return ui.FinalRenderUseNumbersForNames->isChecked(); }
bool FractoriumFinalRenderDialog::KeepAspect() { return ui.FinalRenderKeepAspectCheckBox->isChecked(); }
bool FractoriumFinalRenderDialog::ApplyToAll() { return ui.FinalRenderApplyToAllCheckBox->isChecked(); }
QString FractoriumFinalRenderDialog::Ext() { return m_Tbcw->m_Combo->currentText(); }
QString FractoriumFinalRenderDialog::Path() { return ui.FinalRenderParamsTable->item(m_PathCellIndex, 1)->text(); }
void FractoriumFinalRenderDialog::Path(const QString& s) { ui.FinalRenderParamsTable->item(m_PathCellIndex, 1)->setText(s); }
QString FractoriumFinalRenderDialog::Prefix() { return m_PrefixEdit->text(); }
QString FractoriumFinalRenderDialog::Suffix() { return m_SuffixEdit->text(); }
uint FractoriumFinalRenderDialog::Current() { return ui.FinalRenderCurrentSpin->value(); }
uint FractoriumFinalRenderDialog::StartAt() { return ui.FinalRenderStartAtCheckBox->isChecked() ? Current() - 1 : 0; }
uint FractoriumFinalRenderDialog::ThreadCount() { return ui.FinalRenderThreadCountSpin->value(); }
#ifdef _WIN32
int FractoriumFinalRenderDialog::ThreadPriority() { return ui.FinalRenderThreadPriorityComboBox->currentIndex() - 2; }
#else
int FractoriumFinalRenderDialog::ThreadPriority()
{
	if (ui.FinalRenderThreadPriorityComboBox->currentIndex() == 0)
		return THREAD_PRIORITY_LOWEST;
	else if (ui.FinalRenderThreadPriorityComboBox->currentIndex() == (ui.FinalRenderThreadPriorityComboBox->count() - 1))
		return THREAD_PRIORITY_HIGHEST;
	else
		return ui.FinalRenderThreadPriorityComboBox->currentIndex() * 25;
}
#endif
double FractoriumFinalRenderDialog::OpenCLSubBatchPct() { return ui.FinalRenderOpenCLSubBatchPctSpin->value();  }
double FractoriumFinalRenderDialog::WidthScale() { return m_WidthScaleSpin->value(); }
double FractoriumFinalRenderDialog::HeightScale() { return m_HeightScaleSpin->value(); }
double FractoriumFinalRenderDialog::Quality() { return m_QualitySpin->value(); }
uint FractoriumFinalRenderDialog::TemporalSamples() { return m_TemporalSamplesSpin->value(); }
uint FractoriumFinalRenderDialog::Supersample() { return m_SupersampleSpin->value(); }
uint FractoriumFinalRenderDialog::Strips() { return m_StripsSpin->value(); }
QList<QVariant> FractoriumFinalRenderDialog::Devices() { return DeviceTableToSettings(ui.FinalRenderDeviceTable); }

/// <summary>
/// Capture the current state of the Gui.
/// Used to hold options for performing the final render.
/// </summary>
/// <returns>The state of the Gui as a struct</returns>
FinalRenderGuiState FractoriumFinalRenderDialog::State()
{
	FinalRenderGuiState state;
	state.m_EarlyClip = EarlyClip();
	state.m_YAxisUp = YAxisUp();
	state.m_Transparency = Transparency();
	state.m_OpenCL = OpenCL();
	state.m_Double = Double();
	state.m_SaveXml = SaveXml();
	state.m_DoAll = DoAll();
	state.m_DoSequence = DoSequence();
	state.m_Png16Bit = Png16Bit();
	state.m_KeepAspect = KeepAspect();
	state.m_UseNumbers = UseNumbers();
	state.m_StartAt = StartAt();
	state.m_Scale = Scale();
	state.m_Path = Path();
	state.m_Ext = Ext();
	state.m_Prefix = Prefix();
	state.m_Suffix = Suffix();
	state.m_Devices = Devices();
	state.m_ThreadCount = ThreadCount();
	state.m_ThreadPriority = ThreadPriority();
	state.m_SubBatchPct = OpenCLSubBatchPct();
	state.m_WidthScale = WidthScale();
	state.m_HeightScale = HeightScale();
	state.m_Quality = Quality();
	state.m_TemporalSamples = TemporalSamples();
	state.m_Supersample = Supersample();
	state.m_Strips = Strips();
	return state;
}

/// <summary>
/// Return the type of scaling desired based on what radio button has been selected.
/// </summary>
/// <returns>The type of scaling as an eScaleType enum</returns>
eScaleType FractoriumFinalRenderDialog::Scale()
{
	if (ui.FinalRenderScaleNoneRadioButton->isChecked())
		return eScaleType::SCALE_NONE;
	else if (ui.FinalRenderScaleWidthRadioButton->isChecked())
		return eScaleType::SCALE_WIDTH;
	else if (ui.FinalRenderScaleHeightRadioButton->isChecked())
		return eScaleType::SCALE_HEIGHT;
	else
		return eScaleType::SCALE_NONE;
}

/// <summary>
/// Set the type of scaling desired which will select the corresponding radio button.
/// </summary>
/// <param name="scale">The type of scaling to use</param>
void FractoriumFinalRenderDialog::Scale(eScaleType scale)
{
	ui.FinalRenderScaleNoneRadioButton->blockSignals(true);

	if (scale == eScaleType::SCALE_NONE)
		ui.FinalRenderScaleNoneRadioButton->setChecked(true);
	else if (scale == eScaleType::SCALE_WIDTH)
		ui.FinalRenderScaleWidthRadioButton->setChecked(true);
	else if (scale == eScaleType::SCALE_HEIGHT)
		ui.FinalRenderScaleHeightRadioButton->setChecked(true);
	else
		ui.FinalRenderScaleNoneRadioButton->setChecked(true);

	ui.FinalRenderScaleNoneRadioButton->blockSignals(false);
}

/// <summary>
/// Simple wrapper to put moving the cursor to the end in a signal
/// so it can be called from a thread.
/// </summary>
void FractoriumFinalRenderDialog::MoveCursorToEnd()
{
	ui.FinalRenderTextOutput->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}

/// <summary>
/// Whether to use early clipping before spatial filtering.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void FractoriumFinalRenderDialog::OnEarlyClipCheckBoxStateChanged(Qt::CheckState state)
{
	SetMemory();
}

/// <summary>
/// Whether the positive Y axis of the final output image is up.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void FractoriumFinalRenderDialog::OnYAxisUpCheckBoxStateChanged(Qt::CheckState state)
{
	SetMemory();
}

/// <summary>
/// Whether to use transparency in png images.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void FractoriumFinalRenderDialog::OnTransparencyCheckBoxStateChanged(Qt::CheckState state)
{
	SetMemory();
}

/// <summary>
/// Set whether to use OpenCL in the rendering process or not.
/// Also disable or enable the CPU and OpenCL related controls based on the state passed in.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void FractoriumFinalRenderDialog::OnOpenCLCheckBoxStateChanged(Qt::CheckState state)
{
	const auto checked = state == Qt::CheckState::Checked;
	ui.FinalRenderDeviceTable->setEnabled(checked);
	ui.FinalRenderOpenCLSubBatchPctSpin->setEnabled(checked);
	ui.FinalRenderThreadCountSpin->setEnabled(!checked);
	ui.FinalRenderThreadPriorityLabel->setEnabled(!checked);
	ui.FinalRenderThreadPriorityComboBox->setEnabled(!checked);
	SetMemory();
}

/// <summary>
/// Set whether to use double or single precision in the rendering process or not.
/// This will recreate the entire controller.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void FractoriumFinalRenderDialog::OnDoublePrecisionCheckBoxStateChanged(Qt::CheckState state)
{
	SetMemory();
}

/// <summary>
/// The do all checkbox was changed.
/// If checked, render all embers available in the currently opened file, else
/// only render the current ember.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void FractoriumFinalRenderDialog::OnDoAllCheckBoxStateChanged(Qt::CheckState state)
{
	if (state != Qt::CheckState::Checked)
		ui.FinalRenderDoSequenceCheckBox->setChecked(false);

	ui.FinalRenderDoSequenceCheckBox->setEnabled(ui.FinalRenderDoAllCheckBox->isChecked());
	ui.FinalRenderStartAtCheckBox->setEnabled(ui.FinalRenderDoAllCheckBox->isChecked());
	ui.FinalRenderSaveAgainAsButton->setEnabled(false);
	ui.FinalRenderBumpQualityStartButton->setEnabled(false);
}

/// <summary>
/// The do sequence checkbox was changed.
/// If checked, render all embers available in the currently opened file as an animation sequence, else
/// render them individually.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void FractoriumFinalRenderDialog::OnDoSequenceCheckBoxStateChanged(Qt::CheckState state)
{
	const auto checked = state == Qt::CheckState::Checked;
	m_TemporalSamplesSpin->setEnabled(checked);

	if (checked)
		m_StripsSpin->setValue(1);

	m_StripsSpin->setEnabled(!checked);
	SetMemory();
}

/// <summary>
/// The current ember spinner was changed, update fields.
/// </summary>
/// <param name="d">The 1-based index of the current ember</param>
void FractoriumFinalRenderDialog::OnCurrentSpinChanged(int d)
{
	m_Controller->SetEmber(d - 1, false);
	m_Controller->SyncCurrentToGui();
	SetMemory();
}

/// <summary>
/// The apply all checkbox was changed.
/// If checked, set values for all embers in the file to the values specified in the GUI.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void FractoriumFinalRenderDialog::OnApplyAllCheckBoxStateChanged(Qt::CheckState state)
{
	if (state == Qt::CheckState::Checked && m_Controller.get())
		m_Controller->SyncGuiToEmbers();

	ui.FinalRenderSaveAgainAsButton->setEnabled(false);
	ui.FinalRenderBumpQualityStartButton->setEnabled(false);
}

/// <summary>
/// The width spinner was changed, recompute required memory.
/// If the aspect ratio checkbox is checked, set the value of
/// the height spinner as well to be in proportion.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnWidthScaleChanged(double d)
{
	if (m_Controller.get())
	{
		if (ui.FinalRenderKeepAspectCheckBox->isChecked())
			m_HeightScaleSpin->SetValueStealth(m_WidthScaleSpin->value());

		if (SetMemory())
			m_Controller->SyncCurrentToSizeSpinners(false, true);
	}
}

/// <summary>
/// The width spinner was changed, recompute required memory.
/// If the aspect ratio checkbox is checked, set the value of
/// the height spinner as well to be in proportion.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnWidthChanged(int d)
{
	if (m_Controller.get())
	{
		m_Controller->SyncGuiToEmbers(m_WidthSpin->value(), m_HeightSpin->value());//Copy changed width value from gui to ember (height will be copied even though it hasn't changed).
		m_Controller->SyncCurrentToSizeSpinners(true, false, true, false);//Compute how much the original width had to be scaled by to equal the new width and put the value in the width scale spinner.

		if (ui.FinalRenderKeepAspectCheckBox->isChecked())
		{
			m_HeightScaleSpin->SetValueStealth(m_WidthScaleSpin->value());//Make the height scale spinner match the newly computed width scale spinner.
			m_Controller->SyncGuiToEmbers(0, 0, false, true);//Scale height by the amount in the height scale spinner and assign the value to the ember height.
			m_Controller->SyncCurrentToSizeSpinners(false, true, false, true);//Copy new height value from the ember to the height spinner.
		}
	}

	if (SetMemory()) {}
}

/// <summary>
/// The height spinner was changed, recompute required memory.
/// If the aspect ratio checkbox is checked, set the value of
/// the width spinner as well to be in proportion.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnHeightScaleChanged(double d)
{
	if (m_Controller.get())
	{
		if (ui.FinalRenderKeepAspectCheckBox->isChecked())
			m_WidthScaleSpin->SetValueStealth(m_HeightScaleSpin->value());

		if (SetMemory())
			m_Controller->SyncCurrentToSizeSpinners(false, true);
	}
}

/// <summary>
/// The height spinner was changed, recompute required memory.
/// If the aspect ratio checkbox is checked, set the value of
/// the width spinner as well to be in proportion.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnHeightChanged(int d)
{
	if (m_Controller.get())
	{
		m_Controller->SyncGuiToEmbers(m_WidthSpin->value(), m_HeightSpin->value());//Copy changed height value from gui to ember (width will be copied even though it hasn't changed).
		m_Controller->SyncCurrentToSizeSpinners(true, false, false, true);//Compute how much the original height had to be scaled by to equal the new height and put the value in the height scale spinner.

		if (ui.FinalRenderKeepAspectCheckBox->isChecked())
		{
			m_WidthScaleSpin->SetValueStealth(m_HeightScaleSpin->value());//Make the width scale spinner match the newly computed height scale spinner.
			m_Controller->SyncGuiToEmbers(0, 0, true, false);//Scale width by the amount in the width scale spinner and assign the value to the ember width.
			m_Controller->SyncCurrentToSizeSpinners(false, true, true, false);//Copy new width value from the ember to the width spinner.
		}
	}

	if (SetMemory()) {}
}

/// <summary>
/// Whether to keep the aspect ratio of the desired width and height the same
/// as that of the original width and height.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void FractoriumFinalRenderDialog::OnKeepAspectCheckBoxStateChanged(Qt::CheckState state)
{
	if (state == Qt::CheckState::Checked && m_Controller.get())
		m_HeightScaleSpin->setValue(m_WidthScaleSpin->value());

	//m_HeightScaleSpin->SetValueStealth(m_WidthScaleSpin->value());
	SetMemory();
}

/// <summary>
/// The scaling method radio button selection was changed.
/// </summary>
/// <param name="checked">The state of the radio button</param>
void FractoriumFinalRenderDialog::OnScaleRadioButtonChanged(bool checked)//This is wrong. Does not react when scale is switched back to zero. Fix!//TODO
{
	if (checked)
		SetMemory();
}

/// <summary>
/// The check state of one of the OpenCL devices was changed.
/// This does a special check to always ensure at least one device,
/// as well as one primary is checked.
/// </summary>
/// <param name="row">The row of the cell</param>
/// <param name="col">The column of the cell</param>
void FractoriumFinalRenderDialog::OnDeviceTableCellChanged(int row, int col)
{
	if (const auto item = ui.FinalRenderDeviceTable->item(row, col))
	{
		HandleDeviceTableCheckChanged(ui.FinalRenderDeviceTable, row, col);
		SetMemory();
	}
}

/// <summary>
/// The primary device radio button selection was changed.
/// If the device was specified as primary, but was not selected
/// for inclusion, it will automatically be selected for inclusion.
/// </summary>
/// <param name="checked">The state of the radio button</param>
void FractoriumFinalRenderDialog::OnDeviceTableRadioToggled(bool checked)
{
	int row;
	const auto s = sender();
	const auto table = ui.FinalRenderDeviceTable;
	QRadioButton* radio = nullptr;

	if (s)
	{
		for (row = 0; row < table->rowCount(); row++)
			if ((radio = qobject_cast<QRadioButton*>(table->cellWidget(row, 1))))
				if (s == radio)
				{
					HandleDeviceTableCheckChanged(ui.FinalRenderDeviceTable, row, 1);
					break;
				}
	}

	if (checked)
		SetMemory();
}

/// <summary>
/// The quality spinner was changed, recompute required memory.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnQualityChanged(double d)
{
	SetMemory();
}

/// <summary>
/// The temporal samples spinner was changed, recompute required memory.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnTemporalSamplesChanged(int d)
{
	SetMemory();
}

/// <summary>
/// The supersample spinner was changed, recompute required memory.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnSupersampleChanged(int d)
{
	SetMemory();
}

/// <summary>
/// The supersample spinner was changed, recompute required memory.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnStripsChanged(int d)
{
	SetMemory();
}

/// <summary>
/// Show the save folder dialog.
/// Called when the ... file button is clicked.
/// </summary>
/// <param name="checked">Ignored</param>
void FractoriumFinalRenderDialog::OnFileButtonClicked(bool checked)
{
	const auto s = m_Fractorium->SetupSaveFolderDialog();

	if (Exists(s))
	{
		m_Settings->SaveFolder(s);//Any time they exit the box with a valid value, preserve it in the settings.
		Path(m_Controller->ComposePath(m_Controller->Name()));//And update the GUI.
		SetMemory();
	}
}

/// <summary>
/// Show the folder where the last rendered image was saved to.
/// </summary>
/// <param name="checked">Ignored</param>
void FractoriumFinalRenderDialog::OnShowFolderButtonClicked(bool checked)
{
	const auto s = m_Settings->SaveFolder();

	if (Exists(s))
		QDesktopServices::openUrl(QUrl::fromLocalFile(s));
	else
		QDesktopServices::openUrl(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation)[0]);
}

/// <summary>
/// Change the extension of the output image, which also may change the
/// number of channels used in the final output buffer.
/// </summary>
/// <param name="d">Ignored</param>
void FractoriumFinalRenderDialog::OnExtIndexChanged(int d)
{
	if (SetMemory())
		Path(m_Controller->ComposePath(m_Controller->Name()));
}

/// <summary>
/// Change the prefix prepended to the output file name.
/// </summary>
/// <param name="s">Ignored</param>
void FractoriumFinalRenderDialog::OnPrefixChanged(const QString& s)
{
	Path(m_Controller->ComposePath(m_Controller->Name()));
}

/// <summary>
/// Change the suffix appended to the output file name.
/// </summary>
/// <param name="s">Ignored</param>
void FractoriumFinalRenderDialog::OnSuffixChanged(const QString& s)
{
	Path(m_Controller->ComposePath(m_Controller->Name()));
}

/// <summary>
/// Increase the quality of the last render and start rendering again.
/// Note this is only when rendering a single image with no strips.
/// </summary>
void FractoriumFinalRenderDialog::OnQualityBumpClicked()
{
	const auto act = qobject_cast<QAction*>(sender());
	const auto tbtn = qobject_cast<QToolButton*>(sender());

	if (tbtn)
	{
		if (m_Controller.get())
		{
			double d = tbtn->property("tag").toDouble();
			m_QualitySpin->SetValueStealth(std::ceil(Quality() + (Quality() * d)));
			tbtn->setEnabled(false);
			ui.FinalRenderSaveAgainAsButton->setEnabled(false);
			m_Controller->BumpQualityRender(d);
		}
	}
	else if (act)
	{
		ui.FinalRenderBumpQualityStartButton->setText(act->text());
		ui.FinalRenderBumpQualityStartButton->setProperty("tag", act->property("tag"));
	}
}

/// <summary>
/// Save the last rendered image output again to a different file format.
/// Note this is only when rendering a single image with no strips.
/// </summary>
void FractoriumFinalRenderDialog::OnSaveAgainAsClicked()
{
	const auto act = qobject_cast<QAction*>(sender());
	const auto tbtn = qobject_cast<QToolButton*>(sender());

	if (tbtn)
	{
		if (m_Controller.get())
		{
			const auto s = tbtn->property("tag").toString();
			m_Tbcw->m_Combo->blockSignals(true);
			m_Tbcw->m_Combo->setCurrentText(s);
			m_Tbcw->m_Combo->blockSignals(false);
			const auto filename = m_Controller->SaveCurrentAgain();
			ui.FinalRenderTextOutput->append("\nSaved again as " + filename);
		}
	}
	else if (act)
	{
		ui.FinalRenderSaveAgainAsButton->setText("Save Again as " + act->text());
		ui.FinalRenderSaveAgainAsButton->setProperty("tag", act->text());
	}
}

/// <summary>
/// Start the render process.
/// </summary>
/// <param name="checked">Ignored</param>
void FractoriumFinalRenderDialog::OnRenderClicked(bool checked)
{
	if (CreateControllerFromGUI(true))
	{
		Pause(false);
		ui.FinalRenderSaveAgainAsButton->setEnabled(false);
		ui.FinalRenderBumpQualityStartButton->setEnabled(false);
		m_Controller->Render();
	}
}

/// <summary>
/// Pause or resume the render process.
/// </summary>
/// <param name="checked">Ignored</param>
void FractoriumFinalRenderDialog::OnPauseClicked(bool checked)
{
	if (m_Controller.get())
		if (m_Controller->Running())
			Pause(!m_Controller->Paused());
}

/// <summary>
/// Cancel the render.
/// </summary>
/// <param name="checked">Ignored</param>
void FractoriumFinalRenderDialog::OnCancelRenderClicked(bool checked)
{
	if (m_Controller.get())
	{
		Pause(false);
		m_Controller->CancelRender();
	}
}

/// <summary>
/// Pause or resume the render process.
/// </summary>
/// <param name="checked">True to pause, else resume.</param>
void FractoriumFinalRenderDialog::Pause(bool paused)
{
	if (paused)
	{
		m_Controller->Pause(true);
		ui.FinalRenderPauseButton->setText("Resume");
	}
	else
	{
		m_Controller->Pause(false);
		ui.FinalRenderPauseButton->setText("Pause");
	}

	ui.FinalRenderStartButton->setEnabled(!paused);
}

/// <summary>
/// Uses the options and the ember to populate widgets.
/// Called when the dialog is initially shown.
/// </summary>
/// <param name="e">The event</param>
void FractoriumFinalRenderDialog::showEvent(QShowEvent* e)
{
	if (m_Controller.get())//On Linux, this event will be called when the main window minimized/maximized, so filter it out if the window and controller have already been created.
		return;

	QString firstfile;

	if (CreateControllerFromGUI(true))//Create controller if it does not exist, or if it does and the renderer is not running.
	{
		//If animating, then always start at 0 because the entire sequence usually needs to be rendered and if they need to start at a specific frame, they can use the Start At checkbox.
		const auto index = m_FromSequence ? 0 : static_cast<int>(m_Fractorium->m_Controller->Index()) + 1;
#ifdef DO_DOUBLE
		Ember<double> ed;
		EmberFile<double> efi;
		m_Fractorium->m_Controller->CopyEmberFile(efi, m_FromSequence, [&](Ember<double>& ember)
		{
			ember.SyncSize();
			ember.m_Quality = m_Settings->FinalQuality();
			ember.m_Supersample = m_Settings->FinalSupersample();
			ember.m_TemporalSamples = m_Settings->FinalTemporalSamples();
		});//Copy the whole file, will take about 0.2ms per ember in the file.
		firstfile = efi.m_Filename;
#else
		Ember<float> ed;
		EmberFile<float> efi;
		m_Fractorium->m_Controller->CopyEmberFile(efi, m_FromSequence, [&](Ember<float>& ember)
		{
			ember.SyncSize();
			ember.m_Quality = m_Settings->FinalQuality();
			ember.m_Supersample = m_Settings->FinalSupersample();
			ember.m_TemporalSamples = m_Settings->FinalTemporalSamples();
		});//Copy the whole file, will take about 0.2ms per ember in the file.
		firstfile = efi.m_Filename;
#endif
		m_Controller->SetEmberFile(efi, true);//Move the temp file into the final render controller.
		ui.FinalRenderCurrentSpin->setMaximum(static_cast<int>(efi.Size()));
		ui.FinalRenderCurrentSpin->blockSignals(true);
		ui.FinalRenderCurrentSpin->setValue(index);//Set the currently selected ember to the one that was being edited.
		ui.FinalRenderCurrentSpin->blockSignals(false);
		OnCurrentSpinChanged(index);//Force update in case the ember was new, but at the same index as the previous one.
		m_Controller->m_ImageCount = 0;
		SetMemory();
		m_Controller->ResetProgress();
		QString s = m_Settings->SaveFolder();

		if (Exists(s))
			Path(m_Controller->ComposePath(m_Controller->Name()));//Update the GUI.
	}

	if (m_FromSequence)
	{
		ui.FinalRenderDoAllCheckBox->setChecked(true);
		//ui.FinalRenderDoSequenceCheckBox->setChecked(true);
		ui.FinalRenderApplyToAllCheckBox->setChecked(true);
		//m_PrefixEdit->setText(firstfile + "_");
	}

	ui.FinalRenderTextOutput->clear();
	QDialog::showEvent(e);
}

/// <summary>
/// Close the dialog without running, or if running, cancel and exit.
/// Settings will not be saved.
/// Control will be returned to Fractorium::OnActiOn().
/// </summary>
void FractoriumFinalRenderDialog::reject()
{
	if (m_Controller.get())
	{
		m_Controller->CancelRender();
		m_Controller->DeleteRenderer();
	}

	QDialog::reject();
}

/// <summary>
/// Create the controller from the options and optionally its underlying renderer.
/// </summary>
/// <returns>True if successful, else false.</returns>
bool FractoriumFinalRenderDialog::CreateControllerFromGUI(bool createRenderer)
{
	const auto index = Current() - 1;
#ifdef DO_DOUBLE
	size_t elementSize = Double() ? sizeof(double) : sizeof(float);
#else
	size_t elementSize = sizeof(float);
#endif

	if (!m_Controller.get() || (m_Controller->SizeOfT() != elementSize))
	{
#ifdef DO_DOUBLE
		Ember<double> ed;
		Ember<double> orig;
		EmberFile<double> efd;
#else
		Ember<float> ed;
		Ember<float> orig;
		EmberFile<float> efd;
#endif

		//First check if a controller has already been created, and if so, save its embers and gracefully shut it down.
		if (m_Controller.get())
		{
#ifdef DO_DOUBLE
			m_Controller->CopyEmberFile(efd, false, [&](Ember<double>& ember) { });//Convert float to double or save double verbatim;
#else
			m_Controller->CopyEmberFile(efd, false, [&](Ember<float>& ember) { });//Convert float to double or save double verbatim;
#endif
			m_Controller->Shutdown();
		}

		//Create a float or double controller based on the GUI.
#ifdef DO_DOUBLE

		if (Double())
			m_Controller = unique_ptr<FinalRenderEmberControllerBase>(new FinalRenderEmberController<double>(this));
		else
#endif
			m_Controller = unique_ptr<FinalRenderEmberControllerBase>(new FinalRenderEmberController<float>(this));

		//Restore the ember and ember file.
		if (m_Controller.get())
		{
			m_Controller->SetEmberFile(efd, true);//Convert float to double and move or move double verbatim.
			m_Controller->SetEmber(index, false);
		}
	}

	if (m_Controller.get())
	{
		if (createRenderer)
			return m_Controller->CreateRendererFromGUI();
		else
			return true;
	}
	else
		return false;
}

/// <summary>
/// Compute the amount of memory needed via call to SyncAndComputeMemory(), then
/// assign the result to the table cell as text.
/// Report errors if not enough memory is available for any of the selected devices.
/// </summary>
/// <returns>True if devices and a controller is present, else false.</returns>
bool FractoriumFinalRenderDialog::SetMemory()
{
	if (isVisible() && CreateControllerFromGUI(true))
	{
		const auto p = m_Controller->SyncAndComputeMemory();
		ui.FinalRenderParamsTable->item(m_MemoryCellIndex, 1)->setText(ToString<qulonglong>(get<1>(p)));
		ui.FinalRenderParamsTable->item(m_ItersCellIndex, 1)->setText(ToString<qulonglong>(get<2>(p)));

		if (OpenCL())
			ui.FinalRenderTextOutput->setText(m_Controller->CheckMemory(p));
		else
			ui.FinalRenderTextOutput->clear();

		ui.FinalRenderSaveAgainAsButton->setEnabled(false);
		ui.FinalRenderBumpQualityStartButton->setEnabled(false);
		return true;
	}

	return false;
}
