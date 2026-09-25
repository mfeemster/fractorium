#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the parameters UI.
/// </summary>
void Fractorium::InitParamsUI()
{
	int row = 0;
	int spinHeight = 20;
	double dmax = numeric_limits<double>::max();
	int imax = numeric_limits<int>::max();
	vector<string> comboVals, llComboVals;
	QTableWidget* table = ui.ColorTable;
	//Because QTableWidget does not allow for a single title bar/header
	//at the top of a multi-column table, the workaround hack is to just
	//make another single column table with no rows, and use the single
	//column header as the title bar. Then positioning it right above the table
	//that holds the data. Disallow selecting and resizing of the title bar.
	SetFixedTableHeader(ui.ColorTableHeader->horizontalHeader());
	SetFixedTableHeader(ui.GeometryTableHeader->horizontalHeader());
	SetFixedTableHeader(ui.FilterTableHeader->horizontalHeader());
	SetFixedTableHeader(ui.IterationTableHeader->horizontalHeader());
	SetFixedTableHeader(ui.AnimationTableHeader->horizontalHeader());
	//Color.
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_BrightnessSpin,     spinHeight, 0.01, dmax,       1, SIGNAL(valueChanged(double)), SLOT(OnBrightnessChanged(double)),     true,  4.0,    4.0,  4.0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_GammaSpin,          spinHeight,    1, dmax,     0.5, SIGNAL(valueChanged(double)), SLOT(OnGammaChanged(double)),          true,  4.0,    4.0,  4.0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_GammaThresholdSpin, spinHeight,    0, dmax,    0.01, SIGNAL(valueChanged(double)), SLOT(OnGammaThresholdChanged(double)), true,  0.1,    0.1,  0.0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_VibrancySpin,       spinHeight,    0, dmax,    0.01, SIGNAL(valueChanged(double)), SLOT(OnVibrancyChanged(double)),       true,  1.0,    1.0,  0.0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_HighlightSpin,      spinHeight, -1.0,   10,     0.1, SIGNAL(valueChanged(double)), SLOT(OnHighlightPowerChanged(double)), true,  1.0,    1.0, -1.0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_K2Spin,             spinHeight,    0, 99.0,  0.0001, SIGNAL(valueChanged(double)), SLOT(OnK2Changed(double)),             true,    0, 0.0001,    0);
	m_HighlightSpin->DoubleClickLowVal(-1.0);
	int dec = 6;
	m_BrightnessSpin->setDecimals(dec);
	m_GammaSpin->setDecimals(dec);
	m_GammaThresholdSpin->setDecimals(dec);
	m_VibrancySpin->setDecimals(dec);
	m_HighlightSpin->setDecimals(dec);
	m_K2Spin->setDecimals(dec);
	m_BgRow = row;
	m_BackgroundColorButton = new QPushButton("...", table);
	m_BackgroundColorButton->setMinimumWidth(21);
	m_BackgroundColorButton->setMaximumWidth(21);
	table->setCellWidget(row, 1, m_BackgroundColorButton);
	table->item(row, 1)->setTextAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
	connect(m_BackgroundColorButton, SIGNAL(clicked(bool)), this, SLOT(OnBackgroundColorButtonClicked(bool)), Qt::ConnectionType::QueuedConnection);
	row++;
	comboVals.push_back("Step");
	comboVals.push_back("Linear");
	SetupCombo(table, this, row, 1, m_PaletteModeCombo, comboVals, SIGNAL(currentIndexChanged(int)), SLOT(OnPaletteModeComboCurrentIndexChanged(int)));
	m_PaletteModeCombo->SetCurrentIndexStealth(static_cast<int>(ePaletteMode::PALETTE_LINEAR));
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//Geometry.
	row = 0;
	table = ui.GeometryTable;
	 SetupSpinner<SpinBox, int>         (table, this, row, 1, m_WidthSpin,       spinHeight,    10,   2048,     50, SIGNAL(valueChanged(int)),  SLOT(OnWidthChanged(int)),            true,  width(),  width(),  width());
	 SetupSpinner<SpinBox, int>         (table, this, row, 1, m_HeightSpin,      spinHeight,    10,   2048,     50, SIGNAL(valueChanged(int)),  SLOT(OnHeightChanged(int)),           true, height(), height(), height());
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_CenterXSpin,     spinHeight, -dmax,    dmax,  0.05, SIGNAL(valueChanged(double)), SLOT(OnCenterXChanged(double)),     true,        0,        0,        0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_CenterYSpin,     spinHeight, -dmax,    dmax,  0.05, SIGNAL(valueChanged(double)), SLOT(OnCenterYChanged(double)),     true,        0,        0,        0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_ScaleSpin,       spinHeight,    10,    dmax,    20, SIGNAL(valueChanged(double)), SLOT(OnScaleChanged(double)),       true,      240,      240,      240);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_ZoomSpin,        spinHeight,     0,      25,   0.2, SIGNAL(valueChanged(double)), SLOT(OnZoomChanged(double)),        true,        0,        0,        0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_RotateSpin,      spinHeight, -dmax,    dmax,    10, SIGNAL(valueChanged(double)), SLOT(OnRotateChanged(double)),      true,        0,        0,        0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_ZPosSpin,        spinHeight, -1000,    1000,   0.1, SIGNAL(valueChanged(double)), SLOT(OnZPosChanged(double)),        true,        0,        1,        0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_PerspectiveSpin, spinHeight,  -500,     500,  0.01, SIGNAL(valueChanged(double)), SLOT(OnPerspectiveChanged(double)), true,        0,        1,        0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_PitchSpin,       spinHeight, -dmax,    dmax,     1, SIGNAL(valueChanged(double)), SLOT(OnPitchChanged(double)),       true,        0,       45,        0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_YawSpin,         spinHeight, -dmax,    dmax,     1, SIGNAL(valueChanged(double)), SLOT(OnYawChanged(double)),         true,        0,       45,        0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_DepthBlurSpin,   spinHeight, -dmax,    dmax,  0.01, SIGNAL(valueChanged(double)), SLOT(OnDepthBlurChanged(double)),   true,        0,        1,        0);
	 SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_BlurCurveSpin,   spinHeight,     0,    dmax,   0.1, SIGNAL(valueChanged(double)), SLOT(OnBlurCurveChanged(double)),   true,        0,        1,        0);
	m_WidthSpin->m_DoubleClickNonZeroEvent = [&](SpinBox * sb, int val)
	{
		m_Controller->ResizeAndScale(val, m_HeightSpin->DoubleClickNonZero(), eScaleType::SCALE_WIDTH);
		m_HeightSpin->SetValueStealth(m_HeightSpin->DoubleClickNonZero());
	};
	m_HeightSpin->m_DoubleClickNonZeroEvent = [&](SpinBox * sb, int val)
	{
		m_Controller->ResizeAndScale(m_WidthSpin->DoubleClickNonZero(), val, eScaleType::SCALE_HEIGHT);
		m_WidthSpin->SetValueStealth(m_WidthSpin->DoubleClickNonZero());
	};
	dec = 4;
	m_CenterXSpin->setDecimals(dec);
	m_CenterYSpin->setDecimals(dec);
	m_ZPosSpin->setDecimals(dec);
	m_PerspectiveSpin->setDecimals(dec);
	m_DepthBlurSpin->setDecimals(dec);
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//Filter.
	row = 0;
	table = ui.FilterTable;
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_SpatialFilterWidthSpin, spinHeight,   0, 2, 0.1, SIGNAL(valueChanged(double)), SLOT(OnSpatialFilterWidthChanged(double)), true, 1.0, 1.0, 0);
	comboVals = SpatialFilterCreator<float>::FilterTypes();
	SetupCombo(table, this, row, 1, m_SpatialFilterTypeCombo, comboVals, SIGNAL(currentTextChanged(const QString&)), SLOT(OnSpatialFilterTypeComboCurrentIndexChanged(const QString&)));
	m_SpatialFilterTypeCombo->SetCurrentIndexStealth(0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_DEFilterMinRadiusSpin, spinHeight,    0, 25,   1, SIGNAL(valueChanged(double)), SLOT(OnDEFilterMinRadiusWidthChanged(double)), true,   0,   0,   0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_DEFilterMaxRadiusSpin, spinHeight,    0, 25,   1, SIGNAL(valueChanged(double)), SLOT(OnDEFilterMaxRadiusWidthChanged(double)), true, 0.0, 9.0,   0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_DECurveSpin,			  spinHeight, 0.15,  5, 0.1, SIGNAL(valueChanged(double)), SLOT(OnDEFilterCurveWidthChanged(double)),     true, 0.4, 0.4, 0.4);
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//Iteration.
	row = 0;
	table = ui.IterationTable;
	auto quality = m_Settings->OpenCL() ? m_Settings->OpenClQuality() : m_Settings->CpuQuality();
	SetupSpinner<SpinBox, int>(			table, this, row, 1, m_SbsSpin,				spinHeight, 1000, 100000, 100, SIGNAL(valueChanged(int)),	 SLOT(OnSbsChanged(int)),		   true, DEFAULT_SBS, DEFAULT_SBS, DEFAULT_SBS);
	SetupSpinner<SpinBox, int>(			table, this, row, 1, m_FuseSpin,			spinHeight, 1,      1000,   5, SIGNAL(valueChanged(int)),	 SLOT(OnFuseChanged(int)),	       true,	     15, 100, 15);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_RandRangeSpin,		spinHeight, 0.01,   1000, 0.1, SIGNAL(valueChanged(double)), SLOT(OnRandRangeChanged(double)), true,	      1,  10,  1);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_QualitySpin,			spinHeight, 1,      dmax,  50, SIGNAL(valueChanged(double)), SLOT(OnQualityChanged(double)),   true,     quality, 10, 10);
	SetupSpinner<SpinBox, int>(         table, this, row, 1, m_SupersampleSpin,		spinHeight, 1,         4,   1, SIGNAL(valueChanged(int)),	 SLOT(OnSupersampleChanged(int)),  true,           1,  2,  1);
	m_RandRangeSpin->DoubleClickLowVal(1);
	m_RandRangeSpin->setDecimals(4);
	m_FuseSpin->DoubleClickLowVal(15);
	m_SupersampleSpin->DoubleClickLowVal(1);
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//Animation.
	row = 0;
	table = ui.AnimationTable;
	comboVals.clear();
	comboVals.push_back("Linear");
	comboVals.push_back("Smooth");
	SetupCombo(table, this, row, 1, m_InterpTypeCombo, comboVals, SIGNAL(currentIndexChanged(int)), SLOT(OnInterpTypeComboCurrentIndexChanged(int)));
	m_InterpTypeCombo->SetCurrentIndexStealth(static_cast<int>(eInterp::EMBER_INTERP_SMOOTH));
	comboVals.clear();
	comboVals.push_back("Linear");
	comboVals.push_back("Log");
	llComboVals = comboVals;
	SetupCombo(table, this, row, 1, m_AffineInterpTypeCombo, comboVals, SIGNAL(currentIndexChanged(int)), SLOT(OnAffineInterpTypeComboCurrentIndexChanged(int)));
	m_AffineInterpTypeCombo->SetCurrentIndexStealth(static_cast<int>(eAffineInterp::AFFINE_INTERP_LOG));
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_RotationsSpin,			spinHeight, 0, dmax, 1, SIGNAL(valueChanged(double)), SLOT(OnRotationsChanged(double)),          true, 1.0, 1.0, 0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_SecondsPerRotationSpin,	spinHeight, 0, dmax, 1, SIGNAL(valueChanged(double)), SLOT(OnSecondsPerRotationChanged(double)), true, 1.0, 1.0, 0);
	comboVals.clear();
	comboVals.push_back("Cw");
	comboVals.push_back("Ccw");
	SetupCombo(table, this, row, 1, m_RotateXformsDirCombo, comboVals, SIGNAL(currentIndexChanged(int)), SLOT(OnRotateXformsDirComboCurrentIndexChanged(int)));
	m_RotateXformsDirCombo->SetCurrentIndexStealth(0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_BlendSecondsSpin,	  spinHeight, 0, dmax,  1, SIGNAL(valueChanged(double)), SLOT(OnBlendSecondsChanged(double)),   true, 1.0, 1.0, 0);
	SetupSpinner<SpinBox, int>(         table, this, row, 1, m_RotationsPerBlendSpin, spinHeight, 0, imax,  1, SIGNAL(valueChanged(int)),    SLOT(OnRotationsPerBlendChanged(int)), true,   0,   1, 0);
	SetupCombo(table, this, row, 1, m_BlendXformsRotateDirCombo, comboVals, SIGNAL(currentIndexChanged(int)), SLOT(OnBlendXformsRotateDirComboCurrentIndexChanged(int)));
	m_BlendXformsRotateDirCombo->SetCurrentIndexStealth(0);
	SetupCombo(table, this, row, 1, m_BlendInterpTypeCombo, llComboVals, SIGNAL(currentIndexChanged(int)), SLOT(OnBlendInterpTypeComboCurrentIndexChanged(int)));
	m_BlendInterpTypeCombo->SetCurrentIndexStealth(1);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_StaggerSpin,	            spinHeight, 0,  1, 0.1, SIGNAL(valueChanged(double)), SLOT(OnStaggerChanged(double)),             true, 0, 1.0, 0);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_TemporalFilterWidthSpin, spinHeight, 1, 10,   1, SIGNAL(valueChanged(double)), SLOT(OnTemporalFilterWidthChanged(double)), true, 1,   1, 1);
	comboVals = TemporalFilterCreator<float>::FilterTypes();
	SetupCombo(table, this, row, 1, m_TemporalFilterTypeCombo, comboVals, SIGNAL(currentTextChanged(const QString&)), SLOT(OnTemporalFilterTypeComboCurrentIndexChanged(const QString&)));
	m_TemporalFilterTypeCombo->SetCurrentIndexStealth(static_cast<int>(eTemporalFilterType::BOX_TEMPORAL_FILTER));
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	SetupSpinner<DoubleSpinBox, double>(table, this, row, 1, m_TemporalFilterExpSpin, spinHeight, 0,  5, 0.1, SIGNAL(valueChanged(double)), SLOT(OnExpChanged(double)), true, 1, 1, 0);
	AddSizePreset("HD", 1920, 1080);
	AddSizePreset("QHD", 2560, 1440);
	AddSizePreset("4K UHD", 3840, 2160);
	AddSizePreset("4K DCP", 4096, 2160);
	AddSizePreset("5K", 5120, 2880);
	AddSizePreset("6K", 6144, 3456);
	AddSizePreset("8K UHD", 7680, 4320);
	AddSizePreset("8K", 8192, 4608);
	AddSizePreset("12K", 12288, 6912);
	m_WidthSpin->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);
	m_HeightSpin->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);
}

