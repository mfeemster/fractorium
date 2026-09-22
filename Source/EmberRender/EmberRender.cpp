#include "EmberCommonPch.h"
#include "EmberRender.h"
#include "JpegUtils.h"
#include <xmmintrin.h>
#include <pmmintrin.h>

/// <summary>
/// The core of the EmberRender.exe program.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="opt">A populated EmberOptions object which specifies all program options to be used</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool EmberRender(int argc, _TCHAR* argv[], EmberOptions& opt)
{
	auto info = EmberCLns::OpenCLInfo::Instance();
	std::cout.imbue(std::locale(""));

	if (opt.DumpArgs())
		cout << opt.GetValues(eOptionUse::OPT_USE_RENDER) << "\n";

	if (opt.OpenCLInfo())
	{
		cout << "\nOpenCL Info: \n";
		cout << info->DumpInfo();
		return true;
	}

	VerbosePrint("Using " << (sizeof(T) == sizeof(float) ? "single" : "double") << " precision.");
	Timing t;
	uint padding;
	size_t i;
	size_t strips;
	size_t iterCount;
	string inputPath = GetPath(opt.Input());
	ostringstream os;
	pair<size_t, size_t> p;
	vector<Ember<T>> embers;
	vector<v4F> finalImage;
	EmberStats stats;
	EmberReport emberReport;
	EmberImageComments comments;
	XmlToEmber<T> parser;
	EmberToXml<T> emberToXml;
	vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>> randVec;
	const vector<pair<size_t, size_t>> devices = Devices(opt.Devices());
	auto progress = make_unique<RenderProgress<T>>();
	unique_ptr<Renderer<T, float>> renderer(CreateRenderer<T>(opt.EmberCL() ? eRendererType::OPENCL_RENDERER : eRendererType::CPU_RENDERER, devices, false, 0, emberReport));
	vector<string> errorReport = emberReport.ErrorReport();
	auto fullpath = GetExePath(argv[0]);
	Compat::m_Compat = opt.Flam3Compat();

	if (!errorReport.empty())
		emberReport.DumpErrorReport();

	if (!renderer.get())
	{
		cout << "Renderer creation failed, exiting.\n" ;
		return false;
	}

	if (opt.EmberCL() && renderer->RendererType() != eRendererType::OPENCL_RENDERER)//OpenCL init failed, so fall back to CPU.
		opt.EmberCL(false);

	auto rendererCL = dynamic_cast<RendererCL<T, float>*>(renderer.get());

	if (rendererCL)
	{
		rendererCL->OptAffine(true);//Optimize empty affines for final renderers, this is normally false for the interactive renderer.
		rendererCL->SubBatchPercentPerThread(float(opt.SBPctPerTh()));
	}

	if (!InitPaletteList<float>(fullpath, opt.PalettePath()))//For any modern flames, the palette isn't used. This is for legacy purposes and should be removed.
		return false;

	if (!ParseEmberFile(parser, opt.Input(), embers))
		return false;

	if (!opt.EmberCL())
	{
		if (opt.ThreadCount() == 0)
		{
			cout << "Using " << Timing::ProcessorCount() << " automatically detected threads.\n";
			opt.ThreadCount(Timing::ProcessorCount());
		}
		else
		{
			cout << "Using " << opt.ThreadCount() << " manually specified threads.\n";
		}

		renderer->ThreadCount(opt.ThreadCount(), opt.IsaacSeed() != "" ? opt.IsaacSeed().c_str() : nullptr);
	}
	else
	{
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
		renderer->ThreadCount(opt.ThreadCount(), opt.IsaacSeed() != "" ? opt.IsaacSeed().c_str() : nullptr);

		if (opt.InsertPalette())
		{
			cout << "Inserting palette not supported with OpenCL, insertion will not take place.\n";
			opt.InsertPalette(false);
		}
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

	if (!opt.Out().empty() && (embers.size() > 1))
	{
		cout << "Single output file " << opt.Out() << " specified for multiple images. Changing to use prefix of badname-changethis instead. Always specify prefixes when reading a file with multiple embers.\n";
		opt.Out("");
		opt.Prefix("badname-changethis");
	}

	//Final setup steps before running.
	os.imbue(std::locale(""));
	padding = uint(std::log10(static_cast<double>(embers.size()))) + 1;
	renderer->EarlyClip(opt.EarlyClip());
	renderer->YAxisUp(opt.YAxisUp());
	renderer->LockAccum(opt.LockAccum());
	renderer->InsertPalette(opt.InsertPalette());
	renderer->PixelAspectRatio(T(opt.AspectRatio()));
	renderer->Priority(eThreadPriority(Clamp<intmax_t>(intmax_t(opt.Priority()), intmax_t(eThreadPriority::LOWEST), intmax_t(eThreadPriority::HIGHEST))));
	renderer->Callback(opt.DoProgress() ? progress.get() : nullptr);

	for (i = 0; i < embers.size(); i++)
	{
		auto& ember = embers[i];

		if (opt.Verbose() && embers.size() > 1)
			cout << "\nFlame = " << i + 1 << "/" << embers.size() << "\n";
		else if (embers.size() > 1)
			VerbosePrint("\n");

		if (opt.Supersample() > 0)
			ember.m_Supersample = opt.Supersample();

		if (opt.Quality() > 0)
			ember.m_Quality = T(opt.Quality());

		if (opt.DeMin() > -1)
			ember.m_MinRadDE = T(opt.DeMin());

		if (opt.DeMax() > -1)
			ember.m_MaxRadDE = T(opt.DeMax());

		ember.m_TemporalSamples = 1;//Force temporal samples to 1 for render.
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

			auto w = std::max<size_t>(size_t(ember.m_OrigFinalRasW * opt.WidthScale()), 10);
			auto h = std::max<size_t>(size_t(ember.m_OrigFinalRasH * opt.HeightScale()), 10);
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

		if (ember.m_FinalRasW == 0 || ember.m_FinalRasH == 0)
		{
			cout << "Output image " << i << " has dimension 0: " << ember.m_FinalRasW  << ", " << ember.m_FinalRasH << ". Setting to 1920 x 1080.\n";
			ember.m_FinalRasW = 1920;
			ember.m_FinalRasH = 1080;
		}

		//Cast to double in case the value exceeds 2^32.
		const auto imageMem = static_cast<double>(renderer->NumChannels()) * static_cast<double>(ember.m_FinalRasW)
							  * static_cast<double>(ember.m_FinalRasH) * static_cast<double>(renderer->BytesPerChannel());
		const auto maxMem = pow(2.0, static_cast<double>((sizeof(void*) * 8) - 1));

		if (imageMem > maxMem)//Ensure the max amount of memory for a process is not exceeded.
		{
			cout << "Image " << i << " size > " << maxMem << ". Setting to 1920 x 1080.\n";
			ember.m_FinalRasW = 1920;
			ember.m_FinalRasH = 1080;
		}

		stats.Clear();
		renderer->SetEmber(ember, eProcessAction::FULL_RENDER, true);
		renderer->PrepFinalAccumVector(finalImage);//Must manually call this first because it could be erroneously made smaller due to strips if called inside Renderer::Run().

		if (opt.Strips() > 1)
		{
			strips = opt.Strips();
		}
		else
		{
			p = renderer->MemoryRequired(1, true, false);//No threaded write for render, only for animate.
			strips = CalcStrips(static_cast<double>(p.second), static_cast<double>(renderer->MemoryAvailable()), opt.UseMem());

			if (strips > 1)
				VerbosePrint("Setting strips to " << strips << " with specified memory usage of " << opt.UseMem());
		}

		strips = VerifyStrips(ember.m_FinalRasH, strips,
		[&](const string& s) { cout << s << "\n"; },  //Greater than height.
		[&](const string& s) { cout << s << "\n"; },  //Mod height != 0.
		[&](const string& s) { cout << s << "\n"; });  //Final strips value to be set.
		//For testing incremental renderer.
		//int sb = 1;
		//bool resume = false, success = false;
		//do
		//{
		//	success = renderer->Run(finalImage, 0, sb, false/*resume == false*/) == RENDER_OK;
		//	sb++;
		//	resume = true;
		//}
		//while (success && renderer->ProcessState() != ACCUM_DONE);
		//for (auto gbw = 64; gbw <= 64; gbw <<= 1)
		{
			//for (auto gbh = 2; gbh <= 64; gbh <<= 1)
			{
				//if (rendererCL)
				//{
				//	VerbosePrint("Running OpenCL grid blocks of " << gbw << "x" << gbh);
				//	rendererCL->IterBlocksWide(gbw);
				//	rendererCL->IterBlocksHigh(gbh);
				//}
				stats.Clear();
				StripsRender<T>(renderer.get(), ember, finalImage, 0, strips, opt.YAxisUp(),
								[&](size_t strip)//Pre strip.
				{
					if (opt.Verbose() && (strips > 1) && strip > 0)
						cout << "\n";

					if (strips > 1)
						VerbosePrint("Strip = " << (strip + 1) << "/" << strips);
				},
				[&](size_t strip)//Post strip.
				{
					progress->Clear();
					stats += renderer->Stats();
				},
				[&](size_t strip)//Error.
				{
					cout << "Error: image rendering failed, skipping to next image.\n";
					renderer->DumpErrorReport();//Something went wrong, print errors.
				},
				//Final strip.
				//Original wrote every strip as a full image which could be very slow with many large images.
				//Only write once all strips for this image are finished.
				[&](Ember<T>& finalEmber)
				{
					//TotalIterCount() is actually using ScaledQuality() which does not get reset upon ember assignment,
					//so it ends up using the correct value for quality * strips.
					iterCount = renderer->TotalIterCount(1);
					comments = renderer->ImageComments(stats, opt.PrintEditDepth(), true);
					os.str("");
					os << comments.m_NumIters << " / " << iterCount << " (" << std::fixed << std::setprecision(2) << ((static_cast<double>(stats.m_Iters) / static_cast<double>(iterCount)) * 100) << "%)";
					VerbosePrint("\nIters ran/requested: " + os.str());

					if (!opt.EmberCL())
						VerbosePrint("Bad values: " << stats.m_Badvals);

					VerbosePrint("Render time: " + t.Format(stats.m_RenderMs));
					VerbosePrint("Pure iter time: " + t.Format(stats.m_IterMs));
					VerbosePrint("Iters/sec: " << size_t(stats.m_Iters / (stats.m_IterMs / 1000.0)) << "\n");
					const auto useName = opt.NameEnable() && !finalEmber.m_Name.empty();
					const auto finalImagep = finalImage.data();
					const auto size = finalEmber.m_FinalRasW * finalEmber.m_FinalRasH;
					const auto doBmp = Find(opt.Format(), "bmp");
					const auto doJpg = Find(opt.Format(), "jpg");
					const auto doExr16 = Find(opt.Format(), "exr");
					const auto doExr32 = Find(opt.Format(), "exr32");
					const auto doPng8 = Find(opt.Format(), "png");
					const auto doPng16 = Find(opt.Format(), "png16");
					const auto doOnlyPng8 = doPng8 && !doPng16;
					const auto doOnlyExr16 = doExr16 && !doExr32;
					vector<unsigned char> rgb8Image;
					vector<std::thread> writeFileThreads;
					writeFileThreads.reserve(6);

					if (doBmp || doJpg)
					{
						rgb8Image.resize(size * 3);
						Rgba32ToRgb8(finalImagep, rgb8Image.data(), finalEmber.m_FinalRasW, finalEmber.m_FinalRasH);

						if (doBmp)
						{
							writeFileThreads.push_back(std::thread([&]()
							{
								const auto filename = MakeSingleFilename(inputPath, opt.Out(), finalEmber.m_Name, opt.Prefix(), opt.Suffix(), "bmp", padding, i, useName);
								VerbosePrint("Writing " + filename);
								const auto writeSuccess = WriteBmp(filename.c_str(), rgb8Image.data(), finalEmber.m_FinalRasW, finalEmber.m_FinalRasH);

								if (!writeSuccess)
									cout << "Error writing " << filename << "\n";
							}));
						}

						if (doJpg)
						{
							writeFileThreads.push_back(std::thread([&]()
							{
								const auto filename = MakeSingleFilename(inputPath, opt.Out(), finalEmber.m_Name, opt.Prefix(), opt.Suffix(), "jpg", padding, i, useName);
								VerbosePrint("Writing " + filename);
								const auto writeSuccess = WriteJpeg(filename.c_str(), rgb8Image.data(), finalEmber.m_FinalRasW, finalEmber.m_FinalRasH, int(opt.JpegQuality()), opt.EnableComments(), comments, opt.Id(), opt.Url(), opt.Nick());

								if (!writeSuccess)
									cout << "Error writing " << filename << "\n";
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
								const auto filename = MakeSingleFilename(inputPath, opt.Out(), finalEmber.m_Name, opt.Prefix(), opt.Suffix(), "png", padding, i, useName);
								VerbosePrint("Writing " + filename);
								vector<unsigned char> rgba8Image(size * 4);
								Rgba32ToRgba8(finalImagep, rgba8Image.data(), finalEmber.m_FinalRasW, finalEmber.m_FinalRasH, opt.Transparency());
								const auto writeSuccess = WritePng(filename.c_str(), rgba8Image.data(), finalEmber.m_FinalRasW, finalEmber.m_FinalRasH, 1, opt.EnableComments(), comments, opt.Id(), opt.Url(), opt.Nick());

								if (!writeSuccess)
									cout << "Error writing " << filename << "\n";
							}));
						}

						if (doPng16)//16-bit PNG.
						{
							writeFileThreads.push_back(std::thread([&]()
							{
								auto suffix = opt.Suffix();

								if (doBothPng)//Add suffix if they specified both PNG.
								{
									VerbosePrint("Doing both PNG formats, so adding suffix _p16 to avoid overwriting the same file.");
									suffix += "_p16";
								}

								const auto filename = MakeSingleFilename(inputPath, opt.Out(), finalEmber.m_Name, opt.Prefix(), suffix, "png", padding, i, useName);
								VerbosePrint("Writing " + filename);
								vector<glm::uint16> rgba16Image(size * 4);
								Rgba32ToRgba16(finalImagep, rgba16Image.data(), finalEmber.m_FinalRasW, finalEmber.m_FinalRasH, opt.Transparency());
								const auto writeSuccess = WritePng(filename.c_str(), (unsigned char*)rgba16Image.data(), finalEmber.m_FinalRasW, finalEmber.m_FinalRasH, 2, opt.EnableComments(), comments, opt.Id(), opt.Url(), opt.Nick());

								if (!writeSuccess)
									cout << "Error writing " << filename << "\n";
							}));
						}
					}

					if (doExr16)
					{
						const auto doBothExr = doExr32 && (opt.Format().find("exr") != opt.Format().rfind("exr"));

						if (doBothExr || doOnlyExr16)//16-bit EXR.
						{
							writeFileThreads.push_back(std::thread([&]()
							{
								const auto filename = MakeSingleFilename(inputPath, opt.Out(), finalEmber.m_Name, opt.Prefix(), opt.Suffix(), "exr", padding, i, useName);
								VerbosePrint("Writing " + filename);
								vector<Rgba> rgba32Image(size);
								Rgba32ToRgbaExr(finalImagep, rgba32Image.data(), finalEmber.m_FinalRasW, finalEmber.m_FinalRasH, opt.Transparency());
								const auto writeSuccess = WriteExr16(filename.c_str(),
																	 rgba32Image.data(),
																	 finalEmber.m_FinalRasW, finalEmber.m_FinalRasH, opt.EnableComments(), comments, opt.Id(), opt.Url(), opt.Nick());

								if (!writeSuccess)
									cout << "Error writing " << filename << "\n";
							}));
						}

						if (doExr32)//32-bit EXR.
						{
							writeFileThreads.push_back(std::thread([&]()
							{
								auto suffix = opt.Suffix();

								if (doBothExr)//Add suffix if they specified both EXR.
								{
									VerbosePrint("Doing both EXR formats, so adding suffix _exr32 to avoid overwriting the same file.");
									suffix += "_exr32";
								}

								const auto filename = MakeSingleFilename(inputPath, opt.Out(), finalEmber.m_Name, opt.Prefix(), suffix, "exr", padding, i, useName);
								VerbosePrint("Writing " + filename);
								vector<float> r(size);
								vector<float> g(size);
								vector<float> b(size);
								vector<float> a(size);
								Rgba32ToRgba32Exr(finalImagep, r.data(), g.data(), b.data(), a.data(), finalEmber.m_FinalRasW, finalEmber.m_FinalRasH, opt.Transparency());
								const auto writeSuccess = WriteExr32(filename.c_str(),
																	 r.data(),
																	 g.data(),
																	 b.data(),
																	 a.data(),
																	 finalEmber.m_FinalRasW, finalEmber.m_FinalRasH, opt.EnableComments(), comments, opt.Id(), opt.Url(), opt.Nick());

								if (!writeSuccess)
									cout << "Error writing " << filename << "\n";
							}));
						}
					}

					Join(writeFileThreads);
				});
				//if (!rendererCL)
				//	break;
			}
			//	if (!rendererCL)
			//break;
		}

		if (opt.EmberCL() && opt.DumpKernel())
		{
			if (rendererCL)
			{
				cout << "Iteration kernel:\n" <<
					 rendererCL->IterKernel() << "\n\n" <<
					 "Density filter kernel:\n" <<
					 rendererCL->DEKernel() << "\n\n" <<
					 "Final accumulation kernel:\n" <<
					 rendererCL->FinalAccumKernel() << "\n";
			}
		}

		VerbosePrint("Done.");
	}

	t.Toc("\nFinished in: ", true);
	return true;
}

/// <summary>
/// Main program entry point for EmberRender.exe.
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
	//_putenv_s("GPU_FORCE_64BIT_PTR", "1");
#else
	putenv(const_cast<char*>("GPU_MAX_ALLOC_PERCENT=100"));
#endif
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

	if (!opt.Populate(argc, argv, eOptionUse::OPT_USE_RENDER))
	{
		auto palf = PaletteList<float>::Instance();
#ifdef DO_DOUBLE

		if (!opt.Sp())
			b = EmberRender<double>(argc, argv, opt);
		else
#endif
			b = EmberRender<float>(argc, argv, opt);

		cout << std::flush;
	}

	return b ? 0 : 1;
}
