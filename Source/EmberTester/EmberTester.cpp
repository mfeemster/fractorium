#include "EmberCommonPch.h"
#include "EmberTester.h"
#include "JpegUtils.h"
#include <queue>
#include <list>
#include <deque>

#ifdef __APPLE__
	#include <OpenEXR/ImfRgbaFile.h>
	#include <OpenEXR/ImfStringAttribute.h>
	#include <OpenEXR/half.h>
	//#include <Imath/half.h>
#else
	#include <ImfRgbaFile.h>
	#include <ImfStringAttribute.h>
	#include <half.h>
#endif
//#include <ImfStringAttribute.h>
//#include <ImfMatrixAttribute.h>
//#include <ImfArray.h>
//#include <ImfChannelList.h>

//#include "drawImage.h"
//
//#include <iostream>
//#include <algorithm>
//
//
//#include <ImfNamespace.h>
//
//namespace IMF = Imf;
//
using namespace Imf;
using namespace Imath;

/// <summary>
/// EmberTester is a scratch area used for on the fly testing.
/// It may become a more formalized automated testing system
/// in the future. At the moment it isn't expected to build or
/// give any useful insight into the workings of Ember.
/// </summary>

using namespace EmberNs;
using namespace EmberCommon;

//#define DO_NVIDIA 1

void writeRgba1(const char filename[],
				const Rgba* pixels,
				int width,
				int height)
{
	//
	// Write an RGBA image using class RgbaOutputFile.
	//
	//	- open the file
	//	- describe the memory layout of the pixels
	//	- store the pixels in the file
	//
	//auto& chl = file.header().channels();
	//chl.findChannel("R")->type = PixelType::FLOAT;
	//Header header(width, height);
	//header.channels().insert("R", Channel(IMF::FLOAT));
	//header.channels().insert("G", Channel(IMF::FLOAT));
	//header.channels().insert("B", Channel(IMF::FLOAT));
	////header.channels().insert("A", Channel(IMF::FLOAT));
	//FrameBuffer frameBuffer;
	//frameBuffer.insert("Z",					// name
	//	Slice(IMF::FLOAT,			// type
	//	(char *)zPixels,		// base
	//		sizeof(*zPixels) * 1,		// xStride
	//		sizeof(*zPixels) * width));	// yStride
	try
	{
		RgbaOutputFile file(filename, width, height, WRITE_RGBA);
		file.setFrameBuffer(pixels, 1, width);
		file.writePixels(height);
	}
	catch (std::exception e)
	{
		cout << e.what() << endl;
	}
}
/*
	template <typename T>
	void SaveFinalImage(Renderer<T, T>& renderer, vector<byte>& pixels, char* suffix)
	{
	long newSize;
	ostringstream os;
	os << ".\\BasicFlame_" << sizeof(T) << "_"  << suffix << ".bmp";
	BYTE* bgrBuf = ConvertRGBToBMPBuffer(pixels.data(), renderer.FinalRasW(), renderer.FinalRasH(), newSize);
	SaveBMP(ch, bgrBuf, renderer.FinalRasW(), renderer.FinalRasH(), newSize);
	delete [] bgrBuf;
	}
	template <typename T>
	Ember<T> CreateBasicEmber(uint width, uint height, uint ss, T quality, T centerX, T centerY, T rotate)
	{
	Timing t;
	QTIsaac<ISAAC_SIZE, ISAAC_INT> rand;
	//t.Tic();
	Ember<T> ember1;
	//t.Toc("TestBasicFlame() : Constructor()");
	//t.Tic();
	ember1.m_FinalRasW = width;
	ember1.m_FinalRasH = height;
	ember1.m_Supersample = ss;
	ember1.m_Quality = quality;
	ember1.m_CenterX = centerX;
	ember1.m_CenterY = centerY;
	ember1.m_Rotate = rotate;
	Xform<T> xform1(T(0.25), rand.Frand01<T>(), rand.Frand11<T>(), T(1), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
	Xform<T> xform2(T(0.25), rand.Frand01<T>(), rand.Frand11<T>(), T(1), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
	Xform<T> xform3(T(0.25), rand.Frand01<T>(), rand.Frand11<T>(), T(1), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
	Xform<T> xform4(T(0.25), rand.Frand01<T>(), rand.Frand11<T>(), T(1), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
	xform1.AddVariation(new SphericalVariation<T>());
	xform2.AddVariation(new SphericalVariation<T>());
	xform3.AddVariation(new SphericalVariation<T>());
	xform4.AddVariation(new SphericalVariation<T>());
	xform4.AddVariation(new BlobVariation<T>());
	ember1.AddXform(xform1);
	ember1.AddXform(xform2);
	ember1.AddXform(xform3);
	ember1.AddXform(xform4);
	//ember1.SetFinalXform(xform4);
	return ember1;
	}
*/

string GetEmberCLKernelString(Ember<float>& ember, bool iter, bool log, bool de, uint ss, bool accum)
{
	ostringstream os;
	IterOpenCLKernelCreator<float> iterCreator;
	DEOpenCLKernelCreator deCreator(false, false);
	FinalAccumOpenCLKernelCreator accumCreator(false);
	pair<string, vector<float>> pair, pair2;
	iterCreator.ParVarIndexDefines(ember, pair);
	iterCreator.SharedDataIndexDefines(ember, pair2);

	if (iter)
		os << "Iter kernel: \n" << iterCreator.CreateIterKernelString(ember, pair.first, pair2.first, true);

	if (log)
		os << "Log scale de kernel: \n" << deCreator.LogScaleAssignDEKernel();

	//if (de)
	//	os << "Gaussian DE kernel: \n" << deCreator.GaussianDEKernel(ss);
	//if (accum)
	//	os << "Accum kernel: \n" << accumCreator.FinalAccumKernelLateClipWithoutAlpha();
	return os.str();
}

