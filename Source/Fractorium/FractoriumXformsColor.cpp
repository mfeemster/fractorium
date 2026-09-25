#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the xforms color UI.
/// </summary>
void Fractorium::InitXformsColorUI()
{
	const auto spinHeight = 20;
	auto row = 0;
	m_XformColorValueItem = new QTableWidgetItem();
	//Can't set this in the designer, so do it here.
	m_XformColorValueItem->setToolTip("The index in the palette the current xform uses.\r\n\r\n"
									  "This value can be changed by scrolling the mouse wheel in the box displaying the value or by dragging the scroll bar.");
	ui.XformColorIndexTable->setItem(0, 0, m_XformColorValueItem);
	m_PaletteRefItem = new QTableWidgetItem();
	ui.XformPaletteRefTable->setItem(0, 0, m_PaletteRefItem);
	ui.XformPaletteRefTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	connect(ui.XformPaletteRefTable->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(OnXformRefPaletteResized(int, int, int)), Qt::ConnectionType::QueuedConnection);
	connect(ui.RandomColorIndicesButton,                 SIGNAL(clicked(bool)),                 this, SLOT(OnRandomColorIndicesButtonClicked(bool)), Qt::ConnectionType::QueuedConnection);
	connect(ui.ToggleColorIndicesButton,                 SIGNAL(clicked(bool)),                 this, SLOT(OnToggleColorIndicesButtonClicked(bool)), Qt::ConnectionType::QueuedConnection);
	connect(ui.RandomColorSpeedButton,                   SIGNAL(clicked(bool)),                 this, SLOT(OnRandomColorSpeedButtonClicked(bool)),   Qt::ConnectionType::QueuedConnection);
	connect(ui.ToggleColorSpeedsButton,                  SIGNAL(clicked(bool)),                 this, SLOT(OnToggleColorSpeedsButtonClicked(bool)),  Qt::ConnectionType::QueuedConnection);
	SetupSpinner<DoubleSpinBox, double>(ui.XformColorIndexTable,  this, row, 1, m_XformColorIndexSpin,  spinHeight,  0,   1, 0.01, SIGNAL(valueChanged(double)),  SLOT(OnXformColorIndexChanged(double)),  false,   0,   1, 0);
	SetupSpinner<DoubleSpinBox, double>(ui.XformColorValuesTable, this, row, 1, m_XformColorSpeedSpin,  spinHeight, -1,   1,  0.1,  SIGNAL(valueChanged(double)), SLOT(OnXformColorSpeedChanged(double)),  true,  0.5, 0.5, 0);
	SetupSpinner<DoubleSpinBox, double>(ui.XformColorValuesTable, this, row, 1, m_XformOpacitySpin,	    spinHeight,  0, 100,  0.1,  SIGNAL(valueChanged(double)), SLOT(OnXformOpacityChanged(double)),	   true,    1,   1, 0);
	SetupSpinner<DoubleSpinBox, double>(ui.XformColorValuesTable, this, row, 1, m_XformDirectColorSpin, spinHeight,  0,   1,  0.1,  SIGNAL(valueChanged(double)), SLOT(OnXformDirectColorChanged(double)), true,    1,   1, 0);
	m_XformColorIndexSpin->setDecimals(3);
	m_XformColorSpeedSpin->setDecimals(3);
	m_XformOpacitySpin->setDecimals(3);
	m_XformDirectColorSpin->setDecimals(3);
	connect(ui.XformColorScroll,  SIGNAL(valueChanged(int)),                 this, SLOT(OnXformScrollColorIndexChanged(int)),             Qt::ConnectionType::QueuedConnection);
	connect(ui.SoloXformCheckBox, SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(OnSoloXformCheckBoxStateChanged(Qt::CheckState)), Qt::ConnectionType::QueuedConnection);
}

