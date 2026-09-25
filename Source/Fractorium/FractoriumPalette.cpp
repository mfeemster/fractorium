#include "FractoriumPch.h"
#include "Fractorium.h"
#include "PaletteTableWidgetItem.h"

/// <summary>
/// Initialize the palette UI.
/// </summary>
void Fractorium::InitPaletteUI()
{
	int spinHeight = 20, row = 0;
	auto paletteTable = ui.PaletteListTable;
	auto palettePreviewTable = ui.PalettePreviewTable;
	connect(ui.PaletteFilenameCombo, SIGNAL(currentTextChanged(const QString&)),           this, SLOT(OnPaletteFilenameComboChanged(const QString&)),                Qt::ConnectionType::QueuedConnection);
	connect(paletteTable,            SIGNAL(cellClicked(int, int)),                        this, SLOT(OnPaletteCellClicked(int, int)),                               Qt::ConnectionType::QueuedConnection);
	connect(paletteTable,            SIGNAL(cellDoubleClicked(int, int)),                  this, SLOT(OnPaletteCellDoubleClicked(int, int)),                         Qt::ConnectionType::QueuedConnection);
	connect(palettePreviewTable,     SIGNAL(MouseDragged(const QPointF&, const QPointF&)), this, SLOT(OnPreviewPaletteMouseDragged(const QPointF&, const QPointF&)), Qt::ConnectionType::QueuedConnection);
	connect(palettePreviewTable,     SIGNAL(MouseReleased()),                              this, SLOT(OnPreviewPaletteMouseReleased()),                              Qt::ConnectionType::QueuedConnection);
	connect(palettePreviewTable,     SIGNAL(cellDoubleClicked(int, int)),                  this, SLOT(OnPreviewPaletteCellDoubleClicked(int, int)),                  Qt::ConnectionType::QueuedConnection);
	connect(palettePreviewTable,     SIGNAL(cellPressed(int, int)),                        this, SLOT(OnPreviewPaletteCellPressed(int, int)),                        Qt::ConnectionType::QueuedConnection);
	//Palette adjustment table.
	auto table = ui.PaletteAdjustTable;
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//Split width over all columns evenly.
	SetupSpinner<SpinBox, int>(table, this, row, 1, m_PaletteHueSpin,		 spinHeight, -180, 180, 1, SIGNAL(valueChanged(int)), SLOT(OnPaletteAdjust(int)), true, 0, 0, 0);
	SetupSpinner<SpinBox, int>(table, this, row, 1, m_PaletteSaturationSpin, spinHeight, -100, 100, 1, SIGNAL(valueChanged(int)), SLOT(OnPaletteAdjust(int)), true, 0, 0, 0);
	SetupSpinner<SpinBox, int>(table, this, row, 1, m_PaletteBrightnessSpin, spinHeight, -255, 255, 1, SIGNAL(valueChanged(int)), SLOT(OnPaletteAdjust(int)), true, 0, 0, 0);
	row = 0;
	SetupSpinner<SpinBox, int>(table, this, row, 3, m_PaletteContrastSpin,  spinHeight, -100, 100, 1, SIGNAL(valueChanged(int)), SLOT(OnPaletteAdjust(int)), true, 0, 0, 0);
	SetupSpinner<SpinBox, int>(table, this, row, 3, m_PaletteBlurSpin,	    spinHeight,	   0, 127, 1, SIGNAL(valueChanged(int)), SLOT(OnPaletteAdjust(int)), true, 0, 0, 0);
	SetupSpinner<SpinBox, int>(table, this, row, 3, m_PaletteFrequencySpin, spinHeight,	   1,  10, 1, SIGNAL(valueChanged(int)), SLOT(OnPaletteAdjust(int)), true, 1, 1, 1);
	connect(ui.PaletteRandomSelectButton, SIGNAL(clicked(bool)), this, SLOT(OnPaletteRandomSelectButtonClicked(bool)), Qt::ConnectionType::QueuedConnection);
	connect(ui.PaletteRandomAdjustButton, SIGNAL(clicked(bool)), this, SLOT(OnPaletteRandomAdjustButtonClicked(bool)), Qt::ConnectionType::QueuedConnection);
	//Palette editor.
	connect(ui.PaletteEditorButton, SIGNAL(clicked(bool)), this, SLOT(OnPaletteEditorButtonClicked(bool)), Qt::ConnectionType::QueuedConnection);
	//Preview table.
	palettePreviewTable->setRowCount(1);
	palettePreviewTable->setColumnWidth(1, 260);//256 plus small margin on each side.
	auto previewNameCol = new QTableWidgetItem("");
	palettePreviewTable->setItem(0, 0, previewNameCol);
	auto previewPaletteItem = new QTableWidgetItem();
	palettePreviewTable->setItem(0, 1, previewPaletteItem);
	connect(ui.PaletteFilterLineEdit,	 SIGNAL(textChanged(const QString&)), this, SLOT(OnPaletteFilterLineEditTextChanged(const QString&)));
	connect(ui.PaletteFilterClearButton, SIGNAL(clicked(bool)),				  this, SLOT(OnPaletteFilterClearButtonClicked(bool)));
	paletteTable->setColumnWidth(1, 260);//256 plus small margin on each side.
	paletteTable->horizontalHeader()->setSectionsClickable(true);
	connect(paletteTable->horizontalHeader(), SIGNAL(sectionClicked(int)),                          this, SLOT(OnPaletteHeaderSectionClicked(int)),             Qt::ConnectionType::QueuedConnection);
	connect(ui.ResetCurvesButton,             SIGNAL(clicked(bool)),                                this, SLOT(OnResetCurvesButtonClicked(bool)),               Qt::ConnectionType::QueuedConnection);
	connect(ui.CurvesView,                    SIGNAL(PointChangedSignal(int, int, const QPointF&)), this, SLOT(OnCurvesPointChanged(int, int, const QPointF&)), Qt::ConnectionType::QueuedConnection);
	connect(ui.CurvesView,                    SIGNAL(PointAddedSignal(size_t, const QPointF&)),     this, SLOT(OnCurvesPointAdded(size_t, const QPointF&)),     Qt::ConnectionType::QueuedConnection);
	connect(ui.CurvesView,                    SIGNAL(PointRemovedSignal(size_t, int)),              this, SLOT(OnCurvesPointRemoved(size_t, int)),              Qt::ConnectionType::QueuedConnection);
	connect(ui.CurvesAllRadio,                SIGNAL(toggled(bool)),                                this, SLOT(OnCurvesAllRadioButtonToggled(bool)),            Qt::ConnectionType::QueuedConnection);
	connect(ui.CurvesRedRadio,                SIGNAL(toggled(bool)),                                this, SLOT(OnCurvesRedRadioButtonToggled(bool)),            Qt::ConnectionType::QueuedConnection);
	connect(ui.CurvesGreenRadio,              SIGNAL(toggled(bool)),                                this, SLOT(OnCurvesGreenRadioButtonToggled(bool)),          Qt::ConnectionType::QueuedConnection);
	connect(ui.CurvesBlueRadio,               SIGNAL(toggled(bool)),                                this, SLOT(OnCurvesBlueRadioButtonToggled(bool)),           Qt::ConnectionType::QueuedConnection);
}