template <typename T>
void MakeTestAllVarsRegPrePost(vector<Ember<T>>& embers)
{
	uint index = 0;
	ostringstream ss;
	auto varList = VariationList<T>::Instance();
	auto paletteList = PaletteList<float>::Instance();
	QTIsaac<ISAAC_SIZE, ISAAC_INT> rand;
	paletteList->Add("flam3-palettes.xml");
	Timing t;
	Ember<T> emberNoVars;
	emberNoVars.m_FinalRasW = 1024;
	emberNoVars.m_FinalRasH = 1024;
	emberNoVars.m_Quality = 100;
	Xform<T> xform1(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
	Xform<T> xform2(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
	Xform<T> xform3(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
	//Xform<T> xform4(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
	//Xform<T> xform5(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
	//Xform<T> xform6(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
	//Xform<T> xform7(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
	emberNoVars.AddXform(xform1);
	emberNoVars.AddXform(xform2);
	emberNoVars.AddXform(xform3);
	//emberNoVars.AddXform(xform4);
	//emberNoVars.AddXform(xform5);
	//emberNoVars.AddXform(xform6);
	//emberNoVars.AddXform(xform7);
	ss << "NoVars";
	emberNoVars.m_Name = ss.str();
	ss.str("");
	emberNoVars.m_Palette = *paletteList->GetPaletteByFilename(paletteList->m_DefaultFilename, 0);
	embers.push_back(emberNoVars);

	while (index < varList->RegSize())
	{
		/*  if (index != eVariationId::VAR_SYNTH)
			{
			index++;
			continue;
			}
		*/
		Ember<T> ember1;
		unique_ptr<Variation<T>> regVar(varList->GetVariationCopy(index, eVariationType::VARTYPE_REG));
		unique_ptr<Variation<T>> preVar(varList->GetVariationCopy("pre_" + regVar->Name()));
		unique_ptr<Variation<T>> postVar(varList->GetVariationCopy("post_" + regVar->Name()));
		ember1.m_FinalRasW = 1024;
		ember1.m_FinalRasH = 1024;
		ember1.m_Quality = 500;
		Xform<T> xform1(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
		Xform<T> xform2(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
		Xform<T> xform3(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
		Xform<T> xform4(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
		Xform<T> xform5(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
		Xform<T> xform6(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
		Xform<T> xform7(0.25f, rand.Frand01<T>(), rand.Frand11<T>(), 1, rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());

		if (preVar.get() && postVar.get())
		{
			xform1.AddVariation(preVar->Copy());
			xform2.AddVariation(regVar->Copy());
			xform3.AddVariation(postVar->Copy());
			xform4.AddVariation(preVar->Copy());
			xform4.AddVariation(regVar->Copy());
			xform5.AddVariation(preVar->Copy());
			xform5.AddVariation(postVar->Copy());
			xform6.AddVariation(regVar->Copy());
			xform6.AddVariation(postVar->Copy());
			xform7.AddVariation(preVar->Copy());
			xform7.AddVariation(regVar->Copy());
			xform7.AddVariation(postVar->Copy());
			ember1.AddXform(xform1);
			ember1.AddXform(xform2);
			ember1.AddXform(xform3);
			ember1.AddXform(xform4);
			ember1.AddXform(xform5);
			ember1.AddXform(xform6);
			ember1.AddXform(xform7);
		}
		else
		{
			xform1.AddVariation(regVar->Copy());
			xform2.AddVariation(regVar->Copy());
			xform3.AddVariation(regVar->Copy());
			xform4.AddVariation(regVar->Copy());
			ember1.AddXform(xform1);
			ember1.AddXform(xform2);
			ember1.AddXform(xform3);
			ember1.AddXform(xform4);
		}

		ss << index << "_" << regVar->Name();
		ember1.m_Name = ss.str();
		ss.str("");
		ember1.m_Palette = *paletteList->GetRandomPalette();
		index++;
		embers.push_back(ember1);
	}

	t.Toc("Creating embers for all possible variations");
}

void MakeTestAllVarsRegPrePostComboFile(const string& filename)
{
	EmberToXml<float> writer;
	vector<Ember<float>> embers;
	MakeTestAllVarsRegPrePost(embers);
	writer.Save(filename, embers, 0, true, true, false, false, false);
}

void TestAtomicAdd()
{
	size_t i;
	ostringstream os;
	OpenCLWrapper wrapper;
	vector<float> vec(32);
	os << ConstantDefinesString(false) << UnionCLStructString << endl;
	os <<
	   "void AtomicAdd(volatile __global float* source, const float operand)\n"
	   "{\n"
	   "	union\n"
	   "	{\n"
	   "		uint intVal;\n"
	   "		float floatVal;\n"
	   "	} newVal;\n"
	   "\n"
	   "	union\n"
	   "	{\n"
	   "		uint intVal;\n"
	   "		float floatVal;\n"
	   "	} prevVal;\n"
	   "\n"
	   "	do\n"
	   "	{\n"
	   "		prevVal.floatVal = *source;\n"
	   "		newVal.floatVal = prevVal.floatVal + operand;\n"
	   "	} while (atomic_cmpxchg((volatile __global uint*)source, prevVal.intVal, newVal.intVal) != prevVal.intVal);\n"
	   "}\n"
	   "\n"
	   "__kernel void MyKernel(\n"
	   "	__global float* buff,\n"
	   "	uint lockit\n"
	   "\t)\n"
	   "{\n"
	   "	uint index = THREAD_ID_X;\n"
	   "\n"
	   "	if (lockit)\n"
	   "	{\n"
	   "		AtomicAdd(&(buff[index]), (float)index * 0.54321);\n"
	   "	}\n"
	   "	else\n"
	   "	{\n"
	   "		buff[index] += (float)index * 0.54321;\n"
	   "	}\n"
	   "}\n";
	string program = os.str();
	string entry = "MyKernel";

	if (wrapper.Init(0, 0))
	{
		for (i = 0; i < vec.size(); i++)
			vec[i] = (i * 10.2234f);

		if (wrapper.AddAndWriteBuffer("buff", (void*)vec.data(), (uint)vec.size() * sizeof(vec[0])))
		{
			if (wrapper.AddProgram(entry, program, entry, false))
			{
				wrapper.SetBufferArg(0, 0, 0);
				wrapper.SetArg<uint>(0, 1, 0);

				if (wrapper.RunKernel(0,
									  32,//Total grid dims.
									  1,
									  1,
									  1,//Individual block dims.
									  1,
									  1))
				{
					wrapper.ReadBuffer(0, vec.data(), (uint)vec.size() * sizeof(vec[0]));
					cout << "Vector after unlocked add: " << endl;

					for (i = 0; i < vec.size(); i++)
					{
						cout << "vec[" << i << "] = " << vec[i] << endl;
					}

					for (i = 0; i < vec.size(); i++)
						vec[i] = (i * 10.2234f);

					wrapper.AddAndWriteBuffer("buff", (void*)vec.data(), (uint)vec.size() * sizeof(vec[0]));
					wrapper.SetBufferArg(0, 0, 0);
					wrapper.SetArg<uint>(0, 1, 1);

					if (wrapper.RunKernel(0,
										  32,//Total grid dims.
										  1,
										  1,
										  1,//Individual block dims.
										  1,
										  1))
					{
						wrapper.ReadBuffer(0, vec.data(), (uint)vec.size() * sizeof(vec[0]));
						cout << "\n\nVector after locked add: " << endl;

						for (i = 0; i < vec.size(); i++)
						{
							cout << "vec[" << i << "] = " << vec[i] << endl;
						}
					}
				}
			}
		}
	}
}

bool TestVarCounts()
{
	auto vlf(VariationList<float>::Instance());
#ifdef DO_DOUBLE
	auto vld(VariationList<double>::Instance());
	bool success((vlf->Size() == vld->Size()) && (vlf->Size() == size_t(eVariationId::LAST_VAR)));
#else
	bool success = true;
#endif
	uint start = et(eVariationId::VAR_ARCH);

	if (!success)
	{
		cout << "Variation list size " << vlf->Size() << " does not equal the max var ID enum " << et(eVariationId::LAST_VAR) << "." << endl;
	}

	for (; start < et(eVariationId::LAST_VAR); start++)
	{
		auto var = vlf->GetVariation((eVariationId)start);

		if (!var)
		{
			cout << "Variation " << start << " was not found." << endl;
			success = false;
		}
	}

	return success;
}

template <typename T>
bool TestVarUnique()
{
	bool success = true;
	auto vl = VariationList<T>::Instance();
	vector<eVariationId> ids;
	vector<string> names;
	ids.reserve(vl->Size());
	names.reserve(vl->Size());

	for (size_t i = 0; i < vl->Size(); i++)
	{
		auto var = vl->GetVariation(i);

		if (std::find(ids.begin(), ids.end(), var->VariationId()) != ids.end())
		{
			cout << "Variation " << var->Name() << " was a duplicate ID entry." << endl;
			success = false;
		}
		else
		{
			ids.push_back(var->VariationId());
		}

		if (std::find(names.begin(), names.end(), var->Name()) != names.end())
		{
			cout << "Variation " << var->Name() << " was a duplicate name entry." << endl;
			success = false;
		}
		else
		{
			names.push_back(var->Name());
		}
	}

	return success;
}

template <typename sT, typename dT>
bool TestVarPrecalcEqual(const Variation<sT>* var1, const Variation<dT>* var2)
{
	bool success = true;

	if (var1 && var2)
	{
		if (var1->NeedPrecalcSumSquares() != var2->NeedPrecalcSumSquares())
		{
			cout << "NeedPrecalcSumSquares value of " << var1->NeedPrecalcSumSquares() << " for variation " << var1->Name() << " != NeedPrecalcSumSquares value of " << var2->NeedPrecalcSumSquares() << " for variation " << var2->Name() << endl;
			success = false;
		}

		if (var1->NeedPrecalcSqrtSumSquares() != var2->NeedPrecalcSqrtSumSquares())
		{
			cout << "NeedPrecalcSqrtSumSquares value of " << var1->NeedPrecalcSqrtSumSquares() << " for variation " << var1->Name() << " != NeedPrecalcSqrtSumSquares value of " << var2->NeedPrecalcSqrtSumSquares() << " for variation " << var2->Name() << endl;
			success = false;
		}

		if (var1->NeedPrecalcAngles() != var2->NeedPrecalcAngles())
		{
			cout << "NeedPrecalcAngles value of " << var1->NeedPrecalcAngles() << " for variation " << var1->Name() << " != NeedPrecalcAngles value of " << var2->NeedPrecalcAngles() << " for variation " << var2->Name() << endl;
			success = false;
		}

		if (var1->NeedPrecalcAtanXY() != var2->NeedPrecalcAtanXY())
		{
			cout << "NeedPrecalcAtanXY value of " << var1->NeedPrecalcAtanXY() << " for variation " << var1->Name() << " != NeedPrecalcAtanXY value of " << var2->NeedPrecalcAtanXY() << " for variation " << var2->Name() << endl;
			success = false;
		}

		if (var1->NeedPrecalcAtanYX() != var2->NeedPrecalcAtanYX())
		{
			cout << "NeedPrecalcAtanYX value of " << var1->NeedPrecalcAtanYX() << " for variation " << var1->Name() << " != NeedPrecalcAtanYX value of " << var2->NeedPrecalcAtanYX() << " for variation " << var2->Name() << endl;
			success = false;
		}
	}

	return success;
}

template <typename sT, typename dT>
bool TestVarEqual(const Variation<sT>* var1, const Variation<dT>* var2)
{
	bool success = true;

	if (!var1 || !var2)
	{
		cout << "Variations were null." << endl;
		return false;
	}

	if (var1->VariationId() != var2->VariationId())
	{
		cout << "Variation IDs were not equal: " << et(var1->VariationId()) << " != " << et(var2->VariationId()) << endl;
		success = false;
	}

	if (var1->VarType() != var2->VarType())
	{
		cout << "Variation types were not equal: " << et(var1->VarType()) << " != " << et(var2->VarType()) << endl;
		success = false;
	}

	if (var1->Name() != var2->Name())
	{
		cout << "Variation names were not equal: " << var1->Name() << " != " << var2->Name() << endl;
		success = false;
	}

	if (var1->Prefix() != var2->Prefix())
	{
		cout << "Variation prefixes were not equal: " << var1->Prefix() << " != " << var2->Prefix() << endl;
		success = false;
	}

	if (!TestVarPrecalcEqual<sT, dT>(var1, var2))
	{
		cout << "Variation precalcs were not equal: " << var1->Name() << " and " << var2->Name() << "." << endl;
		success = false;
	}

	auto parVar1 = dynamic_cast<const ParametricVariation<sT>*>(var1);
	auto parVar2 = dynamic_cast<const ParametricVariation<dT>*>(var2);

	if (parVar1 && parVar2)
	{
		if (parVar1->ParamCount() != parVar2->ParamCount())
		{
			cout << "Variation ParamCount were not equal: " << parVar1->ParamCount() << " != " << parVar2->ParamCount() << endl;
			success = false;
		}

		vector<ParamWithName<sT>> params1 = parVar1->ParamsVec();
		vector<ParamWithName<dT>> params2 = parVar2->ParamsVec();

		for (size_t i = 0; i < params1.size(); i++)
		{
			if (params1[i].Name() != params2[i].Name())
			{
				cout << "Param Names were not equal: " << params1[i].Name() << " != " << params2[i].Name() << endl;
				success = false;
			}

			if (params1[i].Type() != params2[i].Type())
			{
				cout << "Param " << params1[i].Name() << " Types were not equal: " << params1[i].Type() << " != " << params2[i].Type() << endl;
				success = false;
			}

			if (params1[i].IsPrecalc() != params2[i].IsPrecalc())
			{
				cout << "Param " << params1[i].Name() << " IsPrecalc were not equal: " << params1[i].IsPrecalc() << " != " << params2[i].IsPrecalc() << endl;
				success = false;
			}

			if (!IsClose<sT>(params1[i].Def(), (sT)params2[i].Def()))
			{
				cout << "Param " << params1[i].Name() << " Def were not equal: " << params1[i].Def() << " != " << params2[i].Def() << endl;
				success = false;
			}

			if (typeid(sT) == typeid(dT))//Min and max can be different for float and double.
			{
				if (!IsClose<sT>(params1[i].Min(), (sT)params2[i].Min()))
				{
					cout << "Param " << params1[i].Name() << " Min were not equal: " << params1[i].Min() << " != " << params2[i].Min() << endl;
					success = false;
				}

				if (!IsClose<sT>(params1[i].Max(), (sT)params2[i].Max()))
				{
					cout << "Param " << params1[i].Name() << " Max were not equal: " << params1[i].Max() << " != " << params2[i].Max() << endl;
					success = false;
				}
			}

			if (!params1[i].IsState() && !params2[i].IsState())//Don't compare state params, they can be different if set to random vals.
			{
				if (!IsClose<sT>(params1[i].ParamVal(), (sT)params2[i].ParamVal(), sT(1e-4)))
				{
					cout << "Param " << params1[i].Name() << " Vals were not equal: " << params1[i].ParamVal() << " != " << params2[i].ParamVal() << endl;
					success = false;
				}
			}
		}
	}

	return success;
}

bool TestVarPrePostNames()
{
	bool success = true;
	auto vlf(VariationList<float>::Instance());

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);
		string name = var->Name();

		if (var->VarType() == eVariationType::VARTYPE_REG)
		{
			if (name.find("pre_") == 0)
			{
				cout << "Regular variation " << name << " must not start with pre_." << endl;
				success = false;
			}

			if (name.find("post_") == 0)
			{
				cout << "Regular variation " << name << " must not start with post_." << endl;
				success = false;
			}
		}
		else if (var->VarType() == eVariationType::VARTYPE_PRE)
		{
			if (name.find("pre_") != 0)
			{
				cout << "Pre variation " << name << " must start with pre_." << endl;
				success = false;
			}
		}
		else if (var->VarType() == eVariationType::VARTYPE_POST)
		{
			if (name.find("post_") != 0)
			{
				cout << "Post variation " << name << " must start with post_." << endl;
				success = false;
			}
		}
		else
		{
			cout << "Invalid variation type." << endl;
			success = false;
			break;
		}

		if (auto parVar = dynamic_cast<ParametricVariation<float>*>(const_cast<Variation<float>*>(var)))
		{
			vector<ParamWithName<float>> params = parVar->ParamsVec();

			for (size_t p = 0; p < params.size(); p++)
			{
				if (params[p].Name().find(name.c_str()) != 0)
				{
					cout << "Param " << params[p].Name() << " must start with " << name << endl;
					success = false;
				}
			}
		}
	}

	return success;
}

template <typename sT, typename dT>
bool TestVarCopy()
{
	bool success = true;
	auto vlf(VariationList<sT>::Instance());

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);
		Variation<dT>* destVar = NULL;
		unique_ptr<Variation<sT>> copyVar(var->Copy());//Test Copy().

		if (!TestVarEqual<sT, sT>(var, copyVar.get()))
		{
			cout << "Variations " << var->Name() << "<" << typeid(sT).name() << "> and " << copyVar->Name() << "<" << typeid(sT).name() << "> (same template type) were not equal after Copy()." << endl;
			success = false;
		}

		var->Copy(destVar);//Test Copy(var*);
		unique_ptr<Variation<dT>> destPtr(destVar);//Just for deletion.

		if (!TestVarEqual<sT, dT>(var, destPtr.get()))
		{
			cout << "Variations " << var->Name() << "<" << typeid(sT).name() << "> and " << destPtr->Name() << "<" << typeid(dT).name() << "> (different template types) were not equal after Copy(Variation<T>*)." << endl;
			success = false;
		}
	}

	return success;
}

bool TestParVars()
{
	bool success = true;
	auto vlf(VariationList<float>::Instance());

	for (size_t i = 0; i < vlf->ParametricSize(); i++)
	{
		if (auto parVar = vlf->GetParametricVariation(i))
		{
			if (parVar->ParamCount() < 1)
			{
				cout << "Parametric variation " << parVar->Name() << " does not have any parameters." << endl;
				success = false;
			}

			vector<string> names;
			vector<float*> addresses;
			auto params = parVar->Params();
			names.reserve(parVar->ParamCount());
			addresses.reserve(parVar->ParamCount());

			for (size_t j = 0; j < parVar->ParamCount(); j++)
			{
				if (std::find(names.begin(), names.end(), params[j].Name()) != names.end())
				{
					cout << "Param " << params[j].Name() << " for variation " << parVar->Name() << " was a duplicate name entry." << endl;
					success = false;
				}
				else
				{
					names.push_back(params[j].Name());
				}

				if (std::find(addresses.begin(), addresses.end(), params[j].Param()) != addresses.end())
				{
					cout << "Param address" << params[j].Param() << " for variation " << parVar->Name() << " was a duplicate name entry." << endl;
					success = false;
				}
				else
				{
					addresses.push_back(params[j].Param());
				}
			}
		}
		else
		{
			cout << "Parametric variation at index " << i << " was NULL." << endl;
			success = false;
		}
	}

	return success;
}

bool TestVarRegPrePost()
{
	bool success = true;
	auto vlf(VariationList<float>::Instance());

	for (size_t i = 0; i < vlf->RegSize(); i++)
	{
		auto regVar = vlf->GetVariation(i, eVariationType::VARTYPE_REG);

		if (regVar)
		{
			if (regVar->Name().find("dc_") != 0)
			{
				string name = regVar->Name();
				auto preVar = vlf->GetVariation("pre_" + name);
				auto postVar = vlf->GetVariation("post_" + name);

				if (!preVar)
				{
					cout << "Pre variation equivalent of " << name << " could not be found." << endl;
					success = false;
				}

				if (!postVar)
				{
					cout << "Post variation equivalent of " << name << " could not be found." << endl;
					success = false;
				}

				if (preVar)
				{
					if (!TestVarPrecalcEqual<float, float>(regVar, preVar))
					{
						cout << "Regular and pre variation precalc test failed for " << regVar->Name() << " and " << preVar->Name() << "." << endl;
						success = false;
					}
				}

				if (postVar)
				{
					if (!TestVarPrecalcEqual<float, float>(regVar, postVar))
					{
						cout << "Regular and post variation precalc test failed for " << regVar->Name() << " and " << postVar->Name() << "." << endl;
						success = false;
					}
				}
			}
		}
		else
		{
			cout << "Regular variation " << i << " was NULL." << endl;
			success = false;
		}
	}

	return success;
}

bool TestVarPrecalcUsedCL()
{
	bool success = true;
	auto vlf(VariationList<float>::Instance());

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);
		string s = var->OpenCLString();

		if (var->NeedPrecalcAngles())
		{
			if (s.find("precalcSina") == string::npos)
			{
				cout << "Variation " << var->Name() << " needed precalcSina, but it wasn't found in the OpenCL string." << endl;
				success = false;
			}

			if (s.find("precalcCosa") == string::npos)
			{
				cout << "Variation " << var->Name() << " needed precalcCosa, but it wasn't found in the OpenCL string." << endl;
				success = false;
			}
		}
		else
		{
			if (s.find("precalcSina") != string::npos)
			{
				cout << "Variation " << var->Name() << " didn't need precalcSina, but it was found in the OpenCL string." << endl;
				success = false;
			}

			if (s.find("precalcCosa") != string::npos)
			{
				cout << "Variation " << var->Name() << " didn't need precalcCosa, but it was found in the OpenCL string." << endl;
				success = false;
			}

			if (var->NeedPrecalcSqrtSumSquares())
			{
				if (s.find("precalcSqrtSumSquares") == string::npos)
				{
					cout << "Variation " << var->Name() << " needed precalcSqrtSumSquares, but it wasn't found in the OpenCL string." << endl;
					success = false;
				}
			}
			else
			{
				if (s.find("precalcSqrtSumSquares") != string::npos)
				{
					cout << "Variation " << var->Name() << " didn't need precalcSqrtSumSquares, but it was found in the OpenCL string." << endl;
					success = false;
				}

				if (var->NeedPrecalcSumSquares())
				{
					if (s.find("precalcSumSquares") == string::npos)
					{
						cout << "Variation " << var->Name() << " needed precalcSumSquares, but it wasn't found in the OpenCL string." << endl;
						success = false;
					}
				}
				else
				{
					if (s.find("precalcSumSquares") != string::npos)
					{
						cout << "Variation " << var->Name() << " didn't need precalcSumSquares, but it was found in the OpenCL string." << endl;
						success = false;
					}
				}
			}
		}

		if (var->NeedPrecalcSumSquares())
		{
			if (s.find("SQR(vIn.x) + SQR(vIn.y)") != string::npos || s.find("vIn.x * vIn.x + vIn.y * vIn.y") != string::npos)
			{
				cout << "Variation " << var->Name() << " needed precalcSumSquares, but is not using it properly." << endl;
				success = false;
			}
		}
		else
		{
			if (s.find("precalcSumSquares") != string::npos)
			{
				cout << "Variation " << var->Name() << " didn't need precalcSumSquares, but it was found in the OpenCL string." << endl;
				success = false;
			}

			if (s.find("SQR(vIn.x) + SQR(vIn.y)") != string::npos || s.find("vIn.x * vIn.x + vIn.y * vIn.y") != string::npos)
			{
				cout << "Variation " << var->Name() << " did not specify precalcSumSquares, but could benefit from it." << endl;
				success = false;
			}
		}

		if (var->NeedPrecalcSqrtSumSquares())
		{
			if (s.find("sqrt(SQR(vIn.x) + SQR(vIn.y))") != string::npos || s.find("sqrt(vIn.x * vIn.x + vIn.y * vIn.y)") != string::npos)
			{
				cout << "Variation " << var->Name() << " needed precalcSqrtSumSquares, but is not using it properly." << endl;
				success = false;
			}
		}
		else
		{
			if (s.find("precalcSqrtSumSquares") != string::npos)
			{
				cout << "Variation " << var->Name() << " didn't need precalcSqrtSumSquares, but it was found in the OpenCL string." << endl;
				success = false;
			}

			if (s.find("sqrt(SQR(vIn.x) + SQR(vIn.y))") != string::npos || s.find("sqrt(vIn.x * vIn.x + vIn.y * vIn.y)") != string::npos)
			{
				cout << "Variation " << var->Name() << " did not specify precalcSqrtSumSquares, but could benefit from it." << endl;
				success = false;
			}
		}

		if (var->NeedPrecalcAtanXY())
		{
			if (s.find("precalcAtanxy") == string::npos)
			{
				cout << "Variation " << var->Name() << " needed precalcAtanxy, but it wasn't found in the OpenCL string." << endl;
				success = false;
			}
		}
		else
		{
			if (s.find("precalcAtanxy") != string::npos)
			{
				cout << "Variation " << var->Name() << " didn't need precalcAtanxy, but it was found in the OpenCL string." << endl;
				success = false;
			}

			if (s.find("atan2(vIn.x, vIn.y)") != string::npos)
			{
				cout << "Variation " << var->Name() << " did not specify precalcAtanxy, but could benefit from it." << endl;
				success = false;
			}
		}

		if (var->NeedPrecalcAtanYX())
		{
			if (s.find("precalcAtanyx") == string::npos)
			{
				cout << "Variation " << var->Name() << " needed precalcAtanyx, but it wasn't found in the OpenCL string." << endl;
				success = false;
			}
		}
		else
		{
			if (s.find("precalcAtanyx") != string::npos)
			{
				cout << "Variation " << var->Name() << " didn't need precalcAtanyx, but it was found in the OpenCL string." << endl;
				success = false;
			}

			if (s.find("atan2(vIn.y, vIn.x)") != string::npos)
			{
				cout << "Variation " << var->Name() << " did not specify precalcAtanyx, but could benefit from it." << endl;
				success = false;
			}
		}
	}

	return success;
}

bool TestVarAssignTypes()
{
	bool success = true;
	auto vlf(VariationList<float>::Instance());
	vector<string> vset, vsum;
	vset.push_back("vIn.x");
	vset.push_back("vIn.y");
	vset.push_back("vIn.z");
	vset.push_back("precalcSumSquares");
	vset.push_back("precalcSqrtSumSquares");
	vset.push_back("precalcSina");
	vset.push_back("precalcCosa");
	vset.push_back("precalcAtanxy");
	vset.push_back("precalcAtanyx");
	vsum.push_back("vIn.x");
	vsum.push_back("vIn.y");
	//vsum.push_back("vIn.z");
	vsum.push_back("precalcSumSquares");
	vsum.push_back("precalcSqrtSumSquares");
	vsum.push_back("precalcSina");
	vsum.push_back("precalcCosa");
	vsum.push_back("precalcAtanxy");
	vsum.push_back("precalcAtanyx");

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);
		string s = var->OpenCLString();

		//Only test pre and post. The assign type for regular is ignored, and will always be summed.
		if (var->VarType() != eVariationType::VARTYPE_REG)
		{
			if (var->AssignType() == eVariationAssignType::ASSIGNTYPE_SET)
			{
				if (!SearchVar(var, vset, false))
				{
					cout << "Variation " << var->Name() << " had an assign type of SET, but did not use its input points. It should have an assign type of SUM." << endl;
					success = false;
				}
			}
			else if (var->AssignType() == eVariationAssignType::ASSIGNTYPE_SUM)
			{
				if (SearchVar(var, vsum, false))
				{
					cout << "Variation " << var->Name() << " had an assign type of SUM, but used its input points. It should have an assign type of SET." << endl;
					success = false;
				}
			}
			else
			{
				cout << "Variation " << var->Name() << " had an invalid assign type of " << et(var->AssignType()) << endl;
			}
		}
	}

	return success;
}

bool TestVarAssignVals()
{
	bool success = true;
	auto vlf(VariationList<float>::Instance());
	vector<string> xout, yout, zout;
	xout.push_back("vOut.x =");
	xout.push_back("vOut.x +=");
	xout.push_back("vOut.x -=");
	xout.push_back("vOut.x *=");
	xout.push_back("vOut.x /=");
	yout.push_back("vOut.y =");
	yout.push_back("vOut.y +=");
	yout.push_back("vOut.y -=");
	yout.push_back("vOut.y *=");
	yout.push_back("vOut.y /=");
	zout.push_back("vOut.z =");
	zout.push_back("vOut.z +=");
	zout.push_back("vOut.z -=");
	zout.push_back("vOut.z *=");
	zout.push_back("vOut.z /=");

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);

		if (!SearchVar(var, xout, false))
		{
			cout << "Variation " << var->Name() << " did not set its x output point. If unused, at least pass through or set to 0." << endl;
			success = false;
		}

		if (!SearchVar(var, yout, false))
		{
			cout << "Variation " << var->Name() << " did not set its y output point. If unused, at least pass through or set to 0." << endl;
			success = false;
		}

		if (!SearchVar(var, zout, false))
		{
			cout << "Variation " << var->Name() << " did not set its z output point. If unused, at least pass through or set to 0." << endl;
			success = false;
		}
	}

	return success;
}

