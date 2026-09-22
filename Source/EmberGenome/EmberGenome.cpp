#include "EmberCommonPch.h"
#include "EmberGenome.h"
#include <xmmintrin.h>
#include <immintrin.h>
#include <pmmintrin.h>

/// <summary>
/// Set various default test values on the passed in ember.
/// </summary>
/// <param name="ember">The ember to test</param>
template <typename T>
void SetDefaultTestValues(Ember<T>& ember)
{
	ember.m_Time = 0.0;
	ember.m_Interp = eInterp::EMBER_INTERP_LINEAR;
	ember.m_PaletteInterp = ePaletteInterp::INTERP_HSV;
	ember.m_Background[0] = 0;
	ember.m_Background[1] = 0;
	ember.m_Background[2] = 0;
	ember.m_Background[3] = 255;
	ember.m_CenterX = 0;
	ember.m_CenterY = 0;
	ember.m_Rotate = 0;
	ember.m_PixelsPerUnit = 64;
	ember.m_FinalRasW = 128;
	ember.m_FinalRasH = 128;
	ember.m_Supersample = 1;
	ember.m_SpatialFilterRadius = T(0.5);
	ember.m_SpatialFilterType = eSpatialFilterType::GAUSSIAN_SPATIAL_FILTER;
	ember.m_Zoom = 0;
	ember.m_Quality = 1;
	ember.m_TemporalSamples = 1;
	ember.m_MaxRadDE = 0;
	ember.m_MinRadDE = 0;
	ember.m_CurveDE = T(0.6);
}

/// <summary>
/// The core of the EmberGenome.exe program.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="opt">A populated EmberOptions object which specifies all program options to be used</param>
/// <returns>True if success, else false.</returns>
template <typename T>
bool EmberGenome(int argc, _TCHAR* argv[], EmberOptions& opt)
{
	auto info = OpenCLInfo::Instance();
	std::cout.imbue(std::locale(""));

	if (opt.DumpArgs())
		cerr << opt.GetValues(eOptionUse::OPT_USE_GENOME) << "\n";

	if (opt.OpenCLInfo())
	{
		cerr << "\nOpenCL Info: \n";
		cerr << info->DumpInfo();
		return true;
	}

	Compat::m_Compat = opt.Flam3Compat();
	auto varList = VariationList<T>::Instance();

	if (opt.AllVars() || opt.SumVars() || opt.AssignVars() || opt.PpSumVars() || opt.PpAssignVars() ||
			opt.DcVars() || opt.StateVars() || opt.ParVars() || opt.NonParVars() ||
			opt.RegVars() || opt.PreVars() || opt.PostVars())
	{
		vector<string> assign{ "outPoint->m_X =", "outPoint->m_Y =", "outPoint->m_Z =",
							   "outPoint->m_X=", "outPoint->m_Y=", "outPoint->m_Z=" };

		if (opt.AllVars())
		{
			auto& vars = varList->AllVars();

			for (auto& v : vars)
				cout << v->Name() << "\n";
		}
		else if (opt.SumVars())
		{
			auto& reg = varList->RegVars();
			auto matches = FindVarsWithout<T>(varList->RegVars(), assign);

			for (auto& v : matches)
				cout << v->Name() << "\n";
		}
		else if (opt.AssignVars())
		{
			auto matches = FindVarsWith<T>(varList->RegVars(), assign);

			for (auto& v : matches)
				cout << v->Name() << "\n";
		}
		else if (opt.PpSumVars())
		{
			auto& pre = varList->PreVars();
			auto& post = varList->PostVars();

			for (auto& v : pre)
				if (v->AssignType() == eVariationAssignType::ASSIGNTYPE_SUM)
					cout << v->Name() << "\n";

			for (auto& v : post)
				if (v->AssignType() == eVariationAssignType::ASSIGNTYPE_SUM)
					cout << v->Name() << "\n";
		}
		else if (opt.PpAssignVars())
		{
			auto& pre = varList->PreVars();
			auto& post = varList->PostVars();

			for (auto& v : pre)
				if (v->AssignType() == eVariationAssignType::ASSIGNTYPE_SET)
					cout << v->Name() << "\n";

			for (auto& v : post)
				if (v->AssignType() == eVariationAssignType::ASSIGNTYPE_SET)
					cout << v->Name() << "\n";
		}
		else if (opt.DcVars())
		{
			auto& all = varList->AllVars();
			auto matches = FindVarsWith<T>(all, vector<string> { "m_ColorX" });

			for (auto& v : matches)
				cout << v->Name() << "\n";
		}
		else if (opt.StateVars())
		{
			auto& all = varList->AllVars();

			for (auto& v : all)
				if (!v->StateOpenCLString().empty())
					cout << v->Name() << "\n";
		}
		else if (opt.ParVars())
		{
			auto& parVars = varList->ParametricVariations();

			for (auto& v : parVars)
				cout << v->Name() << "\n";
		}
		else if (opt.NonParVars())
		{
			auto& vars = varList->NonParametricVariations();

			for (auto& v : vars)
				cout << v->Name() << "\n";
		}
		else
		{
			vector<const Variation<T>*> vars;

			if (opt.RegVars())
				vars.insert(vars.end(), varList->RegVars().begin(), varList->RegVars().end());

			if (opt.PreVars())
				vars.insert(vars.end(), varList->PreVars().begin(), varList->PreVars().end());

			if (opt.PostVars())
				vars.insert(vars.end(), varList->PostVars().begin(), varList->PostVars().end());

			for (auto& v : vars)
				cout << v->Name() << "\n";
		}

		return true;
	}

	VerbosePrint("Using " << (sizeof(T) == sizeof(float) ? "single" : "double") << " precision.");
	//Regular variables.
	Timing t;
	bool exactTimeMatch, randomMode, didColor, seqFlag, random = false;
	size_t i, i0, i1, rep, val, frame, frameCount, count = 0;
	size_t ftime, firstFrame, lastFrame;
	double tot;
	size_t n, totb, totw;
	double avgPix;
	T fractionBlack, fractionWhite, blend, spread, mix0, mix1;
	string token, filename;
	ostringstream os, os2;
	vector<Ember<T>> embers, embers2, templateEmbers;
	vector<eVariationId> vars, noVars;
	vector<v4F> finalImage;
	eCrossMode crossMeth;
	eMutateMode mutMeth;
	Ember<T> orig, save, selp0, selp1, parent0, parent1;
	Ember<T>* aselp0, *aselp1, *pTemplate = nullptr;
	XmlToEmber<T> parser;
	EmberToXml<T> emberToXml;
	Interpolater<T> interpolater;
	EmberReport emberReport, emberReport2;
	const vector<pair<size_t, size_t>> devices = Devices(opt.Devices());
	auto progress = make_unique<RenderProgress<T>>();
	unique_ptr<Renderer<T, float>> renderer(CreateRenderer<T>(opt.EmberCL() ? eRendererType::OPENCL_RENDERER : eRendererType::CPU_RENDERER, devices, false, 0, emberReport));
	QTIsaac<ISAAC_SIZE, ISAAC_INT> rand(ISAAC_INT(t.Tic()), ISAAC_INT(t.Tic() * 2), ISAAC_INT(t.Tic() * 3));
	vector<string> errorReport = emberReport.ErrorReport();
	auto fullpath = GetExePath(argv[0]);
	os.imbue(std::locale(""));
	os2.imbue(std::locale(""));

	if (!errorReport.empty())
		cerr << emberReport.ErrorReportString();

	if (!renderer.get())
	{
		cerr << "Renderer creation failed, exiting.\n";
		return false;
	}

	if (!InitPaletteList<float>(fullpath, opt.PalettePath()))
		return false;

	if (!opt.EmberCL())
	{
		if (opt.ThreadCount() != 0)
			renderer->ThreadCount(opt.ThreadCount(), opt.IsaacSeed() != "" ? opt.IsaacSeed().c_str() : nullptr);
	}
	else
	{
		cerr << "Using OpenCL to render.\n";

		if (opt.Verbose())
		{
			for (auto& device : devices)
			{
				cerr << "Platform: " << info->PlatformName(device.first) << "\n";
				cerr << "Device: " << info->DeviceName(device.first, device.second) << "\n";
			}
		}
	}

	//SheepTools will own the created renderer and will take care of cleaning it up.
	SheepTools<T, float> tools(opt.PalettePath(), CreateRenderer<T>(opt.EmberCL() ? eRendererType::OPENCL_RENDERER : eRendererType::CPU_RENDERER, devices, false, 0, emberReport2));
	tools.SetSpinParams(!opt.UnsmoothEdge(),
						T(opt.Stagger()),
						T(opt.OffsetX()),
						T(opt.OffsetY()),
						opt.Nick(),
						opt.Url(),
						opt.Id(),
						opt.Comment(),
						opt.SheepGen(),
						opt.SheepId());

	if (opt.UseVars() != "" && opt.DontUseVars() != "")
	{
		cerr << "use_vars and dont_use_vars cannot both be specified. Returning without executing.\n";
		return false;
	}

	//Specify reasonable defaults if nothing is specified.
	if (opt.UseVars() == "" && opt.DontUseVars() == "")
	{
		noVars.push_back(eVariationId::VAR_NOISE);
		noVars.push_back(eVariationId::VAR_BLUR);
		noVars.push_back(eVariationId::VAR_GAUSSIAN_BLUR);
		noVars.push_back(eVariationId::VAR_RADIAL_BLUR);
		noVars.push_back(eVariationId::VAR_NGON);
		noVars.push_back(eVariationId::VAR_SQUARE);
		noVars.push_back(eVariationId::VAR_RAYS);
		noVars.push_back(eVariationId::VAR_CROSS);
		noVars.push_back(eVariationId::VAR_PRE_BLUR);
		noVars.push_back(eVariationId::VAR_SEPARATION);
		noVars.push_back(eVariationId::VAR_SPLIT);
		noVars.push_back(eVariationId::VAR_SPLITS);

		//Set ivars to the complement of novars.
		for (i = 0; i < varList->Size(); i++)
			if (!Contains(noVars, varList->GetVariation(i)->VariationId()))
				vars.push_back(varList->GetVariation(i)->VariationId());
	}
	else
	{
		if (opt.UseVars() != "")//Parse comma-separated list of variations to use.
		{
			istringstream iss(opt.UseVars());

			while (std::getline(iss, token, ','))
			{
				if (parser.Aton(token.c_str(), val))
				{
					if (val < varList->Size())
						vars.push_back(static_cast<eVariationId>(val));
				}
			}
		}
		else if (opt.DontUseVars() != "")
		{
			istringstream iss(opt.DontUseVars());

			while (std::getline(iss, token, ','))
			{
				if (parser.Aton(token.c_str(), val))
				{
					if (val < varList->Size())
						noVars.push_back(static_cast<eVariationId>(val));
				}
			}

			//Set ivars to the complement of novars.
			for (i = 0; i < varList->Size(); i++)
				if (!Contains(noVars, varList->GetVariation(i)->VariationId()))
					vars.push_back(varList->GetVariation(i)->VariationId());
		}
	}

	const auto doMutate = opt.Mutate() != "";
	const auto doInter  = opt.Inter()  != "";
	const auto doRotate = opt.Rotate() != "";
	const auto doClone  = opt.Clone()  != "";
	const auto doCross0 = opt.Cross0() != "";
	const auto doCross1 = opt.Cross1() != "";
	count += (doMutate ? 1 : 0);
	count += (doInter  ? 1 : 0);
	count += (doRotate ? 1 : 0);
	count += (doClone  ? 1 : 0);
	count += ((doCross0 || doCross1) ? 1 : 0);

	if (count > 1)
	{
		cerr << "Can only specify one of mutate, clone, cross, rotate, or inter. Returning without executing.\n";
		return false;
	}

	if (doCross0 != doCross1)//Must both be either true or false.
	{
		cerr << "Must specify both crossover arguments. Returning without executing.\n";
		return false;
	}

	if (opt.Method() != "" && (!doCross0 && !doMutate))
	{
		cerr << "Cannot specify method unless doing crossover or mutate. Returning without executing.\n";
		return false;
	}

	if (opt.TemplateFile() != "")
	{
		if (!ParseEmberFile(parser, opt.TemplateFile(), templateEmbers, false))//Do not use defaults here to ensure only present fields get used when applying the template.
			return false;

		if (templateEmbers.size() > 1)
			cerr << "More than one control point in template, ignoring all but first.\n";

		pTemplate = &templateEmbers[0];
	}

	//Methods for genetic manipulation begin here.
	if      (doMutate) filename = opt.Mutate();
	else if (doInter)  filename = opt.Inter();
	else if (doRotate) filename = opt.Rotate();
	else if (doClone)  filename = opt.Clone();
	else if (doCross0) filename = opt.Cross0();
	else if (opt.CloneAll() != "") filename = opt.CloneAll();
	else if (opt.Animate()  != "") filename = opt.Animate();
	else if (opt.Sequence() != "") filename = opt.Sequence();
	else if (opt.Inter()    != "") filename = opt.Inter();
	else if (opt.Rotate()   != "") filename = opt.Rotate();
	else if (opt.Clone()    != "") filename = opt.Clone();
	else if (opt.Mutate()   != "") filename = opt.Mutate();
	else random = true;

	if (!random)
	{
		if (!ParseEmberFile(parser, filename, embers))
			return false;

		if (doCross1)
		{
			if (!ParseEmberFile(parser, opt.Cross1(), embers2))
				return false;
		}
	}

	if (opt.CloneAll() != "")
	{
		cout << "<clone_all version=\"Ember-" << EmberVersion() << "\">\n";

		for (i = 0; i < embers.size(); i++)
		{
			if (pTemplate)
				tools.ApplyTemplate(embers[i], *pTemplate);

			tools.Offset(embers[i], T(opt.OffsetX()), T(opt.OffsetY()));
			cout << emberToXml.ToString(embers[i], opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), opt.HexPalette());
		}

		cout << "</clone_all>\n";
		return true;
	}

	if (opt.Animate() != "")
	{
		Ember<T> interpolated;

		for (i = 0; i < embers.size(); i++)
		{
			if (i > 0 && embers[i].m_Time <= embers[i - 1].m_Time)
			{
				cerr << "Error: control points must be sorted by time, but time " << embers[i].m_Time << " <= " << embers[i - 1].m_Time << ", index " << i << ".\n";
				return false;
			}

			embers[i].DeleteMotionElements();
		}

		firstFrame = size_t(opt.FirstFrame() == UINT_MAX ? embers[0].m_Time : opt.FirstFrame());
		lastFrame  = size_t(opt.LastFrame()  == UINT_MAX ? embers.back().m_Time : opt.LastFrame());

		if (lastFrame < firstFrame)
			lastFrame = firstFrame;

		cout << "<animate version=\"EMBER-" << EmberVersion() << "\">\n";

		for (ftime = firstFrame; ftime <= lastFrame; ftime++)
		{
			exactTimeMatch = false;

			for (i = 0; i < embers.size(); i++)
			{
				if (ftime == size_t(embers[i].m_Time))
				{
					interpolated = embers[i];
					exactTimeMatch = true;
					break;
				}
			}

			if (!exactTimeMatch)
			{
				interpolater.Interpolate(embers, T(ftime), T(opt.Stagger()), interpolated);

				for (i = 0; i < embers.size(); i++)
				{
					if (ftime == size_t(embers[i].m_Time - 1))
					{
						exactTimeMatch = true;
						break;
					}
				}

				if (!exactTimeMatch)
					interpolated.m_AffineInterp = eAffineInterp::AFFINE_INTERP_LINEAR;
			}

			if (pTemplate)
				tools.ApplyTemplate(interpolated, *pTemplate);

			cout << emberToXml.ToString(interpolated, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), opt.HexPalette());
		}

		cout << "</animate>\n";
		return true;
	}

	//Note that in Fractorium, the sequence is created from the params stored in the flame file.
	//Here, it's done using the values passed on the command line, which will override the values in the file.
	if (opt.Sequence() != "")
	{
		Ember<T> result;

		if (!opt.LoopFrames() && !opt.InterpFrames())
		{
			cerr << "loop frames or interp frames must be positive and non-zero, not " << opt.LoopFrames() << ", " << opt.InterpFrames() << ".\n";
			return false;
		}

		if (opt.LoopFrames() > 0 && !opt.Loops())
		{
			cerr << "loop frames cannot be positive while loops is zero: " << opt.LoopFrames() << ", " << opt.Loops() << ".\n";
			return false;
		}

		if (opt.Loops() > 0 && !opt.LoopFrames())
		{
			cerr << "loops cannot be positive while loopframes is zero: " << opt.Loops() << ", " << opt.LoopFrames() << ".\n";
			return false;
		}

		if (opt.Enclosed())
			cout << "<sequence version=\"EMBER-" << EmberVersion() << "\">\n";

		frameCount = 0;
		os.str("");
		os << setfill('0') << setprecision(0) << fixed;
		const auto padding = opt.Padding() ? streamsize(opt.Padding()) : (streamsize(std::log10(opt.StartCount() + (((opt.LoopFrames() * opt.Loops()) + opt.InterpFrames()) * embers.size()))) + 1);
		t.Tic();

		for (i = 0; i < embers.size(); i++)
		{
			if (opt.Loops() > 0)
			{
				const auto roundFrames = static_cast<size_t>(std::round(opt.LoopFrames() * opt.Loops()));

				for (frame = 0; frame < roundFrames; frame++)
				{
					blend = T(frame) / T(opt.LoopFrames());
					tools.Spin(embers[i], pTemplate, result, opt.StartCount() + frameCount++, blend, opt.CwLoops());//Result is cleared and reassigned each time inside of Spin().
					FormatName(result, os, padding);
					cout << emberToXml.ToString(result, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), opt.HexPalette());
				}

				//The loop above will have rotated just shy of a complete rotation.
				//Rotate the next step and save in result, but do not print.
				//result will be the starting point for the interp phase below.
				frame = roundFrames;
				blend = static_cast<T>(frame) / static_cast<T>(opt.LoopFrames());
				tools.Spin(embers[i], pTemplate, result, opt.StartCount() + frameCount, blend, opt.CwLoops());//Do not increment frameCount here.
				FormatName(result, os, padding);
			}

			if (i < embers.size() - 1)
			{
				if (opt.Loops() > 0)//Store the last result as the flame to interpolate from. This applies for whole or fractional values of opt.Loops().
					embers[i] = result;

				for (frame = 0; frame < opt.InterpFrames(); frame++)
				{
					seqFlag = frame == 0 || (frame == opt.InterpFrames() - 1);
					blend = frame / static_cast<T>(opt.InterpFrames());
					result.Clear();
					tools.SpinInter(&embers[i], pTemplate, result, opt.StartCount() + frameCount++, seqFlag, blend, opt.InterpLoops(), opt.CwInterpLoops());
					FormatName(result, os, padding);
					cout << emberToXml.ToString(result, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), opt.HexPalette());
				}
			}
		}

		tools.Spin(embers.back(), pTemplate, result, opt.StartCount() + frameCount, 0, opt.CwInterpLoops());
		FormatName(result, os, padding);
		cout << emberToXml.ToString(result, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), opt.HexPalette());
		t.Toc("Sequencing");

		if (opt.Enclosed())
			cout << "</sequence>\n";

		return true;
	}

	if (doInter || doRotate)
	{
		Ember<T> result, result1, result2, result3;

		if (!opt.LoopFrames() && !opt.InterpFrames())
		{
			cerr << "loop frames or interp frames must be positive and non-zero, not " << opt.LoopFrames() << ", " << opt.InterpFrames() << ".\n";
			return false;
		}

		frame = opt.Frame();
		blend = frame / static_cast<T>(opt.InterpFrames());//Percentage between first and second flame to treat as the center flame.
		spread = 1 / static_cast<T>(opt.InterpFrames());//Amount to move backward and forward from the center flame.

		if (opt.Enclosed())
			cout << "<pick version=\"EMBER-" << EmberVersion() << "\">\n";

		if (doRotate)
		{
			if (embers.size() != 1)
			{
				cerr << "rotation requires one control point, not " << embers.size() << ".\n";
				return false;
			}

			if (frame)//Cannot spin backward below frame zero.
			{
				tools.Spin(embers[0], pTemplate, result1, frame - 1, blend - spread, opt.CwLoops());
				cout << emberToXml.ToString(result1, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), opt.HexPalette());
			}

			tools.Spin(embers[0], pTemplate, result2, frame, blend, opt.CwLoops());
			tools.Spin(embers[0], pTemplate, result3, frame + 1, blend + spread, opt.CwLoops());
			cout << emberToXml.ToString(result2, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), opt.HexPalette());
			cout << emberToXml.ToString(result3, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), opt.HexPalette());
		}
		else
		{
			if (embers.size() != 2)
			{
				cerr << "interpolation requires two control points, not " << embers.size() << ".\n";
				return false;
			}

			if (frame)//Cannot interpolate backward below frame zero.
			{
				tools.SpinInter(embers.data(), pTemplate, result1, frame - 1, false, blend - spread, opt.InterpLoops(), opt.CwInterpLoops());
				cout << emberToXml.ToString(result1, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), opt.HexPalette());
			}

			tools.SpinInter(embers.data(), pTemplate, result2, frame, false, blend, opt.InterpLoops(), opt.CwInterpLoops());
			tools.SpinInter(embers.data(), pTemplate, result3, frame + 1, false, blend + spread, opt.InterpLoops(), opt.CwInterpLoops());
			cout << emberToXml.ToString(result2, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), opt.HexPalette());
			cout << emberToXml.ToString(result3, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), opt.HexPalette());
		}

		if (opt.Enclosed())
			cout << "</pick>\n";

		return true;
	}

	//Repeat.
	renderer->EarlyClip(opt.EarlyClip());
	renderer->YAxisUp(opt.YAxisUp());
	renderer->LockAccum(opt.LockAccum());
	renderer->PixelAspectRatio(T(opt.AspectRatio()));

	if (opt.Repeat() == 0)
	{
		cerr << "Repeat must be positive, not " << opt.Repeat() << "\n";
		return false;
	}

	if (opt.Enclosed())
		cout << "<pick version=\"EMBER-" << EmberVersion() << "\">\n";

	for (rep = 0; rep < opt.Repeat(); rep++)
	{
		count = 0;
		os.str("");
		save.Clear();
		VerbosePrint("Flame = " << rep + 1 << "/" << opt.Repeat() << "...");

		if (opt.Clone() != "")
		{
			os << "clone";//Action is 'clone' with trunc vars concat.

			if (opt.CloneAction() != "")
				os << " " << opt.CloneAction();

			selp0 = embers[rand.Rand(embers.size())];
			save = selp0;
			aselp0 = &selp0;
			aselp1 = nullptr;
			os << tools.TruncateVariations(save, 5);
			save.m_Edits = emberToXml.CreateNewEditdoc(aselp0, aselp1, os.str(), opt.Nick(), opt.Url(), opt.Id(), opt.Comment(), opt.SheepGen(), opt.SheepId());
		}
		else
		{
			do
			{
				randomMode = false;
				didColor = false;
				os.str("");
				VerbosePrint(".");

				if (doMutate)
				{
					selp0 = embers[rand.Rand(embers.size())];
					orig = selp0;
					aselp0 = &selp0;
					aselp1 = nullptr;

					if (opt.Method() == "")
						mutMeth = eMutateMode::MUTATE_NOT_SPECIFIED;
					else if (opt.Method() == "all_vars")
						mutMeth = eMutateMode::MUTATE_ALL_VARIATIONS;
					else if (opt.Method() == "one_xform")
						mutMeth = eMutateMode::MUTATE_ONE_XFORM_COEFS;
					else if (opt.Method() == "add_symmetry")
						mutMeth = eMutateMode::MUTATE_ADD_SYMMETRY;
					else if (opt.Method() == "post_xforms")
						mutMeth = eMutateMode::MUTATE_POST_XFORMS;
					else if (opt.Method() == "color_palette")
						mutMeth = eMutateMode::MUTATE_COLOR_PALETTE;
					else if (opt.Method() == "delete_xform")
						mutMeth = eMutateMode::MUTATE_DELETE_XFORM;
					else if (opt.Method() == "all_coefs")
						mutMeth = eMutateMode::MUTATE_ALL_COEFS;
					else
					{
						cerr << "method " << opt.Method() << " not defined for mutate. Defaulting to random.\n";
						mutMeth = eMutateMode::MUTATE_NOT_SPECIFIED;
					}

					os << tools.Mutate(orig, mutMeth, vars, opt.Symmetry(), T(opt.Speed()), 8);

					//Scan string returned for 'mutate color'.
					if (strstr(os.str().c_str(), "mutate color"))
						didColor = true;

					if (orig.m_Name != "")
					{
						os2.str("");
						os2 << "mutation " << rep << " of " << orig.m_Name;
						orig.m_Name = os2.str();
					}
				}
				else if (doCross0)
				{
					i0 = rand.Rand(embers.size());
					i1 = rand.Rand(embers2.size());
					selp0 = embers[i0];
					selp1 = embers2[i1];
					aselp0 = &selp0;
					aselp1 = &selp1;

					if (opt.Method() == "")
						crossMeth = eCrossMode::CROSS_NOT_SPECIFIED;
					else if (opt.Method() == "union")
						crossMeth = eCrossMode::CROSS_UNION;
					else if (opt.Method() == "interpolate")
						crossMeth = eCrossMode::CROSS_INTERPOLATE;
					else if (opt.Method() == "alternate")
						crossMeth = eCrossMode::CROSS_ALTERNATE;
					else
					{
						cerr << "method '" << opt.Method() << "' not defined for cross. Defaulting to random.\n";
						crossMeth = eCrossMode::CROSS_NOT_SPECIFIED;
					}

					os << tools.Cross(embers[i0], embers2[i1], orig, crossMeth);

					if (embers[i0].m_Name != "" || embers2[i1].m_Name != "")
					{
						os2.str("");
						os2 << rep << " of " << embers[i0].m_Name << " x " << embers2[i1].m_Name;
						orig.m_Name = os2.str();
					}
				}
				else
				{
					os << "random";
					randomMode = true;
					tools.Random(orig, vars, opt.Symmetry(), 0, 8);
					orig.m_FinalRasW = 1920;
					orig.m_FinalRasH = 1080;
					aselp0 = nullptr;
					aselp1 = nullptr;
				}

				//Adjust bounding box half the time.
				if (rand.RandBit() || randomMode)
				{
					T bmin[2], bmax[2];
					tools.EstimateBoundingBox(orig, T(0.01), 100000, bmin, bmax);

					if (rand.Frand01<T>() < T(0.3))
					{
						orig.m_CenterX = (bmin[0] + bmax[0]) / 2;
						orig.m_CenterY = (bmin[1] + bmax[1]) / 2;
						os << " recentered";
					}
					else
					{
						if (rand.RandBit())
						{
							mix0 = rand.GoldenBit<T>() + rand.Frand11<T>() / 5;
							mix1 = rand.GoldenBit<T>();
							os << " reframed0";
						}
						else if (rand.RandBit())
						{
							mix0 = rand.GoldenBit<T>();
							mix1 = rand.GoldenBit<T>() + rand.Frand11<T>() / 5;
							os << " reframed1";
						}
						else
						{
							mix0 = rand.GoldenBit<T>() + rand.Frand11<T>() / 5;
							mix1 = rand.GoldenBit<T>() + rand.Frand11<T>() / 5;
							os << " reframed2";
						}

						orig.m_CenterX = mix0 * bmin[0] + (1 - mix0) * bmax[0];
						orig.m_CenterY = mix1 * bmin[1] + (1 - mix1) * bmax[1];
					}

					orig.m_PixelsPerUnit = orig.m_FinalRasW / (bmax[0] - bmin[0]);
				}

				os << tools.TruncateVariations(orig, 5);

				if (!didColor && rand.RandBit())
				{
					if (opt.Debug())
						cerr << "improving colors...\n";

					tools.ImproveColors(orig, 100, false, 10);
					os << " improved colors";
				}

				orig.m_Edits = emberToXml.CreateNewEditdoc(aselp0, aselp1, os.str(), opt.Nick(), opt.Url(), opt.Id(), opt.Comment(), opt.SheepGen(), opt.SheepId());
				save = orig;
				SetDefaultTestValues(orig);
				renderer->SetEmber(orig, eProcessAction::FULL_RENDER, true);

				if (renderer->Run(finalImage) != eRenderStatus::RENDER_OK)
				{
					cerr << "Error: test image rendering failed, aborting.\n";
					return false;
				}

				tot = 0;
				totb = totw = 0;
				n = orig.m_FinalRasW * orig.m_FinalRasH;

				for (i = 0; i < n; i++)
				{
					tot += (finalImage[i].r + finalImage[i].g + finalImage[i].b);

					if (0   == finalImage[i].r && 0   == finalImage[i].g && 0   == finalImage[i].b) totb++;

					if (1 == finalImage[i].r && 1 == finalImage[i].g && 1 == finalImage[i].g) totw++;
				}

				avgPix = (tot / (3 * n));
				fractionBlack = totb / T(n);
				fractionWhite = totw / T(n);

				if (opt.Debug())
					cerr << "avgPix = " << avgPix << " fractionBlack = " << fractionBlack << " fractionWhite = " << fractionWhite << " n = " << n << "\n";

				orig.Clear();
				count++;
			}
			while ((avgPix < opt.AvgThresh() ||
					fractionBlack < opt.BlackThresh() ||
					fractionWhite > opt.WhiteLimit()) &&
					count < opt.Tries());

			if (count == opt.Tries())
				cerr << "Warning: reached maximum attempts, giving up.\n";
		}

		if (pTemplate)
			tools.ApplyTemplate(save, *pTemplate);

		save.m_Time = T(rep);

		if (opt.MaxXforms() != UINT_MAX)
		{
			save.m_Symmetry = 0;

			while (save.TotalXformCount() > opt.MaxXforms())
				save.DeleteTotalXform(save.TotalXformCount() - 1);
		}

		cout << emberToXml.ToString(save, opt.Extras(), opt.PrintEditDepth(), !opt.NoEdits(), opt.HexPalette());
		VerbosePrint("\nDone. Action = " << os.str() << "\n");
		cout.flush();
		save.Clear();
	}

	if (opt.Enclosed())
		cout << "</pick>\n";

	return true;
}

/// <summary>
/// Main program entry point for EmberGenome.exe.
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

	if (!opt.Populate(argc, argv, eOptionUse::OPT_USE_GENOME))
	{
		auto palf = PaletteList<float>::Instance();
#ifdef DO_DOUBLE

		if (!opt.Sp())
			b = EmberGenome<double>(argc, argv, opt);
		else
#endif
			b = EmberGenome<float>(argc, argv, opt);

		cout << std::flush;
	}

	return b ? 0 : 1;
}
