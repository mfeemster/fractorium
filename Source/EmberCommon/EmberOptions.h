#pragma once

#include "EmberCommon.h"
using namespace SimpleOpt;

/// <summary>
/// EmberOptionEntry and EmberOptions classes.
/// </summary>

static const char* DescriptionString = "Ember - Fractal flames C++ port and enhancement with OpenCL GPU support";

/// <summary>
/// Enum for specifying which command line programs an option is meant to be used with.
/// If an option is used with multiple programs, their values are ORed together.
/// </summary>
enum class eOptionUse : et
{
	OPT_USE_RENDER  = 1,
	OPT_USE_ANIMATE = 1 << 1,
	OPT_USE_GENOME  = 1 << 2,
	OPT_RENDER_ANIM = et(eOptionUse::OPT_USE_RENDER)  | et(eOptionUse::OPT_USE_ANIMATE),
	OPT_ANIM_GENOME = et(eOptionUse::OPT_USE_ANIMATE) | et(eOptionUse::OPT_USE_GENOME),
	OPT_USE_ALL     = et(eOptionUse::OPT_USE_RENDER)  | et(eOptionUse::OPT_USE_ANIMATE) | et(eOptionUse::OPT_USE_GENOME)
};

/// <summary>
/// Unique identifiers for every available option across all programs.
/// </summary>
enum class eOptionIDs : et
{
	//Diagnostic args.
	OPT_HELP,
	OPT_VERSION,
	OPT_VERBOSE,
	OPT_DEBUG,
	OPT_DUMP_ARGS,
	OPT_PROGRESS,
	OPT_DUMP_OPENCL_INFO,
	OPT_ALL_VARS,
	OPT_REG_VARS,
	OPT_PRE_VARS,
	OPT_POST_VARS,
	OPT_SUM_VARS,
	OPT_ASSIGN_VARS,
	OPT_PPSUM_VARS,
	OPT_PPASSIGN_VARS,
	OPT_DC_VARS,
	OPT_STATE_VARS,
	OPT_PAR_VARS,
	OPT_NON_PAR_VARS,

	//Boolean args.
	OPT_OPENCL,
	OPT_SP,
	OPT_EARLYCLIP,
	OPT_POS_Y_UP,
	OPT_TRANSPARENCY,
	OPT_NAME_ENABLE,
    OPT_HEX_PALETTE,
    OPT_INSERT_PALETTE,
	OPT_ENABLE_COMMENTS,
	OPT_WRITE_GENOME,
	OPT_THREADED_WRITE,
	OPT_ENCLOSED,
	OPT_NO_EDITS,
	OPT_UNSMOOTH_EDGE,
	OPT_CW_LOOPS,
	OPT_CW_INTERP_LOOPS,
	OPT_LOCK_ACCUM,
	OPT_DUMP_KERNEL,
	OPT_FLAM3_COMPAT,
	OPT_IGNORE_EXISTING,

	//Value args.
	OPT_NTHREADS,//Int value args.
	OPT_STRIPS,
	OPT_SUPERSAMPLE,
	OPT_TEMPSAMPLES,
	OPT_PRINT_EDIT_DEPTH,
	OPT_JPEG,
	OPT_BEGIN,
	OPT_END,
	OPT_FRAME,
	OPT_DTIME,
	OPT_LOOP_FRAMES,
	OPT_INTERP_FRAMES,
	OPT_INTERP_LOOPS,
	OPT_SYMMETRY,
	OPT_SHEEP_GEN,
	OPT_SHEEP_ID,
	OPT_REPEAT,
	OPT_TRIES,
	OPT_MAX_XFORMS,
	OPT_START_COUNT,
	OPT_PADDING,
	OPT_PRIORITY,
	OPT_W,
	OPT_H,

	OPT_SS,//Float value args.
	OPT_WS,
	OPT_HS,
	OPT_QS,
	OPT_QUALITY,
	OPT_DE_MIN,
	OPT_DE_MAX,
	OPT_SBPCTTH,
	OPT_PIXEL_ASPECT,
	OPT_STAGGER,
	OPT_AVG_THRESH,
	OPT_BLACK_THRESH,
	OPT_WHITE_LIMIT,
	OPT_SPEED,
	OPT_OFFSETX,
	OPT_OFFSETY,
	OPT_USEMEM,
	OPT_LOOPS,

	OPT_SCALE_TYPE,//String value args.
	OPT_OPENCL_DEVICE,
	OPT_ISAAC_SEED,
	OPT_IN,
	OPT_OUT,
	OPT_PREFIX,
	OPT_SUFFIX,
	OPT_FORMAT,
	OPT_PALETTE_FILE,
	OPT_ID,
	OPT_URL,
	OPT_NICK,
	OPT_COMMENT,
	OPT_TEMPLATE,
	OPT_CLONE,
	OPT_CLONE_ALL,
	OPT_CLONE_ACTION,
	OPT_ANIMATE,
	OPT_MUTATE,
	OPT_CROSS0,
	OPT_CROSS1,
	OPT_METHOD,
	OPT_INTER,
	OPT_ROTATE,
	OPT_SEQUENCE,
	OPT_USE_VARS,
	OPT_DONT_USE_VARS,
	OPT_EXTRAS
};

class EmberOptions;

/// <summary>
/// A single option.
/// Template argument expected to be bool, int, uint, double or string.
/// </summary>
template <typename T>
class EmberOptionEntry
{
	friend class EmberOptions;

private:
	/// <summary>
	/// Default constructor. This should never be used, instead use the one that takes arguments.
	/// </summary>
	EmberOptionEntry() noexcept
	{
		m_Option.nId = 0;
		m_Option.pszArg = _T("--fillmein");
		m_Option.nArgType = SO_NONE;
		m_Val = T();
	}

public:
	/// <summary>
	/// Constructor that takes arguments.
	/// </summary>
	/// <param name="optUsage">The specified program usage</param>
	/// <param name="optId">The option identifier enum</param>
	/// <param name="arg">The command line argument (--arg)</param>
	/// <param name="defaultVal">The default value to use the option was not given on the command line</param>
	/// <param name="argType">The format the argument should be given in</param>
	/// <param name="docString">The documentation string describing what the argument means</param>
	EmberOptionEntry(eOptionUse optUsage, eOptionIDs optId, const CharT* arg, T defaultVal, ESOArgType argType, const string& docString)
	{
		m_OptionUse = optUsage;
		m_Option.nId = static_cast<int>(optId);
		m_Option.pszArg = arg;
		m_Option.nArgType = argType;
		m_DocString = docString;
		m_NameWithoutDashes = Trim(string(arg), '-');
		m_Val = Arg<T>(const_cast<char*>(m_NameWithoutDashes.c_str()), defaultVal);
	}

	/// <summary>
	/// Copy constructor.
	/// </summary>
	/// <param name="entry">The EmberOptionEntry object to copy</param>
	EmberOptionEntry(const EmberOptionEntry& entry)
	{
		*this = entry;
	}

	/// <summary>
	/// Default assignment operator.
	/// </summary>
	/// <param name="entry">The EmberOptionEntry object to copy</param>
	EmberOptionEntry<T>& operator = (const EmberOptionEntry<T>& entry)
	{
		if (this != &entry)
		{
			m_OptionUse = entry.m_OptionUse;
			m_Option = entry.m_Option;
			m_DocString = entry.m_DocString;
			m_NameWithoutDashes = entry.m_NameWithoutDashes;
			m_Val = entry.m_Val;
		}

		return *this;
	}

	/// <summary>
	/// Functor accessors.
	/// </summary>
	inline const T& operator() (void) const noexcept { return m_Val; }
	inline void operator() (T t) noexcept { m_Val = t; }

private:
	eOptionUse m_OptionUse = eOptionUse::OPT_USE_ALL;
	CSimpleOpt::SOption m_Option;
	string m_DocString = "Dummy doc";
	string m_NameWithoutDashes;
	T m_Val;
};


