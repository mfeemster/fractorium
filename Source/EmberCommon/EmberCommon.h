#pragma once

/// <summary>
/// Global utility classes and functions that are common to all programs that use
/// Ember and its derivatives.
/// </summary>

namespace EmberCommon
{
enum class eXaosPasteStyle : int { NONE, ZERO_TO_ONE, ZERO_TO_VALS, ONE_TO_VALS, VALS_TO_ONE };

/// <summary>
/// Derivation of the RenderCallback class to do custom printing action
/// whenever the progress function is internally called inside of Ember
/// and its derivatives.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class RenderProgress : public RenderCallback
{
public:
	/// <summary>
	/// Constructor that initializes the state to zero.
	/// </summary>
	RenderProgress() = default;
	RenderProgress(RenderProgress<T>& progress) = delete;
	~RenderProgress() = default;

	/// <summary>
	/// The progress function which will be called from inside the renderer.
	/// </summary>
	/// <param name="ember">The ember currently being rendered</param>
	/// <param name="foo">An extra dummy parameter</param>
	/// <param name="fraction">The progress fraction from 0-100</param>
	/// <param name="stage">The stage of iteration. 1 is iterating, 2 is density filtering, 2 is final accumulation.</param>
	/// <param name="etaMs">The estimated milliseconds to completion of the current stage</param>
	/// <returns>The value of m_Running, which is always true since this is intended to run in an environment where the render runs to completion, unlike interactive rendering.</returns>
	virtual int ProgressFunc(Ember<T>& ember, void* foo, double fraction, int stage, double etaMs)
	{
		if (stage == 0 || stage == 1)
		{
			if (m_LastStage != stage)
				cout << "\n";

			cout << "\r" << string(m_S.length() * 2, ' ');//Clear what was previously here, * 2 just to be safe because the end parts of previous strings might be longer.
			m_SS.str("");//Begin new output.
			m_SS << "\rStage = " << (stage ? "filtering" : "iterating");
			m_SS << ", progress = " << int(fraction) << "%";
			m_SS << ", eta = " << t.Format(etaMs);
			m_S = m_SS.str();
			cout << m_S;
		}

		m_LastStage = stage;
		return m_Running;
	}

	/// <summary>
	/// Reset the state.
	/// </summary>
	void Clear()
	{
		m_Running = 1;
		m_LastStage = 0;
		m_LastLength = 0;
		m_SS.clear();
		m_S.clear();
	}

	/// <summary>
	/// Stop this instance.
	/// </summary>
	void Stop()
	{
		m_Running = 0;
	}

private:
	int m_Running = 1;
	int m_LastStage = 0;
	int m_LastLength = 0;
	stringstream m_SS;
	string m_S;
	Timing t;
};

/// <summary>
/// Wrapper for parsing an ember Xml file, storing the embers in a vector and printing
/// any errors that occurred.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="parser">The parser to use</param>
/// <param name="filename">The full path and name of the file</param>
/// <param name="embers">Storage for the embers read from the file</param>
/// <param name="useDefaults">True to use defaults if they are not present in the file, else false to use invalid values as placeholders to indicate the values were not present. Default: true.</param>
/// <returns>True if success, else false.</returns>
template <typename T>
static bool ParseEmberFile(XmlToEmber<T>& parser, const string& filename, vector<Ember<T>>& embers, bool useDefaults = true)
{
	if (!parser.Parse(filename.c_str(), embers, useDefaults))
	{
		cerr << "Error parsing flame file " << filename << ", returning without executing.\n";
		return false;
	}

	if (embers.empty())
	{
		cerr << "Error: No data present in file " << filename << ". Aborting.\n";
		return false;
	}

	return true;
}

/// <summary>
/// Cross platform wrapper for getting the full path of the current executable.
/// </summary>
/// <param name="programPath">The value of argv[0] passed into main()</param>
/// <returns>The full path of the executable as a string</returns>
static string GetExePath(const char* argv0)
{
	string fullpath;
#ifdef _WIN32
	fullpath = argv0;
#else
	vector<char> v;
	v.resize(2048);
#if __APPLE__
	uint32_t vs = uint32_t(v.size());

	if (_NSGetExecutablePath(v.data(), &vs) == 0)
		fullpath = string(v.data());
	else
		cerr << "Could not discern full path from executable.\n";

#else
	readlink("/proc/self/exe", v.data(), v.size());
	fullpath = string(v.data());
#endif
#endif
	return GetPath(fullpath);
}

/// <summary>
/// Wrapper for parsing palette Xml file and initializing it's private static members,
/// and printing any errors that occurred.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="programPath">The full path of the folder the program is running in</param>
/// <param name="filename">The full path and name of the file</param>
/// <returns>True if success, else false.</returns>
template <typename T>
static bool InitPaletteList(const string& programPath, const string& filename)
{
	auto paletteList = PaletteList<float>::Instance();
	static vector<string> paths =
	{
		programPath
#ifndef _WIN32
		, "~/",
		"~/.config/fractorium/",
		"/usr/share/fractorium/",
		"/usr/local/share/fractorium/"
#endif
	};
	bool added = false;

	for (auto& p : paths)
	{
		auto fullpath = p + filename;
		//cout << "Trying: " << fullpath << endl;

		if (!added)
		{
			if (std::ifstream(fullpath))
				added |= paletteList->Add(fullpath);
		}
		else
			break;
	}

	if (!added || !paletteList->Size())
	{
		cerr << "Error parsing palette file " << filename << ". Reason: \n"
			 << paletteList->ErrorReportString() << "\nReturning without executing.\n";
		return false;
	}

	return true;
}

/// <summary>
/// Formats a filename with digits using the passed in amount of 0 padding.
/// </summary>
/// <param name="result">The ember whose name will be set</param>
/// <param name="os">The ostringstream which will be used to format</param>
/// <param name="padding">The amount of padding to use</param>
template <typename T>
void FormatName(Ember<T>& result, ostringstream& os, streamsize padding)
{
	os << std::setw(padding) << result.m_Time;
	result.m_Name = os.str();
	os.str("");
}

/// <summary>
/// Convert an RGBA 32-bit float buffer to an RGB 8-bit buffer.
/// The two buffers can point to the same memory location if needed.
/// </summary>
/// <param name="rgba">The RGBA 32-bit float buffer</param>
/// <param name="rgb">The RGB 8-bit buffer</param>
/// <param name="width">The width of the image in pixels</param>
/// <param name="height">The height of the image in pixels</param>
static void Rgba32ToRgb8(const v4F* rgba, unsigned char* rgb, size_t width, size_t height)
{
	if (rgba != nullptr && rgb != nullptr)
	{
		for (size_t i = 0, j = 0; i < (width * height); i++)
		{
			rgb[j++] = static_cast<unsigned char>(Clamp<float>(rgba[i].r * 255.0f, 0.0f, 255.0f));
			rgb[j++] = static_cast<unsigned char>(Clamp<float>(rgba[i].g * 255.0f, 0.0f, 255.0f));
			rgb[j++] = static_cast<unsigned char>(Clamp<float>(rgba[i].b * 255.0f, 0.0f, 255.0f));
		}
	}
}

/// <summary>
/// Convert an RGBA 32-bit float buffer to an RGBA 8-bit buffer.
/// The two buffers can point to the same memory location if needed.
/// </summary>
/// <param name="rgba">The RGBA 32-bit float buffer</param>
/// <param name="rgb">The RGBA 8-bit buffer</param>
/// <param name="width">The width of the image in pixels</param>
/// <param name="height">The height of the image in pixels</param>
/// <param name="doAlpha">True to use alpha transparency, false to assign the max alpha value to make each pixel fully visible</param>
static void Rgba32ToRgba8(const v4F* rgba, unsigned char* rgb, size_t width, size_t height, bool doAlpha)
{
	if (rgba != nullptr && rgb != nullptr)
	{
		for (size_t i = 0, j = 0; i < (width * height); i++)
		{
			rgb[j++] = static_cast<unsigned char>(Clamp<float>(rgba[i].r * 255.0f, 0.0f, 255.0f));
			rgb[j++] = static_cast<unsigned char>(Clamp<float>(rgba[i].g * 255.0f, 0.0f, 255.0f));
			rgb[j++] = static_cast<unsigned char>(Clamp<float>(rgba[i].b * 255.0f, 0.0f, 255.0f));
			rgb[j++] = doAlpha ? static_cast<unsigned char>(Clamp<float>(rgba[i].a * 255.0f, 0.0f, 255.0f)) : 255;
		}
	}
}

/// <summary>
/// Convert an RGBA 32-bit float buffer to an RGBA 16-bit buffer.
/// The two buffers can point to the same memory location if needed.
/// </summary>
/// <param name="rgba">The RGBA 32-bit float buffer</param>
/// <param name="rgb">The RGBA 16-bit buffer</param>
/// <param name="width">The width of the image in pixels</param>
/// <param name="height">The height of the image in pixels</param>
/// <param name="doAlpha">True to use alpha transparency, false to assign the max alpha value to make each pixel fully visible</param>
static void Rgba32ToRgba16(const v4F* rgba, glm::uint16* rgb, size_t width, size_t height, bool doAlpha)
{
	if (rgba != nullptr && rgb != nullptr)
	{
		for (size_t i = 0, j = 0; i < (width * height); i++)
		{
			rgb[j++] = static_cast<glm::uint16>(Clamp<float>(rgba[i].r * 65535.0f, 0.0f, 65535.0f));
			rgb[j++] = static_cast<glm::uint16>(Clamp<float>(rgba[i].g * 65535.0f, 0.0f, 65535.0f));
			rgb[j++] = static_cast<glm::uint16>(Clamp<float>(rgba[i].b * 65535.0f, 0.0f, 65535.0f));
			rgb[j++] = doAlpha ? static_cast<glm::uint16>(Clamp<float>(rgba[i].a * 65535.0f, 0.0f, 65535.0f)) : glm::uint16{ 65535 };
		}
	}
}

/// <summary>
/// Convert an RGBA 32-bit float buffer to an EXR RGBA 16-bit float buffer.
/// The two buffers can point to the same memory location if needed.
/// Note that this squares the values coming in, for some reason EXR expects that.
/// </summary>
/// <param name="rgba">The RGBA 32-bit float buffer</param>
/// <param name="ilmfRgba">The EXR RGBA 16-bit float buffer</param>
/// <param name="width">The width of the image in pixels</param>
/// <param name="height">The height of the image in pixels</param>
/// <param name="doAlpha">True to use alpha transparency, false to assign the max alpha value to make each pixel fully visible</param>
static void Rgba32ToRgbaExr(const v4F* rgba, Rgba* ilmfRgba, size_t width, size_t height, bool doAlpha)
{
	if (rgba != nullptr && ilmfRgba != nullptr)
	{
		for (size_t i = 0; i < (width * height); i++)
		{
			ilmfRgba[i].r = Clamp<float>(Sqr(rgba[i].r), 0.0f, 1.0f);
			ilmfRgba[i].g = Clamp<float>(Sqr(rgba[i].g), 0.0f, 1.0f);
			ilmfRgba[i].b = Clamp<float>(Sqr(rgba[i].b), 0.0f, 1.0f);
			ilmfRgba[i].a = doAlpha ? Clamp<float>(rgba[i].a * 1.0f, 0.0f, 1.0f) : 1.0f;
		}
	}
}

/// <summary>
/// Convert an RGBA 32-bit float buffer to an EXR RGBA 32-bit float buffer.
/// The two buffers can point to the same memory location if needed.
/// Note that this squares the values coming in, for some reason EXR expects that.
/// </summary>
/// <param name="rgba">The RGBA 32-bit float buffer</param>
/// <param name="r">The EXR red 32-bit float buffer</param>
/// <param name="g">The EXR green 32-bit float buffer</param>
/// <param name="b">The EXR blue 32-bit float buffer</param>
/// <param name="a">The EXR alpha 32-bit float buffer</param>
/// <param name="width">The width of the image in pixels</param>
/// <param name="height">The height of the image in pixels</param>
/// <param name="doAlpha">True to use alpha transparency, false to assign the max alpha value to make each pixel fully visible</param>
static void Rgba32ToRgba32Exr(const v4F* rgba, float* r, float* g, float* b, float* a, size_t width, size_t height, bool doAlpha)
{
	if (rgba != nullptr && r != nullptr && g != nullptr && b != nullptr && a != nullptr)
	{
		for (size_t i = 0; i < (width * height); i++)
		{
			r[i] = Clamp<float>(Sqr(rgba[i].r), 0.0f, 1.0f);
			g[i] = Clamp<float>(Sqr(rgba[i].g), 0.0f, 1.0f);
			b[i] = Clamp<float>(Sqr(rgba[i].b), 0.0f, 1.0f);
			a[i] = doAlpha ? Clamp<float>(rgba[i].a * 1.0f, 0.0f, 1.0f) : 1.0f;
		}
	}
}

/// <summary>
/// Returns a string with all illegal file path characters removed.
/// </summary>
/// <param name="filename">The path to remove illegal characters from</param>
/// <returns>The cleaned full file path and name.</returns>
static string CleanPath(const string& filename)
{
	static string illegalChars = "\\/:*?\"<>|";
	auto tempfilename = filename;

	for (auto& ch : illegalChars)
		tempfilename.erase(remove(tempfilename.begin(), tempfilename.end(), ch), tempfilename.end());

	return tempfilename;
}

/// <summary>
/// Make a filename for a single render. This is used in EmberRender.
/// </summary>
/// <param name="path">The path portion of where to save the file</param>
/// <param name="out">The full name and path to override everything else</param>
/// <param name="finalName">The name to use when useFinalName is true</param>
/// <param name="prefix">The prefix to prepend to the filename</param>
/// <param name="suffix">True suffix to append to the filename</param>
/// <param name="format">The format extention. This must not contain a period.</param>
/// <param name="padding">The width padding to use, which will be zero filled.</param>
/// <param name="i">The numerical value to use for the filename when useFinalName is false and out is empty</param>
/// <param name="useFinalName">Whether to use the name included in the flame. The i parameter is ignored in this case.</param>
static string MakeSingleFilename(const string& path, const string& out, const string& finalName, const string& prefix, const string& suffix, const string& format, glm::uint padding, size_t i, bool useFinalName)
{
	string filename;

	if (!out.empty())
	{
		filename = out;
	}
	else if (useFinalName)
	{
		filename = path + prefix + CleanPath(finalName + suffix + "." + format);
	}
	else
	{
		ostringstream fnstream;
		fnstream << setfill('0') << setprecision(0) << fixed << setw(padding) << i << suffix << "." << format;
		filename = path + prefix + CleanPath(fnstream.str());
	}

	return filename;
}

/// <summary>
/// Make a filename for a frame of an animation render. This is used in EmberAnimate.
/// </summary>
/// <param name="path">The path portion of where to save the file</param>
/// <param name="prefix">The prefix to prepend to the filename</param>
/// <param name="suffix">True suffix to append to the filename</param>
/// <param name="format">The format extention. This must contain a period.</param>
/// <param name="padding">The width padding to use, which will be zero filled.</param>
/// <param name="ftime">The numerical value to use for the filename</param>
static string MakeAnimFilename(const string& path, const string& prefix, const string& suffix, const string& format, glm::uint padding, size_t ftime)
{
	ostringstream fnstream;
	fnstream << setfill('0') << setprecision(0) << fixed << setw(padding) << ftime << suffix << format;
	return path + prefix + CleanPath(fnstream.str());
}

/// <summary>
/// Calculate the number of strips required if the needed amount of memory
/// is greater than the system memory, or greater than what the user wants to allow.
/// </summary>
/// <param name="mem">Amount of memory required</param>
/// <param name="memAvailable">Amount of memory available on the system</param>
/// <param name="useMem">The maximum amount of memory to use. Use max if 0.</param>
/// <returns>The number of strips to use</returns>
static uint CalcStrips(double memRequired, double memAvailable, double useMem) noexcept
{
	if (useMem > 0)
		memAvailable = useMem;
	else
		memAvailable *= 0.8;

	if (memAvailable >= memRequired)
		return 1;

	return static_cast<uint>(ceil(memRequired / memAvailable));
}

/// <summary>
/// Given a numerator and a denominator, find the next highest denominator that divides
/// evenly into the numerator.
/// </summary>
/// <param name="numerator">The numerator</param>
/// <param name="denominator">The denominator</param>
/// <returns>The next highest divisor if found, else 1.</returns>
template <typename T>
static T NextHighestEvenDiv(T numerator, T denominator) noexcept
{
	T result = 1;
	T numDiv2 = numerator / 2;

	do
	{
		denominator++;

		if (numerator % denominator == 0)
		{
			result = denominator;
			break;
		}
	}
	while (denominator <= numDiv2);

	return result;
}

/// <summary>
/// Given a numerator and a denominator, find the next lowest denominator that divides
/// evenly into the numerator.
/// </summary>
/// <param name="numerator">The numerator</param>
/// <param name="denominator">The denominator</param>
/// <returns>The next lowest divisor if found, else 1.</returns>
template <typename T>
static T NextLowestEvenDiv(T numerator, T denominator) noexcept
{
	T result = 1;
	T numDiv2 = numerator / 2;
	denominator--;

	if (denominator > numDiv2)
		denominator = numDiv2;

	while (denominator >= 1)
	{
		if (numerator % denominator == 0)
		{
			result = denominator;
			break;
		}

		denominator--;
	}

	return result;
}

/// <summary>
/// Wrapper for converting a vector of absolute device indices to a vector
/// of platform,device index pairs.
/// </summary>
/// <param name="selectedDevices">The vector of absolute device indices to convert</param>
/// <returns>The converted vector of platform,device index pairs</returns>
static vector<pair<size_t, size_t>> Devices(const vector<size_t>& selectedDevices)
{
	vector<pair<size_t, size_t>> vec;
	auto info = OpenCLInfo::Instance();
	auto& devices = info->DeviceIndices();
	vec.reserve(selectedDevices.size());

	for (size_t i = 0; i < selectedDevices.size(); i++)
	{
		auto index = selectedDevices[i];

		if (index < devices.size())
			vec.push_back(devices[index]);
	}

	return vec;
}

/// <summary>
/// Wrapper for creating a renderer of the specified type.
/// </summary>
/// <param name="renderType">Type of renderer to create</param>
/// <param name="devices">The vector of platform/device indices to use</param>
/// <param name="shared">True if shared with OpenGL, else false.</param>
/// <param name="texId">The texture ID of the shared OpenGL texture if shared</param>
/// <param name="errorReport">The error report for holding errors if anything goes wrong</param>
/// <returns>A pointer to the created renderer if successful, else false.</returns>
template <typename T>
static Renderer<T, float>* CreateRenderer(eRendererType renderType, const vector<pair<size_t, size_t>>& devices, bool shared, GLuint texId, EmberReport& errorReport)
{
	string s;
	unique_ptr<Renderer<T, float>> renderer;

	try
	{
		if (renderType == eRendererType::OPENCL_RENDERER && !devices.empty())
		{
			s = "OpenCL";
			renderer = unique_ptr<Renderer<T, float>>(new RendererCL<T, float>(devices, shared, texId));//Can't use make_unique here.

			if (!renderer.get() || !renderer->Ok())
			{
				if (renderer.get())
					errorReport.AddToReport(renderer->ErrorReport());

				errorReport.AddToReport("Error initializing OpenCL renderer, using CPU renderer instead.");
				renderer = make_unique<Renderer<T, float>>();
			}
		}
		else
		{
			s = "CPU";
			renderer = make_unique<Renderer<T, float>>();
		}
	}
	catch (const std::exception& e)
	{
		errorReport.AddToReport("Error creating " + s + " renderer: " + e.what() + "\n");
	}
	catch (...)
	{
		errorReport.AddToReport("Error creating " + s + " renderer.\n");
	}

	return renderer.release();
}

/// <summary>
/// Wrapper for creating a vector of renderers of the specified type for each passed in device.
/// If shared is true, only the first renderer will be shared with OpenGL.
/// Although a fallback GPU renderer will be created if a failure occurs, it doesn't really
/// make sense since the concept of devices only applies to OpenCL renderers.
/// </summary>
/// <param name="renderType">Type of renderer to create</param>
/// <param name="devices">The vector of platform/device indices to use</param>
/// <param name="shared">True if shared with OpenGL, else false.</param>
/// <param name="texId">The texture ID of the shared OpenGL texture if shared</param>
/// <param name="errorReport">The error report for holding errors if anything goes wrong</param>
/// <returns>The vector of created renderers if successful, else false.</returns>
template <typename T>
static vector<unique_ptr<Renderer<T, float>>> CreateRenderers(eRendererType renderType, const vector<pair<size_t, size_t>>& devices, bool shared, GLuint texId, EmberReport& errorReport)
{
	string s;
	vector<unique_ptr<Renderer<T, float>>> v;

	try
	{
		if (renderType == eRendererType::OPENCL_RENDERER && !devices.empty())
		{
			s = "OpenCL";
			v.reserve(devices.size());

			for (size_t i = 0; i < devices.size(); i++)
			{
				vector<pair<size_t, size_t>> tempDevices{ devices[i] };
				auto renderer = unique_ptr<Renderer<T, float>>(new RendererCL<T, float>(tempDevices, !i ? shared : false, texId));//Can't use make_unique here.

				if (!renderer.get() || !renderer->Ok())
				{
					ostringstream os;

					if (renderer.get())
						errorReport.AddToReport(renderer->ErrorReport());

					os << "Error initializing OpenCL renderer for platform " << devices[i].first << ", " << devices[i].second;
					errorReport.AddToReport(os.str());
				}
				else
					v.push_back(std::move(renderer));
			}
		}
		else
		{
			s = "CPU";
			v.push_back(std::move(unique_ptr<Renderer<T, float>>(EmberCommon::CreateRenderer<T>(eRendererType::CPU_RENDERER, devices, shared, texId, errorReport))));
		}
	}
	catch (const std::exception& e)
	{
		errorReport.AddToReport("Error creating " + s + " renderer: " + e.what() + "\n");
	}
	catch (...)
	{
		errorReport.AddToReport("Error creating " + s + " renderer.\n");
	}

	if (v.empty() && s != "CPU")//OpenCL creation failed and CPU creation has not been attempted, so just create one CPU renderer and place it in the vector.
	{
		try
		{
			s = "CPU";
			v.push_back(std::move(unique_ptr<Renderer<T, float>>(EmberCommon::CreateRenderer<T>(eRendererType::CPU_RENDERER, devices, shared, texId, errorReport))));
		}
		catch (const std::exception& e)
		{
			errorReport.AddToReport("Error creating fallback" + s + " renderer: " + e.what() + "\n");
		}
		catch (...)
		{
			errorReport.AddToReport("Error creating fallback " + s + " renderer.\n");
		}
	}

	return v;
}

/// <summary>
/// Perform a render which allows for using strips or not.
/// If an error occurs while rendering any strip, the rendering process stops.
/// Note this must be called after SetEmber(ember, eProcessAction::FULL_RENDER, true) is called on the renderer.
/// The last parameter to SetEmber must be true to compute the camera, because is caches certain values that need to be
/// retained between strips.
/// </summary>
/// <param name="renderer">The renderer to use</param>
/// <param name="ember">The ember to render</param>
/// <param name="finalImage">The vector to place the final output in</param>
/// <param name="time">The time position to use, only valid for animation</param>
/// <param name="strips">The number of strips to use. This must be validated before calling this function.</param>
/// <param name="yAxisUp">True to flip the Y axis, else false.</param>
/// <param name="perStripStart">Function called before the start of the rendering of each strip</param>
/// <param name="perStripFinish">Function called after the end of the rendering of each strip</param>
/// <param name="perStripError">Function called if there is an error rendering a strip</param>
/// <param name="allStripsFinished">Function called when all strips successfully finish rendering</param>
/// <returns>True if all rendering was successful, else false.</returns>
template <typename T>
static bool StripsRender(RendererBase* renderer, Ember<T>& ember, vector<v4F>& finalImage, double time, size_t strips, bool yAxisUp,
						 std::function<void(size_t strip)> perStripStart,
						 std::function<void(size_t strip)> perStripFinish,
						 std::function<void(size_t strip)> perStripError,
						 std::function<void(Ember<T>& finalEmber)> allStripsFinished)
{
	bool success = false;
	size_t origHeight, realHeight = ember.m_FinalRasH;
	T centerY = ember.m_CenterY;
	T floatStripH = T(ember.m_FinalRasH) / T(strips);
	T zoomScale = pow(T(2), ember.m_Zoom);
	T centerBase = centerY - ((strips - 1) * floatStripH) / (2 * ember.m_PixelsPerUnit * zoomScale);
	vector<QTIsaac<ISAAC_SIZE, ISAAC_INT>> randVec;
	ember.m_Quality *= strips;
	ember.m_FinalRasH = size_t(ceil(floatStripH));
	Memset(finalImage);

	if (strips > 1)
		randVec = renderer->RandVec();

	for (size_t strip = 0; strip < strips; strip++)
	{
		size_t stripOffset;

		if (yAxisUp)
			stripOffset = ember.m_FinalRasH * ((strips - strip) - 1) * ember.m_FinalRasW;
		else
			stripOffset = ember.m_FinalRasH * strip * ember.m_FinalRasW;

		ember.m_CenterY = centerBase + ember.m_FinalRasH * T(strip) / (ember.m_PixelsPerUnit * zoomScale);

		if ((ember.m_FinalRasH * (strip + 1)) > realHeight)
		{
			origHeight = ember.m_FinalRasH;
			ember.m_FinalRasH = realHeight - origHeight * strip;
			ember.m_CenterY -= (origHeight - ember.m_FinalRasH) * T(0.5) / (ember.m_PixelsPerUnit * zoomScale);
		}

		perStripStart(strip);

		if (strips > 1)
		{
			renderer->RandVec(randVec);//Use the same vector of ISAAC rands for each strip.
			renderer->SetEmber(ember);//Set one final time after modifications for strips.
		}

		if ((renderer->Run(finalImage, time, 0, false, stripOffset) == eRenderStatus::RENDER_OK) && !renderer->Aborted() && !finalImage.empty())
		{
			perStripFinish(strip);
		}
		else
		{
			perStripError(strip);
			break;
		}

		if (strip == strips - 1)
			success = true;
	}

	//Restore the ember values to their original values.
	ember.m_Quality /= strips;
	ember.m_FinalRasH = realHeight;
	ember.m_CenterY = centerY;

	if (strips > 1)
		renderer->SetEmber(ember);//Further processing will require the dimensions to match the original ember, so re-assign.

	if (success)
		allStripsFinished(ember);

	return success;
}

/// <summary>
/// Verify that the specified number of strips is valid for the given height.
/// The passed in error functions will be called if the number of strips needs
/// to be modified for the given height.
/// </summary>
/// <param name="height">The height in pixels of the image to be rendered</param>
/// <param name="strips">The number of strips to split the render into</param>
/// <param name="stripError1">Function called if the number of strips exceeds the height of the image</param>
/// <param name="stripError2">Function called if the number of strips does not divide evently into the height of the image</param>
/// <param name="stripError3">Called if for any reason the number of strips used will differ from the value passed in</param>
/// <returns>The actual number of strips that will be used</returns>
static size_t VerifyStrips(size_t height, size_t strips,
						   std::function<void(const string& s)> stripError1,
						   std::function<void(const string& s)> stripError2,
						   std::function<void(const string& s)> stripError3)
{
	ostringstream os;

	if (strips > height)
	{
		os << "Cannot have more strips than rows: " << strips << " > " << height << ". Setting strips = rows.";
		stripError1(os.str()); os.str("");
		strips = height;
	}

	if (height % strips != 0)
	{
		os << "A strips value of " << strips << " does not divide evenly into a height of " << height << ".";
		stripError2(os.str()); os.str("");
		strips = NextHighestEvenDiv(height, strips);

		if (strips == 1)//No higher divisor, check for a lower one.
			strips = NextLowestEvenDiv(height, strips);

		os << "Setting strips to " << strips << ".";
		stripError3(os.str()); os.str("");
	}

	return strips;
}

/// <summary>
/// Search the variation's OpenCL string to determine whether it contains any of the search strings in stringVec.
/// This is useful for finding variations with certain characteristics since it's not possible
/// to query the CPU C++ code at runtime.
/// </summary>
/// <param name="var">The variation whose OpenCL string will be searched</param>
/// <param name="stringVec">The vector of strings to search for</param>
/// <param name="matchAll">True to find all variations which match any strings, false to break after the first match is found.</param>
/// <returns>True if there was at least one match, else false.</returns>
template <typename T>
bool SearchVar(const Variation<T>* var, const vector<string>& stringVec, bool matchAll)
{
	bool ret = false;
	size_t i;
	auto cl = var->OpenCLFuncsString() + "\n" + var->OpenCLString();

	if (matchAll)
	{
		for (i = 0; i < stringVec.size(); i++)
			if (cl.find(stringVec[i]) == std::string::npos)
				break;

		ret = (i == stringVec.size());
	}
	else
	{
		for (i = 0; i < stringVec.size(); i++)
		{
			if (cl.find(stringVec[i]) != std::string::npos)
			{
				ret = true;
				break;
			}
		}
	}

	return ret;
}

template <typename T>
bool SearchVarWWO(const Variation<T>* var, const vector<string>& withVec, const vector<string>& withoutVec)
{
	bool ret = false;
	size_t i, j, k;
	bool onegood = false;
	auto cl = var->OpenCLFuncsString() + "\n" + var->OpenCLString();
	vector<string> clsplits = Split(cl, '\n');

	for (i = 0; i < clsplits.size(); i++)
	{
		for (j = 0; j < withVec.size(); j++)
		{
			if (clsplits[i].find(withVec[j]) != std::string::npos)
			{
				for (k = 0; k < withoutVec.size(); k++)
				{
					if (clsplits[i].find(withoutVec[k]) != std::string::npos)
					{
						return false;
					}
				}

				onegood = true;
			}
		}
	}

	return onegood;
	//return i == clsplits.size() && j == withVec.size() && k == withoutVec.size();
}

/// <summary>
/// Find all variations whose OpenCL string contains any of the search strings in stringVec.
/// This is useful for finding variations with certain characteristics since it's not possible
/// to query the CPU C++ code at runtime.
/// </summary>
/// <param name="stringVec">The vector of variation pointers to search</param>
/// <param name="stringVec">The vector of strings to search for</param>
/// <param name="findAll">True to find all variations which match any strings, false to break after the first match is found.</param>
/// <param name="matchAll">True to find all variations which match all strings, false to stop searching a variation after the first match succeeds.</param>
/// <returns>A vector of pointers to variations whose OpenCL string matched at least one string in stringVec</returns>
template <typename T>
static vector<const Variation<T>*> FindVarsWith(const vector<const Variation<T>*>& vars, const vector<string>& stringVec, bool findAll = true, bool matchAll = false)
{
	vector<const Variation<T>*> vec;
	auto vl = VariationList<T>::Instance();

	for (auto& v : vars)
	{
		if (SearchVar<T>(v, stringVec, matchAll))
		{
			vec.push_back(v);

			if (!findAll)
				break;
		}
	}

	return vec;
}

template <typename T>
static vector<const Variation<T>*> FindVarsWithWithout(const vector<const Variation<T>*>& vars, const vector<string>& withVec, const vector<string>& withoutVec)
{
	vector<const Variation<T>*> vec;
	auto vl = VariationList<T>::Instance();

	for (auto& v : vars)
	{
		if (SearchVarWWO<T>(v, withVec, withoutVec))
		{
			vec.push_back(v);
		}
	}

	return vec;
}

/// <summary>
/// Find all variations whose OpenCL string does not contain any of the search strings in stringVec.
/// This is useful for finding variations without certain characteristics since it's not possible
/// to query the CPU C++ code at runtime.
/// </summary>
/// <param name="vars">The vector of variation pointers to search</param>
/// <param name="stringVec">The vector of strings to search for</param>
/// <param name="findAll">True to find all variations which don't match any strings, false to break after the first non-match is found.</param>
/// <returns>A vector of pointers to variations whose OpenCL string did not match any string in stringVec</returns>
template <typename T>
static vector<const Variation<T>*> FindVarsWithout(const vector<const Variation<T>*>& vars, const vector<string>& stringVec, bool findAll = true)
{
	vector<const Variation<T>*> vec;
	auto vl = VariationList<T>::Instance();

	for (auto& v : vars)
	{
		if (!SearchVar<T>(v, stringVec, false))
		{
			vec.push_back(v);

			if (!findAll)
				break;
		}
	}

	return vec;
}

/// <summary>
/// Check whether a file exists, and optionally if it's not empty.
/// </summary>
/// <param name="filename">The full path and file name to check for</param>
/// <param name="notempty">Whether to only return true if the file is found and is not empty. Default: true.</param>
/// <returns>True if the file was found and optionally not empty, else false.</returns>
static bool FileExists(const string& filename, bool notempty = true)
{
	try
	{
		ifstream ifs;
		ifs.exceptions(ifstream::failbit);
		ifs.open(filename, ios::binary | ios::ate);

		if (notempty)
			return ifs.tellg() > 0;//Ensure it exists and wasn't empty.
		else
			return true;
	}
	catch (...)
	{
	}

	return false;
}

class ThreadedWriter
{
public:
	ThreadedWriter(int _size)
	{
		m_Size = _size;
		m_WriteThreads.resize(m_Size);
		m_FinalImages.resize(m_Size);
	}

	size_t Current()
	{
		return m_CurrentIndex % m_Size;
	}

	size_t Increment()
	{
		auto ret = ++m_CurrentIndex % m_Size;
		Join(m_WriteThreads[ret]);
		return ret;
	}

	vector<v4F>* GetImage(size_t i)
	{
		return &m_FinalImages[i];
	}

	void SetThread(size_t i, std::thread& th)
	{
		Join(m_WriteThreads[i]);
		m_WriteThreads[i] = std::move(th);
	}

	void JoinAll()
	{
		Join(m_WriteThreads);
	}

private:
	size_t m_Size = 0;
	size_t m_CurrentIndex = 0;
	std::vector<std::thread> m_WriteThreads;
	std::vector<vector<v4F>> m_FinalImages;
};
}

/// <summary>
/// Simple macro to print a string if the --verbose options has been specified.
/// </summary>
#define VerbosePrint(s) if (opt.Verbose()) cout << s << "\n"
