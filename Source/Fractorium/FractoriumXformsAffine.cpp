#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the xforms affine UI.
/// </summary>
void Fractorium::InitXformsAffineUI()
{
	const auto affinePrec = 8;
	const auto spinHeight = 20;
	const auto affineStep = 0.01;
	const auto affineMin = std::numeric_limits<double>::lowest();
	const auto affineMax = std::numeric_limits<double>::max();
	auto table = ui.PreAffineTable;
	table->verticalHeader()->setVisible(true);//The designer continually clobbers these values, so must manually set them here.
	table->horizontalHeader()->setVisible(true);
	table->verticalHeader()->setSectionsClickable(true);
	table->horizontalHeader()->setSectionsClickable(true);
	table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	connect(table->verticalHeader(),   SIGNAL(sectionDoubleClicked(int)), this, SLOT(OnPreAffineRowDoubleClicked(int)), Qt::ConnectionType::QueuedConnection);
	connect(table->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(OnPreAffineColDoubleClicked(int)), Qt::ConnectionType::QueuedConnection);
	//Pre affine spinners.
	SetupAffineSpinner(table, this, 0, 0, m_PreX1Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnX1Changed(double)));
	SetupAffineSpinner(table, this, 0, 1, m_PreX2Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnX2Changed(double)));
	SetupAffineSpinner(table, this, 1, 0, m_PreY1Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnY1Changed(double)));
	SetupAffineSpinner(table, this, 1, 1, m_PreY2Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnY2Changed(double)));
	SetupAffineSpinner(table, this, 2, 0, m_PreO1Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnO1Changed(double)));
	SetupAffineSpinner(table, this, 2, 1, m_PreO2Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnO2Changed(double)));
	table = ui.PostAffineTable;
	table->verticalHeader()->setVisible(true);//The designer continually clobbers these values, so must manually set them here.
	table->horizontalHeader()->setVisible(true);
	table->verticalHeader()->setSectionsClickable(true);
	table->horizontalHeader()->setSectionsClickable(true);
	table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	connect(table->verticalHeader(),   SIGNAL(sectionDoubleClicked(int)), this, SLOT(OnPostAffineRowDoubleClicked(int)), Qt::ConnectionType::QueuedConnection);
	connect(table->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(OnPostAffineColDoubleClicked(int)), Qt::ConnectionType::QueuedConnection);
	//Post affine spinners.
	SetupAffineSpinner(table, this, 0, 0, m_PostX1Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnX1Changed(double)));
	SetupAffineSpinner(table, this, 0, 1, m_PostX2Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnX2Changed(double)));
	SetupAffineSpinner(table, this, 1, 0, m_PostY1Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnY1Changed(double)));
	SetupAffineSpinner(table, this, 1, 1, m_PostY2Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnY2Changed(double)));
	SetupAffineSpinner(table, this, 2, 0, m_PostO1Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnO1Changed(double)));
	SetupAffineSpinner(table, this, 2, 1, m_PostO2Spin, spinHeight, affineMin, affineMax, affineStep, affinePrec, SIGNAL(valueChanged(double)), SLOT(OnO2Changed(double)));
	auto preRotateVal  = new QDoubleValidator(ui.PreRotateCombo);  preRotateVal->setLocale(QLocale::system());
	auto preMoveVal    = new QDoubleValidator(ui.PreMoveCombo);    preMoveVal->setLocale(QLocale::system());
	auto preScaleVal   = new QDoubleValidator(ui.PreScaleCombo);   preScaleVal->setLocale(QLocale::system());
	auto postRotateVal = new QDoubleValidator(ui.PostRotateCombo); postRotateVal->setLocale(QLocale::system());
	auto postMoveVal   = new QDoubleValidator(ui.PostMoveCombo);   postMoveVal->setLocale(QLocale::system());
	auto postScaleVal  = new QDoubleValidator(ui.PostScaleCombo);  postScaleVal->setLocale(QLocale::system());
	ui.PreRotateCombo->setValidator(preRotateVal);
	ui.PreMoveCombo->setValidator(preMoveVal);
	ui.PreScaleCombo->setValidator(preScaleVal);
	ui.PostRotateCombo->setValidator(postRotateVal);
	ui.PostMoveCombo->setValidator(postMoveVal);
	ui.PostScaleCombo->setValidator(postScaleVal);
	QStringList moveList;
	moveList.append(ToString(0.5));
	moveList.append(ToString(0.25));
	moveList.append(ToString(0.1));
	moveList.append(ToString(0.05));
	moveList.append(ToString(0.025));
	moveList.append(ToString(0.01));
	ui.PreMoveCombo->addItems(moveList);
	ui.PostMoveCombo->addItems(moveList);
	connect(ui.PreFlipHorizontalButton,  SIGNAL(clicked(bool)),                     this, SLOT(OnFlipHorizontalButtonClicked(bool)),               Qt::ConnectionType::QueuedConnection);
	connect(ui.PreFlipVerticalButton,    SIGNAL(clicked(bool)),                     this, SLOT(OnFlipVerticalButtonClicked(bool)),                 Qt::ConnectionType::QueuedConnection);
	connect(ui.PreRotate90CButton,       SIGNAL(clicked(bool)),                     this, SLOT(OnRotate90CButtonClicked(bool)),                    Qt::ConnectionType::QueuedConnection);
	connect(ui.PreRotate90CcButton,      SIGNAL(clicked(bool)),                     this, SLOT(OnRotate90CcButtonClicked(bool)),                   Qt::ConnectionType::QueuedConnection);
	connect(ui.PreRotateCButton,         SIGNAL(clicked(bool)),                     this, SLOT(OnRotateCButtonClicked(bool)),                      Qt::ConnectionType::QueuedConnection);
	connect(ui.PreRotateCcButton,        SIGNAL(clicked(bool)),                     this, SLOT(OnRotateCcButtonClicked(bool)),                     Qt::ConnectionType::QueuedConnection);
	connect(ui.PreMoveUpButton,          SIGNAL(clicked(bool)),                     this, SLOT(OnMoveUpButtonClicked(bool)),                       Qt::ConnectionType::QueuedConnection);
	connect(ui.PreMoveDownButton,        SIGNAL(clicked(bool)),                     this, SLOT(OnMoveDownButtonClicked(bool)),                     Qt::ConnectionType::QueuedConnection);
	connect(ui.PreMoveLeftButton,        SIGNAL(clicked(bool)),                     this, SLOT(OnMoveLeftButtonClicked(bool)),                     Qt::ConnectionType::QueuedConnection);
	connect(ui.PreMoveRightButton,       SIGNAL(clicked(bool)),                     this, SLOT(OnMoveRightButtonClicked(bool)),                    Qt::ConnectionType::QueuedConnection);
	connect(ui.PreScaleDownButton,       SIGNAL(clicked(bool)),                     this, SLOT(OnScaleDownButtonClicked(bool)),                    Qt::ConnectionType::QueuedConnection);
	connect(ui.PreScaleUpButton,         SIGNAL(clicked(bool)),                     this, SLOT(OnScaleUpButtonClicked(bool)),                      Qt::ConnectionType::QueuedConnection);
	connect(ui.PreResetButton,           SIGNAL(clicked(bool)),                     this, SLOT(OnResetAffineButtonClicked(bool)),                  Qt::ConnectionType::QueuedConnection);
	connect(ui.PreCopyButton,            SIGNAL(clicked(bool)),                     this, SLOT(OnCopyAffineButtonClicked(bool)),                   Qt::ConnectionType::QueuedConnection);
	connect(ui.PrePasteButton,           SIGNAL(clicked(bool)),                     this, SLOT(OnPasteAffineButtonClicked(bool)),                  Qt::ConnectionType::QueuedConnection);
	connect(ui.PreRandomButton,          SIGNAL(clicked(bool)),                     this, SLOT(OnRandomAffineButtonClicked(bool)),                 Qt::ConnectionType::QueuedConnection);
	connect(ui.PostFlipHorizontalButton, SIGNAL(clicked(bool)),                     this, SLOT(OnFlipHorizontalButtonClicked(bool)),               Qt::ConnectionType::QueuedConnection);
	connect(ui.PostFlipVerticalButton,   SIGNAL(clicked(bool)),                     this, SLOT(OnFlipVerticalButtonClicked(bool)),                 Qt::ConnectionType::QueuedConnection);
	connect(ui.PostRotate90CcButton,     SIGNAL(clicked(bool)),                     this, SLOT(OnRotate90CcButtonClicked(bool)),                   Qt::ConnectionType::QueuedConnection);
	connect(ui.PostRotateCcButton,       SIGNAL(clicked(bool)),                     this, SLOT(OnRotateCcButtonClicked(bool)),                     Qt::ConnectionType::QueuedConnection);
	connect(ui.PostRotateCButton,        SIGNAL(clicked(bool)),                     this, SLOT(OnRotateCButtonClicked(bool)),                      Qt::ConnectionType::QueuedConnection);
	connect(ui.PostRotate90CButton,      SIGNAL(clicked(bool)),                     this, SLOT(OnRotate90CButtonClicked(bool)),                    Qt::ConnectionType::QueuedConnection);
	connect(ui.PostMoveUpButton,         SIGNAL(clicked(bool)),                     this, SLOT(OnMoveUpButtonClicked(bool)),                       Qt::ConnectionType::QueuedConnection);
	connect(ui.PostMoveDownButton,       SIGNAL(clicked(bool)),                     this, SLOT(OnMoveDownButtonClicked(bool)),                     Qt::ConnectionType::QueuedConnection);
	connect(ui.PostMoveLeftButton,       SIGNAL(clicked(bool)),                     this, SLOT(OnMoveLeftButtonClicked(bool)),                     Qt::ConnectionType::QueuedConnection);
	connect(ui.PostMoveRightButton,      SIGNAL(clicked(bool)),                     this, SLOT(OnMoveRightButtonClicked(bool)),                    Qt::ConnectionType::QueuedConnection);
	connect(ui.PostScaleDownButton,      SIGNAL(clicked(bool)),                     this, SLOT(OnScaleDownButtonClicked(bool)),                    Qt::ConnectionType::QueuedConnection);
	connect(ui.PostScaleUpButton,        SIGNAL(clicked(bool)),                     this, SLOT(OnScaleUpButtonClicked(bool)),                      Qt::ConnectionType::QueuedConnection);
	connect(ui.PostResetButton,          SIGNAL(clicked(bool)),                     this, SLOT(OnResetAffineButtonClicked(bool)),                  Qt::ConnectionType::QueuedConnection);
	connect(ui.PostCopyButton,           SIGNAL(clicked(bool)),                     this, SLOT(OnCopyAffineButtonClicked(bool)),                   Qt::ConnectionType::QueuedConnection);
	connect(ui.PostPasteButton,          SIGNAL(clicked(bool)),                     this, SLOT(OnPasteAffineButtonClicked(bool)),                  Qt::ConnectionType::QueuedConnection);
	connect(ui.PostRandomButton,         SIGNAL(clicked(bool)),                     this, SLOT(OnRandomAffineButtonClicked(bool)),                 Qt::ConnectionType::QueuedConnection);
	connect(ui.PreAffineGroupBox,        SIGNAL(toggled(bool)),                     this, SLOT(OnAffineGroupBoxToggled(bool)),                     Qt::ConnectionType::QueuedConnection);
	connect(ui.PostAffineGroupBox,       SIGNAL(toggled(bool)),                     this, SLOT(OnAffineGroupBoxToggled(bool)),                     Qt::ConnectionType::QueuedConnection);
	connect(ui.SwapAffinesButton,        SIGNAL(clicked(bool)),                     this, SLOT(OnSwapAffinesButtonClicked(bool)),                  Qt::ConnectionType::QueuedConnection);
	connect(ui.PolarAffineCheckBox,      SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(OnPolarAffineCheckBoxStateChanged(Qt::CheckState)), Qt::ConnectionType::QueuedConnection);
