#ifdef _WIN32
	#pragma once
#endif

/// <summary>
/// Precompiled header file. Place all system includes here with appropriate #defines for different operating systems and compilers.
/// </summary>

//This special define is made to fix buggy OpenCL compilers on Mac.
//Rendering is much slower there for unknown reasons. Michel traced it down
//to the consec variable which keeps track of how many tries are needed to compute
//a point which is not a bad value. Strangely, keeping this as a local variable
//is slower than keeping it as an element in a global array.
//This is counterintuitive, and lends further weight to the idea that OpenCL on Mac
//is horribly broken.
#ifdef __APPLE__
	#define KNL_USE_GLOBAL_CONSEC
	#define OCL_USE_1_2_V
#endif

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN//Exclude rarely-used stuff from Windows headers.
#define _USE_MATH_DEFINES

#ifndef OCL_USE_1_2_V
	#define CL_TARGET_OPENCL_VERSION 300
	#define CL_HPP_TARGET_OPENCL_VERSION 300
	#define CL_HPP_MINIMUM_OPENCL_VERSION 300
#endif

#include "Timing.h"
#include "Renderer.h"

#ifdef  OCL_USE_1_2_V
	#include <CL/cl.hpp>
#else
	#include <CL/opencl.hpp>
#endif

#if defined(_WIN32)
	#pragma warning(disable : 4251; disable : 4661; disable : 4100)
	#include <windows.h>
	#include <SDKDDKVer.h>
	#include "GL/gl.h"
#elif defined(__APPLE__)
	#include <OpenGL/gl.h>
#else
	#include "GL/glx.h"
#endif

#include <utility>
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <iterator>
#include <time.h>
#include <unordered_map>

#ifdef _WIN32
	#if defined(BUILDING_EMBERCL)
		#define EMBERCL_API __declspec(dllexport)
	#else
		#define EMBERCL_API __declspec(dllimport)
	#endif
#else
	#define EMBERCL_API
#endif

using namespace std;
using namespace EmberNs;
//#define TEST_CL 1
//#define TEST_CL_BUFFERS 1
