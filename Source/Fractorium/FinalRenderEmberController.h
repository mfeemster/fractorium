#pragma once

#include "FractoriumSettings.h"
#include "FractoriumEmberController.h"

/// <summary>
/// FinalRenderEmberControllerBase and FinalRenderEmberController<T> classes.
/// </summary>

/// <summary>
/// FractoriumEmberController and Fractorium need each other, but each can't include the other.
/// So Fractorium includes this file, and Fractorium is declared as a forward declaration here.
/// </summary>
class Fractorium;
class FractoriumFinalRenderDialog;
template <typename T> class FinalRenderPreviewRenderer;

/// <summary>
/// Used to hold the options specified in the current state of the Gui for performing the final render.
/// </summary>
struct FinalRenderGuiState
{
	bool m_EarlyClip;
	bool m_YAxisUp;
	bool m_Transparency;
	bool m_OpenCL;
	bool m_Double;
	bool m_SaveXml;
	bool m_DoAll;
	bool m_Png16Bit;
	bool m_DoSequence;
	bool m_KeepAspect;
	bool m_UseNumbers;
	eScaleType m_Scale;
	QString m_Path;
	QString m_Ext;
	QString m_Prefix;
	QString m_Suffix;
	QList<QVariant> m_Devices;
	uint m_StartAt;
	uint m_ThreadCount;
	int m_ThreadPriority;
	double m_SubBatchPct;
	double m_WidthScale;
	double m_HeightScale;
	double m_Quality;
	uint m_TemporalSamples;
	uint m_Supersample;
	size_t m_Strips;
};

/// <summary>
/// FinalRenderEmberControllerBase serves as a non-templated base class with virtual
/// functions which will be overridden in a derived class that takes a template parameter.
/// Although not meant to be used as an interactive renderer, it derives from FractoriumEmberControllerBase
/// to access a few of its members to avoid having to redefine them here.
/// </summary>
class FinalRenderEmberControllerBase : public FractoriumEmberControllerBase
{
	friend FractoriumFinalRenderDialog;

public:
	FinalRenderEmberControllerBase(FractoriumFinalRenderDialog* finalRenderDialog);
	virtual ~FinalRenderEmberControllerBase() { }

	virtual void SyncCurrentToGui() { }
	virtual void SyncGuiToEmbers(size_t widthOverride = 0, size_t heightOverride = 0, bool dowidth = true, bool doheight = true) { }
	virtual void SyncCurrentToSizeSpinners(bool scale, bool size, bool doWidth = true, bool doHeight = true) { }
	virtual void ResetProgress(bool total = true) { }
	virtual tuple<size_t, size_t, size_t> SyncAndComputeMemory()  { return tuple<size_t, size_t, size_t>(0, 0, 0); }
	virtual double OriginalAspect()  { return 1; }
	virtual QString ComposePath(const QString& name, bool unique = true) { return ""; }
	virtual bool BumpQualityRender(double d)  { return false; }
	virtual QString SaveCurrentAgain() { return ""; }
	virtual void CancelRender() { }
	virtual QString CheckMemory(const tuple<size_t, size_t, size_t>& p) { return ""; }

	bool Running() { return m_Result.isRunning(); }
	bool CreateRendererFromGUI();
	void Output(const QString& s);

protected:
	bool m_Run = false;
	bool m_IsQualityBump = false;
	size_t m_ImageCount = 0;
	std::atomic<size_t> m_FinishedImageCount;

	QFuture<void> m_Result;
	std::function<void (void)> m_FinalRenderFunc;

	shared_ptr<FractoriumSettings> m_Settings;
	FractoriumFinalRenderDialog* m_FinalRenderDialog;
	FinalRenderGuiState m_GuiState;
	std::recursive_mutex m_ProgressCs;
	Timing m_RenderTimer;
	Timing m_TotalTimer;
};

/// <summary>
/// Templated derived class which implements all interaction functionality between the embers
/// of a specific template type and the final render dialog.
/// </summary>
template<typename T>
class FinalRenderEmberController : public FinalRenderEmberControllerBase
{
	friend FinalRenderPreviewRenderer<T>;

public:
	FinalRenderEmberController(FractoriumFinalRenderDialog* finalRender);
	virtual ~FinalRenderEmberController();