#ifndef _WIN32
	//For some reason linux makes these 24x24, even though the designer explicitly says 16x16.
	//Also, in order to get 4 pixels of spacing between elements in the grid layout, 0 must be specified.
	ui.PreFlipHorizontalButton->setIconSize(QSize(16, 16));
	ui.PreFlipVerticalButton->setIconSize(QSize(16, 16));
	ui.PreRotate90CButton->setIconSize(QSize(16, 16));
	ui.PreRotate90CcButton->setIconSize(QSize(16, 16));
	ui.PreRotateCButton->setIconSize(QSize(16, 16));
	ui.PreRotateCcButton->setIconSize(QSize(16, 16));
	ui.PreMoveUpButton->setIconSize(QSize(16, 16));
	ui.PreMoveDownButton->setIconSize(QSize(16, 16));
	ui.PreMoveLeftButton->setIconSize(QSize(16, 16));
	ui.PreMoveRightButton->setIconSize(QSize(16, 16));
	ui.PreScaleDownButton->setIconSize(QSize(16, 16));
	ui.PreScaleUpButton->setIconSize(QSize(16, 16));
	ui.PreResetButton->setIconSize(QSize(16, 16));
	ui.PreAffineGridLayout->setHorizontalSpacing(0);
	ui.PreAffineGridLayout->setVerticalSpacing(0);
	ui.PostFlipHorizontalButton->setIconSize(QSize(16, 16));
	ui.PostFlipVerticalButton->setIconSize(QSize(16, 16));
	ui.PostRotate90CButton->setIconSize(QSize(16, 16));
	ui.PostRotate90CcButton->setIconSize(QSize(16, 16));
	ui.PostRotateCButton->setIconSize(QSize(16, 16));
	ui.PostRotateCcButton->setIconSize(QSize(16, 16));
	ui.PostMoveUpButton->setIconSize(QSize(16, 16));
	ui.PostMoveDownButton->setIconSize(QSize(16, 16));
	ui.PostMoveLeftButton->setIconSize(QSize(16, 16));
	ui.PostMoveRightButton->setIconSize(QSize(16, 16));
	ui.PostScaleDownButton->setIconSize(QSize(16, 16));
	ui.PostScaleUpButton->setIconSize(QSize(16, 16));
	ui.PostResetButton->setIconSize(QSize(16, 16));
	ui.PostAffineGridLayout->setHorizontalSpacing(0);
	ui.PostAffineGridLayout->setVerticalSpacing(0);
	//Further, the size of the dock widget won't be properly adjusted until the xforms tab is shown.
	//So show it here and it will be switched back in Fractorium's constructor.
	//ui.ParamsTabWidget->setCurrentIndex(2);
	//ui.DockWidget->update();