/// <summary>
/// Add a new size preset.
/// </summary>
/// <param name="name">The name of the preset</param>
/// <param name="w">The width of the preset</param>
/// <param name="h">The height of the preset</param>
void Fractorium::AddSizePreset(QString name, int w, int h)
{
	QString caption;
	QTextStream(&caption) << name << " (" << ToString<int>(w) << " x " << ToString<int>(h) << ")";
	auto widthAction = new QAction(caption, m_WidthSpin);
	connect(widthAction, SIGNAL(triggered(bool)), this, SLOT(PresetWidthActionTriggered(bool)), Qt::ConnectionType::QueuedConnection);
	m_WidthSpin->addAction(widthAction);
	auto heightAction = new QAction(caption, m_HeightSpin);
	connect(heightAction, SIGNAL(triggered(bool)), this, SLOT(PresetHeightActionTriggered(bool)), Qt::ConnectionType::QueuedConnection);
	m_HeightSpin->addAction(heightAction);
	m_HeightPresets[caption] = std::pair<int, int>(w, h);
}

/// <summary>
/// Assign a new width and height, and scale by the width.
/// </summary>
/// <param name="w">The width to assign</param>
/// <param name="h">The height to assign</param>
void Fractorium::SetWidthWithAspect(int w, int h)
{
	m_Controller->ResizeAndScale(w, h, eScaleType::SCALE_WIDTH);
	m_WidthSpin->SetValueStealth(w);
	m_HeightSpin->SetValueStealth(h);
}

