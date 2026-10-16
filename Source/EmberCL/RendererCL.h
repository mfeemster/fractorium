#pragma once

#include "EmberCLPch.h"
#include "OpenCLWrapper.h"
#include "DEOpenCLKernelCreator.h"
#include "FinalAccumOpenCLKernelCreator.h"
#include "RendererClDevice.h"

/// <summary>
/// RendererCLBase and RendererCL classes.
/// </summary>

namespace EmberCLns
{
/// <summary>
/// Serves only as an interface for OpenCL specific rendering functions.
/// </summary>
class EMBERCL_API RendererCLBase
{
public:
	virtual ~RendererCLBase() { }
	virtual bool ReadFinal(v4F* pixels) { return false; }
	virtual bool ClearFinal() { return false; }
	virtual bool AnyNvidia() const noexcept { return false; }
	bool OptAffine() const noexcept { return m_OptAffine; }
	void OptAffine(bool optAffine) noexcept { m_OptAffine = optAffine; }

	std::function<void(void)> m_CompileBegun;

protected:
	bool m_OptAffine = false;
};

/// <summary>
/// RendererCL is a derivation of the basic CPU renderer which
/// overrides various functions to render on the GPU using OpenCL.
/// This supports multi-GPU rendering and is done in the following manner:
///		-When rendering a single image, the iterations will be split between devices in sub batches.
///		-When animating, a renderer for each device will be created by the calling code,
///			and the frames will each be rendered by a single device as available.
/// The synchronization across devices is done through a single atomic counter.
/// Since this class derives from EmberReport and also contains an
/// OpenCLWrapper member which also derives from EmberReport, the
/// reporting functions are overridden to aggregate the errors from
/// both sources.
/// Template argument T expected to be float or double.
/// Template argument bucketT must always be float.
/// </summary>
template <typename T, typename bucketT>
class EMBERCL_API RendererCL : public Renderer<T, bucketT>, public RendererCLBase
{
	using EmberNs::Renderer<T, bucketT>::RendererBase::Abort;
	using EmberNs::Renderer<T, bucketT>::RendererBase::EarlyClip;
	using EmberNs::Renderer<T, bucketT>::RendererBase::EnterResize;
	using EmberNs::Renderer<T, bucketT>::RendererBase::LeaveResize;
	using EmberNs::Renderer<T, bucketT>::RendererBase::FinalRasW;
	using EmberNs::Renderer<T, bucketT>::RendererBase::FinalRasH;
	using EmberNs::Renderer<T, bucketT>::RendererBase::SuperRasW;
	using EmberNs::Renderer<T, bucketT>::RendererBase::SuperRasH;
	using EmberNs::Renderer<T, bucketT>::RendererBase::SuperSize;
	using EmberNs::Renderer<T, bucketT>::RendererBase::BytesPerChannel;
	using EmberNs::Renderer<T, bucketT>::RendererBase::TemporalSamples;
	using EmberNs::Renderer<T, bucketT>::RendererBase::ItersPerTemporalSample;
	using EmberNs::Renderer<T, bucketT>::RendererBase::FuseCount;
	using EmberNs::Renderer<T, bucketT>::RendererBase::DensityFilterOffset;
	using EmberNs::Renderer<T, bucketT>::RendererBase::PrepFinalAccumVector;
	using EmberNs::Renderer<T, bucketT>::RendererBase::Paused;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_ProgressParameter;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_YAxisUp;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_LockAccum;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_Abort;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_LastIter;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_LastIterPercent;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_Stats;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_Callback;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_Rand;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_RenderTimer;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_IterTimer;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_ProgressTimer;
	using EmberNs::Renderer<T, bucketT>::RendererBase::EmberReport::AddToReport;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_ResizeCs;
	using EmberNs::Renderer<T, bucketT>::RendererBase::m_ProcessAction;
	using EmberNs::Renderer<T, bucketT>::m_RotMat;
	using EmberNs::Renderer<T, bucketT>::m_Ember;
	using EmberNs::Renderer<T, bucketT>::m_Csa;
	using EmberNs::Renderer<T, bucketT>::m_CurvesSet;
	using EmberNs::Renderer<T, bucketT>::CenterX;
	using EmberNs::Renderer<T, bucketT>::CenterY;
	using EmberNs::Renderer<T, bucketT>::K1;
	using EmberNs::Renderer<T, bucketT>::K2;
	using EmberNs::Renderer<T, bucketT>::Supersample;
	using EmberNs::Renderer<T, bucketT>::HighlightPower;
	using EmberNs::Renderer<T, bucketT>::HistBuckets;
	using EmberNs::Renderer<T, bucketT>::AccumulatorBuckets;
	using EmberNs::Renderer<T, bucketT>::GetDensityFilter;
	using EmberNs::Renderer<T, bucketT>::GetSpatialFilter;
	using EmberNs::Renderer<T, bucketT>::CoordMap;
	using EmberNs::Renderer<T, bucketT>::XformDistributions;
	using EmberNs::Renderer<T, bucketT>::XformDistributionsSize;
	using EmberNs::Renderer<T, bucketT>::m_Dmap;
	using EmberNs::Renderer<T, bucketT>::m_DensityFilter;
	using EmberNs::Renderer<T, bucketT>::m_SpatialFilter;

public:
	RendererCL(const vector<pair<size_t, size_t>>& devices, bool shared = false, GLuint outputTexID = 0);
	RendererCL(const RendererCL<T, bucketT>& renderer) = delete;
	RendererCL<T, bucketT>& operator = (const RendererCL<T, bucketT>& renderer) = delete;
	virtual ~RendererCL() = default;

