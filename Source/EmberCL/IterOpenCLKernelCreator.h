#pragma once

#include "EmberCLPch.h"
#include "EmberCLStructs.h"
#include "EmberCLFunctions.h"
#include "FunctionMapper.h"

/// <summary>
/// IterOpenCLKernelCreator class.
/// </summary>

namespace EmberCLns
{
/// <summary>
/// Class for creating the main iteration code in OpenCL.
/// It uses the Cuburn method of iterating where all conditionals
/// are stripped out and a specific kernel is compiled at run-time.
/// It uses a very sophisticated method for randomization that avoids
/// the problem of warp/wavefront divergence that would occur if every
/// thread selected a random xform to apply.
/// This only works with embers of type float, double is not supported.
/// </summary>
template <typename T>
class EMBERCL_API IterOpenCLKernelCreator
{
public:
	IterOpenCLKernelCreator();
	const string& ZeroizeKernel() const;
	const string& ZeroizeEntryPoint() const;
	const string& SumHistKernel() const;
	const string& SumHistEntryPoint() const;
	const string& IterEntryPoint() const;
	string CreateIterKernelString(const Ember<T>& ember, const string& parVarDefines, const string& globalSharedDefines, bool optAffine, size_t warpSize, bool lockAccum = false, bool doAccum = true);
	string GlobalFunctionsString(const Ember<T>& ember);
	static void ParVarIndexDefines(const Ember<T>& ember, pair<string, vector<T>>& params, bool doVals = true, bool doString = true);
	static void SharedDataIndexDefines(const Ember<T>& ember, pair<string, vector<T>>& params, bool doVals = true, bool doString = true);
	static string VariationStateString(const Ember<T>& ember);
	static string VariationStateInitString(const Ember<T>& ember);
	static bool AnyZeroOpacity(const Ember<T>& ember);
	static bool IsBuildRequired(const Ember<T>& ember1, const Ember<T>& ember2, bool optAffine);

private:
	string CreateZeroizeKernelString() const;
	string CreateSumHistKernelString() const;
	string CreateProjectionString(const Ember<T>& ember) const;

	string m_IterEntryPoint = "IterateKernel";
	string m_ZeroizeKernel;
	string m_ZeroizeEntryPoint = "ZeroizeKernel";
	string m_SumHistKernel;
	string m_SumHistEntryPoint = "SumHisteKernel";
	FunctionMapper m_FunctionMapper;
};

#ifdef OPEN_CL_TEST_AREA
typedef void (*KernelFuncPointer) (size_t gridWidth, size_t gridHeight, size_t blockWidth, size_t blockHeight,
								   size_t BLOCK_ID_X, size_t BLOCK_ID_Y, size_t THREAD_ID_X, size_t THREAD_ID_Y);

static void OpenCLSim(size_t gridWidth, size_t gridHeight, size_t blockWidth, size_t blockHeight, KernelFuncPointer func)
{
	cout << "OpenCLSim(): ";
	cout << "\n	Params: ";
	cout << "\n		gridW: " << gridWidth;
	cout << "\n		gridH: " << gridHeight;
	cout << "\n		blockW: " << blockWidth;
	cout << "\n		blockH: " << blockHeight;

	for (size_t i = 0; i < gridHeight; i += blockHeight)
	{
		for (size_t j = 0; j < gridWidth; j += blockWidth)
		{
			for (size_t k = 0; k < blockHeight; k++)
			{
				for (size_t l = 0; l < blockWidth; l++)
				{
					func(gridWidth, gridHeight, blockWidth, blockHeight, j / blockWidth, i / blockHeight, l, k);
				}
			}
		}
	}
}
#endif
}