/// <summary>
/// Set the color index of the selected xforms.
/// Update the color index scrollbar to match.
/// Called when the spinner in the color index cell in the palette ref table is changed.
/// Optionally resets the rendering process.
/// </summary>
/// <param name="d">The color index, 0-1/</param>
/// <param name="updateRender">True to reset the rendering process, else don't.</param>
/// <param name="updateSpinner">True to update the color index spinner with the new value, else don't.</param>
/// <param name="updateScroll">True to update the color index scroll bar with the new value, else don't.</param>
/// <param name="update">The type of updating to do, default: eXformUpdate::UPDATE_SELECTED.</param>
/// <param name="index">The index of the xform to update. Ignored unless update is eXformUpdate::UPDATE_SPECIFIC. Default: 0.</param>
template <typename T>
void FractoriumEmberController<T>::XformColorIndexChanged(double d, bool updateRender, bool updateSpinner, bool updateScroll, eXformUpdate update, size_t index)
{
	const auto updateGUI = update != eXformUpdate::UPDATE_SPECIFIC || index == (size_t)m_Fractorium->ui.CurrentXformCombo->currentIndex();

	if (updateRender)//False when just updating GUI in response to a change elsewhere, true when in response to a GUI change so update values and reset renderer.
	{
		UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
		{
			xform->m_ColorX = Clamp<T>(d, 0, 1);
		}, update, updateRender, eProcessAction::FULL_RENDER, index);
	}

	//Only do this is coming from elsewhere, like the palette editor. Otherwise, normal events will handle updating the spinner.
	if (updateSpinner && updateGUI)
	{
		m_Fractorium->m_XformColorIndexSpin->SetValueStealth(CurrentXform()->m_ColorX);
	}

	if (updateScroll && updateGUI)
	{
		const auto scroll = m_Fractorium->ui.XformColorScroll;
		const auto scrollVal = d * scroll->maximum();
		scroll->blockSignals(true);
		scroll->setValue(scrollVal);
		scroll->blockSignals(false);
	}

	if (updateGUI)
		m_Fractorium->ui.XformColorIndexTable->item(0, 0)->setBackground(ColorIndexToQColor(d));//Grab the current color from the index and assign it to the first cell of the first table.
}

void Fractorium::OnXformColorIndexChanged(double d) { OnXformColorIndexChanged(d, true, false, true, eXformUpdate::UPDATE_SELECTED, std::numeric_limits<size_t>::max()); }
void Fractorium::OnXformColorIndexChanged(double d, bool updateRender, bool updateSpinner, bool updateScroll, eXformUpdate update, size_t index) { m_Controller->XformColorIndexChanged(d, updateRender, updateSpinner, updateScroll, update, index); }

/// <summary>
/// Set the color index of the current xform.
/// Will trigger an update which will cause the color index cell in the palette ref table to match.
/// Called when the color index scrollbar is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The color index, 0-1.</param>
void Fractorium::OnXformScrollColorIndexChanged(int d)
{
	OnXformColorIndexChanged(d / static_cast<double>(ui.XformColorScroll->maximum()), true, true, false);//Update spinner, but not scrollbar. Trigger render update.
}

/// <summary>
/// Set all xform color indices to a random value between 0 and 1, inclusive.
/// Called when the Random Indices button is clicked.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::RandomColorIndicesButtonClicked()
{
	UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex) { xform->m_ColorX = m_Rand.Frand01<T>(); }, eXformUpdate::UPDATE_ALL, false);//Don't update renderer here...
	m_Fractorium->m_XformColorIndexSpin->setValue(CurrentXform()->m_ColorX);//...do it via GUI. This will set scrollbar value as well.
}
void Fractorium::OnRandomColorIndicesButtonClicked(bool b) { m_Controller->RandomColorIndicesButtonClicked(); }

