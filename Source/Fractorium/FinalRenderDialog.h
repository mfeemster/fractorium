#pragma once

#include "ui_FinalRenderDialog.h"
#include "SpinBox.h"
#include "DoubleSpinBox.h"
#include "TwoButtonComboWidget.h"
#include "FractoriumSettings.h"
#include "FinalRenderEmberController.h"

/// <summary>
/// FractoriumFinalRenderDialog class.
/// </summary>

class Fractorium;//Forward declaration since Fractorium uses this dialog.
class FinalRenderEmberControllerBase;
template <typename T> class FinalRenderEmberController;

/// <summary>
/// The final render dialog is for when the user is satisfied with the parameters they've
/// set and they want to do a final render at a higher quality and at a specific resolution
/// and save it out to a file.
/// It supports rendering either the current ember, or all of them in the opened file.
/// If a single ember is rendered, it will be saved to a single output file.
/// If multiple embers are rendered, they will all be saved to a specified directory using
/// default names.
/// The user can optionally save the Xml file with each ember.
/// They can be treated as individual images, or as an animation sequence in which case
/// motion blurring with temporal samples will be applied.
/// It keeps a pointer to the main window and the global settings object for convenience.
/// The settings used here are saved to the /finalrender portion of the settings file.
/// It has its own OpenCLWrapper member for populating the combo boxes.
/// Upon running, it will delete the main window's renderer to save memory/GPU resources and restore it to its
/// original state upon exiting.
/// This class uses a controller-based design similar to the main window.
/// </summary>
class FractoriumFinalRenderDialog : public QDialog
{
	Q_OBJECT

	friend Fractorium;
	friend FinalRenderEmberControllerBase;
	friend FinalRenderEmberController<float>;
	friend PreviewRenderer<float>;
	friend FinalRenderPreviewRenderer<float>;

#ifdef DO_DOUBLE
	friend FinalRenderEmberController<double>;
	friend PreviewRenderer<double>;
	friend FinalRenderPreviewRenderer<double>;
#endif

public:
	FractoriumFinalRenderDialog(QWidget* p, Qt::WindowFlags f = Qt::WindowType::Widget);
	~FractoriumFinalRenderDialog();
	void Show(bool fromSequence);
	bool EarlyClip();
	bool YAxisUp();
	bool Transparency();
	bool OpenCL();
	bool Double();
	bool SaveXml();
	bool DoAll();
	bool Png16Bit();
	bool DoSequence();
	bool UseNumbers();
	bool KeepAspect();
	bool ApplyToAll();
	eScaleType Scale();
	void Scale(eScaleType scale);
	QString Ext();
	QString Path();
	void Path(const QString& s);
	QString Prefix();
	QString Suffix();
	uint Current();
	uint StartAt();
	uint ThreadCount();
	int ThreadPriority();
	double OpenCLSubBatchPct();
	double WidthScale();
	double HeightScale();
	double Quality();
	uint TemporalSamples();
	uint Supersample();
	uint Strips();
	QList<QVariant> Devices();
	FinalRenderGuiState State();

public slots:
	void MoveCursorToEnd();
	void OnEarlyClipCheckBoxStateChanged(Qt::CheckState state);
	void OnYAxisUpCheckBoxStateChanged(Qt::CheckState state);
	void OnTransparencyCheckBoxStateChanged(Qt::CheckState state);
	void OnOpenCLCheckBoxStateChanged(Qt::CheckState state);
	void OnDoublePrecisionCheckBoxStateChanged(Qt::CheckState state);
	void OnDoAllCheckBoxStateChanged(Qt::CheckState state);
	void OnDoSequenceCheckBoxStateChanged(Qt::CheckState state);
	void OnCurrentSpinChanged(int d);
	void OnApplyAllCheckBoxStateChanged(Qt::CheckState state);
	void OnWidthScaleChanged(double d);
	void OnWidthChanged(int d);
	void OnHeightScaleChanged(double d);
	void OnHeightChanged(int d);
	void OnKeepAspectCheckBoxStateChanged(Qt::CheckState state);
	void OnScaleRadioButtonChanged(bool checked);
	void OnDeviceTableCellChanged(int row, int col);
	void OnDeviceTableRadioToggled(bool checked);
	void OnQualityChanged(double d);
	void OnTemporalSamplesChanged(int d);
	void OnSupersampleChanged(int d);
	void OnStripsChanged(int d);
	void OnFileButtonClicked(bool checked);
	void OnShowFolderButtonClicked(bool checked);
	void OnExtIndexChanged(int d);
	void OnPrefixChanged(const QString& s);
	void OnSuffixChanged(const QString& s);
	void OnQualityBumpClicked();
	void OnSaveAgainAsClicked();
	void OnRenderClicked(bool checked);
	void OnPauseClicked(bool checked);
	void OnCancelRenderClicked(bool checked);
	void Pause(bool paused);
	virtual void reject() override;

protected:
	virtual void showEvent(QShowEvent* e) override;

private:
	bool CreateControllerFromGUI(bool createRenderer);
	bool SetMemory();
	static QString m_Prefix;
	static QString m_Suffix;

	bool m_FromSequence;
	int m_MemoryCellIndex;
	int m_ItersCellIndex;
	int m_PathCellIndex;
	Timing m_RenderTimer;
	DoubleSpinBox* m_WidthScaleSpin;
	SpinBox* m_WidthSpin;
	DoubleSpinBox* m_HeightScaleSpin;
	DoubleSpinBox* m_SubBatchPctSpin;
	SpinBox* m_HeightSpin;
	DoubleIntSpinnerWidget* m_WidthSpinnerWidget;
	DoubleIntSpinnerWidget* m_HeightSpinnerWidget;
	DoubleSpinBox* m_QualitySpin;
	SpinBox* m_TemporalSamplesSpin;
	SpinBox* m_SupersampleSpin;
	SpinBox* m_StripsSpin;
	TwoButtonComboWidget* m_Tbcw;
	QLineEdit* m_PrefixEdit;
	QLineEdit* m_SuffixEdit;
	shared_ptr<FractoriumSettings> m_Settings;
	Fractorium* m_Fractorium;
	shared_ptr<OpenCLInfo> m_Info;
	vector<OpenCLWrapper> m_Wrappers;
	unique_ptr<FinalRenderEmberControllerBase> m_Controller;
	Ui::FinalRenderDialog ui;
};