/// <summary>
/// Read all palette Xml files in the specified folder and populate the palette list with the contents.
/// This will clear any previous contents.
/// Called upon initialization, or controller type change.
/// </summary>
/// <param name="s">The full path to the palette files folder</param>
/// <returns>The number of palettes successfully added</returns>
template <typename T>
size_t FractoriumEmberController<T>::InitPaletteList(const QString& s)
{
	QDirIterator it(s, QStringList() << "*.xml" << "*.ugr" << "*.gradient" << "*.gradients", QDir::Files, QDirIterator::FollowSymlinks);

	while (it.hasNext())
	{
		auto path = it.next();
		auto qfilename = it.fileName();

		try
		{
			if (QFile::exists(path) && m_PaletteList->Add(path.toStdString()))
				m_Fractorium->ui.PaletteFilenameCombo->addItem(qfilename);
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(nullptr, "Palette Parsing Error", QString::fromStdString(e.what()));
		}
		catch (const char* e)
		{
			QMessageBox::critical(nullptr, "Palette Parsing Error", e);
		}
	}

	m_Fractorium->ui.PaletteFilenameCombo->model()->sort(0);
	return m_PaletteList->Size();
}

/// <summary>
/// Read a palette Xml file and populate the palette table with the contents.
/// This will clear any previous contents.
/// Called upon initialization, palette combo index change, and controller type change.
/// </summary>
/// <param name="s">The name of the palette file without the path</param>
/// <returns>True if successful, else false.</returns>
template <typename T>
bool FractoriumEmberController<T>::FillPaletteTable(const string& s)
{
	if (!s.empty())//This occasionally seems to get called with an empty string for reasons unknown.
	{
		auto paletteTable = m_Fractorium->ui.PaletteListTable;
		m_CurrentPaletteFilePath = s;

		if (::FillPaletteTable(m_CurrentPaletteFilePath, paletteTable, m_PaletteList))
		{
			return true;
		}
		else
		{
			vector<string> errors = m_PaletteList->ErrorReport();
			m_Fractorium->ErrorReportToQTextEdit(errors, m_Fractorium->ui.InfoFileOpeningTextEdit);
			m_Fractorium->ShowCritical("Palette Read Error", "Could not load palette file, all images will be black. See info tab for details.");
			m_PaletteList->ClearErrorReport();
		}
	}

	return false;
}