	//Non-virtual member functions for OpenCL specific tasks.
	bool Init(const vector<pair<size_t, size_t>>& devices, bool shared, GLuint outputTexID);
	bool SetOutputTexture(GLuint outputTexID);

	//Iters per kernel/block/grid.
	inline size_t IterCountPerKernel() const noexcept;
	inline size_t IterCountPerBlock() const noexcept;
	inline size_t IterCountPerGrid() const noexcept;

	//Kernels per block.
	inline size_t IterBlockKernelWidth() const noexcept;
	inline size_t IterBlockKernelHeight() const noexcept;
	inline size_t IterBlockKernelCount() const noexcept;

	//Kernels per grid.
	inline size_t IterGridKernelWidth() const noexcept;
	inline size_t IterGridKernelHeight() const noexcept;
	inline size_t IterGridKernelCount() const noexcept;

	//Blocks per grid.
	inline size_t IterGridBlockWidth() const noexcept;
	inline size_t IterGridBlockHeight() const noexcept;
	inline size_t IterGridBlockCount() const noexcept;

	//Allow for changing the number of blocks in each dimension of the grid.
	void IterBlocksWide(size_t w) noexcept;
	void IterBlocksHigh(size_t h) noexcept;

	bool ReadHist(size_t device);
	bool ReadAccum();
	bool ReadPoints(size_t device, vector<PointCL<T>>& vec);
	bool ClearHist();
	bool ClearHist(size_t device);
	bool ClearAccum();
	bool WritePoints(size_t device, vector<PointCL<T>>& vec);
#ifdef TEST_CL
	bool WriteRandomPoints(size_t device);
#endif
	void InitStateVec();
	void SubBatchPercentPerThread(float f);
	float SubBatchPercentPerThread() const;
	const string& IterKernel() const;
	const string& DEKernel() const;
	const string& FinalAccumKernel() const;

	//Access to underlying OpenCL structures. Use cautiously.
	const vector<unique_ptr<RendererClDevice>>& Devices() const;

	//Virtual functions overridden from RendererCLBase.
    virtual bool ReadFinal(v4F* pixels) override;
	virtual bool ClearFinal();

	//Public virtual functions overridden from Renderer or RendererBase.
	size_t MemoryAvailable() override;
	bool Ok() const override;
	size_t SubBatchSize() const override;
	size_t ThreadCount() const override;
	bool CreateDEFilter(bool& newAlloc) override;
	bool CreateSpatialFilter(bool& newAlloc) override;
	eRendererType RendererType() const override;
	bool Shared() const override;
	void ClearErrorReport() noexcept override;
	string ErrorReportString() override;
	vector<string> ErrorReport() override;
	bool RandVec(vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>>& randVec) override;
	bool AnyNvidia() const noexcept override;

#ifndef TEST_CL
protected:
#endif
	//Protected virtual functions overridden from Renderer.
	bool Alloc(bool histOnly = false) override;
	bool ResetBuckets(bool resetHist = true, bool resetAccum = true) override;
    eRenderStatus LogScaleDensityFilter() override;
	eRenderStatus GaussianDensityFilter() override;
	eRenderStatus AccumulatorToFinalImage(vector<v4F>& pixels, size_t finalOffset) override;
	EmberStats Iterate(size_t iterCount, size_t temporalSample) override;

#ifndef TEST_CL
private:
#endif
	//Private functions for making and running OpenCL programs.
	bool BuildIterProgramForEmber(bool doAccum = true);
	bool RunIter(size_t iterCount, size_t temporalSample, size_t& itersRan);
	eRenderStatus RunLogScaleFilter();
	eRenderStatus RunDensityFilter();
	eRenderStatus RunFinalAccum();
	bool ClearBuffer(size_t device, const string& bufferName, uint width, uint height, uint elementSize);
	bool RunDensityFilterPrivate(size_t kernelIndex, size_t gridW, size_t gridH, size_t blockW, size_t blockH, uint chunkSizeW, uint chunkSizeH, uint colChunkPass, uint rowChunkPass);
	int MakeAndGetDensityFilterProgram(size_t ss, uint filterWidth);
	int MakeAndGetFinalAccumProgram();
	int MakeAndGetGammaCorrectionProgram();
	bool CreateHostBuffer();
	bool SumDeviceHist();
	void FillSeeds();

