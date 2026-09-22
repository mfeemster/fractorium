#include "FractoriumPch.h"
#include "PaletteEditor.h"
#include "Fractorium.h"

/// <summary>
/// Constructor which passes parent widget to the base and sets up slots and other ui
/// elements.
/// </summary>
/// <param name="p">The parent widget</param>
PaletteEditor::PaletteEditor(QWidget* p) :
	QDialog(p),
	ui(make_unique<Ui::PaletteEditor>()),
	m_PaletteList(PaletteList<float>::Instance())
{
	ui->setupUi(this);
	m_ColorPicker = new ColorPickerWidget(this);
	m_ColorPicker->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	m_ColorPicker->SetColorPanelColor(Qt::black);
	QVBoxLayout* colorLayout = new QVBoxLayout();
	colorLayout->setContentsMargins(3, 3, 3, 3);
	colorLayout->addWidget(m_ColorPicker);
	ui->ColorPickerGroupBox->setLayout(colorLayout);
	ui->ColorPickerGroupBox->setContentsMargins(3, 8, 3, 3);
	m_GradientColorView = new GradientColorsView(ui->ColorViewGroupBox);
	connect(m_ColorPicker,                         SIGNAL(ColorChanged(const QColor&)),              this, SLOT(OnColorPickerColorChanged(const QColor&)));
	connect(m_GradientColorView,                   SIGNAL(ArrowMove(qreal, const GradientArrow&)),   this, SLOT(OnArrowMoved(qreal, const GradientArrow&)));
	connect(m_GradientColorView,                   SIGNAL(ArrowDoubleClicked(const GradientArrow&)), this, SLOT(OnArrowDoubleClicked(const GradientArrow&)));
	connect(m_GradientColorView,                   SIGNAL(ColorIndexMove(size_t, float)),            this, SLOT(OnColorIndexMove(size_t, float)));
	connect(ui->CreatePaletteFromImageButton,      SIGNAL(clicked()),                                this, SLOT(OnCreatePaletteFromImageButtonClicked()));
	connect(ui->CreatePaletteAgainFromImageButton, SIGNAL(clicked()),                                this, SLOT(OnCreatePaletteAgainFromImageButton()));
	connect(ui->AddColorButton,                    SIGNAL(clicked()),                                this, SLOT(OnAddColorButtonClicked()));
	connect(ui->RemoveColorButton,                 SIGNAL(clicked()),                                this, SLOT(OnRemoveColorButtonClicked()));
	connect(ui->InvertColorsButton,                SIGNAL(clicked()),                                this, SLOT(OnInvertColorsButtonClicked()));
	connect(ui->ResetColorsButton,                 SIGNAL(clicked()),                                this, SLOT(OnResetToDefaultButtonClicked()));
	connect(ui->RandomColorsButton,                SIGNAL(clicked()),                                this, SLOT(OnRandomColorsButtonClicked()));
	connect(ui->DistributeColorsButton,            SIGNAL(clicked()),                                this, SLOT(OnDistributeColorsButtonClicked()));
	connect(ui->SyncCheckBox,                      SIGNAL(stateChanged(int)),                        this, SLOT(OnSyncCheckBoxStateChanged(int)), Qt::QueuedConnection);
	connect(ui->BlendCheckBox,                     SIGNAL(stateChanged(int)),                        this, SLOT(OnBlendCheckBoxStateChanged(int)), Qt::QueuedConnection);
	connect(ui->PaletteFilenameCombo,              SIGNAL(currentTextChanged(const QString&)),       this, SLOT(OnPaletteFilenameComboChanged(const QString&)), Qt::QueuedConnection);
	connect(ui->PaletteListTable,                  SIGNAL(cellClicked(int, int)),                    this, SLOT(OnPaletteCellClicked(int, int)), Qt::QueuedConnection);
	connect(ui->PaletteListTable,                  SIGNAL(cellChanged(int, int)),                    this, SLOT(OnPaletteCellChanged(int, int)), Qt::QueuedConnection);
	connect(ui->NewPaletteFileButton,              SIGNAL(clicked()),                                this, SLOT(OnNewPaletteFileButtonClicked()));
	connect(ui->CopyPaletteFileButton,             SIGNAL(clicked()),                                this, SLOT(OnCopyPaletteFileButtonClicked()));
	connect(ui->AppendPaletteButton,               SIGNAL(clicked()),                                this, SLOT(OnAppendPaletteButtonClicked()));
	connect(ui->OverwritePaletteButton,            SIGNAL(clicked()),                                this, SLOT(OnOverwritePaletteButtonClicked()));
	connect(ui->DeletePaletteButton,               SIGNAL(clicked()),                                this, SLOT(OnDeletePaletteButtonClicked()));
	ui->PaletteListTable->horizontalHeader()->setSectionsClickable(true);
	auto layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->addWidget(m_GradientColorView);
	ui->ColorViewGroupBox->setLayout(layout);
	auto& pals = m_PaletteList->Palettes();

	for (auto& pal : pals)
	{
		QFileInfo info(QString::fromStdString(pal.first));
		ui->PaletteFilenameCombo->addItem(info.fileName());
	}

	ui->PaletteFilenameCombo->model()->sort(0);

	if (ui->PaletteFilenameCombo->count() > 0)
		m_CurrentPaletteFilePath = ui->PaletteFilenameCombo->itemText(0).toStdString();
}