#endif
	//Placing pointers to the spin boxes in arrays makes them easier to access in various places.
	m_PreSpins[0] = m_PreX1Spin;//A
	m_PreSpins[1] = m_PreY1Spin;//B
	m_PreSpins[2] = m_PreO1Spin;//C
	m_PreSpins[3] = m_PreX2Spin;//D
	m_PreSpins[4] = m_PreY2Spin;//E
	m_PreSpins[5] = m_PreO2Spin;//F
	m_PostSpins[0] = m_PostX1Spin;
	m_PostSpins[1] = m_PostY1Spin;
	m_PostSpins[2] = m_PostO1Spin;
	m_PostSpins[3] = m_PostX2Spin;
	m_PostSpins[4] = m_PostY2Spin;
	m_PostSpins[5] = m_PostO2Spin;
	ui.PreAffineGroupBox->setChecked(false);//Flip both once to force enabling/disabling the disabling of the group boxes and the corner buttons.
	ui.PreAffineGroupBox->setChecked(true);//Pre affine enabled.
	ui.PostAffineGroupBox->setChecked(false);
	ui.PostAffineGroupBox->setChecked(true);//Post affine enabled.
}

/// <summary>
/// Set the scale used for drawing the affines to a default value.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::InitLockedScale() noexcept
{
	m_LockedScale = (T)std::min<size_t>(m_Ember.m_FinalRasW, m_Ember.m_FinalRasH) / 4.0;
	m_LockedX = m_Ember.m_CenterX;
	m_LockedY = m_Ember.m_CenterY;
}