	//Private functions passing data to OpenCL programs.
	void ConvertDensityFilter();
	void ConvertSpatialFilter();
	void ConvertEmber(Ember<T>& ember, EmberCL<T>& emberCL, vector<XformCL<T>>& xformsCL);
	void ConvertCarToRas(const CarToRas<T>& carToRas);
	std::string ErrorStr(const std::string& loc, const std::string& error, RendererClDevice* dev);
	bool m_Init = false;
	bool m_Shared = false;
	bool m_DoublePrecision = typeid(T) == typeid(double);
	float m_SubBatchPercentPerThread = 0.025f;//0.025 * 10,240 gives a default value of 256 iters per thread for the default sub batch size of 10,240 which almost all flames will use.
	//It's critical that these numbers never change. They are
	//based on the cuburn model of each kernel launch containing
	//256 threads. 32 wide by 8 high. Everything done in the OpenCL
	//iteraion kernel depends on these dimensions.
	size_t m_IterCountPerKernel = 256;
	size_t m_IterBlocksWide = 64, m_IterBlockWidth = 32;
	size_t m_IterBlocksHigh = 2, m_IterBlockHeight = 8;
	size_t m_MaxDEBlockSizeW;
	size_t m_MaxDEBlockSizeH;

	//Buffer names.
	string m_EmberBufferName = "Ember";
	string m_XformsBufferName = "Xforms";
	string m_ParVarsBufferName = "ParVars";
	string m_GlobalSharedBufferName = "GlobalShared";
	string m_SeedsBufferName = "Seeds";
	string m_DistBufferName = "Dist";
	string m_CarToRasBufferName = "CarToRas";
	string m_DEFilterParamsBufferName = "DEFilterParams";
	string m_SpatialFilterParamsBufferName = "SpatialFilterParams";
	string m_DECoefsBufferName = "DECoefs";
	string m_DEWidthsBufferName = "DEWidths";
	string m_DECoefIndicesBufferName = "DECoefIndices";
	string m_SpatialFilterCoefsBufferName = "SpatialFilterCoefs";
	string m_CurvesCsaName = "CurvesCsa";
	string m_HostBufferName = "Host";
	string m_HistBufferName = "Hist";
	string m_AccumBufferName = "Accum";
	string m_FinalImageName = "Final";
	string m_PointsBufferName = "Points";
#ifdef KNL_USE_GLOBAL_CONSEC
	string m_ConsecBufferName = "Consec";
#endif
	string m_VarStateBufferName = "VarState";

	//Kernels.
	string m_IterKernel;

	cl::ImageFormat m_PaletteFormat;
	cl::ImageFormat m_FinalFormat;
	cl::Image2D m_Palette;
	cl::ImageGL m_AccumImage;
	GLuint m_OutputTexID;
	EmberCL<T> m_EmberCL;
	vector<XformCL<T>> m_XformsCL;
	vector<vector<glm::highp_uvec2>> m_Seeds;
	CarToRasCL<T> m_CarToRasCL;
	DensityFilterCL<bucketT> m_DensityFilterCL;
	SpatialFilterCL<bucketT> m_SpatialFilterCL;
	IterOpenCLKernelCreator<T> m_IterOpenCLKernelCreator;
	DEOpenCLKernelCreator m_DEOpenCLKernelCreator;
	FinalAccumOpenCLKernelCreator m_FinalAccumOpenCLKernelCreator;
	pair<string, vector<T>> m_Params;
	pair<string, vector<T>> m_GlobalShared;
	vector<T> m_VarStates;
	vector<unique_ptr<RendererClDevice>> m_Devices;
	Ember<T> m_LastBuiltEmber;
};
}