/// <summary>
/// Class to force a stringstream to not split on space and
/// read to the end of the string.
/// </summary>
struct NoDelimiters : std::ctype<char>
{
	/// <summary>
	/// Constructor that passes the table created in GetTable() to the base.
	/// </summary>
	NoDelimiters() noexcept
		: std::ctype<char>(GetTable())
	{
	}

	/// <summary>
	/// Create and return a pointer to an empty table with no delimiters.
	/// </summary>
	/// <returns>A pointer to the empty delimiter table</returns>
	static std::ctype_base::mask const* GetTable() noexcept
	{
		typedef std::ctype<char> cctype;
		static cctype::mask rc[cctype::table_size];
		std::memset(rc, 0, cctype::table_size * sizeof(cctype::mask));
		return &rc[0];
	}
};

/// <summary>
/// Macros for setting up and parsing various option types.
/// </summary>

//Bool.
#define Eob EmberOptionEntry<bool>
#define INITBOOLOPTION(member, option) \
	member = option; \
	m_BoolArgs.push_back(&member)

#define PARSEBOOLOPTION(e, member) \
	case (e): \
	{ \
		if (member.m_Option.nArgType == SO_OPT) \
		{ \
			member(!_stricmp(args.OptionArg(), "true")); \
		} \
		else \
		{ \
			member(true); \
		} \
	} \
	break

//Parsing is the same for all numerical option types.
#define PARSEOPTION(e, member) \
	case (e): \
	{ \
		ss.clear(); \
		ss.str(args.OptionArg()); \
		ss >> member.m_Val; \
		break; \
	}

//Int.
#define Eoi EmberOptionEntry<intmax_t>
#define INITINTOPTION(member, option) \
	member = option; \
	m_IntArgs.push_back(&member)

//Uint.
#define Eou EmberOptionEntry<size_t>
#define INITUINTOPTION(member, option) \
	member = option; \
	m_UintArgs.push_back(&member)

//Double.
#define Eod EmberOptionEntry<double>
#define INITDOUBLEOPTION(member, option) \
	member = option; \
	m_DoubleArgs.push_back(&member)

//String.
#define Eos EmberOptionEntry<string>
#define INITSTRINGOPTION(member, option) \
	member = option; \
	m_StringArgs.push_back(&member)