/// <summary>
/// Get whether change events here are propagated back to the main window.
/// </summary>
/// <returns>bool</returns>
bool PaletteEditor::Sync()
{
	return ui->SyncCheckBox->isChecked();
}

/// <summary>
/// Populate and retrieve a reference to the palette from the underlying gradient color view
/// using the specified number of elements.
/// </summary>
/// <param name="size">The number of elements the palette will have</param>
/// <returns>A freference to the palette</returns>
Palette<float>& PaletteEditor::GetPalette(int size)
{
	return m_GradientColorView->GetPalette(size);
}

/// <summary>
/// Set the palette of the underlying gradient color view.
/// This can be a modifiable palette or a fixed one.
/// </summary>
/// <param name="palette">The palette to assign</param>
void PaletteEditor::SetPalette(const Palette<float>& palette)
{
	const auto combo = ui->PaletteFilenameCombo;
	m_PaletteIndex = std::numeric_limits<int>::max();
	m_GradientColorView->SetPalette(palette);
	auto& arrows = m_GradientColorView->GetArrows();

	if (!arrows.empty())
		m_ColorPicker->SetColorPanelColor(arrows.begin()->second.Color());//Will emit PaletteChanged() if color changed...

	if (palette.m_Filename.get())
	{
		QFileInfo info(QString::fromStdString(*palette.m_Filename.get()));
		combo->setCurrentIndex(combo->findData(info.fileName(), Qt::DisplayRole));
	}

	EnablePaletteControls();
	EmitPaletteChanged();//...So emit here just to be safe.
}

/// <summary>
/// Return a temporary copy of the xform color indices as a map.
/// The keys are the xform indices, and the values are the color indices.
/// </summary>
/// <returns>The color indices</returns>
map<size_t, float> PaletteEditor::GetColorIndices() const
{
	return m_GradientColorView->GetColorIndices();
}

/// <summary>
/// Return the previous xform color indices as a map.
/// The keys are the xform indices, and the values are the color indices.
/// </summary>
/// <returns>The color indices</returns>
map<size_t, float> PaletteEditor::GetPreviousColorIndices() const
{
	return m_PreviousColorIndices;
}

/// <summary>
/// Assign the values of the xform color indices to the arrows.
/// This will clear out any existing values first.
/// </summary>
/// <param name="indices">The color indices to assign</param>
void PaletteEditor::SetColorIndices(const map<size_t, float>& indices)
{
	m_GradientColorView->SetColorIndices(indices);
}