void FindFmaCandidates()
{
	auto vlf(VariationList<float>::Instance());

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);
		auto cl = var->OpenCLFuncsString() + "\n" + var->StateInitOpenCLString() + "\n" + var->OpenCLString();
		auto splits = Split(cl, '\n');

		for (auto& split : splits)
		{
			if ((split.find("*") != string::npos || split.find("SQR(") != string::npos || split.find("Sqr(") != string::npos) && split.find("+") != string::npos)
				cout << "Variation " << var->Name() << " is a potential fma() candidate because of line:\n\t" << split << endl;
		}
	}
}

void FindFmaImplemented()
{
	auto vlf(VariationList<float>::Instance());

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);
		auto cl = var->OpenCLFuncsString() + "\n" + var->StateInitOpenCLString() + "\n" + var->OpenCLString();
		auto splits = Split(cl, '\n');

		for (auto& split : splits)
		{
			if ((split.find("fma(") != string::npos))
				cout << "Variation " << var->Name() << " has been implemented with fma():\n\t" << split << endl;
		}
	}
}

bool TestZepsFloor()
{
	bool success = true;
	auto vlf(VariationList<float>::Instance());
	vector<string> zeps;
	zeps.push_back("Zeps(floor");

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);

		if (SearchVar(var, zeps, false))
		{
			cout << "Variation " << var->Name() << " contains Zeps(floor()). This is fine for the GPU, but ensure the CPU uses Zeps<T>((T)Floor<T>())." << endl;
			success = false;
		}
	}

	return success;
}