/// <summary>
/// Assign a new width and height, and scale by the height.
/// </summary>
/// <param name="w">The width to assign</param>
/// <param name="h">The height to assign</param>
void Fractorium::SetHeightWithAspect(int w, int h)
{
	m_Controller->ResizeAndScale(w, h, eScaleType::SCALE_HEIGHT);
	m_WidthSpin->SetValueStealth(w);
	m_HeightSpin->SetValueStealth(h);
}

/// <summary>
/// Return whether the apply all checkbox is checked.
/// </summary>
/// <returns>True if checked, else false.</returns>
bool Fractorium::ApplyAll()
{
	return ui.ApplyAllParamsCheckBox->isChecked();
}

/// <summary>
/// Color.
/// </summary>

/// <summary>
/// Set the brightness to be used for calculating K1 and K2 for filtering and final accum.
/// Called when the brightness spinner is changed.
/// Resets the rendering process to the filtering stage.
/// </summary>
/// <param name="d">The brightness</param>
template <typename T>
void FractoriumEmberController<T>::BrightnessChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_Brightness = d;
	}, true, eProcessAction::FILTER_AND_ACCUM, m_Fractorium->ApplyAll());
}
void Fractorium::OnBrightnessChanged(double d) { m_Controller->BrightnessChanged(d); }

/// <summary>
/// Set the gamma to be used for final accum.
/// Called when the gamma spinner is changed.
/// Resets the rendering process if temporal samples is greater than 1,
/// else if early clip is true, filter and accum, else final accum only.
/// </summary>
/// <param name="d">The gamma value</param>
template <typename T> void FractoriumEmberController<T>::GammaChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_Gamma = d;
	}, true, m_Renderer->EarlyClip() ? eProcessAction::FILTER_AND_ACCUM : eProcessAction::ACCUM_ONLY, m_Fractorium->ApplyAll());
}
void Fractorium::OnGammaChanged(double d) { m_Controller->GammaChanged(d); }

/// <summary>
/// Set the gamma threshold to be used for final accum.
/// Called when the gamma threshold spinner is changed.
/// Resets the rendering process to the final accumulation stage.
/// </summary>
/// <param name="d">The gamma threshold</param>
template <typename T> void FractoriumEmberController<T>::GammaThresholdChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_GammaThresh = d;
	}, true, m_Renderer->EarlyClip() ? eProcessAction::FILTER_AND_ACCUM : eProcessAction::ACCUM_ONLY, m_Fractorium->ApplyAll());
}
void Fractorium::OnGammaThresholdChanged(double d) { m_Controller->GammaThresholdChanged(d); }

/// <summary>
/// Set the vibrancy to be used for final accum.
/// Called when the vibrancy spinner is changed.
/// Resets the rendering process to the final accumulation stage if temporal samples is 1, else full reset.
/// </summary>
/// <param name="d">The vibrancy</param>
template <typename T> void FractoriumEmberController<T>::VibrancyChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_Vibrancy = d;
	}, true, m_Renderer->EarlyClip() ? eProcessAction::FILTER_AND_ACCUM : eProcessAction::ACCUM_ONLY, m_Fractorium->ApplyAll());
}
void Fractorium::OnVibrancyChanged(double d) { m_Controller->VibrancyChanged(d); }

/// <summary>
/// Set the highlight power to be used for final accum.
/// Called when the highlight power spinner is changed.
/// Resets the rendering process to the final accumulation stage.
/// </summary>
/// <param name="d">The highlight power</param>
template <typename T> void FractoriumEmberController<T>::HighlightPowerChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_HighlightPower = d;
	}, true, m_Renderer->EarlyClip() ? eProcessAction::FILTER_AND_ACCUM : eProcessAction::ACCUM_ONLY, m_Fractorium->ApplyAll());
}
void Fractorium::OnHighlightPowerChanged(double d) { m_Controller->HighlightPowerChanged(d); }

/// <summary>
/// Set the k2 brightness value to be used for final accum.
/// Called when the k2 spinner is changed.
/// Resets the rendering process to the final accumulation stage.
/// </summary>
/// <param name="d">The k2 value</param>
template <typename T> void FractoriumEmberController<T>::K2Changed(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_K2 = d;
	}, true, eProcessAction::FILTER_AND_ACCUM, m_Fractorium->ApplyAll());
}
void Fractorium::OnK2Changed(double d) { m_Controller->K2Changed(d); }

/// <summary>
/// Show the color selection dialog.
/// Called when the background color button is clicked.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnBackgroundColorButtonClicked(bool checked)
{
	m_ColorDialog->exec();
}