/// <summary>
/// Resets the rendering process.
/// Set all xform color indices to either 0 and 1, sequentially toggling.
/// Called when the Toggle Indices button is clicked.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ToggleColorIndicesButtonClicked()
{
	char ch = 1;
	UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex) { xform->m_ColorX = static_cast<T>(ch ^= 1); }, eXformUpdate::UPDATE_ALL, false);//Don't update renderer here...
	m_Fractorium->m_XformColorIndexSpin->setValue(CurrentXform()->m_ColorX);//...do it via GUI. This will set scrollbar value as well.
}
void Fractorium::OnToggleColorIndicesButtonClicked(bool b) { m_Controller->ToggleColorIndicesButtonClicked(); }

/// <summary>
/// Set all xform color speeds to a random value between 0 and 1, inclusive.
/// Called when the Random Color Speed button is clicked.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::RandomColorSpeedButtonClicked()
{
	UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex) { xform->m_ColorSpeed = m_Rand.Frand01<T>(); }, eXformUpdate::UPDATE_ALL);
	m_Fractorium->m_XformColorSpeedSpin->SetValueStealth(CurrentXform()->m_ColorSpeed);
}
void Fractorium::OnRandomColorSpeedButtonClicked(bool b) { m_Controller->RandomColorSpeedButtonClicked(); }

/// <summary>
/// Set all xform color speeds to either 0 and 0.5, sequentially toggling.
/// Called when the Toggle Color Speed button is clicked.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ToggleColorSpeedsButtonClicked()
{
	char ch = 1;
	UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex) { xform->m_ColorSpeed = static_cast<T>((ch ^= 1) ? 0.5 : 0.0); }, eXformUpdate::UPDATE_ALL);
	m_Fractorium->m_XformColorSpeedSpin->SetValueStealth(CurrentXform()->m_ColorSpeed);
}
void Fractorium::OnToggleColorSpeedsButtonClicked(bool b) { m_Controller->ToggleColorSpeedsButtonClicked(); }

/// <summary>
/// Set the color speed of the selected xforms.
/// Called when the xform color speed spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The color speed, -1-1.</param>
template <typename T>
void FractoriumEmberController<T>::XformColorSpeedChanged(double d) { UpdateXform([&] (Xform<T>* xform, size_t xfindex, size_t selIndex) { xform->m_ColorSpeed = d; }, eXformUpdate::UPDATE_SELECTED); }
void Fractorium::OnXformColorSpeedChanged(double d) { m_Controller->XformColorSpeedChanged(d); }

/// <summary>
/// Set the opacity of the selected xforms.
/// Called when xform opacity spinner is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The opacity, 0-1.</param>
template <typename T>
void FractoriumEmberController<T>::XformOpacityChanged(double d) { UpdateXform([&] (Xform<T>* xform, size_t xfindex, size_t selIndex) { xform->m_Opacity = d; }, eXformUpdate::UPDATE_SELECTED); }
void Fractorium::OnXformOpacityChanged(double d) { m_Controller->XformOpacityChanged(d); }

/// <summary>
/// Set the direct color percentage of the selected xforms.
/// Called when the xform direct color spinner is changed.
/// Note this only affects xforms that include a dc_ variation.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The direct color percentage, 0-1.</param>
template <typename T>
void FractoriumEmberController<T>::XformDirectColorChanged(double d) { UpdateXform([&] (Xform<T>* xform, size_t xfindex, size_t selIndex) { xform->m_DirectColor = d; }, eXformUpdate::UPDATE_SELECTED); }
void Fractorium::OnXformDirectColorChanged(double d) { m_Controller->XformDirectColorChanged(d); }

/// <summary>
/// Set whether the current xform should be rendered solo.
/// If checked, current is solo, if unchecked, none are solo.
/// Solo means that all other xforms will have their opacity temporarily
/// set to zero while rendering so that only the effect of current xform is visible.
/// This will not permanently alter the opacities of ember, as the temporary opacity values will be applied
/// right before rendering and reset right after.
/// Called when the solo xform check box is checked.
/// Resets the rendering process.
/// </summary>
/// <param name="state">The state of the checkbox</param>
/// <param name="index">The index which has been specified as the solo xform, -1 to specify none.</param>
template <typename T>
void FractoriumEmberController<T>::SoloXformCheckBoxStateChanged(Qt::CheckState state, int index)
{
	if (state == Qt::CheckState::Checked)
	{
		m_Ember.m_Solo = index;
		m_Fractorium->ui.SoloXformCheckBox->setText("Solo (" + ToString(index + 1) + ")");
	}
	else if (state == Qt::CheckState::Unchecked)
	{
		m_Ember.m_Solo = -1;
		m_Fractorium->ui.SoloXformCheckBox->setText("Solo");
	}

	UpdateRender();
}