bool TestConstants()
{
	bool success = true;
	auto vlf(VariationList<float>::Instance());
	vector<string> stringVec;
	stringVec.push_back("2 * M_PI");
	stringVec.push_back("2*M_PI");
	stringVec.push_back("M_PI*2");
	stringVec.push_back("M_PI * 2");
	stringVec.push_back("2 * MPI");
	stringVec.push_back("2*MPI");
	stringVec.push_back("MPI*2");
	stringVec.push_back("MPI * 2");

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);

		if (SearchVar(var, stringVec, false))
		{
			cout << "Variation " << var->Name() << " should be using M_2PI." << endl;
			success = false;
		}
	}

	stringVec.clear();
	stringVec.push_back("3 * M_PI");
	stringVec.push_back("3*M_PI");
	stringVec.push_back("M_PI*3");
	stringVec.push_back("M_PI * 3");
	//stringVec.push_back("3 * MPI");//Gives a false positive for prose3d.
	stringVec.push_back("3*MPI");
	stringVec.push_back("MPI*3");
	stringVec.push_back("MPI * 3");

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);

		if (SearchVar(var, stringVec, false))
		{
			cout << "Variation " << var->Name() << " should be using M_3PI." << endl;
			success = false;
		}
	}

	stringVec.clear();
	stringVec.push_back("M_PI");

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);

		if (SearchVar(var, stringVec, false))
		{
			cout << "Variation " << var->Name() << " is using M_PI, but should be using MPI." << endl;
			success = false;
		}
	}

	stringVec.clear();
	stringVec.push_back("M_2_PI");

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);

		if (SearchVar(var, stringVec, false))
		{
			cout << "Variation " << var->Name() << " is using M_2_PI, but should be using M2PI." << endl;
			success = false;
		}
	}

	stringVec.clear();
	stringVec.push_back("M_1_PI");

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);

		if (SearchVar(var, stringVec, false))
		{
			cout << "Variation " << var->Name() << " is using M_1_PI, but should be using M1PI." << endl;
			success = false;
		}
	}

	stringVec.clear();
	stringVec.push_back("M_PI_4");

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);

		if (SearchVar(var, stringVec, false))
		{
			cout << "Variation " << var->Name() << " is using M_PI_4, but should be using MPI4." << endl;
			success = false;
		}
	}

	stringVec.clear();
	stringVec.push_back("M_PI_2");

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);

		if (SearchVar(var, stringVec, false))
		{
			cout << "Variation " << var->Name() << " is using M_PI_2, but should be using MPI2." << endl;
			success = false;
		}
	}

	return success;
}

void TestFuncs()
{
	auto vlf(VariationList<float>::Instance());
	vector<string> stringVec;
	stringVec.push_back("M_E");
	stringVec.push_back("M_LOG2E");
	stringVec.push_back("M_LOG10E");
	stringVec.push_back("M_LN2");
	stringVec.push_back("M_LN10");
	stringVec.push_back("M_PI");
	stringVec.push_back("M_PI_2");
	stringVec.push_back("M_PI_4");
	stringVec.push_back("M_1_PI");
	stringVec.push_back("M_2_PI");
	stringVec.push_back("M_2_SQRTPI");
	stringVec.push_back("M_SQRT2");
	stringVec.push_back("M_SQRT1_2");
	//stringVec.push_back("M_2PI");
	//stringVec.push_back("M_3PI");
	//stringVec.push_back("DEG_2_RAD");

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);

		if (SearchVar(var, stringVec, false))
		{
			cout << var->Name() << endl;
		}
	}
}

bool TestGlobalFuncs()
{
	bool success = true;
	auto vlf(VariationList<float>::Instance());
	vector<string> funcs;
	FunctionMapper mapper;
	auto funcmap = mapper.GetGlobalMapCopy();

	for (size_t i = 0; i < vlf->Size(); i++)
	{
		auto var = vlf->GetVariation(i);
		funcs = var->OpenCLGlobalFuncNames();
		auto localfuncs = var->OpenCLFuncsString();
		Ember<float> ember;
		Xform<float> xf;
		xf.AddVariation(var->Copy());
		ember.AddXform(xf);
		auto kernel = GetEmberCLKernelString(ember, true, false, false, 1u, false);

		for (auto& func : funcs)//Test if the functions the variation says it requires actually exist.
		{
			if (!mapper.GetGlobalFunc(func))
			{
				cout << "Variation " << var->Name() << " used unknown global funcion " << func << endl;
				success = false;
			}
			else
			{
				//cout << "Variation " << var->Name() << " used valid global funcion " << func << endl;
			}
		}

		auto str = var->OpenCLString();
		auto vec = var->OpenCLGlobalFuncNames();

		for (auto& func : funcmap)//Test if the functions the variation uses are actually included in those it says are required.
		{
			if (Find(str, func.first + "("))
			{
				if (!Contains(vec, func.first))
				{
					cout << "Variation " << var->Name() << " used global funcion " << func.first << ", but it's not found in its global func name vector: " << endl;

					for (auto& v : vec)
						cout << v << endl;
				}
			}
		}

		//Test whether the global functions the variations purports to need are actually used.
		for (auto& v : vec)
		{
			bool found = false;

			for (auto& v2 : vec)//Test if the functions the variation uses possibly use this function. It can be the case sometimes where a variation does not use it directly, but its global functions do.
			{
				if (v != v2)
				{
					auto it = funcmap.find(v2);

					if (it != funcmap.end())
					{
						if (Find(it->second, v + "("))
						{
							found = true;
							break;
						}
					}
				}
			}

			if (!found && Find(str, v + "("))
			{
				found = true;
			}

			if (!found && Find(localfuncs, v + "("))
			{
				found = true;
			}

			if (!found)
			{
				cout << "Variation " << var->Name() << " purported to require the usage of global function " << v << ", but it's not found in its OpenCL function string:\n" /*<< kernel*/ << endl;
			}
		}
	}

	return success;
}

void PrintAllVars()
{
	uint i = 0;
	auto vlf(VariationList<float>::Instance());

	while (auto var = vlf->GetVariation(i++))
		cout << var->Name() << endl;
}

