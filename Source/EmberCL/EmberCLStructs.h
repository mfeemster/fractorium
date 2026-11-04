#pragma once

#include "EmberCLPch.h"
//#define NEW_OPTS 1

/// <summary>
/// Various data structures defined for the CPU and OpenCL.
/// These are stripped down versions of THE classes in Ember, for use with OpenCL.
/// Their sole purpose is to pass values from the host to the device.
/// They retain most of the member variables, but do not contain the functions.
/// Visual Studio defaults to alighment of 16, but it's made explicit in case another compiler is used.
/// This must match the alignment specified in the kernel.
/// </summary>

namespace EmberCLns
{
/// <summary>
/// Various constants needed for rendering.
/// </summary>
static string ConstantDefinesString(bool doublePrecision)
{
	ostringstream os;
	os << "#if defined(cl_amd_fp64)\n"//AMD extension available?
	   "	#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n"
	   "#endif\n"
	   "#if defined(cl_khr_fp64)\n"//Khronos extension available?
	   "	#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
	   "#endif\n"
	   "#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable\n";//Only supported on nVidia.

	if (doublePrecision)
	{
		os <<
		   "typedef long intPrec;\n"
		   "typedef uint atomi;\n"//Same size as real_bucket_t, always 4 bytes.
		   "typedef double real_t;\n"
		   "typedef float real_bucket_t;\n"//Assume buckets are always float, even though iter calcs are in double.
		   "typedef double2 real2;\n"
		   "typedef double3 real3;\n"
		   "typedef double4 real4;\n"
		   "typedef float4 real4_bucket;\n"//And here too.
		   "#define EPS (DBL_EPSILON)\n"
		   "#define TLOW (DBL_MIN)\n"
		   "#define TMAX (DBL_MAX)\n";
	}
	else
	{
		os << "typedef int intPrec;\n"
		   "typedef uint atomi;\n"
		   "typedef float real_t;\n"
		   "typedef float real_bucket_t;\n"
		   "typedef float2 real2;\n"
		   "typedef float3 real3;\n"
		   "typedef float4 real4;\n"
		   "typedef float4 real4_bucket;\n"
		   "#define EPS (FLT_EPSILON)\n"
		   "#define TLOW (FLT_MIN)\n"
		   "#define TMAX (FLT_MAX)\n";
	}

	os <<
#ifdef __APPLE__
		"#define fma(a, b, c) mad((a), (b), (c))\n"
#endif
	   "typedef          long int int64;\n"
	   "typedef unsigned long int uint64;\n"
	   "\n"
	   "#define EPS6 ((1e-6))\n"
	   "\n"
	   "//The number of threads per block used in the iteration function. Don't change\n"
	   "//it lightly; the block size is hard coded to be exactly 32 x 8.\n"
	   "#define NTHREADS 256u\n"
	   "#define THREADS_PER_WARP 32u\n"
	   "#define NWARPS (NTHREADS / THREADS_PER_WARP)\n"
	   "#define DE_THRESH 100u\n"
	   "#define BadVal(x) (isnan(x))\n"
	   "#define SQR(x) ((x) * (x))\n"
	   "#define CUBE(x) ((x) * (x) * (x))\n"
	   "#define MPI ((real_t)M_PI)\n"
	   "#define MPI2 ((real_t)M_PI_2)\n"
	   "#define MPI3 ((real_t)(1.0471975511965977461542144610932))\n"
	   "#define MPI4 ((real_t)M_PI_4)\n"
	   "#define MPI6 ((real_t)(0.52359877559829887307710723054658))\n"
	   "#define M1PI ((real_t)M_1_PI)\n"
	   "#define M2PI ((real_t)M_2_PI)\n"
	   "#define M_2PI (MPI * 2)\n"
	   "#define M_3PI (MPI * 3)\n"
	   "#define M_SQRT3 ((real_t)(1.7320508075688772935274463415059))\n"
	   "#define M_SQRT3_2 ((real_t)(0.86602540378443864676372317075294))\n"
	   "#define M_SQRT3_3 ((real_t)(0.57735026918962576450914878050196))\n"
	   "#define M_SQRT5 ((real_t)(2.2360679774997896964091736687313))\n"
	   "#define M_PHI ((real_t)(1.61803398874989484820458683436563))\n"
	   "#define M_1_2PI ((real_t)(0.15915494309189533576888376337251))\n"
	   "#define DEG_2_RAD (MPI / 180)\n"
	   "#define CURVES_LENGTH_M1 ((real_bucket_t)" << CURVES_LENGTH_M1 << ")\n" <<
	   "#define ONE_OVER_CURVES_LENGTH_M1 ((real_bucket_t)" << ONE_OVER_CURVES_LENGTH_M1 << ")\n" <<
	   "#define MAX_UINT_RECIP ((real_t)(2.32831e-10))\n"//1 / ((2^32)-1).
	   "\n"
	   "//Index in each dimension of a thread within a block.\n"
	   "#define THREAD_ID_X   (get_local_id(0))\n"
	   "#define THREAD_ID_Y   (get_local_id(1))\n"
	   "#define THREAD_ID_Z   (get_local_id(2))\n"
	   "\n"
	   "//Index in each dimension of a block within a grid.\n"
	   "#define BLOCK_ID_X    (get_group_id(0))\n"
	   "#define BLOCK_ID_Y    (get_group_id(1))\n"
	   "#define BLOCK_ID_Z    (get_group_id(2))\n"
	   "\n"
	   "//Absolute index in each dimension of a thread within a grid.\n"
	   "#define GLOBAL_ID_X   (get_global_id(0))\n"
	   "#define GLOBAL_ID_Y   (get_global_id(1))\n"
	   "#define GLOBAL_ID_Z   (get_global_id(2))\n"
	   "\n"
	   "//Dimensions of a block.\n"
	   "#define BLOCK_SIZE_X  (get_local_size(0))\n"
	   "#define BLOCK_SIZE_Y  (get_local_size(1))\n"
	   "#define BLOCK_SIZE_Z  (get_local_size(2))\n"
	   "\n"
	   "//Dimensions of a grid, in terms of blocks.\n"
	   "#define GRID_SIZE_X   (get_num_groups(0))\n"
	   "#define GRID_SIZE_Y   (get_num_groups(1))\n"
	   "#define GRID_SIZE_Z   (get_num_groups(2))\n"
	   "\n"
	   "//Dimensions of a grid, in terms of threads.\n"
	   "#define GLOBAL_SIZE_X (get_global_size(0))\n"
	   "#define GLOBAL_SIZE_Y (get_global_size(1))\n"
	   "#define GLOBAL_SIZE_Z (get_global_size(2))\n"
	   "\n"
	   "#define INDEX_IN_BLOCK_2D (THREAD_ID_Y * BLOCK_SIZE_X + THREAD_ID_X)\n"
	   "#define INDEX_IN_BLOCK_3D ((BLOCK_SIZE_X * BLOCK_SIZE_Y * THREAD_ID_Z) + INDEX_IN_BLOCK_2D)\n"
	   "\n"
	   "#define INDEX_IN_GRID_2D (GLOBAL_ID_Y * GLOBAL_SIZE_X + GLOBAL_ID_X)\n"
	   "#define INDEX_IN_GRID_3D ((GLOBAL_SIZE_X * GLOBAL_SIZE_Y * GLOBAL_ID_Z) + INDEX_IN_GRID_2D)\n"
	   "\n"
	   "#define BLOCK_START_INDEX_IN_GRID_2D ((BLOCK_ID_Y * GRID_SIZE_X * BLOCK_SIZE_Y * BLOCK_SIZE_X) + (BLOCK_ID_X * BLOCK_SIZE_X * BLOCK_SIZE_Y))\n"
	   "\n";
	return os.str();
}

/// <summary>
/// A point structure on the host that maps to the one used on the device to iterate in OpenCL.
/// It might seem better to use vec4, however 2D palettes and even 3D coordinates may eventually
/// be supported, which will make it more than 4 members.
/// </summary>
#ifdef NEW_OPTS
	#ifdef _WIN32
		#pragma pack(push, 1)
	#endif
#endif
template <typename T>
struct
#ifdef NEW_OPTS
	#ifndef _WIN32
		__attribute__((packed))
	#endif
#endif
PointCL
{
	T m_X;
	T m_Y;
	T m_Z;
	T m_ColorX;
	uint m_LastXfUsed;
};
#ifdef NEW_OPTS
	#ifdef _WIN32
		#pragma pack(pop)
	#endif
#endif

/// <summary>
/// The point structure used to iterate in OpenCL.
/// It might seem better to use float4, however 2D palettes and even 3D coordinates may eventually
/// be supported, which will make it more than 4 members.
/// </summary>
static constexpr char PointCLStructString[] =
	"typedef struct _Point\n"
	"{\n"
	"	real_t m_X;\n"
	"	real_t m_Y;\n"
	"	real_t m_Z;\n"
	"	real_t m_ColorX;\n"
	"	uint m_LastXfUsed;\n"
#ifdef NEW_OPTS
	"} __attribute__((packed)) Point;\n"
#else
	"} Point;\n"
#endif
	"\n";

/// <summary>
/// A structure on the host used to hold all of the needed information for an xform used on the device to iterate in OpenCL.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
struct ALIGN XformCL
{
	T m_A, m_B, m_C, m_D, m_E, m_F;//24 (48)
	T m_PostA, m_PostB, m_PostC, m_PostD, m_PostE, m_PostF;//48 (96)
	T m_DirectColor;//52 (104)
	T m_ColorSpeedCache;//56 (112)
	T m_OneMinusColorCache;//60 (120)
	T m_Opacity;//64 (128)
};

/// <summary>
/// The xform structure used to iterate in OpenCL.
/// </summary>
static constexpr char XformCLStructString[] =
	"typedef struct __attribute__ " ALIGN_CL " _XformCL\n"
	"{\n"
	"	real_t m_A, m_B, m_C, m_D, m_E, m_F;\n"
	"	real_t m_PostA, m_PostB, m_PostC, m_PostD, m_PostE, m_PostF;\n"
	"	real_t m_DirectColor;\n"
	"	real_t m_ColorSpeedCache;\n"
	"	real_t m_OneMinusColorCache;\n"
	"	real_t m_Opacity;\n"
	"} XformCL;\n"
	"\n";

/// <summary>
/// A structure on the host used to hold all of the needed information for an ember used on the device to iterate in OpenCL.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
struct ALIGN EmberCL
{
	T m_RandPointRange;
	T m_CamZPos;
	T m_CamPerspective;
	T m_CamYaw;
	T m_CamPitch;
	T m_BlurCurve;
	T m_CamDepthBlur;
	T m_BlurCoef;
	m3T m_CamMat;
	T m_CenterX, m_CenterY;
	T m_RotA, m_RotB, m_RotD, m_RotE;
	T m_Psm1;
	T m_Psm2;
};

/// <summary>
/// The ember structure used to iterate in OpenCL.
/// </summary>
static constexpr char EmberCLStructString[] =
	"typedef struct __attribute__ " ALIGN_CL " _EmberCL\n"
	"{\n"
	"	real_t m_RandPointRange;\n"
	"	real_t m_CamZPos;\n"
	"	real_t m_CamPerspective;\n"
	"	real_t m_CamYaw;\n"
	"	real_t m_CamPitch;\n"
	"	real_t m_BlurCurve;\n"
	"	real_t m_CamDepthBlur;\n"
	"	real_t m_BlurCoef;\n"
	"	real_t m_C00;\n"
	"	real_t m_C01;\n"
	"	real_t m_C02;\n"
	"	real_t m_C10;\n"
	"	real_t m_C11;\n"
	"	real_t m_C12;\n"
	"	real_t m_C20;\n"
	"	real_t m_C21;\n"
	"	real_t m_C22;\n"
	"	real_t m_CenterX, m_CenterY;\n"
	"	real_t m_RotA, m_RotB, m_RotD, m_RotE;\n"
	"	real_t m_Psm1;\n"
	"	real_t m_Psm2;\n"
	"} EmberCL;\n"
	"\n";

/// <summary>
/// A structure on the host used to hold all of the needed information for cartesian to raster mapping used on the device to iterate in OpenCL.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
struct ALIGN CarToRasCL
{
	T m_PixPerImageUnitW, m_RasLlX;
	uint m_RasWidth;
	T m_PixPerImageUnitH, m_RasLlY;
	T m_CarLlX, m_CarUrX, m_CarUrY, m_CarLlY;
	T m_CarHalfX, m_CarHalfY, m_CarCenterX, m_CarCenterY;
};

/// <summary>
/// The cartesian to raster structure used to iterate in OpenCL.
/// </summary>
static constexpr char CarToRasCLStructString[] =
	"typedef struct __attribute__ " ALIGN_CL " _CarToRasCL\n"
	"{\n"
	"	real_t m_PixPerImageUnitW, m_RasLlX;\n"
	"	uint m_RasWidth;\n"
	"	real_t m_PixPerImageUnitH, m_RasLlY;\n"
	"	real_t m_CarLlX, m_CarUrX, m_CarUrY, m_CarLlY;\n"
	"	real_t m_CarHalfX, m_CarHalfY, m_CarCenterX, m_CarCenterY;\n"
	"} CarToRasCL;\n"
	"\n";

/// <summary>
/// A structure on the host used to hold all of the needed information for density filtering used on the device to iterate in OpenCL.
/// Note that the actual filter buffer is held elsewhere.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
struct ALIGN DensityFilterCL
{
	T m_Curve;
	T m_K1;
	T m_K2;
	uint m_Supersample;
	uint m_SuperRasW;
	uint m_SuperRasH;
	uint m_KernelSize;
	uint m_MaxFilterIndex;
	uint m_MaxFilteredCounts;
	uint m_FilterWidth;
};

/// <summary>
/// The density filtering structure used to iterate in OpenCL.
/// Note that the actual filter buffer is held elsewhere.
/// </summary>
static constexpr char DensityFilterCLStructString[] =
	"typedef struct __attribute__ " ALIGN_CL " _DensityFilterCL\n"
	"{\n"
	"	real_bucket_t m_Curve;\n"
	"	real_bucket_t m_K1;\n"
	"	real_bucket_t m_K2;\n"
	"	uint m_Supersample;\n"
	"	uint m_SuperRasW;\n"
	"	uint m_SuperRasH;\n"
	"	uint m_KernelSize;\n"
	"	uint m_MaxFilterIndex;\n"
	"	uint m_MaxFilteredCounts;\n"
	"	uint m_FilterWidth;\n"
	"} DensityFilterCL;\n"
	"\n";

/// <summary>
/// A structure on the host used to hold all of the needed information for spatial filtering used on the device to iterate in OpenCL.
/// Note that the actual filter buffer is held elsewhere.
/// </summary>
template <typename T>
struct ALIGN SpatialFilterCL
{
	uint m_SuperRasW = 0;
	uint m_SuperRasH = 0;
	uint m_FinalRasW = 0;
	uint m_FinalRasH = 0;
	uint m_Supersample = 0;
	uint m_FilterWidth = 0;
	uint m_DensityFilterOffset = 0;
	uint m_YAxisUp = 0;
	T m_Vibrancy = 0;
	T m_HighlightPower = 0;
	T m_Gamma = 0;
	T m_LinRange = 0;
	Color<T> m_Background;
};

/// <summary>
/// The spatial filtering structure used to iterate in OpenCL.
/// Note that the actual filter buffer is held elsewhere.
/// </summary>
static constexpr char SpatialFilterCLStructString[] =
	"typedef struct __attribute__" ALIGN_CL "_SpatialFilterCL\n"
	"{\n"
	"	uint m_SuperRasW;\n"
	"	uint m_SuperRasH;\n"
	"	uint m_FinalRasW;\n"
	"	uint m_FinalRasH;\n"
	"	uint m_Supersample;\n"
	"	uint m_FilterWidth;\n"
	"	uint m_DensityFilterOffset;\n"
	"	uint m_YAxisUp;\n"
	"	real_bucket_t m_Vibrancy;\n"
	"	real_bucket_t m_HighlightPower;\n"
	"	real_bucket_t m_Gamma;\n"
	"	real_bucket_t m_LinRange;\n"
	"	real_bucket_t m_Background[4];\n"//For some reason, using float4/double4 here does not align no matter what. So just use an array of 4.
	"} SpatialFilterCL;\n"
	"\n";

/// <summary>
/// EmberCL makes extensive use of the build in vector types, however accessing
/// their members as a buffer is not natively supported.
/// Declaring them in a union with a buffer resolves this problem.
/// </summary>
static constexpr char UnionCLStructString[] =
	"typedef union\n"
	"{\n"
	"	uchar3 m_Uchar3;\n"
	"	uchar m_Uchars[3];\n"
	"} uchar3uchars;\n"
	"\n"
	"typedef union\n"
	"{\n"
	"	uchar4 m_Uchar4;\n"
	"	uchar m_Uchars[4];\n"
	"} uchar4uchars;\n"
	"\n"
	"typedef union\n"
	"{\n"
	"	uint4 m_Uint4;\n"
	"	uint m_Uints[4];\n"
	"} uint4uints;\n"
	"\n"
	"typedef union\n"//Use in places where float is required.
	"{\n"
	"	float4 m_Float4;\n"
	"	float m_Floats[4];\n"
	"} float4floats;\n"
	"\n"
	"typedef union\n"//Use in places where float or double can be used depending on the template type.
	"{\n"
	"	real4 m_Real4;\n"
	"	real_t m_Reals[4];\n"
	"} real4reals;\n"
	"\n"
	"typedef union\n"//Used to match the bucket template type.
	"{\n"
	"	real4_bucket m_Real4;\n"
	"	real_bucket_t m_Reals[4];\n"
	"} real4reals_bucket;\n"
	"\n";
}