	//Virtual functions overridden from FractoriumEmberControllerBase.
	void SetEmberFile(const EmberFile<float>& emberFile, bool move) override;
	void CopyEmberFile(EmberFile<float>& emberFile, bool sequence, std::function<void(Ember<float>& ember)> perEmberOperation/* = [&](Ember<float>& ember) { }*/) override;
#ifdef DO_DOUBLE
	void SetEmberFile(const EmberFile<double>& emberFile, bool move) override;
	void CopyEmberFile(EmberFile<double>& emberFile, bool sequence, std::function<void(Ember<double>& ember)> perEmberOperation/* = [&](Ember<double>& ember) { }*/) override;
#endif
	void SetEmber(size_t index, bool verbatim) override;
	void SaveCurrentAsXml(QString filename = "") override;
	bool Render() override;
	bool BumpQualityRender(double d)  override;
    bool CreateRenderer(eRendererType renderType, const vector<pair<size_t, size_t>>& devices, bool updatePreviews, bool shared = true) override;
	int ProgressFunc(Ember<T>& ember, void* foo, double fraction, int stage, double etaMs) override;
	size_t Index() const noexcept override { return m_Ember->m_Index; }
	uint SizeOfT() const noexcept override { return sizeof(T); }

	//Virtual functions overridden from FinalRenderEmberControllerBase.
	void SyncCurrentToGui() override;
	void SyncGuiToEmbers(size_t widthOverride = 0, size_t heightOverride = 0, bool dowidth = true, bool doheight = true) override;
	void SyncCurrentToSizeSpinners(bool scale, bool size, bool doWidth = true, bool doHeight = true) override;
	void ResetProgress(bool total = true)  override;
	tuple<size_t, size_t, size_t> SyncAndComputeMemory() override;
	double OriginalAspect() override { return double(m_Ember->m_OrigFinalRasW) / m_Ember->m_OrigFinalRasH; }
	QString Name() const override { return QString::fromStdString(m_Ember->m_Name); }
	QString ComposePath(const QString& name, bool unique = true) override;
	QString SaveCurrentAgain() override;
	void CancelRender() override;
	QString CheckMemory(const tuple<size_t, size_t, size_t>& p) override;

	//Non Virtual functions.
	EmberNs::Renderer<T, float>* FirstOrDefaultRenderer();

protected:
	void Pause(bool pause) override;
	bool Paused() override;
	void HandleFinishedProgress();
	QString SaveCurrentRender(Ember<T>& ember);
	QString SaveCurrentRender(Ember<T>& ember, const EmberImageComments& comments, vector<v4F>& pixels, size_t width, size_t height, bool png16Bit, bool transparency);
	void RenderComplete(Ember<T>& ember);
	void RenderComplete(Ember<T>& ember, const EmberStats& stats, Timing& renderTimer);
	void SyncGuiToEmber(Ember<T>& ember, size_t widthOverride = 0, size_t heightOverride = 0, bool dowidth = true, bool doheight = true);
	bool SyncGuiToRenderer();
	void SetProgressComplete(int val);
	bool RenderSingleEmber(Ember<T>& ember, bool fullRender, size_t& stripForProgress);
	bool RenderSingleEmberFromSeries(std::atomic<size_t>* atomfTime, size_t index);

	Ember<T>* m_Ember;
	EmberFile<T> m_EmberFile;
	EmberToXml<T> m_XmlWriter;
	unique_ptr<FinalRenderPreviewRenderer<T>> m_FinalPreviewRenderer;
	vector<unique_ptr<EmberNs::Renderer<T, float>>> m_Renderers;
	ThreadedWriter m_ThreadedWriter;
};

/// <summary>
/// Thin derivation to handle preview rendering that is specific to the final render dialog.
/// This differs from the preview renderers on the main window because they render multiple embers
/// to a tree, whereas this renders a single preview.
/// </summary>
template <typename T>
class FinalRenderPreviewRenderer : public PreviewRenderer<T>
{
public:
	using PreviewRenderer<T>::m_PreviewRun;
	using PreviewRenderer<T>::m_PreviewVec;
	using PreviewRenderer<T>::m_PreviewEmber;
	using PreviewRenderer<T>::m_PreviewRenderer;
	using PreviewRenderer<T>::m_PreviewFinalImage;

    FinalRenderPreviewRenderer(FinalRenderEmberController<T>* controller) :
        PreviewRenderer<T>(nullptr),
        m_Controller(controller)
	{
	}

	void PreviewRenderFunc(uint start, uint end) override;

private:
	FinalRenderEmberController<T>* m_Controller;
};