/// <summary>
/// Set a new ember background color when the user accepts the color dialog.
/// Also change the background and foreground colors of the color cell in the
/// color params table.
/// Resets the rendering process.
/// </summary>
/// <param name="color">The color to set, RGB in the 0-255 range</param>
template <typename T>
void FractoriumEmberController<T>::BackgroundChanged(const QColor& color)
{
	const auto itemRow = m_Fractorium->m_BgRow;
	const auto colorTable = m_Fractorium->ui.ColorTable;
	const auto r = ToString(color.red());
	const auto g = ToString(color.green());
	const auto b = ToString(color.blue());
	colorTable->item(itemRow, 1)->setBackground(color);
	colorTable->item(itemRow, 1)->setForeground(VisibleColor(color));
	colorTable->item(itemRow, 1)->setText("rgb(" + r + ", " + g + ", " + b + ")");
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		//Color is 0-255, normalize to 0-1.
		ember.m_Background.r = color.red()   / 255.0;
		ember.m_Background.g = color.green() / 255.0;
		ember.m_Background.b = color.blue()  / 255.0;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnColorSelected(const QColor& color) { m_Controller->BackgroundChanged(color); }

/// <summary>
/// Set the palette index interpolation mode.
/// Called when the palette mode combo box index is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="index">The index of the palette mode combo box</param>
template <typename T> void FractoriumEmberController<T>::PaletteModeChanged(uint i)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_PaletteMode = i == 0 ? ePaletteMode::PALETTE_STEP : ePaletteMode::PALETTE_LINEAR;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnPaletteModeComboCurrentIndexChanged(int index) { m_Controller->PaletteModeChanged(index); }

/// <summary>
/// Geometry.
/// </summary>

/// <summary>
/// Set the width of the ember in pixels to the passed in value.
/// Called when the width spinner is changed in a manner other than double clicking.
/// Resets the rendering process.
/// </summary>
/// <param name="i">The width value in pixels to set</param>
template <typename T> void FractoriumEmberController<T>::WidthChanged(uint i)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_FinalRasW = i;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnWidthChanged(int i) { m_Controller->WidthChanged(i); }

/// <summary>
/// Set the height of the ember in pixels to the passed in value.
/// Called when the height spinner is changed in a manner other than double clicking.
/// Resets the rendering process.
/// </summary>
/// <param name="i">The height value in pixels to set</param>
template <typename T> void FractoriumEmberController<T>::HeightChanged(uint i)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_FinalRasH = i;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnHeightChanged(int i) { m_Controller->HeightChanged(i); }

/// <summary>
/// Change the width and height to the value specified in the preset, and scale relative to the original width.
/// Called when the popup menu is clicked in the width spinner.
/// Resets the rendering process.
/// </summary>
/// <param name="b">Ignored</param>
void Fractorium::PresetWidthActionTriggered(bool b)
{
	const auto act = qobject_cast<QAction*>(sender());

	if (act)
	{
		auto it = m_HeightPresets.find(act->text());

		if (it != m_HeightPresets.end())
		{
			SetWidthWithAspect(it->second.first, it->second.second);
		}
	}
}

/// <summary>
/// Change the width and height to the value specified in the preset, and scale relative to the original height.
/// Called when the popup menu is clicked in the height spinner.
/// Resets the rendering process.
/// </summary>
/// <param name="b">Ignored</param>
void Fractorium::PresetHeightActionTriggered(bool b)
{
	const auto act = qobject_cast<QAction*>(sender());

	if (act)
	{
		auto it = m_HeightPresets.find(act->text());

		if (it != m_HeightPresets.end())
		{
			SetHeightWithAspect(it->second.first, it->second.second);
		}
	}
}

/// <summary>
/// Set the width and height of the ember in pixels to the passed in values.
/// Called when either the width or height spinners are double clicked.
/// Because this will change the scale value, the scale spinner gets a stealth update.
/// For this reason, the affine scales are reset, even though they are not when doing a manual
/// height or width adjustment.
/// Resets the rendering process.
/// </summary>
/// <param name="width">The width value in pixels to set</param>
/// <param name="height">The height value in pixels to set</param>
/// <param name="scaleType">The height value in pixels to set</param>
template <typename T>
void FractoriumEmberController<T>::ResizeAndScale(int width, int height, eScaleType scaleType)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.SetSizeAndAdjustScale(width, height, false, scaleType);
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
	m_Fractorium->m_ScaleSpin->SetValueStealth(m_Ember.m_PixelsPerUnit);
	m_Fractorium->OnActionResetScale(true);
}

/// <summary>
/// Set the x offset applied to the center of the image.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The x offset value</param>
template <typename T> void FractoriumEmberController<T>::CenterXChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_CenterX = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnCenterXChanged(double d) { m_Controller->CenterXChanged(d); }

/// <summary>
/// Set the y offset applied to the center of the image.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The y offset value</param>
template <typename T> void FractoriumEmberController<T>::CenterYChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_CenterY = ember.m_RotCenterY = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnCenterYChanged(double d) { m_Controller->CenterYChanged(d); }

/// <summary>
/// Set the scale (pixels per unit) value of the image.
/// Note this will not increase the number of iters ran, but will degrade quality.
/// To preserve quality, but exponentially increase iters, use zoom.
/// Called when the scale spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The scale value</param>
template <typename T> void FractoriumEmberController<T>::ScaleChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_PixelsPerUnit = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnScaleChanged(double d) { m_Controller->ScaleChanged(d); }

/// <summary>
/// Set the zoom value of the image.
/// Note this will increase the number of iters ran exponentially.
/// To zoom in without increasing iters, but sacrifice quality, use scale.
/// Called when the zoom spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The zoom value</param>
template <typename T> void FractoriumEmberController<T>::ZoomChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_Zoom = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnZoomChanged(double d) { m_Controller->ZoomChanged(d); }

/// <summary>
/// Set the angular rotation of the image.
/// Called when the rotate spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The rotation in degrees</param>
template <typename T> void FractoriumEmberController<T>::RotateChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_Rotate = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnRotateChanged(double d) { m_Controller->RotateChanged(d); }

/// <summary>
/// Set the 3D z position of the image.
/// Called when the 3D zpos spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The 3D zpos in world space units</param>
template <typename T> void FractoriumEmberController<T>::ZPosChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_CamZPos = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnZPosChanged(double d) {  m_Controller->ZPosChanged(d); }

/// <summary>
/// Set the 3D persepctive of the image.
/// Called when the 3D persepctive spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The 3D perspective in world space units</param>
template <typename T> void FractoriumEmberController<T>::PerspectiveChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_CamPerspective = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnPerspectiveChanged(double d) { m_Controller->PerspectiveChanged(d); }

/// <summary>
/// Set the 3D pitch of the image.
/// Called when the 3D pitch spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The 3D pitch in degrees</param>
template <typename T> void FractoriumEmberController<T>::PitchChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_CamPitch = d * DEG_2_RAD;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnPitchChanged(double d) { m_Controller->PitchChanged(d); }