/// <summary>
/// Class for holding all available options across all command line programs.
/// Some are used only for a single program, while others are used for more than one.
/// This prevents having to keep separate documentation strings around for different programs.
/// </summary>
class EmberOptions
{
public:
	/// <summary>
	/// Constructor that populates all available options.
	/// </summary>
	EmberOptions()
	{
        constexpr size_t size = static_cast<size_t>(eOptionIDs::OPT_EXTRAS);
		m_BoolArgs.reserve(size);
		m_IntArgs.reserve(size);
		m_UintArgs.reserve(size);
		m_DoubleArgs.reserve(size);
		m_StringArgs.reserve(size);
		//Informational bools.
		INITBOOLOPTION(Help,           Eob(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_HELP,             _T("--help"),                 false,                SO_NONE,     "   --help                    Show this screen and exit.\n"));
		INITBOOLOPTION(Version,        Eob(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_VERSION,          _T("--version"),              false,                SO_NONE,     "   --version                 Show version and exit.\n"));
		INITBOOLOPTION(OpenCLInfo,     Eob(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_DUMP_OPENCL_INFO, _T("--openclinfo"),           false,                SO_NONE,     "   --openclinfo              Display platforms and devices for OpenCL and exit.\n"));
		INITBOOLOPTION(AllVars,        Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_ALL_VARS,         _T("--allvars"),              false,                SO_NONE,     "   --allvars                 Display the names of all supported variations and exit.\n"));
		INITBOOLOPTION(RegVars,        Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_REG_VARS,         _T("--regvars"),              false,                SO_NONE,     "   --regvars                 Display the names of all supported regular variations and exit.\n"));
		INITBOOLOPTION(PreVars,        Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_PRE_VARS,         _T("--prevars"),              false,                SO_NONE,     "   --prevars                 Display the names of all supported pre variations and exit.\n"));
		INITBOOLOPTION(PostVars,       Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_POST_VARS,        _T("--postvars"),             false,                SO_NONE,     "   --postvars                Display the names of all supported post variations and exit.\n"));
		INITBOOLOPTION(SumVars,        Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_SUM_VARS,         _T("--sumvars"),              false,                SO_NONE,     "   --sumvars                 Display the names of all regular variations which have the standard behavior of summing their output, and exit.\n"));
		INITBOOLOPTION(AssignVars,     Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_ASSIGN_VARS,      _T("--assignvars"),           false,                SO_NONE,     "   --assignvars              Display the names of all regular variations which have the non-standard behavior of assigning their output, and exit.\n"));
		INITBOOLOPTION(PpSumVars,      Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_PPSUM_VARS,       _T("--ppsumvars"),            false,                SO_NONE,     "   --ppsumvars               Display the names of all pre/post variations which have the non-standard behavior of summing their output, and exit.\n"));
		INITBOOLOPTION(PpAssignVars,   Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_PPASSIGN_VARS,    _T("--ppassignvars"),         false,                SO_NONE,     "   --ppassignvars            Display the names of all pre/post variations which have the standard behavior of assigning their output, and exit.\n"));
		INITBOOLOPTION(DcVars,         Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_DC_VARS,          _T("--dcvars"),               false,                SO_NONE,     "   --dcvars                  Display the names of all variations which alter the color index and exit.\n"));
		INITBOOLOPTION(StateVars,      Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_STATE_VARS,       _T("--statevars"),            false,                SO_NONE,     "   --statevars               Display the names of all variations which alter their state on each iteration and exit.\n"));
		INITBOOLOPTION(ParVars,        Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_PAR_VARS,         _T("--parvars"),              false,                SO_NONE,     "   --parvars                 Display the names of all variations which have parameters and exit.\n"));
		INITBOOLOPTION(NonParVars,     Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_NON_PAR_VARS,     _T("--nonparvars"),           false,                SO_NONE,     "   --nonparvars              Display the names of all variations which do not have parameters (weight only) and exit.\n"));
		//Diagnostic bools.
		INITBOOLOPTION(Verbose,        Eob(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_VERBOSE,          _T("--verbose"),              false,                SO_NONE,     "   --verbose                 Verbose output [default: false].\n"));
		INITBOOLOPTION(Debug,          Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_DEBUG,            _T("--debug"),                false,                SO_NONE,     "   --debug                   Debug output [default: false].\n"));
		INITBOOLOPTION(DumpArgs,       Eob(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_DUMP_ARGS,        _T("--dumpargs"),             false,                SO_NONE,     "   --dumpargs                Print all arguments entered from either the command line or environment variables [default: false].\n"));
		INITBOOLOPTION(DoProgress,     Eob(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_PROGRESS,         _T("--progress"),             false,                SO_NONE,     "   --progress                Display progress. This will slow down processing by about 10% [default: false].\n"));
		//Execution bools.
		INITBOOLOPTION(EmberCL,        Eob(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_OPENCL,           _T("--opencl"),               false,                SO_NONE,     "   --opencl                  Use OpenCL renderer (EmberCL) for rendering [default: false].\n"));
		INITBOOLOPTION(Sp,             Eob(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_SP,               _T("--sp"),                   false,                SO_NONE,     "   --sp                      Use single precision for rendering instead of double precision [default: false].\n"));
		INITBOOLOPTION(EarlyClip,      Eob(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_EARLYCLIP,        _T("--earlyclip"),            false,                SO_NONE,     "   --earlyclip               Perform clipping of RGB values before spatial filtering for better antialiasing and resizing [default: false].\n"));
		INITBOOLOPTION(YAxisUp,        Eob(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_POS_Y_UP,         _T("--yaxisup"),              false,                SO_NONE,     "   --yaxisup                 Orient the image with the positive y axis pointing up [default: false].\n"));
		INITBOOLOPTION(Transparency,   Eob(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_TRANSPARENCY,     _T("--transparency"),         false,                SO_NONE,     "   --transparency            Include alpha channel in final output [default: false except for PNG].\n"));
		INITBOOLOPTION(NameEnable,     Eob(eOptionUse::OPT_USE_RENDER,  eOptionIDs::OPT_NAME_ENABLE,      _T("--name_enable"),          false,                SO_NONE,     "   --name_enable             Use the name attribute contained in the Xml as the output filename [default: false].\n"));
		INITBOOLOPTION(HexPalette,     Eob(eOptionUse::OPT_ANIM_GENOME, eOptionIDs::OPT_HEX_PALETTE,      _T("--hex_palette"),          true,                 SO_OPT,      "   --hex_palette             Force palette RGB values to be hex when saving to Xml [default: true].\n"));
		INITBOOLOPTION(InsertPalette,  Eob(eOptionUse::OPT_USE_RENDER,  eOptionIDs::OPT_INSERT_PALETTE,   _T("--insert_palette"),       false,                SO_NONE,     "   --insert_palette          Insert the palette into the image for debugging purposes. Disabled when running with OpenCL [default: false].\n"));
		INITBOOLOPTION(EnableComments, Eob(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_ENABLE_COMMENTS,  _T("--enable_comments"),      false,				  SO_NONE,	   "   --enable_comments         Enables embedding the flame parameters and user identifying information in the header of a jpg, png or exr [default: false].\n"));
		INITBOOLOPTION(WriteGenome,    Eob(eOptionUse::OPT_USE_ANIMATE, eOptionIDs::OPT_WRITE_GENOME,     _T("--write_genome"),         false,                SO_NONE,     "   --write_genome            Write out flame associated with center of motion blur window [default: false].\n"));
		INITBOOLOPTION(ThreadedWrite,  Eob(eOptionUse::OPT_USE_ANIMATE, eOptionIDs::OPT_THREADED_WRITE,   _T("--threaded_write"),       true,                 SO_OPT,      "   --threaded_write          Use a separate thread to write images to disk. This gives better performance, but doubles the memory required for the final output buffer. [default: true].\n"));
		INITBOOLOPTION(Enclosed,	   Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_ENCLOSED,		  _T("--enclosed"),				true,				  SO_OPT,	   "   --enclosed                Use enclosing Xml tags [default: true].\n"));
		INITBOOLOPTION(NoEdits,        Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_NO_EDITS,         _T("--noedits"),              false,                SO_NONE,     "   --noedits                 Exclude edit tags when writing Xml [default: false].\n"));
		INITBOOLOPTION(UnsmoothEdge,   Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_UNSMOOTH_EDGE,    _T("--unsmoother"),           false,                SO_NONE,     "   --unsmoother              Do not use smooth blending for sheep edges [default: false].\n"));
		INITBOOLOPTION(CwLoops,        Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_CW_LOOPS,         _T("--cwloops"),              false,                SO_NONE,     "   --cwloops                 Rotate loops clockwise [default: false].\n"));
		INITBOOLOPTION(CwInterpLoops,  Eob(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_CW_INTERP_LOOPS,  _T("--cwinterploops"),        false,                SO_NONE,     "   --cwinterploops           Rotate clockwise during interpolation, ignored if --interploops is 0 [default: false].\n"));
		INITBOOLOPTION(LockAccum,	   Eob(eOptionUse::OPT_USE_ALL,		eOptionIDs::OPT_LOCK_ACCUM,       _T("--lock_accum"),           false,                SO_NONE,     "   --lock_accum              Lock threads when accumulating to the histogram using the CPU. This will drop performance to that of single threading [default: false].\n"));
		INITBOOLOPTION(DumpKernel,	   Eob(eOptionUse::OPT_USE_RENDER,	eOptionIDs::OPT_DUMP_KERNEL,      _T("--dump_kernel"),          false,                SO_NONE,     "   --dump_kernel             Print the iteration kernel string when using OpenCL (ignored for CPU) [default: false].\n"));
		INITBOOLOPTION(Flam3Compat,	   Eob(eOptionUse::OPT_USE_ALL,		eOptionIDs::OPT_FLAM3_COMPAT,     _T("--flam3_compat"),         false,                SO_NONE,     "   --flam3_compat            The behavior of the cos, cosh, cot, coth, csc, csch, sec, sech, sin, sinh, tan and tanh variations are different in flam3/Apophysis versus Chaotica. True for flam3/Apophysis behavior, false for Chaotica behavior [default: true].\n"));
		INITBOOLOPTION(IgnoreExisting, Eob(eOptionUse::OPT_USE_ANIMATE,	eOptionIDs::OPT_IGNORE_EXISTING,  _T("--ignore-existing"),      false,                SO_NONE,     "   --ignore-existing         Skip animating a frame if the output images for all of the specified file output types already exist in the output folder [default: false].\n"));
		//Int.
		INITINTOPTION(Symmetry,        Eoi(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_SYMMETRY,         _T("--symmetry"),					0,			   SO_REQ_SEP,	   "   --symmetry=<val>          Set symmetry of result [default: 0].\n"));
		INITINTOPTION(SheepGen,        Eoi(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_SHEEP_GEN,        _T("--sheep_gen"),	           -1,			   SO_REQ_SEP,	   "   --sheep_gen=<val>         Sheep generation of this flame [default: -1].\n"));
		INITINTOPTION(SheepId,         Eoi(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_SHEEP_ID,         _T("--sheep_id"),				   -1,			   SO_REQ_SEP,	   "   --sheep_id=<val>          Sheep ID of this flame [default: -1].\n"));
#ifdef _WIN32
		INITINTOPTION(Priority,		   Eoi(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_PRIORITY,		  _T("--priority"), int(eThreadPriority::NORMAL),  SO_REQ_SEP,	   "   --priority=<val>          The priority of the CPU rendering threads from -2 - 2. This does not apply to OpenCL rendering.\n"));
#else
		INITINTOPTION(Priority,		   Eoi(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_PRIORITY,		  _T("--priority"),	int(eThreadPriority::NORMAL),  SO_REQ_SEP,	   "   --priority=<val>          The priority of the CPU rendering threads, 1, 25, 50, 75, 99. This does not apply to OpenCL rendering.\n"));