void TestXformsInOutPoints()
{
	uint index = 0;
	auto varList = VariationList<float>::Instance();
	auto paletteList = PaletteList<float>::Instance();
	QTIsaac<ISAAC_SIZE, ISAAC_INT> rand;
	paletteList->Add("flam3-palettes.xml");

	while (index < varList->RegSize())
	{
		vector<Xform<float>> xforms;
		unique_ptr<Variation<float>> regVar(varList->GetVariationCopy(index, eVariationType::VARTYPE_REG));
		string s = regVar->OpenCLString() + regVar->OpenCLFuncsString();

		if (s.find("MwcNext") == string::npos)
		{
			unique_ptr<Variation<float>> preVar(varList->GetVariationCopy("pre_" + regVar->Name()));
			unique_ptr<Variation<float>> postVar(varList->GetVariationCopy("post_" + regVar->Name()));
			Xform<float> xform0(0.25f, rand.Frand01<float>(), rand.Frand11<float>(), 1, rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>());
			Xform<float> xform1(0.25f, rand.Frand01<float>(), rand.Frand11<float>(), 1, rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>());
			Xform<float> xform2(0.25f, rand.Frand01<float>(), rand.Frand11<float>(), 1, rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>());
			Xform<float> xform3(0.25f, rand.Frand01<float>(), rand.Frand11<float>(), 1, rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>());
			Xform<float> xform4(0.25f, rand.Frand01<float>(), rand.Frand11<float>(), 1, rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>());
			Xform<float> xform5(0.25f, rand.Frand01<float>(), rand.Frand11<float>(), 1, rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>());
			Xform<float> xform6(0.25f, rand.Frand01<float>(), rand.Frand11<float>(), 1, rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>());
			Xform<float> xform7(0.25f, rand.Frand01<float>(), rand.Frand11<float>(), 1, rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>(), rand.Frand11<float>());

			if (preVar.get() && postVar.get())
			{
				xform1.AddVariation(preVar->Copy());
				xform2.AddVariation(regVar->Copy());
				xform3.AddVariation(postVar->Copy());
				xform4.AddVariation(preVar->Copy());
				xform4.AddVariation(regVar->Copy());
				xform5.AddVariation(preVar->Copy());
				xform5.AddVariation(postVar->Copy());
				xform6.AddVariation(regVar->Copy());
				xform6.AddVariation(postVar->Copy());
				xform7.AddVariation(preVar->Copy());
				xform7.AddVariation(regVar->Copy());
				xform7.AddVariation(postVar->Copy());
				xforms.push_back(xform0);
				xforms.push_back(xform1);
				xforms.push_back(xform2);
				xforms.push_back(xform3);
				xforms.push_back(xform4);
				xforms.push_back(xform5);
				xforms.push_back(xform6);
				xforms.push_back(xform7);
			}
			else
			{
				xform1.AddVariation(regVar->Copy());
				xforms.push_back(xform0);
				xforms.push_back(xform1);
			}

			for (size_t i = 0; i < xforms.size(); i++)
			{
				bool badVals = false;
				Point<float> orig;
				orig.m_X = rand.Frand11<float>();
				orig.m_Y = rand.Frand11<float>();
				orig.m_Z = rand.Frand11<float>();
				orig.m_ColorX = rand.Frand01<float>();
				orig.m_Opacity = rand.Frand01<float>();
				Point<float> p1 = orig, p2 = orig, p3;
				xforms[i].Apply(&p1, &p1, rand);
				xforms[i].Apply(&p2, &p3, rand);
				badVals |= (p1.m_X != p1.m_X);
				badVals |= (p1.m_Y != p1.m_Y);
				badVals |= (p1.m_Z != p1.m_Z);
				badVals |= (p1.m_ColorX != p1.m_ColorX);
				badVals |= (p1.m_Opacity != p1.m_Opacity);
				badVals |= (p3.m_X != p3.m_X);
				badVals |= (p3.m_Y != p3.m_Y);
				badVals |= (p3.m_Z != p3.m_Z);
				badVals |= (p3.m_ColorX != p3.m_ColorX);
				badVals |= (p3.m_Opacity != p3.m_Opacity);

				if (badVals)
					cout << "Variation " << regVar->Name() << ": Bad value detected" << endl;

				if (!badVals)
				{
					if (p1.m_X != p3.m_X)
						cout << "Variation " << regVar->Name() << ": p1.m_X " << p1.m_X << " != p3.m_X " << p3.m_X << endl;

					if (p1.m_Y != p3.m_Y)
						cout << "Variation " << regVar->Name() << ": p1.m_Y " << p1.m_Y << " != p3.m_Y " << p3.m_Y << endl;

					if (p1.m_Z != p3.m_Z)
						cout << "Variation " << regVar->Name() << ": p1.m_Z " << p1.m_Z << " != p3.m_Z " << p3.m_Z << endl;

					if (p1.m_ColorX != p3.m_ColorX)
						cout << "Variation " << regVar->Name() << ": p1.m_ColorX " << p1.m_ColorX << " != p3.m_ColorX " << p3.m_ColorX << endl;

					if (p1.m_Opacity != p3.m_Opacity)
						cout << "Variation " << regVar->Name() << ": p1.m_Opacity " << p1.m_Opacity << " != p3.m_Opacity " << p3.m_Opacity << endl;
				}
			}
		}

		index++;
	}
}

static int SortPairByTime(const pair<string, double>& a, pair<string, double>& b)
{
	return a.second < b.second;
}

template <typename T>
void TestVarTime()
{
	int i = 0, iters = 10;
	Timing t;
	VariationList<T> vlf;
	IteratorHelper<T> helper;
	QTIsaac<ISAAC_SIZE, ISAAC_INT> rand;
	vector<pair<string, double>> times;
	times.reserve(vlf->RegSize());

	while (i < vlf->RegSize())
	{
		double sum = 0;
		Xform<T> xform;
		Variation<T>* var = vlf->GetVariationCopy(i, eVariationType::VARTYPE_REG);
		xform.AddVariation(var);

		for (int iter = 0; iter < iters; iter++)
		{
			Point<T> p;
			xform.m_Affine.A(rand.Frand<T>(-5, 5));
			xform.m_Affine.B(rand.Frand<T>(-5, 5));
			xform.m_Affine.C(rand.Frand<T>(-5, 5));
			xform.m_Affine.D(rand.Frand<T>(-5, 5));
			xform.m_Affine.E(rand.Frand<T>(-5, 5));
			xform.m_Affine.F(rand.Frand<T>(-5, 5));
			p.m_X = rand.Frand<T>(-20, 20);
			p.m_Y = rand.Frand<T>(-20, 20);
			p.m_Z = rand.Frand<T>(-20, 20);
			helper.In.x = helper.m_TransX = (xform.m_Affine.A() * p.m_X) + (xform.m_Affine.B() * p.m_Y) + xform.m_Affine.C();
			helper.In.y = helper.m_TransY = (xform.m_Affine.D() * p.m_X) + (xform.m_Affine.E() * p.m_Y) + xform.m_Affine.F();
			helper.In.z = helper.m_TransZ = p.m_Z;
			helper.m_Color.x = p.m_ColorX = rand.Frand01<T>();
			p.m_Opacity = rand.Frand01<T>();
			helper.m_PrecalcSumSquares = SQR(helper.m_TransX) + SQR(helper.m_TransY);
			helper.m_PrecalcSqrtSumSquares = std::sqrt(helper.m_PrecalcSumSquares);
			helper.m_PrecalcSina = helper.m_TransX / helper.m_PrecalcSqrtSumSquares;
			helper.m_PrecalcCosa = helper.m_TransY / helper.m_PrecalcSqrtSumSquares;
			helper.m_PrecalcAtanxy = std::atan2(helper.m_TransX, helper.m_TransY);
			helper.m_PrecalcAtanyx = std::atan2(helper.m_TransY, helper.m_TransX);
			var->Random(rand);
			t.Tic();
			var->Func(helper, p, rand);
			sum += t.Toc();
		}

		i++;
		times.push_back(pair<string, double>(var->Name(), sum / iters));
	}

	std::sort(times.begin(), times.end(), &SortPairByTime);
	//forr (auto& p : times) cout << p.first << "\t" << p.second << "" << endl;
}

void TestCasting()
{
	vector<string> stringVec;
	auto varList = VariationList<float>::Instance();
	auto& vars = varList->AllVars();
	stringVec.push_back("T(");
	stringVec.push_back(".0f");
	stringVec.push_back(".1f");
	stringVec.push_back(".2f");
	stringVec.push_back(".3f");
	stringVec.push_back(".4f");
	stringVec.push_back(".5f");
	stringVec.push_back(".6f");
	stringVec.push_back(".7f");
	stringVec.push_back(".8f");
	stringVec.push_back(".9f");
	auto varVec = FindVarsWith<float>(vars, stringVec);

	for (auto& it : varVec)
	{
		cout << "Variation " << it->Name() << " contained an improper float cast." << endl;
	}
}

template <typename T>
void TestOperations()
{
	vector<string> stringVec;
	auto varList = VariationList<T>::Instance();
	auto& vars = varList->AllVars();
	//stringVec.push_back("%");
	//varVec = FindVarsWith<T>(Vec);
	//
	//for (size_t i = 0; i < varVec.size(); i++)
	//{
	//	cout << "Variation " << varVec[i]->Name() << " contained a modulo operation. Ensure its right hand operand is not zero." << endl;
	//}
	//
	//stringVec.clear();
	//ClearVec<Variation<T>>(varVec);
	stringVec.push_back("MwcNext(mwc) %");
	stringVec.push_back("MwcNext(mwc)%");
	auto varVec = FindVarsWith<T>(vars, stringVec);

	for (size_t i = 0; i < varVec.size(); i++)
	{
		cout << "Variation " << varVec[i]->Name() << " contained MwcNext(mwc) %. Use MwcNextRange() instead." << endl;
	}
}

void TestArbitrary()
{
	vector<string> stringVec, withoutVec, emptyVec;
	auto varList = VariationList<float>::Instance();
	auto& vars = varList->AllVars();
	stringVec.push_back("switch");
	//stringVec.push_back("if (!");
	withoutVec.push_back("=");
	withoutVec.push_back("<");
	withoutVec.push_back(">");
	//stringVec.push_back(" = vIn.x - ");
	//stringVec.push_back(" = vIn.y - ");
	//stringVec.push_back("sqrt(");
	//stringVec.push_back("atan2(");
	//stringVec.push_back("sin(");
	//stringVec.push_back("cos(");
	//stringVec.push_back("sincos(");
	auto varVecWith = FindVarsWith<float>(vars, stringVec, true, true);
	auto varVecWithout = FindVarsWithWithout<float>(vars, emptyVec, withoutVec);

	for (auto& it : varVecWith)
		cout << "Variation " << it->Name() << " contained the desired 'with' strings." << endl;

	if (varVecWithout.size() > 0)
	{
		cout << "There shouldn't be any variations that don't contain =, < or >. They were: " << endl;
		
		for (auto& it : varVecWithout)
			cout << "\t" << it->Name() << endl;
	}
	else
		cout << "All variations contained =, < or >." << endl;
}