/// <summary>
/// Fill the palette table with the passed in string.
/// Called when the palette name combo box changes.
/// </summary>
/// <param name="text">The full path to the palette file</param>
void Fractorium::OnPaletteFilenameComboChanged(const QString& text)
{
	auto s = text.toStdString();
	m_Controller->FillPaletteTable(s);
	auto fullname = m_Controller->m_PaletteList->GetFullPathFromFilename(s);
	ui.PaletteFilenameCombo->setToolTip(QString::fromStdString(fullname));
	ui.PaletteListTable->sortItems(0, m_PaletteSortMode == 0 ? Qt::SortOrder::AscendingOrder : Qt::SortOrder::DescendingOrder);
}

/// <summary>
/// Apply adjustments to the current ember's palette.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ApplyPaletteToEmber()
{
	const uint blur = m_Fractorium->m_PaletteBlurSpin->value();
	const uint freq = m_Fractorium->m_PaletteFrequencySpin->value();
	const auto sat = m_Fractorium->m_PaletteSaturationSpin->value() / 100.0;
	const auto brightness = m_Fractorium->m_PaletteBrightnessSpin->value() / 255.0;
	const auto contrast = double(m_Fractorium->m_PaletteContrastSpin->value() > 0 ? m_Fractorium->m_PaletteContrastSpin->value() * 2.0 : m_Fractorium->m_PaletteContrastSpin->value()) / 100.0;
	const auto hue = m_Fractorium->m_PaletteHueSpin->value() / 360.0;
	//Use the temp palette as the base and apply the adjustments gotten from the GUI and save the result in the ember palette.
	m_TempPalette.MakeAdjustedPalette(m_Ember.m_Palette, m_Fractorium->m_PreviewPaletteRotation, hue, sat, brightness, contrast, blur, freq);
}

/// <summary>
/// Use adjusted palette to update all related GUI controls with new color values.
/// Resets the rendering process.
/// </summary>
/// <param name="palette">The palette to use</param>
/// <param name="paletteName">Name of the palette</param>
template <typename T>
void FractoriumEmberController<T>::UpdateAdjustedPaletteGUI(Palette<float>& palette)
{
	const auto xform = CurrentXform();
	const auto palettePreviewTable = m_Fractorium->ui.PalettePreviewTable;
	const auto paletteName = QString::fromStdString(m_Ember.m_Palette.m_Name);
	auto previewPaletteItem = palettePreviewTable->item(0, 1);

	if (previewPaletteItem)//This can be null if the palette file was moved or corrupted.
	{
		//Use the adjusted palette to fill the preview palette control so the user can see the effects of applying the adjustements.
		vector<unsigned char> v = palette.MakeRgbPaletteBlock(PALETTE_CELL_HEIGHT);//Make the palette repeat for PALETTE_CELL_HEIGHT rows.
		m_FinalPaletteImage = QImage(int(palette.Size()), PALETTE_CELL_HEIGHT, QImage::Format_RGB888);//Create a QImage out of it.
		memcpy(m_FinalPaletteImage.scanLine(0), v.data(), v.size() * sizeof(v[0]));//Memcpy the data in.
		QPixmap pixmap(QPixmap::fromImage(m_FinalPaletteImage));//Create a QPixmap out of the QImage.
		previewPaletteItem->setData(Qt::ItemDataRole::DecorationRole, pixmap.scaled(QSize(pixmap.width(), palettePreviewTable->rowHeight(0) + 2), Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));//Set the pixmap on the palette tab.
		m_Fractorium->SetPaletteTableItem(&pixmap, m_Fractorium->ui.XformPaletteRefTable, m_Fractorium->m_PaletteRefItem, 0, 0);//Set the palette ref table on the xforms | color tab.

		if (auto previewNameItem = palettePreviewTable->item(0, 0))
		{
			previewNameItem->setText(paletteName);//Finally, set the name of the palette to be both the text and the tooltip.
			previewNameItem->setToolTip(paletteName);
		}
	}

	//Update the current xform's color and reset the rendering process.
	//Update all controls to be safe.
	if (xform)
		XformColorIndexChanged(xform->m_ColorX, true, true, true);
}