/// <summary>
/// Backup xform color
/// </summary>
/// <param name="indices">The color indices to backup</param>
void PaletteEditor::SetPreviousColorIndices(const map<size_t, float>& indices)
{
	m_PreviousColorIndices = indices;
}

/// <summary>
/// Return the filename of the currently selected palette.
/// Note this will only be filled in if the user has clicked in the palette
/// table at least once.
/// </summary>
/// <returns>The palette filename</returns>
string PaletteEditor::GetPaletteFile() const
{
	return m_CurrentPaletteFilePath;
}

/// <summary>
/// Set the selected palette file in the combo box.
/// </summary>
/// <param name="filename">The filename of the palette file to set to the current one</param>
void PaletteEditor::SetPaletteFile(const string& filename)
{
	ui->PaletteFilenameCombo->setCurrentText(QString::fromStdString(GetFilename(filename)));
}

/// <summary>
/// Add a new arrow using the current color.
/// Called when the Add Color button is clicked.
/// </summary>
void PaletteEditor::OnAddColorButtonClicked()
{
	AddArrow(m_ColorPicker->Color());
	EmitPaletteChanged();
}

/// <summary>
/// Delete the focused arrow and optionally redistribute the arrows.
/// Called when the Remove Color button is clicked.
/// </summary>
void PaletteEditor::OnRemoveColorButtonClicked()
{
	m_GradientColorView->DeleteFocusedArrow();

	if (ui->AutoDistributeCheckBox->isChecked())
		m_GradientColorView->DistributeColors();

	EmitPaletteChanged();
}

/// <summary>
/// Invert the colors of the arrows.
/// Called when the Invert Colors button is clicked.
/// </summary>
void PaletteEditor::OnInvertColorsButtonClicked()
{
	m_GradientColorView->InvertColors();
	EmitPaletteChanged();
}

/// <summary>
/// Randomize the colors of the arrows.
/// Called when the Random Colors button is clicked.
/// </summary>
void PaletteEditor::OnRandomColorsButtonClicked()
{
	m_GradientColorView->RandomColors();
	EmitPaletteChanged();
}

/// <summary>
/// Set the distance between each arrow to be equal.
/// Called when the Distribute Colors button is clicked.
/// </summary>
void PaletteEditor::OnDistributeColorsButtonClicked()
{
	m_GradientColorView->DistributeColors();
	EmitPaletteChanged();
}

/// <summary>
/// Delete all arrows and add a white arrow at index 0, and a black
/// arrow at index 1.
/// Called when the Reset button is clicked.
/// </summary>
void PaletteEditor::OnResetToDefaultButtonClicked()
{
	m_GradientColorView->ResetToDefault();
	EmitPaletteChanged();
}

/// <summary>
/// Create a palette by opening an image and selecting the colors from
/// pixels at random locations.
/// Ensure arrows spin box has a value of at least two.
/// Called when the From Image button is clicked.
/// </summary>
void PaletteEditor::OnCreatePaletteFromImageButtonClicked()
{
	const auto filenames = SetupOpenImagesDialog();

	if (!filenames.empty())
	{
		m_Filename = filenames[0];

		if (ui->ArrowsSpinBox->value() < 2)
			ui->ArrowsSpinBox->setValue(2);

		auto colors = GetRandomColorsFromImage(m_Filename, ui->ArrowsSpinBox->value());
		m_GradientColorView->SetArrows(colors);
		EmitPaletteChanged();
	}
}

/// <summary>
/// Create a palette by re-opening the previously selected image and selecting the colors from
/// pixels at random locations.
/// Ensure arrows spin box has a value of at least two.
/// Called when the From Image Again button is clicked.
/// </summary>
void PaletteEditor::OnCreatePaletteAgainFromImageButton()
{
	if (QFile::exists(m_Filename))
	{
		if (ui->ArrowsSpinBox->value() < 2)
			ui->ArrowsSpinBox->setValue(2);

		auto colors = GetRandomColorsFromImage(m_Filename, ui->ArrowsSpinBox->value());
		m_GradientColorView->SetArrows(colors);
		EmitPaletteChanged();
	}
}