template <typename T>
void TestVarsSimilar()
{
	int i = 0, compIndex = 0, iters = 10;
	Timing t;
	VariationList<T> vlf;
	IteratorHelper<T> helper;
	QTIsaac<ISAAC_SIZE, ISAAC_INT> rand;
	vector<pair<string, double>> diffs;
	diffs.reserve(vlf->RegSize());

	while (i < vlf->RegSize())
	{
		double diff = 0, highest = TMAX;
		Xform<T> xform;
		Variation<T>* var = vlf->GetVariationCopy(i, eVariationType::VARTYPE_REG);
		pair<string, double> match("", TMAX);
		compIndex = 0;
		xform.AddVariation(var);

		while (compIndex < vlf->RegSize())
		{
			if (compIndex == i)
			{
				compIndex++;
				continue;
			}

			double sum = 0, xdiff = 0, ydiff = 0, zdiff = 0;
			Xform<T> xformComp;
			Variation<T>* varComp = vlf->GetVariationCopy(compIndex, eVariationType::VARTYPE_REG);
			xformComp.AddVariation(varComp);
			ParametricVariation<T>* parVar = dynamic_cast<ParametricVariation<T>*>(var);
			ParametricVariation<T>* parVarComp = dynamic_cast<ParametricVariation<T>*>(varComp);

			for (int iter = 0; iter < iters; iter++)
			{
				Point<T> p, pComp;
				xform.m_Affine.A(rand.Frand<T>(-5, 5));
				xform.m_Affine.B(rand.Frand<T>(-5, 5));
				xform.m_Affine.C(rand.Frand<T>(-5, 5));
				xform.m_Affine.D(rand.Frand<T>(-5, 5));
				xform.m_Affine.E(rand.Frand<T>(-5, 5));
				xform.m_Affine.F(rand.Frand<T>(-5, 5));
				xformComp.m_Affine = xform.m_Affine;
				p.m_X = rand.Frand<T>(-20, 20);
				p.m_Y = rand.Frand<T>(-20, 20);
				p.m_Z = rand.Frand<T>(-20, 20);
				helper.In.x = helper.m_TransX = (xform.m_Affine.A() * p.m_X) + (xform.m_Affine.B() * p.m_Y) + xform.m_Affine.C();
				helper.In.y = helper.m_TransY = (xform.m_Affine.D() * p.m_X) + (xform.m_Affine.E() * p.m_Y) + xform.m_Affine.F();
				helper.In.z = helper.m_TransZ = p.m_Z;
				helper.m_Color.x = p.m_ColorX = rand.Frand01<T>();
				p.m_Opacity = rand.Frand01<T>();
				pComp = p;
				helper.m_PrecalcSumSquares = SQR(helper.m_TransX) + SQR(helper.m_TransY);
				helper.m_PrecalcSqrtSumSquares = std::sqrt(helper.m_PrecalcSumSquares);
				helper.m_PrecalcSina = helper.m_TransX / helper.m_PrecalcSqrtSumSquares;
				helper.m_PrecalcCosa = helper.m_TransY / helper.m_PrecalcSqrtSumSquares;
				helper.m_PrecalcAtanxy = std::atan2(helper.m_TransX, helper.m_TransY);
				helper.m_PrecalcAtanyx = std::atan2(helper.m_TransY, helper.m_TransX);

				if (parVar)
				{
					for (size_t v = 0; v < parVar->ParamCount(); v++)
						parVar->SetParamVal(v, (T)iter);
				}

				if (parVarComp)
				{
					for (size_t v = 0; v < parVarComp->ParamCount(); v++)
						parVarComp->SetParamVal(v, (T)iter);
				}

				//For debugging.
				if (var->VariationId() == eVariationId::VAR_BWRAPS && varComp->VariationId() == eVariationId::VAR_ECLIPSE)
				{
					//cout << "Break." << endl;
				}

				helper.Out = v4T(0);
				var->m_Weight = T(iter + 1);
				var->Precalc();
				var->Func(helper, p, rand);
				v4T varOut = helper.Out;
				helper.Out = v4T(0);
				varComp->m_Weight = T(iter + 1);
				varComp->Precalc();
				varComp->Func(helper, pComp, rand);
				v4T varCompOut = helper.Out;
				xdiff += std::abs(varOut.x - varCompOut.x);
				ydiff += std::abs(varOut.y - varCompOut.y);
				zdiff += std::abs(varOut.z - varCompOut.z);
			}

			sum = (xdiff + ydiff + zdiff) / iters;

			if (sum < highest)
			{
				match.first = varComp->Name();
				match.second = highest = sum;
			}

			compIndex++;
		}

		if (match.second < 0.001)
			cout << "The closest match to variation " << var->Name() << " is " << match.first << " with a total difference of " << match.second << endl;

		i++;
		//times.push_back(pair<string, double>(var->Name(), sum / iters));
	}

	//std::sort(times.begin(), times.end(), &SortPairByTime);
}

#ifdef TEST_CL

template <typename T>
bool TestAllVarsCLBuild(size_t platform, size_t device, bool printSuccess = true)
{
	bool success = true;
	vector<Ember<T>> embers;
	QTIsaac<ISAAC_SIZE, ISAAC_INT> rand;
	vector<pair<size_t, size_t>> devices{ std::make_pair(platform, device) };
	RendererCL<T, float> renderer(devices);
	const char* loc = __FUNCTION__;

	if (!renderer.Ok())
	{
		cout << loc << "Creating RendererCL failed, tests will not be run." << endl;
		return false;
	}

	MakeTestAllVarsRegPrePost(embers);

	for (auto& it : embers)
	{
		renderer.SetEmber(it, eProcessAction::FULL_RENDER, true);

		//if (platform != 0 &&
		//		((it.GetXform(0)->GetVariationById(eVariationId::VAR_SYNTH) != nullptr) ||//Nvidia OpenCL driver crashes when building too many synths.
		//		 (it.GetXform(0)->GetVariationById(eVariationId::VAR_PRE_SYNTH) != nullptr) ||
		//		 (it.GetXform(0)->GetVariationById(eVariationId::VAR_POST_SYNTH) != nullptr)))
		//{
		//	cout << "Skipping synth.\n";
		//	continue;
		//}

		if (renderer.BuildIterProgramForEmber())
		{
			if (printSuccess)
				cout << loc << ": Build succeeded for ember " << it.m_Name << endl;
		}
		else
		{
			success = false;
			cout << loc << ": OpenCL program build failed for ember " << it.m_Name << ":\n" << renderer.ErrorReportString() << endl;
			break;
		}
	}

	return success;
}

template <typename T>
void TestCpuGpuResults(size_t platform, size_t device)
{
	bool breakOnBad = true;
	int i = 0;//(int)eVariationId::VAR_TARGET;//Start at the one you want to test.
	int iters = 10;
	int skipped = 0;
	T thresh = T(1e-3);
	Timing t;
	VariationList<T> vlf;
	QTIsaac<ISAAC_SIZE, ISAAC_INT> rand;
	vector<PointCL<T>> points;
	vector<pair<size_t, size_t>> devices{ std::make_pair(platform, device) };
	RendererCL<T, float> renderer(devices);
	points.resize(renderer.TotalIterKernelCount());

	while (i < vlf->RegSize())
	{
		bool bad = false;
		double sum = 0;
		Variation<T>* var = vlf->GetVariation(i, eVariationType::VARTYPE_REG);
		string s = var->OpenCLString() + var->OpenCLFuncsString();

		if (s.find("MwcNext") != string::npos)
		{
			i++;
			skipped++;
			continue;
		}

		cout << "Testing cpu-gpu equality for variation: " << var->Name() << " (" << (int)var->VariationId() << ")" << endl;

		for (int iter = 0; iter < iters; iter++)
		{
			Point<T> p, p2;
			Ember<T> ember;
			Xform<T> xform;
			Variation<T>* varCopy = var->Copy();
			p.m_X = rand.Frand<T>(-5, 5);
			p.m_Y = rand.Frand<T>(-5, 5);
			p.m_Z = rand.Frand<T>(-5, 5);
			p.m_ColorX = rand.Frand01<T>();
			p.m_Opacity = rand.Frand01<T>();
			varCopy->Random(rand);
			xform.AddVariation(varCopy);
			ember.AddXform(xform);
			ember.CacheXforms();
			renderer.SetEmber(ember, eProcessAction::FULL_RENDER, true);
			renderer.AssignIterator();

			if (!renderer.Alloc())
				return;

			points[0].m_X = p.m_X;
			points[0].m_Y = p.m_Y;
			points[0].m_Z = p.m_Z;
			points[0].m_ColorX = p.m_ColorX;
			xform.Apply(&p, &p2, rand);
			renderer.WritePoints(points);
			renderer.Iterate(1, 0, 1);
			renderer.ReadPoints(points);
			T xdiff = std::abs(p2.m_X - points[0].m_X);
			T ydiff = std::abs(p2.m_Y - points[0].m_Y);
			T zdiff = std::abs(p2.m_Z - points[0].m_Z);

			if (xdiff > thresh || ydiff > thresh || zdiff > thresh)
			{
				bad = true;
				cout << __FUNCTION__ << ": Variation cpu-gpu diff for iter " << iter << ": " << varCopy->Name() << " (" << (int)varCopy->VariationId() << ") xdiff: " << xdiff << endl;
				cout << __FUNCTION__ << ": Variation cpu-gpu diff for iter " << iter << ": " << varCopy->Name() << " (" << (int)varCopy->VariationId() << ") ydiff: " << ydiff << endl;
				cout << __FUNCTION__ << ": Variation cpu-gpu diff for iter " << iter << ": " << varCopy->Name() << " (" << (int)varCopy->VariationId() << ") zdiff: " << zdiff << endl;
				cout << varCopy->ToString() << endl;
			}
			else
			{
				//cout << "Variation " << var->Name() << " had no difference between cpu and gpu for iter " << iter << endl;
			}
		}

		if (breakOnBad && bad)
			break;

		i++;
		bad = false;
	}

	cout << "Skipped " << skipped << endl;
}

template <typename T>
void TestGpuVectorRead(size_t platform, size_t device)
{
	T minx = TMAX, miny = TMAX, minz = TMAX, mincolorx = TMAX;
	T maxx = TLOW, maxy = TLOW, maxz = TLOW, maxcolorx = TLOW;
	double sumx = 0, avgx = 0;
	double sumy = 0, avgy = 0;
	double sumz = 0, avgz = 0;
	double sumcolorx = 0, avgcolorx = 0;
	Timing t;
	VariationList<T> vlf;
	QTIsaac<ISAAC_SIZE, ISAAC_INT> rand;
	vector<PointCL<T>> points;
	vector<pair<size_t, size_t>> devices{ std::make_pair(platform, device) };
	RendererCL<T, float> renderer(devices);
	points.resize(renderer.TotalIterKernelCount());
	Variation<T>* var = vlf->GetVariation(eVariationId::VAR_LINEAR);
	Point<T> p, p2;
	Ember<T> ember;
	Xform<T> xform;
	Variation<T>* varCopy = var->Copy();
	p.m_X = rand.Frand<T>(-5, 5);
	p.m_Y = rand.Frand<T>(-5, 5);
	p.m_Z = rand.Frand<T>(-5, 5);
	p.m_ColorX = rand.Frand01<T>();
	p.m_Opacity = rand.Frand01<T>();
	varCopy->Random(rand);
	xform.AddVariation(varCopy);
	ember.AddXform(xform);
	ember.CacheXforms();
	renderer.SetEmber(ember, eProcessAction::FULL_RENDER, true);
	renderer.AssignIterator();

	if (!renderer.Alloc())
		return;

	uint i, iters = renderer.IterCountPerKernel() * renderer.TotalIterKernelCount();//Make each thread in each block run at least once.
	renderer.Iterate(iters, 0, 1);
	renderer.ReadPoints(points);
	cout << __FUNCTION__ << ": GPU point test value results:" << endl;

	for (i = 0; i < points.size(); i++)
	{
		cout << "point[" << i << "].m_X = " << points[i].m_X << endl;
		cout << "point[" << i << "].m_Y = " << points[i].m_Y << endl;
		cout << "point[" << i << "].m_Z = " << points[i].m_Z << endl;
		cout << "point[" << i << "].m_ColorX = " << points[i].m_ColorX << endl << endl;
		minx = min<T>(points[i].m_X, minx);
		miny = min<T>(points[i].m_Y, miny);
		minz = min<T>(points[i].m_Z, minz);
		mincolorx = min<T>(points[i].m_ColorX, mincolorx);
		maxx = max<T>(points[i].m_X, maxx);
		maxy = max<T>(points[i].m_Y, maxy);
		maxz = max<T>(points[i].m_Z, maxz);
		maxcolorx = max<T>(points[i].m_ColorX, maxcolorx);
		sumx += points[i].m_X;
		sumy += points[i].m_Y;
		sumz += points[i].m_Z;
		sumcolorx += points[i].m_ColorX;
	}

	avgx = sumx / i;
	avgy = sumy / i;
	avgz = sumz / i;
	avgcolorx = sumcolorx / i;
	cout << "avgx = " << avgx << endl;
	cout << "avgy = " << avgy << endl;
	cout << "avgz = " << avgz << endl;
	cout << "avgcolorx = " << avgcolorx << endl;
	cout << "minx = " << minx << endl;
	cout << "miny = " << miny << endl;
	cout << "minz = " << minz << endl;
	cout << "mincolorx = " << mincolorx << endl << endl;
	cout << "maxx = " << maxx << endl;
	cout << "maxy = " << maxy << endl;
	cout << "maxz = " << maxz << endl;
	cout << "maxcolorx = " << maxcolorx << endl << endl << endl;
}
#endif

