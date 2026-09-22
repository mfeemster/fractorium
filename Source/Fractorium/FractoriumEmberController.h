#pragma once

#include "EmberFile.h"
#include "GLEmberController.h"

/// <summary>
/// FractoriumEmberControllerBase and FractoriumEmberController<T> classes.
/// </summary>

/// <summary>
/// An enum representing the type of edit being done.
/// </summary>
enum class eEditUndoState : et { REGULAR_EDIT, UNDO_REDO, EDIT_UNDO };

/// <summary>
/// An enum representing which xforms an update should be applied to.
/// </summary>
enum class eXformUpdate : et { UPDATE_SPECIFIC, UPDATE_CURRENT, UPDATE_SELECTED, UPDATE_CURRENT_AND_SELECTED, UPDATE_SELECTED_EXCEPT_FINAL, UPDATE_ALL, UPDATE_ALL_EXCEPT_FINAL };

/// <summary>
/// An enum representing the type of synchronizing to do between the list of Embers kept in memory
/// and the widgets in the library tree.
/// </summary>
enum class eLibraryUpdate { INDEX = 1, NAME = 2, POINTER = 4 };

/// <summary>
/// FractoriumEmberController and Fractorium need each other, but each can't include the other.
/// So Fractorium includes this file, and Fractorium is declared as a forward declaration here.
/// </summary>
class Fractorium;
template <typename T> class PreviewRenderer;
template <typename T> class TreePreviewRenderer;

#define PREVIEW_SIZE 128
#define UNDO_SIZE 512

/// <summary>
/// FractoriumEmberControllerBase serves as a non-templated base class with virtual
/// functions which will be overridden in a derived class that takes a template parameter.
/// The controller serves as a way to access both the Fractorium GUI and the underlying ember
/// objects through an interface that doesn't require template argument, but does allow
/// templated objects to be used underneath.
/// Note that there are a few functions which access a templated object, so for those both
/// versions for float and double must be provided, then overridden in the templated derived
/// class. It's definitely a design flaw, but C++ doesn't offer any alternative since
/// templated virtual functions are not supported.
/// The functions not implemented in this file can be found in the GUI files which use them.
/// </summary>
class FractoriumEmberControllerBase : public RenderCallback
{
	friend Fractorium;

public:
	FractoriumEmberControllerBase(Fractorium* fractorium);
	FractoriumEmberControllerBase(const FractoriumEmberControllerBase& controller) = delete;
	virtual ~FractoriumEmberControllerBase();

	//Embers.
	virtual void SetEmber(const Ember<float>& ember, bool verbatim, bool updatePointer, int xformIndex) { }
	virtual void CopyEmber(Ember<float>& ember, std::function<void(Ember<float>& ember)> perEmberOperation/* = [&](Ember<float>& ember) { }*/) { }//Uncomment default lambdas once LLVM fixes a crash in their compiler with default lambda parameters.//TODO
	virtual void SetEmberFile(const EmberFile<float>& emberFile, bool move) { }
	virtual void CopyEmberFile(EmberFile<float>& emberFile, bool sequence, std::function<void(Ember<float>& ember)> perEmberOperation/* = [&](Ember<float>& ember) { }*/) { }
	virtual void CopyXaosToggleEmber(Ember<float>& ember) { }
	virtual void SetXaosToggleEmber(const Ember<float>& ember) { }
	virtual void SetTempPalette(const Palette<float>& palette) { }
	virtual void CopyTempPalette(Palette<float>& palette) { }
#ifdef DO_DOUBLE
	virtual void SetEmber(const Ember<double>& ember, bool verbatim, bool updatePointer, int xformIndex) { }
	virtual void CopyEmber(Ember<double>& ember, std::function<void(Ember<double>& ember)> perEmberOperation/* = [&](Ember<double>& ember) { }*/) { }
	virtual void SetEmberFile(const EmberFile<double>& emberFile, bool move) { }
	virtual void CopyEmberFile(EmberFile<double>& emberFile, bool sequence, std::function<void(Ember<double>& ember)> perEmberOperation/* = [&](Ember<double>& ember) { }*/) { }
	virtual void CopyXaosToggleEmber(Ember<double>& ember) { }
	virtual void SetXaosToggleEmber(const Ember<double>& ember) { }
	virtual void SetTempPalette(const Palette<double>& palette) { }
	virtual void CopyTempPalette(Palette<double>& palette) { }
#endif
	virtual void SetEmber(size_t index, bool verbatim) { }
	virtual void AddXform() { }
	virtual void AddLinkedXform() { }
	virtual void DuplicateXform() { }
	virtual void ClearXform() { }
	virtual void DeleteXforms() { }
	virtual void AddFinalXform() { }
	virtual bool UseFinalXform() const noexcept { return false; }
	virtual size_t XformCount() const noexcept { return 0; }
	virtual size_t TotalXformCount() const noexcept { return 0; }
	virtual QString Name() const { return ""; }
	virtual void Name(const string& s) { }
	virtual size_t FinalRasW() const noexcept { return 0; }
	virtual void FinalRasW(size_t w) noexcept { }
	virtual size_t FinalRasH() const noexcept { return 0; }
	virtual void FinalRasH(size_t h) noexcept { }
	virtual size_t Index() const noexcept { return 0; }
	virtual void AddSymmetry(int sym, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) { }
	virtual void CalcNormalizedWeights() { }