#endif
		//Uint.
		INITUINTOPTION(ThreadCount,     Eou(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_NTHREADS,         _T("--nthreads"),             0,                    SO_REQ_SEP, "   --nthreads=<val>          The number of threads to use [default: use all available cores].\n"));
		INITUINTOPTION(Strips,		    Eou(eOptionUse::OPT_USE_RENDER,  eOptionIDs::OPT_STRIPS,           _T("--nstrips"),              1,                    SO_REQ_SEP, "   --nstrips=<val>           The number of fractions to split a single render frame into. Useful for print size renders or low memory systems [default: 1].\n"));
		INITUINTOPTION(Supersample,     Eou(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_SUPERSAMPLE,      _T("--supersample"),          0,                    SO_REQ_SEP, "   --supersample=<val>       The supersample value used to override the one specified in the file [default: 0 (use value from file), Range: 0 - 4].\n"));
		INITUINTOPTION(TemporalSamples, Eou(eOptionUse::OPT_USE_ANIMATE, eOptionIDs::OPT_TEMPSAMPLES,      _T("--ts"),                   0,                    SO_REQ_SEP, "   --ts=<val>                The temporal samples value used to override all of the temporal sample values specified in the file when animating [default: 0 (use value from file)].\n"));
		INITUINTOPTION(PrintEditDepth,  Eou(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_PRINT_EDIT_DEPTH, _T("--print_edit_depth"),     0,                    SO_REQ_SEP, "   --print_edit_depth=<val>  Depth to truncate <edit> tag structure when converting a flame to Xml. 0 prints all <edit> tags [default: 0].\n"));
		INITUINTOPTION(JpegQuality,     Eou(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_JPEG,             _T("--jpeg"),                 95,                   SO_REQ_SEP, "   --jpeg=<val>              Jpeg quality 0-100 for compression [default: 95].\n"));
		INITUINTOPTION(FirstFrame,      Eou(eOptionUse::OPT_USE_ANIMATE, eOptionIDs::OPT_BEGIN,            _T("--begin"),                UINT_MAX,             SO_REQ_SEP, "   --begin=<val>             Time of first frame to render [default: first time specified in file].\n"));
		INITUINTOPTION(LastFrame,       Eou(eOptionUse::OPT_USE_ANIMATE, eOptionIDs::OPT_END,              _T("--end"),	                 UINT_MAX,             SO_REQ_SEP, "   --end=<val>               Time of last frame to render [default: last time specified in the input file].\n"));
		INITUINTOPTION(Frame,           Eou(eOptionUse::OPT_ANIM_GENOME, eOptionIDs::OPT_FRAME,            _T("--frame"),                UINT_MAX,             SO_REQ_SEP, "   --frame=<val>             Time of first and last frame (ie do one frame).\n"));
		INITUINTOPTION(Dtime,           Eou(eOptionUse::OPT_USE_ANIMATE, eOptionIDs::OPT_DTIME,            _T("--dtime"),                1,                    SO_REQ_SEP, "   --dtime=<val>             Time between frames [default: 1].\n"));
		INITUINTOPTION(LoopFrames,      Eou(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_LOOP_FRAMES,      _T("--loopframes"),           20,                   SO_REQ_SEP, "   --loopframes=<val>        Number of frames per loop in the animation [default: 20].\n"));
		INITUINTOPTION(InterpFrames,    Eou(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_INTERP_FRAMES,    _T("--interpframes"),         20,                   SO_REQ_SEP, "   --interpframes=<val>      Number of frames per interpolation in the animation [default: 20].\n"));
		INITUINTOPTION(InterpLoops,     Eou(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_INTERP_LOOPS,     _T("--interploops"),          1,                    SO_REQ_SEP, "   --interploops=<val>       The number of 360 degree loops to rotate when interpolating between keyframes [default: 1].\n"));
		INITUINTOPTION(Repeat,          Eou(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_REPEAT,           _T("--repeat"),               1,                    SO_REQ_SEP, "   --repeat=<val>            Number of new flames to create. Ignored if sequence, inter or rotate were specified [default: 1].\n"));
		INITUINTOPTION(Tries,           Eou(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_TRIES,            _T("--tries"),                10,                   SO_REQ_SEP, "   --tries=<val>             Number times to try creating a flame that meets the specified constraints. Ignored if sequence, inter or rotate were specified [default: 10].\n"));
		INITUINTOPTION(MaxXforms,       Eou(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_MAX_XFORMS,       _T("--maxxforms"),            UINT_MAX,             SO_REQ_SEP, "   --maxxforms=<val>         The maximum number of xforms allowed in the final output.\n"));
		INITUINTOPTION(StartCount,      Eou(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_START_COUNT,      _T("--startcount"),           0,                    SO_REQ_SEP, "   --startcount=<val>        The number to add to each flame name when generating a sequence. Useful for programs like ffmpeg which require numerically increasing filenames [default: 0].\n"));
		INITUINTOPTION(Padding,         Eou(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_PADDING,          _T("--padding"),              0,                    SO_REQ_SEP, "   --padding=<val>           Override the amount of zero padding added to each flame name when generating a sequence. Useful for programs like ffmpeg which require fixed width filenames [default: 0 (auto calculate padding)].\n"));
		INITUINTOPTION(Width,           Eou(eOptionUse::OPT_RENDER_ANIM,  eOptionIDs::OPT_W,               _T("--width"),                0,                    SO_REQ_SEP, "   --width=<val>             Override the width the flame is rendered at. Ideally used in conjunction with --scaletype [default: 0 (no override)].\n"));
		INITUINTOPTION(Height,          Eou(eOptionUse::OPT_RENDER_ANIM,  eOptionIDs::OPT_H,               _T("--height"),               0,                    SO_REQ_SEP, "   --height=<val>            Override the height the flame is rendered at. Ideally used in conjunction with --scaletype [default: 0 (no override)].\n"));
		//Double.
		INITDOUBLEOPTION(SizeScale,     Eod(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_SS,               _T("--ss"),                   1.0,                  SO_REQ_SEP, "   --ss=<val>                Size scale. All dimensions are scaled by this amount. Ideally used in conjunction with --scaletype [default: 1.0 (no scaling)].\n"));
		INITDOUBLEOPTION(WidthScale,    Eod(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_WS,               _T("--ws"),                   1.0,                  SO_REQ_SEP, "   --ws=<val>                Width scale. The width is scaled by this amount. Ideally used in conjunction with --scaletype [default: 1.0 (no scaling)].\n"));
		INITDOUBLEOPTION(HeightScale,   Eod(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_HS,               _T("--hs"),                   1.0,                  SO_REQ_SEP, "   --hs=<val>                Height scale. The height is scaled by this amount. Ideally used in conjunction with --scaletype [default: 1.0 (no scaling)].\n"));
		INITDOUBLEOPTION(QualityScale,  Eod(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_QS,               _T("--qs"),                   1.0,                  SO_REQ_SEP, "   --qs=<val>                Quality scale. All quality values are scaled by this amount [default: 1.0 (no scaling)].\n"));
		INITDOUBLEOPTION(Quality,	    Eod(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_QUALITY,		   _T("--quality"),				 0.0,                  SO_REQ_SEP, "   --quality=<val>           Override the quality of the flame if not 0 [default: 0].\n"));
		INITDOUBLEOPTION(SBPctPerTh,    Eod(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_SBPCTTH,          _T("--sbpctth"),              0.025,                SO_REQ_SEP, "   --sbpctth=<val>           The percentage of a sub batch from 0.01 to 1.0 to complete per thread per kernel launch done in OpenCL rendering. This is for performance tuning with OpenCL and does not apply to CPU rendering [default: 0.025 (256 iters with the default sub batch size of 10k)].\n"));
		INITDOUBLEOPTION(DeMin,		    Eod(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_DE_MIN,		   _T("--demin"),			    -1.0,                  SO_REQ_SEP, "   --demin=<val>             Override the minimum size of the density estimator filter radius if not -1 [default: -1].\n"));
		INITDOUBLEOPTION(DeMax,		    Eod(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_DE_MAX,		   _T("--demax"),			    -1.0,                  SO_REQ_SEP, "   --demax=<val>             Override the maximum size of the density estimator filter radius if not -1 [default: -1].\n"));
		INITDOUBLEOPTION(AspectRatio,   Eod(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_PIXEL_ASPECT,     _T("--pixel_aspect"),         1.0,                  SO_REQ_SEP, "   --pixel_aspect=<val>      Aspect ratio of pixels (width over height), eg. 0.90909 for NTSC [default: 1.0].\n"));
		INITDOUBLEOPTION(Stagger,       Eod(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_STAGGER,          _T("--stagger"),              0.0,                  SO_REQ_SEP, "   --stagger=<val>           Affects simultaneity of xform interpolation during flame interpolation.\n"
											"\t                         Represents how 'separate' the xforms are interpolated. Set to 1 for each\n"
											"\t                         xform to be interpolated individually, fractions control interpolation overlap [default: 0].\n"));
		INITDOUBLEOPTION(AvgThresh,    Eod(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_AVG_THRESH,       _T("--avg"),                  20.0,                 SO_REQ_SEP,  "   --avg=<val>               Minimum average pixel channel sum (r + g + b) threshold from 0 - 765. Ignored if sequence, inter or rotate were specified [default: 20].\n"));
		INITDOUBLEOPTION(BlackThresh,  Eod(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_BLACK_THRESH,     _T("--black"),                0.01,                 SO_REQ_SEP,  "   --black=<val>             Minimum number of allowed black pixels as a percentage from 0 - 1. Ignored if sequence, inter or rotate were specified [default: 0.01].\n"));
		INITDOUBLEOPTION(WhiteLimit,   Eod(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_WHITE_LIMIT,      _T("--white"),                0.05,                 SO_REQ_SEP,  "   --white=<val>             Maximum number of allowed white pixels as a percentage from 0 - 1. Ignored if sequence, inter or rotate were specified [default: 0.05].\n"));
		INITDOUBLEOPTION(Speed,        Eod(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_SPEED,            _T("--speed"),                0.1,                  SO_REQ_SEP,  "   --speed=<val>             Speed as a percentage from 0 - 1 that the affine transform of an existing flame mutates with the new flame. Ignored if sequence, inter or rotate were specified [default: 0.1].\n"));
		INITDOUBLEOPTION(OffsetX,      Eod(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_OFFSETX,          _T("--offsetx"),              0.0,                  SO_REQ_SEP,  "   --offsetx=<val>           Amount to move each flame horizontally when applying genome tools [default: 0].\n"));
		INITDOUBLEOPTION(OffsetY,      Eod(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_OFFSETY,          _T("--offsety"),              0.0,                  SO_REQ_SEP,  "   --offsety=<val>           Amount to move each flame vertically when applying genome tools [default: 0].\n"));
		INITDOUBLEOPTION(UseMem,       Eod(eOptionUse::OPT_USE_RENDER,  eOptionIDs::OPT_USEMEM,           _T("--use_mem"),              0.0,                  SO_REQ_SEP,  "   --use_mem=<val>           Number of bytes of memory to use [default: max system memory].\n"));
		INITDOUBLEOPTION(Loops,        Eod(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_LOOPS,            _T("--loops"),                1.0,                  SO_REQ_SEP,  "   --loops=<val>             Number of times to rotate each control point in sequence [default: 1].\n"));
		//String.
		INITSTRINGOPTION(ScaleType,    Eos(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_SCALE_TYPE, 	  _T("--scaletype"),			"none",				  SO_REQ_SEP,  "   --scaletype               The type of scaling to use with the --ws or --hs options. Valid values are --width --height [default: width].\n"));
		INITSTRINGOPTION(Device,	   Eos(eOptionUse::OPT_USE_ALL,		eOptionIDs::OPT_OPENCL_DEVICE,	  _T("--device"),				"0",				  SO_REQ_SEP,  "   --device                  The comma-separated OpenCL device indices to use. Single device: 0 Multi device: 0,1,3,4 [default: 0].\n"));
		INITSTRINGOPTION(IsaacSeed,    Eos(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_ISAAC_SEED,       _T("--isaac_seed"),           "",                   SO_REQ_SEP,  "   --isaac_seed=<val>        Character-based seed for the random number generator [default: random].\n"));
		INITSTRINGOPTION(Input,        Eos(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_IN,               _T("--in"),                   "",                   SO_REQ_SEP,  "   --in=<val>                Name of the input file.\n"));
		INITSTRINGOPTION(Out,          Eos(eOptionUse::OPT_USE_RENDER,	eOptionIDs::OPT_OUT,              _T("--out"),                  "",                   SO_REQ_SEP,  "   --out=<val>               Name of a single output file. Not recommended when rendering more than one image.\n"));
		INITSTRINGOPTION(Prefix,       Eos(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_PREFIX,           _T("--prefix"),               "",                   SO_REQ_SEP,  "   --prefix=<val>            Prefix to prepend to all output files.\n"));
		INITSTRINGOPTION(Suffix,       Eos(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_SUFFIX,           _T("--suffix"),               "",                   SO_REQ_SEP,  "   --suffix=<val>            Suffix to append to all output files.\n"));