/// <summary>
/// Apply all adjustments to the selected palette, show it
/// and assign it to the current ember.
/// Called when any adjustment spinner is modified.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::PaletteAdjust()
{
	Update([&]()
	{
		ApplyPaletteToEmber();
		UpdateAdjustedPaletteGUI(m_Ember.m_Palette);
	});
}

void Fractorium::OnPaletteAdjust(int d) { m_Controller->PaletteAdjust(); }

/// <summary>
/// Set the passed in palette as the current one,
/// applying any adjustments previously specified.
/// Resets the rendering process.
/// </summary>
/// <param name="palette">The palette to assign to the temporary palette</param>
template <typename T>
void FractoriumEmberController<T>::SetBasePaletteAndAdjust(const Palette<float>& palette)
{
	//The temp palette is assigned the palette read when the file was parsed/saved. The user can apply adjustments on the GUI later.
	//These adjustments will be applied to the temp palette, then assigned back to m_Ember.m_Palette.
	m_TempPalette = palette;//Deep copy.
	ApplyPaletteToEmber();//Copy temp palette to ember palette and apply adjustments.
	UpdateAdjustedPaletteGUI(m_Ember.m_Palette);//Show the adjusted palette.
}

/// <summary>
/// Set the selected palette as the current one,
/// applying any adjustments previously specified.
/// Called when a palette cell is clicked. Unfortunately,
/// this will get called twice on a double click when moving
/// from one palette to another. It happens quickly so it shouldn't
/// be too much of a problem.
/// Resets the rendering process.
/// </summary>
/// <param name="row">The table row clicked</param>
/// <param name="col">The table column clicked</param>
template <typename T>
void FractoriumEmberController<T>::PaletteCellClicked(int row, int col)
{
	if (const auto palette = m_PaletteList->GetPaletteByFilename(m_CurrentPaletteFilePath, row))
		SetBasePaletteAndAdjust(*palette);
}

/// <summary>
/// Map the palette in the clicked row index to the index
/// in the palette list, then pass that index to PaletteCellClicked().
/// This resolves the case where the sort order of the palette table
/// is different than the internal order of the palette list.
/// </summary>
/// <param name="row">The table row clicked</param>
/// <param name="col">The table column clicked, ignored</param>
void Fractorium::OnPaletteCellClicked(int row, int col)
{
	if (const auto item = dynamic_cast<PaletteTableWidgetItem*>(ui.PaletteListTable->item(row, 1)))
	{
		const auto index = int(item->Index());

		if (m_PreviousPaletteRow != index)
		{
			m_Controller->PaletteCellClicked(index, col);
			m_PreviousPaletteRow = index;//Save for comparison on next click.
		}
	}
}

/// <summary>
/// Called when the mouse has been moved while pressed on the palette preview table.
/// Computes the difference between where the mouse was clicked and where it is now, then
/// uses that difference as a rotation value to pass into the palette adjustment.
/// Updates the palette and resets the rendering process.
/// </summary>
/// <param name="local">The local mouse coordinates relative to the palette preview table</param>
/// <param name="global">The global mouse coordinates</param>
void Fractorium::OnPreviewPaletteMouseDragged(const QPointF& local, const QPointF& global)
{
	if (m_PreviewPaletteMouseDown)
	{
		m_PreviewPaletteRotation = m_PreviewPaletteMouseDownRotation + (global.x() - m_PreviewPaletteMouseDownPosition.x());
		//qDebug() << "Palette preview table drag reached main window event: " << local.x() << ' ' << local.y() << ", global: " << global.x() << ' ' << global.y() << ", final: " << m_PreviewPaletteRotation;
		m_Controller->PaletteAdjust();
	}
}

/// <summary>
/// Called when the mouse has been released over the palette preview table.
/// Does nothing but set the dragging state to false.
/// </summary>
void Fractorium::OnPreviewPaletteMouseReleased()
{
	m_PreviewPaletteMouseDown = false;
}

/// <summary>
/// Sets the palette rotation to zero.
/// Updates the palette and resets the rendering process.
/// </summary>
/// <param name="row">Ignored</param>
/// <param name="col">Ignored</param>
void Fractorium::OnPreviewPaletteCellDoubleClicked(int row, int col)
{
	m_PreviewPaletteRotation = m_PreviewPaletteMouseDownRotation = 0;
	m_PreviewPaletteMouseDown = false;
	m_Controller->PaletteAdjust();
}