	//Menu.
	virtual void NewFlock(size_t count) { }//File.
	virtual void NewEmptyFlameInCurrentFile() { }
	virtual void NewRandomFlameInCurrentFile() { }
	virtual void CopyFlameInCurrentFile() { }
	virtual void CreateReferenceFile() { }
	virtual void OpenAndPrepFiles(const QStringList& filenames, bool append) { }
	virtual void SaveCurrentAsXml(QString filename = "") { }
	virtual void SaveEntireFileAsXml() { }
	virtual uint SaveCurrentToOpenedFile(bool render = true) { return 0; }
	virtual void SaveCurrentFileOnShutdown() { }
	virtual void Undo() { }//Edit.
	virtual void Redo() { }
	virtual void CopyXml() { }
	virtual void CopyAllXml() { }
	virtual void PasteXmlAppend() { }
	virtual void PasteXmlOver() { }
	virtual void CopySelectedXforms() { }
	virtual void PasteSelectedXforms() { }
	virtual void CopyKernel() { }
	virtual void AddReflectiveSymmetry() { }//Tools.
	virtual void AddRotationalSymmetry() { }
	virtual void AddBothSymmetry() { }
	virtual void Flatten() { }
	virtual void Unflatten() { }
	virtual void ClearFlame() { }

	//Toolbar.

	//Library.
	virtual void SyncLibrary(eLibraryUpdate update) { }
	virtual void FillLibraryTree(int selectIndex = -1) { }
	virtual void UpdateLibraryTree() { }
	virtual void EmberTreeItemChanged(QTreeWidgetItem* item, int col) { }
	virtual void EmberTreeItemDoubleClicked(QTreeWidgetItem* item, int col) { }
	virtual void RenderLibraryPreviews(uint start = UINT_MAX, uint end = UINT_MAX) { }
	virtual void RenderSequencePreviews(uint start = UINT_MAX, uint end = UINT_MAX) { }
	virtual void SequenceTreeItemChanged(QTreeWidgetItem* item, int col) { }
	virtual void StopLibraryPreviewRender() { }
	virtual void StopSequencePreviewRender() { }
	virtual void StopAllPreviewRenderers() { }
	virtual void MoveLibraryItems(const QModelIndexList& items, int destRow) { }
	virtual void Delete(const vector<pair<size_t, QTreeWidgetItem*>>& v) { }
	virtual void FillSequenceTree() { }
	virtual void AddAnimationItem() { }
	virtual void SequenceGenerateButtonClicked() { }
	virtual void SequenceSaveButtonClicked() { }
	virtual void SequenceOpenButtonClicked() { }
	virtual void SequenceAnimateButtonClicked() { }
	virtual void SequenceAnimateNextFrame() { }
	virtual void SequenceClearButtonClicked() { }

