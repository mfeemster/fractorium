#pragma once

#include "EmberCLPch.h"
#include "EmberCLStructs.h"

#define USEFMA 1

/// <summary>
/// OpenCL global function strings.
/// </summary>

namespace EmberCLns
{
/// <summary>
/// OpenCL equivalent of Palette::RgbToHsv().
/// </summary>
static constexpr char RgbToHsvFunctionString[] =
	//rgb 0 - 1,
	//h 0 - 6, s 0 - 1, v 0 - 1
	"static inline void RgbToHsv(real4_bucket* rgb, real4_bucket* hsv)\n"
	"{\n"
	"	real_bucket_t max, min, del, rc, gc, bc;\n"
	"\n"
	//Compute maximum of r, g, b.
	"	if ((*rgb).x >= (*rgb).y)\n"
	"	{\n"
	"		if ((*rgb).x >= (*rgb).z)\n"
	"			max = (*rgb).x;\n"
	"		else\n"
	"			max = (*rgb).z;\n"
	"	}\n"
	"	else\n"
	"	{\n"
	"		if ((*rgb).y >= (*rgb).z)\n"
	"			max = (*rgb).y;\n"
	"		else\n"
	"			max = (*rgb).z;\n"
	"	}\n"
	"\n"
	//Compute minimum of r, g, b.
	"	if ((*rgb).x <= (*rgb).y)\n"
	"	{\n"
	"		if ((*rgb).x <= (*rgb).z)\n"
	"			min = (*rgb).x;\n"
	"		else\n"
	"			min = (*rgb).z;\n"
	"	}\n"
	"	else\n"
	"	{\n"
	"		if ((*rgb).y <= (*rgb).z)\n"
	"			min = (*rgb).y;\n"
	"		else\n"
	"			min = (*rgb).z;\n"
	"	}\n"
	"\n"
	"	del = max - min;\n"
	"	(*hsv).z = max;\n"
	"\n"
	"	if (max != 0)\n"
	"		(*hsv).y = del / max;\n"
	"	else\n"
	"		(*hsv).y = 0;\n"
	"\n"
	"	(*hsv).x = 0;\n"
	"	if ((*hsv).y != 0)\n"
	"	{\n"
	"		rc = (max - (*rgb).x) / del;\n"
	"		gc = (max - (*rgb).y) / del;\n"
	"		bc = (max - (*rgb).z) / del;\n"
	"\n"
	"		if ((*rgb).x == max)\n"
	"			(*hsv).x = bc - gc;\n"
	"		else if ((*rgb).y == max)\n"
	"			(*hsv).x = 2 + rc - bc;\n"
	"		else if ((*rgb).z == max)\n"
	"			(*hsv).x = 4 + gc - rc;\n"
	"\n"
	"		if ((*hsv).x < 0)\n"
	"			(*hsv).x += 6;\n"
	"	}\n"
	"}\n"
	"\n";

/// <summary>
/// OpenCL equivalent of Palette::HsvToRgb().
/// </summary>
static constexpr char HsvToRgbFunctionString[] =
	//h 0 - 6, s 0 - 1, v 0 - 1
	//rgb 0 - 1
	"static inline void HsvToRgb(real4_bucket* hsv, real4_bucket* rgb)\n"
	"{\n"
	"	int j;\n"
	"	real_bucket_t f, p, q, t;\n"
	"\n"
	"	while ((*hsv).x >= 6)\n"
	"		(*hsv).x = (*hsv).x - 6;\n"
	"\n"
	"	while ((*hsv).x <  0)\n"
	"		(*hsv).x = (*hsv).x + 6;\n"
	"\n"
	"	j = (int)floor((*hsv).x);\n"
	"	f = (*hsv).x - j;\n"
	"	p = (*hsv).z * (1 - (*hsv).y);\n"
	"	q = (*hsv).z * (1 - ((*hsv).y * f));\n"
	"	t = (*hsv).z * (1 - ((*hsv).y * (1 - f)));\n"
	"\n"
	"	switch (j)\n"
	"	{\n"
	"		case 0:  (*rgb).x = (*hsv).z; (*rgb).y = t;		   (*rgb).z = p;	    break;\n"
	"		case 1:  (*rgb).x = q;		  (*rgb).y = (*hsv).z; (*rgb).z = p;	    break;\n"
	"		case 2:  (*rgb).x = p;		  (*rgb).y = (*hsv).z; (*rgb).z = t;	    break;\n"
	"		case 3:  (*rgb).x = p;		  (*rgb).y = q;		   (*rgb).z = (*hsv).z; break;\n"
	"		case 4:  (*rgb).x = t;		  (*rgb).y = p;		   (*rgb).z = (*hsv).z; break;\n"
	"		case 5:  (*rgb).x = (*hsv).z; (*rgb).y = p;		   (*rgb).z = q;	    break;\n"
	"		default: (*rgb).x = (*hsv).z; (*rgb).y = t;		   (*rgb).z = p;	    break;\n"
	"	}\n"
	"}\n"
	"\n";

/// <summary>
/// OpenCL equivalent of Palette::CalcAlpha().
/// </summary>
static constexpr char CalcAlphaFunctionString[] =
	"static inline real_t CalcAlpha(real_bucket_t density, real_bucket_t gamma, real_bucket_t linrange)\n"//Not the slightest clue what this is doing.//DOC
	"{\n"
	"	real_bucket_t frac, alpha, funcval = pow(linrange, gamma);\n"
	"\n"
	"	if (density > 0)\n"
	"	{\n"
	"		if (density < linrange)\n"
	"		{\n"
	"			frac = density / linrange;\n"
	"			alpha = (1.0 - frac) * density * (funcval / linrange) + frac * pow(density, gamma);\n"
	"		}\n"
	"		else\n"
	"			alpha = pow(density, gamma);\n"
	"	}\n"
	"	else\n"
	"		alpha = 0;\n"
	"\n"
	"	return alpha;\n"
	"}\n"
	"\n";


/// <summary>
/// OpenCL equivalent of Renderer::CurveAdjust().
/// Only use float here instead of real_t because the output will be passed to write_imagef()
/// during final accumulation, which only takes floats.
/// </summary>
static constexpr char CurveAdjustFunctionString[] =
	"static inline void CurveAdjust(__global real4reals_bucket* csa, float* a, uint index)\n"
	"{\n"
	"	uint tempIndex = (uint)clamp(*a * CURVES_LENGTH_M1, 0.0f, CURVES_LENGTH_M1);\n"
	"	uint tempIndex2 = (uint)clamp(csa[tempIndex].m_Real4.x * CURVES_LENGTH_M1, 0.0f, CURVES_LENGTH_M1);\n"
	"\n"
	"	*a = (float)csa[tempIndex2].m_Reals[index];\n"
	"}\n"
	"\n";

/// <summary>
/// Use MWC 64 from David Thomas at the Imperial College of London for
/// random numbers in OpenCL, instead of ISAAC which was used
/// for CPU rendering.
/// </summary>
static constexpr char RandFunctionString[] =
	"enum { MWC64X_A = 4294883355u };\n\n"
	"inline uint MwcNext(uint2* s)\n"
	"{\n"
	"	uint res = (*s).x ^ (*s).y;			\n"//Calculate the result.
	"	uint hi = mul_hi((*s).x, MWC64X_A); \n"//Step the RNG.
	"	(*s).x = (*s).x * MWC64X_A + (*s).y;\n"//Pack the state back up.
	"	(*s).y = hi + ((*s).x < (*s).y);	\n"
	"	return res;							\n"//Return the next result.
	"}\n"
	"\n"
	"inline uint MwcNextRange(uint2* s, uint val)\n"
	"{\n"
	"	return (val == 0) ? MwcNext(s) : (uint)(((ulong)MwcNext(s) * (ulong)val) >> 32);\n"
	"}\n"
	"\n"
#if NEW_OPTS
	"inline real_t MwcNext01(uint2* s)\n"
	"{\n"
	"	return MwcNext(s) * MAX_UINT_RECIP;\n"
	"}\n"
	"\n"
	"inline uint MwcNextCrand(uint2* s)\n"
	"{\n"
	"	return MwcNextRange(s, 32767u);\n"
	"}\n"
	"\n"
	"inline real_t MwcNextFRange(uint2* s, real_t lower, real_t upper)\n"
	"{\n"
	"	real_t f = MwcNext(s) * MAX_UINT_RECIP;\n"
#ifdef USEFMA
	"	return fma(f, upper - lower, lower);\n"
#else
	"	return (f * (upper - lower) + lower);\n"
#endif
	"}\n"
	"\n"
	"inline real_t MwcNextNeg1Pos1(uint2* s)\n"
	"{\n"
	"	real_t f = MwcNext(s) * MAX_UINT_RECIP;\n"
#ifdef USEFMA
	"	return fma(f, (real_t)2.0, (real_t)-1.0);\n"
#else
	"	return (f * (real_t)2.0 + (real_t)-1.0);\n"
#endif
	"}\n"
	"\n"
	"inline real_t MwcNext0505(uint2* s)\n"
	"{\n"
	"	real_t f = MwcNext(s) * MAX_UINT_RECIP;\n"
	"	return (real_t)-0.5 + f;\n"
	"}\n"
#else
	"inline real_t MwcNext01(uint2* s)\n"
	"{\n"
	"	return MwcNext(s) * (real_t)(1.0 / 4294967296.0);\n"
	"}\n"
	"\n"
	"inline uint MwcNextCrand(uint2* s)\n"
	"{\n"
	"	return MwcNextRange(s, 32767u);\n"
	"}\n"
	"\n"
	"inline real_t MwcNextFRange(uint2* s, real_t lower, real_t upper)\n"
	"{\n"
	"	real_t f = (real_t)MwcNext(s) / (real_t)UINT_MAX;\n"
#ifdef USEFMA
	"	return fma(f, upper - lower, lower);\n"
#else
	"	return (f * (upper - lower) + lower);\n"
#endif
	"}\n"
	"\n"
	"inline real_t MwcNextNeg1Pos1(uint2* s)\n"
	"{\n"
	"	real_t f = (real_t)MwcNext(s) / (real_t)UINT_MAX;\n"
#ifdef USEFMA
	"	return fma(f, (real_t)2.0, (real_t)-1.0);\n"
#else
	"	return (f * (real_t)2.0 + (real_t)-1.0);\n"
#endif
	"}\n"
	"\n"
	"inline real_t MwcNext0505(uint2* s)\n"
	"{\n"
	"	real_t f = (real_t)MwcNext(s) / (real_t)UINT_MAX;\n"
	"	return -0.5 + f;\n"
	"}\n"
#endif
	"\n";

/// <summary>
/// OpenCL equivalent Renderer::AddToAccum().
/// </summary>
static constexpr char AddToAccumWithCheckFunctionString[] =
	"inline bool AccumCheck(int superRasW, int superRasH, int i, int ii, int j, int jj)\n"
	"{\n"
	"	return (j + jj >= 0 && j + jj < superRasH && i + ii >= 0 && i + ii < superRasW);\n"
	"}\n"
	"\n";

/// <summary>
/// OpenCL equivalent various CarToRas member functions.
/// Normaly would subtract m_RasLlX and m_RasLlY, but they were negated in RendererCL before being passed in, so they could be used with fma().
/// </summary>
static constexpr char CarToRasFunctionString[] =
	"inline void CarToRasConvertPointToSingle(__constant CarToRasCL* carToRas, Point* point, uint* singleBufferIndex)\n"
	"{\n"
#ifdef USEFMA
	"	*singleBufferIndex = (uint)fma(carToRas->m_PixPerImageUnitW, point->m_X, carToRas->m_RasLlX) + (carToRas->m_RasWidth * (uint)fma(carToRas->m_PixPerImageUnitH, point->m_Y, carToRas->m_RasLlY));\n"
#else
	"	*singleBufferIndex = (uint)(carToRas->m_PixPerImageUnitW * point->m_X + carToRas->m_RasLlX) + (carToRas->m_RasWidth * (uint)(carToRas->m_PixPerImageUnitH * point->m_Y + carToRas->m_RasLlY));\n"
#endif
	"}\n"
	"\n"
	"inline bool CarToRasInBounds(__constant CarToRasCL* carToRas, Point* point)\n"
	"{\n"
	"	return point->m_X >= carToRas->m_CarLlX &&\n"
	"		point->m_X < carToRas->m_CarUrX &&\n"
	"		point->m_Y < carToRas->m_CarUrY &&\n"
	"		point->m_Y >= carToRas->m_CarLlY;\n"
	"}\n"
	"\n";

static constexpr char AtomicString[] =
   "void AtomicAdd(volatile __global real_bucket_t* source, const real_bucket_t operand)\n"
   "{\n"
   "	union\n"
   "	{\n"
   "		atomi intVal;\n"
   "		real_bucket_t realVal;\n"
   "	} newVal;\n"
   "\n"
   "	union\n"
   "	{\n"
   "		atomi intVal;\n"
   "		real_bucket_t realVal;\n"
   "	} prevVal;\n"
   "\n"
   "	do\n"
   "	{\n"
   "		prevVal.realVal = *source;\n"
   "		newVal.realVal = prevVal.realVal + operand;\n"
   "	} while (atomic_cmpxchg((volatile __global atomi*)source, prevVal.intVal, newVal.intVal) != prevVal.intVal);\n"
   "}\n";

}