template <typename T>
void TestRandomAccess(size_t vsize, size_t ipp, bool cache)
{
	size_t iters = vsize * ipp;
	vector<v4T> vec;
	QTIsaac<ISAAC_SIZE, ISAAC_INT> rand;
	vec.resize(vsize);
	v4T* vdata = vec.data();

	if (cache)
	{
		for (size_t i = 0; i < iters; i++)
		{
			v4T v4(rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
			int index = rand.Rand((ISAAC_INT)vsize);
			v4T v42 = vdata[index];
			v4.x = std::log(v4.x);
			v4.y = std::sqrt(v4.y);
			v4.z = std::sin(v4.z);
			v4.w = std::cos(v4.w);
			v4 += T(1.234);
			v4 *= T(55.55);
			v4 /= T(0.0045);
			vdata[index] = v4 + v42;
		}
	}
	else
	{
		for (size_t i = 0; i < iters; i++)
		{
			v4T v4(rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>(), rand.Frand11<T>());
			int index = rand.Rand((ISAAC_INT)vsize);
			v4.x = std::log(v4.x);
			v4.y = std::sqrt(v4.y);
			v4.z = std::sin(v4.z);
			v4.w = std::cos(v4.w);
			v4 += T(1.234);
			v4 *= T(55.55);
			v4 /= T(0.0045);
			vdata[index] += v4;
		}
	}
}

template <typename T>
void TestCross(T x, T y, T weight)
{
	T s = x * x - y * y;
	T r = weight * std::sqrt(1 / (s * s + EPS));
	T outX = x * r;
	T outY = y * r;
	cout << "First way, outX, outY == " << outX << ", " << outY << endl;
	r = std::abs((x - y) * (x + y) + EPS);

	if (r < 0)
		r = -r;

	r = weight / r;
	outX = x * r;
	outY = y * r;
	cout << "Second way, outX, outY == " << outX << ", " << outY << endl;
}

double RandD(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
{
	return ((((rand.Rand() ^ (rand.Rand() << 15)) & 0xfffffff) * 3.72529e-09) - 0.5);
}
//
//#define BEZ_POINT_LENGTH 4
//
//void BezierSolve(double t, glm::vec2* src, double* w, glm::vec2& solution)
//{
//	double s, s2, s3, t2, t3, nom_x, nom_y, denom;
//
//	s = 1 - t;
//	s2 = s * s;
//	s3 = s * s * s;
//	t2 = t * t;
//	t3 = t * t * t;
//
//	nom_x = w[0] * s3 * src[0].x + w[1] * s2 * 3 * t * src[1].x + w[2] * s * 3 * t2 * src[2].x + w[3] * t3 * src[3].x;
//
//	nom_y = w[0] * s3 * src[0].y + w[1] * s2 * 3 * t * src[1].y + w[2] * s * 3 * t2 * src[2].y + w[3] * t3 * src[3].y;
//
//	denom = w[0] * s3 + w[1] * s2 * 3 * t + w[2] * s * 3 * t2 + w[3] * t3;
//
//
//	if (isnan(nom_x) || isnan(nom_y) || isnan(denom) || denom == 0)
//		return;
//
//	solution.x = nom_x / denom;
//	solution.y = nom_y / denom;
//}
//
//void BezierSetRect(glm::vec2* points, bool flip, glm::vec4& rect)
//{
//	double f;
//
//	for (int i = 0; i < BEZ_POINT_LENGTH; i++)
//	{
//		if (flip)
//			f = 1 - points[i].y;
//		else
//			f = points[i].y;
//
//		points[i].x = points[i].x * (rect.z - rect.x) + rect.x;
//		points[i].y = f * (rect.w - rect.y) + rect.y;
//	}
//}
//
//void BezierUnsetRect(glm::vec2* points, bool flip, glm::vec4& rect)
//{
//	if ((rect.z - rect.x) == 0 || (rect.w - rect.y) == 0)
//		return;
//
//	for (int i = 0; i < BEZ_POINT_LENGTH; i++)
//	{
//		points[i].x = (points[i].x - rect.x) / (rect.z - rect.x);
//		points[i].y = (points[i].y - rect.y) / (rect.w - rect.y);
//
//		if (flip)
//			points[i].y = 1 - points[i].y;
//	}
//}
//
//struct BezierPoints
//{
//	glm::vec2 points[4];
//};
//
//struct BezierWeights
//{
//	double points[4];
//};

void TestThreadedKernel()
{
	OpenCLWrapper wrapper1, wrapper2;

	if (wrapper1.Init(1, 0) && wrapper2.Init(2, 0))
	{
		string k = ConstantDefinesString(false) + "\n__kernel void Kern()\n"
				   "{\n"
				   "	int gid = GLOBAL_ID_X + GLOBAL_ID_Y;\n"
				   "}\n"
				   "\n";

		if (wrapper1.AddProgram("prog1", k, "Kern", false) &&
				wrapper2.AddProgram("prog1", k, "Kern", false))
		{
			cout << "Builds ok, now run..." << endl;
			std::thread th1([&]()
			{
				if (wrapper1.RunKernel(0, 256, 16, 1, 16, 16, 1))
				{
					cout << "Successful run inside thread 1..." << endl;
				}
			});
			std::thread th2([&]()
			{
				if (wrapper2.RunKernel(0, 256, 16, 1, 16, 16, 1))
				{
					cout << "Successful run inside thread 2..." << endl;
				}
			});
			Join(th1);
			Join(th2);
			cout << "Successful join of kernel thread..." << endl;
		}
	}
}

template <typename T>
void DistribTester()
{
	size_t i;
	size_t distribCount = 1;
	size_t xformCount = 3;
	vector<byte> m_XformDistributions;
	size_t j = 0;
	vector<T> weights { T(0.333333), T(1.0), T(0.25) };
	double tempDensity = 0, currentDensityLimit = 0, densityPerElement;

	if (m_XformDistributions.size() < CHOOSE_XFORM_GRAIN * distribCount)
		m_XformDistributions.resize(CHOOSE_XFORM_GRAIN * distribCount);

	if (m_XformDistributions.size() < CHOOSE_XFORM_GRAIN * distribCount)
		return;

	for (size_t distrib = 0; distrib < distribCount; distrib++)
	{
		double totalDensity = 0;

		//First find the total densities of all xforms.
		for (i = 0; i < xformCount; i++)
		{
			T d = weights[i];
			totalDensity += d;
		}

		//Original returned false if all were 0, but it's allowed here
		//which will just end up setting all elements to 0 which means
		//only the first xform will get used.
		//Calculate how much of a fraction of a the total density each element represents.
		j = 0;
		tempDensity = 0;
		currentDensityLimit = 0;
		densityPerElement = totalDensity / CHOOSE_XFORM_GRAIN;

		//Assign xform indices in order to each element of m_XformDistributions.
		//The number of elements assigned a given index is proportional to that xform's
		//density relative to the sum of all densities.
		for (i = 0; i < xformCount; i++)
		{
			T temp = weights[i];
			currentDensityLimit += temp;

			//Populate points corresponding to this xform's weight/density.
			//Also check that j is within the bounds of the distribution array just to be safe in the case of a rounding error.
			while (tempDensity < currentDensityLimit && j < CHOOSE_XFORM_GRAIN)
			{
#ifdef _DEBUG

				//Ensure distribution contains no out of bounds indices.
				if ((unsigned char)i >= xformCount)
					throw "Out of bounds xform index in selection distribution.";

#endif
				//cout << "offset = " << j << ", xform = " << i << ", running sum = " << tempDensity << "\n";
				m_XformDistributions[(distrib * CHOOSE_XFORM_GRAIN) + j] = byte(i);
				tempDensity += densityPerElement;
				j++;
			}
		}

		//Flam3 did this, which gives the same result.
		//T t = xforms[0].m_Weight;
		//
		//if (distrib > 0)
		//	t *= xforms[distrib - 1].Xaos(0);
		//
		//T r = 0;
		//
		//for (i = 0; i < CHOOSE_XFORM_GRAIN; i++)
		//{
		//	while (r >= t)
		//	{
		//		j++;
		//
		//		if (distrib > 0)
		//			t += xforms[j].m_Weight * xforms[distrib - 1].Xaos(j);
		//		else
		//			t += xforms[j].m_Weight;
		//	}
		//
		//	m_XformDistributions[(distrib * CHOOSE_XFORM_GRAIN) + i] = j;
		//	r += densityPerElement;
		//}
	}
}

template <typename T>
void TestAffine()
{
	v2T x(1, 0), y(0, 1), t(1, 0);
	Affine2D<T> af(x, y, t);
	auto af2 = af;
	cout << af.ToString() << "\n\n";
	af.RotateTrans(90);
	cout << af.ToString() << "\n\n";
	af2.RotateTrans(-90);
	cout << af2.ToString() << "\n\n";
}

template <typename T>
void TestRotate()
{
	T angle = 45;
	v3T x(1, 0, 0), y(0, 1, 0), xy(1, 1, 0);
	auto xtrans = glm::translate(m4T(1), xy);
	auto xrot = glm::rotate(xtrans, angle * DEG_2_RAD_T, v3T(0, 0, 1));
}

template<typename T>
class EmberContainerTester
{
public:
	template <typename Alloc, template <typename, typename> class C>
	static void TestEmberContainer(C<Ember<T>, Alloc>& cont)
	{
		Ember<T> e;
		cont.push_back(e);
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	//int i;
	bool b = true;
	size_t times = 1'000'000'001;
	QTIsaac<ISAAC_SIZE, ISAAC_INT> rand;
	std::vector<unsigned int> vec(16, 0);
	double accum = 0;
	Timing t(4);
	/*
	    for (size_t i = 0; i < times; i++)
	    {
		auto res = rand.Frand11<double>();
		auto flr = (double)(Floor(res));
		accum += flr;
	    }

	    t.Toc("Floor");
	    cout << accum << endl;
	    accum = 0;
	    t.Tic();

	    for (size_t i = 0; i < times; i++)
	    {
		auto res = rand.Frand11<double>();
		auto flr = std::floor(res);
		accum += flr;
	    }

	    t.Toc("std::floor");
	    cout << accum << endl;

	    t.Tic();

	    for (size_t i = 0; i < times; i++)
	    {
		accum += rand.Frand01<double>();
	    }

	    t.Toc("Frand01");
	    cout << accum << endl;
	    accum = 0;
	    t.Tic();

	    for (size_t i = 0; i < times; i++)
	    {
		accum += rand.Frand<double>(static_cast<double>(0), static_cast<double>(1));
	    }

	    t.Toc("Frand(0, 1)");
	    cout << accum << endl;
	    return 1;
	*/
	/*

		for (auto& it : vec)
		{
			cout << it / (double)times << endl;
			it = 0;
		}

		cout << "\n\n";
		t.Tic();

		for (size_t i = 1; i < times; i++)
		{
			//auto res = (uint)(((size_t)rand.Rand() * i) >> 32);
			auto res = rand.Rand(i);
			vec[res & 15]++;
		}

		t.Toc("rand mult shift");

		for (auto& it : vec)
		{
			cout << it / (double)times << endl;
			it = 0;
		}

		return 1;
		  vector<Ember<float>> fv;
			vector<Ember<double>> dv;
			list<Ember<float>> fl;
			list<Ember<double>> dl;
			int w = 1000, h = 1000;
			string filename = ".\\testexr.exr";
			vector<Rgba> pixels;
			pixels.resize(w * h);

			for (auto& pix : pixels)
			{
			pix.r = 1.0;
			pix.b = 0.0;
			pix.a = 1.0;
			//pix.r = std::numeric_limits<float>::max();
			}

			writeRgba1(filename.c_str(), pixels.data(), w, h);
			TestFuncs();
			string line = "title=\"cj_aerie\" smooth=no", delim = " =\"";
			auto vec = Split(line, delim, true);

			for (auto& s : vec) cout << s << endl;

			line = "index=0 color=2177354", delim = " =";
			vec = Split(line, delim, true);

			for (auto& s : vec) cout << s << endl;


			EmberContainerTester<float>::TestEmberContainer(fv);
			EmberContainerTester<double>::TestEmberContainer(dv);
			EmberContainerTester<float>::TestEmberContainer(fl);
			EmberContainerTester<double>::TestEmberContainer(dl);
			CopyCont(fv, fl);
	*/
	//QTIsaac<ISAAC_SIZE, ISAAC_INT> rand(1, 2, 3);
	//mt19937 meow(1729);
	/*  TestAffine<float>();
		TestAffine<double>();*/
	/*  TestRotate<float>();
		TestRotate<double>();
		return 1;
	*/
	//MakeTestAllVarsRegPrePostComboFile("testallvarsout.flame");
	//cout << (10.0 / 2.0 * 5.0) << endl;
	//FindFmaCandidates();
	//FindFmaImplemented();
	//cout << 5 * 3 + 2 << endl;
	//cout << 2 + 5 * 3 << endl;
	/*  return 0;

				TestThreadedKernel();

				auto palf = PaletteList<float>::Instance();
				Palette<float>* pal = palf->GetRandomPalette();

				cout << pal->Size() << endl;

				double d = 1;

				for (int i = 0; i < 10; i++)
				{
				cout << "log10(" << d << ") = " << std::max<uint>(1u, uint(std::log10(d)) + 1u) << endl;
				d *= 10;
				}

				return 0;*/
	/*
		uint i, iters = (uint)10e7;
		size_t total = 0;

		t.Tic();
		for (i = 0; i < iters; i++)
		{
		total += rand.RandByte();
		total += rand.Rand();
		}
		t.Toc("Isaac sum");

		cout << "Isaac total = " << total << " for " << i << " iters." << endl;

		total = 0;

		t.Tic();
		for (i = 0; i < iters; i++)
		{
		total += meow();
		}
		t.Toc("Mt sum");

		cout << "Mt total = " << total << " for " << i << " iters." << endl;
	*/
	//glm::vec2 solution, src[4];
	//double bezT = 1, w[4];
	//BezierPoints curvePoints[4];
	//BezierWeights curveWeights[4];
	//
	//BezierSolve(bezT, src, w, solution);
	//cout << pow(-1, 5.1) << endl;
	/*  for (i = 0; i < 2500000000; i++)
		{
		double d = std::abs(RandD(rand));

		if (d >= 0.5)
			cout << d << endl;
		}

		return 0;*/
	//cout << "sizeof(Ember<float>): " << sizeof(Ember<float>) << endl;
	//cout << "sizeof(Ember<double>): " << sizeof(Ember<double>) << endl;
	//
	//cout << "sizeof(Renderer<float>): " << sizeof(Renderer<float, float>) << endl;
	//cout << "sizeof(Renderer<double>): " << sizeof(Renderer<double, double>) << endl;
	//
	//cout << "sizeof(RendererCL<float>): " << sizeof(RendererCL<float>) << endl;
	//cout << "sizeof(RendererCL<double>): " << sizeof(RendererCL<double>) << endl;
	/*  unique_ptr<LinearVariation<float>> linV(new LinearVariation<float>());
		unique_ptr<PreLinearVariation<float>> preLinV(new PreLinearVariation<float>());
		unique_ptr<PostLinearVariation<float>> postLinV(new PostLinearVariation<float>());

		cout << linV->BaseName() << endl;
		cout << preLinV->BaseName() << endl;
		cout << postLinV->BaseName() << endl;*/
	//float num = 1;
	//float denom = 4294967296.0f;
	//float frac = num / denom;
	//
	//cout << "num, denom, frac = " << num << ", " << denom << ", " << frac << endl;
	//TestGpuVectorRead<double>();
	//TestGpuVectorRead<float>();
	//return 0;
	//unique_ptr<PreFarblurVariation<float>> preFarblurV(new PreFarblurVariation<float>());
	//size_t vsize = 1024 * 1024;
	//
	//t.Tic();
	//TestRandomAccess<float>(vsize, 10, true);
	//t.Toc("TestRandomAccess<float>(true)");
	//
	//t.Tic();
	//TestRandomAccess<float>(vsize, 10, false);
	//t.Toc("TestRandomAccess<float>(false)");
	//
	//t.Tic();
	//TestRandomAccess<double>(vsize, 10, true);
	//t.Toc("TestRandomAccess<double>(true)");
	//
	//t.Tic();
	//TestRandomAccess<double>(vsize, 10, false);
	//t.Toc("TestRandomAccess<double>(false)");
	//TestCross<double>(rand.Frand<double>(-5, 5), rand.Frand<double>(-5, 5), rand.Frand<double>(-5, 5));
	//TestCross<double>(rand.Frand<double>(-5, 5), rand.Frand<double>(-5, 5), rand.Frand<double>(-5, 5));
	//TestCross<double>(rand.Frand<double>(-5, 5), rand.Frand<double>(-5, 5), rand.Frand<double>(-5, 5));
	//TestCross<double>(rand.Frand<double>(-5, 5), rand.Frand<double>(-5, 5), rand.Frand<double>(-5, 5));
	//TestCross<double>(rand.Frand<double>(-5, 5), rand.Frand<double>(-5, 5), rand.Frand<double>(-5, 5));
	//std::complex<double> cd, cd2;
	//cd2 = sin(cd);
	auto testfunc = [&]()
	{
		t.Tic();
		TestCasting();
		t.Toc("TestCasting()");
		t.Tic();
		auto vlf(VariationList<float>::Instance());
		t.Toc("Creating VariationList<float>");
		cout << "There are " << vlf->Size() << " variations present." << endl;
#ifdef DO_DOUBLE
		t.Tic();
		auto vld(VariationList<double>::Instance());
		t.Toc("Creating VariationList<double>");
#endif
		t.Tic();
		TestVarCounts();
		t.Toc("TestVarCounts()");
		t.Tic();
		TestVarUnique<float>();
		t.Toc("TestVarUnique<float>()");
#ifdef DO_DOUBLE
		t.Tic();
		TestVarUnique<double>();
		t.Toc("TestVarUnique<double>()");
#endif
		t.Tic();
		TestVarCopy<float, float>();
		t.Toc("TestVarCopy<float, float>()");
#ifdef DO_DOUBLE
		t.Tic();
		TestVarCopy<double, double>();
		t.Toc("TestVarCopy<double, double>()");
		t.Tic();
		TestVarCopy<float, double>();
		t.Toc("TestVarCopy<float, double>()");
		t.Tic();
		TestVarCopy<double, float>();
		t.Toc("TestVarCopy<double, float>()");
#endif
		t.Tic();
		TestVarRegPrePost();
		t.Toc("TestVarRegPrePost()");
		t.Tic();
		TestParVars();
		t.Toc("TestParVars()");
		t.Tic();
		TestVarPrePostNames();
		t.Toc("TestVarPrePostNames()");
		t.Tic();
		TestVarPrecalcUsedCL();
		t.Toc("TestVarPrecalcUsedCL()");
		t.Tic();
		TestVarAssignTypes();
		t.Toc("TestVarAssignTypes()");
		t.Tic();
		TestVarAssignVals();
		t.Toc("TestVarAssignVals()");
		t.Tic();
		TestZepsFloor();
		t.Toc("TestZepsFloor()");
		t.Tic();
		TestConstants();
		t.Toc("TestConstants()");
		t.Tic();
		TestGlobalFuncs();
		t.Toc("TestGlobalFuncs()");
		/*  t.Tic();
			TestXformsInOutPoints();
			t.Toc("TestXformsInOutPoints()");

			t.Tic();
			TestVarTime<float>();
			t.Toc("TestVarTime()");
		*/
		t.Tic();
		TestOperations<float>();
		t.Toc("TestOperations()");
		t.Tic();
		TestArbitrary();
		t.Toc("TestArbitrary()");
		//t.Tic();
		//TestVarsSimilar<float>();
		//t.Toc("TestVarsSimilar()");
#ifdef TEST_CL
		//t.Tic();
		//TestCpuGpuResults<float>();
		//t.Toc("TestCpuGpuResults<float>()");
		t.Tic();
		b = TestAllVarsCLBuild<float>(0, 0, true);
		t.Toc("TestAllVarsCLBuild<float>()");

		if (b)
		{
#ifdef DO_NVIDIA
			t.Tic();
			b = TestAllVarsCLBuild<float>(1, 0, true);
			t.Toc("TestAllVarsCLBuild<float>()");
#endif
		}

#ifdef DO_DOUBLE

		//t.Tic();
		//TestCpuGpuResults<double>();
		//t.Toc("TestCpuGpuResults<double>()");
		if (b)
		{
			t.Tic();
			b = TestAllVarsCLBuild<double>(0, 0, true);
			t.Toc("TestAllVarsCLBuild<double>()");

			if (b)
			{
#ifdef DO_NVIDIA
				t.Tic();
				TestAllVarsCLBuild<double>(1, 0, true);
				t.Toc("TestAllVarsCLBuild<double>()");
#endif
			}
		}

#endif
#endif
	};
	cout << "\n\nTesting in compatibility mode:\n\n";
	Compat::m_Compat = true;
	testfunc();
	cout << "\n\nTesting in non-compatibility mode:\n\n";
	Compat::m_Compat = false;
	testfunc();
	//PrintAllVars();
	//_CrtDumpMemoryLeaks();
	return 0;
}