/// <summary>
/// Multiply the scale used for drawing the affines by the passed in value.
/// </summary>
/// <param name="value">The value to scale by</param>
template <typename T>
void FractoriumEmberController<T>::ChangeLockedScale(T value)
{
	T min_size = 25.0;
	m_LockedScale *= value;

	if (m_LockedScale < min_size)
		m_LockedScale = min_size;
}

/// <summary>
/// Return the value needed to multiply the current scale by to get back to the locked scale.
/// </summary>
/// <returns>The scale value</returns>
template <typename T>
double FractoriumEmberController<T>::AffineScaleCurrentToLocked() noexcept
{
	return LockedScale() / m_Ember.m_PixelsPerUnit;
}

/// <summary>
/// Return the value needed to multiply the locked scale by to get to the current scale.
/// </summary>
/// <returns>The scale value</returns>
template <typename T>
double FractoriumEmberController<T>::AffineScaleLockedToCurrent() noexcept
{
	return m_Ember.m_PixelsPerUnit / LockedScale();
}

/// <summary>
/// Toggle all pre affine values in one row for the selected xforms.
/// Resets the rendering process.
/// </summary>
/// <param name="logicalIndex">The index of the row that was double clicked</param>
void Fractorium::OnPreAffineRowDoubleClicked(int logicalIndex)
{
	ToggleTableRow(ui.PreAffineTable, logicalIndex);
}

/// <summary>
/// Toggle all pre affine values in one column for the selected xforms.
/// Resets the rendering process.
/// </summary>
/// <param name="logicalIndex">The index of the row that was double clicked</param>
void Fractorium::OnPreAffineColDoubleClicked(int logicalIndex)
{
	ToggleTableCol(ui.PreAffineTable, logicalIndex);
}

/// <summary>
/// Toggle all post affine values in one row for the selected xforms.
/// Resets the rendering process.
/// </summary>
/// <param name="logicalIndex">The index of the row that was double clicked</param>
void Fractorium::OnPostAffineRowDoubleClicked(int logicalIndex)
{
	ToggleTableRow(ui.PostAffineTable, logicalIndex);
}

/// <summary>
/// Toggle all post affine values in one column for the selected xforms.
/// Resets the rendering process.
/// </summary>
/// <param name="logicalIndex">The index of the row that was double clicked</param>
void Fractorium::OnPostAffineColDoubleClicked(int logicalIndex)
{
	ToggleTableCol(ui.PostAffineTable, logicalIndex);
}