void Fractorium::OnSoloXformCheckBoxStateChanged(Qt::CheckState state) { m_Controller->SoloXformCheckBoxStateChanged(state, ui.CurrentXformCombo->currentIndex()); }

/// <summary>
/// Redraw the palette ref table.
/// Called on resize.
/// </summary>
/// <param name="logicalIndex">Ignored</param>
/// <param name="oldSize">Ignored</param>
/// <param name="newSize">Ignored</param>
void Fractorium::OnXformRefPaletteResized(int logicalIndex, int oldSize, int newSize)
{
	QPixmap pixmap(QPixmap::fromImage(m_Controller->FinalPaletteImage()));
	SetPaletteTableItem(&pixmap, ui.XformPaletteRefTable, m_PaletteRefItem, 0, 0);
}

/// <summary>
/// Look up the passed in index in the current ember's palette
/// and return the QColor equivalent.
/// </summary>
/// <param name="d">The palette index to look up, 0-1.</param>
/// <returns>The palette color at the given index as a QColor</returns>
template <typename T>
QColor FractoriumEmberController<T>::ColorIndexToQColor(double d)
{
	v4F entry = m_Ember.m_Palette[Clamp<size_t>(d * (m_Ember.m_Palette.Size() - 1), 0, m_Ember.m_Palette.Size())];
	entry.r *= 255;
	entry.g *= 255;
	entry.b *= 255;
	QRgb rgb = static_cast<uint>(entry.r) << 16 | static_cast<uint>(entry.g) << 8 | static_cast<uint>(entry.b);
	return QColor::fromRgb(rgb);
}

/// <summary>
/// Set the color index, speed and opacity spinners with the values of the passed in xform.
/// Set the cells of the palette ref table as well.
/// </summary>
/// <param name="xform">The xform whose values will be copied to the GUI</param>
template <typename T>
void FractoriumEmberController<T>::FillColorWithXform(Xform<T>* xform)
{
	m_Fractorium->m_XformColorIndexSpin->SetValueStealth(xform->m_ColorX);//Probably ought to put scroll bar update here too.
	m_Fractorium->m_XformColorSpeedSpin->SetValueStealth(xform->m_ColorSpeed);
	m_Fractorium->m_XformOpacitySpin->SetValueStealth(xform->m_Opacity);
	m_Fractorium->m_XformDirectColorSpin->SetValueStealth(xform->m_DirectColor);
	m_Fractorium->OnXformColorIndexChanged(xform->m_ColorX, false, false, true);//Had to call stealth before to avoid doing an update, now manually update related controls, still without doing an update.
}

/// <summary>
/// Set the cell at the row and column in the passed in table to the passed in pixmap.
/// </summary>
/// <param name="pixmap">The pixmap to assign</param>
/// <param name="table">The table whose cell will be filled with the image</param>
/// <param name="item">The QTableWidgetItem in the cell</param>
/// <param name="row">The row of the cell</param>
/// <param name="col">The column of the cell</param>
void Fractorium::SetPaletteTableItem(QPixmap* pixmap, QTableWidget* table, QTableWidgetItem* item, int row, int col)
{
	if (pixmap && !pixmap->isNull())
	{
		const QSize size(table->columnWidth(col), table->rowHeight(row) + 1);
		item->setData(Qt::ItemDataRole::DecorationRole, pixmap->scaled(size, Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));
	}
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