	//Params.
	virtual void ParamsToEmber(Ember<float>& ember, bool imageParamsOnly = false) { };
#ifdef DO_DOUBLE
	virtual void ParamsToEmber(Ember<double>& ember, bool imageParamsOnly = false) { };
#endif
	virtual void SetCenter(double x, double y) { }
	virtual void FillParamTablesAndPalette() { }
	virtual void BrightnessChanged(double d) { }
	virtual void GammaChanged(double d) { }
	virtual void GammaThresholdChanged(double d) { }
	virtual void VibrancyChanged(double d) { }
	virtual void HighlightPowerChanged(double d) { }
	virtual void K2Changed(double d) { }
	virtual void BackgroundChanged(const QColor& color) { }
	virtual void PaletteModeChanged(uint i) { }
	virtual void WidthChanged(uint i) { }
	virtual void HeightChanged(uint i) { }
	virtual void ResizeAndScale(int width, int height, eScaleType scaleType) { }
	virtual void CenterXChanged(double d) { }
	virtual void CenterYChanged(double d) { }
	virtual void ScaleChanged(double d) { }
	virtual void ZoomChanged(double d) { }
	virtual void RotateChanged(double d) { }
	virtual void ZPosChanged(double d) { }
	virtual void PerspectiveChanged(double d) { }
	virtual void PitchChanged(double d) { }
	virtual void YawChanged(double d) { }
	virtual void DepthBlurChanged(double d) { }
	virtual void BlurCurveChanged(double d) { }
	virtual void SpatialFilterWidthChanged(double d) { }
	virtual void SpatialFilterTypeChanged(const QString& text) { }
	virtual void DEFilterMinRadiusWidthChanged(double d) { }
	virtual void DEFilterMaxRadiusWidthChanged(double d) { }
	virtual void DEFilterCurveWidthChanged(double d) { }
	virtual void SbsChanged(int d) { }
	virtual void FuseChanged(int d) { }
	virtual void RandRangeChanged(double d) { }
	virtual void QualityChanged(double d) { }
	virtual void SupersampleChanged(int d) { }
	virtual void InterpTypeChanged(int i) { }
	virtual void AffineInterpTypeChanged(int i) { }
	virtual void RotationsChanged(double d) { }
	virtual void SecondsPerRotationChanged(double d) { }
	virtual void RotateXformsDirChanged(uint i) { }
	virtual void BlendSecondsChanged(double d) { };
	virtual void RotationsPerBlendChanged(uint i) { };
	virtual void BlendXformsRotateDirChanged(uint i) { }
	virtual void BlendInterpTypeChanged(uint i) { };
	virtual void StaggerChanged(double d) { };
	virtual void TemporalFilterWidthChanged(double d) { }
	virtual void TemporalFilterTypeChanged(const QString& text) { }
	virtual void ExpChanged(double d) { }
	virtual void ClearColorCurves(int i) { }
	virtual void ColorCurveChanged(int curveIndex, int pointInxed, const QPointF& point) { }
	virtual void ColorCurvesPointAdded(size_t curveIndex, const QPointF& point) { }
	virtual void ColorCurvesPointRemoved(size_t curveIndex, int pointIndex) { }

	//Xforms.
	virtual void CurrentXformComboChanged(int index) { }
	virtual void XformWeightChanged(double d) { }
	virtual void EqualizeWeights() { }
	virtual void XformNameChanged(const QString& s) { }
	virtual void XformAnimateChangedHelper(int state, bool local) { }
	virtual void FillXforms(int index = 0) { }
	virtual void UpdateXformName(int index) { }

	//Xforms Affine.
	virtual void AffineSetHelper(double d, int index, bool pre) { }
	virtual void FlipXforms(bool horizontal, bool vertical, bool pre) { }
	virtual void RotateXformsByAngle(double angle, bool pre) { }
	virtual void MoveXforms(double x, double y, bool pre) { }
	virtual void ScaleXforms(double scale, bool pre) { }
	virtual void ResetXformsAffine(bool pre) { }
	virtual void CopyXformsAffine(bool pre) { }
	virtual void PasteXformsAffine(bool pre) { }
	virtual void RandomXformsAffine(bool pre) { }
	virtual void FillBothAffines() { }
	virtual void SwapAffines() { }
	double LockedScale() const noexcept { return m_LockedScale; }
	double LockedX() const noexcept { return m_LockedX; }
	double LockedY() const noexcept { return m_LockedY; }
	void LockedScale(double scale) noexcept { m_LockedScale = scale; }
	virtual void InitLockedScale() noexcept { }
	virtual double AffineScaleCurrentToLocked() noexcept { return 0; };
	virtual double AffineScaleLockedToCurrent() noexcept { return 0; };

	//Xforms Color.
	virtual void XformColorIndexChanged(double d, bool updateRender, bool updateSpinner, bool updateScroll, eXformUpdate update = eXformUpdate::UPDATE_SELECTED, size_t index = 0) { }
	virtual void RandomColorIndicesButtonClicked() { }
	virtual void ToggleColorIndicesButtonClicked() { }
	virtual void RandomColorSpeedButtonClicked() { }
	virtual void ToggleColorSpeedsButtonClicked() { }
	virtual void XformColorSpeedChanged(double d) { }
	virtual void XformOpacityChanged(double d) { }
	virtual void XformDirectColorChanged(double d) { }
	virtual void SoloXformCheckBoxStateChanged(int state, int index) { }
	virtual QColor ColorIndexToQColor(double d) { return QColor(); }

	//Xforms Variations.
	virtual void Filter(const QString& text) { }
	virtual void SetupVariationsTree() { }
	virtual void ClearVariationsTree() { }
	virtual void VariationSpinBoxValueChanged(double d) { }
	virtual void FilteredVariations() { }
	virtual void FillVariationTreeWithCurrentXform() { }

	//Xforms Selection.
	virtual QString MakeXformCaption(size_t i) { return ""; }