/// <summary>
/// Helper for setting the value of a single pre/post affine coefficient.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The value to set</param>
/// <param name="index">The index to set, 0-5, corresponding to a, b, c, d, e, f</param>
/// <param name="pre">True if pre affine, false if post affine.</param>
template <typename T>
void FractoriumEmberController<T>::AffineSetHelper(double d, int index, bool pre)
{
	UpdateXform([&] (Xform<T>* xform, size_t xfindex, size_t selIndex)
	{
		auto& affine = pre ? xform->m_Affine : xform->m_Post;
		AffineDoubleSpinBox** spinners = pre ? m_Fractorium->m_PreSpins : m_Fractorium->m_PostSpins;

		if (m_Fractorium->ui.PolarAffineCheckBox->isChecked())
		{
			switch (index)
			{
				case 0:
				case 3:
					affine.A(std::cos(spinners[0]->value() * DEG_2_RAD) * spinners[3]->value());
					affine.D(std::sin(spinners[0]->value() * DEG_2_RAD) * spinners[3]->value());
					break;

				case 1:
				case 4:
					affine.B(std::cos(spinners[1]->value() * DEG_2_RAD) * spinners[4]->value());
					affine.E(std::sin(spinners[1]->value() * DEG_2_RAD) * spinners[4]->value());
					break;

				case 2:
				case 5:
				default:
					affine.C(std::cos(spinners[2]->value() * DEG_2_RAD) * spinners[5]->value());
					affine.F(std::sin(spinners[2]->value() * DEG_2_RAD) * spinners[5]->value());
					break;
			}
		}
		else
		{
			switch (index)
			{
				case 0:
					affine.A(d);
					break;

				case 1:
					affine.B(d);
					break;

				case 2:
					affine.C(d);
					break;

				case 3:
					affine.D(d);
					break;

				case 4:
					affine.E(d);
					break;

				case 5:
					affine.F(d);
					break;
			}
		}
	}, eXformUpdate::UPDATE_SELECTED);
}

/// <summary>
/// Pre and post affine spinner changed events.
/// Resets the rendering process.
/// </summary>
void Fractorium::OnX1Changed(double d) { m_Controller->AffineSetHelper(d, 0, sender() == m_PreX1Spin); }
void Fractorium::OnX2Changed(double d) { m_Controller->AffineSetHelper(d, 3, sender() == m_PreX2Spin); }
void Fractorium::OnY1Changed(double d) { m_Controller->AffineSetHelper(d, 1, sender() == m_PreY1Spin); }
void Fractorium::OnY2Changed(double d) { m_Controller->AffineSetHelper(d, 4, sender() == m_PreY2Spin); }
void Fractorium::OnO1Changed(double d) { m_Controller->AffineSetHelper(d, 2, sender() == m_PreO1Spin); }
void Fractorium::OnO2Changed(double d) { m_Controller->AffineSetHelper(d, 5, sender() == m_PreO2Spin); }

/// <summary>
/// Flip the selected pre/post affines vertically and/or horizontally.
/// Resets the rendering process.
/// </summary>
/// <param name="horizontal">True to flip horizontally</param>
/// <param name="vertical">True to flip vertically</param>
/// <param name="pre">True if pre affine, else post affine.</param>
template <typename T>
void FractoriumEmberController<T>::FlipXforms(bool horizontal, bool vertical, bool pre)
{
	UpdateXform([&] (Xform<T>* xform, size_t xfindex, size_t selIndex)
	{
		auto& affine = pre ? xform->m_Affine : xform->m_Post;

		if (horizontal)
		{
			affine.A(-affine.A());
			affine.B(-affine.B());

			if (!m_Fractorium->LocalPivot())
				affine.C(-affine.C());
		}

		if (vertical)
		{
			affine.D(-affine.D());
			affine.E(-affine.E());

			if (!m_Fractorium->LocalPivot())
				affine.F(-affine.F());
		}
	}, eXformUpdate::UPDATE_SELECTED);
	FillAffineWithXform(CurrentXform(), pre);
}

void Fractorium::OnFlipHorizontalButtonClicked(bool checked) { m_Controller->FlipXforms(true, false, sender() == ui.PreFlipHorizontalButton); }
void Fractorium::OnFlipVerticalButtonClicked(bool checked) { m_Controller->FlipXforms(false, true, sender() == ui.PreFlipVerticalButton); }

/// <summary>
/// Rotate the selected pre/post affines transform x degrees.
/// Resets the rendering process.
/// </summary>
/// <param name="angle">The angle to rotate by</param>
/// <param name="pre">True if pre affine, else post affine.</param>
template <typename T>
void FractoriumEmberController<T>::RotateXformsByAngle(double angle, bool pre)
{
	UpdateXform([&] (Xform<T>* xform, size_t xfindex, size_t selIndex)
	{
		auto& affine = pre ? xform->m_Affine : xform->m_Post;
		affine.Rotate(angle * DEG_2_RAD_T);
	}, eXformUpdate::UPDATE_SELECTED);
	FillAffineWithXform(CurrentXform(), pre);
}

/// <summary>
/// Rotate the selected pre/post affine transform 90 degrees clockwise/counter clockwise.
/// Called when the rotate 90 c/cc pre/post buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnRotate90CButtonClicked(bool checked) { m_Controller->RotateXformsByAngle(90, sender() == ui.PreRotate90CButton); }
void Fractorium::OnRotate90CcButtonClicked(bool checked) { m_Controller->RotateXformsByAngle(-90, sender() == ui.PreRotate90CcButton); }