/// <summary>
/// Set the focus color as a result of selecting a stock in the color picker.
/// Called when the color picker signals the ColorChanged event.
/// </summary>
void PaletteEditor::OnColorPickerColorChanged(const QColor& col)
{
	m_GradientColorView->SetFocusColor(col);

	if (m_GradientColorView->ArrowCount())
		EmitPaletteChanged();
}

/// <summary>
/// Set the color panel color as a result of double clicking an arrow.
/// Called when the color view signals the ArrowDoubleClicked event.
/// </summary>
/// <param name="arrow">The arrow which was double clicked on</param>
void PaletteEditor::OnArrowDoubleClicked(const GradientArrow& arrow)
{
	blockSignals(true);//Do not update main window when Sync is checked because selecting an arrow as the main color doesn't actually change anything.
	m_ColorPicker->SetColorPanelColor(arrow.Color());
	blockSignals(false);
}

/// <summary>
/// Change whether palette changes are synced with the main window.
/// Called when the Sync checkbox is checked/unchecked.
/// </summary>
/// <param name="state">Ignored</param>
void PaletteEditor::OnSyncCheckBoxStateChanged(int state)
{
	EmitPaletteChanged();
	EmitColorIndexChanged(std::numeric_limits<size_t>::max(), 0);//Pass special value to update all.
}

/// <summary>
/// Change whether palette colors are blended between points, or instead do hard cuts.
/// Called when the Blend checkbox is checked/unchecked.
/// </summary>
/// <param name="state">Ignored</param>
void PaletteEditor::OnBlendCheckBoxStateChanged(int state)
{
	m_GradientColorView->Blend(static_cast<bool>(state));
	m_GradientColorView->update();
	EmitPaletteChanged();
}

/// <summary>
/// Load the palette file based on the currently selected index in the combo box.
/// Called when the index of the palette filename combo box changes.
/// </summary>
/// <param name="text">The text of the combo box, which is just the palette filename without the path.</param>
void PaletteEditor::OnPaletteFilenameComboChanged(const QString& text)
{
	if (!text.isEmpty())//This occasionally seems to get called with an empty string for reasons unknown.
	{
		const auto paletteTable = ui->PaletteListTable;
		m_CurrentPaletteFilePath = text.toStdString();
		::FillPaletteTable(text.toStdString(), paletteTable, m_PaletteList);
		const auto fullname = m_PaletteList->GetFullPathFromFilename(m_CurrentPaletteFilePath);
		ui->PaletteFilenameCombo->setToolTip(QString::fromStdString(fullname));
		EnablePaletteFileControls();
	}
}

/// <summary>
/// Load the palette into the editor controls.
/// Called when the second column in a row in the palette list is clicked.
/// </summary>
/// <param name="row">The table row clicked</param>
/// <param name="col">The table column clicked</param>
void PaletteEditor::OnPaletteCellClicked(int row, int col)
{
	if (col == 1)
	{
		if (const auto palette = m_PaletteList->GetPaletteByFilename(m_CurrentPaletteFilePath, row))
		{
			SetPalette(*palette);
			m_PaletteIndex = row;
		}
	}
}

/// <summary>
/// Update the name of the palette.
/// Called when the first column in a row in the palette list is clicked and edited.
/// </summary>
/// <param name="row">The table row clicked</param>
/// <param name="col">The table column clicked</param>
void PaletteEditor::OnPaletteCellChanged(int row, int col)
{
	if (col == 0)
	{
		if (const auto palette = m_PaletteList->GetPaletteByFilename(m_CurrentPaletteFilePath, row))
		{
			if (!palette->m_SourceColors.empty())
			{
				palette->m_Name = ui->PaletteListTable->item(row, col)->text().toStdString();
				emit PaletteFileChanged();
			}
		}
	}
}