	//Xaos.
	virtual void FillXaos() { }
	virtual void FillAppliedXaos() { }
	virtual void XaosChanged(int x, int y, double val) { }
	virtual void ClearXaos() { }
	virtual void RandomXaos() { }
	virtual void AddLayer(int xforms) { }
	virtual void TransposeXaos() { }
	virtual void ToggleXaos() { }

	//Palette.
	virtual size_t InitPaletteList(const QString& s) { return 0; }
	virtual bool FillPaletteTable(const string& s) { return false; }
	virtual void ApplyPaletteToEmber() { }
	virtual void PaletteAdjust() { }
	virtual void PaletteCellClicked(int row, int col) { }
	virtual void SetBasePaletteAndAdjust(const Palette<float>& palette) { }
	virtual void PaletteEditorButtonClicked() { }
	virtual void PaletteEditorColorChanged() { }
	virtual void SyncPalette(bool accepted) { }
	QImage& FinalPaletteImage() { return m_FinalPaletteImage; }

	//Info.
	virtual void FillSummary() { }
	virtual void ReorderVariations(QTreeWidgetItem* item) { }

	//Rendering/progress.
	virtual bool Render() { return false; }
	virtual bool CreateRenderer(eRendererType renderType, const vector<pair<size_t, size_t>>& devices, bool updatePreviews, bool shared = true) { return false; }
	virtual uint SizeOfT() const noexcept { return 0; }
	virtual void ClearUndo() { }
	virtual void DeleteRenderer() { }
	virtual GLEmberControllerBase* GLController() { return nullptr; }
	virtual void Pause(bool pause);
	virtual bool Paused();
	bool RenderTimerRunning();
	void StartRenderTimer();
	void DelayedStartRenderTimer();
	void StopRenderTimer(bool wait);
	void ClearFinalImages();
	void Shutdown();
	void UpdateRender(eProcessAction action = eProcessAction::FULL_RENDER);
	bool SaveCurrentRender(const QString& filename, const EmberImageComments& comments, vector<v4F>& pixels, size_t width, size_t height, bool png16Bit, bool transparency);
	RendererBase* Renderer() { return m_Renderer.get(); }
	vector<v4F>* FinalImage() { return &(m_FinalImage); }
	vector<v4F>* PreviewFinalImage() { return &m_PreviewFinalImage; }
	EmberStats Stats() { return m_Stats; }
	eProcessState ProcessState() { return m_Renderer.get() ? m_Renderer->ProcessState() : eProcessState::NONE; }

protected:
	//Rendering/progress.
	void AddProcessAction(eProcessAction action);
	eProcessAction CondenseAndClearProcessActions();

	//Non-templated members.
	bool m_Rendering = false;
	bool m_LastEditWasUndoRedo;
	vector<pair<size_t, size_t>> m_Devices;
	size_t m_SubBatchCount = 1;//Will be ovewritten by the options on first render.
	uint m_FailedRenders = 0;
	size_t m_UndoIndex = 0;
	double m_LockedScale = 1;
	double m_LockedX = 0;
	double m_LockedY = 0;
	eRendererType m_RenderType = eRendererType::CPU_RENDERER;
	eEditUndoState m_EditState;
	GLuint m_OutputTexID = 0;
	Timing m_RenderElapsedTimer;
	EmberStats m_Stats;
	QImage m_FinalPaletteImage;
	QString m_LastSaveAll;
	QString m_LastSaveCurrent;
	string m_CurrentPaletteFilePath;
	std::recursive_mutex m_Cs;
	std::thread m_WriteThread;
	vector<v4F> m_FinalImage;
	vector<v4F> m_PreviewFinalImage;
	vector<eProcessAction> m_ProcessActions;
	vector<eVariationId> m_FilteredVariations;
	unique_ptr<EmberNs::RendererBase> m_Renderer;
	QTIsaac<ISAAC_SIZE, ISAAC_INT> m_Rand;
	Fractorium* m_Fractorium;
	Palette<float> m_TempPalette, m_PreviousTempPalette;
	std::unique_ptr<QTimer> m_RenderTimer;
	std::unique_ptr<QTimer> m_RenderRestartTimer;
	std::unique_ptr<QTimer> m_AnimateTimer;
	shared_ptr<PaletteList<float>> m_PaletteList;
	shared_ptr<OpenCLInfo> m_Info = OpenCLInfo::Instance();
	int m_AnimateFrame = 0;
};

/// <summary>
/// Templated derived class which implements all interaction functionality between the embers
/// of a specific template type and the GUI.
/// Switching between template arguments requires complete re-creation of the controller and the
/// underlying renderer. Switching between CPU and OpenCL only requires re-creation of the renderer.
/// </summary>
template<typename T>
class FractoriumEmberController : public FractoriumEmberControllerBase
{
	friend PreviewRenderer<T>;
	friend TreePreviewRenderer<T>;

public:
	FractoriumEmberController(Fractorium* fractorium);
	FractoriumEmberController(const FractoriumEmberController<T>& controller) = delete;
	virtual ~FractoriumEmberController();