/// <summary>
/// Rotate the selected pre/post affine transform x degrees clockwise.
/// Called when the rotate x c pre/post buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnRotateCButtonClicked(bool checked)
{
	bool ok;
	const auto pre = sender() == ui.PreRotateCButton;
	const auto combo = pre ? ui.PreRotateCombo : ui.PostRotateCombo;
	const auto d = ToDouble(combo->currentText(), &ok);

	if (ok)
		m_Controller->RotateXformsByAngle(d, pre);
}

/// <summary>
/// Rotate the selected pre/post affine transform x degrees counter clockwise.
/// Called when the rotate x cc pre/post buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnRotateCcButtonClicked(bool checked)
{
	bool ok;
	const auto pre = sender() == ui.PreRotateCcButton;
	const auto combo = pre ? ui.PreRotateCombo : ui.PostRotateCombo;
	const auto d = ToDouble(combo->currentText(), &ok);

	if (ok)
		m_Controller->RotateXformsByAngle(-d, pre);
}

/// <summary>
/// Move the selected pre/post affines in the x and y directions by the specified amount.
/// Resets the rendering process.
/// </summary>
/// <param name="x">The x direction to move</param>
/// <param name="y">The y direction to move</param>
/// <param name="pre">True if pre affine, else post affine.</param>
template <typename T>
void FractoriumEmberController<T>::MoveXforms(double x, double y, bool pre)
{
	UpdateXform([&] (Xform<T>* xform, size_t xfindex, size_t selIndex)
	{
		auto& affine = pre ? xform->m_Affine : xform->m_Post;
		affine.C(affine.C() + x);
		affine.F(affine.F() + y);
	}, eXformUpdate::UPDATE_SELECTED);
	FillAffineWithXform(CurrentXform(), pre);
}

/// <summary>
/// Move the selected pre/post affine transform x units up.
/// Called when the move pre/post up buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnMoveUpButtonClicked(bool checked)
{
	bool ok;
	const auto pre = sender() == ui.PreMoveUpButton;
	const auto combo = pre ? ui.PreMoveCombo : ui.PostMoveCombo;
	const auto d = ToDouble(combo->currentText(), &ok);

	if (ok)
		m_Controller->MoveXforms(0, m_Settings->YAxisUp() ? d : -d, pre);
}

/// <summary>
/// Move the selected pre/post affine transform x units down.
/// Called when the move pre/post down buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnMoveDownButtonClicked(bool checked)
{
	bool ok;
	const auto pre = sender() == ui.PreMoveDownButton;
	const auto combo = pre ? ui.PreMoveCombo : ui.PostMoveCombo;
	const auto d = ToDouble(combo->currentText(), &ok);

	if (ok)
		m_Controller->MoveXforms(0, m_Settings->YAxisUp() ? -d : d, pre);
}

/// <summary>
/// Move the selected pre/post affine transform x units left.
/// Called when the move pre/post left buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnMoveLeftButtonClicked(bool checked)
{
	bool ok;
	const auto pre = sender() == ui.PreMoveLeftButton;
	const auto combo = pre ? ui.PreMoveCombo : ui.PostMoveCombo;
	const auto d = ToDouble(combo->currentText(), &ok);

	if (ok)
		m_Controller->MoveXforms(-d, 0, pre);
}

/// <summary>
/// Move the selected pre/post affine transform x units right.
/// Called when the move pre/post right buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnMoveRightButtonClicked(bool checked)
{
	bool ok;
	const auto pre = sender() == ui.PreMoveRightButton;
	const auto combo = pre ? ui.PreMoveCombo : ui.PostMoveCombo;
	const auto d = ToDouble(combo->currentText(), &ok);

	if (ok)
		m_Controller->MoveXforms(d, 0, pre);
}

/// <summary>
/// Scale the selected pre/post affines by the specified amount.
/// Resets the rendering process.
/// </summary>
/// <param name="scale">The scale value</param>
/// <param name="pre">True if pre affine, else post affine.</param>
template <typename T>
void FractoriumEmberController<T>::ScaleXforms(double scale, bool pre)
{
	UpdateXform([&] (Xform<T>* xform, size_t xfindex, size_t selIndex)
	{
		auto& affine = pre ? xform->m_Affine : xform->m_Post;
		affine.A(affine.A() * scale);
		affine.B(affine.B() * scale);
		affine.D(affine.D() * scale);
		affine.E(affine.E() * scale);
	}, eXformUpdate::UPDATE_SELECTED);
	FillAffineWithXform(CurrentXform(), pre);
}

/// <summary>
/// Scale the selected pre/post affine transform x units down.
/// Called when the scale pre/post down buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnScaleDownButtonClicked(bool checked)
{
	bool ok;
	const auto pre = sender() == ui.PreScaleDownButton;
	const auto combo = pre ? ui.PreScaleCombo : ui.PostScaleCombo;
	const auto d = ToDouble(combo->currentText(), &ok);

	if (ok)
		m_Controller->ScaleXforms(1.0 / (d / 100.0), pre);
}