/// <summary>
/// Create a new palette file.
/// The newly created file will have a unique name.
/// Called when the new palette file button is clicked.
/// </summary>
void PaletteEditor::OnNewPaletteFileButtonClicked()
{
	const auto filename = EmberFile<float>::UniqueFilename(GetDefaultUserPath() + "/user-palettes.xml");

	if (m_PaletteList->AddEmptyPaletteFile(filename.toStdString()))
	{
		QFileInfo info(filename);
		ui->PaletteFilenameCombo->addItem(info.fileName());
		ui->PaletteFilenameCombo->setCurrentIndex(ui->PaletteFilenameCombo->count() - 1);
	}
}

/// <summary>
/// Copy the current palette file, add it to the combo box and load the new palette file.
/// The newly created file will have a unique name.
/// Called when the copy palette file button is clicked.
/// </summary>
void PaletteEditor::OnCopyPaletteFileButtonClicked()
{
	const auto qscurr = QString::fromStdString(m_CurrentPaletteFilePath);
	const auto qfilename = EmberFile<float>::UniqueFilename(GetDefaultUserPath() + "/" + qscurr);
	const auto filename = qfilename.toStdString();

	if (m_PaletteList->GetPaletteListByFullPath(filename) == nullptr)//Ensure the new filename does not exist in the map.
	{
		//Get the list of palettes for the current filename, this will be added as a copy below.
		if (const auto currentPaletteFile = m_PaletteList->GetPaletteListByFilename(m_CurrentPaletteFilePath))//Ensure the list of palettes was properly retrieved.
		{
			if (m_PaletteList->AddPaletteFile(filename, *currentPaletteFile))//Add the current vector of palettes to an entry with the new filename.
			{
				const QFileInfo info(qfilename);
				ui->PaletteFilenameCombo->addItem(info.fileName());
				ui->PaletteFilenameCombo->setCurrentIndex(ui->PaletteFilenameCombo->count() - 1);
			}
			else
				QMessageBox::critical(this, "Copy palette file error", "Failed copy palette to " + qfilename + ", because already exists in memory, but not on disk. Did you delete it while the program was running?");
		}
		else
			QMessageBox::critical(this, "Copy palette file error", "The current file " + qscurr + " did not exist. Did you delete it while the program was running?");
	}
	else
		QMessageBox::critical(this, "Copy palette file error", "Failed copy palette to " + qfilename + ", because it likely already exists");
}

/// <summary>
/// Copy the current palette to the end of the palette file.
/// Called when the append palette button is clicked.
/// </summary>
void PaletteEditor::OnAppendPaletteButtonClicked()
{
	const auto& pal = GetPalette(256);
	m_PaletteList->AddPaletteToFile(m_CurrentPaletteFilePath, pal);
	::FillPaletteTable(m_CurrentPaletteFilePath, ui->PaletteListTable, m_PaletteList);
	m_PaletteIndex = ui->PaletteListTable->rowCount() - 1;
	emit PaletteFileChanged();
}

/// <summary>
/// Overwrite the current palette in the palette file.
/// Called when the overwrite palette button is clicked.
/// </summary>
void PaletteEditor::OnOverwritePaletteButtonClicked()
{
	const auto& pal = GetPalette(256);
	m_PaletteList->Replace(m_CurrentPaletteFilePath, pal, m_PaletteIndex);
	::FillPaletteTable(m_CurrentPaletteFilePath, ui->PaletteListTable, m_PaletteList);
	emit PaletteFileChanged();
}