#ifdef _WIN32
		INITSTRINGOPTION(Format,       Eos(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_FORMAT,           _T("--format"),               "png",                SO_REQ_SEP,  "   --format=<val>            Format of the output file. Valid values are: bmp, jpg, png, png16, exr or exr32 [default: png].\n"));
#else
		INITSTRINGOPTION(Format,       Eos(eOptionUse::OPT_RENDER_ANIM, eOptionIDs::OPT_FORMAT,           _T("--format"),               "png",                SO_REQ_SEP,  "   --format=<val>            Format of the output file. Valid values are: jpg, png, png16, exr or exr32 [default: png].\n"));
#endif
		INITSTRINGOPTION(PalettePath,  Eos(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_PALETTE_FILE,     _T("--flam3_palettes"),       "flam3-palettes.xml", SO_REQ_SEP,  "   --flam3_palettes=<val>    Path and name of the palette file [default: flam3-palettes.xml].\n"));
		INITSTRINGOPTION(Id,           Eos(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_ID,               _T("--id"),                   "",                   SO_REQ_SEP,  "   --id=<val>                ID to use in <edit> tags / image comments.\n"));
		INITSTRINGOPTION(Url,          Eos(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_URL,              _T("--url"),                  "",                   SO_REQ_SEP,  "   --url=<val>               URL to use in <edit> tags / image comments.\n"));
		INITSTRINGOPTION(Nick,         Eos(eOptionUse::OPT_USE_ALL,     eOptionIDs::OPT_NICK,             _T("--nick"),                 "",                   SO_REQ_SEP,  "   --nick=<val>              Nickname to use in <edit> tags / image comments.\n"));
		INITSTRINGOPTION(Comment,      Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_COMMENT,          _T("--comment"),              "",                   SO_REQ_SEP,  "   --comment=<val>           Comment to use in <edit> tags.\n"));
		INITSTRINGOPTION(TemplateFile, Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_TEMPLATE,         _T("--template"),             "",                   SO_REQ_SEP,  "   --template=<val>          Apply defaults based on this flame.\n"));
		INITSTRINGOPTION(Clone,        Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_CLONE,            _T("--clone"),                "",                   SO_REQ_SEP,  "   --clone=<val>             Clone random flame in input.\n"));
		INITSTRINGOPTION(CloneAll,     Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_CLONE_ALL,        _T("--clone_all"),            "",                   SO_REQ_SEP,  "   --clone_all=<val>         Clones all flames in the input file. Useful for applying template to all flames.\n"));
		INITSTRINGOPTION(CloneAction,  Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_CLONE_ACTION,     _T("--clone_action"),         "",                   SO_REQ_SEP,  "   --clone_action=<val>      A description of the clone action taking place.\n"));
		INITSTRINGOPTION(Animate,      Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_ANIMATE,          _T("--animate"),              "",                   SO_REQ_SEP,  "   --animate=<val>           Interpolates between all flames in the input file, using times specified in file.\n"));
		INITSTRINGOPTION(Mutate,       Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_MUTATE,           _T("--mutate"),               "",                   SO_REQ_SEP,  "   --mutate=<val>            Randomly mutate a random flame from the input file.\n"));
		INITSTRINGOPTION(Cross0,       Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_CROSS0,           _T("--cross0"),               "",                   SO_REQ_SEP,  "   --cross0=<val>            Randomly select one flame from the input file to genetically cross...\n"));
		INITSTRINGOPTION(Cross1,       Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_CROSS1,           _T("--cross1"),               "",                   SO_REQ_SEP,  "   --cross1=<val>            ...with one flame from this file.\n"));
		INITSTRINGOPTION(Method,       Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_METHOD,           _T("--method"),               "",                   SO_REQ_SEP,  "   --method=<val>            Method used for genetic cross: alternate, interpolate, or union. For mutate: all_vars, one_xform, add_symmetry, post_xforms, color_palette, delete_xform, all_coefs [default: random].\n"));//Original ommitted this important documentation for mutate!
		INITSTRINGOPTION(Inter,        Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_INTER,            _T("--inter"),                "",                   SO_REQ_SEP,  "   --inter=<val>             Interpolate the input file.\n"));
		INITSTRINGOPTION(Rotate,       Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_ROTATE,           _T("--rotate"),               "",                   SO_REQ_SEP,  "   --rotate=<val>            Rotate the input file.\n"));
		INITSTRINGOPTION(Sequence,     Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_SEQUENCE,         _T("--sequence"),             "",                   SO_REQ_SEP,  "   --sequence=<val>          360 degree rotation 'loops' times of each control point in the input file plus rotating transitions.\n"));
		INITSTRINGOPTION(UseVars,      Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_USE_VARS,         _T("--use_vars"),             "",                   SO_REQ_SEP,  "   --use_vars=<val>          Comma separated list of variation #'s to use when generating a random flame.\n"));
		INITSTRINGOPTION(DontUseVars,  Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_DONT_USE_VARS,    _T("--dont_use_vars"),        "",                   SO_REQ_SEP,  "   --dont_use_vars=<val>     Comma separated list of variation #'s to NOT use when generating a random flame.\n"));
		INITSTRINGOPTION(Extras,       Eos(eOptionUse::OPT_USE_GENOME,  eOptionIDs::OPT_EXTRAS,           _T("--extras"),               "",                   SO_REQ_SEP,  "   --extras=<val>            Extra attributes to place in the flame section of the Xml.\n"));
	}

	/// <summary>
	/// Parse and populate the supplied command line options for the specified program usage.
	/// If --help or --version were specified, information will be printed
	/// and parsing will cease.
	/// </summary>
	/// <param name="argc">The number of command line arguments passed</param>
	/// <param name="argv">The command line arguments passed</param>
	/// <param name="optUsage">The program for which these options are to be parsed and used.</param>
	/// <returns>True if --help or --version specified, else false</returns>
	bool Populate(int argc, _TCHAR* argv[], eOptionUse optUsage)
	{
		EmberOptions options;
		vector<CSimpleOpt::SOption> sOptions = options.GetSimpleOptions();
		CSimpleOpt args(argc, argv, sOptions.data());
		stringstream ss;
		ss.imbue(std::locale(std::locale(), new NoDelimiters()));

		//Process args.
		while (args.Next())
		{
			const auto errorCode = args.LastError();

			if (errorCode == SO_SUCCESS)
			{
				const auto e = eOptionIDs(args.OptionId());

				switch (e)
				{
					case eOptionIDs::OPT_HELP://Bool args.
					{
						ShowUsage(optUsage);
						return true;
					}

					case eOptionIDs::OPT_VERSION:
					{
						cout << EmberVersion() << "\n";
						return true;
					}

					PARSEBOOLOPTION(eOptionIDs::OPT_VERBOSE, Verbose);
					PARSEBOOLOPTION(eOptionIDs::OPT_DEBUG, Debug);
					PARSEBOOLOPTION(eOptionIDs::OPT_DUMP_ARGS, DumpArgs);
					PARSEBOOLOPTION(eOptionIDs::OPT_PROGRESS, DoProgress);
					PARSEBOOLOPTION(eOptionIDs::OPT_DUMP_OPENCL_INFO, OpenCLInfo);
					PARSEBOOLOPTION(eOptionIDs::OPT_ALL_VARS, AllVars);
					PARSEBOOLOPTION(eOptionIDs::OPT_REG_VARS, RegVars);
					PARSEBOOLOPTION(eOptionIDs::OPT_PRE_VARS, PreVars);
					PARSEBOOLOPTION(eOptionIDs::OPT_POST_VARS, PostVars);
					PARSEBOOLOPTION(eOptionIDs::OPT_SUM_VARS, SumVars);
					PARSEBOOLOPTION(eOptionIDs::OPT_ASSIGN_VARS, AssignVars);
					PARSEBOOLOPTION(eOptionIDs::OPT_PPSUM_VARS, PpSumVars);
					PARSEBOOLOPTION(eOptionIDs::OPT_PPASSIGN_VARS, PpAssignVars);
					PARSEBOOLOPTION(eOptionIDs::OPT_DC_VARS, DcVars);
					PARSEBOOLOPTION(eOptionIDs::OPT_STATE_VARS, StateVars);
					PARSEBOOLOPTION(eOptionIDs::OPT_PAR_VARS, ParVars);
					PARSEBOOLOPTION(eOptionIDs::OPT_NON_PAR_VARS, NonParVars);
					PARSEBOOLOPTION(eOptionIDs::OPT_OPENCL, EmberCL);
					PARSEBOOLOPTION(eOptionIDs::OPT_SP, Sp);
					PARSEBOOLOPTION(eOptionIDs::OPT_EARLYCLIP, EarlyClip);
					PARSEBOOLOPTION(eOptionIDs::OPT_POS_Y_UP, YAxisUp);
					PARSEBOOLOPTION(eOptionIDs::OPT_TRANSPARENCY, Transparency);
					PARSEBOOLOPTION(eOptionIDs::OPT_NAME_ENABLE, NameEnable);
					PARSEBOOLOPTION(eOptionIDs::OPT_HEX_PALETTE, HexPalette);
					PARSEBOOLOPTION(eOptionIDs::OPT_INSERT_PALETTE, InsertPalette);
					PARSEBOOLOPTION(eOptionIDs::OPT_ENABLE_COMMENTS, EnableComments);
					PARSEBOOLOPTION(eOptionIDs::OPT_WRITE_GENOME, WriteGenome);
					PARSEBOOLOPTION(eOptionIDs::OPT_THREADED_WRITE, ThreadedWrite);
					PARSEBOOLOPTION(eOptionIDs::OPT_ENCLOSED, Enclosed);
					PARSEBOOLOPTION(eOptionIDs::OPT_NO_EDITS, NoEdits);
					PARSEBOOLOPTION(eOptionIDs::OPT_UNSMOOTH_EDGE, UnsmoothEdge);
					PARSEBOOLOPTION(eOptionIDs::OPT_CW_LOOPS, CwLoops);
					PARSEBOOLOPTION(eOptionIDs::OPT_CW_INTERP_LOOPS, CwInterpLoops);
					PARSEBOOLOPTION(eOptionIDs::OPT_LOCK_ACCUM, LockAccum);
					PARSEBOOLOPTION(eOptionIDs::OPT_DUMP_KERNEL, DumpKernel);
					PARSEBOOLOPTION(eOptionIDs::OPT_FLAM3_COMPAT, Flam3Compat);
					PARSEBOOLOPTION(eOptionIDs::OPT_IGNORE_EXISTING, IgnoreExisting);
					PARSEOPTION(eOptionIDs::OPT_SYMMETRY, Symmetry);//Int args
					PARSEOPTION(eOptionIDs::OPT_SHEEP_GEN, SheepGen);
					PARSEOPTION(eOptionIDs::OPT_SHEEP_ID, SheepId);
					PARSEOPTION(eOptionIDs::OPT_PRIORITY, Priority);
					PARSEOPTION(eOptionIDs::OPT_NTHREADS, ThreadCount);//uint args.
					PARSEOPTION(eOptionIDs::OPT_STRIPS, Strips);
					PARSEOPTION(eOptionIDs::OPT_SUPERSAMPLE, Supersample);
					PARSEOPTION(eOptionIDs::OPT_TEMPSAMPLES, TemporalSamples);
					PARSEOPTION(eOptionIDs::OPT_PRINT_EDIT_DEPTH, PrintEditDepth);
					PARSEOPTION(eOptionIDs::OPT_JPEG, JpegQuality);
					PARSEOPTION(eOptionIDs::OPT_BEGIN, FirstFrame);
					PARSEOPTION(eOptionIDs::OPT_END, LastFrame);
					PARSEOPTION(eOptionIDs::OPT_FRAME, Frame);
					PARSEOPTION(eOptionIDs::OPT_DTIME, Dtime);
					PARSEOPTION(eOptionIDs::OPT_LOOP_FRAMES, LoopFrames);
					PARSEOPTION(eOptionIDs::OPT_INTERP_FRAMES, InterpFrames);
					PARSEOPTION(eOptionIDs::OPT_INTERP_LOOPS, InterpLoops);
					PARSEOPTION(eOptionIDs::OPT_REPEAT, Repeat);
					PARSEOPTION(eOptionIDs::OPT_TRIES, Tries);
					PARSEOPTION(eOptionIDs::OPT_MAX_XFORMS, MaxXforms);
					PARSEOPTION(eOptionIDs::OPT_START_COUNT, StartCount);
					PARSEOPTION(eOptionIDs::OPT_PADDING, Padding);
					PARSEOPTION(eOptionIDs::OPT_W, Width);
					PARSEOPTION(eOptionIDs::OPT_H, Height);
					PARSEOPTION(eOptionIDs::OPT_SS, SizeScale);//Double args.
					PARSEOPTION(eOptionIDs::OPT_WS, WidthScale);
					PARSEOPTION(eOptionIDs::OPT_HS, HeightScale);
					PARSEOPTION(eOptionIDs::OPT_QS, QualityScale);
					PARSEOPTION(eOptionIDs::OPT_QUALITY, Quality);
					PARSEOPTION(eOptionIDs::OPT_SBPCTTH, SBPctPerTh);
					PARSEOPTION(eOptionIDs::OPT_DE_MIN, DeMin);
					PARSEOPTION(eOptionIDs::OPT_DE_MAX, DeMax);
					PARSEOPTION(eOptionIDs::OPT_PIXEL_ASPECT, AspectRatio);
					PARSEOPTION(eOptionIDs::OPT_STAGGER, Stagger);
					PARSEOPTION(eOptionIDs::OPT_AVG_THRESH, AvgThresh);
					PARSEOPTION(eOptionIDs::OPT_BLACK_THRESH, BlackThresh);
					PARSEOPTION(eOptionIDs::OPT_WHITE_LIMIT, WhiteLimit);
					PARSEOPTION(eOptionIDs::OPT_SPEED, Speed);
					PARSEOPTION(eOptionIDs::OPT_OFFSETX, OffsetX);
					PARSEOPTION(eOptionIDs::OPT_OFFSETY, OffsetY);
					PARSEOPTION(eOptionIDs::OPT_USEMEM, UseMem);
					PARSEOPTION(eOptionIDs::OPT_LOOPS, Loops);
					PARSEOPTION(eOptionIDs::OPT_SCALE_TYPE, ScaleType);//String args.
					PARSEOPTION(eOptionIDs::OPT_OPENCL_DEVICE, Device);
					PARSEOPTION(eOptionIDs::OPT_ISAAC_SEED, IsaacSeed);
					PARSEOPTION(eOptionIDs::OPT_IN, Input);
					PARSEOPTION(eOptionIDs::OPT_OUT, Out);
					PARSEOPTION(eOptionIDs::OPT_PREFIX, Prefix);
					PARSEOPTION(eOptionIDs::OPT_SUFFIX, Suffix);
					PARSEOPTION(eOptionIDs::OPT_FORMAT, Format);
					PARSEOPTION(eOptionIDs::OPT_PALETTE_FILE, PalettePath);
					PARSEOPTION(eOptionIDs::OPT_ID, Id);
					PARSEOPTION(eOptionIDs::OPT_URL, Url);
					PARSEOPTION(eOptionIDs::OPT_NICK, Nick);
					PARSEOPTION(eOptionIDs::OPT_COMMENT, Comment);
					PARSEOPTION(eOptionIDs::OPT_TEMPLATE, TemplateFile);
					PARSEOPTION(eOptionIDs::OPT_CLONE, Clone);
					PARSEOPTION(eOptionIDs::OPT_CLONE_ALL, CloneAll);
					PARSEOPTION(eOptionIDs::OPT_CLONE_ACTION, CloneAction);
					PARSEOPTION(eOptionIDs::OPT_ANIMATE, Animate);
					PARSEOPTION(eOptionIDs::OPT_MUTATE, Mutate);
					PARSEOPTION(eOptionIDs::OPT_CROSS0, Cross0);
					PARSEOPTION(eOptionIDs::OPT_CROSS1, Cross1);
					PARSEOPTION(eOptionIDs::OPT_METHOD, Method);
					PARSEOPTION(eOptionIDs::OPT_INTER, Inter);
					PARSEOPTION(eOptionIDs::OPT_ROTATE, Rotate);
					PARSEOPTION(eOptionIDs::OPT_SEQUENCE, Sequence);
					PARSEOPTION(eOptionIDs::OPT_USE_VARS, UseVars);
					PARSEOPTION(eOptionIDs::OPT_DONT_USE_VARS, DontUseVars);
					PARSEOPTION(eOptionIDs::OPT_EXTRAS, Extras);

					default:
					{
						break;//Do nothing.
					}
				}
			}
			else
			{
				cout << "Invalid argument: " << args.OptionText() << "\n";
				cout << "\tReason: " << GetLastErrorText(errorCode) << "\n";
			}
		}

		auto strings = Split(Device(), ',');

		if (!strings.empty())
		{
			for (auto& s : strings)
			{
				size_t device = 0;
				istringstream istr(s);
				istr >> device;

				if (!istr.bad() && !istr.fail())
					m_Devices.push_back(device);
				else
					cout << "Failed to parse device index " << s;
			}
		}

		return false;
	}

	/// <summary>
	/// Return a const ref to m_Devices.
	/// </summary>
	/// <returns>A const ref to the vector of absolute device indices to be used</returns>
	const vector<size_t>& Devices() noexcept
	{
		return m_Devices;
	}

	/// <summary>
	/// Return a vector of all available options for the specified program.
	/// </summary>
	/// <param name="optUsage">The specified program usage</param>
	/// <returns>A vector of all available options for the specified program</returns>
	vector<CSimpleOpt::SOption> GetSimpleOptions(eOptionUse optUsage = eOptionUse::OPT_USE_ALL)
	{
		vector<CSimpleOpt::SOption> entries;
		CSimpleOpt::SOption const endOption = SO_END_OF_OPTIONS;
		entries.reserve(75);

		for (auto entry : m_BoolArgs)   if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) entries.push_back(entry->m_Option);

		for (auto entry : m_IntArgs)    if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) entries.push_back(entry->m_Option);

		for (auto entry : m_UintArgs)   if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) entries.push_back(entry->m_Option);

		for (auto entry : m_DoubleArgs) if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) entries.push_back(entry->m_Option);

		for (auto entry : m_StringArgs) if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) entries.push_back(entry->m_Option);

		entries.push_back(endOption);
		return entries;
	}

	/// <summary>
	/// Return a string with the descriptions of all available options for the specified program.
	/// </summary>
	/// <param name="optUsage">The specified program usage</param>
	/// <returns>A string with the descriptions of all available options for the specified program</returns>
	string GetUsage(eOptionUse optUsage = eOptionUse::OPT_USE_ALL)
	{
		ostringstream os;

		for (auto entry : m_BoolArgs)   if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) os << entry->m_DocString << "\n";

		for (auto entry : m_IntArgs)    if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) os << entry->m_DocString << "\n";

		for (auto entry : m_UintArgs)   if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) os << entry->m_DocString << "\n";

		for (auto entry : m_DoubleArgs) if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) os << entry->m_DocString << "\n";

		for (auto entry : m_StringArgs) if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) os << entry->m_DocString << "\n";

		return os.str();
	}

	/// <summary>
	/// Return a string with all of the names and values for all available options for the specified program.
	/// </summary>
	/// <param name="optUsage">The specified program usage</param>
	/// <returns>A string with all of the names and values for all available options for the specified program</returns>
	string GetValues(eOptionUse optUsage = eOptionUse::OPT_USE_ALL)
	{
		ostringstream os;
		os << std::boolalpha;

		for (auto entry : m_BoolArgs)   if (entry != nullptr) { if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) os << entry->m_NameWithoutDashes << ": " << (*entry)() << "\n"; }

		for (auto entry : m_IntArgs)    if (entry != nullptr) { if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) os << entry->m_NameWithoutDashes << ": " << (*entry)() << "\n"; }

		for (auto entry : m_UintArgs)   if (entry != nullptr) { if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) os << entry->m_NameWithoutDashes << ": " << (*entry)() << "\n"; }

		for (auto entry : m_DoubleArgs) if (entry != nullptr) { if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) os << entry->m_NameWithoutDashes << ": " << (*entry)() << "\n"; }

		for (auto entry : m_StringArgs) if (entry != nullptr) { if (static_cast<et>(entry->m_OptionUse) & static_cast<et>(optUsage)) os << entry->m_NameWithoutDashes << ": " << (*entry)() << "\n"; }

		return os.str();
	}

	/// <summary>
	/// Print description string, version and description of all available options for the specified program.
	/// </summary>
	/// <param name="optUsage">The specified program usage</param>
	void ShowUsage(eOptionUse optUsage)
	{
		cout << DescriptionString << " version " << EmberVersion() << "\n\n";

		if (optUsage == eOptionUse::OPT_USE_RENDER)
		{
			cout << "Usage:\n"
#ifdef _WIN32
				 "\tEmberRender.exe --in=test.flame [--out=outfile --verbose --progress --opencl]\n\n";
#else
				 "\temberrender --in=test.flame [--out=outfile --verbose --progress --opencl]\n\n";
#endif
		}
		else if (optUsage == eOptionUse::OPT_USE_ANIMATE)
		{
			cout << "Usage:\n"
#ifdef _WIN32
				 "\tEmberAnimate.exe --in=sequence.flame [--verbose --progress --opencl]\n\n";
#else
				 "\temberanimate --in=sequence.flame [--verbose --progress --opencl]\n\n";
#endif
		}
		else if (optUsage == eOptionUse::OPT_USE_GENOME)
		{
			cout << "Usage:\n"
#ifdef _WIN32
				 "\tEmberGenome.exe --sequence=test.flame > sequenceout.flame\n\n";
#else
				 "\tembergenome --sequence=test.flame > sequenceout.flame\n\n";
#endif
		}

		cout << GetUsage(optUsage) << "\n";
	}

	/// <summary>
	/// Return the last option parsing error text as a string.
	/// </summary>
	/// <param name="errorCode">The code of the last parsing error</param>
	/// <returns>The last option parsing error text as a string</returns>
	string GetLastErrorText(int errorCode) noexcept
	{
		switch (errorCode)
		{
			case SO_SUCCESS:            return "Success";

			case SO_OPT_INVALID:        return "Unrecognized option";

			case SO_OPT_MULTIPLE:       return "Option matched multiple strings";

			case SO_ARG_INVALID:        return "Option does not accept argument";

			case SO_ARG_INVALID_TYPE:   return "Invalid argument format";

			case SO_ARG_MISSING:        return "Required argument is missing";

			case SO_ARG_INVALID_DATA:   return "Invalid argument data";

			default:                    return "Unknown error";
		}
	}

	//Break from the usual m_* notation for members here because
	//each of these is a functor, so it looks nicer and is less typing
	//to just say opt.Member().
	Eob Help;//Diagnostic bool.
	Eob Version;
	Eob Verbose;
	Eob Debug;
	Eob DumpArgs;
	Eob DoProgress;
	Eob OpenCLInfo;
	Eob AllVars;
	Eob RegVars;
	Eob PreVars;
	Eob PostVars;
	Eob SumVars;
	Eob AssignVars;
	Eob PpSumVars;
	Eob PpAssignVars;
	Eob DcVars;
	Eob StateVars;
	Eob ParVars;
	Eob NonParVars;

	Eob EmberCL;//Value bool.
	Eob Sp;
	Eob EarlyClip;
	Eob YAxisUp;
	Eob Transparency;
	Eob NameEnable;
	Eob HexPalette;
	Eob InsertPalette;
	Eob EnableComments;
	Eob WriteGenome;
	Eob ThreadedWrite;
	Eob Enclosed;
	Eob NoEdits;
	Eob UnsmoothEdge;
	Eob CwLoops;
	Eob CwInterpLoops;
	Eob LockAccum;
	Eob DumpKernel;
	Eob Flam3Compat;
	Eob IgnoreExisting;

	Eoi Symmetry;//Value int.
	Eoi SheepGen;
	Eoi SheepId;
	Eoi Priority;
	Eou ThreadCount;//Value uint.
	Eou Strips;
	Eou Supersample;
	Eou TemporalSamples;
	Eou Bits;
	Eou PrintEditDepth;
	Eou JpegQuality;
	Eou FirstFrame;
	Eou LastFrame;
	Eou Frame;
	Eou Dtime;
	Eou LoopFrames;
	Eou InterpFrames;
	Eou InterpLoops;
	Eou Repeat;
	Eou Tries;
	Eou MaxXforms;
	Eou StartCount;
	Eou Padding;
	Eou Width;
	Eou Height;

	Eod SizeScale;//Value double.
	Eod WidthScale;
	Eod HeightScale;
	Eod QualityScale;
	Eod Quality;
	Eod DeMin;
	Eod DeMax;
	Eod SBPctPerTh;
	Eod AspectRatio;
	Eod Stagger;
	Eod AvgThresh;
	Eod BlackThresh;
	Eod WhiteLimit;
	Eod Speed;
	Eod OffsetX;
	Eod OffsetY;
	Eod UseMem;
	Eod Loops;

	Eos ScaleType;//Value string.
	Eos Device;
	Eos IsaacSeed;
	Eos Input;
	Eos Out;
	Eos Prefix;
	Eos Suffix;
	Eos Format;
	Eos PalettePath;
	Eos Id;
	Eos Url;
	Eos Nick;
	Eos Comment;
	Eos TemplateFile;
	Eos Clone;
	Eos CloneAll;
	Eos CloneAction;
	Eos Animate;
	Eos Mutate;
	Eos Cross0;
	Eos Cross1;
	Eos Method;
	Eos Inter;
	Eos Rotate;
	Eos Sequence;
	Eos UseVars;
	Eos DontUseVars;
	Eos Extras;

private:
	vector<size_t> m_Devices;
	vector<Eob*> m_BoolArgs;
	vector<Eoi*> m_IntArgs;
	vector<Eou*> m_UintArgs;
	vector<Eod*> m_DoubleArgs;
	vector<Eos*> m_StringArgs;
};
