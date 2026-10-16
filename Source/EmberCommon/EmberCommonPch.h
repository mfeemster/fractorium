#ifdef _WIN32
	#pragma once
#endif

/// <summary>
/// Precompiled header file. Place all system includes here with appropriate #defines for different operating systems and compilers.
/// </summary>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN//Exclude rarely-used stuff from Windows headers.
#define _USE_MATH_DEFINES

#ifdef _WIN32
	#pragma warning(disable : 4251; disable : 4661; disable : 4100)
	#include <SDKDDKVer.h>
	#include <windows.h>
	#include <winsock.h>//For htons().
	#include <BaseTsd.h>
	#include <crtdbg.h>
	#include <tchar.h>
#else
	#include <arpa/inet.h>
	#include <unistd.h>
	#define _TCHAR char
	#define _tmain main
	#define _T
#endif

#include <iostream>
#include <iomanip>
#include <ostream>
#include <random>
#include <sstream>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jconfig.h"
#include "jpeglib.h"

#define PNG_SKIP_SETJMP_CHECK 1

#include "png.h"

//Ember.
#include "Ember.h"
#include "Variation.h"
#include "EmberToXml.h"
#include "XmlToEmber.h"
#include "PaletteList.h"
#include "Iterator.h"
#include "Renderer.h"
#include "RendererCL.h"
#include "SheepTools.h"

#ifndef FRACTORIUM_PCH_H
	//Options.
	#include "SimpleGlob.h"
	#include "SimpleOpt.h"
#endif

//Exr
#ifdef _WIN32
	#define OPENEXR_DLL 1
#endif

#ifndef _WIN32
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
    #pragma GCC diagnostic ignored "-Wshadow"
    #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#ifdef __APPLE__
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfStringAttribute.h>
#include <half.h>
//May need these instead for openexr3
//#include <Imath/half.h>
//#include <Imath/ImathInt64.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfOutputFile.h>
#define ENUM_DYLD_BOOL
#include <mach-o/dyld.h>

#define _MM_DENORMALS_ZERO_MASK   0x0040
#define _MM_DENORMALS_ZERO_ON     0x0040
#define _MM_DENORMALS_ZERO_OFF    0x0000

#define _MM_SET_DENORMALS_ZERO_MODE(mode)                                   \
	_mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (mode))
#define _MM_GET_DENORMALS_ZERO_MODE()                                       \
	(_mm_getcsr() & _MM_DENORMALS_ZERO_MASK)
#else
#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfChannelList.h>
#include <ImfOutputFile.h>
#include <half.h>
#endif

#ifndef _WIN32
    #pragma GCC diagnostic pop
#endif

using namespace Imf;
using namespace Imath;
#ifdef _WIN32
using namespace Imf_3_4;
#endif
using namespace EmberNs;
using namespace EmberCLns;
#include "EmberCommon.h"
using namespace EmberCommon;