	//Embers.
	void SetEmber(const Ember<float>& ember, bool verbatim, bool updatePointer, int xformIndex) override;
	void CopyEmber(Ember<float>& ember, std::function<void(Ember<float>& ember)> perEmberOperation/* = [&](Ember<float>& ember) { }*/) override;
	void SetEmberFile(const EmberFile<float>& emberFile, bool move) override;
	void CopyEmberFile(EmberFile<float>& emberFile, bool sequence, std::function<void(Ember<float>& ember)> perEmberOperation/* = [&](Ember<float>& ember) { }*/) override;
	void CopyXaosToggleEmber(Ember<float>& ember) override;
	void SetXaosToggleEmber(const Ember<float>& ember) override;
	void SetTempPalette(const Palette<float>& palette) override;
	void CopyTempPalette(Palette<float>& palette) override;
#ifdef DO_DOUBLE
	void SetEmber(const Ember<double>& ember, bool verbatim, bool updatePointer, int xformIndex) override;
	void CopyEmber(Ember<double>& ember, std::function<void(Ember<double>& ember)> perEmberOperation/* = [&](Ember<double>& ember) { }*/) override;
	void SetEmberFile(const EmberFile<double>& emberFile, bool move) override;
	void CopyEmberFile(EmberFile<double>& emberFile, bool sequence, std::function<void(Ember<double>& ember)> perEmberOperation/* = [&](Ember<double>& ember) { }*/) override;
	void CopyXaosToggleEmber(Ember<double>& ember) override;
	void SetXaosToggleEmber(const Ember<double>& ember) override;
	void SetTempPalette(const Palette<double>& palette) override;
	void CopyTempPalette(Palette<double>& palette) override;
#endif
	void SetEmber(size_t index, bool verbatim) override;
	void AddXform() override;
	void AddLinkedXform() override;
	void DuplicateXform() override;
	void ClearXform() override;
	void DeleteXforms() override;
	void AddFinalXform() override;
	bool UseFinalXform() const noexcept override { return m_Ember.UseFinalXform(); }
	size_t XformCount() const noexcept override { return m_Ember.XformCount(); }
	size_t TotalXformCount() const noexcept override { return m_Ember.TotalXformCount(); }
	QString Name() const override { return QString::fromStdString(m_Ember.m_Name); }
	void Name(const string& s) override { m_Ember.m_Name = s; }
	size_t FinalRasW() const noexcept override { return m_Ember.m_FinalRasW; }
	void FinalRasW(size_t w) noexcept override { m_Ember.m_FinalRasW = w; }
	size_t FinalRasH() const noexcept override { return m_Ember.m_FinalRasH; }
	void FinalRasH(size_t h) noexcept override { m_Ember.m_FinalRasH = h; }
	size_t Index() const noexcept override { return m_Ember.m_Index; }
	void AddSymmetry(int sym, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override { m_Ember.AddSymmetry(sym, rand); }
	void CalcNormalizedWeights() override { m_Ember.CalcNormalizedWeights(m_NormalizedWeights); }
	void ConstrainDimensions(Ember<T>& ember);
	Ember<T>* CurrentEmber();

	//Menu.
	void NewFlock(size_t count) override;
	void NewEmptyFlameInCurrentFile() override;
	void NewRandomFlameInCurrentFile() override;
	void CopyFlameInCurrentFile() override;
	void CreateReferenceFile() override;
	void OpenAndPrepFiles(const QStringList& filenames, bool append) override;
	void SaveCurrentAsXml(QString filename = "") override;
	void SaveEntireFileAsXml() override;
	uint SaveCurrentToOpenedFile(bool render = true) override;
	void SaveCurrentFileOnShutdown() override;
	void Undo() override;
	void Redo() override;
	void CopyXml() override;
	void CopyAllXml() override;
	void PasteXmlAppend() override;
	void PasteXmlOver() override;
	void CopySelectedXforms() override;
	void PasteSelectedXforms() override;
	void CopyKernel() override;
	void AddReflectiveSymmetry() override;
	void AddRotationalSymmetry() override;
	void AddBothSymmetry() override;
	void Flatten() override;
	void Unflatten() override;
	void ClearFlame() override;

	//Toolbar.

	//Library.
	void SyncLibrary(eLibraryUpdate update) override;
	void FillLibraryTree(int selectIndex = -1) override;
	void UpdateLibraryTree() override;
	void MoveLibraryItems(const QModelIndexList& items, int destRow) override;
	void Delete(const vector<pair<size_t, QTreeWidgetItem*>>& v) override;
	void EmberTreeItemChanged(QTreeWidgetItem* item, int col) override;
	void EmberTreeItemDoubleClicked(QTreeWidgetItem* item, int col) override;
	void RenderPreviews(QTreeWidget* tree, TreePreviewRenderer<T>* renderer, EmberFile<T>& file, uint start = UINT_MAX, uint end = UINT_MAX);
	void RenderLibraryPreviews(uint start = UINT_MAX, uint end = UINT_MAX) override;
	void RenderSequencePreviews(uint start = UINT_MAX, uint end = UINT_MAX) override;
	void SequenceTreeItemChanged(QTreeWidgetItem* item, int col) override;
	void StopLibraryPreviewRender() override;
	void StopSequencePreviewRender() override;
	void StopAllPreviewRenderers() override;
	void FillSequenceTree() override;
	void AddAnimationItem() override;
	void SequenceGenerateButtonClicked() override;
	void SequenceSaveButtonClicked() override;
	void SequenceOpenButtonClicked() override;
	void SequenceAnimateButtonClicked() override;
	void SequenceAnimateNextFrame() override;
	void SequenceClearButtonClicked() override;

	//Params.
	void ParamsToEmber(Ember<float>& ember, bool imageParamsOnly = false) override;
#ifdef DO_DOUBLE
	void ParamsToEmber(Ember<double>& ember, bool imageParamsOnly = false) override;
#endif
	void SetCenter(double x, double y) override;
	void FillParamTablesAndPalette() override;
	void BrightnessChanged(double d) override;
	void GammaChanged(double d) override;
	void GammaThresholdChanged(double d) override;
	void VibrancyChanged(double d) override;
	void HighlightPowerChanged(double d) override;
	void K2Changed(double d) override;
	void BackgroundChanged(const QColor& col) override;
	void PaletteModeChanged(uint i) override;
	void WidthChanged(uint i) override;
	void HeightChanged(uint i) override;
	void ResizeAndScale(int width, int height, eScaleType scaleType) override;
	void CenterXChanged(double d) override;
	void CenterYChanged(double d) override;
	void ScaleChanged(double d) override;
	void ZoomChanged(double d) override;
	void RotateChanged(double d) override;
	void ZPosChanged(double d) override;
	void PerspectiveChanged(double d) override;
	void PitchChanged(double d) override;
	void YawChanged(double d) override;
	void DepthBlurChanged(double d) override;
	void BlurCurveChanged(double d) override;
	void SpatialFilterWidthChanged(double d) override;
	void SpatialFilterTypeChanged(const QString& text) override;
	void DEFilterMinRadiusWidthChanged(double d) override;
	void DEFilterMaxRadiusWidthChanged(double d) override;
	void DEFilterCurveWidthChanged(double d) override;
	void SbsChanged(int d) override;
	void FuseChanged(int d) override;
	void RandRangeChanged(double d) override;
	void QualityChanged(double d) override;
	void SupersampleChanged(int d) override;
	void InterpTypeChanged(int index) override;
	void AffineInterpTypeChanged(int index) override;
	void RotationsChanged(double d) override;
	void SecondsPerRotationChanged(double d) override;
	void RotateXformsDirChanged(uint i) override;
	void BlendSecondsChanged(double d) override;
	void RotationsPerBlendChanged(uint i) override;
	void BlendXformsRotateDirChanged(uint i) override;
	void BlendInterpTypeChanged(uint i) override;
	void StaggerChanged(double d) override;
	void TemporalFilterWidthChanged(double d) override;
	void TemporalFilterTypeChanged(const QString& text) override;
	void ExpChanged(double d) override;
	void ClearColorCurves(int i) override;
	void ColorCurveChanged(int curveIndex, int pointInxed, const QPointF& point) override;
	void ColorCurvesPointAdded(size_t curveIndex, const QPointF& point) override;
	void ColorCurvesPointRemoved(size_t curveIndex, int pointIndex) override;

	//Xforms.
	void CurrentXformComboChanged(int index) override;
	void XformWeightChanged(double d) override;
	void EqualizeWeights() override;
	void XformNameChanged(const QString& s) override;
	void XformAnimateChangedHelper(int state, bool local) override;
	void FillXforms(int index = 0) override;
	void UpdateXformName(int index) override;
	void FillWithXform(Xform<T>* xform);
	Xform<T>* CurrentXform();
	void UpdateXform(std::function<void(Xform<T>*, size_t, size_t)> func, eXformUpdate updateType = eXformUpdate::UPDATE_CURRENT, bool updateRender = true, eProcessAction action = eProcessAction::FULL_RENDER, size_t index = 0);
	static void AddXformsWithXaos(Ember<T>& ember, std::vector<std::pair<Xform<T>, size_t>>& xforms, eXaosPasteStyle pastestyle);

	//Xforms Affine.
	void AffineSetHelper(double d, int index, bool pre) override;
	void FlipXforms(bool horizontal, bool vertical, bool pre) override;
	void RotateXformsByAngle(double angle, bool pre) override;
	void MoveXforms(double x, double y, bool pre) override;
	void ScaleXforms(double scale, bool pre) override;
	void ResetXformsAffine(bool pre) override;
	void CopyXformsAffine(bool pre) override;
	void PasteXformsAffine(bool pre) override;
	void RandomXformsAffine(bool pre) override;
	void FillBothAffines() override;
	void SwapAffines() override;
	void InitLockedScale() noexcept override;
	double AffineScaleCurrentToLocked() noexcept override;
	double AffineScaleLockedToCurrent() noexcept override;
	void FillAffineWithXform(Xform<T>* xform, bool pre);
	void ChangeLockedScale(T value);

	//Xforms Color.
	void XformColorIndexChanged(double d, bool updateRender, bool updateSpinner, bool updateScroll, eXformUpdate update = eXformUpdate::UPDATE_SELECTED, size_t index = 0) override;
	void RandomColorIndicesButtonClicked() override;
	void ToggleColorIndicesButtonClicked() override;
	void RandomColorSpeedButtonClicked() override;
	void ToggleColorSpeedsButtonClicked() override;
	void XformColorSpeedChanged(double d) override;
	void XformOpacityChanged(double d) override;
	void XformDirectColorChanged(double d) override;
	void SoloXformCheckBoxStateChanged(int state, int index) override;
	QColor ColorIndexToQColor(double d) override;
	void FillColorWithXform(Xform<T>* xform);

	//Xforms Variations.
	void Filter(const QString& text) override;
	void SetupVariationsTree() override;
	void ClearVariationsTree() override;
	void VariationSpinBoxValueChanged(double d) override;
	void FilteredVariations() override;
	void FillVariationTreeWithCurrentXform() override;
	void FillVariationTreeWithXform(Xform<T>* xform);
	QIcon MakeVariationIcon(const Variation<T>* var, int iconSize);

	//Xforms Xaos.
	void FillXaos() override;
	void FillAppliedXaos() override;
	void XaosChanged(int x, int y, double val) override;
	void ClearXaos() override;
	void RandomXaos() override;
	void AddLayer(int xforms) override;
	void TransposeXaos() override;
	void ToggleXaos() override;

	//Xforms Selection.
	virtual QString MakeXformCaption(size_t i) override;
	bool XformCheckboxAt(int i, std::function<void(QCheckBox*)> func);
	bool XformCheckboxAt(Xform<T>* xform, std::function<void(QCheckBox*)> func);

	//Palette.
	size_t InitPaletteList(const QString& s) override;
	bool FillPaletteTable(const string& s) override;
	void ApplyPaletteToEmber() override;
	void PaletteAdjust() override;
	void PaletteCellClicked(int row, int col) override;
	void SetBasePaletteAndAdjust(const Palette<float>& palette) override;
	void PaletteEditorButtonClicked() override;
	void PaletteEditorColorChanged() override;
	void SyncPalette(bool accepted) override;

	//Info.
	void FillSummary() override;
	void ReorderVariations(QTreeWidgetItem* item) override;

	//Rendering/progress.
	bool Render() override;
	bool CreateRenderer(eRendererType renderType, const vector<pair<size_t, size_t>>& devices, bool updatePreviews, bool shared = true) override;
	uint SizeOfT() const noexcept override { return sizeof(T); }
	int ProgressFunc(Ember<T>& ember, void* foo, double fraction, int stage, double etaMs) override;
	void ClearUndo() override;
	GLEmberControllerBase* GLController() override { return m_GLController.get(); }
	void DeleteRenderer() override;

private:
	//Embers.
	void ApplyXmlSavingTemplate(Ember<T>& ember);
	template <typename U> void SetEmberPrivate(const Ember<U>& ember, bool verbatim, bool updatePointer, int xformIndex);

	//Params.
	template <typename U> void ParamsToEmberPrivate(Ember<U>& ember, bool imageParamsOnly);

	//Xforms.
	void SetNormalizedWeightText(Xform<T>* xform);
	bool IsFinal(Xform<T>* xform);

	//Xforms Color.
	void FillCurvesControl();

	//Palette.
	void UpdateAdjustedPaletteGUI(Palette<float>& palette);

	//Rendering/progress.
	void Update(std::function<void (void)> func, bool updateRender = true, eProcessAction action = eProcessAction::FULL_RENDER);
	void UpdateAll(std::function<void (Ember<T>&, bool)> func, bool updateRender = true, eProcessAction action = eProcessAction::FULL_RENDER, bool applyAll = false);
	bool SyncSizes();

	//Templated members.
	bool m_PreviewRunning = false;
	vector<T> m_TempOpacities;
	vector<T> m_NormalizedWeights;
	Ember<T> m_Ember;
	Ember<T> m_XaosToggleEmber;
	Ember<T>* m_EmberFilePointer = nullptr;
	EmberFile<T> m_EmberFile;
	EmberFile<T> m_SequenceFile;
	deque<Ember<T>> m_UndoList;
	vector<std::pair<Xform<T>, size_t>> m_CopiedXforms;
	Xform<T> m_CopiedFinalXform;
	Affine2D<T> m_CopiedAffine;
	shared_ptr<VariationList<T>> m_VariationList;
	unique_ptr<SheepTools<T, float>> m_SheepTools;
	unique_ptr<GLEmberController<T>> m_GLController;
	unique_ptr<TreePreviewRenderer<T>> m_LibraryPreviewRenderer;
	unique_ptr<TreePreviewRenderer<T>> m_SequencePreviewRenderer;
};

/// <summary>
/// Base class for encapsulating a preview renderer which will be used
/// in such places as the main library tree, the sequence tree and the
/// single preview thumbnail shown in the final render dialog.
/// Derived classes will implement PreviewRenderFunc() to handle the rendering
/// functionality specific to their previews.
/// </summary>
template <typename T>
class PreviewRenderer
{
public:
	PreviewRenderer(QProgressBar* p)
	{
		m_Pb = p;
	}