/// <summary>
/// Scale the selected pre/post affine transform x units up.
/// Called when the scale pre/post up buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnScaleUpButtonClicked(bool checked)
{
	bool ok;
	const auto pre = sender() == ui.PreScaleUpButton;
	const auto combo = pre ? ui.PreScaleCombo : ui.PostScaleCombo;
	const auto d = ToDouble(combo->currentText(), &ok);

	if (ok)
		m_Controller->ScaleXforms(d / 100.0, pre);
}

/// <summary>
/// Reset selected pre/post affines to the identity matrix.
/// Called when reset pre/post affine buttons are clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="pre">True for pre affine, false for post.</param>
template <typename T>
void FractoriumEmberController<T>::ResetXformsAffine(bool pre)
{
	UpdateXform([&] (Xform<T>* xform, size_t xfindex, size_t selIndex)
	{
		auto& affine = pre ? xform->m_Affine : xform->m_Post;
		affine.MakeID();
	}, eXformUpdate::UPDATE_SELECTED);
	FillAffineWithXform(CurrentXform(), pre);
}

void Fractorium::OnResetAffineButtonClicked(bool checked) { m_Controller->ResetXformsAffine(sender() == ui.PreResetButton); }

/// <summary>
/// Copy the pre or post affine transform values from the current xform.
/// </summary>
/// <param name="pre">True for pre affine, false for post.</param>
template <typename T>
void FractoriumEmberController<T>::CopyXformsAffine(bool pre)
{
	if (const auto xform = CurrentXform())
		m_CopiedAffine = pre ? xform->m_Affine : xform->m_Post;
}

void Fractorium::OnCopyAffineButtonClicked(bool checked)
{
	m_Controller->CopyXformsAffine(sender() == ui.PreCopyButton);
}

/// <summary>
/// Paste the last copied affine transform values to the pre or post affine values in the current xform.
/// </summary>
/// <param name="pre">True for pre affine, false for post.</param>
template <typename T>
void FractoriumEmberController<T>::PasteXformsAffine(bool pre)
{
	UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
	{
		auto& affine = pre ? xform->m_Affine : xform->m_Post;
		affine = m_CopiedAffine;
	}, eXformUpdate::UPDATE_CURRENT);
	FillAffineWithXform(CurrentXform(), pre);
}

void Fractorium::OnPasteAffineButtonClicked(bool checked)
{
	m_Controller->PasteXformsAffine(sender() == ui.PrePasteButton);
}

/// <summary>
/// Randomize all values in selected pre/post affines to a range of -1 to 1.
/// Called when random pre/post affine buttons are clicked.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::RandomXformsAffine(bool pre)
{
	UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
	{
		auto& affine = pre ? xform->m_Affine : xform->m_Post;
		affine.A(m_Rand.Frand11<T>());
		affine.B(m_Rand.Frand11<T>());
		affine.C(m_Rand.Frand11<T>());
		affine.D(m_Rand.Frand11<T>());
		affine.E(m_Rand.Frand11<T>());
		affine.F(m_Rand.Frand11<T>());
	}, eXformUpdate::UPDATE_SELECTED);
	FillAffineWithXform(CurrentXform(), pre);
}

void Fractorium::OnRandomAffineButtonClicked(bool checked) { m_Controller->RandomXformsAffine(sender() == ui.PreRandomButton); }

/// <summary>
/// Fill the GUI with the pre and post affine xform values.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::FillBothAffines()
{
	FillAffineWithXform(CurrentXform(), true);
	FillAffineWithXform(CurrentXform(), false);
}

/// <summary>
/// Fill the GUI with the pre/post affine xform values.
/// </summary>
/// <param name="xform">The xform to fill with</param>
/// <param name="pre">True if pre affine, else post affine.</param>
template <typename T>
void FractoriumEmberController<T>::FillAffineWithXform(Xform<T>* xform, bool pre)
{
	AffineDoubleSpinBox** spinners = pre ? m_Fractorium->m_PreSpins : m_Fractorium->m_PostSpins;
	auto& affine = pre ? xform->m_Affine : xform->m_Post;

	if (m_Fractorium->ui.PolarAffineCheckBox->isChecked())
	{
		spinners[0]->SetValueStealth(NormalizeDeg360(RAD_2_DEG * std::atan2(affine.D(), affine.A())));
		spinners[1]->SetValueStealth(NormalizeDeg360(RAD_2_DEG * std::atan2(affine.E(), affine.B())));
		spinners[2]->SetValueStealth(NormalizeDeg360(RAD_2_DEG * std::atan2(affine.F(), affine.C())));
		spinners[3]->SetValueStealth(VarFuncs<T>::Hypot(affine.D(), affine.A()));
		spinners[4]->SetValueStealth(VarFuncs<T>::Hypot(affine.E(), affine.B()));
		spinners[5]->SetValueStealth(VarFuncs<T>::Hypot(affine.F(), affine.C()));
	}
	else
	{
		spinners[0]->SetValueStealth(affine.A());
		spinners[1]->SetValueStealth(affine.B());
		spinners[2]->SetValueStealth(affine.C());
		spinners[3]->SetValueStealth(affine.D());
		spinners[4]->SetValueStealth(affine.E());
		spinners[5]->SetValueStealth(affine.F());
	}
}

