#include "EmberPch.h"
#include "Renderer.h"

namespace EmberNs
{
static void parallel_for(size_t start, size_t end, size_t parlevel, std::function<void(size_t)> func)
{
	const auto ct = parlevel == 0 ? EmberNs::Timing::ProcessorCount() : parlevel;
	std::vector<std::thread> threads(ct);
	const auto chunkSize = (end - start) / ct;

	for (size_t i = 0; i < ct; i++)
	{
		threads.push_back(std::thread([&](size_t _i, size_t _ct)
									  {
										  const auto chunkStart = chunkSize * _i;
										  const auto chunkEnd = _i == _ct - 1 ? end : std::min(chunkStart + chunkSize, end);

										  for (size_t j = chunkStart; j < chunkEnd; j++)
											  func(j);
									  }, i, ct));
	}

	EmberNs::Join(threads);
}

/// <summary>
/// Constructor that allocates various pieces of memory.
/// </summary>
template <typename T, typename bucketT>
Renderer<T, bucketT>::Renderer()
{
	//Use a very large number regardless of the size of the output pixels. This should be sufficient granularity, even though
	//it's technically less than the number of distinct values representable by a 32-bit float.
	m_Csa.resize(static_cast<size_t>(CURVES_LENGTH));
	//Ensure the renderer at least has sane values for the camera upon startup.
	//This is needed because due to timing/threading disconnects, the GUI can use the camera
	//values before the render has started, which will lead to corrupt values.
	Ember<T> ember;
	SetEmber(ember, eProcessAction::NOTHING, false);
	//Manually call these instead of passing true to SetEmber() because it would have created the spatial filter
	//which we don't want to do until rendering starts (this is so the derived RendererCL can properly create the needed buffers).
	ComputeBounds();
	ComputeQuality();
	ComputeCamera();
}

/// <summary>
/// Non-virtual processing functions.
/// </summary>

/// <summary>
/// Add an ember to the end of the embers vector and reset the rendering process.
/// Reset the rendering process.
/// </summary>
/// <param name="ember">The ember to add</param>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::AddEmber(Ember<T>& ember)
{
	ChangeVal([&]
	{
		m_Embers.push_back(ember);

		if (m_Embers.size() == 1)
			m_Ember = m_Embers[0];
	}, eProcessAction::FULL_RENDER);
	Prepare();
}

/// <summary>
/// Set the m_Iterator member to point to the appropriate
/// iterator based on whether the ember currently being rendered
/// contains xaos.
/// After assigning, initialize the xform selection buffer.
/// </summary>
/// <returns>True if assignment and distribution initialization succeeded, else false.</returns>
template <typename T, typename bucketT>
bool Renderer<T, bucketT>::AssignIterator()
{
	//Setup iterator and distributions.
	//Both iterator types were setup in the constructor (add more in the future if needed).
	//So simply assign the pointer to the correct type and re-initialize its distributions
	//based on the current ember.
	if (XaosPresent())
		m_Iterator = m_XaosIterator.get();
	else
		m_Iterator = m_StandardIterator.get();

	//Timing t;
	return m_Iterator->InitDistributions(m_Ember);
	//t.Toc("Distrib creation");
}

/// <summary>
/// Virtual processing functions overriden from RendererBase.
/// </summary>

/// <summary>
/// Prepare values for the filters, bounds, quality and camera.
/// </summary>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::Prepare()
{
	bool b = false;
	CreateSpatialFilter(b);
	CreateTemporalFilter(b);
	ComputeBounds();
	ComputeQuality();
	ComputeCamera();
	m_CarToRas.UpdateCachedHalf(m_CarToRas.CarHalfX(), m_CarToRas.CarHalfY());
}

/// <summary>
/// Compute the bounds of the histogram and density filtering buffers.
/// These are affected by the final requested dimensions, spatial and density
/// filter sizes and supersampling.
/// </summary>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::ComputeBounds()
{
	//Original did a lot of work to compute a gutter that changes size based on various parameters, which seems to be of no benefit.
	//It also prevents the renderer from only performing filtering or final accum based on a filter parameter change, since that
	//change may have changed the gutter.
	//By using a fixed gutter, a filter change can be applied without fully restarting iteration.
	m_GutterWidth = 10 * Supersample();//Should be enough to fully accommodate most spatial and density filter widths.
	m_SuperRasW = (Supersample() * FinalRasW()) + (2 * m_GutterWidth);
	m_SuperRasH = (Supersample() * FinalRasH()) + (2 * m_GutterWidth);
	m_SuperSize = m_SuperRasW * m_SuperRasH;
}

/// <summary>
/// Compute the scale based on the zoom, then the quality based on the computed scale.
/// This must be called before ComputeCamera() which will use scale.
/// </summary>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::ComputeQuality()
{
	m_Scale = std::pow(static_cast<T>(2), Zoom());
	m_ScaledQuality = Quality() * SQR(m_Scale);
}

/// <summary>
/// Compute the camera.
/// This sets up the bounds of the cartesian plane that the raster bounds correspond to.
/// This must be called after ComputeBounds() which sets up the raster bounds.
/// </summary>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::ComputeCamera()
{
	m_PixelsPerUnitX = PixelsPerUnit() * m_Scale;
	m_PixelsPerUnitY = m_PixelsPerUnitX;
	m_PixelsPerUnitX /= PixelAspectRatio();
	T shift = 0;
	T t0 = static_cast<T>(m_GutterWidth) / (Supersample() * m_PixelsPerUnitX);
	T t1 = static_cast<T>(m_GutterWidth) / (Supersample() * m_PixelsPerUnitY);
	//These go from ll to ur, moving from negative to positive.
	m_LowerLeftX = CenterX() - FinalRasW() / m_PixelsPerUnitX / static_cast<T>(2);
	m_LowerLeftY = CenterY() - FinalRasH() / m_PixelsPerUnitY / static_cast<T>(2);
	m_UpperRightX = m_LowerLeftX + FinalRasW() / m_PixelsPerUnitX;
	m_UpperRightY = m_LowerLeftY + FinalRasH() / m_PixelsPerUnitY;
	T carLlX = m_LowerLeftX - t0;
	T carLlY = m_LowerLeftY - t1 + shift;
	T carUrX = m_UpperRightX + t0;
	T carUrY = m_UpperRightY + t1 + shift;
	m_RotMat.MakeID();
	m_RotMat.Rotate(-Rotate() * DEG_2_RAD_T);
	m_CarToRas.Init(carLlX, carLlY, carUrX, carUrY, m_SuperRasW, m_SuperRasH);
}

/// <summary>
/// Set the current ember.
/// This will also populate the vector of embers with a single element copy
/// of the ember passed in.
/// Temporal samples will be set to 1 since there's only a single ember.
/// </summary>
/// <param name="ember">The ember to assign</param>
/// <param name="action">The requested process action. Note that it's critical the user supply the proper value here.
/// For example: Changing dimensions without setting action to eProcessAction::FULL_RENDER will crash the program.
/// However, changing only the brightness and setting action to ACCUM_ONLY is perfectly fine.
/// <param name="prep">Whether to also compute bounds, camera, filters etc. This is useful when other code outside of this needs these values
/// before the render actually starts. Default: false.</param>
/// </param>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::SetEmber(const Ember<T>& ember, eProcessAction action, bool prep)
{
	ChangeVal([&]
	{
		m_Embers.clear();
		m_Embers.push_back(ember);
		m_Embers[0].m_TemporalSamples = 1;//Set temporal samples here to 1 because using the real value only makes sense when using a vector of Embers for animation.
		m_Ember = m_Embers[0];
		m_EmbersP = &m_Embers;
	}, action);

	if (prep)
		Prepare();
}

/// <summary>
/// Copy the embers in the passed in container to the internal vector of embers
/// and set the m_Ember member to a copy of the first element.
/// Reset the rendering process.
/// </summary>
/// <param name="embers">The container of embers to be copied</param>
template <typename T, typename bucketT>
template <typename C>
void Renderer<T, bucketT>::SetEmber(const C& embers)
{
	ChangeVal([&]
	{
		CopyCont(m_Embers, embers);
		m_EmbersP = &m_Embers;

		if (!m_Embers.empty())
			m_Ember = m_Embers[0];

	}, eProcessAction::FULL_RENDER);
	Prepare();//Always prepare with a collection.
}

/// <summary>
/// Move the embers in the passed in vector to the internal vector of embers
/// and set the m_Ember member to a copy of the first element.
/// Reset the rendering process.
/// This is preferred over SetEmber when the size of embers is large and/or
/// the caller no longer needs to use the argument after this function returns.
/// </summary>
/// <param name="embers">The vector of embers to be moved</param>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::MoveEmbers(vector<Ember<T>>& embers)
{
	ChangeVal([&]
	{
		m_Embers = std::move(embers);
		m_EmbersP = &m_Embers;

		if (!m_Embers.empty())
			m_Ember = m_Embers[0];

	}, eProcessAction::FULL_RENDER);
	Prepare();
}

template <typename T, typename bucketT>
void Renderer<T, bucketT>::SetExternalEmbersPointer(vector<Ember<T>>* embers)
{
	ChangeVal([&]
	{
		m_Embers.clear();
		m_EmbersP = embers;

		if (!m_EmbersP->empty())
			m_Ember = (*m_EmbersP)[0];

	}, eProcessAction::FULL_RENDER);
	Prepare();
}

/// <summary>
/// Create the density filter if the current filter parameters differ
/// from the last density filter created.
/// The filter will be deleted if the max DE radius is 0, in which case regular
/// log scale filtering will be used.
/// </summary>
/// <param name="newAlloc">True if a new filter instance was created, else false.</param>
/// <returns>True if the filter is not nullptr (whether a new one was created or not) or if max rad is 0, else false.</returns>
template <typename T, typename bucketT>
bool Renderer<T, bucketT>::CreateDEFilter(bool& newAlloc)
{
	//If they wanted DE, create it if needed, else clear the last DE filter which means we'll do regular log filtering after iters are done.
	newAlloc = false;

	if (m_Ember.m_MaxRadDE > 0)
	{
		//Use intelligent testing so it isn't created every time a new ember is passed in.
		if ((!m_DensityFilter.get()) ||
				(m_Ember.m_MinRadDE != m_DensityFilter->MinRad()) ||
				(m_Ember.m_MaxRadDE != m_DensityFilter->MaxRad()) ||
				(m_Ember.m_CurveDE != m_DensityFilter->Curve()) ||
				(m_Ember.m_Supersample != m_DensityFilter->Supersample()))
		{
			m_DensityFilter = make_unique<DensityFilter<bucketT>>(static_cast<bucketT>(m_Ember.m_MinRadDE), static_cast<bucketT>(m_Ember.m_MaxRadDE),
							  static_cast<bucketT>(m_Ember.m_CurveDE), m_Ember.m_Supersample);
			newAlloc = true;
		}

		if (newAlloc)
		{
			if (!m_DensityFilter.get()) { return false; }//Did object creation succeed?

			if (!m_DensityFilter->Create()) { return false; }//Object creation succeeded, did filter creation succeed?
		}
		else if (!m_DensityFilter->Valid()) { return false; } //Previously created, are values ok?
	}
	else
	{
		m_DensityFilter.reset();//They want to do log filtering. Return true because even though the filter is being deleted, nothing went wrong.
	}

	return true;
}

/// <summary>
/// Create the spatial filter if the current filter parameters differ
/// from the last spatial filter created.
/// </summary>
/// <param name="newAlloc">True if a new filter instance was created, else false.</param>
/// <returns>True if the filter is not nullptr (whether a new one was created or not), else false.</returns>
template <typename T, typename bucketT>
bool Renderer<T, bucketT>::CreateSpatialFilter(bool& newAlloc)
{
	newAlloc = false;

	//Use intelligent testing so it isn't created every time a new ember is passed in.
	if ((!m_SpatialFilter.get()) ||
			(m_Ember.m_SpatialFilterType != m_SpatialFilter->FilterType()) ||
			(m_Ember.m_SpatialFilterRadius != m_SpatialFilter->FilterRadius()) ||
			(m_Ember.m_Supersample != m_SpatialFilter->Supersample()) ||
			(m_PixelAspectRatio != m_SpatialFilter->PixelAspectRatio()))
	{
		m_SpatialFilter = unique_ptr<SpatialFilter<bucketT>>(
							  SpatialFilterCreator<bucketT>::Create(m_Ember.m_SpatialFilterType,
									  static_cast<bucketT>(m_Ember.m_SpatialFilterRadius), m_Ember.m_Supersample, static_cast<bucketT>(m_PixelAspectRatio)));
		m_Ember.m_SpatialFilterRadius = m_SpatialFilter->FilterRadius();//It may have been changed internally if it was too small, so ensure they're synced.
		newAlloc = true;
	}

	return m_SpatialFilter.get() != nullptr;
}

/// <summary>
/// Create the temporal filter if the current filter parameters differ
/// from the last temporal filter created.
/// </summary>
/// <param name="newAlloc">True if a new filter instance was created, else false.</param>
/// <returns>True if the filter is not nullptr (whether a new one was created or not), else false.</returns>
template <typename T, typename bucketT>
bool Renderer<T, bucketT>::CreateTemporalFilter(bool& newAlloc)
{
	newAlloc = false;
	//static int i = 0;

	//Use intelligent testing so it isn't created every time a new ember is passed in.
	if ((!m_TemporalFilter.get()) ||
			(m_Ember.m_TemporalSamples != m_TemporalFilter->TemporalSamples()) ||
			(m_Ember.m_TemporalFilterType != m_TemporalFilter->FilterType()) ||
			(m_Ember.m_TemporalFilterWidth != m_TemporalFilter->FilterWidth()) ||
			(m_Ember.m_TemporalFilterExp != m_TemporalFilter->FilterExp()))
	{
		m_TemporalFilter = unique_ptr<TemporalFilter<T>>(
							   TemporalFilterCreator<T>::Create(m_Ember.m_TemporalFilterType, m_Ember.m_TemporalSamples, m_Ember.m_TemporalFilterWidth, m_Ember.m_TemporalFilterExp));
		newAlloc = true;
		//auto name = TemporalFilterCreator<T>::ToString(m_TemporalFilter->FilterType());
		//ostringstream os;
		//os << "./" << ++i << "_" << name << "_filter.txt";
		//ofstream of (os.str());
		//auto str = m_TemporalFilter->ToString();
		//
		//if (of.is_open())
		//	of << str;
	}

	return m_TemporalFilter.get() != nullptr;
}

/// <summary>
/// The main render loop. This is the core of the algorithm.
/// The processing steps are: Iterating, density filtering, final accumulation.
/// Various functions in it are virtual so they will resolve
/// to whatever overrides are provided in derived classes. This
/// future-proofs the algorithm for GPU-based renderers.
/// If the caller calls Abort() at any time, or the progress function returns 0,
/// the entire rendering process will exit as soon as it can.
/// The concept of passes from flam3 has been removed as it was never used.
/// The loop structure is:
/// {
///		Temporal Samples (Default 1 for single image)
///		{
///			Iterate (Either to completion or to a specified number of iterations)
///			{
///			}
///		}
///
///		Density filtering (Basic log, or full density estimation)
///		Final accumulation (Color correction and spatial filtering)
/// }
/// This loop structure has admittedly been severely butchered from what
/// flam3 did. The reason is that it was made to support interactive rendering
/// that can exit the process and pick up where it left off in response to the
/// user changing values in a fractal flame GUI editor.
/// To achieve this, each step in the rendering process is given an enumeration state
/// as well as a goto label. This allows the renderer to pick up in the state it left
/// off in if no changes prohibiting that have been made.
/// It also allows for the bare minimum amount of processing needed to complete the requested
/// action. For example, if the process has completed and the user only adjusts the brightness
/// of the last rendered ember then there is no need to perform the entire iteration process
/// over again. Rather, only final accumulation is needed.
/// </summary>
/// <param name="finalImage">Storage for the final image. It will be allocated if needed.</param>
/// <param name="time">The time if animating, else ignored.</param>
/// <param name="subBatchCountOverride">Run a specified number of sub batches. Default: 0, meaning run to completion.</param>
/// <param name="forceOutput">True to force rendering a complete image even if iterating is not complete, else don't. Default: false.</param>
/// <param name="finalOffset">Offset in finalImage to store the pixels to. Default: 0.</param>
/// <returns>True if nothing went wrong, else false.</returns>
template <typename T, typename bucketT>
eRenderStatus Renderer<T, bucketT>::Run(vector<v4F>& finalImage, double time, size_t subBatchCountOverride, bool forceOutput, size_t finalOffset)
{
	m_RenderTimer.Tic();
	m_InRender = true;
	EnterRender();
	m_Abort = false;
	bool filterAndAccumOnly = m_ProcessAction == eProcessAction::FILTER_AND_ACCUM;
	bool accumOnly = m_ProcessAction == eProcessAction::ACCUM_ONLY;
	bool resume = m_ProcessState != eProcessState::NONE;
	bool newFilterAlloc;
	size_t temporalSample = 0;
	T deTime;
	auto success = eRenderStatus::RENDER_OK;

	//Reset timers and progress percent if: Beginning anew or only filtering and/or accumulating.
	if (!resume || accumOnly || filterAndAccumOnly)
	{
		if (!resume)//Only set this if it's the first run through.
			m_ProcessState = eProcessState::ITER_STARTED;

		m_ProgressTimer.Tic();
	}

	if (!resume)//Beginning, reset everything.
	{
		m_LastTemporalSample = 0;
		m_LastIter = 0;
		m_LastIterPercent = 0;
		m_Stats.Clear();
		m_Gamma = 0;
		m_Vibrancy = 0;//Accumulate these after each temporal sample.
		m_VibGamCount = 0;
		m_CurvesSet = false;
		m_Background.Clear();
	}
	//User requested an increase in quality after finishing.
	else if (m_ProcessState == eProcessState::ITER_STARTED && m_ProcessAction == eProcessAction::KEEP_ITERATING && TemporalSamples() == 1)
	{
		m_LastTemporalSample = 0;
		m_LastIter = m_Stats.m_Iters;
		m_LastIterPercent = 0;//Might skip a progress update, but shouldn't matter.
		m_Gamma = 0;
		m_Vibrancy = 0;
		m_VibGamCount = 0;
		m_Background.Clear();
		ComputeQuality();//Must recompute quality when doing a quality increase.
	}

	//Make sure values are within valid range.
	ClampGteRef(m_Ember.m_Supersample, static_cast<size_t>(1));

	//Make sure to get most recent update since loop won't be entered to call Interp().
	//Vib, gam and background are normally summed for each temporal sample. However if iteration is skipped, make sure to get the latest.
	if ((filterAndAccumOnly || accumOnly) && TemporalSamples() == 1)//Disallow jumping when temporal samples > 1.
	{
		m_Ember = (*m_EmbersP)[0];
		m_Vibrancy = Vibrancy();
		m_Gamma = Gamma();
		m_Background = m_Ember.m_Background;

		if (filterAndAccumOnly)
			goto FilterAndAccum;

		if (accumOnly)
			goto AccumOnly;
	}

	//it.Tic();
	//Interpolate.
	if (m_EmbersP->size() > 1)
		m_Interpolater.Interpolate(*m_EmbersP, static_cast<T>(time), 0, m_Ember);

	//it.Toc("Interp 1");

	//Save only for palette insertion.
	if (m_InsertPalette)
		m_TempEmber = m_Ember;

	if (!resume)//Only need to create this when starting a new render.
	{
		CreateSpatialFilter(newFilterAlloc);//Will be checked and recreated again if necessary right before final output.
		CreateTemporalFilter(newFilterAlloc);//But create here just to ensure allocation succeeded.
		ComputeBounds();
	}

	if (m_SpatialFilter.get() == nullptr || m_TemporalFilter.get() == nullptr)
	{
		AddToReport("Spatial and temporal filter allocations failed, aborting.\n");
		success = eRenderStatus::RENDER_ERROR;
		goto Finish;
	}

	if (!resume && !Alloc())
	{
		AddToReport("Histogram, accumulator and samples buffer allocations failed, aborting.\n");
		success = eRenderStatus::RENDER_ERROR;
		goto Finish;
	}

	if (!resume)
	{
		if (!ResetBuckets(true, false))//Only reset hist here and do accum when needed later on.
		{
			success = eRenderStatus::RENDER_ERROR;
			goto Finish;
		}
	}

	deTime = static_cast<T>(time) + *m_TemporalFilter->Deltas();

	//Interpolate and get an ember for DE purposes.
	//Additional interpolation will be done in the temporal samples loop.
	//it.Tic();
	if (m_EmbersP->size() > 1)
		m_Interpolater.Interpolate(*m_EmbersP, deTime, 0, m_Ember);

	//it.Toc("Interp 2");
	ClampGteRef<T>(m_Ember.m_MinRadDE, 0);
	ClampGteRef<T>(m_Ember.m_MaxRadDE, 0);
	ClampGteRef<T>(m_Ember.m_MaxRadDE, m_Ember.m_MinRadDE);

	if (!CreateDEFilter(newFilterAlloc))//Will be checked and recreated again if necessary right before density filtering.
	{
		AddToReport("Density filter creation failed, aborting.\n");
		success = eRenderStatus::RENDER_ERROR;
		goto Finish;
	}

	//Temporal samples, loop 1.
	temporalSample = resume ? m_LastTemporalSample : 0;

	for (; (temporalSample < TemporalSamples()) && !m_Abort;)
	{
		T colorScalar = m_TemporalFilter->Filter()[temporalSample];
		T temporalTime = static_cast<T>(time) + m_TemporalFilter->Deltas()[temporalSample];

		//Interpolate again.
		//it.Tic();
		if (TemporalSamples() > 1 && m_EmbersP->size() > 1)
			m_Interpolater.Interpolate(*m_EmbersP, temporalTime, 0, m_Ember);//This will perform all necessary precalcs via the ember/xform/variation assignment operators.

		//it.Toc("Interp 3");

		if (!resume && !AssignIterator())
		{
			AddToReport("Iterator assignment failed, aborting.\n");
			success = eRenderStatus::RENDER_ERROR;
			goto Finish;
		}

		//Do this every iteration for an animation, or else do it once for a single image.
		if (TemporalSamples() > 1 || !resume)
		{
			ComputeQuality();
			ComputeCamera();
			//m_CarToRas.UpdateCachedHalf(m_CarToRas.CarHalfX(), m_CarToRas.CarHalfY());
			MakeDmap(colorScalar);//For each temporal sample, the palette m_Dmap needs to be re-created with color scalar. 1 if no temporal samples.
		}

		//The actual number of times to iterate. Each thread will get (totalIters / ThreadCount) iters to do.
		//This is based on zoom and scale calculated in ComputeQuality().
		//Note that the iter count is based on the final image dimensions, and not the super sampled dimensions.
		size_t itersPerTemporalSample = ItersPerTemporalSample();//The total number of iterations for this temporal sample without overrides.
		size_t sampleItersToDo;//The number of iterations to actually do in this sample, considering overrides.

		if (subBatchCountOverride > 0)
			sampleItersToDo = subBatchCountOverride * SubBatchSize() * ThreadCount();//Run a specific number of sub batches.
		else
			sampleItersToDo = itersPerTemporalSample;//Run as many iters as specified to complete this temporal sample.

		sampleItersToDo = std::min<size_t>(sampleItersToDo, itersPerTemporalSample - m_LastIter);
		EmberStats stats = Iterate(sampleItersToDo, temporalSample);//The heavy work is done here.

		//Abort does not indicate an error, it just means the process was interrupted, most likely by the user on the GUI.
		if (m_Abort)
		{
			success = eRenderStatus::RENDER_ABORT;
			goto Finish;
		}

		//If no iters were executed, something went catastrophically wrong.
		if (!stats.m_Success && stats.m_Iters == 0)
		{
			AddToReport("Zero iterations ran, rendering failed, aborting.\n");
			success = eRenderStatus::RENDER_ERROR;
			Abort();
			goto Finish;
		}

		//Accumulate stats whether this batch ran to completion or exited prematurely.
		m_LastIter += stats.m_Iters;//Sum of iter count of all threads, reset each temporal sample.
		m_Stats.m_Iters += stats.m_Iters;//Sum of iter count of all threads, cumulative from beginning to end.
		m_Stats.m_Badvals += stats.m_Badvals;
		m_Stats.m_IterMs += stats.m_IterMs;

		//After each temporal sample, accumulate these.
		//Allow for incremental rendering by only taking action if the iter loop for this temporal sample is completely done.
		if (m_LastIter >= itersPerTemporalSample)
		{
			m_Vibrancy += Vibrancy();
			m_Gamma += Gamma();
			m_Background.r += static_cast<bucketT>(m_Ember.m_Background.r);
			m_Background.g += static_cast<bucketT>(m_Ember.m_Background.g);
			m_Background.b += static_cast<bucketT>(m_Ember.m_Background.b);
			m_VibGamCount++;
			m_LastIter = 0;
			temporalSample++;
		}

		m_LastTemporalSample = temporalSample;

		if (subBatchCountOverride > 0)//Don't keep going through this loop if only doing an incremental render.
			break;
	}//Temporal samples.

	//If we've completed all temporal samples, then it was a complete render, so report progress.
	if (temporalSample >= TemporalSamples())
	{
		m_ProcessState = eProcessState::ITER_DONE;

		if (m_Callback && !m_Callback->ProgressFunc(m_Ember, m_ProgressParameter, 100.0, 0, 0))
		{
			Abort();
			success = eRenderStatus::RENDER_ABORT;
			goto Finish;
		}
	}

FilterAndAccum:

	if (filterAndAccumOnly || temporalSample >= TemporalSamples() || forceOutput)
	{
		//t.Toc("Iterating and accumulating");
		//Compute k1 and k2.
		auto fullRun = eRenderStatus::RENDER_OK;//Whether density filtering was run to completion without aborting prematurely or triggering an error.
		T area = FinalRasW() * FinalRasH() / (m_PixelsPerUnitX * m_PixelsPerUnitY);//Need to use temps from field if ever implemented.
		m_K1 = Brightness();

		if (!m_Ember.m_K2 || forceOutput)
		{
			//When doing an interactive render, force output early on in the render process, before all iterations are done.
			//This presents a problem with the normal calculation of K2 since it relies on the quality value; it will scale the colors
			//to be very dark. Correct it by pretending the number of iters done is the exact quality desired and then scale according to that.
			if (forceOutput)
			{
				T quality = (static_cast<T>(m_Stats.m_Iters) / static_cast<T>(FinalDimensions())) * (m_Scale * m_Scale);
				m_K2 = static_cast<bucketT>((Supersample() * Supersample()) / (area * quality * m_TemporalFilter->SumFilt()));
			}
			else
				m_K2 = static_cast<bucketT>((Supersample() * Supersample()) / (area * m_ScaledQuality * m_TemporalFilter->SumFilt()));
		}
		else
			m_K2 = static_cast<bucketT>(m_Ember.m_K2);

		if (!ResetBuckets(false, true))//Only the histogram was reset above, now reset the density filtering buffer.
		{
			success = eRenderStatus::RENDER_ERROR;
			goto Finish;
		}

		//t.Tic();
		//Make sure a density filter was created with the latest values.
		ClampGteRef<T>(m_Ember.m_MinRadDE, 0);
		ClampGteRef<T>(m_Ember.m_MaxRadDE, 0);
		ClampGteRef<T>(m_Ember.m_MaxRadDE, m_Ember.m_MinRadDE);
		CreateDEFilter(newFilterAlloc);

		//Apply appropriate filter if iterating is complete.
		if (filterAndAccumOnly || temporalSample >= TemporalSamples())
		{
			fullRun = m_DensityFilter.get() ? GaussianDensityFilter() : LogScaleDensityFilter(forceOutput);
		}
		else
		{
			//Apply requested filter for a forced output during interactive rendering.
			if (m_DensityFilter.get() && m_InteractiveFilter == eInteractiveFilter::FILTER_DE)
				fullRun = GaussianDensityFilter();
			else if (!m_DensityFilter.get() || m_InteractiveFilter == eInteractiveFilter::FILTER_LOG)
				fullRun = LogScaleDensityFilter(forceOutput);
		}

		//Only update state if iterating and filtering finished completely (didn't arrive here via forceOutput).
		if (fullRun == eRenderStatus::RENDER_OK && m_ProcessState == eProcessState::ITER_DONE)
			m_ProcessState = eProcessState::FILTER_DONE;

		//Take special action if filtering exited prematurely.
		if (fullRun != eRenderStatus::RENDER_OK)
		{
			if (!ResetBuckets(false, true))//Reset the accumulator, come back and try again on the next call.
				success = eRenderStatus::RENDER_ERROR;
			else
				success = fullRun;

			goto Finish;
		}

		if (m_Abort)
		{
			success = eRenderStatus::RENDER_ABORT;
			goto Finish;
		}

		//t.Toc("Density estimation filtering time: ", true);
	}

AccumOnly:

	if (m_ProcessState == eProcessState::FILTER_DONE || forceOutput)
	{
		//Original only allowed stages 0 and 1. Add 2 to mean final accum.
		//Do not update state/progress on forced output because it will be immediately overwritten.
		if (m_Callback && !forceOutput && !m_Callback->ProgressFunc(m_Ember, m_ProgressParameter, 0, 2, 0))
		{
			Abort();
			success = eRenderStatus::RENDER_ABORT;
			goto Finish;
		}

		//Make sure a filter has been created.
		CreateSpatialFilter(newFilterAlloc);
		m_DensityFilterOffset = m_GutterWidth - static_cast<size_t>(Clamp<T>((static_cast<T>(m_SpatialFilter->FinalFilterWidth()) - static_cast<T>(Supersample())) / 2, 0, static_cast<T>(m_GutterWidth)));
		m_CurvesSet = m_Ember.m_Curves.CurvesSet();
		ComputeCurves();//Color curves must be re-calculated as well.

		if (AccumulatorToFinalImage(finalImage, finalOffset) == eRenderStatus::RENDER_OK)
		{
			//Even though the ember changes throughought the inner loops because of interpolation, it's probably ok to assign here.
			//This will hold the last interpolated value (even though spatial and temporal filters were created based off of one of the first interpolated values).
			m_LastEmber = m_Ember;

			if (m_ProcessState == eProcessState::FILTER_DONE)//Only update state if gotten here legitimately, and not via forceOutput.
			{
				m_ProcessState = eProcessState::ACCUM_DONE;

				if (m_Callback && !m_Callback->ProgressFunc(m_Ember, m_ProgressParameter, 100.0, 2, 0))//Finished.
				{
					Abort();
					success = eRenderStatus::RENDER_ABORT;
					goto Finish;
				}
			}
		}
		else
		{
			success = eRenderStatus::RENDER_ERROR;
		}
	}

Finish:

	if (success == eRenderStatus::RENDER_OK && m_Abort)//If everything ran ok, but they've aborted, record abort as the status.
		success = eRenderStatus::RENDER_ABORT;
	else if (success != eRenderStatus::RENDER_OK)//Regardless of abort status, if there was an error, leave that as the return status.
		Abort();

	LeaveRender();
	m_InRender = false;
	m_Stats.m_RenderMs += m_RenderTimer.Toc();//Record total time from the very beginning to the very end, including all intermediate calls.
	return success;
}

/// <summary>
/// Return EmberImageComments object with image comments filled out.
/// Run() should have completed before calling this.
/// </summary>
/// <param name="printEditDepth">The depth of the edit tags</param>
/// <param name="hexPalette">If true, embed a hexadecimal palette instead of Xml Color tags, else use Xml color tags.</param>
/// <returns>The EmberImageComments object with image comments filled out</returns>
template <typename T, typename bucketT>
EmberImageComments Renderer<T, bucketT>::ImageComments(const EmberStats& stats, size_t printEditDepth, bool hexPalette)
{
	ostringstream ss;
	EmberImageComments comments;
	ss.imbue(std::locale(""));
	comments.m_Genome = m_EmberToXml.ToString(m_Ember, "", printEditDepth, false, hexPalette);
	ss << (static_cast<double>(stats.m_Badvals) / static_cast<double>(stats.m_Iters));//Percentage of bad values to iters.
	comments.m_Badvals = ss.str(); ss.str("");
	ss << stats.m_Iters;
	comments.m_NumIters = ss.str(); ss.str("");//Total iters.
	ss << (stats.m_RenderMs / 1000.0);
	comments.m_Runtime = ss.str();//Number of seconds for iterating, accumulating and filtering.
	return comments;
}

/// <summary>
/// New virtual functions to be overridden in derived renderers that use the GPU, but not accessed outside.
/// </summary>

/// <summary>
/// Make the final palette used for iteration.
/// </summary>
/// <param name="colorScalar">The color scalar to multiply the ember's palette by</param>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::MakeDmap(T colorScalar)
{
	m_Ember.m_Palette.template MakeDmap<bucketT>(m_Dmap, static_cast<bucketT>(colorScalar));
}

/// <summary>
/// Allocate various buffers if the image dimensions, thread count, or sub batch size
/// has changed.
/// </summary>
/// <returns>True if success, else false</returns>
template <typename T, typename bucketT>
bool Renderer<T, bucketT>::Alloc(bool histOnly)
{
	auto b = true;
	const auto lock =
		(m_SuperSize         != m_HistBuckets.size())        ||
		(m_SuperSize         != m_AccumulatorBuckets.size()) ||
		(m_ThreadsToUse      != m_Samples.size())            ||
		(m_Samples[0].size() != SubBatchSize());

	if (lock)
		EnterResize();

	if (m_SuperSize != m_HistBuckets.size())
	{
		m_HistBuckets.resize(m_SuperSize);

		if (m_ReclaimOnResize)
			m_HistBuckets.shrink_to_fit();

		b &= (m_HistBuckets.size() == m_SuperSize);
	}

	if (histOnly)
	{
		if (lock)
			LeaveResize();

		return b;
	}

	if (m_SuperSize != m_AccumulatorBuckets.size())
	{
		m_AccumulatorBuckets.resize(m_SuperSize);

		if (m_ReclaimOnResize)
			m_AccumulatorBuckets.shrink_to_fit();

		b &= (m_AccumulatorBuckets.size() == m_SuperSize);
	}

	if (m_ThreadsToUse != m_Samples.size())
	{
		m_Samples.resize(m_ThreadsToUse);

		if (m_ReclaimOnResize)
			m_Samples.shrink_to_fit();

		b &= (m_Samples.size() == m_ThreadsToUse);
	}

	for (auto& sample : m_Samples)
	{
		if (sample.size() != SubBatchSize())
		{
			sample.resize(SubBatchSize());

			if (m_ReclaimOnResize)
				sample.shrink_to_fit();

			b &= (sample.size() == SubBatchSize());
		}
	}

	if (!m_StandardIterator.get())
		m_StandardIterator = make_unique<StandardIterator<T>>();

	if (!m_XaosIterator.get())
		m_XaosIterator = make_unique<XaosIterator<T>>();

	if (lock)
		LeaveResize();

	return b;
}

/// <summary>
/// Clear histogram and/or density filtering buffers to all zeroes.
/// </summary>
/// <param name="resetHist">Clear histogram if true, else don't.</param>
/// <param name="resetAccum">Clear density filtering buffer if true, else don't.</param>
/// <returns>True if anything was cleared, else false.</returns>
template <typename T, typename bucketT>
bool Renderer<T, bucketT>::ResetBuckets(bool resetHist, bool resetAccum)
{
	if (resetHist && !m_HistBuckets.empty())
		Memset(m_HistBuckets);

	if (resetAccum && !m_AccumulatorBuckets.empty())
		Memset(m_AccumulatorBuckets);

	return resetHist || resetAccum;
}

/// <summary>
/// THIS IS UNUSED.
/// Log scales a single row with a specially structured loop that will be vectorized by the compiler.
/// Note this adds an epsilon to the denomiator used to compute the logScale
/// value because the conditional check for zero would have prevented the loop from
/// being vectorized.
/// </summary>
/// <param name="row">The absolute element index in the histogram this row starts on</param>
/// <param name="rowEnd">The absolute element index in the histogram this row ends on</param>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::VectorizedLogScale(size_t row, size_t rowEnd)
{
	const auto k1 = static_cast<float>(m_K1);//All types must be float.
	const auto k2 = static_cast<float>(m_K2);
	auto* __restrict hist = m_HistBuckets.data();//Vectorizer can't tell these point to different locations.
	auto* __restrict acc = m_AccumulatorBuckets.data();

	for (size_t i = row; i < rowEnd; i++)
	{
		const float logScale = (k1 * std::log(1.0f + hist[i].a * k2)) / (hist[i].a + std::numeric_limits<float>::epsilon());
		acc[i].r = hist[i].r * logScale;//Must break these out individually. Vectorizer can't reason about vec4's overloaded * operator.
		acc[i].g = hist[i].g * logScale;
		acc[i].b = hist[i].b * logScale;
		acc[i].a = hist[i].a * logScale;
	}
}

/// <summary>
/// Perform log scale density filtering.
/// Base case for simple log scale density estimation as discussed (mostly) in the paper
/// in section 4, p. 6-9.
/// </summary>
/// <param name="forceOutput">Whether this output was forced due to an interactive render</param>
/// <returns>True if not prematurely aborted, else false.</returns>
template <typename T, typename bucketT>
eRenderStatus Renderer<T, bucketT>::LogScaleDensityFilter(bool forceOutput)
{
	size_t startRow = 0;
	size_t endRow = m_SuperRasH;
	size_t endCol = m_SuperRasW;
	//Timing t(4);
	//Original didn't parallelize this, doing so gives a 50-75% speedup.
	//The value can be directly assigned, which is quicker than summing.
	parallel_for(startRow, endRow, m_ThreadsToUse, [&](size_t j)
	{
		size_t row = j * m_SuperRasW;
		size_t rowEnd = row + endCol;

		if (!m_Abort)
		{
			for (size_t i = row; i < rowEnd; i++)
			{
				//Check for visibility first before doing anything else to avoid all possible unnecessary calculations.
				if (m_HistBuckets[i].a != 0)
				{
					const bucketT logScale = (m_K1 * std::log(1 + m_HistBuckets[i].a * m_K2)) / m_HistBuckets[i].a;
					//Original did a temporary assignment, then *= logScale, then passed the result to bump_no_overflow().
					//Combine here into one operation for a slight speedup.
					//Vectorized version:
					bucketT* __restrict hist = glm::value_ptr(m_HistBuckets[i]);//Vectorizer can't tell these point to different locations.
					bucketT* __restrict acc = glm::value_ptr(m_AccumulatorBuckets[i]);

					for (size_t v = 0; v < 4; v++)//Vectorized by compiler.
						acc[v] = hist[v] * logScale;
				}
			}
		}
	});

	if (m_Callback && !m_Abort)
		if (!m_Callback->ProgressFunc(m_Ember, m_ProgressParameter, 100.0, 1, 0))
			Abort();

	//t.Toc(__FUNCTION__);
	return m_Abort ? eRenderStatus::RENDER_ABORT : eRenderStatus::RENDER_OK;
}

/// <summary>
/// Perform the more advanced Gaussian density filter.
/// More advanced density estimation filtering given less mention in the paper, but used
/// much more in practice as it gives the best results.
/// Section 8, p. 11-13.
/// </summary>
/// <returns>True if not prematurely aborted, else false.</returns>
template <typename T, typename bucketT>
eRenderStatus Renderer<T, bucketT>::GaussianDensityFilter()
{
	Timing totalTime, localTime;
	bool scf = !(Supersample() & 1);
	intmax_t ss = Floor<T>(Supersample() / static_cast<T>(2));
	T scfact = std::pow(Supersample() / (Supersample() + static_cast<T>(1)), static_cast<T>(2));
	size_t startRow = Supersample() - 1;
	size_t endRow = m_SuperRasH - (Supersample() - 1);//Original did + which is most likely wrong.
	intmax_t startCol = Supersample() - 1;
	intmax_t endCol = m_SuperRasW - (Supersample() - 1);
	size_t chunkSize = static_cast<size_t>(std::ceil(static_cast<double>(endRow - startRow) / static_cast<double>(m_ThreadsToUse)));
	//parallel_for scales very well, dividing the work almost perfectly among all processors.
	parallel_for(static_cast<size_t>(0), m_ThreadsToUse, m_ThreadsToUse, [&] (size_t threadIndex)
	{
		size_t pixelNumber = 0;
		const auto localStartRow = static_cast<intmax_t>(std::min<size_t>(startRow + (threadIndex * chunkSize), endRow - 1));
		const auto localEndRow = static_cast<intmax_t>(std::min<size_t>(localStartRow + chunkSize, endRow));
		const size_t pixelsThisThread = static_cast<size_t>(localEndRow - localStartRow) * m_SuperRasW;
		double lastPercent = 0;
		tvec4<bucketT, glm::defaultp> logScaleBucket;

		for (intmax_t j = localStartRow; (j < localEndRow) && !m_Abort; j++)
		{
			const auto buckets = m_HistBuckets.data();
			const auto bucketRowStart = buckets + (j * m_SuperRasW);//Pull out of inner loop for optimization.
			const auto filterCoefs = m_DensityFilter->Coefs();
			const auto filterWidths = m_DensityFilter->Widths();

			for (intmax_t i = startCol; i < endCol; i++)
			{
				intmax_t ii, jj, arrFilterWidth;
				size_t filterSelectInt, filterCoefIndex;
				T filterSelect = 0;
				auto bucket = bucketRowStart + i;

				//Don't do anything if there's no hits here. Must also put this first to avoid dividing by zero below.
				if (bucket->a == 0)
					continue;

				const bucketT cacheLog = (m_K1 * std::log(1 + bucket->a * m_K2)) / bucket->a;//Caching this calculation gives a 30% speedup.

				if (ss == 0)
				{
					filterSelect = bucket->a;
				}
				else
				{
					//The original contained a glaring flaw as it would run past the boundaries of the buffers
					//when calculating the density for a box centered on the last row or column.
					//Clamp here to not run over the edge.
					const intmax_t densityBoxLeftX = (i - std::min(i, ss));
					const intmax_t densityBoxRightX = (i + std::min(ss, static_cast<intmax_t>(m_SuperRasW) - i - 1));
					const intmax_t densityBoxTopY = (j - std::min(j, ss));
					const intmax_t densityBoxBottomY = (j + std::min(ss, static_cast<intmax_t>(m_SuperRasH) - j - 1));

					//Count density in ssxss area.
					//Original went one col at a time, which is cache inefficient. Go one row at at time here for a slight speedup.
					for (jj = densityBoxTopY; jj <= densityBoxBottomY; jj++)
						for (ii = densityBoxLeftX; ii <= densityBoxRightX; ii++)
							filterSelect += buckets[ii + (jj * m_SuperRasW)].a;//Original divided by 255 in every iteration. Omit here because colors are already in the range of [0..1].
				}

				//Scale if supersample > 1 for equal iters.
				if (scf)
					filterSelect *= scfact;

				if (filterSelect > m_DensityFilter->MaxFilteredCounts())
					filterSelectInt = m_DensityFilter->MaxFilterIndex();
				else if (filterSelect <= DE_THRESH)
					filterSelectInt = static_cast<size_t>(std::ceil(filterSelect)) - 1;
				else
					filterSelectInt = DE_THRESH + static_cast<size_t>(Floor<T>(std::pow(filterSelect - DE_THRESH, m_DensityFilter->Curve())));

				//If the filter selected below the min specified clamp it to the min.
				if (filterSelectInt > m_DensityFilter->MaxFilterIndex())
					filterSelectInt = m_DensityFilter->MaxFilterIndex();

				//Only have to calculate the values for ~1/8 of the square.
				filterCoefIndex = filterSelectInt * m_DensityFilter->KernelSize();
				arrFilterWidth = static_cast<intmax_t>(std::ceil(filterWidths[filterSelectInt])) - 1;

				for (jj = 0; jj <= arrFilterWidth; jj++)
				{
					for (ii = 0; ii <= jj; ii++, filterCoefIndex++)
					{
						//Skip if coef is 0.
						if (filterCoefs[filterCoefIndex] == 0)
							continue;

						bucketT logScale = filterCoefs[filterCoefIndex] * cacheLog;
						//Original first assigned the fields, then scaled them. Combine into a single step for a 1% optimization.
						logScaleBucket = (*bucket * logScale);

						if (jj == 0 && ii == 0)
						{
							AddToAccum(logScaleBucket, i, ii, j, jj);
						}
						else if (ii == 0)
						{
							AddToAccum(logScaleBucket, i,   0, j, -jj);
							AddToAccum(logScaleBucket, i, -jj, j,   0);
							AddToAccum(logScaleBucket, i,  jj, j,   0);
							AddToAccum(logScaleBucket, i,   0, j,  jj);
						}
						else if (jj == ii)
						{
							AddToAccum(logScaleBucket, i, -ii, j, -jj);
							AddToAccum(logScaleBucket, i,  ii, j, -jj);
							AddToAccum(logScaleBucket, i, -ii, j,  jj);
							AddToAccum(logScaleBucket, i,  ii, j,  jj);
						}
						else
						{
							//Attempting to optimize cache access by putting these in order makes no difference, even on large images, but do it anyway.
							AddToAccum(logScaleBucket, i, -ii, j, -jj);
							AddToAccum(logScaleBucket, i,  ii, j, -jj);
							AddToAccum(logScaleBucket, i, -jj, j, -ii);
							AddToAccum(logScaleBucket, i,  jj, j, -ii);
							AddToAccum(logScaleBucket, i, -jj, j,  ii);
							AddToAccum(logScaleBucket, i,  jj, j,  ii);
							AddToAccum(logScaleBucket, i, -ii, j,  jj);
							AddToAccum(logScaleBucket, i,  ii, j,  jj);
						}
					}
				}
			}

			if (m_Callback && threadIndex == 0)
			{
				pixelNumber += m_SuperRasW;
				const auto percent = (static_cast<double>(pixelNumber) / static_cast<double>(pixelsThisThread)) * 100.0;
				const auto percentDiff = percent - lastPercent;
				const auto toc = localTime.Toc();

				if (percentDiff >= 10 || (toc > 1000 && percentDiff >= 1))
				{
					const auto etaMs = ((100.0 - percent) / percent) * totalTime.Toc();

					if (!m_Callback->ProgressFunc(m_Ember, m_ProgressParameter, percent, 1, etaMs))
						Abort();

					lastPercent = percent;
					localTime.Tic();
				}
			}
		}
	});

	if (m_Callback && !m_Abort)
		m_Callback->ProgressFunc(m_Ember, m_ProgressParameter, 100.0, 1, 0);

	//totalTime.Toc(__FUNCTION__);
	return m_Abort ? eRenderStatus::RENDER_ABORT : eRenderStatus::RENDER_OK;
}

/// <summary>
/// Produce a final, visible image by clipping, gamma correcting and spatial filtering the color values
/// in the density filtering buffer and save to the passed in buffer.
/// </summary>
/// <param name="pixels">The pixel vector to allocate and store the final image in</param>
/// <param name="finalOffset">Offset in the buffer to store the pixels to</param>
/// <returns>True if not prematurely aborted, else false.</returns>
template <typename T, typename bucketT>
eRenderStatus Renderer<T, bucketT>::AccumulatorToFinalImage(vector<v4F>& pixels, size_t finalOffset)
{
	EnterFinalAccum();

	if (!PrepFinalAccumVector(pixels))
	{
		LeaveFinalAccum();
		return eRenderStatus::RENDER_ERROR;
	}

	//Timing t(4);
	const size_t filterWidth = m_SpatialFilter->FinalFilterWidth();
	bucketT g, linRange, vibrancy;
	Color<bucketT> background;
	auto p = pixels.data();
	p += finalOffset;
	PrepFinalAccumVals(background, g, linRange, vibrancy);//After this, background has been scaled from 0-1 to 0-255.

	//If early clip, go through the entire accumulator and perform gamma correction first.
	//The original does it this way as well and it's roughly 11 times faster to do it this way than inline below with each pixel.
	if (EarlyClip())
	{
		parallel_for(static_cast<size_t>(0), m_SuperRasH, m_ThreadsToUse, [&](size_t j)
		{
			auto rowStart = m_AccumulatorBuckets.data() + (j * m_SuperRasW);//Pull out of inner loop for optimization.
			const auto rowEnd = rowStart + m_SuperRasW;

			while (rowStart < rowEnd && !m_Abort)//Use the pointer itself as the offset to save an extra addition per iter.
			{
				GammaCorrection(*rowStart, background, g, linRange, vibrancy, false, glm::value_ptr(*rowStart));//Write back in place.
				rowStart++;
			}
		});
	}

	if (m_Abort)
	{
		LeaveFinalAccum();
		return eRenderStatus::RENDER_ABORT;
	}

	//Note that abort is not checked here. The final accumulation must run to completion
	//otherwise artifacts that resemble page tearing will occur in an interactive run. It's
	//critical to never exit this loop prematurely.
	//for (size_t j = 0; j < FinalRasH(); j++)//Keep around for debugging.
	parallel_for(static_cast<size_t>(0), FinalRasH(), m_ThreadsToUse, [&](size_t j)
	{
		Color<bucketT> newBucket;
		size_t pixelsRowStart = (m_YAxisUp ? ((FinalRasH() - j) - 1) : j) * FinalRasW();//Pull out of inner loop for optimization.
		size_t y = m_DensityFilterOffset + (j * Supersample());//Start at the beginning row of each super sample block.
		size_t clampedFilterH = std::min(filterWidth, m_SuperRasH - y);//Make sure the filter doesn't go past the bottom of the gutter.
		auto pv4T = p + pixelsRowStart;

		for (size_t i = 0; i < FinalRasW(); i++, pv4T++)
		{
			size_t ii, jj;
			const size_t x = m_DensityFilterOffset + (i * Supersample());//Start at the beginning column of each super sample block.
			const size_t clampedFilterW = std::min(filterWidth, m_SuperRasW - x);//Make sure the filter doesn't go past the right of the gutter.
			newBucket.Clear();

			//Original was iterating column-wise, which is slow.
			//Here, iterate one row at a time, giving a 10% speed increase.
			for (jj = 0; jj < clampedFilterH; jj++)
			{
				size_t filterKRowIndex = jj * filterWidth;//Use the full, non-clamped width to get the filter value.
				size_t accumRowIndex = (y + jj) * m_SuperRasW;//Pull out of inner loop for optimization.

				for (ii = 0; ii < clampedFilterW; ii++)
				{
					//Need to dereference the spatial filter pointer object to use the [] operator. Makes no speed difference.
					bucketT k = ((*m_SpatialFilter)[filterKRowIndex + ii]);
					newBucket += (m_AccumulatorBuckets[accumRowIndex + (x + ii)] * k);
				}
			}

			auto pf = reinterpret_cast<float*>(pv4T);
			GammaCorrection(*(reinterpret_cast<tvec4<bucketT, glm::defaultp>*>(&newBucket)), background, g, linRange, vibrancy, true, pf);
		}
	});

	//Insert the palette into the image for debugging purposes. Not implemented on the GPU.
	if (m_InsertPalette)
	{
		size_t i, j, ph = 100;

		if (ph >= FinalRasH())
			ph = FinalRasH();

		for (j = 0; j < ph; j++)
		{
			for (i = 0; i < FinalRasW(); i++)
			{
				const auto pp = p + (i + j * FinalRasW());
				pp->r = m_TempEmber.m_Palette[i * 256 / FinalRasW()][0];
				pp->g = m_TempEmber.m_Palette[i * 256 / FinalRasW()][1];
				pp->b = m_TempEmber.m_Palette[i * 256 / FinalRasW()][2];
				pp->a = 1;
			}
		}
	}

	//t.Toc(__FUNCTION__);
	LeaveFinalAccum();
	return m_Abort ? eRenderStatus::RENDER_ABORT : eRenderStatus::RENDER_OK;
}

//#define TG 1
//#define NEWSUBBATCH 1

/// <summary>
/// Run the iteration algorithm for the specified number of iterations.
/// This is only called after all other setup has been done.
/// This function will be called multiple times for an interactive rendering, and
/// once for a straight through render.
/// The iteration is reset and fused in each thread after each sub batch is done
/// which by default is 10,240 iterations.
/// </summary>
/// <param name="iterCount">The number of iterations to run</param>
/// <param name="temporalSample">The temporal sample this is running for</param>
/// <returns>Rendering statistics</returns>
template <typename T, typename bucketT>
EmberStats Renderer<T, bucketT>::Iterate(size_t iterCount, size_t temporalSample)
{
	//Timing t2(4);
	m_IterTimer.Tic();
	const size_t totalItersPerThread = static_cast<size_t>(std::ceil(static_cast<double>(iterCount) / static_cast<double>(m_ThreadsToUse)));
	EmberStats stats;
	//vector<double> accumTimes(4);

	//Do this every iteration for an animation, or else do it once for a single image. CPU only.
	if (!m_LastIter)
	{
		m_ThreadEmbers.clear();
		m_ThreadEmbers.insert(m_ThreadEmbers.begin(), m_ThreadsToUse, m_Ember);
	}

	parallel_for(static_cast<size_t>(0), m_ThreadsToUse, m_ThreadsToUse, [&] (size_t threadIndex)
	{
#if defined(_WIN32)
		SetThreadPriority(GetCurrentThread(), static_cast<int>(m_Priority));
#elif defined(__APPLE__)
		sched_param sp = {0};
		sp.sched_priority = static_cast<int>(m_Priority);
		pthread_setschedparam(pthread_self(), SCHED_RR, &sp);
#else
		pthread_setschedprio(pthread_self(), static_cast<int>(m_Priority));
#endif
		//Timing t;
		IterParams<T> params;
		m_BadVals[threadIndex] = 0;
		params.m_Count = std::min(totalItersPerThread, SubBatchSize());
		params.m_Skip = FuseCount();
		//params.m_OneColDiv2 = m_CarToRas.OneCol() / 2;
		//params.m_OneRowDiv2 = m_CarToRas.OneRow() / 2;

		//Sub batch iterations, loop 2.
		for (m_SubBatch[threadIndex] = 0; (m_SubBatch[threadIndex] < totalItersPerThread) && !m_Abort; m_SubBatch[threadIndex] += params.m_Count)
		{
			//Must recalculate the number of iters to run on each sub batch because the last batch will most likely have less than SubBatchSize iters.
			//For example, if 51,000 are requested, and the sbs is 10,000, it should run 5 sub batches of 10,000 iters, and one final sub batch of 1,000 iters.
			params.m_Count = std::min(params.m_Count, totalItersPerThread - m_SubBatch[threadIndex]);
			//Use first as random point, the rest are iterated points.
			//Note that this gets reset with a new random point for each SubBatchSize iterations.
			//This helps correct if iteration happens to be on a bad trajectory.
			m_Samples[threadIndex][0].m_X = m_Rand[threadIndex].template Frand<T>(-m_ThreadEmbers[threadIndex].m_RandPointRange, m_ThreadEmbers[threadIndex].m_RandPointRange);
			m_Samples[threadIndex][0].m_Y = m_Rand[threadIndex].template Frand<T>(-m_ThreadEmbers[threadIndex].m_RandPointRange, m_ThreadEmbers[threadIndex].m_RandPointRange);
			m_Samples[threadIndex][0].m_Z = 0;//m_Ember.m_CamZPos;//Apo set this to 0, then made the user use special variations to kick it. It seems easier to just set it to zpos.
			m_Samples[threadIndex][0].m_ColorX = m_Rand[threadIndex].template Frand01<T>();

			//Check if the user wanted to suspend the process.
			while (Paused())
				std::this_thread::sleep_for(500ms);

			//Finally, iterate.
			//t.Tic();
			//Iterating, loop 3.
			m_BadVals[threadIndex] += m_Iterator->Iterate(m_ThreadEmbers[threadIndex], params, m_CarToRas, m_Samples[threadIndex].data(), m_Rand[threadIndex]);
			//iterationTime += t.Toc();

			if (m_LockAccum)
				m_AccumCs.lock();

			//t.Tic();
			//Map temp buffer samples into the histogram using the palette for color.
			Accumulate(m_Rand[threadIndex], m_Samples[threadIndex].data(), params.m_Count, &m_Dmap);
			//accumTimes[threadIndex] += t.Toc();

			if (m_LockAccum)
				m_AccumCs.unlock();

			if (m_Callback && threadIndex == 0)
			{
				auto percent = 100.0 *
							   static_cast<double>
							   (
								   static_cast<double>
								   (
									   static_cast<double>
									   (
										   //Takes progress of current thread and multiplies by thread count.
										   //This assumes the threads progress at roughly the same speed.
										   //Adding m_LastIter is done so that an incremental render still gives an accurate percentage.
										   static_cast<double>(m_LastIter + (m_SubBatch[threadIndex] * m_ThreadsToUse)) / static_cast<double>(ItersPerTemporalSample())
									   ) + temporalSample
								   ) / static_cast<double>(TemporalSamples())
							   );
				const auto percentDiff = percent - m_LastIterPercent;
				const auto toc = m_ProgressTimer.Toc();

				if (percentDiff >= 10 || (toc > 1000 && percentDiff >= 1))//Call callback function if either 10% has passed, or one second (and 1%).
				{
					const auto startingpercent = 100.0 * (m_LastIter / static_cast<double>(ItersPerTemporalSample()));//This is done to support incremental renders, starting from the percentage it left off on.
					const auto currentpercent = percent - startingpercent;//Current percent in terms of starting percentage. So starting at 50% and progressing 5% will give a value of 5%, not 55%.
					const auto etaMs = currentpercent == 0 ? 0 : (((100.0 - startingpercent) - currentpercent) / currentpercent) * m_RenderTimer.Toc();//Subtract startingpercent from 100% so that it's properly scaled, meaning rendering from 50% - 100% will be treated as 0% - 100%.

					if (!m_Callback->ProgressFunc(m_Ember, m_ProgressParameter, percent, 0, etaMs))
						Abort();

					m_LastIterPercent = percent;
					m_ProgressTimer.Tic();
				}
			}
		}
	});
	stats.m_Iters = std::accumulate(m_SubBatch.begin(), m_SubBatch.end(), 0ULL);//Sum of iter count of all threads.
	stats.m_Badvals = std::accumulate(m_BadVals.begin(), m_BadVals.end(), 0ULL);
	stats.m_IterMs = m_IterTimer.Toc();
	//cout << "Accum time: " << std::accumulate(accumTimes.begin(), accumTimes.end(), 0.0) << endl;
	//t2.Toc(__FUNCTION__);
	return stats;
}

/// <summary>
/// Non-virtual render properties, getters and setters.
/// </summary>

/// <summary>
/// Get the pixel aspect ratio of the output image.
/// Default: 1.
/// </summary>
/// <returns>The pixel aspect ratio.</returns>
template <typename T, typename bucketT> T Renderer<T, bucketT>::PixelAspectRatio() const { return m_PixelAspectRatio; }

/// <summary>
/// Set the pixel aspect ratio of the output image.
/// Reset the rendering process.
/// </summary>
/// <param name="pixelAspectRatio">The pixel aspect ratio.</param>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::PixelAspectRatio(T pixelAspectRatio)
{
	ChangeVal([&] { m_PixelAspectRatio = pixelAspectRatio; }, eProcessAction::FULL_RENDER);
}

/// <summary>
/// Non-virtual renderer properties, getters only.
/// </summary>

template <typename T, typename bucketT> T							   Renderer<T, bucketT>::Scale()			   const { return m_Scale; }
template <typename T, typename bucketT> T							   Renderer<T, bucketT>::PixelsPerUnitX()	   const { return m_PixelsPerUnitX; }
template <typename T, typename bucketT> T							   Renderer<T, bucketT>::PixelsPerUnitY()	   const { return m_PixelsPerUnitY; }
template <typename T, typename bucketT> bucketT						   Renderer<T, bucketT>::K1()				   const { return m_K1; }
template <typename T, typename bucketT> bucketT						   Renderer<T, bucketT>::K2()				   const { return m_K2; }
template <typename T, typename bucketT> const CarToRas<T>&			   Renderer<T, bucketT>::CoordMap()			   const { return m_CarToRas; }
template <typename T, typename bucketT> tvec4<bucketT, glm::defaultp>* Renderer<T, bucketT>::HistBuckets()				 { return m_HistBuckets.data(); }
template <typename T, typename bucketT> tvec4<bucketT, glm::defaultp>* Renderer<T, bucketT>::AccumulatorBuckets()		 { return m_AccumulatorBuckets.data(); }
template <typename T, typename bucketT> SpatialFilter<bucketT>*		   Renderer<T, bucketT>::GetSpatialFilter()			 { return m_SpatialFilter.get(); }
template <typename T, typename bucketT> TemporalFilter<T>*			   Renderer<T, bucketT>::GetTemporalFilter()		 { return m_TemporalFilter.get(); }

/// <summary>
/// Virtual renderer properties overridden from RendererBase, getters only.
/// </summary>

template <typename T, typename bucketT> double			   Renderer<T, bucketT>::ScaledQuality()		  const { return static_cast<double>(m_ScaledQuality); }
template <typename T, typename bucketT> double			   Renderer<T, bucketT>::LowerLeftX(bool  gutter) const { return static_cast<double>(gutter ? m_CarToRas.CarLlX() : m_LowerLeftX); }
template <typename T, typename bucketT> double			   Renderer<T, bucketT>::LowerLeftY(bool  gutter) const { return static_cast<double>(gutter ? m_CarToRas.CarLlY() : m_LowerLeftY); }
template <typename T, typename bucketT> double			   Renderer<T, bucketT>::UpperRightX(bool gutter) const { return static_cast<double>(gutter ? m_CarToRas.CarUrX() : m_UpperRightX); }
template <typename T, typename bucketT> double			   Renderer<T, bucketT>::UpperRightY(bool gutter) const { return static_cast<double>(gutter ? m_CarToRas.CarUrY() : m_UpperRightY); }
template <typename T, typename bucketT> DensityFilterBase* Renderer<T, bucketT>::GetDensityFilter()			    { return m_DensityFilter.get(); }

/// <summary>
/// Non-virtual ember wrappers, getters only.
/// </summary>

template <typename T, typename bucketT> bool                  Renderer<T, bucketT>::XaosPresent()         const { return m_Ember.XaosPresent(); }
template <typename T, typename bucketT> size_t			      Renderer<T, bucketT>::Supersample()         const { return m_Ember.m_Supersample; }
template <typename T, typename bucketT> size_t			      Renderer<T, bucketT>::PaletteIndex()        const { return m_Ember.PaletteIndex(); }
template <typename T, typename bucketT> T                     Renderer<T, bucketT>::Time()                const { return m_Ember.m_Time; }
template <typename T, typename bucketT> T                     Renderer<T, bucketT>::Quality()             const { return m_Ember.m_Quality; }
template <typename T, typename bucketT> T                     Renderer<T, bucketT>::SpatialFilterRadius() const { return m_Ember.m_SpatialFilterRadius; }
template <typename T, typename bucketT> T                     Renderer<T, bucketT>::PixelsPerUnit()       const { return m_Ember.m_PixelsPerUnit; }
template <typename T, typename bucketT> T                     Renderer<T, bucketT>::Zoom()                const { return m_Ember.m_Zoom; }
template <typename T, typename bucketT> T                     Renderer<T, bucketT>::CenterX()             const { return m_Ember.m_CenterX; }
template <typename T, typename bucketT> T                     Renderer<T, bucketT>::CenterY()             const { return m_Ember.m_CenterY; }
template <typename T, typename bucketT> T                     Renderer<T, bucketT>::Rotate()              const { return m_Ember.m_Rotate; }
template <typename T, typename bucketT> bucketT               Renderer<T, bucketT>::Brightness()		  const { return static_cast<bucketT>(m_Ember.m_Brightness); }
template <typename T, typename bucketT> bucketT               Renderer<T, bucketT>::Gamma()				  const { return static_cast<bucketT>(m_Ember.m_Gamma); }
template <typename T, typename bucketT> bucketT               Renderer<T, bucketT>::Vibrancy()			  const { return static_cast<bucketT>(m_Ember.m_Vibrancy); }
template <typename T, typename bucketT> bucketT               Renderer<T, bucketT>::GammaThresh()		  const { return static_cast<bucketT>(m_Ember.m_GammaThresh); }
template <typename T, typename bucketT> bucketT               Renderer<T, bucketT>::HighlightPower()	  const { return static_cast<bucketT>(m_Ember.m_HighlightPower); }
template <typename T, typename bucketT> Color<T>		      Renderer<T, bucketT>::Background()		  const { return m_Ember.m_Background; }
template <typename T, typename bucketT> const Xform<T>*       Renderer<T, bucketT>::Xforms()			  const { return m_Ember.Xforms(); }
template <typename T, typename bucketT> Xform<T>*             Renderer<T, bucketT>::NonConstXforms()			{ return m_Ember.NonConstXforms(); }
template <typename T, typename bucketT> size_t			      Renderer<T, bucketT>::XformCount()          const { return m_Ember.XformCount(); }
template <typename T, typename bucketT> const Xform<T>*       Renderer<T, bucketT>::FinalXform()          const { return m_Ember.FinalXform(); }
template <typename T, typename bucketT> Xform<T>*             Renderer<T, bucketT>::NonConstFinalXform()        { return m_Ember.NonConstFinalXform(); }
template <typename T, typename bucketT> bool                  Renderer<T, bucketT>::UseFinalXform()       const { return m_Ember.UseFinalXform(); }
template <typename T, typename bucketT> const Palette<float>* Renderer<T, bucketT>::GetPalette()          const { return &m_Ember.m_Palette; }
template <typename T, typename bucketT> ePaletteMode          Renderer<T, bucketT>::PaletteMode()         const { return m_Ember.m_PaletteMode; }

/// <summary>
/// Virtual ember wrappers overridden from RendererBase, getters only.
/// </summary>

template <typename T, typename bucketT> size_t Renderer<T, bucketT>::TemporalSamples() const { return m_Ember.m_TemporalSamples; }
template <typename T, typename bucketT> size_t Renderer<T, bucketT>::FinalRasW()       const { return m_Ember.m_FinalRasW; }
template <typename T, typename bucketT> size_t Renderer<T, bucketT>::FinalRasH()       const { return m_Ember.m_FinalRasH; }
template <typename T, typename bucketT> size_t Renderer<T, bucketT>::SubBatchSize()    const { return m_Ember.m_SubBatchSize; }
template <typename T, typename bucketT> size_t Renderer<T, bucketT>::FuseCount()	   const { return m_Ember.m_FuseCount; }

/// <summary>
/// Non-virtual iterator wrappers.
/// </summary>

template <typename T, typename bucketT> const byte* Renderer<T, bucketT>::XformDistributions() 		  const { return m_Iterator ? m_Iterator->XformDistributions() : nullptr; }
template <typename T, typename bucketT> size_t		Renderer<T, bucketT>::XformDistributionsSize()    const { return m_Iterator ? m_Iterator->XformDistributionsSize() : 0; }
template <typename T, typename bucketT> Point<T>*   Renderer<T, bucketT>::Samples(size_t threadIndex) const { return threadIndex < m_Samples.size() ? const_cast<Point<T>*>(m_Samples[threadIndex].data()) : nullptr; }

/// <summary>
/// Non-virtual functions that might be needed by a derived class.
/// </summary>

/// <summary>
/// Prepare various values needed for producing a final output image.
/// </summary>
/// <param name="background">The computed background value, which may differ from the background member</param>
/// <param name="g">The computed gamma</param>
/// <param name="linRange">The computed linear range</param>
/// <param name="vibrancy">The computed vibrancy</param>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::PrepFinalAccumVals(Color<bucketT>& background, bucketT& g, bucketT& linRange, bucketT& vibrancy)
{
	//If they are doing incremental rendering, they can get here without doing a full temporal
	//sample, which means the values will be zero.
	vibrancy = m_Vibrancy == 0 ? Vibrancy() : m_Vibrancy;
	size_t vibGamCount = m_VibGamCount == 0 ? 1 : m_VibGamCount;
	const bucketT gamma = m_Gamma == 0 ? Gamma() : m_Gamma;
	g = 1 / ClampGte<bucketT>(gamma / vibGamCount, static_cast<bucketT>(0.01));//Ensure a divide by zero doesn't occur.
	linRange = GammaThresh();
	vibrancy /= vibGamCount;
	background.x = (IsNearZero(m_Background.r) ? static_cast<bucketT>(m_Ember.m_Background.r) : m_Background.r) / vibGamCount;
	background.y = (IsNearZero(m_Background.g) ? static_cast<bucketT>(m_Ember.m_Background.g) : m_Background.g) / vibGamCount;
	background.z = (IsNearZero(m_Background.b) ? static_cast<bucketT>(m_Ember.m_Background.b) : m_Background.b) / vibGamCount;
}

/// <summary>
/// Miscellaneous non-virtual functions used only in this class.
/// </summary>

/// <summary>
/// Accumulate the samples to the histogram.
/// To be called after a sub batch is finished iterating.
/// </summary>
/// <param name="samples">The samples to accumulate</param>
/// <param name="sampleCount">The number of samples</param>
/// <param name="palette">The palette to use</param>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::Accumulate(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand, Point<T>* samples, size_t sampleCount, const Palette<bucketT>* palette)
{
	intmax_t intColorIndex;
	size_t histIndex, histSize = m_HistBuckets.size();
	bucketT colorIndex, colorIndexFrac;
	const intmax_t psm1 = m_Ember.m_Palette.Size() - 1;

	//Linear is a linear scale for when the color index is not a whole number, which is most of the time.
	//It uses a portion of the value of the index, and the remainder of the next index.
	//Example: index = 25.7
	//Fraction = 0.7
	//Color = (dmap[25] * 0.3) + (dmap[26] * 0.7)
	//Use overloaded addition and multiplication operators in vec4 to perform the accumulation.
	if (PaletteMode() == ePaletteMode::PALETTE_LINEAR)
	{
		const intmax_t psm2 = psm1 - 1;

		//It's critical to understand what's going on here as it's one of the most important parts of the algorithm.
		//A color value gets retrieved from the palette and
		//its RGB values are added to the existing RGB values in the histogram bucket.
		//Alpha is always 1 in the palettes, so that serves as the hit count.
		//This differs from the original since redundantly adding both an alpha component and a hit count is omitted.
		//This will eventually leave us with large values for pixels with many hits, which will be log scaled down later.
		//Original used a function called bump_no_overflow(). Just do a straight add because the type will always be float or double.
		//Doing so gives a 25% speed increase.
		//Splitting these conditionals into separate loops makes no speed difference.
		for (size_t i = 0; i < sampleCount && !m_Abort; i++)
		{
			Point<T> p(samples[i]);//Slightly faster to cache this.

			if (p.m_Opacity != 0)
			{
				if (Rotate() != 0)
				{
					T p00 = p.m_X - m_Ember.m_CenterX;
					T p11 = p.m_Y - m_Ember.m_RotCenterY;
					p.m_X = (p00 * m_RotMat.A()) + (p11 * m_RotMat.B()) + m_Ember.m_CenterX;
					p.m_Y = (p00 * m_RotMat.D()) + (p11 * m_RotMat.E()) + m_Ember.m_RotCenterY;
				}

				//Checking this first before converting gives better performance than converting and checking a single value, which the original did.
				//Second, an interesting optimization observation is that when keeping the bounds vars within m_CarToRas and calling its InBounds() member function,
				//rather than here as members, about a 7% speedup is achieved. This is possibly due to the fact that data from m_CarToRas is accessed
				//right after the call to Convert(), so some caching efficiencies get realized.
				if (m_CarToRas.InBounds(p))
				{
					m_CarToRas.Convert(p, histIndex);

					//There is a very slim chance that a point will be right on the border and will technically be in bounds, passing the InBounds() test,
					//but ends up being mapped to a histogram bucket that is out of bounds due to roundoff error. Perform one final check before proceeding.
					//This will result in a few points at the very edges getting discarded, but prevents a crash and doesn't seem to make a speed difference.
					if (histIndex < histSize)
					{
						colorIndex = static_cast<bucketT>(p.m_ColorX) * psm1;
						intColorIndex = static_cast<intmax_t>(colorIndex);

						if (intColorIndex < 0)
						{
							intColorIndex = 0;
							colorIndexFrac = 0;
						}
						else if (intColorIndex >= psm1)
						{
							intColorIndex = psm2;
							colorIndexFrac = 1;
						}
						else
						{
							colorIndexFrac = colorIndex - static_cast<bucketT>(intColorIndex);//Interpolate between intColorIndex and intColorIndex + 1.
						}

						bucketT* __restrict hist = glm::value_ptr(m_HistBuckets[histIndex]);//Vectorizer can't tell these point to different locations.
						const bucketT* __restrict pal = glm::value_ptr(palette->m_Entries[intColorIndex]);
						const bucketT* __restrict pal2 = glm::value_ptr(palette->m_Entries[intColorIndex + 1]);
						const auto cifm1 = static_cast<bucketT>(1) - colorIndexFrac;

						//Loops are unrolled to allow auto vectorization.
						if (p.m_Opacity == 1)
						{
							hist[0] += (pal[0] * cifm1) + (pal2[0] * colorIndexFrac);
							hist[1] += (pal[1] * cifm1) + (pal2[1] * colorIndexFrac);
							hist[2] += (pal[2] * cifm1) + (pal2[2] * colorIndexFrac);
							hist[3] += (pal[3] * cifm1) + (pal2[3] * colorIndexFrac);
						}
						else
						{
							const auto va = static_cast<bucketT>(p.m_Opacity);
							hist[0] += ((pal[0] * cifm1) + (pal2[0] * colorIndexFrac)) * va;
							hist[1] += ((pal[1] * cifm1) + (pal2[1] * colorIndexFrac)) * va;
							hist[2] += ((pal[2] * cifm1) + (pal2[2] * colorIndexFrac)) * va;
							hist[3] += ((pal[3] * cifm1) + (pal2[3] * colorIndexFrac)) * va;
						}
					}
				}
			}
		}
	}
	else if (PaletteMode() == ePaletteMode::PALETTE_STEP)//Duplicate of above, but for step mode.
	{
		for (size_t i = 0; i < sampleCount && !m_Abort; i++)
		{
			Point<T> p(samples[i]);//Slightly faster to cache this.

			if (p.m_Opacity != 0)
			{
				if (Rotate() != 0)
				{
					const T p00 = p.m_X - m_Ember.m_CenterX;
					const T p11 = p.m_Y - m_Ember.m_RotCenterY;
					p.m_X = (p00 * m_RotMat.A()) + (p11 * m_RotMat.B()) + m_Ember.m_CenterX;
					p.m_Y = (p00 * m_RotMat.D()) + (p11 * m_RotMat.E()) + m_Ember.m_RotCenterY;
				}

				if (m_CarToRas.InBounds(p))
				{
					m_CarToRas.Convert(p, histIndex);

					if (histIndex < histSize)
					{
						intColorIndex = Clamp<size_t>(static_cast<size_t>(p.m_ColorX * psm1), 0, psm1);
						bucketT* __restrict hist = glm::value_ptr(m_HistBuckets[histIndex]);//Vectorizer can't tell these point to different locations.
						const bucketT* __restrict pal = glm::value_ptr(palette->m_Entries[intColorIndex]);

						if (p.m_Opacity == 1)
						{
							hist[0] += pal[0];
							hist[1] += pal[1];
							hist[2] += pal[2];
							hist[3] += pal[3];
						}
						else
						{
							auto va = static_cast<bucketT>(p.m_Opacity);
							hist[0] += pal[0] * va;
							hist[1] += pal[1] * va;
							hist[2] += pal[2] * va;
							hist[3] += pal[3] * va;
						}
					}
				}
			}
		}
	}
}

/// <summary>
/// Add a value to the density filtering buffer with a bounds check.
/// </summary>
/// <param name="bucket">The bucket being filtered</param>
/// <param name="i">The column of the bucket</param>
/// <param name="ii">The offset to add to the column</param>
/// <param name="j">The row of the bucket</param>
/// <param name="jj">The offset to add to the row</param>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::AddToAccum(const tvec4<bucketT, glm::defaultp>& bucket, intmax_t i, intmax_t ii, intmax_t j, intmax_t jj)
{
	if (j + jj >= 0 && j + jj < static_cast<intmax_t>(m_SuperRasH) && i + ii >= 0 && i + ii < static_cast<intmax_t>(m_SuperRasW))
	{
		auto* __restrict accum = m_AccumulatorBuckets.data() + ((i + ii) + ((j + jj) * m_SuperRasW));//For vectorizer, results in a 33% speedup.
		accum->r += bucket.r;
		accum->g += bucket.g;
		accum->b += bucket.b;
		accum->a += bucket.a;
	}
}

/// <summary>
/// Clip and gamma correct a pixel.
/// Because this code is used in both early and late clipping, a few extra arguments are passed
/// to specify what actions to take. Coupled with an additional template argument, this allows
/// using one function to perform all color clipping, gamma correction and final accumulation.
/// Template argument accumT is expected to always be float4.
/// </summary>
/// <param name="bucket">The pixel to correct</param>
/// <param name="background">The background color</param>
/// <param name="g">The gamma to use</param>
/// <param name="linRange">The linear range to use</param>
/// <param name="vibrancy">The vibrancy to use</param>
/// <param name="scale">True if late clip, else false.</param>
/// <param name="correctedChannels">The storage space for the corrected values to be written to</param>
template <typename T, typename bucketT>
template <typename accumT>
void Renderer<T, bucketT>::GammaCorrection(tvec4<bucketT, glm::defaultp>& bucket, Color<bucketT>& background, bucketT g, bucketT linRange, bucketT vibrancy, bool scale, accumT* correctedChannels)
{
	auto bt1 = static_cast<bucketT>(1);

	if (scale && EarlyClip())
	{
		if (m_CurvesSet)
		{
			CurveAdjust(bucket.r, 1);
			CurveAdjust(bucket.g, 2);
			CurveAdjust(bucket.b, 3);
		}

		correctedChannels[0] = static_cast<accumT>(Clamp<bucketT>(bucket.r, 0, bt1));
		correctedChannels[1] = static_cast<accumT>(Clamp<bucketT>(bucket.g, 0, bt1));
		correctedChannels[2] = static_cast<accumT>(Clamp<bucketT>(bucket.b, 0, bt1));
		correctedChannels[3] = static_cast<accumT>(Clamp<bucketT>(bucket.a, 0, bt1));
	}
	else
	{
		bucketT alpha, ls, a, newRgb[3];//Would normally use a Color<bucketT>, but don't want to call a needless constructor every time this function is called, which is once per pixel.

		if (bucket.a <= 0)
		{
			alpha = 0;
			ls = 0;
		}
		else
		{
			alpha = Palette<bucketT>::CalcAlpha(bucket.a, g, linRange);
			ls = vibrancy * alpha / bucket.a;
			ClampRef<bucketT>(alpha, 0, 1);
		}

		Palette<bucketT>::template CalcNewRgb<bucketT>(glm::value_ptr(bucket), ls, HighlightPower(), newRgb);

		for (glm::length_t rgbi = 0; rgbi < 3; rgbi++)
		{
			a = newRgb[rgbi] + ((1 - vibrancy) * std::pow(std::abs(bucket[rgbi]), g));//Must use abs(), else it it could be a negative value and return NAN.
			a += (1 - alpha) * background[rgbi];

			if (scale && m_CurvesSet)
				CurveAdjust(a, rgbi + 1);

			correctedChannels[rgbi] = static_cast<accumT>(Clamp<bucketT>(a, 0, bt1));//Early clip, just assign directly.
		}

		correctedChannels[3] = static_cast<accumT>(alpha);
	}
}

/// <summary>
/// Setup the curve values when they are being used.
/// </summary>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::ComputeCurves()
{
	if (m_CurvesSet)
	{
		auto st = m_Csa.size();

		for (glm::length_t i = 0; i < m_Ember.m_Curves.m_Points.size(); i++)//Overall, r, g, b.
		{
			if (!m_Ember.m_Curves.m_Points[i].empty())
			{
				Spline<float> spline(m_Ember.m_Curves.m_Points[i]);//Will internally sort.

				for (size_t j = 0; j < st; j++)
					m_Csa[j][i] = spline.Interpolate(j * ONE_OVER_CURVES_LENGTH_M1);
			}
		}
	}
}

/// <summary>
/// Apply the curve adjustment to a single channel.
/// </summary>
/// <param name="aScaled">The value of the channel to apply curve adjustment to.</param>
/// <param name="index">The index of the channel to apply curve adjustment to</param>
template <typename T, typename bucketT>
void Renderer<T, bucketT>::CurveAdjust(bucketT& a, const glm::length_t& index)
{
	size_t tempIndex = static_cast<size_t>(Clamp<bucketT>(a * CURVES_LENGTH_M1, 0, CURVES_LENGTH_M1));
	size_t tempIndex2 = static_cast<size_t>(Clamp<bucketT>(m_Csa[tempIndex].x * CURVES_LENGTH_M1, 0, CURVES_LENGTH_M1));
	a = m_Csa[tempIndex2][index];
}

//This class had to be implemented in a cpp file because the compiler was breaking.
//So the explicit instantiation must be declared here rather than in Ember.cpp where
//all of the other classes are done.
template class EMBER_API Renderer<float, float>;
template EMBER_API void Renderer<float, float>::SetEmber(const vector<Ember<float>>& embers);
template EMBER_API void Renderer<float, float>::SetEmber(const list<Ember<float>>& embers);

#ifdef DO_DOUBLE
	template class EMBER_API Renderer<double, float>;
	template EMBER_API void  Renderer<double, float>::SetEmber(const vector<Ember<double>>& embers);
	template EMBER_API void  Renderer<double, float>::SetEmber(const list<Ember<double>>& embers);
#endif
}