/// <summary>
/// Called when the mouse has been pressed on the palette preview table.
/// Subsequent mouse movements will compute a rotation value to pass into the palette adjustment, based on the location
/// of the mouse when this slot is called.
/// </summary>
/// <param name="row">Ignored</param>
/// <param name="col">Ignored</param>
void Fractorium::OnPreviewPaletteCellPressed(int row, int col)
{
	m_PreviewPaletteMouseDown = true;
	m_PreviewPaletteMouseDownPosition = QCursor::pos();//Get the global mouse position.
	m_PreviewPaletteMouseDownRotation = m_PreviewPaletteRotation;
	//qDebug() << "Mouse down with initial pos: " << m_PreviewPaletteMouseDownPosition.x() << " and initial rotation: " << m_PreviewPaletteMouseDownRotation;
}

/// <summary>
/// Set the selected palette as the current one,
/// resetting any adjustments previously specified.
/// Called when a palette cell is double clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="row">The table row clicked</param>
/// <param name="col">The table column clicked</param>
void Fractorium::OnPaletteCellDoubleClicked(int row, int col)
{
	ResetPaletteControls();
	m_PreviousPaletteRow = -1;
	OnPaletteCellClicked(row, col);
}

/// <summary>
/// Set the selected palette to a randomly selected one,
/// applying any adjustments previously specified if the checked parameter is true.
/// Called when the Random Palette button is clicked.
/// Resets the rendering process.
/// </summary>
/// <param name="checked">True to clear the current adjustments, else leave current adjustments and apply them to the newly selected palette.</param>
void Fractorium::OnPaletteRandomSelectButtonClicked(bool checked)
{
	uint i = 0;
	const auto rowCount = ui.PaletteListTable->rowCount();

	if (rowCount > 1)//If only one palette in the current palette file, just use it.
		while (((i = QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(rowCount)) == uint(m_PreviousPaletteRow)) || i >= static_cast<uint>(rowCount));

	if (checked)
		OnPaletteCellDoubleClicked(i, 1);//Will clear the adjustments.
	else
		OnPaletteCellClicked(i, 1);
}

/// <summary>
/// Apply random adjustments to the selected palette.
/// Called when the Random Adjustment button is clicked.
/// Resets the rendering process.
/// </summary>
void Fractorium::OnPaletteRandomAdjustButtonClicked(bool checked)
{
	m_PaletteHueSpin->setValue(-180 + QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(361));
	m_PaletteSaturationSpin->setValue(-50 + QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(101));//Full range of these leads to bad palettes, so clamp range.
	m_PaletteBrightnessSpin->setValue(-50 + QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(101));
	m_PaletteContrastSpin->setValue(-50 + QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(101));

	//Doing frequency and blur together gives bad palettes that are just a solid color.
	if (QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRandBit())
	{
		m_PaletteBlurSpin->setValue(QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(21));
		m_PaletteFrequencySpin->setValue(1);
	}
	else
	{
		m_PaletteBlurSpin->setValue(0);
		m_PaletteFrequencySpin->setValue(1 + QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(10));
	}

	OnPaletteAdjust(0);
}

/// <summary>
/// Open the palette editor dialog.
/// Called when the palette editor button is clicked.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::PaletteEditorButtonClicked()
{
	size_t i = 0;
	const auto ed = m_Fractorium->m_PaletteEditor.get();
	map<size_t, float> colorIndices;
	const auto forceFinal = m_Fractorium->HaveFinal();
	m_PreviousTempPalette = m_TempPalette; // it's necessary because m_TempPalette is changed when the user make changes in palette editor
	ed->SetPalette(m_TempPalette);

	while (auto xform = m_Ember.GetTotalXform(i, forceFinal))
		colorIndices[i++] = xform->m_ColorX;

	ed->SetColorIndices(colorIndices);
	ed->SetPreviousColorIndices(colorIndices); // also necessary because the colors are changed in palette editor
	ed->SetPaletteFile(m_CurrentPaletteFilePath);
#ifdef __linux__
	ed->show();
#else
	SyncPalette(ed->exec() == QDialog::Accepted);
#endif
}

/// <summary>
/// Slot called when the palette editor changes the palette and the Sync checkbox is checked.
/// </summary>
bool Fractorium::PaletteChanged()
{
	return m_PaletteChanged;
}