/// <summary>
/// Trigger a redraw which will show or hide the circle affine transforms
/// based on whether each group box is checked or not.
/// Note that all sub buttons must manually be disabled/enabled in order to
/// get the top left corner button in the proper state. This is needed so
/// any style sheets can properly draw it based on its state.
/// Without explicitly setting it, that button is never actually disabled.
/// Called when the group box check box for pre or post affine is checked.
/// </summary>
/// <param name="on">Ignored</param>
void Fractorium::OnAffineGroupBoxToggled(bool on)
{
	auto widgetList = sender()->findChildren<QAbstractButton*>();

	for (auto& it : widgetList)
		it->setEnabled(on);

	ui.GLDisplay->update();
}

/// <summary>
/// Swap the values of the pre and post affines for the selected xforms.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SwapAffines()
{
	UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
	{
		const auto pre = xform->m_Affine;
		const auto post = xform->m_Post;
		xform->m_Affine = post;
		xform->m_Post = pre;
	}, eXformUpdate::UPDATE_SELECTED);
	FillBothAffines();
}

void Fractorium::OnSwapAffinesButtonClicked(bool checked)
{
	m_Controller->SwapAffines();
}

/// <summary>
/// Trigger a redraw which will show all, or only the current, circle affine transforms
/// based on which radio buttons are selected.
/// Called when and pre/post show all/current radio buttons are checked.
/// </summary>
/// <param name="on">Ignored</param>
void Fractorium::OnAffineDrawAllCurrentRadioButtonToggled(bool checked)
{
	OnCurrentXformComboChanged(ui.CurrentXformCombo->currentIndex());
	ui.GLDisplay->update();
}

/// <summary>
/// Set whether to display affine values in polar vs. rectangular coordinates.
/// Updates the current affine display.
/// </summary>
/// <param name="state">The state of the checkbox</param>
void Fractorium::OnPolarAffineCheckBoxStateChanged(Qt::CheckState state)
{
	const auto mult = state ? 100 : 0.01;
	const auto step = m_PreX1Spin->Step() * mult;
	const auto small = m_PreX1Spin->SmallStep() * mult;
	const auto click = state ? 90 : 1;

	for (int i = 0; i < 3; i++)
	{
		m_PreSpins[i]->Step(step);
		m_PreSpins[i]->SmallStep(small);
		m_PostSpins[i]->Step(step);
		m_PostSpins[i]->SmallStep(small);
		m_PreSpins[i]->DoubleClickZero(click);
		m_PostSpins[i]->DoubleClickZero(click);
	}

	m_Controller->FillBothAffines();
}

/// <summary>
/// Setup a spinner to be placed in a table cell.
/// Special setup function for affine spinners which differs slightly from the regular
/// SetupSpinner() function.
/// </summary>
/// <param name="table">The table the spinner belongs to</param>
/// <param name="receiver">The receiver object</param>
/// <param name="row">The row in the table where this spinner resides</param>
/// <param name="col">The col in the table where this spinner resides</param>
/// <param name="spinBox">Double pointer to spin box which will hold the spinner upon exit</param>
/// <param name="height">The height of the spinner</param>
/// <param name="min">The minimum value of the spinner</param>
/// <param name="max">The maximum value of the spinner</param>
/// <param name="step">The step of the spinner</param>
/// <param name="prec">The precision of the spinner</param>
/// <param name="signal">The signal the spinner emits</param>
/// <param name="slot">The slot to receive the signal</param>
void Fractorium::SetupAffineSpinner(QTableWidget* table, const QObject* receiver, int row, int col, AffineDoubleSpinBox*& spinBox, int height, double min, double max, double step, double prec, const char* signal, const char* slot)
{
	spinBox = new AffineDoubleSpinBox(table, height, step);
	spinBox->setRange(min, max);
	spinBox->setDecimals(prec);
	table->setCellWidget(row, col, spinBox);
	connect(spinBox, signal, receiver, slot, Qt::ConnectionType::QueuedConnection);
	spinBox->DoubleClick(true);
	spinBox->DoubleClickNonZero(0);
	spinBox->DoubleClickZero(1);
}

/// <summary>
/// GUI wrapper function, getter only.
/// </summary>

bool Fractorium::LocalPivot()  { return ui.LocalPivotRadio->isChecked();        }

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