	virtual ~PreviewRenderer()
	{
	}

	void Render(uint start, uint end)
	{
		Stop();
		m_PreviewResult = QtConcurrent::run([&](uint s, uint e)
		{
			rlg l(m_PreviewCs);
			m_PreviewRun = true;
			PreviewRenderFunc(s, e);
			m_PreviewRun = false;
		}, start, end);
	}

	void Stop()
	{
		m_PreviewRun = false;
		m_PreviewRenderer.Reset();

		while (m_PreviewResult.isRunning())
			QApplication::processEvents();
	}

	bool EarlyClip()
	{
		return m_PreviewRenderer.EarlyClip();
	}

	bool YAxisUp()
	{
		return m_PreviewRenderer.YAxisUp();
	}

	bool Running()
	{
		return m_PreviewRun || m_PreviewResult.isRunning();
	}

	virtual void PreviewRenderFunc(uint start, uint end) {}

protected:
	volatile bool m_PreviewRun = false;
	Ember<T> m_PreviewEmber;
	vector<unsigned char> m_PreviewVec;
	vv4F m_PreviewFinalImage;
	EmberNs::Renderer<T, float> m_PreviewRenderer;
	QProgressBar* m_Pb = nullptr;

private:
	QFuture<void> m_PreviewResult;
	std::recursive_mutex m_PreviewCs;
};

/// <summary>
/// Thin derivation to handle preview rendering multiple embers previews to a tree.
/// </summary>
template <typename T>
class TreePreviewRenderer : public PreviewRenderer<T>
{
public:
	using PreviewRenderer<T>::m_PreviewRun;
	using PreviewRenderer<T>::m_PreviewEmber;
	using PreviewRenderer<T>::m_PreviewVec;
	using PreviewRenderer<T>::m_PreviewRenderer;
	using PreviewRenderer<T>::m_PreviewFinalImage;
	using PreviewRenderer<T>::m_Pb;

	/// <summary>
	/// Initializes a new instance of the <see cref="TreePreviewRenderer{T}"/> class.
	/// </summary>
	/// <param name="controller">A pointer to the controller this instance is a member of</param>
	/// <param name="tree">A pointer to the tree to render to</param>
	/// <param name="emberFile">A reference to the ember file to render</param>
	TreePreviewRenderer(FractoriumEmberController<T>* controller, QTreeWidget* tree, EmberFile<T>& emberFile, QProgressBar* p) :
		m_Controller(controller),
		m_Tree(tree),
		m_EmberFile(emberFile),
		PreviewRenderer<T>(p)
	{
		const auto f = m_Controller->m_Fractorium;
		m_PreviewRenderer.Callback(nullptr);
		m_PreviewRenderer.EarlyClip(f->m_Settings->EarlyClip());
		m_PreviewRenderer.YAxisUp(f->m_Settings->YAxisUp());
	}

	void PreviewRenderFunc(uint start, uint end) override;

protected:
	FractoriumEmberController<T>* m_Controller;
	QTreeWidget* m_Tree;
	EmberFile<T>& m_EmberFile;
};