/// <summary>
/// Open the palette editor dialog.
/// This creates the palette editor dialog if it has not been created at least once.
/// Called when the palette editor button is clicked.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnPaletteEditorButtonClicked(bool checked)
{
	if (!m_PaletteEditor.get())
	{
		m_PaletteEditor = std::make_unique<PaletteEditor>(this);
		connect(m_PaletteEditor.get(), SIGNAL(PaletteChanged()),                 this, SLOT(OnPaletteEditorColorChanged()), Qt::ConnectionType::QueuedConnection);
		connect(m_PaletteEditor.get(), SIGNAL(PaletteFileChanged()),             this, SLOT(OnPaletteEditorFileChanged()), Qt::ConnectionType::QueuedConnection);
		connect(m_PaletteEditor.get(), SIGNAL(ColorIndexChanged(size_t, float)), this, SLOT(OnPaletteEditorColorIndexChanged(size_t, float)), Qt::ConnectionType::QueuedConnection);
#ifdef __linux__
		connect(m_PaletteEditor.get(), SIGNAL(finished(int)),                    this, SLOT(OnPaletteEditorFinished(int)), Qt::ConnectionType::QueuedConnection);
#endif
	}

	m_PaletteChanged = false;
	m_PaletteFileChanged = false;
	m_Controller->PaletteEditorButtonClicked();
}

/// <summary>
/// Slot called when palette editor window is closed.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SyncPalette(bool accepted)
{
	const auto ed = m_Fractorium->m_PaletteEditor.get();
	Palette<float> edPal;
	Palette<float> prevPal = m_PreviousTempPalette;
	map<size_t, float> colorIndices;
	const auto forceFinal = m_Fractorium->HaveFinal();

	if (accepted)
	{
		//Copy all just to be safe, because they may or may not have synced.
		colorIndices = ed->GetColorIndices();

		for (auto& index : colorIndices)
			if (auto xform = m_Ember.GetTotalXform(index.first, forceFinal))
				xform->m_ColorX = index.second;

		edPal = ed->GetPalette(static_cast<int>(prevPal.Size()));
		SetBasePaletteAndAdjust(edPal);//This will take care of updating the color index controls.

		if (edPal.m_Filename.get() && !edPal.m_Filename->empty())
			m_Fractorium->SetPaletteFileComboIndex(*edPal.m_Filename);
	}
	else if (m_Fractorium->PaletteChanged())//They clicked cancel, but synced at least once, restore the previous palette.
	{
		colorIndices = ed->GetPreviousColorIndices();

		for (auto& index : colorIndices)
			if (auto xform = m_Ember.GetTotalXform(index.first, forceFinal))
				xform->m_ColorX = index.second;

		SetBasePaletteAndAdjust(prevPal);//This will take care of updating the color index controls.
	}

	//Whether the current palette file was changed or not, if it's modifiable then reload it just to be safe (even though it might be overkill).
	if (m_PaletteList->IsModifiable(m_CurrentPaletteFilePath))
		m_Fractorium->OnPaletteFilenameComboChanged(QString::fromStdString(m_CurrentPaletteFilePath));
}

/// <summary>
/// Slot called every time a color is changed in the palette editor.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::PaletteEditorColorChanged()
{
	SetBasePaletteAndAdjust(m_Fractorium->m_PaletteEditor->GetPalette(static_cast<int>(m_TempPalette.Size())));
}

void Fractorium::OnPaletteEditorColorChanged()
{
	m_PaletteChanged = true;
	m_Controller->PaletteEditorColorChanged();
}

/// <summary>
/// Slot called every time a palette file is changed in the palette editor.
/// </summary>
void Fractorium::OnPaletteEditorFileChanged()
{
	m_PaletteFileChanged = true;
}

/// <summary>
/// Slot called every time an xform color index is changed in the palette editor.
/// If a special value of size_t max is passed for index, it means update all color indices.
/// </summary>
/// <param name="index">The index of the xform whose color index has been changed. Special value of size_t max to update all</param>
/// <param name="value">The value of the color index</param>
void Fractorium::OnPaletteEditorColorIndexChanged(size_t index, float value)
{
	if (index == std::numeric_limits<size_t>::max())//Update all in this case.
	{
		auto indices = m_PaletteEditor->GetColorIndices();

		for (auto& it : indices)
			OnXformColorIndexChanged(it.second, true, true, true, eXformUpdate::UPDATE_SPECIFIC, it.first);
	}
	else//Only update the xform index that was selected and dragged inside of the palette editor.
		OnXformColorIndexChanged(value, true, true, true, eXformUpdate::UPDATE_SPECIFIC, index);
}

/// Slot called after EditPallete is closed.
/// </summary>
/// <param name="result">Cancel/OK action</param>
void Fractorium::OnPaletteEditorFinished(int result)
{
	m_Controller->SyncPalette(result == QDialog::Accepted);
}

