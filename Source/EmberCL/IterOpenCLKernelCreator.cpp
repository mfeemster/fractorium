#include "EmberCLPch.h"
#include "IterOpenCLKernelCreator.h"

//#define STRAIGHT_RAND 1

namespace EmberCLns
{
/// <summary>
/// Constructor that sets up some basic entry point strings and creates
/// the zeroization kernel string since it requires no conditional inputs.
/// </summary>
template <typename T>
IterOpenCLKernelCreator<T>::IterOpenCLKernelCreator()
{
	m_ZeroizeKernel = CreateZeroizeKernelString();
	m_SumHistKernel = CreateSumHistKernelString();
}

/// <summary>
/// Accessors.
/// </summary>

template <typename T> const string& IterOpenCLKernelCreator<T>::ZeroizeKernel() const     { return m_ZeroizeKernel;     }
template <typename T> const string& IterOpenCLKernelCreator<T>::ZeroizeEntryPoint() const { return m_ZeroizeEntryPoint; }
template <typename T> const string& IterOpenCLKernelCreator<T>::SumHistKernel() const     { return m_SumHistKernel;     }
template <typename T> const string& IterOpenCLKernelCreator<T>::SumHistEntryPoint() const { return m_SumHistEntryPoint; }
template <typename T> const string& IterOpenCLKernelCreator<T>::IterEntryPoint() const    { return m_IterEntryPoint;    }
/// <summary>
/// Create the iteration kernel string using the Cuburn method.
/// Template argument expected to be float or double.
/// Pre Reg Post Formula
/// x			 trans = affine(inpoint)
///				 foreach prevar
///					tempin = trans
///					tempout = prevar(i, tempin)
///					trans = tempout
///				 outpoint = trans
///
/// x	x		 trans = affine(inpoint)
///				 foreach prevar
///					tempin = trans
///					tempout = prevar(i, tempin)
///					trans = tempout
///				 tempin = trans
///				 outpoint = 0
///				 foreach regvar
///					tempout = regvar(i, tempin)
///					outpoint += tempout
//
/// x	x	x
///				 trans = affine(inpoint)
///				 foreach prevar
///					tempin = trans
///					tempout = prevar(i, tempin)
///					trans = tempout
///				 tempin = trans
///				 outpoint = 0
///				 foreach regvar
///					tempout = regvar(i, tempin)
///					outpoint += tempout
///				 foreach postvar
///					tempin = outpoint
///					tempout = postvar(i, tempin)
///					outpoint = tempout
///
/// x		x
///				 trans = affine(inpoint)
///				 foreach prevar
///					tempin = trans
///					tempout = prevar(i, tempin)
///					trans = tempout
///				 outpoint = trans
///				 foreach postvar
///					tempin = outpoint
///					tempout = postvar(i, tempin)
///					outpoint = tempout
///
///		x
///				 trans = affine(inpoint)
///				 tempin = trans
///				 outpoint = 0
///				 foreach regvar
///					tempout = regvar(i, tempin)
///					outpoint += tempout
///
///		x	x
///				 trans = affine(inpoint)
///				 tempin = trans
///				 outpoint = 0
///				 foreach regvar
///					tempout = regvar(i, tempin)
///					outpoint += tempout
///				 foreach postvar
///					tempin = outpoint
///					tempout = postvar(i, tempin)
///					outpoint = tempout
///
///			x
///				 trans = affine(inpoint)
///				 outpoint = 0
///				 foreach postvar
///					tempin = outpoint
///					tempout = postvar(i, tempin)
///					outpoint = tempout
///
///	none		 trans = affine(inpoint)
///				 outpoint = 0
///
/// </summary>
/// <param name="ember">The ember to create the kernel string for</param>
/// <param name="params">The parametric variation #define string</param>
/// <param name="optAffine">True to optimize with a simple assignment when the pre affine transform is empty, else false. True is better for final renders, false for interactive to reduce repeated compilations.</param>
/// <param name="lockAccum">Whether to lock when accumulating to the histogram. This is only for debugging. Default: false.</param>
/// <param name="doAccum">Debugging parameter to include or omit accumulating to the histogram. Default: true.</param>
/// <returns>The kernel string</returns>
template <typename T>
string IterOpenCLKernelCreator<T>::CreateIterKernelString(const Ember<T>& ember, const string& parVarDefines, const string& globalSharedDefines, bool optAffine, bool lockAccum, bool doAccum)
{
	bool doublePrecision = typeid(T) == typeid(double);
	size_t i = 0, v, varIndex, varCount;
	ostringstream kernelIterBody, xformFuncs, os;
	vector<Variation<T>*> variations;
	xformFuncs << VariationStateString(ember);
	xformFuncs << parVarDefines << globalSharedDefines;
	ember.GetPresentVariations(variations);
	bool hasVarState = ember.GetVariationStateParamCount();

	for (auto var : variations)
		if (var)
			xformFuncs << var->OpenCLFuncsString();

	while (auto xform = ember.GetTotalXform(i))
	{
		bool needPrecalcSumSquares = false;
		bool needPrecalcSqrtSumSquares = false;
		bool needPrecalcAngles = false;
		bool needPrecalcAtanXY = false;
		bool needPrecalcAtanYX = false;
		bool hasPreReg = (xform->PreVariationCount() + xform->VariationCount()) > 0;
		v = varIndex = varCount = 0;
		xformFuncs <<
				   "void Xform" << i << "(__constant XformCL* xform, __constant real_t* parVars, __global real_t* globalShared, Point* inPoint, Point* outPoint, uint2* mwc, VariationState* varState)\n" <<
				   "{\n"
				   "	real_t transX, transY, transZ;\n"
				   "	real4 vIn, vOut = 0.0;\n";

		//Determine if any variations, regular, pre, or post need precalcs.
		while (Variation<T>* var = xform->GetVariation(v++))
		{
			needPrecalcSumSquares     |= var->NeedPrecalcSumSquares();
			needPrecalcSqrtSumSquares |= var->NeedPrecalcSqrtSumSquares();
			needPrecalcAngles         |= var->NeedPrecalcAngles();
			needPrecalcAtanXY         |= var->NeedPrecalcAtanXY();
			needPrecalcAtanYX         |= var->NeedPrecalcAtanYX();
		}

		if (needPrecalcSumSquares)
			xformFuncs << "\treal_t precalcSumSquares;\n";

		if (needPrecalcSqrtSumSquares)
			xformFuncs << "\treal_t precalcSqrtSumSquares;\n";

		if (needPrecalcAngles)
		{
			xformFuncs << "\treal_t precalcSina;\n";
			xformFuncs << "\treal_t precalcCosa;\n";
		}

		if (needPrecalcAtanXY)
			xformFuncs << "\treal_t precalcAtanxy;\n";

		if (needPrecalcAtanYX)
			xformFuncs << "\treal_t precalcAtanyx;\n";

#ifdef USEFMA
		xformFuncs << "\treal_t tempColor = outPoint->m_ColorX = fma(xform->m_OneMinusColorCache, inPoint->m_ColorX, xform->m_ColorSpeedCache);\n\n";
#else
		xformFuncs << "\treal_t tempColor = outPoint->m_ColorX = (xform->m_OneMinusColorCache * inPoint->m_ColorX + xform->m_ColorSpeedCache);\n\n";
#endif

		if (optAffine && xform->m_Affine.IsID())
		{
			xformFuncs <<
					   "	transX = inPoint->m_X;\n" <<
					   "	transY = inPoint->m_Y;\n";
		}
		else
		{
#ifdef USEFMA
			xformFuncs <<
					   "	transX = fma(xform->m_A, inPoint->m_X, fma(xform->m_B, inPoint->m_Y, xform->m_C));\n" <<
					   "	transY = fma(xform->m_D, inPoint->m_X, fma(xform->m_E, inPoint->m_Y, xform->m_F));\n";
#else
			xformFuncs <<
					   "	transX = xform->m_A * inPoint->m_X + (xform->m_B * inPoint->m_Y + xform->m_C);\n" <<
					   "	transY = xform->m_D * inPoint->m_X + (xform->m_E * inPoint->m_Y + xform->m_F);\n";
#endif
		}

		xformFuncs << "	transZ = inPoint->m_Z;\n";
		varCount = xform->PreVariationCount();

		if (hasPreReg)
		{
			if (varCount > 0)
			{
				xformFuncs << "\n\t//Apply each of the " << varCount << " pre variations in this xform.\n";

				//Output the code for each pre variation in this xform.
				for (varIndex = 0; varIndex < varCount; varIndex++)
				{
					if (Variation<T>* var = xform->GetVariation(varIndex))
					{
						xformFuncs << "\n\t//" << var->Name() << ".\n";
						xformFuncs << xform->ReadOpenCLString(eVariationType::VARTYPE_PRE) << "\n";
						xformFuncs << var->PrePostPrecalcOpenCLString();
						xformFuncs << var->OpenCLString() << "\n";
						xformFuncs << xform->WriteOpenCLString(eVariationType::VARTYPE_PRE, var->AssignType()) << "\n";
					}
				}
			}

			if (xform->VariationCount() > 0)
			{
				if (xform->NeedPrecalcSumSquares())
					xformFuncs << "\tprecalcSumSquares = SQR(transX) + SQR(transY);\n";

				if (xform->NeedPrecalcSqrtSumSquares())
					xformFuncs << "\tprecalcSqrtSumSquares = sqrt(precalcSumSquares);\n";

				if (xform->NeedPrecalcAngles())
				{
					xformFuncs << "\tprecalcCosa = transX / Zeps(precalcSqrtSumSquares);\n";
					xformFuncs << "\tprecalcSina = transY / Zeps(precalcSqrtSumSquares);\n";
				}

				if (xform->NeedPrecalcAtanXY())
					xformFuncs << "\tprecalcAtanxy = atan2(transX, transY);\n";

				if (xform->NeedPrecalcAtanYX())
					xformFuncs << "\tprecalcAtanyx = atan2(transY, transX);\n";

				xformFuncs << "\n\toutPoint->m_X = 0;";
				xformFuncs << "\n\toutPoint->m_Y = 0;";
				xformFuncs << "\n\toutPoint->m_Z = 0;\n";
				xformFuncs << "\n\t//Apply each of the " << xform->VariationCount() << " regular variations in this xform.\n\n";
				xformFuncs << xform->ReadOpenCLString(eVariationType::VARTYPE_REG);
				varCount += xform->VariationCount();

				//Output the code for each regular variation in this xform.
				for (; varIndex < varCount; varIndex++)
				{
					if (Variation<T>* var = xform->GetVariation(varIndex))
					{
						xformFuncs << "\n\t//" << var->Name() << ".\n"
								   << var->OpenCLString() << (varIndex == varCount - 1 ? "\n" : "\n\n")
								   << xform->WriteOpenCLString(eVariationType::VARTYPE_REG, eVariationAssignType::ASSIGNTYPE_SUM);
					}
				}
			}
			else
			{
				xformFuncs <<
						   "	outPoint->m_X = transX;\n"
						   "	outPoint->m_Y = transY;\n"
						   "	outPoint->m_Z = transZ;\n";
			}
		}
		else
		{
			xformFuncs <<
					   "	outPoint->m_X = 0;\n"
					   "	outPoint->m_Y = 0;\n"
					   "	outPoint->m_Z = 0;\n";
		}

		if (xform->PostVariationCount() > 0)
		{
			varCount += xform->PostVariationCount();
			xformFuncs << "\n\t//Apply each of the " << xform->PostVariationCount() << " post variations in this xform.\n";

			//Output the code for each post variation in this xform.
			for (; varIndex < varCount; varIndex++)
			{
				if (Variation<T>* var = xform->GetVariation(varIndex))
				{
					xformFuncs << "\n\t//" << var->Name() << ".\n";
					xformFuncs << xform->ReadOpenCLString(eVariationType::VARTYPE_POST) << "\n";
					xformFuncs << var->PrePostPrecalcOpenCLString();
					xformFuncs << var->OpenCLString() << "\n";
					xformFuncs << xform->WriteOpenCLString(eVariationType::VARTYPE_POST, var->AssignType()) << (varIndex == varCount - 1 ? "\n" : "\n\n");
				}
			}
		}

		if (xform->HasPost())
		{
			xformFuncs <<
					   "\n\t//Apply post affine transform.\n"
					   "\treal_t tempX = outPoint->m_X;\n"
					   "\n"
#ifdef USEFMA
					   "\toutPoint->m_X = fma(xform->m_PostA, tempX, fma(xform->m_PostB, outPoint->m_Y, xform->m_PostC));\n" <<
					   "\toutPoint->m_Y = fma(xform->m_PostD, tempX, fma(xform->m_PostE, outPoint->m_Y, xform->m_PostF));\n";
#else
					   "\toutPoint->m_X = (xform->m_PostA * tempX + (xform->m_PostB * outPoint->m_Y + xform->m_PostC));\n" <<
					   "\toutPoint->m_Y = (xform->m_PostD * tempX + (xform->m_PostE * outPoint->m_Y + xform->m_PostF));\n";
#endif
		}

#ifdef USEFMA
		xformFuncs << "\toutPoint->m_ColorX = fma(xform->m_DirectColor, (outPoint->m_ColorX - tempColor), tempColor);\n";
#else
		xformFuncs << "\toutPoint->m_ColorX = (xform->m_DirectColor * (outPoint->m_ColorX - tempColor) + tempColor);\n";
#endif
		xformFuncs << "\n";
		xformFuncs << "\tif (isnan(outPoint->m_ColorX))\n";
		xformFuncs << "\t	outPoint->m_ColorX = 0.0; \n";
		xformFuncs << "}\n"
				   << "\n";
		i++;
	}

	auto varStateString = VariationStateInitString(ember);
	os <<
	   ConstantDefinesString(doublePrecision) <<
	   GlobalFunctionsString(ember) <<
	   RandFunctionString <<
	   PointCLStructString <<
	   XformCLStructString <<
	   EmberCLStructString <<
	   UnionCLStructString <<
	   CarToRasCLStructString <<
	   CarToRasFunctionString;

	if (lockAccum)
		os << AtomicString;

	os <<
	   xformFuncs.str() <<
	   "__kernel void " << m_IterEntryPoint << "(\n" <<
	   "	uint iterCount,\n"
	   "	uint fuseCount,\n"
	   "	__global uint2* seeds,\n"
	   "	__constant EmberCL* ember,\n"
	   "	__constant XformCL* xforms,\n"
	   "	__constant real_t* parVars,\n"
	   "	__global real_t* globalShared,\n"
	   "	__global uchar* xformDistributions,\n"//Using uchar is quicker than uint. Can't be constant because the size can be too large to fit when using xaos.
	   "	__constant CarToRasCL* carToRas,\n"
	   "	__global real4reals_bucket* histogram,\n";

	if (hasVarState)
	{
		os <<
		   "	__global VariationState* varStates,\n";
	}

	os <<
	   "	uint histSize,\n"
	   "	__read_only image2d_t palette,\n"
	   "	__global Point* points"
#ifndef KNL_USE_GLOBAL_CONSEC
	   "\n"
#else
	   ",\n"
	   "	__global uchar* consec\n"
#endif
	   "\t)\n"
	   "{\n"
	   "	bool fuse, ok;\n"
	   "	uint threadIndex = INDEX_IN_BLOCK_2D;\n"
	   "	uint pointsIndex = INDEX_IN_GRID_2D;\n"
	   "	uint blockStartIndex = BLOCK_START_INDEX_IN_GRID_2D;\n"
	   "	uint blockStartThreadIndex = blockStartIndex + threadIndex;\n"
	   "	uint i, itersToDo;\n"
#ifndef KNL_USE_GLOBAL_CONSEC
	   "	uint consec = 0;\n"
#endif
	   //"	int badvals = 0;\n"
	   "	uint histIndex;\n"
	   "	real_t p00, p01;\n"
	   "	Point firstPoint, secondPoint, tempPoint;\n"
	   "	uint2 mwc = seeds[pointsIndex];\n"
	   "	float4 palColor1;\n"
	   "	int2 iPaletteCoord;\n"
	   "	const sampler_t paletteSampler = CLK_NORMALIZED_COORDS_FALSE |\n"//Coords from 0 to 255.
	   "		CLK_ADDRESS_CLAMP_TO_EDGE |\n"//Clamp to edge
	   "		CLK_FILTER_NEAREST;\n"//Don't interpolate
	   "	uint threadXY = (THREAD_ID_X + THREAD_ID_Y);\n"
	   "	uint threadXDivRows = (THREAD_ID_X / NWARPS);\n"
	   "	uint threadsMinus1 = NTHREADS - 1;\n"
	   "	VariationState varState;\n"
	   "\n";
#ifndef STRAIGHT_RAND

	if (ember.XformCount() > 1)
	{
		os <<
		   "	__local Point swap[NTHREADS];\n"
		   "	__local uint xfsel[NWARPS];\n";
	}

#endif
	os <<
	   "	iPaletteCoord.y = 0;\n"
	   "\n"
	   "	if (fuseCount > 0)\n"
	   "	{\n"
	   "		fuse = true;\n"
	   "		itersToDo = fuseCount;\n"
	   //Calling MwcNextFRange() twice is deliberate. The first call to mwc is not very random since it just does
	   //an xor. So it must be called twice to get it in a good random state.
	   "		firstPoint.m_X = MwcNextFRange(&mwc, -ember->m_RandPointRange, ember->m_RandPointRange);\n"
	   "		firstPoint.m_X = MwcNextFRange(&mwc, -ember->m_RandPointRange, ember->m_RandPointRange);\n"
	   "		firstPoint.m_Y = MwcNextFRange(&mwc, -ember->m_RandPointRange, ember->m_RandPointRange);\n"
	   "		firstPoint.m_Z = 0.0;\n"
	   "		firstPoint.m_ColorX = MwcNext01(&mwc);\n"
	   "		firstPoint.m_LastXfUsed = 0 - 1;\n";//This ensures the first iteration chooses from the unweighted distribution array, all subsequent iterations will choose from the weighted ones.
	//os <<
	// varStateString << '\n';
	os <<
	   "	}\n"
	   "	else\n"
	   "	{\n"
	   "		fuse = false;\n"
	   "		itersToDo = iterCount;\n"
	   "		firstPoint = points[blockStartThreadIndex];\n"
	   "	}\n"
	   "\n"
	   ;

	if (hasVarState)
	{
		os <<
		   "	varState = varStates[blockStartThreadIndex];\n";
	}

	//This is done once initially here and then again after each swap-sync in the main loop.
	//This along with the randomness that the point shuffle provides gives sufficient randomness
	//to produce results identical to those produced on the CPU.
	if (ember.XformCount() > 1)
	{
#ifndef STRAIGHT_RAND
		os <<
		   "\n"
		   "	if (THREAD_ID_Y == 0 && THREAD_ID_X < NWARPS)\n"
		   "		xfsel[THREAD_ID_X] = MwcNext(&mwc) & " << CHOOSE_XFORM_GRAIN_M1 << "u;\n"//It's faster to do the & here ahead of time than every time an xform is looked up to use inside the loop.
		   "\n";
#endif
	}
	else
	{
		os <<
		   "	secondPoint.m_LastXfUsed = 0;\n";
	}

	os <<
#ifndef STRAIGHT_RAND
	   "	barrier(CLK_LOCAL_MEM_FENCE);\n"
#endif
	   "\n"
	   "	for (i = 0; i < itersToDo; i++)\n"
	   "	{\n"
#ifndef KNL_USE_GLOBAL_CONSEC
	   "		consec = 0;\n"
#else
	   "		consec[blockStartThreadIndex] = 0;\n"
#endif
	   ;

	if (ember.XformCount() > 1)
	{
		//If xaos is present, the a hybrid of the cuburn method is used.
		//This makes each thread in a row pick the same offset into a distribution, using xfsel.
		//However, the distribution the offset is in, is determined by firstPoint.m_LastXfUsed.
		if (ember.XaosPresent())
		{
			os <<
#ifdef STRAIGHT_RAND
			   "		secondPoint.m_LastXfUsed = xformDistributions[(MwcNext(&mwc) & " << CHOOSE_XFORM_GRAIN_M1 << "u) + (" << CHOOSE_XFORM_GRAIN << "u * (firstPoint.m_LastXfUsed + 1u))];\n\n";
#else
			   "		secondPoint.m_LastXfUsed = xformDistributions[xfsel[THREAD_ID_Y] + (" << CHOOSE_XFORM_GRAIN << "u * (firstPoint.m_LastXfUsed + 1u))];\n\n";//Partial cuburn hybrid.
#endif
		}
		else
		{
			os <<
#ifdef STRAIGHT_RAND
			   "		secondPoint.m_LastXfUsed = xformDistributions[MwcNext(&mwc) & " << CHOOSE_XFORM_GRAIN_M1 << "u];\n\n";//For testing, using straight rand flam4/fractron style instead of cuburn.
#else
			   "		secondPoint.m_LastXfUsed = xformDistributions[xfsel[THREAD_ID_Y]];\n\n";
#endif
		}
	}

	os <<
	   "		do\n"
	   "		{\n";

	for (i = 0; i < ember.XformCount(); i++)
	{
		if (ember.XformCount() > 1)
		{
			if (i == 0)
			{
				os <<
				   "			switch (secondPoint.m_LastXfUsed)\n"
				   "			{\n";
			}

			os <<
			   "				case " << i << ":\n"
			   "				{\n" <<
			   "					Xform" << i << "(&(xforms[" << i << "]), parVars, globalShared, &firstPoint, &secondPoint, &mwc, &varState);\n" <<
			   "					break;\n"
			   "				}\n";

			if (i == ember.XformCount() - 1)
			{
				os <<
				   "			}\n";
			}
		}
		else
		{
			os << "			Xform0(&(xforms[0]), parVars, globalShared, &firstPoint, &secondPoint, &mwc, &varState);";
		}
	}

	os <<
	   "\n"
	   "			ok = !BadVal(secondPoint.m_X) && !BadVal(secondPoint.m_Y);\n"
	   //"			ok = !BadVal(secondPoint.m_X) && !BadVal(secondPoint.m_Y) && !isnan(secondPoint.m_Z);\n"
	   "\n"
	   "			if (!ok)\n"
	   "			{\n"
	   "				firstPoint.m_X = MwcNextFRange(&mwc, -ember->m_RandPointRange, ember->m_RandPointRange);\n"
	   "				firstPoint.m_Y = MwcNextFRange(&mwc, -ember->m_RandPointRange, ember->m_RandPointRange);\n"
	   "				firstPoint.m_Z = 0.0;\n"
	   "				firstPoint.m_ColorX = secondPoint.m_ColorX;\n"
#ifndef KNL_USE_GLOBAL_CONSEC
	   "				consec++;\n"
#else
	   "				consec[blockStartThreadIndex]++;\n"
#endif
	   //"				badvals++;\n"
	   "			}\n"
	   "		}\n"
#ifndef KNL_USE_GLOBAL_CONSEC
	   "		while (!ok && consec < 5);\n"
#else
	   "		while (!ok && consec[blockStartThreadIndex] < 5);\n"
#endif
	   "\n"
	   "		if (!ok)\n"
	   "		{\n"
	   "			secondPoint.m_X = MwcNextFRange(&mwc, -ember->m_RandPointRange, ember->m_RandPointRange);\n"
	   "			secondPoint.m_Y = MwcNextFRange(&mwc, -ember->m_RandPointRange, ember->m_RandPointRange);\n"
	   "			secondPoint.m_Z = 0.0;\n"
	   "		}\n";
#ifndef STRAIGHT_RAND

	if (ember.XformCount() > 1)
	{
		os <<
		   "\n"//Rotate points between threads. This is how randomization is achieved.
		   "		uint swr = threadXY + ((i & 1u) * threadXDivRows);\n"
		   "		uint sw = (swr * THREADS_PER_WARP + THREAD_ID_X) & threadsMinus1;\n"
		   "\n"
		   //Write to another thread's location.
		   "		swap[sw] = secondPoint;\n";
		os <<
		   "\n"
		   //Populate randomized xform index buffer with new random values.
		   "		if (THREAD_ID_Y == 0 && THREAD_ID_X < NWARPS)\n"
		   "			xfsel[THREAD_ID_X] = MwcNext(&mwc) & " << CHOOSE_XFORM_GRAIN_M1 << "u;\n"
		   "\n"
		   "		barrier(CLK_LOCAL_MEM_FENCE);\n"
		   //Another thread will have written to this thread's location, so read the new value and use it for accumulation below.
		   "		firstPoint = swap[threadIndex];\n";
	}
	else
	{
		os <<
		   "\n"
		   "		firstPoint = secondPoint;\n";
	}

#else
	os <<
	   "\n"
	   "		firstPoint = secondPoint;\n";//For testing, using straight rand flam4/fractron style instead of cuburn.
#endif
	os <<
	   "\n"
	   "		if (fuse)\n"
	   "		{\n";

	if (hasVarState && ember.XformCount() > 1)
	{
		os <<
		   "			varStates[blockStartIndex + sw] = varState;\n\n";
	}

	os <<
	   "			if (i >= fuseCount - 1)\n"
	   "			{\n"
	   "				i = 0;\n"
	   "				fuse = false;\n"
	   "				itersToDo = iterCount;\n";
#ifndef STRAIGHT_RAND

	if (ember.XformCount() > 1)
		os <<
		   "				barrier(CLK_LOCAL_MEM_FENCE);\n"//Sort of seems necessary, sort of doesn't. Makes no speed difference.
		   ;

#endif
	os <<
	   "			}\n"
	   ;

	if (hasVarState && ember.XformCount() > 1)
	{
		os <<
		   "\n"
		   "			barrier(CLK_GLOBAL_MEM_FENCE);\n"//Sort of seems necessary, sort of doesn't. Makes no speed difference.
		   "			varState = varStates[blockStartThreadIndex];"
		   ;
	}

	os <<
	   "\n"
	   "			continue;\n"
	   "		}\n"
	   "\n";

	if (ember.UseFinalXform())
	{
		size_t finalIndex = ember.TotalXformCount() - 1;
		//CPU takes an extra step here to preserve the opacity of the randomly selected xform, rather than the final xform's opacity.
		//The same thing takes place here automatically because secondPoint.m_LastXfUsed is used below to retrieve the opacity when accumulating.
		os <<
		   "		if ((xforms[" << finalIndex << "].m_Opacity == 1) || (MwcNext01(&mwc) < xforms[" << finalIndex << "].m_Opacity))\n"
		   "		{\n"
		   "			tempPoint.m_LastXfUsed = secondPoint.m_LastXfUsed;\n"
		   "			Xform" << finalIndex << "(&(xforms[" << finalIndex << "]), parVars, globalShared, &secondPoint, &tempPoint, &mwc, &varState);\n"
		   "			secondPoint = tempPoint;\n"
		   "		}\n"
		   "\n";
	}

	if (hasVarState && ember.XformCount() > 1)
	{
		os <<
		   "		varStates[blockStartIndex + sw] = varState;\n"
		   ;
	}

	os << CreateProjectionString(ember);

	if (doAccum)
	{
		if (optAffine && AnyZeroOpacity(ember))
		{
			os <<
			   "		if (xforms[secondPoint.m_LastXfUsed].m_Opacity != (real_t)(0.0))\n";
		}

		os <<
		   "		{\n";

		//Add this point to the appropriate location in the histogram.
		if (optAffine && ember.m_Rotate == 0)
		{
			os <<
			   "			if (CarToRasInBounds(carToRas, &secondPoint))\n"
			   "			{\n"
			   "				CarToRasConvertPointToSingle(carToRas, &secondPoint, &histIndex);\n";
		}
		else
		{
			os <<
			   "			p00 = secondPoint.m_X - ember->m_CenterX;\n"
			   "			p01 = secondPoint.m_Y - ember->m_CenterY;\n"
			   "			tempPoint.m_X = fma(p00, ember->m_RotA, fma(p01, ember->m_RotB, ember->m_CenterX));\n"
			   "			tempPoint.m_Y = fma(p00, ember->m_RotD, fma(p01, ember->m_RotE, ember->m_CenterY));\n"
			   "\n"
			   "			if (CarToRasInBounds(carToRas, &tempPoint))\n"
			   "			{\n"
			   "				CarToRasConvertPointToSingle(carToRas, &tempPoint, &histIndex);\n";
		}

		os <<
		   "\n"
		   "				if (histIndex < histSize)\n"//Provides an extra level of safety and makes no speed difference.
		   "				{\n";

		//Basic texture index interoplation does not produce identical results
		//to the CPU. So the code here must explicitly do the same thing and not
		//rely on the GPU texture coordinate lookup.
		if (ember.m_PaletteMode == ePaletteMode::PALETTE_LINEAR)
		{
			os <<
			   "					real_t colorIndexFrac;\n"
			   "					real_t colorIndex = secondPoint.m_ColorX * ember->m_Psm1;\n"
			   "					int intColorIndex;\n"
			   "					float4 palColor2;\n"
			   "\n"
			   "					if (colorIndex < 0)\n"
			   "					{\n"
			   "						intColorIndex = 0;\n"
			   "						colorIndexFrac = 0;\n"
			   "					}\n"
			   "					else if (colorIndex >= ember->m_Psm1)\n"
			   "					{\n"
			   "						intColorIndex = (int)ember->m_Psm2;\n"
			   "						colorIndexFrac = 1.0;\n"
			   "					}\n"
			   "					else\n"
			   "					{\n"
			   "						intColorIndex = (int)colorIndex;\n"
			   "						colorIndexFrac = colorIndex - intColorIndex;\n"//Interpolate between intColorIndex and intColorIndex + 1.
			   "					}\n"
			   "\n"
			   "					iPaletteCoord.x = intColorIndex;\n"//Palette operations are strictly float because OpenCL does not support dp64 textures.
			   "					palColor1 = read_imagef(palette, paletteSampler, iPaletteCoord);\n"
			   "					iPaletteCoord.x += 1;\n"
			   "					palColor2 = read_imagef(palette, paletteSampler, iPaletteCoord);\n"
#ifdef USEFMA
			   "					palColor1 = fma(palColor2, (float)colorIndexFrac, palColor1 * (1.0f - (float)colorIndexFrac));\n";//The 1.0f here *must* have the 'f' suffix at the end to compile.
#else
			   "					palColor1 = (palColor2 * (float)colorIndexFrac + (palColor1 * (1.0f - (float)colorIndexFrac)));\n";//The 1.0f here *must* have the 'f' suffix at the end to compile.
#endif
		}
		else if (ember.m_PaletteMode == ePaletteMode::PALETTE_STEP)
		{
			os <<
			   "					iPaletteCoord.x = (int)(secondPoint.m_ColorX * ember->m_Psm1);\n"
			   "					palColor1 = read_imagef(palette, paletteSampler, iPaletteCoord);\n";
		}

		if (lockAccum)
		{
			os <<
			   "					AtomicAdd(&(histogram[histIndex].m_Reals[0]), palColor1.x * (real_bucket_t)xforms[secondPoint.m_LastXfUsed].m_Opacity);\n"//Always apply opacity, even though it's usually 1.
			   "					AtomicAdd(&(histogram[histIndex].m_Reals[1]), palColor1.y * (real_bucket_t)xforms[secondPoint.m_LastXfUsed].m_Opacity);\n"
			   "					AtomicAdd(&(histogram[histIndex].m_Reals[2]), palColor1.z * (real_bucket_t)xforms[secondPoint.m_LastXfUsed].m_Opacity);\n"
			   "					AtomicAdd(&(histogram[histIndex].m_Reals[3]), palColor1.w * (real_bucket_t)xforms[secondPoint.m_LastXfUsed].m_Opacity);\n";
		}
		else
		{
			os <<
			   "					histogram[histIndex].m_Real4 += (palColor1 * (real_bucket_t)xforms[secondPoint.m_LastXfUsed].m_Opacity);\n";//real_bucket_t should always be float.
		}

		os <<
		   "				}\n"//histIndex < histSize.
		   "			}\n"//CarToRasInBounds.
		   "		}\n"//Opacity != 0.
		   "\n";
		os <<
		   "		barrier(CLK_GLOBAL_MEM_FENCE);\n";//Barrier every time, whether or not the point was in bounds, else artifacts will occur when doing strips.

		if (hasVarState && ember.XformCount() > 1)
		{
			os <<
			   "		varState = varStates[blockStartThreadIndex];\n"
			   ;
		}
	}

	os <<
	   "	}\n"//Main for loop.
	   "\n"
	   //At this point, iterating for this round is done, so write the final points back out
	   //to the global points buffer to be used as inputs for the next round. This preserves point trajectory
	   //between kernel calls.
#ifdef TEST_CL_BUFFERS//Use this to populate with test values and read back in EmberTester.
	   "	points[pointsIndex].m_X = MwcNextNeg1Pos1(&mwc);\n"
	   "	points[pointsIndex].m_Y = MwcNextNeg1Pos1(&mwc);\n"
	   "	points[pointsIndex].m_Z = MwcNextNeg1Pos1(&mwc);\n"
	   "	points[pointsIndex].m_ColorX = MwcNextNeg1Pos1(&mwc);\n"
#else
	   "	seeds[pointsIndex] = mwc;\n"
	   "	points[blockStartThreadIndex] = firstPoint;\n";

	if (hasVarState && ember.XformCount() == 1)
	{
		os <<
		   "	varStates[blockStartThreadIndex] = varState;\n";
	}

#endif
	   os <<
	   "	barrier(CLK_GLOBAL_MEM_FENCE);\n"
	   //"	printf(\"Global ID0: %d Global ID1: %d WorkDim: %d ThreadIndex: %d\\n\", get_global_id(0), get_global_id(1), get_work_dim(), blockStartThreadIndex);\n"
	   "}\n";
	return os.str();
}

/// <summary>
/// Return a string containing all of the global functions needed by the passed in ember.
/// </summary>
/// <param name="ember">The ember to create the global function strings from</param>
/// <returns>String of all global function names and bodies</returns>
template <typename T>
string IterOpenCLKernelCreator<T>::GlobalFunctionsString(const Ember<T>& ember)
{
	size_t i = 0, j;
	vector<string> funcNames;//Can't use a set here because they sort and we must preserve the insertion order due to nested function calls.
	ostringstream os;
	static string zeps = "Zeps";

	while (const auto xform = ember.GetTotalXform(i++))
	{
		size_t varCount = xform->TotalVariationCount();

		if (xform->NeedPrecalcAngles())
			if (!Contains(funcNames, zeps))
				funcNames.push_back(zeps);

		for (j = 0; j < varCount; j++)
		{
			if (const auto var = xform->GetVariation(j))
			{
				const auto names = var->OpenCLGlobalFuncNames();

				if (var->NeedPrecalcAngles())
					if (!Contains(funcNames, zeps))
						funcNames.push_back(zeps);

				for (auto& name : names)
					if (!Contains(funcNames, name))
						funcNames.push_back(name);
			}
		}
	}

	if (ember.ProjBits())
		if (!Contains(funcNames, zeps))
			funcNames.push_back(zeps);

	for (auto& funcName : funcNames)
		if (const auto text = m_FunctionMapper.GetGlobalFunc(funcName))
			os << *text << "\n";

	return os.str();
}
/// <summary>
/// Create an OpenCL string of #defines and a corresponding host side vector for variation weights and parametric variation values.
/// Parametric variations present a special problem in the iteration code.
/// The values can't be passed in with the array of other xform values because
/// the length of the parametric values is unknown.
/// This is solved by passing a separate buffer of values dedicated specifically
/// to parametric variations.
/// In OpenCL, a series of #define constants are declared which specify the indices in
/// the buffer where the various values are stored.
/// The possibility of a parametric variation type being present in multiple xforms is taken
/// into account by appending the xform index to the #define, thus making each unique.
/// The kernel creator then uses these to retrieve the values in the iteration code.
/// Example:
/// Xform1: Curl (curl_c1: 1.1, curl_c2: 2.2)
/// Xform2: Curl (curl_c1: 4.4, curl_c2: 5.5)
/// Xform3: Blob (blob_low: 1, blob_high: 2, blob_waves: 3)
///
/// Host vector to be passed as arg to the iter kernel call:
/// [1.1][2.2][4.4][5.5][1][2][3]
///
/// #defines in OpenCL to access the buffer:
///
/// #define CURL_C1_1 0
/// #define CURL_C2_1 1
/// #define CURL_C1_2 2
/// #define CURL_C2_2 3
/// #define BLOB_LOW_3 4
/// #define BLOB_HIGH_3 5
/// #define BLOB_WAVES_3 6
///
/// The variations use these #defines by first looking up the index of the
/// xform they belong to in the parent ember and generating the OpenCL string based on that
/// in their overridden OpenCLString() functions.
/// Note that variation weights are also included in this buffer and are looked up in a similar manner.
/// Template argument expected to be float or double.
/// </summary>
/// <param name="ember">The ember to create the values from</param>
/// <param name="params">The string,vector pair to store the values in</param>
/// <param name="doVals">True if the vector should be populated, else false. Default: true.</param>
/// <param name="doString">True if the string should be populated, else false. Default: true.</param>
template <typename T>
void IterOpenCLKernelCreator<T>::ParVarIndexDefines(const Ember<T>& ember, pair<string, vector<T>>& params, bool doVals, bool doString)
{
	size_t i = 0, size = 0;
	ostringstream os;

	if (doVals)
		params.second.clear();

	while (const auto xform = ember.GetTotalXform(i))
	{
		size_t j = 0;

		while (const auto var = xform->GetVariation(j))
		{
			if (doString)
				os << "#define WEIGHT_" << i << "_" << j << " " << size++ << "\n";//Uniquely identify the weight of this variation in this xform.

			if (doVals)
				params.second.push_back(var->m_Weight);

			if (const auto parVar = dynamic_cast<ParametricVariation<T>*>(var))
			{
				for (size_t k = 0; k < parVar->ParamCount(); k++)
				{
					if (!parVar->Params()[k].IsState())
					{
						if (doString)
							os << "#define " << ToUpper(parVar->Params()[k].Name()) << "_" << i << " " << size << "\n";//Uniquely identify this param in this variation in this xform.

						const auto elements = parVar->Params()[k].Size() / sizeof(T);

						if (doVals)
						{
                            for (size_t l = 0; l < elements; l++)
								params.second.push_back(*(parVar->Params()[k].Param() + l));
						}

						size += elements;
					}
				}
			}

			j++;
		}

		i++;
	}

	if (doString)
	{
		os << "\n";
		params.first = os.str();
	}
}
/// <summary>
/// Create an OpenCL string of #defines and a corresponding host side vector for globally shared data.
/// Certain variations, such as crackle and dc_perlin use static, read-only buffers of data.
/// These need to be created separate from the buffer of parametric variation values.
/// </summary>
/// <param name="ember">The ember to create the values from</param>
/// <param name="params">The string,vector pair to store the values in</param>
/// <param name="doVals">True if the vector should be populated, else false. Default: true.</param>
/// <param name="doString">True if the string should be populated, else false. Default: true.</param>
template <typename T>
void IterOpenCLKernelCreator<T>::SharedDataIndexDefines(const Ember<T>& ember, pair<string, vector<T>>& params, bool doVals, bool doString)
{
	size_t i = 0, j, offset = 0;
	string s;
	vector<string> dataNames;//Can't use a set here because they sort and we must preserve the insertion order due to nested function calls.
	ostringstream os;
	const auto varFuncs = VarFuncs<T>::Instance();

	if (doVals)
		params.second.clear();

	while (const auto xform = ember.GetTotalXform(i++))
	{
		size_t varCount = xform->TotalVariationCount();

		for (j = 0; j < varCount; j++)
		{
			if (const auto var = xform->GetVariation(j))
			{
				const auto names = var->OpenCLGlobalDataNames();

				for (auto& name : names)
				{
					if (!Contains(dataNames, name))
					{
						s = ToUpper(name);

						if (const auto dataInfo = varFuncs->GetSharedData(s))///Will contain a name, pointer to data, and size of the data in units of sizeof(T).
						{
							if (doString)
								os << "#define " << ToUpper(name) << " " << offset << '\n';

							if (doVals)
								params.second.insert(params.second.end(), dataInfo->first, dataInfo->first + dataInfo->second);

							dataNames.push_back(name);
							offset += dataInfo->second;
						}
					}
				}
			}
		}
	}

	if (doString)
	{
		os << "#define TOTAL_GLOBAL_SIZE_END " << offset << "\n\n";
		params.first = os.str();
	}
}
/// <summary>
/// Create the string needed for the struct whose values will change between each iteration.
/// This is only needed for variations whose state changes.
/// If none are present, the struct will be empty.
/// </summary>
/// <param name="ember">The ember to generate the variation state struct string for</param>
/// <returns>The variation state struct string</returns>
template <typename T>
string IterOpenCLKernelCreator<T>::VariationStateString(const Ember<T>& ember)
{
	size_t i = 0;
	ostringstream os;
	os << "typedef struct __attribute__ " ALIGN_CL " _VariationState\n{";

	while (const auto xform = ember.GetTotalXform(i++))
		for (size_t j = 0; j < xform->TotalVariationCount(); j++)
			if (const auto var = xform->GetVariation(j))
				os << var->StateOpenCLString();

	os << "\n} VariationState;\n\n";
	return os.str();
}
/// <summary>
/// Create the string needed for the initial state of the struct whose values will change between each iteration.
/// This is only needed for variations whose state changes.
/// If none are present, the returned init string will be empty.
/// This will be called at the beginning of each kernel.
/// </summary>
/// <param name="ember">The ember to generate the variation state struct init string for</param>
/// <returns>The variation state struct init string</returns>
template <typename T>
string IterOpenCLKernelCreator<T>::VariationStateInitString(const Ember<T>& ember)
{
	size_t i = 0;
	ostringstream os;

	while (const auto xform = ember.GetTotalXform(i++))
		for (size_t j = 0; j < xform->TotalVariationCount(); j++)
			if (const auto var = xform->GetVariation(j))
				os << var->StateInitOpenCLString();

	return os.str();
}

/// <summary>
/// Determine whether the passed in ember has at least one xform with an opacity of 0.
/// </summary>
/// <param name="ember">The first ember to compare</param>
/// <returns>True if at least one xform had an opacity of 0, else false</returns>
template <typename T>
bool IterOpenCLKernelCreator<T>::AnyZeroOpacity(const Ember<T>& ember)
{
	size_t i = 0;

	while (const auto xform = ember.GetXform(i++))
		if (xform->m_Opacity == 0)
			return true;

	return false;
}

/// <summary>
/// Determine whether the two embers passed in differ enough
/// to require a rebuild of the iteration code.
/// A rebuild is required if they differ in the following ways:
/// Xform count
/// Final xform presence
/// Xaos presence
/// Palette accumulation mode
/// Xform post affine presence
/// Variation count
/// Variation type
/// Template argument expected to be float or double.
/// </summary>
/// <param name="ember1">The first ember to compare</param>
/// <param name="ember2">The second ember to compare</param>
/// <param name="optAffine">True to optimize with a simple assignment when the pre affine transform is empty, else false. True is better for final renders, false for interactive to reduce repeated compilations.</param>
/// <returns>True if a rebuild is required, else false</returns>
template <typename T>
bool IterOpenCLKernelCreator<T>::IsBuildRequired(const Ember<T>& ember1, const Ember<T>& ember2, bool optAffine)
{
	size_t i, j, xformCount = ember1.TotalXformCount();
	static bool lastCompat = Compat::m_Compat;

	if (lastCompat != Compat::m_Compat)
	{
		lastCompat = Compat::m_Compat;
		return true;
	}

	if (xformCount != ember2.TotalXformCount())
		return true;

	if (ember1.UseFinalXform() != ember2.UseFinalXform())
		return true;

	if (ember1.XaosPresent() != ember2.XaosPresent())
		return true;

	if (ember1.m_PaletteMode != ember2.m_PaletteMode)
		return true;

	if (ember1.ProjBits() != ember2.ProjBits())
		return true;

	if (optAffine &&
			((ember1.m_Rotate == 0) ^ (ember2.m_Rotate == 0)))
		return true;

	if (optAffine &&
			(AnyZeroOpacity(ember1) != AnyZeroOpacity(ember2)))
		return true;

	for (i = 0; i < xformCount; i++)
	{
		const auto xform1 = ember1.GetTotalXform(i);
		const auto xform2 = ember2.GetTotalXform(i);
		const auto varCount = xform1->TotalVariationCount();

		if (optAffine && (xform1->m_Affine.IsID() != xform2->m_Affine.IsID()))
			return true;

		if (xform1->HasPost() != xform2->HasPost())
			return true;

		if (varCount != xform2->TotalVariationCount())
			return true;

		for (j = 0; j < varCount; j++)
			if (xform1->GetVariation(j)->VariationId() != xform2->GetVariation(j)->VariationId())
				return true;
	}

	return false;
}
/// <summary>
/// Create the zeroize kernel string.
/// OpenCL comes with no way to zeroize a buffer like memset()
/// would do on the CPU. So a special kernel must be ran to set a range
/// of memory addresses to zero.
/// </summary>
/// <returns>The kernel string</returns>
template <typename T>
string IterOpenCLKernelCreator<T>::CreateZeroizeKernelString() const
{
	ostringstream os;
	os <<
	   ConstantDefinesString(typeid(T) == typeid(double)) <<//Double precision doesn't matter here since it's not used.
	   "__kernel void " << m_ZeroizeEntryPoint << "(__global uchar* buffer, uint width, uint height)\n"
	   "{\n"
	   "	if (GLOBAL_ID_X >= width || GLOBAL_ID_Y >= height)\n"
	   "		return;\n"
	   "\n"
	   "	buffer[(GLOBAL_ID_Y * width) + GLOBAL_ID_X] = 0;\n"//Can't use INDEX_IN_GRID_2D here because the grid might be larger than the buffer to make even dimensions.
	   "	barrier(CLK_GLOBAL_MEM_FENCE);\n"//Just to be safe.
	   "}\n"
	   "\n";
	return os.str();
}
/// <summary>
/// Create the histogram summing kernel string.
/// This is used when running with multiple GPUs. It takes
/// two histograms present on a single device, source and dest,
/// and adds the values of source to dest.
/// It optionally sets all values of source to zero.
/// </summary>
/// <returns>The kernel string</returns>
template <typename T>
string IterOpenCLKernelCreator<T>::CreateSumHistKernelString() const
{
	ostringstream os;
	os <<
	   ConstantDefinesString(typeid(T) == typeid(double)) <<//Double precision doesn't matter here since it's not used.
	   "__kernel void " << m_SumHistEntryPoint << "(__global real4_bucket* source, __global real4_bucket* dest, uint width, uint height, uint clear)\n"
	   "{\n"
	   "	if (GLOBAL_ID_X >= width || GLOBAL_ID_Y >= height)\n"
	   "		return;\n"
	   "\n"
	   "	dest[(GLOBAL_ID_Y * width) + GLOBAL_ID_X] += source[(GLOBAL_ID_Y * width) + GLOBAL_ID_X];\n"//Can't use INDEX_IN_GRID_2D here because the grid might be larger than the buffer to make even dimensions.
	   "\n"
	   "	if (clear)\n"
	   "		source[(GLOBAL_ID_Y * width) + GLOBAL_ID_X] = 0;\n"
	   "\n"
	   "	barrier(CLK_GLOBAL_MEM_FENCE);\n"//Just to be safe.
	   "}\n"
	   "\n";
	return os.str();
}
/// <summary>
/// Create the string for 3D projection based on the 3D values of the ember.
/// Projection is done on the second point.
/// If any of these fields toggle between 0 and nonzero between runs, a recompile is triggered.
/// </summary>
/// <param name="ember">The ember to create the projection string for</param>
/// <returns>The kernel string</returns>
template <typename T>
string IterOpenCLKernelCreator<T>::CreateProjectionString(const Ember<T>& ember) const
{
	size_t projBits = ember.ProjBits();
	ostringstream os;

	if (projBits)
	{
		if (projBits & size_t(eProjBits::PROJBITS_BLUR))
		{
			if (projBits & size_t(eProjBits::PROJBITS_YAW))
			{
				os <<
				   "		real_t dsin, dcos;\n"
				   "		real_t t = MwcNext01(&mwc) * M_2PI;\n"
				   "		real_t z = secondPoint.m_Z - ember->m_CamZPos;\n"
				   "		real_t x = fma(ember->m_C00, secondPoint.m_X, ember->m_C10 * secondPoint.m_Y);\n"
				   "		real_t y = fma(ember->m_C01, secondPoint.m_X, fma(ember->m_C11, secondPoint.m_Y, ember->m_C21 * z));\n"
				   "\n"
				   "		z = fma(ember->m_C02, secondPoint.m_X, fma(ember->m_C12, secondPoint.m_Y, ember->m_C22 * z));\n"
				   "\n"
				   "		real_t zr = Zeps(1 - ember->m_CamPerspective * z);\n"
				   "		real_t prcx = x / carToRas->m_CarHalfX;\n"
				   "		real_t prcy = y / carToRas->m_CarHalfY;\n"
				   "		real_t dist = sqrt(SQR(prcx) + SQR(prcy)) * (real_t)(10.0);\n"
				   "		real_t scale = ember->m_BlurCurve != (real_t)(0.0) ? (SQR(dist) / (4 * ember->m_BlurCurve)) : (real_t)(1.0);\n"
				   "		real_t dr = MwcNext01(&mwc) * (ember->m_BlurCoef * scale) * z;\n"
				   "\n"
				   "		dsin = sin(t);\n"
				   "		dcos = cos(t);\n"
				   "\n"
				   "		secondPoint.m_X  = fma(dr, dcos, x) / zr;\n"
				   "		secondPoint.m_Y  = fma(dr, dsin, y) / zr;\n"
				   "		secondPoint.m_Z -= ember->m_CamZPos;\n";
			}
			else
			{
				os <<
				   "		real_t y, z, zr;\n"
				   "		real_t dsin, dcos;\n"
				   "		real_t t = MwcNext01(&mwc) * M_2PI;\n"
				   "\n"
				   "		z = secondPoint.m_Z - ember->m_CamZPos;\n"
				   "		y = fma(ember->m_C11, secondPoint.m_Y, ember->m_C21 * z);\n"
				   "		z = fma(ember->m_C12, secondPoint.m_Y, ember->m_C22 * z);\n"
				   "		zr = Zeps(1 - ember->m_CamPerspective * z);\n"
				   "\n"
				   "		dsin = sin(t);\n"
				   "		dcos = cos(t);\n"
				   "\n"
				   "		real_t prcx = secondPoint.m_X / carToRas->m_CarHalfX;\n"
				   "		real_t prcy = y / carToRas->m_CarHalfY;\n"
				   "		real_t dist = sqrt(SQR(prcx) + SQR(prcy)) * (real_t)(10.0);\n"
				   "		real_t scale = ember->m_BlurCurve != (real_t)(0.0) ? (SQR(dist) / (4 * ember->m_BlurCurve)) : (real_t)(1.0);\n"
				   "		real_t dr = MwcNext01(&mwc) * (ember->m_BlurCoef * scale) * z;\n"
				   "\n"
				   "		secondPoint.m_X = fma(dr, dcos, secondPoint.m_X) / zr;\n"
				   "		secondPoint.m_Y = fma(dr, dsin, y) / zr;\n"
				   "		secondPoint.m_Z -= ember->m_CamZPos;\n";
			}
		}
		else if ((projBits & size_t(eProjBits::PROJBITS_PITCH)) || (projBits & size_t(eProjBits::PROJBITS_YAW)))
		{
			if (projBits & size_t(eProjBits::PROJBITS_YAW))
			{
				os <<
				   "		real_t z  = secondPoint.m_Z - ember->m_CamZPos;\n"
				   "		real_t x  = fma(ember->m_C00, secondPoint.m_X, ember->m_C10 * secondPoint.m_Y);\n"
				   "		real_t y  = fma(ember->m_C01, secondPoint.m_X, fma(ember->m_C11, secondPoint.m_Y, ember->m_C21 * z));\n"
				   "		real_t zr = Zeps(1 - ember->m_CamPerspective * fma(ember->m_C02, secondPoint.m_X, fma(ember->m_C12, secondPoint.m_Y, ember->m_C22 * z)));\n"
				   "\n"
				   "		secondPoint.m_X = x / zr;\n"
				   "		secondPoint.m_Y = y / zr;\n"
				   "		secondPoint.m_Z -= ember->m_CamZPos;\n";
			}
			else
			{
				os <<
				   "		real_t z  = secondPoint.m_Z - ember->m_CamZPos;\n"
				   "		real_t y  = fma(ember->m_C11, secondPoint.m_Y, ember->m_C21 * z);\n"
				   "		real_t zr = Zeps(1 - ember->m_CamPerspective * fma(ember->m_C12, secondPoint.m_Y, ember->m_C22 * z));\n"
				   "\n"
				   "		secondPoint.m_X /= zr;\n"
				   "		secondPoint.m_Y  = y / zr;\n"
				   "		secondPoint.m_Z -= ember->m_CamZPos;\n";
			}
		}
		else
		{
			os <<
			   "		real_t zr = Zeps(1 - ember->m_CamPerspective * (secondPoint.m_Z - ember->m_CamZPos));\n"
			   "\n"
			   "		secondPoint.m_X /= zr;\n"
			   "		secondPoint.m_Y /= zr;\n"
			   "		secondPoint.m_Z -= ember->m_CamZPos;\n";
		}
	}

	return os.str();
}
template EMBERCL_API class IterOpenCLKernelCreator<float>;
#ifdef DO_DOUBLE
	template EMBERCL_API class IterOpenCLKernelCreator<double>;
#endif
}