/// <summary>
/// Set the 3D yaw of the image.
/// Called when the 3D yaw spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The 3D yaw in degrees</param>
template <typename T> void FractoriumEmberController<T>::YawChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_CamYaw = d * DEG_2_RAD;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnYawChanged(double d) { m_Controller->YawChanged(d); }

/// <summary>
/// Set the 3D depth blur of the image.
/// Called when the 3D depth blur spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The 3D depth blur</param>
template <typename T> void FractoriumEmberController<T>::DepthBlurChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_CamDepthBlur = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnDepthBlurChanged(double d) { m_Controller->DepthBlurChanged(d); }

/// <summary>
/// Set the 3D blur curve used to calculate the 3D depth blur of the image.
/// Called when the blur curve spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The 3D blur curve</param>
template <typename T> void FractoriumEmberController<T>::BlurCurveChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_BlurCurve = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnBlurCurveChanged(double d) { m_Controller->BlurCurveChanged(d); }

/// <summary>
/// Filter.
/// </summary>

/// <summary>
/// Set the spatial filter width.
/// Called when the spatial filter width spinner is changed.
/// Resets the rendering process to density filtering if early clip is used, else to final accumulation.
/// </summary>
/// <param name="d">The spatial filter width</param>
template <typename T> void FractoriumEmberController<T>::SpatialFilterWidthChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_SpatialFilterRadius = d;
	}, true, m_Renderer->EarlyClip() ? eProcessAction::FILTER_AND_ACCUM : eProcessAction::ACCUM_ONLY, m_Fractorium->ApplyAll());
}
void Fractorium::OnSpatialFilterWidthChanged(double d) { m_Controller->SpatialFilterWidthChanged(d); }

/// <summary>
/// Set the spatial filter type.
/// Called when the spatial filter type combo box index is changed.
/// Resets the rendering process to density filtering if early clip is used, else to final accumulation.
/// </summary>
/// <param name="text">The spatial filter type</param>
template <typename T> void FractoriumEmberController<T>::SpatialFilterTypeChanged(const QString& text)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_SpatialFilterType = SpatialFilterCreator<T>::FromString(text.toStdString());
	}, true, m_Renderer->EarlyClip() ? eProcessAction::FILTER_AND_ACCUM : eProcessAction::ACCUM_ONLY, m_Fractorium->ApplyAll());
}
void Fractorium::OnSpatialFilterTypeComboCurrentIndexChanged(const QString& text) { m_Controller->SpatialFilterTypeChanged(text); }

/// <summary>
/// Set the density estimation filter min radius value.
/// Resets the rendering process to density filtering.
/// </summary>
/// <param name="d">The min radius value</param>
template <typename T>
void FractoriumEmberController<T>::DEFilterMinRadiusWidthChanged(double d)
{
	if (m_Ember.m_MinRadDE != d)
	{
		UpdateAll([&](Ember<T>& ember, bool isMain)
		{
			ember.m_MinRadDE = d;
		}, true, eProcessAction::FILTER_AND_ACCUM, m_Fractorium->ApplyAll());
	}
}

void Fractorium::OnDEFilterMinRadiusWidthChanged(double d)
{
	ConstrainLow(m_DEFilterMinRadiusSpin, m_DEFilterMaxRadiusSpin);
	m_Controller->DEFilterMinRadiusWidthChanged(m_DEFilterMinRadiusSpin->value());
}

/// <summary>
/// Set the density estimation filter max radius value.
/// Resets the rendering process to density filtering.
/// </summary>
/// <param name="d">The max radius value</param>
template <typename T>
void FractoriumEmberController<T>::DEFilterMaxRadiusWidthChanged(double d)
{
	if (m_Ember.m_MaxRadDE != d)
	{
		UpdateAll([&](Ember<T>& ember, bool isMain)
		{
			ember.m_MaxRadDE = d;
		}, true, eProcessAction::FILTER_AND_ACCUM, m_Fractorium->ApplyAll());
	}
}

void Fractorium::OnDEFilterMaxRadiusWidthChanged(double d)
{
	ConstrainHigh(m_DEFilterMinRadiusSpin, m_DEFilterMaxRadiusSpin);
	m_Controller->DEFilterMaxRadiusWidthChanged(m_DEFilterMaxRadiusSpin->value());
}

/// <summary>
/// Set the density estimation filter curve value.
/// Resets the rendering process to density filtering.
/// </summary>
/// <param name="d">The curve value</param>
template <typename T> void FractoriumEmberController<T>::DEFilterCurveWidthChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_CurveDE = d;
	}, true, eProcessAction::FILTER_AND_ACCUM, m_Fractorium->ApplyAll());
}
void Fractorium::OnDEFilterCurveWidthChanged(double d) { m_Controller->DEFilterCurveWidthChanged(d); }

/// <summary>
/// Iteration.
/// </summary>