/// <summary>
/// Apply the text in the palette filter text box to only show palettes whose names
/// contain the substring.
/// Called when the user types in the palette filter text box.
/// </summary>
/// <param name="text">The text to filter on</param>
void Fractorium::OnPaletteFilterLineEditTextChanged(const QString& text)
{
	auto table = ui.PaletteListTable;
	table->setUpdatesEnabled(false);

	for (int i = 0; i < table->rowCount(); i++)
	{
		if (auto item = table->item(i, 0))
		{
			if (!item->text().contains(text, Qt::CaseSensitivity::CaseInsensitive))
				table->hideRow(i);
			else
				table->showRow(i);
		}
	}

	ui.PaletteListTable->sortItems(0, m_PaletteSortMode == 0 ? Qt::SortOrder::AscendingOrder : Qt::SortOrder::DescendingOrder);//Must re-sort every time the filter changes.
	table->setUpdatesEnabled(true);
}

/// <summary>
/// Clear the palette name filter, which will display all palettes.
/// Called when clear palette filter button is clicked.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnPaletteFilterClearButtonClicked(bool checked)
{
	ui.PaletteFilterLineEdit->clear();
}

/// <summary>
/// Change the sorting to be either ascending or descending.
/// Called when user clicks the table headers.
/// </summary>
/// <param name="col">Column index of the header clicked, ignored.</param>
void Fractorium::OnPaletteHeaderSectionClicked(int col)
{
	m_PaletteSortMode = (m_PaletteSortMode + 1) & 1;
	ui.PaletteListTable->sortItems(0, m_PaletteSortMode == 0 ? Qt::SortOrder::AscendingOrder : Qt::SortOrder::DescendingOrder);
}

/// <summary>
/// Reset the palette controls.
/// Usually in response to a palette cell double click.
/// </summary>
void Fractorium::ResetPaletteControls()
{
	m_PreviewPaletteRotation = m_PreviewPaletteMouseDownRotation = 0;
	m_PaletteHueSpin->SetValueStealth(0);
	m_PaletteSaturationSpin->SetValueStealth(0);
	m_PaletteBrightnessSpin->SetValueStealth(0);
	m_PaletteContrastSpin->SetValueStealth(0);
	m_PaletteBlurSpin->SetValueStealth(0);
	m_PaletteFrequencySpin->SetValueStealth(1);
}

/// <summary>
/// Set the index of the palette file combo box.
/// This is for display purposes only so the user can see which file, if any,
/// the current palette came from.
/// For embedded palettes with no filename, this will have no effect.
/// </summary>
/// <param name="filename">The string to set the index to</param>
void Fractorium::SetPaletteFileComboIndex(const string& filename)
{
	if (!filename.empty())
		ui.PaletteFilenameCombo->setCurrentText(QFileInfo(QString::fromStdString(filename)).fileName());
}

/// <summary>
/// Reset the color curve values for the selected curve in the current ember to their default state and also update the curves control.
/// Called when ResetCurvesButton is clicked.
/// Note if they click Reset Curves when the ctrl is pressed, then it clears all curves.
/// Resets the rendering process at either ACCUM_ONLY by default, or FILTER_AND_ACCUM when using early clip.
/// </summary>
/// <param name="i">The index of the curve to be cleared, 0 to clear all.</param>
template <typename T>
void FractoriumEmberController<T>::ClearColorCurves(int i)
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		if (i < 0)
			ember.m_Curves.Init();
		else
			ember.m_Curves.Init(i);
	}, true, m_Renderer->EarlyClip() ? eProcessAction::FILTER_AND_ACCUM : eProcessAction::ACCUM_ONLY, m_Fractorium->ApplyAll());
	FillCurvesControl();
}

void Fractorium::OnResetCurvesButtonClicked(bool checked)
{
	if (!QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier))
	{
		if (ui.CurvesAllRadio->isChecked())
			m_Controller->ClearColorCurves(0);
		else if (ui.CurvesRedRadio->isChecked())
			m_Controller->ClearColorCurves(1);
		else if (ui.CurvesGreenRadio->isChecked())
			m_Controller->ClearColorCurves(2);
		else if (ui.CurvesBlueRadio->isChecked())
			m_Controller->ClearColorCurves(3);
		else
			m_Controller->ClearColorCurves(0);
	}
	else
	{
		m_Controller->ClearColorCurves(-1);
	}
}