/// <summary>
/// Delete the current palette from the palette file.
/// Called when the delete palette button is clicked.
/// Note that the palette will not be deleted if it's the only palette in the file.
/// </summary>
void PaletteEditor::OnDeletePaletteButtonClicked()
{
	const auto table = ui->PaletteListTable;

	if (table->rowCount() > 1)
	{
		m_PaletteList->Delete(m_CurrentPaletteFilePath, m_PaletteIndex);
		::FillPaletteTable(m_CurrentPaletteFilePath, table, m_PaletteList);
		emit PaletteFileChanged();
		OnPaletteCellClicked(table->rowCount() - 1, 1);
	}
}

/// <summary>
/// Emit a palette changed event.
/// Called when an arrow is moved.
/// </summary>
void PaletteEditor::OnArrowMoved(qreal, const GradientArrow&)
{
	EmitPaletteChanged();
}

/// <summary>
/// Emit an xform color index changed event.
/// Called when one of the top arrows are moved.
/// </summary>
/// <param name="index">The index of the xform whose color index has been changed. Special value of size_t max to update all</param>
/// <param name="value">The value of the color index</param>
void PaletteEditor::OnColorIndexMove(size_t index, float value)
{
	EmitColorIndexChanged(index, value);
}

/// <summary>
/// Emit a palette changed event if the sync checkbox is checked.
/// </summary>
void PaletteEditor::EmitPaletteChanged()
{
	if (ui->SyncCheckBox->isChecked())
		emit PaletteChanged();
}

/// <summary>
/// Emit an xform color index changed event if the sync checkbox is checked.
/// </summary>
/// <param name="index">The index of the xform whose color index has been changed. Special value of size_t max to update all</param>
/// <param name="value">The value of the color index</param>
void PaletteEditor::EmitColorIndexChanged(size_t index, float value)
{
	if (ui->SyncCheckBox->isChecked())
		emit ColorIndexChanged(index, value);
}

/// <summary>
/// Helper to lazily instantiate an open file dialog.
/// Once created, it will remain alive for the duration of the program run.
/// </summary>
/// <returns>The list of filenames selected</returns>
QStringList PaletteEditor::SetupOpenImagesDialog()
{
	QStringList filenames;
	const auto settings = FractoriumSettings::Instance();
#ifndef __APPLE__

	if (!m_FileDialog)
	{
		m_FileDialog = new QFileDialog(this);
		m_FileDialog->setViewMode(QFileDialog::List);
		m_FileDialog->setFileMode(QFileDialog::ExistingFile);
		m_FileDialog->setAcceptMode(QFileDialog::AcceptOpen);
		m_FileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
#ifdef _WIN32
		m_FileDialog->setNameFilter("Image Files (*.png *.jpg *.bmp)");
#else
		m_FileDialog->setNameFilter("Image Files ( *.jpg *.png)");
#endif
		m_FileDialog->setWindowTitle("Open Image");
		m_FileDialog->setDirectory(settings->OpenPaletteImageFolder());
		m_FileDialog->selectNameFilter("*.jpg");
		m_FileDialog->setSidebarUrls(dynamic_cast<Fractorium*>(parent())->Urls());
	}

	if (m_FileDialog->exec() == QDialog::Accepted)
	{
		filenames = m_FileDialog->selectedFiles();

		if (!filenames.empty())
		{
			const auto path = QFileInfo(filenames[0]).canonicalPath();
			m_FileDialog->setDirectory(path);
			settings->OpenPaletteImageFolder(path);
		}
	}

#else
	const auto filename = QFileDialog::getOpenFileName(this, tr("Open Image"), settings->OpenPaletteImageFolder(), tr("Image Files (*.jpg *.png)"));

	if (filename.size() > 0)
	{
		filenames.append(filename);
		const auto path = QFileInfo(filenames[0]).canonicalPath();
		settings->OpenPaletteImageFolder(path);
	}

#endif
	return filenames;
}

