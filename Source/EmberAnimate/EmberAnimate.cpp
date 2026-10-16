#include "EmberCommonPch.h"
#include "EmberAnimate.h"
#include "JpegUtils.h"
#include <xmmintrin.h>
#include <immintrin.h>
#include <pmmintrin.h>

using namespace EmberCommon;

/// <summary>
/// The core of the EmberAnimate.exe program.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="opt">A populated EmberOptions object which specifies all program options to be used</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool EmberAnimate(int argc, _TCHAR* argv[], EmberOptions& opt)
{
	auto info = OpenCLInfo::Instance();
	std::cout.imbue(std::locale(""));

	if (opt.DumpArgs())
		cout << opt.GetValues(eOptionUse::OPT_USE_ANIMATE) << "\n";

	if (opt.OpenCLInfo())
	{
		cout << "\nOpenCL Info: \n";
		cout << info->DumpInfo();
		return true;
	}

	VerbosePrint("Using " << (sizeof(T) == sizeof(float) ? "single" : "double") << " precision.");
	//Regular variables.
	Timing t;
	bool unsorted = false;
	uint padding;
	size_t i, firstUnsortedIndex = 0;
	string inputPath = GetPath(opt.Input());
	vector<Ember<T>> embers;
	XmlToEmber<T> parser;
	EmberToXml<T> emberToXml;
	Interpolater<T> interpolater;
	EmberReport emberReport;
	const vector<pair<size_t, size_t>> devices = Devices(opt.Devices());
	std::atomic<size_t> atomfTime;
	vector<std::thread> threadVec;
	auto progress = make_unique<RenderProgress<T>>();
	vector<unique_ptr<Renderer<T, float>>> renderers;
	vector<string> errorReport;
	std::recursive_mutex verboseCs;
	auto fullpath = GetExePath(argv[0]);
	Compat::m_Compat = opt.Flam3Compat();

	if (opt.EmberCL())
	{
		renderers = CreateRenderers<T>(eRendererType::OPENCL_RENDERER, devices, false, 0, emberReport);
		errorReport = emberReport.ErrorReport();

		if (!errorReport.empty())
			emberReport.DumpErrorReport();

		if (!renderers.size() || renderers.size() != devices.size())
		{
			cout << "Only created " << renderers.size() << " renderers out of " << devices.size() << " requested, exiting.\n";
			return false;
		}

		for (auto& renderer : renderers)
			if (auto rendererCL = dynamic_cast<RendererCL<T, float>*>(renderer.get()))
			{
				rendererCL->OptAffine(true);//Optimize empty affines for final renderers, this is normally false for the interactive renderer.
				rendererCL->SubBatchPercentPerThread(float(opt.SBPctPerTh()));
			}

		if (opt.DoProgress())
			renderers[0]->Callback(progress.get());

		cout << "Using OpenCL to render.\n";

		if (opt.Verbose())
		{
			for (auto& device : devices)
			{
				cout << "Platform: " << info->PlatformName(device.first) << "\n";
				cout << "Device: " << info->DeviceName(device.first, device.second) << "\n";
			}
		}

		if (opt.ThreadCount() > 1)
			cout << "Cannot specify threads with OpenCL, using 1 thread.\n";

		opt.ThreadCount(1);

		if (opt.IsaacSeed().empty())
		{
			for (auto& r : renderers)
				r->ThreadCount(opt.ThreadCount(), nullptr);
		}
		else
		{
			for (i = 0; i < renderers.size(); i++)
			{
				string ns;
				auto& is = opt.IsaacSeed();
				ns.reserve(is.size());

				for (auto& c : is)
					ns.push_back(c + char(i * opt.ThreadCount()));

				renderers[i]->ThreadCount(opt.ThreadCount(), ns.c_str());
			}
		}
	}
	else
	{
		unique_ptr<Renderer<T, float>> tempRenderer(CreateRenderer<T>(eRendererType::CPU_RENDERER, devices, false, 0, emberReport));
		errorReport = emberReport.ErrorReport();

		if (!errorReport.empty())
			emberReport.DumpErrorReport();

		if (!tempRenderer.get())
		{
			cout << "Renderer creation failed, exiting.\n";
			return false;
		}

		if (opt.DoProgress())
			tempRenderer->Callback(progress.get());

		if (opt.ThreadCount() == 0)
		{
			cout << "Using " << Timing::ProcessorCount() << " automatically detected threads.\n";
			opt.ThreadCount(Timing::ProcessorCount());
		}
		else
		{
			cout << "Using " << opt.ThreadCount() << " manually specified threads.\n";
		}

		tempRenderer->ThreadCount(opt.ThreadCount(), opt.IsaacSeed() != "" ? opt.IsaacSeed().c_str() : nullptr);
		renderers.push_back(std::move(tempRenderer));
	}

	if (!InitPaletteList<float>(fullpath, opt.PalettePath())) //For any modern flames, the palette isn't used. This is for legacy purposes and should be removed.
		return false;

	cout << "Parsing ember file " << opt.Input() << "\n";

	if (!ParseEmberFile(parser, opt.Input(), embers))
		return false;

	cout << "Finished parsing.\n";

	if (embers.size() <= 1)
	{
		cout << "Read " << embers.size() << " embers from file. At least 2 required to animate, exiting.\n";
		return false;
	}

	if (!Find(opt.Format(), "jpg") &&
			!Find(opt.Format(), "png") &&
#ifdef _WIN32
			!Find(opt.Format(), "bmp") &&
#endif
			!Find(opt.Format(), "exr"))
	{
#ifdef _WIN32
		cout << "Format must be bmp, jpg, png, png16 or exr, not " << opt.Format() << ". Setting to png.\n";
#else
		cout << "Format must be jpg, png, png16 or exr, not " << opt.Format() << ". Setting to png.\n";
#endif
		opt.Format("png");
	}

	if (opt.AspectRatio() < 0)
	{
		cout << "Invalid pixel aspect ratio " << opt.AspectRatio() << "\n. Must be positive, setting to 1.\n";
		opt.AspectRatio(1);
	}

	if (opt.Dtime() < 1)
	{
		cout << "Warning: dtime must be positive, not " << opt.Dtime() << ". Setting to 1.\n";
		opt.Dtime(1);
	}

	if (opt.Frame() != UINT_MAX)
	{
		if (opt.FirstFrame() != UINT_MAX || opt.LastFrame() != UINT_MAX)
		{
			cout << "Cannot specify both frame and begin or end.\n";
			return false;
		}

		if (opt.Frame())
			opt.FirstFrame(opt.Frame() - 1);

		opt.LastFrame(opt.FirstFrame() + 1);
	}

	//Prep all embers, by ensuring they:
	//-Are sorted by time.
	//-Do not have a dimension of 0.
	//-Do not have a memory requirement greater than max uint.
	//-Have quality and size scales applied, if present.
	//-Have equal dimensions.
	for (i = 0; i < embers.size(); i++)
	{
		auto& ember = embers[i];
		auto& emberm1 = embers[i - 1];

		if (i > 0 && ember.m_Time <= emberm1.m_Time)
		{
			if (!unsorted)
				firstUnsortedIndex = i;

			unsorted = true;
		}

		if (i > 0 && ember.m_Time == emberm1.m_Time)
		{
			cout << "Image " << i << " time of " << ember.m_Time << " equaled previous image time of " << emberm1.m_Time << ". Adjusting up by 1.\n";
			ember.m_Time++;
		}

		if (opt.Supersample() > 0)
			ember.m_Supersample = opt.Supersample();

		if (opt.TemporalSamples() > 0)
			ember.m_TemporalSamples = opt.TemporalSamples();

		if (opt.Quality() > 0)
			ember.m_Quality = T(opt.Quality());

		if (opt.DeMin() > -1)
			ember.m_MinRadDE = T(opt.DeMin());

		if (opt.DeMax() > -1)
			ember.m_MaxRadDE = T(opt.DeMax());

		ember.m_Quality *= T(opt.QualityScale());

		if (opt.SizeScale() != 1.0)
		{
			ember.m_FinalRasW = size_t(T(ember.m_FinalRasW) * opt.SizeScale());
			ember.m_FinalRasH = size_t(T(ember.m_FinalRasH) * opt.SizeScale());
			ember.m_PixelsPerUnit *= T(opt.SizeScale());
		}
		else if (opt.WidthScale() != 1.0 || opt.HeightScale() != 1.0)
		{
			auto scaleType = eScaleType::SCALE_NONE;

			if (ToLower(opt.ScaleType()) == "width")
				scaleType = eScaleType::SCALE_WIDTH;
			else if (ToLower(opt.ScaleType()) == "height")
				scaleType = eScaleType::SCALE_HEIGHT;
			else if (ToLower(opt.ScaleType()) != "none")
				cout << "Scale type must be width height or none. Setting to none.\n";

			const auto w = std::max<size_t>(size_t(ember.m_OrigFinalRasW * opt.WidthScale()), 10);
			const auto h = std::max<size_t>(size_t(ember.m_OrigFinalRasH * opt.HeightScale()), 10);
			ember.SetSizeAndAdjustScale(w, h, false, scaleType);
		}
		else if (opt.Width() || opt.Height())
		{
			auto scaleType = eScaleType::SCALE_NONE;

			if (ToLower(opt.ScaleType()) == "width")
				scaleType = eScaleType::SCALE_WIDTH;
			else if (ToLower(opt.ScaleType()) == "height")
				scaleType = eScaleType::SCALE_HEIGHT;
			else if (ToLower(opt.ScaleType()) != "none")
				cout << "Scale type must be width height or none. Setting to none.\n";

			auto w = opt.Width() ? opt.Width() : ember.m_OrigFinalRasW;
			auto h = opt.Height() ? opt.Height() : ember.m_OrigFinalRasH;
			ember.SetSizeAndAdjustScale(w, h, false, scaleType);
		}

		//Cast to double in case the value exceeds 2^32.
		const auto imageMem = 4 * static_cast<double>(ember.m_FinalRasW)
							  * static_cast<double>(ember.m_FinalRasH) * static_cast<double>(renderers[0]->BytesPerChannel());
		const auto maxMem = pow(2.0, static_cast<double>((sizeof(void*) * 8) - 1));

		if (imageMem > maxMem)//Ensure the max amount of memory for a process isn't exceeded.
		{
			cout << "Image " << i << " size > " << maxMem << ". Setting to 1920 x 1080.\n";
			ember.m_FinalRasW = 1920;
			ember.m_FinalRasH = 1080;
		}

		if (ember.m_FinalRasW == 0 || ember.m_FinalRasH == 0)
		{
			cout << "Warning: Output image " << i << " has dimension 0: " << ember.m_FinalRasW  << ", " << ember.m_FinalRasH << ". Setting to 1920 x 1080.\n";
			ember.m_FinalRasW = 1920;
			ember.m_FinalRasH = 1080;
		}

		if ((ember.m_FinalRasW != embers[0].m_FinalRasW) ||
				(ember.m_FinalRasH != embers[0].m_FinalRasH))
		{
			cout << "Warning: flame " << i << " at time " << ember.m_Time << " size mismatch. (" << ember.m_FinalRasW << ", " << ember.m_FinalRasH <<
				 ") should be (" << embers[0].m_FinalRasW << ", " << embers[0].m_FinalRasH << "). Setting to " << embers[0].m_FinalRasW << ", " << embers[0].m_FinalRasH << ".\n";
			ember.m_FinalRasW = embers[0].m_FinalRasW;
			ember.m_FinalRasH = embers[0].m_FinalRasH;
		}
	}

	if (unsorted)
	{
		cout << "Embers were unsorted by time. First out of order index was " << firstUnsortedIndex << ". Sorting.\n";
		std::sort(embers.begin(), embers.end(), &CompareEmbers<T>);
	}

	if (opt.Frame() == UINT_MAX)
	{
		if (opt.FirstFrame() == UINT_MAX)
			opt.FirstFrame(size_t(embers[0].m_Time));

		if (opt.LastFrame() == UINT_MAX)
			opt.LastFrame(ClampGte<size_t>(size_t(embers.back().m_Time),
										   opt.FirstFrame() + opt.Dtime()));//Make sure the final value is at least first frame + dtime.
	}

	//Final setup steps before running.
	padding = uint(std::log10(double(embers.size()))) + 1;

	for (auto& r : renderers)
	{
		r->SetExternalEmbersPointer(&embers);//All will share a pointer to the original vector to conserve memory with large files. Ok because the vec doesn't get modified.
		r->EarlyClip(opt.EarlyClip());
		r->YAxisUp(opt.YAxisUp());
		r->LockAccum(opt.LockAccum());
		r->PixelAspectRatio(T(opt.AspectRatio()));
		r->Priority(eThreadPriority(Clamp<intmax_t>(intmax_t(opt.Priority()), intmax_t(eThreadPriority::LOWEST), intmax_t(eThreadPriority::HIGHEST))));
	}

	const auto doBmp = Find(opt.Format(), "bmp");
	const auto doJpg = Find(opt.Format(), "jpg");
	const auto doExr16 = Find(opt.Format(), "exr");
	const auto doExr32 = Find(opt.Format(), "exr32");
	const auto doPng8 = Find(opt.Format(), "png");
	const auto doPng16 = Find(opt.Format(), "png16");
	const auto doOnlyPng8 = doPng8 && !doPng16;
	const auto doOnlyExr16 = doExr16 && !doExr32;
	std::function<void (vector<v4F>&, string, EmberImageComments, size_t, size_t, size_t)> saveFunc = [&](vector<v4F>& finalImage,
			string baseFilename,//These are copies because this will be launched in a thread.
			EmberImageComments comments,
			size_t w,
			size_t h,
			size_t chan)
	{
		const auto finalImagep = finalImage.data();
		const auto size = w * h;
		vector<unsigned char> rgb8Image;
		vector<std::thread> writeFileThreads;
		writeFileThreads.reserve(6);

		if (doBmp || doJpg)
		{
			rgb8Image.resize(size * 3);
			Rgba32ToRgb8(finalImagep, rgb8Image.data(), w, h);

			if (doBmp)
			{
				writeFileThreads.push_back(std::thread([&]()
				{
					const auto fn = baseFilename + ".bmp";
					VerbosePrint("Writing " + fn);
					const auto writeSuccess = WriteBmp(fn.c_str(), rgb8Image.data(), w, h);

					if (!writeSuccess)
						cout << "Error writing " << fn << "\n";
				}));
			}

			if (doJpg)
			{
				writeFileThreads.push_back(std::thread([&]()
				{
					const auto fn = baseFilename + ".jpg";
					VerbosePrint("Writing " + fn);
					const auto writeSuccess = WriteJpeg(fn.c_str(), rgb8Image.data(), w, h, int(opt.JpegQuality()), opt.EnableComments(), comments, opt.Id(), opt.Url(), opt.Nick());

					if (!writeSuccess)
						cout << "Error writing " << fn << "\n";
				}));
			}
		}

		if (doPng8)
		{
			bool doBothPng = doPng16 && (opt.Format().find("png") != opt.Format().rfind("png"));

			if (doBothPng || doOnlyPng8)//8-bit PNG.
			{
				writeFileThreads.push_back(std::thread([&]()
				{
					const auto fn = baseFilename + ".png";
					VerbosePrint("Writing " + fn);
					vector<unsigned char> rgba8Image(size * 4);
					Rgba32ToRgba8(finalImagep, rgba8Image.data(), w, h, opt.Transparency());
					const auto writeSuccess = WritePng(fn.c_str(), rgba8Image.data(), w, h, 1, opt.EnableComments(), comments, opt.Id(), opt.Url(), opt.Nick());

					if (!writeSuccess)
						cout << "Error writing " << fn << "\n";
				}));
			}

			if (doPng16)//16-bit PNG.
			{
				writeFileThreads.push_back(std::thread([&]()
				{
					const auto suffix = opt.Suffix();
					auto fn = baseFilename;

					if (doBothPng)//Add suffix if they specified both PNG.
					{
						VerbosePrint("Doing both PNG formats, so adding suffix _p16 to avoid overwriting the same file.");
						fn += "_p16";
					}

					fn += ".png";
					VerbosePrint("Writing " + fn);
					vector<glm::uint16> rgba16Image(size * 4);
					Rgba32ToRgba16(finalImagep, rgba16Image.data(), w, h, opt.Transparency());
                    const auto writeSuccess = WritePng(fn.c_str(), reinterpret_cast<unsigned char*>(rgba16Image.data()), w, h, 2, opt.EnableComments(), comments, opt.Id(), opt.Url(), opt.Nick());

					if (!writeSuccess)
						cout << "Error writing " << fn << "\n";
				}));
			}
		}

		if (doExr16)
		{
			bool doBothExr = doExr32 && (opt.Format().find("exr") != opt.Format().rfind("exr"));

			if (doBothExr || doOnlyExr16)//16-bit EXR
			{
				writeFileThreads.push_back(std::thread([&]()
				{
					const auto fn = baseFilename + ".exr";
					VerbosePrint("Writing " + fn);
					vector<Rgba> rgba32Image(size);
					Rgba32ToRgbaExr(finalImagep, rgba32Image.data(), w, h, opt.Transparency());
					const auto writeSuccess = WriteExr16(fn.c_str(), rgba32Image.data(), w, h, opt.EnableComments(), comments, opt.Id(), opt.Url(), opt.Nick());

					if (!writeSuccess)
						cout << "Error writing " << fn << "\n";
				}));
			}

			if (doExr32)//32-bit EXR.
			{
				writeFileThreads.push_back(std::thread([&]()
				{
					const auto suffix = opt.Suffix();
					auto fn = baseFilename;

					if (doBothExr)//Add suffix if they specified both EXR.
					{
						VerbosePrint("Doing both EXR formats, so adding suffix _exr32 to avoid overwriting the same file.");
						fn += "_exr32";
					}

					fn += ".exr";
					VerbosePrint("Writing " + fn);
					vector<float> r(size);
					vector<float> g(size);
					vector<float> b(size);
					vector<float> a(size);
					Rgba32ToRgba32Exr(finalImagep, r.data(), g.data(), b.data(), a.data(), w, h, opt.Transparency());
					const auto writeSuccess = WriteExr32(fn.c_str(),
														 r.data(),
														 g.data(),
														 b.data(),
														 a.data(),
														 w, h, opt.EnableComments(), comments, opt.Id(), opt.Url(), opt.Nick());

					if (!writeSuccess)
						cout << "Error writing " << fn << "\n";
				}));
			}
		}

		Join(writeFileThreads);
	};
	atomfTime.store(opt.FirstFrame());
	std::function<void(size_t)> iterFunc = [&](size_t index)
	{
        size_t ftime;
		RendererBase* renderer = renderers[index].get();
		ostringstream os;
		EmberStats stats;
		EmberImageComments comments;
		Ember<T> centerEmber;
		ThreadedWriter threadedWriter(16);
		os.imbue(std::locale(""));

		//The conditions of this loop use atomics to synchronize when running on multiple GPUs.
		//The order is reversed from the usual loop: rather than compare and increment the counter,
		//it's incremented, then compared. This is done to ensure the GPU on this thread "claims" this
		//frame before working on it.
		//The mechanism for incrementing is:
		//	Do an atomic add, which returns the previous value.
		//	Add the time increment Dtime() to the return value to mimic what the new atomic value should be.
		//	Assign the result to the ftime counter.
		//	Do a <= comparison to LastFrame().
		//		If true, enter the loop and immediately decrement the counter by Dtime() to make up for the fact
		//		that it was first incremented before comparing.
		while ((ftime = (atomfTime.fetch_add(opt.Dtime()) + opt.Dtime())) <= opt.LastFrame())
		{
			const auto localTime = static_cast<T>(ftime) - opt.Dtime();
			const auto baseFilename = MakeAnimFilename(inputPath, opt.Prefix(), opt.Suffix(), "", padding, size_t(localTime));

			if (opt.IgnoreExisting())
			{
				auto doRender = false;

				if (doBmp)
				{
					const auto fn = baseFilename + ".bmp";

					if (!FileExists(fn))
						doRender = true;
				}

				if (!doRender && doJpg)
				{
					const auto fn = baseFilename + ".jpg";

					if (!FileExists(fn))
						doRender = true;
				}

				if (!doRender && doPng8)
				{
					bool doBothPng = doPng16 && (opt.Format().find("png") != opt.Format().rfind("png"));

					if (doBothPng || doOnlyPng8)//8-bit PNG.
					{
						const auto fn = baseFilename + ".png";

						if (!FileExists(fn))
							doRender = true;
					}

					if (!doRender && doPng16)//16-bit PNG.
					{
						auto fn = baseFilename;

						if (doBothPng)//Add suffix if they specified both PNG.
							fn += "_p16";

						fn += ".png";

						if (!FileExists(fn))
							doRender = true;
					}
				}

				if (!doRender && doExr16)
				{
					bool doBothExr = doExr32 && (opt.Format().find("exr") != opt.Format().rfind("exr"));

					if (doBothExr || doOnlyExr16)//16-bit EXR
					{
						const auto fn = baseFilename + ".exr";

						if (!FileExists(fn))
							doRender = true;
					}

					if (!doRender && doExr32)//32-bit EXR.
					{
						auto fn = baseFilename;

						if (doBothExr)//Add suffix if they specified both EXR.
							fn += "_exr32";

						fn += ".exr"; if (!FileExists(fn))
							doRender = true;
					}
				}

				if (!doRender)
				{
					VerbosePrint("Skipping " + baseFilename + " because --ignore-existing was specified and all of the files with the requested extensions already exist.");
					continue;
				}
			}

			if (opt.Verbose() && ((opt.LastFrame() - opt.FirstFrame()) / opt.Dtime() >= 1))
			{
				rlg l(verboseCs);
				cout << "Time = " << ftime << " / " << opt.LastFrame() << " / " << opt.Dtime() << "\n";
			}

			renderer->Reset();
			auto threadIndex = threadedWriter.Increment();
			auto threadImage = threadedWriter.GetImage(opt.ThreadedWrite() ? threadIndex : 0);

			if ((renderer->Run(*threadImage, localTime) != eRenderStatus::RENDER_OK) || renderer->Aborted() || threadImage->empty())
			{
				cout << "Error: image rendering failed, aborting.\n";
				renderer->DumpErrorReport();//Something went wrong, print errors.
				atomfTime.store(opt.LastFrame() + 1);//Abort all threads if any of them encounter an error.
				break;
			}

			if (opt.WriteGenome())
			{
				const auto flameName = MakeAnimFilename(inputPath, opt.Prefix(), opt.Suffix(), ".flame", padding, size_t(localTime));

				if (opt.Verbose())
				{
					rlg l(verboseCs);
					cout << "Writing " << flameName << "\n";
				}

				interpolater.Interpolate(embers, localTime, 0, centerEmber);//Get center flame.
				emberToXml.Save(flameName, centerEmber, opt.PrintEditDepth(), true, opt.HexPalette(), true, false, false);
				centerEmber.Clear();
			}

			stats = renderer->Stats();
			comments = renderer->ImageComments(stats, opt.PrintEditDepth(), true);
			os.str("");
			const auto iterCount = renderer->TotalIterCount(1);
			os << comments.m_NumIters << " / " << iterCount << " (" << std::fixed << std::setprecision(2) << ((static_cast<double>(stats.m_Iters) / static_cast<double>(iterCount)) * 100) << "%)";

			if (opt.Verbose())
			{
				rlg l(verboseCs);
				cout << "\nIters ran/requested: " + os.str() << "\n";

				if (!opt.EmberCL()) cout << "Bad values: " << stats.m_Badvals << "\n";

				cout << "Render time: " << t.Format(stats.m_RenderMs) << "\n";
				cout << "Pure iter time: " << t.Format(stats.m_IterMs) << "\n";
				cout << "Iters/sec: " << size_t(stats.m_Iters / (stats.m_IterMs / 1000.0)) << "\n";
			}

			//Run image writing in a thread. Although doing it this way duplicates the final output memory, it saves a lot of time
			//when running with OpenCL. Call join() to ensure the previous thread call has completed.
			//Join(writeThread);

			if (opt.ThreadedWrite())//Copies of all but the first parameter are passed to saveFunc(), to avoid conflicting with those values changing when starting the render for the next image.
			{
				auto writeThread = std::thread(saveFunc, std::ref(*threadImage), baseFilename, comments, renderer->FinalRasW(), renderer->FinalRasH(), renderer->NumChannels());
				threadedWriter.SetThread(threadIndex, writeThread);
			}
			else
				saveFunc(*threadImage, baseFilename, comments, renderer->FinalRasW(), renderer->FinalRasH(), renderer->NumChannels());//Will always use the first index, thereby not requiring more memory.
		}

		threadedWriter.JoinAll();//One final check to make sure all writing is done before exiting this thread.
	};
	threadVec.reserve(renderers.size());

	for (size_t r = 0; r < renderers.size(); r++)
	{
		threadVec.push_back(std::thread([&](size_t dev)
		{
			iterFunc(dev);
		}, r));
	}

	Join(threadVec);
	t.Toc("\nFinished in: ", true);
	return true;
}

/// <summary>
/// Main program entry point for EmberAnimate.exe.
/// </summary>
/// <param name="argc">The number of command line arguments passed</param>
/// <param name="argv">The command line arguments passed</param>
/// <returns>0 if successful, else 1.</returns>
int _tmain(int argc, _TCHAR* argv[])
{
	bool b = false;
	EmberOptions opt;
	//Required for large allocs, else GPU memory usage will be severely limited to small sizes.
	//This must be done in the application and not in the EmberCL DLL.
#ifdef _WIN32
	_putenv_s("GPU_MAX_ALLOC_PERCENT", "100");
#else
	putenv(const_cast<char*>("GPU_MAX_ALLOC_PERCENT=100"));
#endif
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

	if (!opt.Populate(argc, argv, eOptionUse::OPT_USE_ANIMATE))
	{
		const auto palf = PaletteList<float>::Instance();
#ifdef DO_DOUBLE

		if (!opt.Sp())
			b = EmberAnimate<double>(argc, argv, opt);
		else
#endif
			b = EmberAnimate<float>(argc, argv, opt);

		cout << std::flush;
	}

	return b ? 0 : 1;
}
