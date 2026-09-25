#pragma once

#include "FractoriumPch.h"
#include "Fractorium.h"
#include "ColorPickerWidget.h"
#include "GradientColorsView.h"
#include "EmberFile.h"
#include "ui_PaletteEditor.h"

namespace Ui
{
class PaletteEditor;
}

/// <summary>
/// Dialog for editing user created palettes.
/// This will load with all available user created palettes populated in the combo
/// box. As the user changes the selected index, the palettes for that file
/// are shown in the list box. The user can click on those and then edit them and either
/// save it back to the same position in the file, or append it to the end.
/// They can also click in the name column to set/rename the palette name.
/// Any changes on this dialog can be "synced". That is, when the Sync checkbox is checked
/// any changes result in a signal being sent back to the main window.
/// </summary>
class PaletteEditor : public QDialog
{
	Q_OBJECT

public:
	explicit PaletteEditor(QWidget* p = nullptr);

public:
	bool Sync();
	Palette<float>& GetPalette(int size);
	void SetPalette(const Palette<float>& palette);
	map<size_t, float> GetColorIndices() const;
	map<size_t, float> GetPreviousColorIndices() const;
	void SetColorIndices(const map<size_t, float>& indices);
	void SetPreviousColorIndices(const map<size_t, float>& indices);
	string GetPaletteFile() const;
	void SetPaletteFile(const string& filename);

Q_SIGNALS:
	void PaletteChanged();
	void PaletteFileChanged();
	void ColorIndexChanged(size_t index, float value);

private Q_SLOTS:
	void OnAddColorButtonClicked();
	void OnRemoveColorButtonClicked();
	void OnInvertColorsButtonClicked();
	void OnRandomColorsButtonClicked();
	void OnDistributeColorsButtonClicked();
	void OnResetToDefaultButtonClicked();
	void OnCreatePaletteFromImageButtonClicked();
	void OnCreatePaletteAgainFromImageButton();
	void OnColorPickerColorChanged(const QColor& col);
	void OnArrowDoubleClicked(const GradientArrow& arrow);
	void OnSyncCheckBoxStateChanged(Qt::CheckState state);
	void OnBlendCheckBoxStateChanged(Qt::CheckState state);
	void OnArrowMoved(qreal lastPos, const GradientArrow& arrow);
	void OnColorIndexMove(size_t index, float value);
	void OnPaletteFilenameComboChanged(const QString& text);
	void OnPaletteCellClicked(int row, int col);
	void OnPaletteCellChanged(int row, int col);
	void OnNewPaletteFileButtonClicked();
	void OnCopyPaletteFileButtonClicked();
	void OnAppendPaletteButtonClicked();
	void OnOverwritePaletteButtonClicked();
	void OnDeletePaletteButtonClicked();

private:
	void EmitPaletteChanged();
	void EmitColorIndexChanged(size_t index, float value);
	QStringList SetupOpenImagesDialog();
	void AddArrow(const QColor& color);
	map<float, GradientArrow> GetRandomColorsFromImage(QString filename, int numPoints);
	void EnablePaletteFileControls();
	void EnablePaletteControls();
	bool IsCurrentPaletteAndFileEditable();
	bool m_PaletteFileChanged = false;
	int m_PaletteIndex = 0;
	map<size_t, float> m_PreviousColorIndices;
	QString m_Filename;
	string m_CurrentPaletteFilePath;
	ColorPickerWidget* m_ColorPicker = nullptr;
	GradientColorsView* m_GradientColorView = nullptr;
#ifndef __APPLE__
	QFileDialog* m_FileDialog = nullptr;
#endif
	shared_ptr<PaletteList<float>> m_PaletteList;
	std::unique_ptr<Ui::PaletteEditor> ui;
};