/// <summary>
/// Set the coordinate of the curve point.
/// Called when the position of any of the points in the curves editor is is changed.
/// Resets the rendering process at either ACCUM_ONLY by default, or FILTER_AND_ACCUM when using early clip.
/// </summary>
/// <param name="curveIndex">The curve index, 0-3/</param>
/// <param name="pointIndex">The point index within the selected curve, 1-2.</param>
/// <param name="point">The new coordinate of the point in terms of the curves control rect.</param>
template <typename T>
void FractoriumEmberController<T>::ColorCurveChanged(int curveIndex, int pointIndex, const QPointF& point)
{
	Update([&]
	{
		m_Ember.m_Curves.m_Points[curveIndex][pointIndex].x = point.x();
		m_Ember.m_Curves.m_Points[curveIndex][pointIndex].y = point.y();
	}, true, m_Renderer->EarlyClip() ? eProcessAction::FILTER_AND_ACCUM : eProcessAction::ACCUM_ONLY);
}

void Fractorium::OnCurvesPointChanged(int curveIndex, int pointIndex, const QPointF& point) { m_Controller->ColorCurveChanged(curveIndex, pointIndex, point); }

/// <summary>
/// Remove curve point.
/// Called when right clicking on a color curve point.
/// Resets the rendering process at either ACCUM_ONLY by default, or FILTER_AND_ACCUM when using early clip.
/// </summary>
/// <param name="curveIndex">The curve index./</param>
/// <param name="pointIndex">The point index within the selected curve.</param>
template <typename T>
void FractoriumEmberController<T>::ColorCurvesPointRemoved(size_t curveIndex, int pointIndex)
{
	Update([&]
	{
		if (m_Ember.m_Curves.m_Points[curveIndex].size() > 2)
		{
			m_Ember.m_Curves.m_Points[curveIndex].erase(m_Ember.m_Curves.m_Points[curveIndex].begin() + pointIndex);
			std::sort(m_Ember.m_Curves.m_Points[curveIndex].begin(), m_Ember.m_Curves.m_Points[curveIndex].end(), [&](auto & lhs, auto & rhs) { return lhs.x < rhs.x; });
		}
	}, true, m_Renderer->EarlyClip() ? eProcessAction::FILTER_AND_ACCUM : eProcessAction::ACCUM_ONLY);
	FillCurvesControl();
}

void Fractorium::OnCurvesPointRemoved(size_t curveIndex, int pointIndex) { m_Controller->ColorCurvesPointRemoved(curveIndex, pointIndex); }

/// <summary>
/// Add a curve point.
/// Called when clicking in between points on a color curve.
/// Resets the rendering process at either ACCUM_ONLY by default, or FILTER_AND_ACCUM when using early clip.
/// </summary>
/// <param name="curveIndex">The curve index./</param>
/// <param name="pointIndex">The point to add to the selected curve.</param>
template <typename T>
void FractoriumEmberController<T>::ColorCurvesPointAdded(size_t curveIndex, const QPointF& point)
{
	Update([&]
	{
		m_Ember.m_Curves.m_Points[curveIndex].push_back({ point.x(), point.y() });
		std::sort(m_Ember.m_Curves.m_Points[curveIndex].begin(), m_Ember.m_Curves.m_Points[curveIndex].end(), [&](auto & lhs, auto & rhs) { return lhs.x < rhs.x; });
	}, true, m_Renderer->EarlyClip() ? eProcessAction::FILTER_AND_ACCUM : eProcessAction::ACCUM_ONLY);
	FillCurvesControl();
}

void Fractorium::OnCurvesPointAdded(size_t curveIndex, const QPointF& point) { m_Controller->ColorCurvesPointAdded(curveIndex, point); }

/// <summary>
/// Set the top most points in the curves control, which makes it easier to
/// select a point by putting it on top of all the others.
/// Called when the any of the curve color radio buttons are toggled.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnCurvesAllRadioButtonToggled(bool checked)   { if (checked) ui.CurvesView->SetTop(CurveIndex::ALL); }
void Fractorium::OnCurvesRedRadioButtonToggled(bool checked)   { if (checked) ui.CurvesView->SetTop(CurveIndex::RED); }
void Fractorium::OnCurvesGreenRadioButtonToggled(bool checked) { if (checked) ui.CurvesView->SetTop(CurveIndex::GREEN); }
void Fractorium::OnCurvesBlueRadioButtonToggled(bool checked)  { if (checked) ui.CurvesView->SetTop(CurveIndex::BLUE); }

/// <summary>
/// Set the points in the curves control to the values of the curve points in the current ember.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::FillCurvesControl()
{
	m_Fractorium->ui.CurvesView->blockSignals(true);
	m_Fractorium->ui.CurvesView->Set(m_Ember.m_Curves);
	m_Fractorium->ui.CurvesView->blockSignals(false);
	m_Fractorium->ui.CurvesView->update();
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif