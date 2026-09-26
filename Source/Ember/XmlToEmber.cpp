#include "EmberPch.h"
#include "XmlToEmber.h"

namespace EmberNs
{
/// <summary>
/// Constructor which saves the state of the current locale and
/// sets the new one based on the parameters passed in.
/// </summary>
/// <param name="category">The locale category. Default: LC_NUMERIC.</param>
/// <param name="loc">The locale. Default: "C".</param>
Locale::Locale(int category, const char* loc)
{
	m_Category = category;
	m_NewLocale = string(loc);
	m_OriginalLocale = setlocale(category, nullptr);//Query.

	if (m_OriginalLocale.empty())
		cout << "Couldn't get original locale.\n";

	if (setlocale(category, loc) == nullptr)//Set.
		cout << "Couldn't set new locale " << category << ", " << loc << ".\n";
}

/// <summary>
/// Reset the locale to the value stored during construction.
/// </summary>
Locale::~Locale()
{
	if (!m_OriginalLocale.empty())
		if (setlocale(m_Category, m_OriginalLocale.c_str()) == nullptr)//Restore.
			cout << "Couldn't restore original locale " << m_Category << ", " << m_OriginalLocale << ".\n";
}

/// <summary>
/// Constructor that initializes the random context.
/// </summary>
template <typename T>
XmlToEmber<T>::XmlToEmber()
	: m_VariationList(VariationList<T>::Instance()),
	  m_PaletteList(PaletteList<float>::Instance())
{
	Timing t;

	if (!m_Init)
	{
		//This list is for variation params which are incorrect, but their parent variation name may or may not be correct.
		//This has some overlap with the list below since some of these have parent variation names that are also incorrect.
		m_BadParamNames = unordered_map<string, string>
		{
			{ "swtin_distort", "stwin_distort" },           //stwin.
			{ "pow_numerator", "pow_block_numerator" },           //pow_block.
			{ "pow_denominator", "pow_block_denominator" },
			{ "pow_root", "pow_block_root" },
			{ "pow_correctn", "pow_block_correctn" },
			{ "pow_correctd", "pow_block_correctd" },
			{ "pow_power", "pow_block_power" },
			{ "lt_powx", "linearT_powX" },                      //linearT.
			{ "lt_powy", "linearT_powY" },
			{ "re_a", "Mobius_Re_A" },                    //Mobius.
			{ "im_a", "Mobius_Im_A" },
			{ "re_b", "Mobius_Re_B" },
			{ "im_b", "Mobius_Im_B" },
			{ "re_c", "Mobius_Re_C" },
			{ "im_c", "Mobius_Im_C" },
			{ "re_d", "Mobius_Re_D" },
			{ "im_d", "Mobius_Im_D" },
			{ "rx_sin", "rotate_x_sin" },                  //rotate_x.
			{ "rx_cos", "rotate_x_cos" },
			{ "ry_sin", "rotate_y_sin" },                  //rotate_y.
			{ "ry_cos", "rotate_y_cos" },
			{ "intrfr2_a1", "interference2_a1" },              //interference2.
			{ "intrfr2_b1", "interference2_b1" },
			{ "intrfr2_c1", "interference2_c1" },
			{ "intrfr2_p1", "interference2_p1" },
			{ "intrfr2_t1", "interference2_t1" },
			{ "intrfr2_a2", "interference2_a2" },
			{ "intrfr2_b2", "interference2_b2" },
			{ "intrfr2_c2", "interference2_c2" },
			{ "intrfr2_p2", "interference2_p2" },
			{ "intrfr2_t2", "interference2_t2" },
			{ "octa_x", "octagon_x" },                  //octagon.
			{ "octa_y", "octagon_y" },
			{ "octa_z", "octagon_z" },
			{ "bubble_x", "bubble2_x" },                //bubble2.
			{ "bubble_y", "bubble2_y" },
			{ "bubble_z", "bubble2_z" },
			{ "cubic3d_xpand", "cubicLattice_3D_xpand" },           //cubicLattice_3D.
			{ "cubic3d_style", "cubicLattice_3D_style" },
			{ "splitb_x", "SplitBrdr_x" },                //SplitBrdr.
			{ "splitb_y", "SplitBrdr_y" },
			{ "splitb_px", "SplitBrdr_px" },
			{ "splitb_py", "SplitBrdr_py" },
			{ "dc_cyl_offset", "dc_cylinder_offset" },           //dc_cylinder.
			{ "dc_cyl_angle", "dc_cylinder_angle" },
			{ "dc_cyl_scale", "dc_cylinder_scale" },
			{ "cyl_x", "dc_cylinder_x" },
			{ "cyl_y", "dc_cylinder_y" },
			{ "cyl_blur", "dc_cylinder_blur" },
			{ "mobius_radius", "mobius_strip_radius" },           //mobius_strip.
			{ "mobius_width", "mobius_strip_width" },
			{ "mobius_rect_x", "mobius_strip_rect_x" },
			{ "mobius_rect_y", "mobius_strip_rect_y" },
			{ "mobius_rotate_x", "mobius_strip_rotate_x" },
			{ "mobius_rotate_y", "mobius_strip_rotate_y" },
			{ "bwraps2_cellsize", "bwraps_cellsize" },        //bwraps2.
			{ "bwraps2_space", "bwraps_space" },
			{ "bwraps2_gain", "bwraps_gain" },
			{ "bwraps2_inner_twist", "bwraps_inner_twist" },
			{ "bwraps2_outer_twist", "bwraps_outer_twist" },
			{ "bwraps7_cellsize", "bwraps_cellsize" },        //bwraps7.
			{ "bwraps7_space", "bwraps_space" },
			{ "bwraps7_gain", "bwraps_gain" },
			{ "bwraps7_inner_twist", "bwraps_inner_twist" },
			{ "bwraps7_outer_twist", "bwraps_outer_twist" },
			{ "pre_bwraps2_cellsize", "pre_bwraps_cellsize" },    //bwraps2.
			{ "pre_bwraps2_space", "pre_bwraps_space" },
			{ "pre_bwraps2_gain", "pre_bwraps_gain" },
			{ "pre_bwraps2_inner_twist", "pre_bwraps_inner_twist" },
			{ "pre_bwraps2_outer_twist", "pre_bwraps_outer_twist" },
			{ "post_bwraps2_cellsize", "post_bwraps_cellsize" },
			{ "post_bwraps2_space", "post_bwraps_space" },
			{ "post_bwraps2_gain", "post_bwraps_gain" },
			{ "post_bwraps2_inner_twist", "post_bwraps_inner_twist" },
			{ "post_bwraps2_outer_twist", "post_bwraps_outer_twist" },
			{ "hexa3d_majp", "hexaplay3D_majp" },             //hexaplay3D.
			{ "hexa3d_scale", "hexaplay3D_scale" },
			{ "hexa3d_zlift", "hexaplay3D_zlift" },
			{ "nb_numedges", "nBlur_numEdges" },             //nBlur.
			{ "nb_numstripes", "nBlur_numStripes" },
			{ "nb_ratiostripes", "nBlur_ratioStripes" },
			{ "nb_ratiohole", "nBlur_ratioHole" },
			{ "nb_circumcircle", "nBlur_circumCircle" },
			{ "nb_adjusttolinear", "nBlur_adjustToLinear" },
			{ "nb_equalblur", "nBlur_equalBlur" },
			{ "nb_exactcalc", "nBlur_exactCalc" },
			{ "nb_highlightedges", "nBlur_highlightEdges" },
			{ "octapol_r", "octapol_radius" },               //octapol.
			//{ "number_of_stripes", "bubbleT3D_number_of_stripes" },       //bubbleT3D.
			//{ "ratio_of_stripes", "bubbleT3D_ratio_of_stripes" },
			//{ "angle_of_hole", "bubbleT3D_angle_of_hole" },
			//{ "exponentZ", "bubbleT3D_exponentZ" },
			//{ "_symmetryZ", "bubbleT3D_symmetryZ" },
			//{ "_modusBlur", "bubbleT3D_modusBlur" },
			{ "post_scrop_power", "post_smartcrop_power" },        //post_smartcrop.
			{ "post_scrop_radius", "post_smartcrop_radius" },
			{ "post_scrop_roundstr", "post_smartcrop_roundstr" },
			{ "post_scrop_roundwidth", "post_smartcrop_roundwidth" },
			{ "post_scrop_distortion", "post_smartcrop_distortion" },
			{ "post_scrop_edge", "post_smartcrop_edge" },
			{ "post_scrop_scatter", "post_smartcrop_scatter" },
			{ "post_scrop_offset", "post_smartcrop_offset" },
			{ "post_scrop_rotation", "post_smartcrop_rotation" },
			{ "post_scrop_cropmode", "post_smartcrop_cropmode" },
			{ "post_scrop_static", "post_smartcrop_static" },
			{ "cs_radius", "circlesplit_radius" },
			{ "cs_split", "circlesplit_split" },
			{ "w2r_freqx", "waves2_radial_freqx" },
			{ "w2r_scalex", "waves2_radial_scalex" },
			{ "w2r_freqy", "waves2_radial_freqy" },
			{ "w2r_scaley", "waves2_radial_scaley" },
			{ "w2r_null", "waves2_radial_null" },
			{ "w2r_distance", "waves2_radial_distance" },
			{ "tf_exponent", "Truchet_fill_exponent" },
			{ "tf_arc_width", "Truchet_fill_arc_width" },
			{ "tf_seed", "Truchet_fill_seed" },
			//{ "blockSize", "randCubes_blockSize" },
			//{ "blockHeight", "randCubes_blockHeight" },
			//{ "spread", "randCubes_spread" },
			//{ "seed", "randCubes_seed" },
			//{ "density", "randCubes_density" },
			//{ "radius", "concentric_radius" },
			//{ "density", "concentric_density" },//Can't have two, which means you can never properly paste from Apophysis with both of these in one xform.
			//{ "r_blur", "concentric_R_blur" },
			//{ "z_blur", "concentric_Z_blur" },
			//{ "angle", "pixel_flow_angle" },
			///{ "len", "pixel_flow_len" },
			//{ "width", "pixel_flow_width" },
			//{ "seed", "pixel_flow_seed" },//randCubes above already uses "seed", but it's just for randomness, so it shouldn't matter.
			//{ "enable_dc", "pixel_flow_enable_dc" },
			{ "radial_gaussian_angle", "radial_blur_angle" },
			{ "pr_a",	"projective_A"  },
			{ "pr_b",	"projective_B"  },
			{ "pr_c",	"projective_C"  },
			{ "pr_a1",	"projective_A1" },
			{ "pr_b1",	"projective_B1" },
			{ "pr_c1",	"projective_C1" },
			{ "pr_a2",	"projective_A2" },
			{ "pr_b2",	"projective_B2" },
			{ "pr_c2",	"projective_C2" },
			{ "db_power",  "depth_blur_power" },
			{ "db_range",  "depth_blur_range" },
			{ "db_blur",   "depth_blur_blur" },
			{ "db_radius", "depth_blur_radius" },
			{ "osco2_separation", "oscilloscope2_separation" },
			{ "osco2_frequencyx", "oscilloscope2_frequencyx" },
			{ "osco2_frequencyy", "oscilloscope2_frequencyy" },
			{ "osco2_amplitude", "oscilloscope2_amplitude" },
			{ "osco2_perturbation", "oscilloscope2_perturbation" },
			{ "osco2_damping", "oscilloscope2_damping" },
			{ "oscope_separation", "oscilloscope_separation" },
			{ "oscope_frequency", "oscilloscope_frequency" },
			{ "oscope_amplitude", "oscilloscope_amplitude" },
			{ "oscope_damping", "oscilloscope_damping" },
			//{ "power", "scry2_power" },
			{ "faber_w_angle", "w_angle" },
			{ "faber_w_hypergon", "w_hypergon" },
			{ "faber_w_hypergon_n", "w_hypergon_n" },
			{ "faber_w_hypergon_r", "w_hypergon_r" },
			{ "faber_w_star", "w_star" },
			{ "faber_w_star_n", "w_star_n" },
			{ "faber_w_star_slope", "w_star_slope" },
			{ "faber_w_lituus", "w_lituus" },
			{ "faber_w_lituus_a", "w_lituus_a" },
			{ "faber_w_super", "w_super" },
			{ "faber_w_super_m", "w_super_m" },
			{ "faber_w_super_n1", "w_super_n1" },
			{ "faber_w_super_n2", "w_super_n2" },
			{ "faber_w_super_n3", "w_super_n3" },
			{ "faber_x_hypergon", "x_hypergon" },
			{ "faber_x_hypergon_n", "x_hypergon_n" },
			{ "faber_x_hypergon_r", "x_hypergon_r" },
			{ "faber_x_star", "x_star" },
			{ "faber_x_star_n", "x_star_n" },
			{ "faber_x_star_slope", "x_star_slope" },
			{ "faber_x_lituus", "x_lituus" },
			{ "faber_x_lituus_a", "x_lituus_a" },
			{ "faber_x_super", "x_super" },
			{ "faber_x_super_m", "x_super_m" },
			{ "faber_x_super_n1", "x_super_n1" },
			{ "faber_x_super_n2", "x_super_n2" },
			{ "faber_x_super_n3", "x_super_n3" },
			{ "sshape_power", "smartshape_power" },
			{ "sshape_roundstr", "smartshape_roundstr" },
			{ "sshape_roundwidth", "smartshape_roundwidth" },
			{ "sshape_distortion", "smartshape_distortion" },
			{ "sshape_compensation", "smartshape_compensation" },
			{ "post_sshape_power", "post_smartshape_power" },
			{ "post_sshape_roundstr", "post_smartshape_roundstr" },
			{ "post_sshape_roundwidth", "post_smartshape_roundwidth" },
			{ "post_sshape_distortion", "post_smartshape_distortion" },
			{ "post_sshape_compensation", "post_smartshape_compensation" },
			//{ "mult_x", "unicorngaloshen_mult_x" },
			//{ "mult_y", "unicorngaloshen_mult_y" },
			//{ "sine", "unicorngaloshen_sine" },
			//{ "sin_x_amplitude", "unicorngaloshen_sin_x_amplitude" },
			//{ "sin_x_freq", "unicorngaloshen_sin_x_freq" },
			//{ "sin_y_amplitude", "unicorngaloshen_sin_y_amplitude" },
			//{ "sin_y_freq", "unicorngaloshen_sin_y_freq" },
			//{ "mode", "unicorngaloshen_mode" },
			{ "d_spher_weight", "d_spherical_weight" },
			{ "poincare_p", "poincare2_p" },
			{ "poincare_q", "poincare2_q" },
			{ "phoenix_power", "phoenix_julia_power"},
			{ "phoenix_dist", "phoenix_julia_dist" },
			//{ "x_distort", "phoenix_julia_x_distort"},
			//{ "y_distort", "phoenix_julia_y_distort" },
			{ "phoenixjulia_power", "phoenix_julia_power" },
			{ "phoenixjulia_dist", "phoenix_julia_dist" },
			{ "phoenixjulia_x_distort", "phoenix_julia_x_distort" },
			{ "phoenixjulia_y_distort", "phoenix_julia_y_distort" },
			{ "log_", "log" }//Chaotica uses log_ as the weight for log to avoid a name conflict with the function log().
		};
		m_FlattenNames =
		{
			"pre_crop",
			"pre_falloff2",
			"pre_rotate_x",
			"pre_rotate_y",
			"pre_ztranslate",
			"blur3D",
			"bubble",
			"bwraps",
			"bwraps2",
			"crop",
			"cylinder",
			"falloff2",
			"hemisphere",
			"julia3D",
			"julia3Dz",
			"linear3D",
			"zblur",
			"zcone",
			"ztranslate",
			"post_crop",
			"post_falloff2",
			"post_rotate_x",
			"post_rotate_y",
			"curl3D_cz",
			"flatten"//Of course don't flatten if it's already present.
		};
		//This is a vector of the incorrect variation names and their param names as they are in the legacy, badly named flam3/Apophysis code.
		vector<string> badParams =
		{
			"bwraps7_cellsize",
			"bwraps7_space",
			"bwraps7_gain",
			"bwraps7_inner_twist",
			"bwraps7_outer_twist"
		};
		m_BadVariationNames.push_back(make_pair(make_pair(string("bwraps7"), string("bwraps")), badParams));//bwraps7 is the same as bwraps.
		badParams =
		{
			"bwraps2_cellsize",
			"bwraps2_space",
			"bwraps2_gain",
			"bwraps2_inner_twist",
			"bwraps2_outer_twist"
		};
		m_BadVariationNames.push_back(make_pair(make_pair(string("bwraps2"), string("bwraps")), badParams));//bwraps2 is the same as bwraps.
		badParams =
		{
			"pre_bwraps2_cellsize",
			"pre_bwraps2_space",
			"pre_bwraps2_gain",
			"pre_bwraps2_inner_twist",
			"pre_bwraps2_outer_twist"
		};
		m_BadVariationNames.push_back(make_pair(make_pair(string("pre_bwraps2"), string("pre_bwraps")), badParams));
		badParams =
		{
			"post_bwraps2_cellsize",
			"post_bwraps2_space",
			"post_bwraps2_gain",
			"post_bwraps2_inner_twist",
			"post_bwraps2_outer_twist"
		};
		m_BadVariationNames.push_back(make_pair(make_pair(string("post_bwraps2"), string("post_bwraps")), badParams));
		badParams =
		{
			"mobius_radius",
			"mobius_width",
			"mobius_rect_x",
			"mobius_rect_y",
			"mobius_rotate_x",
			"mobius_rotate_y"
		};
		m_BadVariationNames.push_back(make_pair(make_pair(string("mobius"), string("mobius_strip")), badParams));//mobius_strip clashes with Mobius.
		badParams =
		{
			"post_dcztransl_x0",
			"post_dcztransl_x1",
			"post_dcztransl_factor",
			"post_dcztransl_overwrite",
			"post_dcztransl_clamp"
		};
		m_BadVariationNames.push_back(make_pair(make_pair(string("post_dcztransl"), string("post_dc_ztransl")), badParams));
		badParams =
		{
			"post_scrop_power",
			"post_scrop_radius",
			"post_scrop_roundstr",
			"post_scrop_roundwidth",
			"post_scrop_distortion",
			"post_scrop_edge",
			"post_scrop_scatter",
			"post_scrop_offset",
			"post_scrop_rotation",
			"post_scrop_cropmode",
			"post_scrop_static"
		};
		m_BadVariationNames.push_back(make_pair(make_pair(string("post_scrop"), string("post_smartcrop")), badParams));
		badParams =
		{
			"sshape_power",
			"sshape_roundstr",
			"sshape_roundwidth",
			"sshape_distortion",
			"sshape_compensation"
		};
		m_BadVariationNames.push_back(make_pair(make_pair(string("sshape"), string("smartshape")), badParams));
		badParams =
		{
			"post_sshape_power",
			"post_sshape_roundstr",
			"post_sshape_roundwidth",
			"post_sshape_distortion",
			"post_sshape_compensation"
		};
		m_BadVariationNames.push_back(make_pair(make_pair(string("post_sshape"), string("post_smartshape")), badParams));
		badParams =
		{
			"radial_gaussian_angle"
		};
		m_BadVariationNames.push_back(make_pair(make_pair(string("radial_gaussian"), string("radial_blur")), badParams));
		badParams =
		{
			"faber_w_angle",
			"faber_w_hypergon",
			"faber_w_hypergon_n",
			"faber_w_hypergon_r",
			"faber_w_star",
			"faber_w_star_n",
			"faber_w_star_slope",
			"faber_w_lituus",
			"faber_w_lituus_a",
			"faber_w_super",
			"faber_w_super_m",
			"faber_w_super_n1",
			"faber_w_super_n2",
			"faber_w_super_n3"
		};
		m_BadVariationNames.push_back(make_pair(make_pair(string("faber_w"), string("w")), badParams));
		badParams =
		{
			"faber_x_angle",
			"faber_x_hypergon",
			"faber_x_hypergon_n",
			"faber_x_hypergon_r",
			"faber_x_star",
			"faber_x_star_n",
			"faber_x_star_slope",
			"faber_x_lituus",
			"faber_x_lituus_a",
			"faber_x_super",
			"faber_x_super_m",
			"faber_x_super_n1",
			"faber_x_super_n2",
			"faber_x_super_n3"
		};
		m_BadVariationNames.push_back(make_pair(make_pair(string("faber_x"), string("x")), badParams));
		//Note that splits3D can't be done here because its param names are also used by splits.
		badParams.clear();
		m_BadVariationNames.push_back(make_pair(make_pair(string("pre_blur"), string("pre_gaussian_blur")), badParams));//No other special params for these.
		m_BadVariationNames.push_back(make_pair(make_pair(string("pre_spin_z"), string("pre_rotate_z")), badParams));
		m_BadVariationNames.push_back(make_pair(make_pair(string("post_spin_z"), string("post_rotate_z")), badParams));
		m_Init = true;
	}
}

/// <summary>
/// Parse the specified buffer and place the results in the container of embers passed in.
/// </summary>
/// <param name="buf">The buffer to parse</param>
/// <param name="filename">Full path and filename, optionally empty</param>
/// <param name="embers">The newly constructed embers based on what was parsed</param>
/// <param name="useDefaults">True to use defaults if they are not present in the file, else false to use invalid values as placeholders to indicate the values were not present. Default: true.</param>
/// <returns>True if there were no errors, else false.</returns>
template <typename T>
template <typename Alloc, template <typename, typename> class C>
bool XmlToEmber<T>::Parse(byte* buf, const char* filename, C<Ember<T>, Alloc>& embers, bool useDefaults)
{
	const char* xmlPtr;
	const char* loc = __FUNCTION__;
	size_t emberSize;
	size_t bufSize;
	xmlDocPtr doc;//Parsed XML document tree.
	xmlNodePtr rootnode;
	Locale locale;//Sets and restores on exit.
	//Timing t;
	ClearErrorReport();
	//Parse XML string into internal document.
	xmlPtr = CX(&buf[0]);
	bufSize = strlen(xmlPtr);
	//embers.reserve(bufSize / 2500);//The Xml text for an ember is around 2500 bytes, but can be much more. Pre-allocate to aovid unnecessary resizing.
	int flags = XML_PARSE_HUGE;// | XML_PARSE_RECOVER;
	doc = xmlReadMemory(xmlPtr, int(bufSize), filename, "ISO-8859-1", flags);
	//t.Toc("xmlReadMemory");

	if (doc == nullptr)
	{
		doc = xmlReadMemory(xmlPtr, int(bufSize), filename, "UTF-8", flags);

		if (doc == nullptr)
		{
			doc = xmlReadMemory(xmlPtr, int(bufSize), filename, "UTF-16", flags);
		}
	}

	if (doc == nullptr)
	{
		AddToReport(string(loc) + " : Error parsing xml file " + string(filename));
		return false;
	}

	//What is the root node of the document?
	rootnode = xmlDocGetRootElement(doc);

	//Scan for <flame> nodes, starting with this node.
	//t.Tic();
	if (string(filename).empty())
	{
		ScanForEmberNodes(rootnode, filename, embers, useDefaults);

		if (embers.empty())
			ScanForChaosNodes(rootnode, filename, embers, useDefaults);
	}
	else if (EndsWith(filename, ".chaos"))
		ScanForChaosNodes(rootnode, filename, embers, useDefaults);
	else
		ScanForEmberNodes(rootnode, filename, embers, useDefaults);

	xmlFreeDoc(doc);
	emberSize = embers.size();
	const auto first = embers.begin();

	//t.Toc("ScanForEmberNodes");

	//Check to see if the first control point or the second-to-last
	//control point has interpolation="smooth".  This is invalid
	//and should be reset to linear (with a warning).
	if (emberSize > 0)
	{
		if (first->m_Interp == eInterp::EMBER_INTERP_SMOOTH)
			first->m_Interp = eInterp::EMBER_INTERP_LINEAR;

		if (emberSize >= 2)
		{
			auto secondToLast = Advance(embers.begin(), emberSize - 2);

			if (secondToLast->m_Interp == eInterp::EMBER_INTERP_SMOOTH)
				secondToLast->m_Interp = eInterp::EMBER_INTERP_LINEAR;
		}
	}
	else
	{
		AddToReport(string(loc) + " : Error parsing xml file " + string(filename) + ", no flames present.");
		return false;
	}

	//Finally, ensure that consecutive 'rotate' parameters never exceed
	//a difference of more than 180 degrees (+/-) for interpolation.
	//An adjustment of +/- 360 degrees is made until this is true.
	if (emberSize > 1)
	{
		auto prev = embers.begin();
		const auto second = Advance(embers.begin(), 1);

		for (auto it = second; it != embers.end(); ++it)
		{
			//Only do this adjustment if not in compat mode.
			if (prev->m_AffineInterp != eAffineInterp::AFFINE_INTERP_COMPAT && prev->m_AffineInterp != eAffineInterp::AFFINE_INTERP_OLDER)
			{
				while (it->m_Rotate < prev->m_Rotate - 180)
					it->m_Rotate += 360;

				while (it->m_Rotate > prev->m_Rotate + 180)
					it->m_Rotate -= 360;
			}

			prev = it;
		}
	}

	return true;
}
/// <summary>
/// Parse the specified file and place the results in the container of embers passed in.
/// This will strip out ampersands because the Xml parser can't handle them.
/// </summary>
/// <param name="filename">Full path and filename</param>
/// <param name="embers">The newly constructed embers based on what was parsed</param>
/// <param name="useDefaults">True to use defaults if they are not present in the file, else false to use invalid values as placeholders to indicate the values were not present. Default: true.</param>
/// <returns>True if there were no errors, else false.</returns>
template <typename T>
template <typename Alloc, template <typename, typename> class C>
bool XmlToEmber<T>::Parse(const char* filename, C<Ember<T>, Alloc>& embers, bool useDefaults)
{
	const char* loc = __FUNCTION__;
	string buf;

	//Ensure palette list is setup first.
	if (!m_PaletteList->Size())
	{
		AddToReport(string(loc) + " : Palette list must be initialized before parsing embers.");
		return false;
	}

	if (ReadFile(filename, buf))
	{
		std::replace(buf.begin(), buf.end(), '&', '+');
		return Parse(reinterpret_cast<byte*>(const_cast<char*>(buf.data())), filename, embers, useDefaults);
	}
	else
		return false;
}
/// <summary>
/// Thin wrapper around converting the string to a numeric value and return a bool indicating success.
/// See error report for errors.
/// </summary>
/// <param name="str">The string to convert</param>
/// <param name="val">The converted value</param>
/// <returns>True if success, else false.</returns>
template <typename T>
template <typename valT>
bool XmlToEmber<T>::Aton(const char* str, valT& val)
{
	bool b = true;
	const char* loc = __FUNCTION__;
	std::istringstream istr(str);
	istr >> val;

	if (istr.bad() || istr.fail())
	{
		AddToReport(string(loc) + " : Error converting " + string(str));
		b = false;
	}

	return b;
}
/// <summary>
/// Scan the file for ember nodes, and parse them out into the container of embers.
/// </summary>
/// <param name="curNode">The current node to parse</param>
/// <param name="parentFile">The full path and filename</param>
/// <param name="embers">The newly constructed embers based on what was parsed</param>
/// <param name="useDefaults">True to use defaults if they are not present in the file, else false to use invalid values as placeholders to indicate the values were not present.</param>
template <typename T>
template <typename Alloc, template <typename, typename> class C>
void XmlToEmber<T>::ScanForEmberNodes(xmlNode* curNode, const char* parentFile, C<Ember<T>, Alloc>& embers, bool useDefaults)
{
	bool parseEmberSuccess;
	xmlNodePtr thisNode = nullptr;
	const char* loc = __FUNCTION__;
	string parentFileString = string(parentFile);

	//Original memset to 0, but the constructors should handle that.
	//Loop over this level of elements.
	for (thisNode = curNode; thisNode; thisNode = thisNode->next)
	{
		//Check to see if this element is a <ember> element.
		if (thisNode->type == XML_ELEMENT_NODE && !Compare(thisNode->name, "flame"))
		{
			Ember<T> currentEmber;//Place this inside here so its constructor is called each time.

			//Useful for parsing templates when not every member should be set.
			if (!useDefaults)
				currentEmber.Clear(false);

			parseEmberSuccess = ParseEmberElement(thisNode, currentEmber);

			if (!parseEmberSuccess)
			{
				//Original leaked memory here, ours doesn't.
				AddToReport(string(loc) + " : Error parsing ember element");
				return;
			}

			if (currentEmber.PaletteIndex() != -1)
			{
				if (const auto pal = m_PaletteList->GetPaletteByFilename(m_PaletteList->m_DefaultFilename, currentEmber.PaletteIndex()))
					currentEmber.m_Palette = *pal;
				else
					AddToReport(string(loc) + " : Error assigning palette with index " + std::to_string(currentEmber.PaletteIndex()));
			}

			if (!currentEmber.XformCount())//Ensure there is always at least one xform or else the renderer will crash when trying to render.
			{
				Xform<T> xform;
				currentEmber.AddXform(xform);
			}

			currentEmber.CacheXforms();
			currentEmber.m_Index = embers.size();
			currentEmber.m_ParentFilename = parentFileString;
			embers.push_back(currentEmber);
		}
		else
		{
			//Check all of the children of this element.
			ScanForEmberNodes(thisNode->children, parentFile, embers, useDefaults);
		}
	}
}

/// <summary>
/// Helper function to verify that the name field of a node
/// matches the one passed in.
/// </summary>
/// <param name="node">The node whose name field will be inspected</param>
/// <param name="name">The name string to compare against</param>
/// <returns>The name value if they matched, else nullptr.</returns>
static const char* CheckNameVal(xmlNode* node, const char* name)
{
	if (const auto att = node->properties)
		if (!Compare(att->name, "name"))
			if (const auto attStr = XC(xmlGetProp(node, att->name)))
				if (!Compare(attStr, name))
					return CCX(attStr);

	return nullptr;
}

/// <summary>
/// Helper function to verify that the name of a node
/// matches the one passed in.
/// </summary>
/// <param name="node">The node whose name will be inspected</param>
/// <param name="name">The name string to compare against</param>
/// <returns>The node if they matched, else nullptr.</returns>
static xmlNode* CheckNodeName(xmlNode* node, const char* name)
{
	if (!Compare(node->name, name))
		return node;

	return nullptr;
}

/// <summary>
/// Helper function to get the value of the name field of a node.
/// </summary>
/// <param name="node">The node whose name field will be returned</param>
/// <param name="name">The name of the field whose value will be compared against. Default: "name".</param>
/// <returns>The value of the name field if found, else nullptr.</returns>
static const char* GetNameVal(xmlNode* node, const char* name = "name")
{
	if (const auto att = node->properties)
		if (!Compare(att->name, name ? name : "name"))
			if (const auto attStr = XC(xmlGetProp(node, att->name)))
				return CCX(attStr);

	return nullptr;
}

/// <summary>
/// Helper function to get the child of a node based on the value of its name field.
/// </summary>
/// <param name="node">The node whose children will be searched</param>
/// <param name="name">The name string to compare the child nodes' name fields against</param>
/// <returns>The child node if found, else nullptr.</returns>
static xmlNode* GetChildNode(xmlNode* node, const char* name)
{
	for (auto childNode = node->children; childNode; childNode = childNode->next)
		if (childNode->type == XML_ELEMENT_NODE)
			if (CheckNameVal(childNode, name))
				return childNode;

	return nullptr;
}

/// <summary>
/// Helper function to get the child of a node based on the name of the child node.
/// </summary>
/// <param name="node">The node whose children will be searched</param>
/// <param name="name">The name string to compare the child nodes' name against</param>
/// <returns>The child node if found, else nullptr.</returns>
static xmlNode* GetChildNodeByNodeName(xmlNode* node, const char* name)
{
	for (auto childNode = node->children; childNode; childNode = childNode->next)
		if (childNode->type == XML_ELEMENT_NODE)
            if (const auto foundNode = CheckNodeName(childNode, name))
                return foundNode;

	return nullptr;
}

/// <summary>
/// Helper function to parse the content of a field of a node and convert the string into a value of type T and store in the passed in val parameter.
/// </summary>
/// <param name="node">The node whose property fieldname will be parsed</param>
/// <param name="fieldname">The name of the node's field to parse</param>
/// <param name="fieldnameval">The name of the property within the field</param>
/// <param name="val">The storage location to store the parse and converted value in</param>
/// <returns>True if the field and property were successfully found and parsed, else false.</returns>
template <typename T>
template <typename valT>
bool XmlToEmber<T>::ParseAndAssignContent(xmlNode* node, const char* fieldname, const char* fieldnameval, valT& val)
{
	bool ret = false;

	if (const auto att = node->properties)
	{
		if (!Compare(att->name, fieldname))
		{
			if (const auto attStr = XC(xmlGetProp(node, att->name)))
			{
				if (!fieldnameval || !Compare(attStr, fieldnameval))
				{
					if (const auto cont = xmlNodeGetContent(node))
					{
						istringstream istr(CCX(cont));
						istr >> val;
						ret = !istr.bad() && !istr.fail();//Means the Compare() was right, and the conversion succeeded.
					}
				}
			}
		}
	}

	return ret;
}

/// <summary>
/// Special overload for string.
/// </summary>
/// <param name="node">The node whose property fieldname will be parsed</param>
/// <param name="fieldname">The name of the node's field to parse</param>
/// <param name="fieldnameval">The name of the property within the field</param>
/// <param name="val">The storage location to store the parse and converted string in</param>
/// <returns>True if the field and property were successfully found and parsed, else false.</returns>
template <typename T>
bool XmlToEmber<T>::ParseAndAssignContent(xmlNode* node, const char* fieldname, const char* fieldnameval, std::string& val)
{
	bool ret = false;

	if (const auto att = node->properties)
	{
		if (!Compare(att->name, fieldname))
		{
			if (const auto attStr = XC(xmlGetProp(node, att->name)))
			{
				if (!fieldnameval || !Compare(attStr, fieldnameval))
				{
					if (const auto cont = xmlNodeGetContent(node))
					{
						val = CX(cont);
						return true;
					}
				}
			}
		}
	}

	return ret;
}

/// <summary>
/// Scan the file for chaos nodes from a .chaos file from Chaotica, and parse them out into the container of embers.
/// </summary>
/// <param name="curNode">The current node to parse</param>
/// <param name="parentFile">The full path and filename</param>
/// <param name="embers">The newly constructed embers based on what was parsed</param>
/// <param name="useDefaults">True to use defaults if they are not present in the file, else false to use invalid values as placeholders to indicate the values were not present.</param>
template <typename T>
template <typename Alloc, template <typename, typename> class C>
void XmlToEmber<T>::ScanForChaosNodes(xmlNode* curNode, const char* parentFile, C<Ember<T>, Alloc>& embers, bool useDefaults)
{
	bool parseEmberSuccess;
	xmlNodePtr thisNode = nullptr;
	const char* loc = __FUNCTION__;
	string parentFileString = string(parentFile);

	//Loop over this level of elements.
	for (thisNode = curNode; thisNode; thisNode = thisNode->next)
	{
		//Check to see if this element is a <ember> element.
		if (thisNode->type == XML_ELEMENT_NODE && !Compare(thisNode->name, "IFS"))
		{
			Ember<T> currentEmber;//Place this inside here so its constructor is called each time.

			//Useful for parsing templates when not every member should be set.
			if (!useDefaults)
				currentEmber.Clear(false);

			if (const auto embername = GetNameVal(thisNode, "name"))
				currentEmber.m_Name = embername;

			const auto childNode = thisNode;
			parseEmberSuccess = ParseEmberElementFromChaos(childNode, currentEmber);

			if (!parseEmberSuccess)
			{
				AddToReport(string(loc) + " : Error parsing ember element");
				return;
			}

			if (!currentEmber.XformCount())//Ensure there is always at least one xform or else the renderer will crash when trying to render.
			{
				Xform<T> xform;
				currentEmber.AddXform(xform);
			}

			currentEmber.CacheXforms();
			currentEmber.m_MinRadDE = currentEmber.m_MaxRadDE = 0;//Chaotica doesn't support density filtering.
			currentEmber.m_Index = embers.size();
			currentEmber.m_FuseCount = 30;//Chaotica doesn't publish its fuse count, but the images appear like they were using a fuse count of at least 30.
			currentEmber.m_ParentFilename = parentFileString;
			embers.push_back(currentEmber);
		}
		else
		{
			//Check all of the children of this element.
			ScanForChaosNodes(thisNode->children, parentFile, embers, useDefaults);
		}
	}
}

/// <summary>
/// Parse an ember element from a .chaos file.
/// </summary>
/// <param name="emberNode">The current node to parse</param>
/// <param name="currentEmber">The newly constructed ember based on what was parsed</param>
/// <returns>True if there were no errors, else false.</returns>
template <typename T>
bool XmlToEmber<T>::ParseEmberElementFromChaos(xmlNode* emberNode, Ember<T>& currentEmber)
{
	const char* loc = __FUNCTION__;
	T sensorWidth = 2;
	xmlAttrPtr att;
	currentEmber.m_Palette.Clear();//Wipe out the current palette.
	att = emberNode->properties;//The top level element is a ember element, read the attributes of it and store them.
	auto variationsfunc = [&](const string& prefix, const char* nodename, xmlNode * node, Xform<T>& xf, std::vector<std::string>& alliterweights)
	{
		if (const auto transformsChildNode = GetChildNode(node, nodename))
		{
			for (auto variationNode = transformsChildNode->children; variationNode; variationNode = variationNode->next)
			{
				if (variationNode->type == XML_ELEMENT_NODE && (!Compare(variationNode->name, "flam3_variation") || !Compare(variationNode->name, "transform")))
				{
					auto variationNameNode = GetChildNode(variationNode, "variation_name");

					if (!variationNameNode)
						variationNameNode = GetChildNode(variationNode, "type_name");

					if (variationNameNode)
					{
						std::string varname;

						if (ParseAndAssignContent(variationNameNode, "name", "variation_name", varname)) {}
						else ParseAndAssignContent(variationNameNode, "name", "type_name", varname);

						if (!varname.empty())
						{
							const auto corrvarname = GetCorrectedVariationName(m_BadVariationNames, varname);
							const auto corrwprefix = !StartsWith(corrvarname, prefix) ? prefix + corrvarname : corrvarname;
							const auto varnamewithunderscore = corrvarname + "_";
							const auto fullprefix = prefix + varnamewithunderscore;

							if (auto var = m_VariationList->GetVariation(corrwprefix))
							{
								auto vc = std::unique_ptr<Variation<T>>(var->Copy());
								vc->m_Weight = 1;
								auto varCopy = vc.get();

								if (xf.AddVariation(varCopy))
								{
									vc.release();

									if (auto paramsNode = GetChildNodeByNodeName(variationNode, "params"))
									{
										const auto parvar = dynamic_cast<ParametricVariation<T>*>(varCopy);

										for (auto paramsChildNode = paramsNode->children; paramsChildNode; paramsChildNode = paramsChildNode->next)
										{
											if (paramsChildNode->type == XML_ELEMENT_NODE)
											{
												if (const auto paramname = GetNameVal(paramsChildNode))
												{
													T val = 1;

													if (const auto paramCurveChildNode = GetChildNodeByNodeName(paramsChildNode, "curve"))
													{
														if (const auto paramCurveValuesChildNode = GetChildNode(paramCurveChildNode, "values"))
														{
															if (const auto paramCurveValuesContentChildNode = GetChildNodeByNodeName(paramCurveValuesChildNode, "values"))
															{
																if (paramCurveValuesContentChildNode->children)
																{
																	string valstr = CCX(paramCurveValuesContentChildNode->children->content);
																	istringstream istr(valstr);
																	istr >> val;
																	varCopy->m_Weight = val;
																}
															}
														}
													}
													else if (ParseAndAssignContent(paramsChildNode, "name", paramname, val))
													{
														auto paramstr = GetCorrectedParamName(m_BadParamNames, paramname);

														//Compare non corrected or corrected names.
														//This is needed for the case of "log_" which gets corrected to "log".
														if (paramname == varname || paramstr == varname)
														{
															varCopy->m_Weight = val;
														}
														else if (parvar)
														{
															if (StartsWith(paramstr, fullprefix, true))
															{
																//Do nothing.
															}
															else if (!StartsWith(paramstr, varnamewithunderscore, true) && (prefix.size() == 0 || !StartsWith(paramstr, prefix, true)))
															{
																paramstr = fullprefix + paramstr;
															}
															else if (StartsWith(paramstr, varnamewithunderscore, true))
															{
																paramstr = prefix + paramstr;
															}

															//Need some special corrections here because Chaotica allows values that don't make sense.
															if (varname == "falloff2")
															{
																if (paramstr == "falloff2_mul_c")
																	val = 0;
																else if (paramstr == "falloff2_type")
																	val = std::ceil(val);
															}
															else if (varname == "falloff3")
															{
																if (paramstr == "falloff3_mul_c")
																	val = 0;
																else if (paramstr == "falloff3_blur_type")
																	val = std::ceil(val);
															}

															parvar->SetParamVal(paramstr.c_str(), val);
															//if (!parvar->SetParamVal(paramstr.c_str(), val))
															//	AddToReport(string(loc) + " : Failed to set parametric variation parameter " + paramstr);
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	};
	auto xformfunc = [&](xmlNode * node, Xform<T>& xf, std::vector<std::string>& alliterweights) -> bool
	{
		bool found = false;

		if (auto transformChildNode = GetChildNodeByNodeName(node, "flam3_transform"))
		{
			found = true;
			const auto itername = GetNameVal(node, "name");
			xf.m_Name = itername;

			if (const auto affineChildNode = GetChildNode(transformChildNode, "Pre affine"))
			{
				std::string offsetstr;
				double xangle = 0, xlength = 1, yangle = 90, ylength = 1, xoffset = 0, yoffset = 0;

				if (const auto xangleChildNode = GetChildNode(affineChildNode, "x_axis_angle"))
                {
					if (const auto paramCurveChildNode = GetChildNodeByNodeName(xangleChildNode, "curve"))
					{
						if (const auto paramCurveValuesChildNode = GetChildNode(paramCurveChildNode, "values"))
						{
							if (const auto paramCurveValuesContentChildNode = GetChildNodeByNodeName(paramCurveValuesChildNode, "values"))
							{
								if (paramCurveValuesContentChildNode->children)
								{
									string valstr = CCX(paramCurveValuesContentChildNode->children->content);
									istringstream istr(valstr);
									istr >> xangle;
								}
							}
						}
					}
					else
						ParseAndAssignContent(xangleChildNode, "name", "x_axis_angle", xangle);
                }

				if (const auto xlengthChildNode = GetChildNode(affineChildNode, "x_axis_length"))
					if (ParseAndAssignContent(xlengthChildNode, "name", "x_axis_length", xlength)) {}

				if (const auto yangleChildNode = GetChildNode(affineChildNode, "y_axis_angle"))
                {
					if (const auto paramCurveChildNode = GetChildNodeByNodeName(yangleChildNode, "curve"))
					{
						if (const auto paramCurveValuesChildNode = GetChildNode(paramCurveChildNode, "values"))
						{
							if (const auto paramCurveValuesContentChildNode = GetChildNodeByNodeName(paramCurveValuesChildNode, "values"))
							{
								if (paramCurveValuesContentChildNode->children)
								{
									string valstr = CCX(paramCurveValuesContentChildNode->children->content);
									istringstream istr(valstr);
									istr >> yangle;
								}
							}
						}
					}
					else
						ParseAndAssignContent(yangleChildNode, "name", "y_axis_angle", yangle);
                }

				if (const auto ylengthChildNode = GetChildNode(affineChildNode, "y_axis_length"))
					if (ParseAndAssignContent(ylengthChildNode, "name", "y_axis_length", ylength)) {}

				if (const auto offsetChildNode = GetChildNode(affineChildNode, "offset"))
                {
					if (ParseAndAssignContent(offsetChildNode, "name", "offset", offsetstr))
					{
						istringstream istr(offsetstr);
						istr >> xoffset >> yoffset;
					}
                }

				T x1 = T(xlength * std::cos(xangle * DEG_2_RAD));
				T y1 = T(xlength * std::sin(xangle * DEG_2_RAD));
				T x2 = T(ylength * std::cos(yangle * DEG_2_RAD));
				T y2 = T(ylength * std::sin(yangle * DEG_2_RAD));
				T o1 = T(xoffset);
				T o2 = T(yoffset);
				xf.m_Affine.A(x1);
				xf.m_Affine.B(x2);
				xf.m_Affine.C(o1);
				xf.m_Affine.D(y1);
				xf.m_Affine.E(y2);
				xf.m_Affine.F(o2);
			}

			if (const auto affineChildNode = GetChildNode(transformChildNode, "Post affine"))
			{
				std::string offsetstr;
				double xangle = 0, xlength = 1, yangle = 90, ylength = 1, xoffset = 0, yoffset = 0;

				if (const auto xangleChildNode = GetChildNode(affineChildNode, "x_axis_angle"))
                {
					if (!ParseAndAssignContent(xangleChildNode, "name", "x_axis_angle", xangle))
                    {
						if (const auto paramCurveChildNode = GetChildNodeByNodeName(affineChildNode, "curve"))
						{
							if (const auto paramCurveValuesChildNode = GetChildNode(paramCurveChildNode, "values"))
							{
								if (const auto paramCurveValuesContentChildNode = GetChildNodeByNodeName(paramCurveValuesChildNode, "values"))
								{
									if (paramCurveValuesContentChildNode->children)
									{
										string valstr = CCX(paramCurveValuesContentChildNode->children->content);
										istringstream istr(valstr);
										istr >> xangle;
									}
								}
							}
						}
                    }
                }

				if (const auto xlengthChildNode = GetChildNode(affineChildNode, "x_axis_length"))
					if (ParseAndAssignContent(xlengthChildNode, "name", "x_axis_length", xlength)) {}

				if (const auto yangleChildNode = GetChildNode(affineChildNode, "y_axis_angle"))
                {
					if (const auto paramCurveChildNode = GetChildNodeByNodeName(yangleChildNode, "curve"))
					{
						if (const auto paramCurveValuesChildNode = GetChildNode(paramCurveChildNode, "values"))
						{
							if (const auto paramCurveValuesContentChildNode = GetChildNodeByNodeName(paramCurveValuesChildNode, "values"))
							{
								if (paramCurveValuesContentChildNode->children)
								{
									string valstr = CCX(paramCurveValuesContentChildNode->children->content);
									istringstream istr(valstr);
									istr >> yangle;
								}
							}
						}
					}
					else
						ParseAndAssignContent(yangleChildNode, "name", "y_axis_angle", yangle);
                }

				if (const auto ylengthChildNode = GetChildNode(affineChildNode, "y_axis_length"))
					if (ParseAndAssignContent(ylengthChildNode, "name", "y_axis_length", ylength)) {}

				if (const auto offsetChildNode = GetChildNode(affineChildNode, "offset"))
                {
					if (ParseAndAssignContent(offsetChildNode, "name", "offset", offsetstr))
					{
						istringstream istr(offsetstr);
						istr >> xoffset >> yoffset;
					}
                }

				T x1 = static_cast<T>(xlength * std::cos(xangle * DEG_2_RAD));
				T y1 = static_cast<T>(xlength * std::sin(xangle * DEG_2_RAD));
				T x2 = static_cast<T>(ylength * std::cos(yangle * DEG_2_RAD));
				T y2 = static_cast<T>(ylength * std::sin(yangle * DEG_2_RAD));
				T o1 = static_cast<T>(xoffset);
				T o2 = static_cast<T>(yoffset);
				xf.m_Post.A(x1);
				xf.m_Post.B(x2);
				xf.m_Post.C(o1);
				xf.m_Post.D(y1);
				xf.m_Post.E(y2);
				xf.m_Post.F(o2);
			}

			string prefix;
			variationsfunc(prefix, "transforms", transformChildNode, xf, alliterweights);
			prefix = "pre_";
			variationsfunc(prefix, "pre_transforms", transformChildNode, xf, alliterweights);
			prefix = "post_";
			variationsfunc(prefix, "post_transforms", transformChildNode, xf, alliterweights);
		}

		if (const auto shaderChildNode = GetChildNodeByNodeName(node, "flam3_shader"))
		{
			T paletteIndex = 0, colorSpeed = 0.5, opacity = 1;

			if (const auto paletteIndexChildNode = GetChildNode(shaderChildNode, "palette_index"))
				if (ParseAndAssignContent(paletteIndexChildNode, "name", "palette_index", paletteIndex)) { xf.m_ColorX = xf.m_ColorY = paletteIndex; }

			if (const auto colrSpeedChildNode = GetChildNode(shaderChildNode, "blend_speed"))
				if (ParseAndAssignContent(colrSpeedChildNode, "name", "blend_speed", colorSpeed)) { xf.m_ColorSpeed = colorSpeed; }

			if (const auto opacityChildNode = GetChildNode(shaderChildNode, "opacity"))
				if (ParseAndAssignContent(opacityChildNode, "name", "opacity", opacity)) { xf.m_Opacity = opacity; }
		}


		if (const auto weightsChildNode = GetChildNodeByNodeName(node, "weights_selector"))
		{
			T weight = 0;
			std::string periterweights;

            if (const auto baseWeightChildNode1 = GetChildNode(weightsChildNode, "base_weight"))
			{
                if (ParseAndAssignContent(baseWeightChildNode1, "name", "base_weight", weight))
					xf.m_Weight = weight;
			}
            else if (const auto baseWeightChildNode2 = GetChildNode(weightsChildNode, "Base weight"))
			{
                if (ParseAndAssignContent(baseWeightChildNode2, "name", "Base weight", weight))
					xf.m_Weight = weight;
			}

			if (const auto periterweightsChildNode = GetChildNode(weightsChildNode, "per_iterator_weights"))
			{
				for (auto iterweightChildNode = periterweightsChildNode->children; iterweightChildNode; iterweightChildNode = iterweightChildNode->next)
				{
					if (iterweightChildNode->type == XML_ELEMENT_NODE)
					{
						std::string iterweight;

						if (ParseAndAssignContent(iterweightChildNode, "name", nullptr, iterweight))//All of these subsequent nodes are for iter specific weights (xaos).
							periterweights += iterweight + ' ';
					}
				}
			}

			if (!periterweights.empty())
				alliterweights.push_back(Trim(periterweights));
		}

		return found;
	};

	if (att == nullptr)
	{
		AddToReport(string(loc) + " : <IFS> element has no attributes");
		return false;
	}

	auto childNode = emberNode->children;

	for (; childNode; childNode = childNode->next)
	{
		if (childNode->type == XML_ELEMENT_NODE)
		{
			if (!Compare(childNode->name, "imaging"))
			{
				std::string bgstr, useHighlightPower;

				for (auto imgChildNode = childNode->children; imgChildNode; imgChildNode = imgChildNode->next)
				{
					if (imgChildNode->type == XML_ELEMENT_NODE)
					{
						if (ParseAndAssignContent(imgChildNode, "name", "image_width", currentEmber.m_FinalRasW)) {}
						else if (ParseAndAssignContent(imgChildNode, "name", "image_height", currentEmber.m_FinalRasH)) {}
						else if (ParseAndAssignContent(imgChildNode, "name", "image_aa_level", currentEmber.m_Supersample)) { ClampRef(currentEmber.m_Supersample, size_t(1), size_t(4)); }
						else if (ParseAndAssignContent(imgChildNode, "name", "image_quality", currentEmber.m_Quality)) { currentEmber.m_Quality = std::max(currentEmber.m_Quality, T(1)); }
						else if (ParseAndAssignContent(imgChildNode, "name", "brightness", currentEmber.m_Brightness)) {}
						else if (ParseAndAssignContent(imgChildNode, "name", "flam3_gamma", currentEmber.m_Gamma)) {}
						else if (ParseAndAssignContent(imgChildNode, "name", "flam3_vibrancy", currentEmber.m_Vibrancy)) {}
						else if (ParseAndAssignContent(imgChildNode, "name", "flam3_use_highlight_power", useHighlightPower)) {}
						else if (ParseAndAssignContent(imgChildNode, "name", "flam3_highlight_power", currentEmber.m_HighlightPower)) {}
						else if (ParseAndAssignContent(imgChildNode, "name", "flam3_gamma_linear_threshold", currentEmber.m_GammaThresh)) {}
						else if (ParseAndAssignContent(imgChildNode, "name", "background_colour", bgstr))
						{
							istringstream is(bgstr);
							is >> currentEmber.m_Background[0]//[0..1]
							   >> currentEmber.m_Background[1]
							   >> currentEmber.m_Background[2]
							   >> currentEmber.m_Background[3];
						}
					}
				}

				// there is no warranty that flam3_use_highlight_power will be read before flam3_highlight_power. So, better to be here.
				bool bVal; istringstream istr(useHighlightPower); istr >> std::boolalpha >> bVal;

				if (!bVal && !istr.bad() && !istr.fail())
					currentEmber.m_HighlightPower = T(-1);

				if (const auto curvesnode = GetChildNodeByNodeName(childNode, "curves"))
				{
                    auto curvenodesfunc = [&](xmlNode * node, size_t index)
					{
						float x, y;
						string knots, values;
						vector<v2F> vals;

						if (const auto knotsnode = GetChildNode(node, "knots"))
						{
							if (const auto knotvalsnode = GetChildNodeByNodeName(knotsnode, "values"))
								if (knotvalsnode->children)
									knots = CCX(knotvalsnode->children->content);
						}

						if (const auto valuesnode = GetChildNode(node, "values"))
						{
							if (const auto valvalsnode = GetChildNodeByNodeName(valuesnode, "values"))
								if (valvalsnode->children)
									values = CCX(valvalsnode->children->content);
						}

						if (knots.empty() && values.empty())
						{
							bool haveknots = false, havevals = false;

							for (auto innerChildNode = node->children; innerChildNode; innerChildNode = innerChildNode->next)
							{
								if (innerChildNode->type == XML_ELEMENT_NODE)
								{
									if (const auto innernode = CheckNodeName(innerChildNode, "table"))
									{
										if (!haveknots)
										{
											if (const auto knotvalsnode = GetChildNodeByNodeName(innernode, "values"))
											{
												if (knotvalsnode->children)
												{
													knots = CCX(knotvalsnode->children->content);
													haveknots = true;
												}

												continue;
											}
										}
										else if (!havevals)
										{
											if (const auto valvalsnode = GetChildNodeByNodeName(innernode, "values"))
											{
												if (valvalsnode->children)
												{
													values = CCX(valvalsnode->children->content);
													havevals = true;
												}

												continue;
											}
										}
									}
								}
							}
						}

						istringstream kistr(knots);
						istringstream vistr(values);

						while (kistr >> x && vistr >> y)
							vals.push_back({ x, y });

						if (index < currentEmber.m_Curves.m_Points.size() && vals.size() >= 2)
						{
							std::sort(vals.begin(), vals.end(), [&](auto & lhs, auto & rhs) { return lhs.x < rhs.x; });
							currentEmber.m_Curves.m_Points[index].clear();
							currentEmber.m_Curves.m_Points[index].push_back(vals[0]);
							currentEmber.m_Curves.m_Points[index].push_back(vals[1]);

							for (size_t i = 2; i < vals.size(); i++)
								if (vals[i] != currentEmber.m_Curves.m_Points[index][i - 1])//An attempt to remove duplicates.
									currentEmber.m_Curves.m_Points[index].push_back(vals[i]);
						}
					};

					if (const auto overallnode = GetChildNode(curvesnode, "overall"))
						curvenodesfunc(overallnode, 0);

					if (const auto rednode = GetChildNode(curvesnode, "0"))
						curvenodesfunc(rednode, 1);

					if (const auto greennode = GetChildNode(curvesnode, "5"))
						curvenodesfunc(greennode, 2);

					if (const auto bluenode = GetChildNode(curvesnode, "10"))
						curvenodesfunc(bluenode, 3);
				}
			}
			else if (!Compare(childNode->name, "camera"))
			{
				std::string pos;

				for (auto camChildNode = childNode->children; camChildNode; camChildNode = camChildNode->next)
				{
					if (camChildNode->type == XML_ELEMENT_NODE)
					{
						if (ParseAndAssignContent(camChildNode, "name", "rotate", currentEmber.m_Rotate)) { currentEmber.m_Rotate = NormalizeDeg360<T>(currentEmber.m_Rotate); }
						else if (ParseAndAssignContent(camChildNode, "name", "sensor_width", sensorWidth)) {  }
						else if (ParseAndAssignContent(camChildNode, "name", "pos", pos))
						{
							istringstream istr(pos);
							istr >> currentEmber.m_CenterX >> currentEmber.m_CenterY;
							currentEmber.m_CenterY *= -1;
							currentEmber.m_RotCenterY = currentEmber.m_CenterY;
						}
						else
						{
							Xform<T> finalXform;
							std::vector<std::string> alliterweights;

							if (xformfunc(childNode, finalXform, alliterweights))//Iter weights are unused in the final xform.
							{
								finalXform.m_Weight = 0;
								finalXform.m_Animate = 0;//Do not animate final by default.
								finalXform.m_AnimateOrigin = 0;
								finalXform.m_ColorX = finalXform.m_ColorY = 0;//Chaotica does not support any kind of coloring for final xforms, opacity remains 1 though.
								finalXform.m_ColorSpeed = 0;
								currentEmber.SetFinalXform(finalXform);
							}
						}
					}
				}
			}
			else if (!Compare(childNode->name, "colouring"))
			{
				if (const auto palettenode = GetChildNode(childNode, "flam3_palette"))
				{
					if (const auto palettevalsnode = GetChildNodeByNodeName(palettenode, "values"))
					{
						float r = 0, g = 0, b = 0;
						auto colors = CCX(palettevalsnode->children->content);
						istringstream istr(colors);
						currentEmber.m_Palette.m_Entries.clear();
						std::vector<v4F> tempv;
						tempv.reserve(256);

						while (istr >> r && istr >> g && istr >> b)
							tempv.push_back(v4F(r, g, b, 1));

						if (!tempv.empty())
							currentEmber.m_Palette.m_Entries = std::move(tempv);
					}
				}
				else
				{
					std::string huek, huev, satk, satv, valk, valv;

					if (const auto huenode = GetChildNode(childNode, "hue"))
					{
						if (const auto knotsnode = GetChildNode(huenode, "knots"))
							if (const auto knotvalsnode = GetChildNodeByNodeName(knotsnode, "values"))
								if (knotvalsnode->children)
									huek = CCX(knotvalsnode->children->content);

						if (const auto valuesnode = GetChildNode(huenode, "values"))
							if (const auto valvalsnode = GetChildNodeByNodeName(valuesnode, "values"))
								if (valvalsnode->children)
									huev = CCX(valvalsnode->children->content);
					}

					if (const auto satnode = GetChildNode(childNode, "saturation"))
					{
						if (const auto knotsnode = GetChildNode(satnode, "knots"))
							if (const auto knotvalsnode = GetChildNodeByNodeName(knotsnode, "values"))
								if (knotvalsnode->children)
									satk = CCX(knotvalsnode->children->content);

						if (const auto valuesnode = GetChildNode(satnode, "values"))
							if (const auto valvalsnode = GetChildNodeByNodeName(valuesnode, "values"))
								if (valvalsnode->children)
									satv = CCX(valvalsnode->children->content);
					}

					if (const auto valnode = GetChildNode(childNode, "value"))
					{
						if (const auto knotsnode = GetChildNode(valnode, "knots"))
							if (const auto knotvalsnode = GetChildNodeByNodeName(knotsnode, "values"))
								if (knotvalsnode->children)
									valk = CCX(knotvalsnode->children->content);

						if (const auto valuesnode = GetChildNode(valnode, "values"))
							if (const auto valvalsnode = GetChildNodeByNodeName(valuesnode, "values"))
								if (valvalsnode->children)
									valv = CCX(valvalsnode->children->content);
					}

					auto parsehsvfunc = [&](const std::string& knots, const std::string& vals, vector<v2F>& vec)
					{
						istringstream kstr(knots);
						istringstream vstr(vals);
						float k, v;
						vec.clear();
						vec.reserve(8);

						while (kstr >> k && vstr >> v)
						{
							vec.push_back({k, v});
						}
					};
					vector<v2F> hvec, svec, vvec;
					parsehsvfunc(huek, huev, hvec);
					parsehsvfunc(satk, satv, svec);
					parsehsvfunc(valk, valv, vvec);

					if (huek.size() >= 2 && huev.size() >= 2 &&
							satk.size() >= 2 && satv.size() >= 2 &&
							valk.size() >= 2 && valv.size() >= 2)
					{
						Spline<float> hspline(hvec);
						Spline<float> sspline(svec);
						Spline<float> vspline(vvec);
						currentEmber.m_Palette.m_Entries.resize(COLORMAP_LENGTH);
						const auto stepsize = (1.0f / (currentEmber.m_Palette.Size() - 1));

                        for (size_t palindex = 0; palindex < currentEmber.m_Palette.Size(); palindex++)
						{
							const float t = palindex * stepsize;
							const auto h = hspline.Interpolate(t);
							const auto s = sspline.Interpolate(t);
							const auto v = vspline.Interpolate(t);
							float r, g, b;
							Palette<float>::HsvToRgb(static_cast<float>(h * 2 * M_PI), s, v, r, g, b);
							currentEmber.m_Palette.m_Entries[palindex][0] = r;
							currentEmber.m_Palette.m_Entries[palindex][1] = g;
							currentEmber.m_Palette.m_Entries[palindex][2] = b;
							currentEmber.m_Palette.m_Entries[palindex][3] = 1;
						}
					}
				}
			}
			else if (!Compare(childNode->name, "node"))
			{
				if (const auto nodename = CheckNameVal(childNode, "iterators"))
				{
					std::vector<std::string> alliterweights;

					for (auto iterChildNode = childNode->children; iterChildNode; iterChildNode = iterChildNode->next)
					{
						if (iterChildNode->type == XML_ELEMENT_NODE && !Compare(iterChildNode->name, "iterator"))
						{
							Xform<T> xf;
							xformfunc(iterChildNode, xf, alliterweights);
							currentEmber.AddXform(xf);
						}
					}

					if (!alliterweights.empty())
					{
						size_t i = 0;

						while (const auto xform = currentEmber.GetXform(i))
						{
							if (i < alliterweights.size() && !alliterweights[i].empty())
							{
								istringstream istr(alliterweights[i]);
								T xaoselement = 0;
								size_t j = 0;

								while (istr >> xaoselement)
									xform->SetXaos(j++, xaoselement);
							}

							i++;
						}
					}
				}
			}
		}
	}

	currentEmber.m_OrigPixPerUnit = currentEmber.m_PixelsPerUnit = currentEmber.m_FinalRasW / Zeps(sensorWidth);
	return currentEmber.XformCount() > 0;
}

/// <summary>
/// Parse an ember element.
/// </summary>
/// <param name="emberNode">The current node to parse</param>
/// <param name="currentEmber">The newly constructed ember based on what was parsed</param>
/// <returns>True if there were no errors, else false.</returns>
template <typename T>
bool XmlToEmber<T>::ParseEmberElement(xmlNode* emberNode, Ember<T>& currentEmber)
{
	bool ret = true;
	bool fromEmber = false;
	size_t newLinear = 0;
	char* attStr;
	const char* loc = __FUNCTION__;
	int soloXform = -1;
	size_t i, count = 0, index = 0;
	xmlAttrPtr att, curAtt;
	xmlNodePtr editNode, childNode, motionNode;
	currentEmber.m_Palette.Clear();//Wipe out the current palette.
	att = emberNode->properties;//The top level element is a ember element, read the attributes of it and store them.

	if (att == nullptr)
	{
		AddToReport(string(loc) + " : <flame> element has no attributes");
		return false;
	}

	for (curAtt = att; curAtt; curAtt = curAtt->next)
	{
		attStr = reinterpret_cast<char*>(xmlGetProp(emberNode, curAtt->name));

		//First parse out simple float reads.
        if (ParseAndAssign(curAtt->name, attStr, "time", currentEmber.m_Time)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "scale", currentEmber.m_PixelsPerUnit)) { currentEmber.m_OrigPixPerUnit = currentEmber.m_PixelsPerUnit; }
        else if (ParseAndAssign(curAtt->name, attStr, "rotate", currentEmber.m_Rotate)) { currentEmber.m_Rotate = NormalizeDeg360<T>(currentEmber.m_Rotate); }
        else if (ParseAndAssign(curAtt->name, attStr, "zoom", currentEmber.m_Zoom)) { ClampGteRef<T>(currentEmber.m_Zoom, 0); }
        else if (ParseAndAssign(curAtt->name, attStr, "cam_zoom", currentEmber.m_Zoom)) { ClampGteRef<T>(currentEmber.m_Zoom, 0); }//JWildfire uses cam_zoom.
        else if (ParseAndAssign(curAtt->name, attStr, "filter", currentEmber.m_SpatialFilterRadius)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "temporal_filter_width", currentEmber.m_TemporalFilterWidth)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "temporal_filter_exp", currentEmber.m_TemporalFilterExp)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "quality", currentEmber.m_Quality)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "brightness", currentEmber.m_Brightness)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "gamma", currentEmber.m_Gamma)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "highlight_power", currentEmber.m_HighlightPower)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "logscale_k2", currentEmber.m_K2)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "vibrancy", currentEmber.m_Vibrancy)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "estimator_radius", currentEmber.m_MaxRadDE)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "estimator_minimum", currentEmber.m_MinRadDE)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "estimator_curve", currentEmber.m_CurveDE)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "gamma_threshold", currentEmber.m_GammaThresh)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "cam_zpos", currentEmber.m_CamZPos)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "cam_persp", currentEmber.m_CamPerspective)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "cam_perspective", currentEmber.m_CamPerspective)) {}//Apo bug.
        else if (ParseAndAssign(curAtt->name, attStr, "cam_yaw", currentEmber.m_CamYaw)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "cam_pitch", currentEmber.m_CamPitch)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "cam_dof", currentEmber.m_CamDepthBlur)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "blur_curve", currentEmber.m_BlurCurve)) {}
		//Parse simple int reads.
        else if (ParseAndAssign(curAtt->name, attStr, "palette", currentEmber.m_Palette.m_Index)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "oversample", currentEmber.m_Supersample)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "supersample", currentEmber.m_Supersample)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "temporal_samples", currentEmber.m_TemporalSamples)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "sub_batch_size", currentEmber.m_SubBatchSize)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "fuse", currentEmber.m_FuseCount)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "rand_range", currentEmber.m_RandPointRange)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "soloxform", soloXform)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "new_linear", newLinear)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "stagger", currentEmber.m_Stagger)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "rotations", currentEmber.m_Rotations)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "seconds_per_rotation", currentEmber.m_SecondsPerRotation)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "rotate_xforms_cw", currentEmber.m_RotateXformsCw)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "blend_seconds", currentEmber.m_BlendSeconds)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "rotations_per_blend", currentEmber.m_RotationsPerBlend)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "blend_rotate_xforms_cw", currentEmber.m_BlendRotateXformsCw)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "linear_blend", currentEmber.m_Linear)) {}
		//Parse more complicated reads that have multiple possible values.
		else if (!Compare(curAtt->name, "interpolation"))
		{
			if (!_stricmp("linear", attStr))
				currentEmber.m_Interp = eInterp::EMBER_INTERP_LINEAR;
			else if (!_stricmp("smooth", attStr))
				currentEmber.m_Interp = eInterp::EMBER_INTERP_SMOOTH;
			else
				AddToReport(string(loc) + " : Unrecognized interpolation type " + string(attStr));
		}
		else if (!Compare(curAtt->name, "palette_interpolation"))
		{
			if (!_stricmp("hsv", attStr))
				currentEmber.m_PaletteInterp = ePaletteInterp::INTERP_HSV;
			else if (!_stricmp("sweep", attStr))
				currentEmber.m_PaletteInterp = ePaletteInterp::INTERP_SWEEP;
			else
				AddToReport(string(loc) + " : Unrecognized palette interpolation type " + string(attStr));
		}
		else if (!Compare(curAtt->name, "interpolation_space") || !Compare(curAtt->name, "interpolation_type"))
		{
			if (!_stricmp("linear", attStr))
				currentEmber.m_AffineInterp = eAffineInterp::AFFINE_INTERP_LINEAR;
			else if (!_stricmp("log", attStr))
				currentEmber.m_AffineInterp = eAffineInterp::AFFINE_INTERP_LOG;
			else if (!_stricmp("old", attStr))
				currentEmber.m_AffineInterp = eAffineInterp::AFFINE_INTERP_COMPAT;
			else if (!_stricmp("older", attStr))
				currentEmber.m_AffineInterp = eAffineInterp::AFFINE_INTERP_OLDER;
			else
				AddToReport(string(loc) + " : Unrecognized interpolation type " + string(attStr));
		}
		else if (!Compare(curAtt->name, "name"))
		{
			currentEmber.m_Name = string(attStr);
			std::replace(currentEmber.m_Name.begin(), currentEmber.m_Name.end(), ' ', '_');
		}
		else if (!Compare(curAtt->name, "version"))
		{
			if (ToLower(string(attStr)).find("ember") != string::npos)
				fromEmber = true;
		}
		else if (!Compare(curAtt->name, "size"))
		{
			istringstream is(attStr);
			is >> currentEmber.m_FinalRasW >> currentEmber.m_FinalRasH;
			currentEmber.m_OrigFinalRasW = currentEmber.m_FinalRasW;
			currentEmber.m_OrigFinalRasH = currentEmber.m_FinalRasH;
		}
		else if (!Compare(curAtt->name, "center"))
		{
			istringstream is(attStr);
			is >> currentEmber.m_CenterX >> currentEmber.m_CenterY;
			currentEmber.m_RotCenterY = currentEmber.m_CenterY;
		}
		else if (!Compare(curAtt->name, "filter_shape"))
		{
			currentEmber.m_SpatialFilterType = SpatialFilterCreator<T>::FromString(string(attStr));
		}
		else if (!Compare(curAtt->name, "temporal_filter_type"))
		{
			currentEmber.m_TemporalFilterType = TemporalFilterCreator<T>::FromString(string(attStr));
		}
		else if (!Compare(curAtt->name, "palette_mode"))
		{
			if (!_stricmp("step", attStr))
				currentEmber.m_PaletteMode = ePaletteMode::PALETTE_STEP;
			else if (!_stricmp("linear", attStr))
				currentEmber.m_PaletteMode = ePaletteMode::PALETTE_LINEAR;
			else
			{
				currentEmber.m_PaletteMode = ePaletteMode::PALETTE_STEP;
				AddToReport(string(loc) + " : Unrecognized palette mode " + string(attStr) + ", using step");
			}
		}
		else if (!Compare(curAtt->name, "background"))
		{
			istringstream is(attStr);
			is >> currentEmber.m_Background[0]//[0..1]
			   >> currentEmber.m_Background[1]
			   >> currentEmber.m_Background[2];
		}
		else if (!Compare(curAtt->name, "curves"))
		{
			auto splits = Split(attStr, ' ');
			istringstream is(attStr);

			for (i = 0; i < 4; i++)
			{
				vector<v2F> vals;

				for (glm::length_t j = 0; j < 4; j++)
				{
					T w, x = 0, y = 0;
					is >> x
					   >> y
					   >> w;//Weights appear to be unused.
					vals.push_back({ Clamp<T>(x, 0, 1), Clamp<T>(y, 0, 1) });
				}

				std::sort(vals.begin(), vals.end(), [&](auto & lhs, auto & rhs) { return lhs.x < rhs.x; });

				if (vals[0] == v2F(0) && vals[1] == v2F(0))
					vals[1] = v2F(0.25);

				if (vals[2] == v2F(1) && vals[3] == v2F(1))
					vals[3] = v2F(0.75);

				currentEmber.m_Curves.m_Points[i] = vals;
			}
		}
		else if (!Compare(curAtt->name, "overall_curve"))
		{
			//cout << "found overall curves\n";
			const auto splits = Split(attStr, ' ');
			istringstream is(attStr);
			vector<v2F> vals;
			T x = 0, y = 0;

			while (is >> x && is >> y)//No weights when using this format.
			{
				vals.push_back({ Clamp<T>(x, 0, 1), Clamp<T>(y, 0, 1) });
			}

			std::sort(vals.begin(), vals.end(), [&](auto & lhs, auto & rhs) { return lhs.x < rhs.x; });

			if (vals[0] == v2F(0) && vals[1] == v2F(0))
				vals[1] = v2F(0.25);

			if (vals[2] == v2F(1) && vals[3] == v2F(1))
				vals[3] = v2F(0.75);

			currentEmber.m_Curves.m_Points[0] = vals;
		}
		else if (!Compare(curAtt->name, "red_curve"))
		{
			//cout << "found red curves\n";
			const auto splits = Split(attStr, ' ');
			istringstream is(attStr);
			vector<v2F> vals;
			T x = 0, y = 0;

			while (is >> x && is >> y)//No weights when using this format.
			{
				vals.push_back({ Clamp<T>(x, 0, 1), Clamp<T>(y, 0, 1) });
			}

			std::sort(vals.begin(), vals.end(), [&](auto & lhs, auto & rhs) { return lhs.x < rhs.x; });

			if (vals[0] == v2F(0) && vals[1] == v2F(0))
				vals[1] = v2F(0.25);

			if (vals[2] == v2F(1) && vals[3] == v2F(1))
				vals[3] = v2F(0.75);

			currentEmber.m_Curves.m_Points[1] = vals;
		}
		else if (!Compare(curAtt->name, "green_curve"))
		{
			//cout << "found green curves\n";
			const auto splits = Split(attStr, ' ');
			istringstream is(attStr);
			vector<v2F> vals;
			T x = 0, y = 0;

			while (is >> x && is >> y)//No weights when using this format.
			{
				vals.push_back({ Clamp<T>(x, 0, 1), Clamp<T>(y, 0, 1) });
			}

			std::sort(vals.begin(), vals.end(), [&](auto & lhs, auto & rhs) { return lhs.x < rhs.x; });

			if (vals[0] == v2F(0) && vals[1] == v2F(0))
				vals[1] = v2F(0.25);

			if (vals[2] == v2F(1) && vals[3] == v2F(1))
				vals[3] = v2F(0.75);

			currentEmber.m_Curves.m_Points[2] = vals;
		}
		else if (!Compare(curAtt->name, "blue_curve"))
		{
			//cout << "found blue curves\n";
			const auto splits = Split(attStr, ' ');
			istringstream is(attStr);
			vector<v2F> vals;
			T x = 0, y = 0;

			while (is >> x && is >> y)//No weights when using this format.
			{
				vals.push_back({ Clamp<T>(x, 0, 1), Clamp<T>(y, 0, 1) });
			}

			std::sort(vals.begin(), vals.end(), [&](auto & lhs, auto & rhs) { return lhs.x < rhs.x; });

			if (vals[0] == v2F(0) && vals[1] == v2F(0))
				vals[1] = v2F(0.25);

			if (vals[2] == v2F(1) && vals[3] == v2F(1))
				vals[3] = v2F(0.75);

			currentEmber.m_Curves.m_Points[3] = vals;
		}
		else if (!Compare(curAtt->name, "animations"))
		{
		}

		xmlFree(attStr);
	}

	//Finished with ember attributes. Now look at the children of the ember element.
	for (childNode = emberNode->children; childNode; childNode = childNode->next)
	{
		if (!Compare(childNode->name, "color"))
		{
			index = -1;
			float r = 0, g = 0, b = 0, a = 0;
			//Loop through the attributes of the color element.
			att = childNode->properties;

			if (att == nullptr)
			{
				AddToReport(string(loc) + " : No attributes for color element");
				continue;
			}

			for (curAtt = att; curAtt; curAtt = curAtt->next)
			{
				a = 255;
				attStr = reinterpret_cast<char*>(xmlGetProp(childNode, curAtt->name));
				istringstream is(attStr);
				//This signifies that a palette is not being retrieved from the palette file, rather it's being parsed directly out of the ember xml.
				//This also means the palette has already been hue adjusted and it doesn't need to be done again, which would be necessary if it were
				//coming from the palette file.
				currentEmber.m_Palette.m_Index = -1;

				if (!Compare(curAtt->name, "index"))
					Aton(attStr, index);
				else if (!Compare(curAtt->name, "rgb"))
					is >> r >> g >> b;
				else if (!Compare(curAtt->name, "rgba"))
					is >> r >> g >> b >> a;
				else if (!Compare(curAtt->name, "a"))
					is >> a;
				else
					AddToReport(string(loc) + " : Unknown color attribute " + string(CCX(curAtt->name)));

				xmlFree(attStr);
			}

			while (index >= currentEmber.m_Palette.Size())
				currentEmber.m_Palette.m_Entries.push_back(v4F());

			float alphaPercent = a / 255.0f;//Aplha percentage in the range of 0 to 1.
			//Premultiply the palette.
			currentEmber.m_Palette.m_Entries[index].r = alphaPercent * (r / 255.0f);//Palette colors are [0..255], convert to [0..1].
			currentEmber.m_Palette.m_Entries[index].g = alphaPercent * (g / 255.0f);
			currentEmber.m_Palette.m_Entries[index].b = alphaPercent * (b / 255.0f);
			currentEmber.m_Palette.m_Entries[index].a = a / 255.0f;//Will be one for RGB, and other than one if RGBA with A != 255.
		}
		else if (!Compare(childNode->name, "colors"))
		{
			//Loop through the attributes of the color element.
			att = childNode->properties;

			if (att == nullptr)
			{
				AddToReport(string(loc) + " : No attributes for colors element");
				continue;
			}

			for (curAtt = att; curAtt; curAtt = curAtt->next)
			{
				attStr = reinterpret_cast<char*>(xmlGetProp(childNode, curAtt->name));

				if (!Compare(curAtt->name, "count"))
				{
					Aton(attStr, count);
				}
				else if (!Compare(curAtt->name, "data"))
				{
					if (!ParseHexColors(attStr, currentEmber, count, -4))
					{
						AddToReport(string(loc) + " : Error parsing hexformatted colors, some may be set to zero");
					}
				}
				else
				{
					AddToReport(string(loc) + " : Unknown color attribute " + string(CCX(curAtt->name)));
				}

				xmlFree(attStr);
			}
		}
		else if (!Compare(childNode->name, "palette"))
		{
			//This could be either the old form of palette or the new form.
			//Make sure BOTH are not specified, otherwise either are ok.
			int numColors = 0;
			int numBytes = 0;
			//Loop through the attributes of the palette element.
			att = childNode->properties;

			if (att == nullptr)
			{
				AddToReport(string(loc) + " : No attributes for palette element");
				continue;
			}

			for (curAtt = att; curAtt; curAtt = curAtt->next)
			{
				attStr = reinterpret_cast<char*>(xmlGetProp(childNode, curAtt->name));

				if (!Compare(curAtt->name, "count"))
				{
					Aton(attStr, numColors);
				}
				else if (!Compare(curAtt->name, "format"))
				{
					if (!_stricmp(attStr, "RGB"))
						numBytes = 3;
					else if (!_stricmp(attStr, "RGBA"))
						numBytes = 4;
					else
					{
						AddToReport(string(loc) + " : Unrecognized palette format string " + string(attStr) + ", defaulting to RGB");
						numBytes = 3;
					}
				}
				else if (!Compare(curAtt->name, "source_colors"))
				{
					string s(attStr);
					const auto vec1 = Split(s, ' ');

					for (auto& v : vec1)
					{
						const auto vec2 = Split(v, ',');

						if (vec2.size() == 4)
						{
							float d1 = Clamp(std::stof(vec2[0]), 0.0f, 1.0f);
							currentEmber.m_Palette.m_SourceColors[d1] = v4F(
										Clamp(std::stof(vec2[1]), 0.0f, 1.0f),
										Clamp(std::stof(vec2[2]), 0.0f, 1.0f),
										Clamp(std::stof(vec2[3]), 0.0f, 1.0f), 1.0f);
						}
						else
							AddToReport(string(loc) + " : Bad palette color source value: " + v);
					}
				}
				else
				{
					AddToReport(string(loc) + " : Unknown palette attribute " + string(CCX(curAtt->name)));
				}

				xmlFree(attStr);
			}

			//Removing support for whatever "old format" was in flam3.
			//Read formatted string from contents of tag.
			char* palStr = CX(xmlNodeGetContent(childNode));

			if (!ParseHexColors(palStr, currentEmber, numColors, numBytes))
			{
				AddToReport(string(loc) + " : Problem reading hexadecimal color data in palette");
			}

			xmlFree(palStr);
		}
		else if (!Compare(childNode->name, "symmetry"))
		{
			int symKind = INT_MAX;
			//Loop through the attributes of the palette element.
			att = childNode->properties;

			if (att == nullptr)
			{
				AddToReport(string(loc) + " : No attributes for palette element");
				continue;
			}

			for (curAtt = att; curAtt; curAtt = curAtt->next)
			{
				attStr = reinterpret_cast<char*>(xmlGetProp(childNode, curAtt->name));

				if (!Compare(curAtt->name, "kind"))
				{
					Aton(attStr, symKind);
				}
				else
				{
					AddToReport(string(loc) + " : Unknown symmetry attribute " + string(attStr));
					continue;
				}

				xmlFree(attStr);
			}

			//if (symKind != INT_MAX)//What to do about this? Should sym not be saved? Or perhaps better intelligence when adding?//TODO//BUG.
			//{
			//	currentEmber.AddSymmetry(symKind, *(GlobalRand.get()));//Determine what to do here.
			//}
		}
		else if (!Compare(childNode->name, "xform") || !Compare(childNode->name, "finalxform"))
		{
			Xform<T>* theXform = nullptr;

			if (!Compare(childNode->name, "finalxform"))
			{
				Xform<T> finalXform;
				finalXform.m_Animate = 0;//Do not animate final by default.
				finalXform.m_AnimateOrigin = 0;

				if (!ParseXform(childNode, finalXform, false, fromEmber))
				{
					AddToReport(string(loc) + " : Error parsing final xform");
				}
				else
				{
					if (finalXform.m_Weight != 0)
					{
						finalXform.m_Weight = 0;
						AddToReport(string(loc) + " : Final xforms should not have weight specified, setting to zero");
					}

					currentEmber.SetFinalXform(finalXform);
					theXform = currentEmber.NonConstFinalXform();
				}
			}
			else
			{
				Xform<T> xform;

				if (!ParseXform(childNode, xform, false, fromEmber))
				{
					AddToReport(string(loc) + " : Error parsing xform");
				}
				else
				{
					currentEmber.AddXform(xform);
					theXform = currentEmber.GetXform(currentEmber.XformCount() - 1);
				}
			}

			if (theXform)
			{
				//Check for non-zero motion params.
				if (fabs(theXform->m_MotionFreq) > 0.0)//Original checked for motion func being non-zero, but it was set to MOTION_SIN (1) in Xform::Init(), so don't check for 0 here.
				{
					AddToReport(string(loc) + " : Motion parameters should not be specified in regular, non-motion xforms");
				}

				//Motion Language:  Check the xform element for children - should be named 'motion'.
				for (motionNode = childNode->children; motionNode; motionNode = motionNode->next)
				{
					if (!Compare(motionNode->name, "motion"))
					{
						Xform<T> xform(false);//Will only have valid values in fields parsed for motion, all others will be EMPTYFIELD.

						if (!ParseXform(motionNode, xform, true, fromEmber))
							AddToReport(string(loc) + " : Error parsing motion xform");
						else
							theXform->m_Motion.push_back(xform);
					}
				}
			}
		}
		else if (!Compare(childNode->name, "edit"))
		{
			//Create a new XML document with this edit node as the root node.
			currentEmber.m_Edits = xmlNewDoc(XC("1.0"));
			editNode = xmlCopyNode(childNode, 1);
			xmlDocSetRootElement(currentEmber.m_Edits, editNode);
		}
		else if (!Compare(childNode->name, "flame_motion"))
		{
			EmberMotion<T> motion;
			att = childNode->properties;

			if (att == nullptr)
			{
				AddToReport(string(loc) + " : <flame_motion> element has no attributes");
				return false;
			}

			for (curAtt = att; curAtt; curAtt = curAtt->next)
			{
				attStr = reinterpret_cast<char*>(xmlGetProp(childNode, curAtt->name));

                if (ParseAndAssign(curAtt->name, attStr, "motion_frequency", motion.m_MotionFreq)) {}
                else if (ParseAndAssign(curAtt->name, attStr, "motion_offset", motion.m_MotionOffset)) {}
				else if (!Compare(curAtt->name, "motion_function"))
				{
					string func(attStr);

					if (func == "sin")
						motion.m_MotionFunc = eMotion::MOTION_SIN;
					else if (func == "triangle")
						motion.m_MotionFunc = eMotion::MOTION_TRIANGLE;
					else if (func == "hill")
						motion.m_MotionFunc = eMotion::MOTION_HILL;
					else if (func == "saw")
						motion.m_MotionFunc = eMotion::MOTION_SAW;
					else
					{
						AddToReport(string(loc) + " : invalid flame motion function " + func + ", setting to sin");
						motion.m_MotionFunc = eMotion::MOTION_SIN;
					}
				}
				else if (!Compare(curAtt->name, "zoom"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_ZOOM, motion);
				else if (!Compare(curAtt->name, "cam_zpos"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_ZPOS, motion);
				else if (!Compare(curAtt->name, "cam_persp"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_PERSPECTIVE, motion);
				else if (!Compare(curAtt->name, "cam_yaw"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_YAW, motion);
				else if (!Compare(curAtt->name, "cam_pitch"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_PITCH, motion);
				else if (!Compare(curAtt->name, "cam_dof"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_DEPTH_BLUR, motion);
				else if (!Compare(curAtt->name, "rotate"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_ROTATE, motion);
				else if (!Compare(curAtt->name, "brightness"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_BRIGHTNESS, motion);
				else if (!Compare(curAtt->name, "gamma"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_GAMMA, motion);
				else if (!Compare(curAtt->name, "gamma_threshold"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_GAMMA_THRESH, motion);
				else if (!Compare(curAtt->name, "highlight_power"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_HIGHLIGHT_POWER, motion);
				else if (!Compare(curAtt->name, "logscale_k2"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_K2, motion);
				else if (!Compare(curAtt->name, "rand_range"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_RAND_RANGE, motion);
				else if (!Compare(curAtt->name, "vibrancy"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_VIBRANCY, motion);
				else if (!Compare(curAtt->name, "scale"))
					ret = ret && AttToEmberMotionFloat(att, attStr, eEmberMotionParam::FLAME_MOTION_SCALE, motion);
				else if (!Compare(curAtt->name, "background"))
				{
					double r = 0, g = 0, b = 0;
					istringstream is(attStr);
					is >> r >> g >> b;

					if (r != 0)
						motion.m_MotionParams.push_back(MotionParam<T>(eEmberMotionParam::FLAME_MOTION_BACKGROUND_R, T(r)));

					if (g != 0)
						motion.m_MotionParams.push_back(MotionParam<T>(eEmberMotionParam::FLAME_MOTION_BACKGROUND_G, T(g)));

					if (b != 0)
						motion.m_MotionParams.push_back(MotionParam<T>(eEmberMotionParam::FLAME_MOTION_BACKGROUND_B, T(b)));
				}
				else if (!Compare(curAtt->name, "center"))
				{
					double cx = 0, cy = 0;
					istringstream is(attStr);
					is >> cx >> cy;

					if (cx != 0)
						motion.m_MotionParams.push_back(MotionParam<T>(eEmberMotionParam::FLAME_MOTION_CENTER_X, T(cx)));

					if (cy != 0)
						motion.m_MotionParams.push_back(MotionParam<T>(eEmberMotionParam::FLAME_MOTION_CENTER_Y, T(cy)));
				}
				else
				{
					AddToReport(string(loc) + " : Unknown flame motion attribute " + string(CCX(curAtt->name)));
				}

				xmlFree(attStr);
			}

			currentEmber.m_EmberMotionElements.push_back(motion);
		}
	}

	//If new_linear == 0, manually add a linear
	if (!fromEmber && !newLinear)
		currentEmber.Flatten(m_FlattenNames);

	if (soloXform >= 0)
		for (i = 0; i < currentEmber.XformCount(); i++)
            currentEmber.GetXform(i)->m_Opacity = T(i == static_cast<size_t>(soloXform));//Will calc the cached adjusted viz value later.

	return true;
}
/// <summary>
/// Parse a floating point value from an xml attribute and add the value to a EmberMotion object
/// </summary>
/// <param name="att">The current attribute</param>
/// <param name="attStr">The attribute value to parse</param>
/// <param name="param">The flame motion parameter type</param>
/// <param name="motion">The flame motion element to add the parameter to</param>
/// <returns>True if there were no errors, else false.</returns>
template <typename T>
bool XmlToEmber<T>::AttToEmberMotionFloat(xmlAttrPtr att, const char* attStr, eEmberMotionParam param, EmberMotion<T>& motion)
{
	const char* loc = __FUNCTION__;
	bool r = false;
	T val = 0.0;

	if (Aton(attStr, val))
	{
		motion.m_MotionParams.push_back(MotionParam<T>(param, val));
		r = true;
	}
	else
	{
		AddToReport(string(loc) + " : Failed to parse float value for flame motion attribute \"" + string(CCX(att->name)) + "\" : " + string(attStr));
	}

	return r;
}
/// <summary>
/// Parse an xform element.
/// </summary>
/// <param name="childNode">The current node to parse</param>
/// <param name="xform">The newly constructed xform based on what was parsed</param>
/// <param name="motion">True if this xform is a motion within a parent xform, else false</param>
/// <returns>True if there were no errors, else false.</returns>
template <typename T>
bool XmlToEmber<T>::ParseXform(xmlNode* childNode, Xform<T>& xform, bool motion, bool fromEmber)
{
	char* attStr;
	const char* loc = __FUNCTION__;
	size_t j;
	T temp;
	double a, b, c, d, e, f;
	xmlAttrPtr attPtr, curAtt;
	//Loop through the attributes of the xform element.
	attPtr = childNode->properties;

	if (attPtr == nullptr)
	{
		AddToReport(string(loc) + " : Error: No attributes for element");
		return false;
	}

	for (curAtt = attPtr; curAtt; curAtt = curAtt->next)
	{
		attStr = reinterpret_cast<char*>(xmlGetProp(childNode, curAtt->name));

		//First parse out simple float reads.
        if (ParseAndAssign(curAtt->name, attStr, "weight", xform.m_Weight)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "color_speed", xform.m_ColorSpeed)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "animate", xform.m_Animate)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "animate_origin", xform.m_AnimateOrigin)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "opacity", xform.m_Opacity)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "var_color", xform.m_DirectColor)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "motion_frequency", xform.m_MotionFreq)) {}
        else if (ParseAndAssign(curAtt->name, attStr, "motion_offset", xform.m_MotionOffset)) {}
		//Parse more complicated reads that have multiple possible values.
		else if (!Compare(curAtt->name, "name"))
		{
			xform.m_Name = string(attStr);
			std::replace(xform.m_Name.begin(), xform.m_Name.end(), ' ', '_');
		}
		else if (!fromEmber && !Compare(curAtt->name, "symmetry"))//Legacy support.
		{
			//Deprecated, set both color_speed and animate to this value.
			//Huh? Either set it or not?
			Aton(attStr, temp);
			xform.m_ColorSpeed = (1 - temp) / 2;
			xform.m_Animate = T(temp > 0 ? 0 : 1);
			//Ignore m_AnimateOrigin because it's new and symmetry is a legacy setting.
		}
		else if (!Compare(curAtt->name, "motion_function"))
		{
			if (!_stricmp("sin", attStr))
				xform.m_MotionFunc = eMotion::MOTION_SIN;
			else if (!_stricmp("triangle", attStr))
				xform.m_MotionFunc = eMotion::MOTION_TRIANGLE;
			else if (!_stricmp("hill", attStr))
				xform.m_MotionFunc = eMotion::MOTION_HILL;
			else if (!_stricmp("saw", attStr))
				xform.m_MotionFunc = eMotion::MOTION_SAW;
			else
			{
				xform.m_MotionFunc = eMotion::MOTION_SIN;
				AddToReport(string(loc) + " : Unknown motion function " + string(attStr) + ", using sin");
			}
		}
		else if (!Compare(curAtt->name, "color"))
		{
			istringstream is(attStr);
			xform.m_ColorX = xform.m_ColorY = static_cast<T>(0.5);
			is >> xform.m_ColorX;
			is >> xform.m_ColorY;//Very unlikely to be present, but leave for future use.
		}
		else if (!Compare(curAtt->name, "chaos"))
		{
			istringstream is(attStr);
			j = 0;

			while (is >> temp)
			{
				xform.SetXaos(j, temp);
				j++;
			}
		}
		else if (!Compare(curAtt->name, "plotmode"))
		{
			if (motion == 1)
			{
				AddToReport(string(loc) + " : Motion element cannot have a plotmode attribute");
			}
			else if (!_stricmp("off", attStr))
				xform.m_Opacity = 0;
		}
		else if (!Compare(curAtt->name, "coefs"))
		{
			istringstream is(attStr);
			a = b = c = d = e = f = 0;
			is >> a >> d >> b >> e >> c >> f;
			xform.m_Affine.A(T(a));
			xform.m_Affine.B(T(b));
			xform.m_Affine.C(T(c));
			xform.m_Affine.D(T(d));
			xform.m_Affine.E(T(e));
			xform.m_Affine.F(T(f));
		}
		else if (!Compare(curAtt->name, "post"))
		{
			istringstream is(attStr);
			a = b = c = d = e = f = 0;
			is >> a >> d >> b >> e >> c >> f;
			xform.m_Post.A(T(a));
			xform.m_Post.B(T(b));
			xform.m_Post.C(T(c));
			xform.m_Post.D(T(d));
			xform.m_Post.E(T(e));
			xform.m_Post.F(T(f));
		}
		else
		{
			//Only correct names if it came from an outside source. Names originating from this library are always considered correct.
			string s = fromEmber ? string(CCX(curAtt->name)) : GetCorrectedVariationName(m_BadVariationNames, curAtt);

			if (const auto var = m_VariationList->GetVariation(s))
			{
				T weight = 0;
				Aton(attStr, weight);

				if (!IsNearZero(weight))//Having a variation with zero weight makes no sense, so guard against it.
				{
					auto varCopy = var->Copy();
					varCopy->m_Weight = weight;
					xform.AddVariation(varCopy);
				}
			}

			//else
			//{
			//	AddToReport("Unsupported variation: " + string((const char*)curAtt->name));
			//}
		}

		xmlFree(attStr);
	}

	//Handle var1.
	for (curAtt = attPtr; curAtt; curAtt = curAtt->next)//Legacy fields, most likely not used.
	{
		bool var1 = false;

		if (!Compare(curAtt->name, "var1"))
		{
			attStr = reinterpret_cast<char*>(xmlGetProp(childNode, curAtt->name));

			for (j = 0; j < xform.TotalVariationCount(); j++)
				xform.GetVariation(j)->m_Weight = 0;

			if (Aton(attStr, temp))
			{
				uint iTemp = static_cast<uint>(temp);

				if (iTemp < xform.TotalVariationCount())
				{
					xform.GetVariation(iTemp)->m_Weight = 1;
					var1 = true;
				}
			}

			if (!var1)
				AddToReport(string(loc) + " : Bad value for var1 " + string(attStr));

			xmlFree(attStr);
			break;
		}
	}

	//Handle var.
	for (curAtt = attPtr; curAtt; curAtt = curAtt->next)//Legacy fields, most likely not used.
	{
		bool var = false;

		if (!Compare(curAtt->name, "var"))
		{
			attStr = reinterpret_cast<char*>(xmlGetProp(childNode, curAtt->name));

			if (Aton(attStr, temp))
			{
				for (j = 0; j < xform.TotalVariationCount(); j++)
					xform.GetVariation(j)->m_Weight = temp;

				var = true;
			}

			if (!var)
				AddToReport(string(loc) + " : Bad value for var " + string(attStr));

			xmlFree(attStr);
			break;
		}
	}

	//Now that all xforms have been parsed, go through and try to find params for the parametric variations.
	for (size_t i = 0; i < xform.TotalVariationCount(); i++)
	{
		if (const auto parVar = dynamic_cast<ParametricVariation<T>*>(xform.GetVariation(i)))
		{
			string prefix;

			if (!fromEmber)
			{
				if (parVar->VarType() == eVariationType::VARTYPE_PRE)
					prefix = "pre_";
				else if (parVar->VarType() == eVariationType::VARTYPE_POST)
					prefix = "post_";
			}

			for (curAtt = attPtr; curAtt; curAtt = curAtt->next)
			{
				//Only correct names if it came from an outside source. Names originating from this library are always considered correct.
				string s = fromEmber ? string(CCX(curAtt->name)) : GetCorrectedParamName(m_BadParamNames, ToLower(CCX(curAtt->name)).c_str());

				if (!fromEmber)
				{
					auto varnamewithunderscore = parVar->m_Name + "_";
					auto fullprefix = prefix + varnamewithunderscore;

					if (StartsWith(s, fullprefix, true))
					{
						//Do nothing.
					}
					else if (!StartsWith(s, varnamewithunderscore, true) && (prefix.size() == 0 || !StartsWith(s, prefix, true)))
					{
						s = fullprefix + s;
					}
					else if (StartsWith(s, varnamewithunderscore, true))
					{
						s = prefix + s;
					}
				}

				const char* name = s.c_str();

				if (parVar->ContainsParam(name))
				{
					T val = 0;
					attStr = CX(xmlGetProp(childNode, curAtt->name));

					if (Aton(attStr, val))
					{
						if (!parVar->SetParamVal(name, val))
							AddToReport(string(loc) + " : Failed to set parametric variation parameter " + name);
					}
					else
					{
						AddToReport(string(loc) + " : Failed to parse parametric variation parameter " + s + " - " + string(attStr));
					}

					xmlFree(attStr);
				}
			}
		}
	}

	return true;
}
/// <summary>
/// Some Apophysis plugins use an inconsistent naming scheme for the parametric variation variables.
/// This function identifies and converts them to Ember's consistent naming convention.
/// </summary>
/// <param name="names">The map of corrected names to search</param>
/// <param name="name">The current Xml node to check</param>
/// <returns>The corrected name if one was found, else the passed in name.</returns>
template <typename T>
string XmlToEmber<T>::GetCorrectedParamName(const unordered_map<string, string>& names, const char* name)
{
	const auto& newName = names.find(ToLower(name));

	if (newName != names.end())
		return newName->second;
	else
		return name;
}
/// <summary>
/// Some Apophysis plugins use an inconsistent naming scheme for variation names.
/// This function identifies and converts them to Ember's consistent naming convention.
/// It uses some additional intelligence to ensure the variation is the expected one,
/// by examining the rest of the xform for the existence of parameter names.
/// </summary>
/// <param name="vec">The vector of corrected names to search</param>
/// <param name="att">The xml attribute pointer whose name member is the name of the variation to check</param>
/// <returns>The corrected name if one was found, else the passed in name.</returns>
template <typename T>
string XmlToEmber<T>::GetCorrectedVariationName(vector<pair<pair<string, string>, vector<string>>>& vec, xmlAttrPtr att)
{
	for (auto& v : vec)
	{
		if (!_stricmp(v.first.first.c_str(), CCX(att->name)))//Do case insensitive here.
		{
			if (!v.second.empty())
			{
				for (size_t j = 0; j < v.second.size(); j++)
				{
					if (XmlContainsTag(att, v.second[j].c_str()))
						return v.first.second;
				}
			}
			else
			{
				return v.first.second;
			}
		}
	}

	return string(CCX(att->name));
}
/// <summary>
/// Some Apophysis plugins use an inconsistent naming scheme for variation names.
/// This function identifies and converts them to Ember's consistent naming convention.
/// It uses some additional intelligence to ensure the variation is the expected one,
/// by examining the rest of the xform for the existence of parameter names.
/// This overload is specifically for use when parsing .chaos files from Chaotica.
/// </summary>
/// <param name="vec">The vector of corrected names to search</param>
/// <param name="varname">The name of the variation to check</param>
/// <returns>The corrected name if one was found, else the passed in name.</returns>
template <typename T>
string XmlToEmber<T>::GetCorrectedVariationName(vector<pair<pair<string, string>, vector<string>>>& vec, const string& varname)
{
	if (varname == "poincare")//for Apo flames, poincare must be the same, but Chaotica poincare is implemented as poincare2
		return "poincare2";
	else if (varname == "fisheye")
		return "eyefish";//Chaotica automatically uses eyefish instead of fisheye.
	else if (varname == "phoenixjulia")
		return "phoenix_julia";
	else if (varname != "mobius")//Chaotica actually gets this right, but Apophysis doesn't.
		for (auto& v : vec)
			if (!_stricmp(v.first.first.c_str(), varname.c_str()))//Do case insensitive here.
				return v.first.second;

	return varname;
}

/// <summary>
/// Determine if an Xml node contains a given tag.
/// </summary>
/// <param name="att">The Xml node to search</param>
/// <param name="name">The node name to search for</param>
/// <returns>True if name was found, else false.</returns>
template <typename T>
bool XmlToEmber<T>::XmlContainsTag(xmlAttrPtr att, const char* name)
{
	xmlAttrPtr temp = att;

	do
	{
		if (!_stricmp(name, CCX(temp->name)))
			return true;
	}
	while ((temp = temp->next));

	return false;
}
/// <summary>
/// Parse hexadecimal colors.
/// This can read RGB and RGBA, however only RGB will be stored.
/// </summary>
/// <param name="colstr">The string of hex colors to parse</param>
/// <param name="ember">The ember whose palette will be assigned the colors</param>
/// <param name="numColors">The number of colors present</param>
/// <param name="chan">The number of channels in each color</param>
/// <returns>True if there were no errors, else false.</returns>
template <typename T>
bool XmlToEmber<T>::ParseHexColors(const char* colstr, Ember<T>& ember, size_t numColors, intmax_t chan)
{
	stringstream ss, temp(colstr); ss >> std::hex;
	string s1, s;
	size_t tmp, colorCount = 0;
	s.reserve(1536);

	while (temp >> s1)
		s += s1;

	auto length = s.size();

	for (size_t strIndex = 0; strIndex < length;)
	{
		for (glm::length_t i = 0; i < 3; i++)
		{
			const char tmpStr[3] = { s[strIndex++], s[strIndex++], 0 };//Read out and convert the string two characters at a time.
			ss.clear();//Reset and fill the string stream.
			ss.str(tmpStr);
			ss >> tmp;//Do the conversion.

			while (colorCount >= ember.m_Palette.Size())
				ember.m_Palette.m_Entries.push_back(v4F());

			ember.m_Palette.m_Entries[colorCount][i] = static_cast<float>(tmp) / 255.0f;//Hex palette is [0..255], convert to [0..1].
		}

		ember.m_Palette.m_Entries[colorCount][3] = static_cast<float>(1);
		colorCount++;
	}

	return length >= 256;
}
/// <summary>
/// Wrapper to parse a numeric Xml string value and convert it.
/// </summary>
/// <param name="name">The xml tag to parse</param>
/// <param name="attStr">The name of the Xml attribute</param>
/// <param name="str">The name of the Xml tag</param>
/// <param name="val">The parsed value</param>
/// <param name="b">Bitwise ANDed with true if name matched str and the conversion succeeded, else false. Used for keeping a running value between successive calls.</param>
/// <returns>True if the tag was matched and the conversion succeeded, else false</returns>
template <typename T>
template <typename valT>
bool XmlToEmber<T>::ParseAndAssign(const xmlChar* name, const char* attStr, const char* str, valT& val)
{
	bool ret = false;

	if (!Compare(name, str))
	{
		istringstream istr(attStr);
		istr >> val;
		ret = !istr.bad() && !istr.fail();//Means the Compare() was right, and the conversion succeeded.
	}

	return ret;
}
//This class had to be implemented in a cpp file because the compiler was breaking.
//So the explicit instantiation must be declared here rather than in Ember.cpp where
//all of the other classes are done.
#define EXPORT_XML_TO_EMBER(T) \
	template EMBER_API class XmlToEmber<T>; \
	template EMBER_API bool XmlToEmber<T>::Parse(byte*, const char*, list<Ember<T>>&, bool); \
	template EMBER_API bool XmlToEmber<T>::Parse(byte*, const char*, vector<Ember<T>>&, bool); \
	template EMBER_API bool XmlToEmber<T>::Parse(const char*, list<Ember<T>>&, bool); \
	template EMBER_API bool XmlToEmber<T>::Parse(const char*, vector<Ember<T>>&, bool); \
	template EMBER_API bool XmlToEmber<T>::Aton(const char*, size_t&);
EXPORT_XML_TO_EMBER(float)
#ifdef DO_DOUBLE
	EXPORT_XML_TO_EMBER(double)
#endif
}