/// <summary>
/// Set the iteration depth.
/// Called when the sub batch size spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The sub batch size value to set</param>
template <typename T> void FractoriumEmberController<T>::SbsChanged(int d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_SubBatchSize = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnSbsChanged(int d) { m_Controller->SbsChanged(d); }

/// <summary>
/// Set the range from which to chose the starting random points, as well as point resets due to bad points.
/// Called when the rand range spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The sub batch size value to set</param>
template <typename T> void FractoriumEmberController<T>::RandRangeChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_RandPointRange = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnRandRangeChanged(double d) { m_Controller->RandRangeChanged(d); }

/// <summary>
/// Set the number of samples to disregard for each sub batch.
/// Called when the fuse count spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The fuse count value to set</param>
template <typename T> void FractoriumEmberController<T>::FuseChanged(int d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_FuseCount = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnFuseChanged(int d) { m_Controller->FuseChanged(d); }

/// <summary>
/// Set the quality.
/// 10 is good for interactive rendering on the CPU.
/// 20-50 is good for OpenCL.
/// Above 500 seems to offer little additional value for final renders.
/// Called when the quality spinner is changed.
/// If rendering is done, and the value is greater than the last value,
/// the rendering process is continued, else it's reset.
/// </summary>
/// <param name="d">The quality in terms of iterations per pixel</param>
template <typename T> void FractoriumEmberController<T>::QualityChanged(double d) { }
void Fractorium::OnQualityChanged(double d) { m_Controller->QualityChanged(d); }

/// <summary>
/// Set the supersample.
/// Note this will dramatically degrade performance, especially in
/// OpenCL, while only giving a minor improvement in visual quality.
/// Values above 2 add no noticeable difference.
/// The user should only use this for a final render, or a quick preview, and then
/// reset it back to 1 when done.
/// Called when the supersample spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The supersample value to set</param>
template <typename T> void FractoriumEmberController<T>::SupersampleChanged(int d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_Supersample = d;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
}
void Fractorium::OnSupersampleChanged(int d) { m_Controller->SupersampleChanged(d); }

/// <summary>
/// Set the interpolation type.
/// Does not reset anything because this is only used for animation.
/// Called when the interp type combo box index is changed.
/// </summary>
/// <param name="i">The index</param>
template <typename T>
void FractoriumEmberController<T>::InterpTypeChanged(int i)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		eInterp interp;

		if (i == 0)
			interp = eInterp::EMBER_INTERP_LINEAR;
		else if (i == 1)
			interp = eInterp::EMBER_INTERP_SMOOTH;
		else
			interp = eInterp::EMBER_INTERP_LINEAR;

		ember.m_Interp = interp;

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_Interp = interp;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnInterpTypeComboCurrentIndexChanged(int index) { m_Controller->InterpTypeChanged(index); }

/// <summary>
/// Set the affine interpolation type.
/// Does not reset anything because this is only used for animation.
/// In the future, when animation is implemented, this will have an effect.
/// Called when the affine interp type combo box index is changed.
/// </summary>
/// <param name="index">The index</param>
template <typename T>
void FractoriumEmberController<T>::AffineInterpTypeChanged(int i)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		eAffineInterp interp;

		if (i == 0)
			interp = eAffineInterp::AFFINE_INTERP_LINEAR;
		else if (i == 1)
			interp = eAffineInterp::AFFINE_INTERP_LOG;
		else
			interp = eAffineInterp::AFFINE_INTERP_LINEAR;

		ember.m_AffineInterp = interp;

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_AffineInterp = interp;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnAffineInterpTypeComboCurrentIndexChanged(int index) { m_Controller->AffineInterpTypeChanged(index); }

/// <summary>
/// Set the rotations to be used in animation.
/// Called when the rotations spinner is changed.
/// Does not reset anything because this is only used for animation.
/// </summary>
/// <param name="d">The number of rotations to perform in a loop</param>
template <typename T> void FractoriumEmberController<T>::RotationsChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_Rotations = d;

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_Rotations = d;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnRotationsChanged(double d) { m_Controller->RotationsChanged(d); }

/// <summary>
/// Set the seconds each loop rotation takes in animation.
/// Called when the seconds per rotation spinner is changed.
/// Does not reset anything because this is only used for animation.
/// </summary>
/// <param name="d">The number of seconds each loop rotation should take</param>
template <typename T> void FractoriumEmberController<T>::SecondsPerRotationChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_SecondsPerRotation = d;

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_SecondsPerRotation = d;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnSecondsPerRotationChanged(double d) { m_Controller->SecondsPerRotationChanged(d); }

/// <summary>
/// Set the direction loop rotations rotate in animation.
/// Called when the xforms rotation direction combobox index is changed.
/// Does not reset anything because this is only used for animation.
/// </summary>
/// <param name="d">The index</param>
template <typename T> void FractoriumEmberController<T>::RotateXformsDirChanged(uint d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_RotateXformsCw = d == 0;

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_RotateXformsCw = ember.m_RotateXformsCw;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnRotateXformsDirComboCurrentIndexChanged(int i) { m_Controller->RotateXformsDirChanged(i); }

/// <summary>
/// Set the seconds each blend takes in animation.
/// Called when the blend seconds spinner is changed.
/// Does not reset anything because this is only used for animation.
/// </summary>
/// <param name="d">The number of seconds each blend should take</param>
template <typename T> void FractoriumEmberController<T>::BlendSecondsChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_BlendSeconds = d;

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_BlendSeconds = ember.m_BlendSeconds;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnBlendSecondsChanged(double d) { m_Controller->BlendSecondsChanged(d); }

/// <summary>
/// Set the rotations each blend performs in animation.
/// Called when the rotations per blend spinner is changed.
/// Does not reset anything because this is only used for animation.
/// </summary>
/// <param name="d">The number of rotations each blend should perform</param>
template <typename T> void FractoriumEmberController<T>::RotationsPerBlendChanged(uint d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_RotationsPerBlend = d;

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_RotationsPerBlend = d;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnRotationsPerBlendChanged(int d) { m_Controller->RotationsPerBlendChanged(d); }

/// <summary>
/// Set the direction blend rotations rotate in animation.
/// Called when the blend xforms rotation direction combobox index is changed.
/// Does not reset anything because this is only used for animation.
/// </summary>
/// <param name="d">The index</param>
template <typename T> void FractoriumEmberController<T>::BlendXformsRotateDirChanged(uint d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_BlendRotateXformsCw = d == 0;

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_BlendRotateXformsCw = ember.m_BlendRotateXformsCw;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnBlendXformsRotateDirComboCurrentIndexChanged(int i) { m_Controller->BlendXformsRotateDirChanged(i); }

/// <summary>
/// Set the blend interpolation type in animation.
/// Called when the blend interpolation type combobox index is changed.
/// Does not reset anything because this is only used for animation.
/// </summary>
/// <param name="d">The index</param>
template <typename T> void FractoriumEmberController<T>::BlendInterpTypeChanged(uint d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_Linear = d == 0;

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_Linear = ember.m_Linear;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnBlendInterpTypeComboCurrentIndexChanged(int i) { m_Controller->BlendInterpTypeChanged(i); }

/// <summary>
/// Set the stagger amount in animation.
/// Called when the stagger spinner is changed.
/// Does not reset anything because this is only used for animation.
/// </summary>
/// <param name="d">The amount to stagger the blending of xforms</param>
template <typename T> void FractoriumEmberController<T>::StaggerChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_Stagger = Clamp(d, 0.0, 1.0);

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_Stagger = ember.m_Stagger;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnStaggerChanged(double d) { m_Controller->StaggerChanged(d); }

/// <summary>
/// Set the temporal filter width to be used with animation.
/// Called when the temporal filter width spinner is changed.
/// Does not reset anything because this is only used for animation.
/// </summary>
/// <param name="d">The temporal filter width</param>
template <typename T>
void FractoriumEmberController<T>::TemporalFilterWidthChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_TemporalFilterWidth = d;

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_TemporalFilterWidth = d;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnTemporalFilterWidthChanged(double d) { m_Controller->TemporalFilterWidthChanged(d); }

/// <summary>
/// Set the temporal filter type to be used with animation.
/// Called when the temporal filter combo box index is changed.
/// Does not reset anything because this is only used for animation.
/// </summary>
/// <param name="text">The name of the temporal filter</param>
template <typename T>
void FractoriumEmberController<T>::TemporalFilterTypeChanged(const QString& text)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		auto filter = TemporalFilterCreator<T>::FromString(text.toStdString());
		ember.m_TemporalFilterType = filter;

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_TemporalFilterType = filter;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnTemporalFilterTypeComboCurrentIndexChanged(const QString& text) { m_Controller->TemporalFilterTypeChanged(text); }

/// <summary>
/// Set the exponent value for the Exp temporal filter type to be used with animation.
/// Called when the exp value combo box index is changed.
/// Does not reset anything because this is only used for animation.
/// </summary>
/// <param name="text">The name of the temporal filter</param>
template <typename T>
void FractoriumEmberController<T>::ExpChanged(double d)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_TemporalFilterExp = d;

		if (!m_Fractorium->ApplyAll())
			if (m_EmberFilePointer)
				m_EmberFilePointer->m_TemporalFilterExp = d;
	}, false, eProcessAction::NOTHING, m_Fractorium->ApplyAll());
}
void Fractorium::OnExpChanged(double d) { m_Controller->ExpChanged(d); }

/// <summary>
/// Set the center.
/// This updates the spinners as well as the current ember center.
/// Resets the renering process.
/// </summary>
/// <param name="x">The x offset</param>
/// <param name="y">The y offset</param>
template <typename T>
void FractoriumEmberController<T>::SetCenter(double x, double y)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.m_CenterX = x;
		ember.m_CenterY = ember.m_RotCenterY = y;
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
	m_Fractorium->m_CenterXSpin->SetValueStealth(x);//Don't trigger a redraw twice.
	m_Fractorium->m_CenterYSpin->SetValueStealth(y);
}

/// <summary>
/// Fill the parameter tables and palette widgets with values from the current ember.
/// This takes ~1-2ms.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::FillParamTablesAndPalette()
{
	m_Fractorium->m_BrightnessSpin->SetValueStealth(m_Ember.m_Brightness);//Color.
	m_Fractorium->m_GammaSpin->SetValueStealth(m_Ember.m_Gamma);
	m_Fractorium->m_GammaThresholdSpin->SetValueStealth(m_Ember.m_GammaThresh);
	m_Fractorium->m_VibrancySpin->SetValueStealth(m_Ember.m_Vibrancy);
	m_Fractorium->m_HighlightSpin->SetValueStealth(m_Ember.m_HighlightPower);
	m_Fractorium->m_K2Spin->SetValueStealth(m_Ember.m_K2);
	m_Fractorium->m_ColorDialog->setCurrentColor(QColor(m_Ember.m_Background.r * 255, m_Ember.m_Background.g * 255, m_Ember.m_Background.b * 255));
	m_Fractorium->ui.ColorTable->item(m_Fractorium->m_BgRow, 1)->setBackground(m_Fractorium->m_ColorDialog->currentColor());
	BackgroundChanged(m_Fractorium->m_ColorDialog->currentColor());
	m_Fractorium->m_PaletteModeCombo->SetCurrentIndexStealth(static_cast<int>(m_Ember.m_PaletteMode));
	m_Fractorium->m_WidthSpin->SetValueStealth(m_Ember.m_FinalRasW);//Geometry.
	m_Fractorium->m_HeightSpin->SetValueStealth(m_Ember.m_FinalRasH);
	m_Fractorium->m_CenterXSpin->SetValueStealth(m_Ember.m_CenterX);
	m_Fractorium->m_CenterYSpin->SetValueStealth(m_Ember.m_CenterY);
	m_Fractorium->m_ScaleSpin->SetValueStealth(m_Ember.m_PixelsPerUnit);
	m_Fractorium->m_ZoomSpin->SetValueStealth(m_Ember.m_Zoom);
	m_Fractorium->m_RotateSpin->SetValueStealth(m_Ember.m_Rotate);
	m_Fractorium->m_ZPosSpin->SetValueStealth(m_Ember.m_CamZPos);
	m_Fractorium->m_PerspectiveSpin->SetValueStealth(m_Ember.m_CamPerspective);
	m_Fractorium->m_PitchSpin->SetValueStealth(double{ m_Ember.m_CamPitch } * RAD_2_DEG_T);
	m_Fractorium->m_YawSpin->SetValueStealth(double {m_Ember.m_CamYaw} * RAD_2_DEG_T);
	m_Fractorium->m_DepthBlurSpin->SetValueStealth(m_Ember.m_CamDepthBlur);
	m_Fractorium->m_BlurCurveSpin->SetValueStealth(m_Ember.m_BlurCurve);
	m_Fractorium->m_SpatialFilterWidthSpin->SetValueStealth(m_Ember.m_SpatialFilterRadius);//Filter.
	m_Fractorium->m_SpatialFilterTypeCombo->SetCurrentIndexStealth(static_cast<int>(m_Ember.m_SpatialFilterType));
	m_Fractorium->m_DEFilterMinRadiusSpin->SetValueStealth(m_Ember.m_MinRadDE);
	m_Fractorium->m_DEFilterMaxRadiusSpin->SetValueStealth(m_Ember.m_MaxRadDE);
	m_Fractorium->m_DECurveSpin->SetValueStealth(m_Ember.m_CurveDE);
	m_Fractorium->m_SbsSpin->SetValueStealth(m_Ember.m_SubBatchSize);//Iteration.
	m_Fractorium->m_FuseSpin->SetValueStealth(m_Ember.m_FuseCount);
	m_Fractorium->m_RandRangeSpin->SetValueStealth(m_Ember.m_RandPointRange);
	m_Fractorium->m_QualitySpin->SetValueStealth(m_Ember.m_Quality);
	m_Fractorium->m_SupersampleSpin->SetValueStealth(m_Ember.m_Supersample);
	m_Fractorium->m_InterpTypeCombo->SetCurrentIndexStealth(static_cast<int>(m_Ember.m_Interp));
	m_Fractorium->m_AffineInterpTypeCombo->SetCurrentIndexStealth(static_cast<int>(m_Ember.m_AffineInterp));
	m_Fractorium->m_RotationsSpin->SetValueStealth(m_Ember.m_Rotations);
	m_Fractorium->m_SecondsPerRotationSpin->SetValueStealth(m_Ember.m_SecondsPerRotation);
	m_Fractorium->m_RotateXformsDirCombo->SetCurrentIndexStealth(m_Ember.m_RotateXformsCw ? 0 : 1);
	m_Fractorium->m_BlendSecondsSpin->SetValueStealth(m_Ember.m_BlendSeconds);
	m_Fractorium->m_RotationsPerBlendSpin->SetValueStealth(m_Ember.m_RotationsPerBlend);
	m_Fractorium->m_BlendXformsRotateDirCombo->SetCurrentIndexStealth(m_Ember.m_BlendRotateXformsCw ? 0 : 1);
	m_Fractorium->m_BlendInterpTypeCombo->SetCurrentIndexStealth(m_Ember.m_Linear ? 0 : 1);
	m_Fractorium->m_StaggerSpin->SetValueStealth(m_Ember.m_Stagger);
	m_Fractorium->m_TemporalFilterWidthSpin->SetValueStealth(m_Ember.m_TemporalFilterWidth);
	m_Fractorium->m_TemporalFilterTypeCombo->SetCurrentIndexStealth(static_cast<int>(m_Ember.m_TemporalFilterType));
	m_Fractorium->m_TemporalFilterExpSpin->SetValueStealth(m_Ember.m_TemporalFilterExp);
	auto temp = m_Ember.m_Palette.m_Filename;

	if (temp.get())
		m_Fractorium->SetPaletteFileComboIndex(*temp.get());

	//Update the palette preview widget.
	m_Fractorium->ResetPaletteControls();
	//Since the controls were cleared above, the adjusted palette will be identical to the base palette.
	//Callers can set, apply and display palette adjustments after this function exits if needed.
	SetBasePaletteAndAdjust(m_Ember.m_Palette);//Updating the palette GUI will trigger a full render.
	InitLockedScale();
}

/// <summary>
/// Copy all GUI widget values on the parameters tab to the passed in ember.
/// </summary>
/// <param name="ember">The ember to copy values to.</param>
/// <param name="imageParamsOnly">True to get just spatial and density filters plus coloring params.</param>
template <typename T> void FractoriumEmberController<T>::ParamsToEmber(Ember<float>& ember, bool imageParamsOnly) { ParamsToEmberPrivate<float>(ember, imageParamsOnly); }
#ifdef DO_DOUBLE
template <typename T> void FractoriumEmberController<T>::ParamsToEmber(Ember<double>& ember, bool imageParamsOnly) { ParamsToEmberPrivate<double>(ember, imageParamsOnly); }
#endif
template <typename T>
template <typename U>
void FractoriumEmberController<T>::ParamsToEmberPrivate(Ember<U>& ember, bool imageParamsOnly)
{
	ember.m_Brightness = m_Fractorium->m_BrightnessSpin->value();//Color.
	ember.m_Gamma = m_Fractorium->m_GammaSpin->value();
	ember.m_GammaThresh = m_Fractorium->m_GammaThresholdSpin->value();
	ember.m_Vibrancy = m_Fractorium->m_VibrancySpin->value();
	ember.m_HighlightPower = m_Fractorium->m_HighlightSpin->value();
	ember.m_K2 = m_Fractorium->m_K2Spin->value();
	ember.m_SpatialFilterRadius = m_Fractorium->m_SpatialFilterWidthSpin->value();//Filter.
	ember.m_SpatialFilterType = static_cast<eSpatialFilterType>(m_Fractorium->m_SpatialFilterTypeCombo->currentIndex());
	ember.m_MinRadDE = m_Fractorium->m_DEFilterMinRadiusSpin->value();
	ember.m_MaxRadDE = m_Fractorium->m_DEFilterMaxRadiusSpin->value();
	ember.m_CurveDE  = m_Fractorium->m_DECurveSpin->value();

	if (imageParamsOnly)
		return;

	const auto color = m_Fractorium->ui.ColorTable->item(5, 1)->background();
	ember.m_Background.r = color.color().red() / 255.0;
	ember.m_Background.g = color.color().green() / 255.0;
	ember.m_Background.b = color.color().blue() / 255.0;
	ember.m_PaletteMode = static_cast<ePaletteMode>(m_Fractorium->m_PaletteModeCombo->currentIndex());
	ember.m_FinalRasW = m_Fractorium->m_WidthSpin->value();//Geometry.
	ember.m_FinalRasH = m_Fractorium->m_HeightSpin->value();
	ember.m_CenterX = m_Fractorium->m_CenterXSpin->value();
	ember.m_CenterY = ember.m_RotCenterY = m_Fractorium->m_CenterYSpin->value();
	ember.m_PixelsPerUnit = m_Fractorium->m_ScaleSpin->value();
	ember.m_Zoom = m_Fractorium->m_ZoomSpin->value();
	ember.m_Rotate = m_Fractorium->m_RotateSpin->value();
	ember.m_CamZPos = m_Fractorium->m_ZPosSpin->value();
	ember.m_CamPerspective = m_Fractorium->m_PerspectiveSpin->value();
	ember.m_CamPitch = m_Fractorium->m_PitchSpin->value() * DEG_2_RAD_T;
	ember.m_CamYaw = m_Fractorium->m_YawSpin->value() * DEG_2_RAD_T;
	ember.m_CamDepthBlur = m_Fractorium->m_DepthBlurSpin->value();
	ember.m_BlurCurve = m_Fractorium->m_BlurCurveSpin->value();
	ember.m_SubBatchSize = m_Fractorium->m_SbsSpin->value();
	ember.m_FuseCount = m_Fractorium->m_FuseSpin->value();
	ember.m_RandPointRange = m_Fractorium->m_RandRangeSpin->value();
	ember.m_Quality = m_Fractorium->m_QualitySpin->value();
	ember.m_Supersample = m_Fractorium->m_SupersampleSpin->value();
	ember.m_Interp = static_cast<eInterp>(m_Fractorium->m_InterpTypeCombo->currentIndex());
	ember.m_AffineInterp = static_cast<eAffineInterp>(m_Fractorium->m_AffineInterpTypeCombo->currentIndex());
	ember.m_Rotations = m_Fractorium->m_RotationsSpin->value();
	ember.m_SecondsPerRotation = m_Fractorium->m_SecondsPerRotationSpin->value();
	ember.m_RotateXformsCw = m_Fractorium->m_RotateXformsDirCombo->currentIndex() == 0;
	ember.m_BlendSeconds = m_Fractorium->m_BlendSecondsSpin->value();
	ember.m_RotationsPerBlend = m_Fractorium->m_RotationsPerBlendSpin->value();
	ember.m_BlendRotateXformsCw = m_Fractorium->m_BlendXformsRotateDirCombo->currentIndex() == 0;
	ember.m_Linear = m_Fractorium->m_BlendInterpTypeCombo->currentIndex() == 0;
	ember.m_Stagger = m_Fractorium->m_StaggerSpin->value();
	ember.m_TemporalFilterWidth = m_Fractorium->m_TemporalFilterWidthSpin->value();
	ember.m_TemporalFilterType = static_cast<eTemporalFilterType>(m_Fractorium->m_TemporalFilterTypeCombo->currentIndex());
	ember.m_TemporalFilterExp = m_Fractorium->m_TemporalFilterExpSpin->value();
	ember.SyncSize();
}

/// <summary>
/// Set the rotation.
/// This updates the spinner, optionally stealth.
/// </summary>
/// <param name="rot">The rotation value in angles to set</param>
/// <param name="stealth">True if stealth to skip re-rendering, else false to trigger a new render</param>
void Fractorium::SetRotation(double rot, bool stealth)
{
	if (stealth)
		m_RotateSpin->SetValueStealth(rot);
	else
		m_RotateSpin->setValue(rot);
}

/// <summary>
/// Set the scale.
/// This is the number of raster pixels that correspond to the distance
/// between 0-1 in the cartesian plane. The higher the number, the more
/// zoomed in the image is.
/// Resets the rendering process.
/// </summary>
/// <param name="scale">The scale value</param>
void Fractorium::SetScale(double scale)
{
	m_ScaleSpin->setValue(scale);
}

/// <summary>
/// Set the pitch.
/// This updates the spinner
/// </summary>
/// <param name="pitch">The pitch value</param>
void Fractorium::SetPitch(double pitch)
{
	m_PitchSpin->setValue(pitch);
}

/// <summary>
/// Set the yaw.
/// This updates the spinner
/// </summary>
/// <param name="yaw">The yaw value</param>
void Fractorium::SetYaw(double yaw)
{
	m_YawSpin->setValue(yaw);
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