/// <summary>
/// Add an arrow whose color will be assigned the passed in color.
/// Optionally distribute colors and emit a palette changed event.
/// </summary>
/// <param name="color">The color to assign to the new arrow</param>
void PaletteEditor::AddArrow(const QColor& color)
{
	const auto count = std::min<int>(std::abs(256 - m_GradientColorView->ArrowCount()), ui->ArrowsSpinBox->value());

	for (auto i = 0; i < count; i++)
	{
		m_GradientColorView->AddArrow(color);

		if (ui->AutoDistributeCheckBox->isChecked())
			m_GradientColorView->DistributeColors();
	}
}

/// <summary>
/// Helper to get the colors of random pixels within an image.
/// </summary>
/// <param name="filename">The full path to the image file to get random colors from</param>
/// <param name="numPoints">The number of colors to get</param>
/// <returns>A map whose keys are the color indices from 0-1, and whose values are the gradient arrows containing the color for each key position.</returns>
map<float, GradientArrow> PaletteEditor::GetRandomColorsFromImage(QString filename, int numPoints)
{
	map<float, GradientArrow> colors;
	const QImage image(filename);
	const qreal gSize = 512;
	float off = 0.0f, inc = 1.0f / std::max(1, numPoints - 1);
	QLinearGradient grad(QPoint(0, 0), QPoint(gSize, 1));

	for (auto i = 0; i < numPoints; i++)
	{
		const auto x = QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(image.width());
		const auto y = QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(image.height());
		const auto rgb = image.pixel(x, y);
		GradientArrow arrow;
		arrow.Color(QColor::fromRgb(rgb));
		arrow.Focus(i == 0);
		colors[off] = arrow;
		off += inc;
	}

	return colors;
}

/// <summary>
/// Enable/disable controls related to switching between modifiable and fixed palette files.
/// </summary>
void PaletteEditor::EnablePaletteFileControls()
{
	const auto b = IsCurrentPaletteAndFileEditable();//Both the file and the current palette must be editable.
	ui->DeletePaletteButton->setEnabled(b);
	ui->CopyPaletteFileButton->setEnabled(b);
	ui->AppendPaletteButton->setEnabled(b);
	ui->OverwritePaletteButton->setEnabled(b);
}

/// <summary>
/// Enable/disable controls related to switching between modifiable and fixed palettes.
/// </summary>
void PaletteEditor::EnablePaletteControls()
{
	const auto b = IsCurrentPaletteAndFileEditable();//Both the file and the current palette must be editable.
	const auto& pal = GetPalette(256);
	ui->DeletePaletteButton->setEnabled(b);
	ui->CopyPaletteFileButton->setEnabled(b);
	ui->AppendPaletteButton->setEnabled(b);
	ui->OverwritePaletteButton->setEnabled(b && pal.m_Filename.get() && (GetFilename(m_CurrentPaletteFilePath) == GetFilename(*pal.m_Filename.get())));//Only allow overwrite if the palette is from the file it's overwriting a palette in.
	ui->AddColorButton->setEnabled(b);
	ui->DistributeColorsButton->setEnabled(b);
	ui->AutoDistributeCheckBox->setEnabled(b);
	ui->BlendCheckBox->setEnabled(b);
	ui->RandomColorsButton->setEnabled(b);
	ui->RemoveColorButton->setEnabled(b);
	ui->ResetColorsButton->setEnabled(b);
	ui->ArrowsSpinBox->setEnabled(b);
	ui->CreatePaletteFromImageButton->setEnabled(b);
	ui->CreatePaletteAgainFromImageButton->setEnabled(b);
}

/// <summary>
/// Determine whether the current file and the palette selected within it are both editable.
/// </summary>
/// <returns>True if both the currently selected palette is editable and if all palettes in the currently selected file are editable.</returns>
bool PaletteEditor::IsCurrentPaletteAndFileEditable()
{
	return m_PaletteList->IsModifiable(m_CurrentPaletteFilePath) && !GetPalette(256).m_SourceColors.empty();
}
