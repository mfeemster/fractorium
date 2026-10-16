#pragma once

#include "Variation.h"

namespace EmberNs
{
/// <summary>
/// DC Bubble.
/// This accesses the summed output point in a rare and different way.
/// </summary>
template <typename T>
class DCBubbleVariation : public ParametricVariation<T>
{
public:
	DCBubbleVariation(T weight = 1.0) : ParametricVariation<T>("dc_bubble", eVariationId::VAR_DC_BUBBLE, weight, true)
	{
		Init();
	}

	PARVARCOPY(DCBubbleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = helper.m_PrecalcSumSquares;
		T r4_1 = Zeps(r / 4 + 1);
		r4_1 = m_Weight / r4_1;
		helper.Out.x = r4_1 * helper.In.x;
		helper.Out.y = r4_1 * helper.In.y;
		helper.Out.z = m_Weight * (2 / r4_1 - 1);
		T sumX, sumY;

		if (m_VarType == eVariationType::VARTYPE_PRE)
		{
			sumX = helper.In.x;
			sumY = helper.In.y;
		}
		else
		{
			sumX = outPoint.m_X;
			sumY = outPoint.m_Y;
		}

		T tempX = helper.Out.x + sumX;
		T tempY = helper.Out.y + sumY;
		outPoint.m_ColorX = fmod(std::abs(m_Bdcs * (Sqr<T>(tempX + m_CenterX) + Sqr<T>(tempY + m_CenterY))), T(1.0));
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string scale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string centerX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string centerY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bdcs = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		ss << "\t{\n"
		   << "\t\treal_t r = precalcSumSquares;\n"
		   << "\t\treal_t r4_1 = Zeps(r / 4 + 1);\n"
		   << "\t\tr4_1 = " << weight << " / r4_1;\n"
		   << "\n"
		   << "\t\tvOut.x = r4_1 * vIn.x;\n"
		   << "\t\tvOut.y = r4_1 * vIn.y;\n"
		   << "\t\tvOut.z = " << weight << " * (2 / r4_1 - 1);\n"
		   << "\n"
		   << "\t\treal_t sumX, sumY;\n\n";

		if (m_VarType == eVariationType::VARTYPE_PRE)
		{
			ss
					<< "\t\tsumX = vIn.x;\n"
					<< "\t\tsumY = vIn.y;\n";
		}
		else
		{
			ss
					<< "\t\tsumX = outPoint->m_X;\n"
					<< "\t\tsumY = outPoint->m_Y;\n";
		}

		ss
				<< "\t\treal_t tempX = vOut.x + sumX;\n"
				<< "\t\treal_t tempY = vOut.y + sumY;\n"
				<< "\n"
				<< "\t\toutPoint->m_ColorX = fmod(fabs(" << bdcs << " * (Sqr(tempX + " << centerX << ") + Sqr(tempY + " << centerY << "))), (real_t)(1.0));\n"
				<< "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Sqr", "Zeps" };
	}

	virtual void Precalc() override
	{
		m_Bdcs = 1 / (m_Scale == 0 ? T(10E-6) : m_Scale);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_CenterX, prefix + "dc_bubble_centerx"));
		m_Params.push_back(ParamWithName<T>(&m_CenterY, prefix + "dc_bubble_centery"));
		m_Params.push_back(ParamWithName<T>(&m_Scale, prefix + "dc_bubble_scale", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Bdcs, prefix + "dc_bubble_bdcs"));//Precalc.
	}

private:
	T m_CenterX;
	T m_CenterY;
	T m_Scale;
	T m_Bdcs;//Precalc.
};

/// <summary>
/// DC Carpet.
/// </summary>
template <typename T>
class DCCarpetVariation : public ParametricVariation<T>
{
public:
	DCCarpetVariation(T weight = 1.0) : ParametricVariation<T>("dc_carpet", eVariationId::VAR_DC_CARPET, weight)
	{
		Init();
	}

	PARVARCOPY(DCCarpetVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		int x0 = rand.RandBit() ? -1 : 1;
		int y0 = rand.RandBit() ? -1 : 1;
		T x = helper.In.x + x0;
		T y = helper.In.y + y0;
		T x0_xor_y0 = T(x0 ^ y0);
		T h = -m_H + (1 - x0_xor_y0) * m_H;
		helper.Out.x = m_Weight * (m_Xform->m_Affine.A() * x + m_Xform->m_Affine.B() * y + m_Xform->m_Affine.E());
		helper.Out.y = m_Weight * (m_Xform->m_Affine.C() * x + m_Xform->m_Affine.D() * y + m_Xform->m_Affine.F());
		helper.Out.z = DefaultZ(helper);
		outPoint.m_ColorX = fmod(std::abs(outPoint.m_ColorX * T(0.5) * (1 + h) + x0_xor_y0 * (1 - h) * T(0.5)), T(1.0));
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string origin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string h = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		ss << "\t{\n"
		   << "\t\tint x0 = (MwcNext(mwc) & 1) ? -1 : 1;\n"
		   << "\t\tint y0 = (MwcNext(mwc) & 1) ? -1 : 1;\n"
		   << "\t\treal_t x = vIn.x + x0;\n"
		   << "\t\treal_t y = vIn.y + y0;\n"
		   << "\t\treal_t x0_xor_y0 = (real_t)(x0 ^ y0);\n"
		   << "\t\treal_t h = -" << h << " + (1 - x0_xor_y0) * " << h << ";\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (xform->m_A * x + xform->m_B * y + xform->m_E);\n"
		   << "\t\tvOut.y = " << weight << " * (xform->m_C * x + xform->m_D * y + xform->m_F);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t\toutPoint->m_ColorX = fmod(fabs(outPoint->m_ColorX * (real_t)(0.5) * (real_t)(1 + h) + x0_xor_y0 * (real_t)(1 - h) * (real_t)(0.5)), (real_t)(1.0));\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_H = T(0.1) * m_Origin;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Origin, prefix + "dc_carpet_origin"));
		m_Params.push_back(ParamWithName<T>(true, &m_H, prefix + "dc_carpet_h"));//Precalc.
	}

private:
	T m_Origin;
	T m_H;//Precalc.
};

/// <summary>
/// DC Cube.
/// </summary>
template <typename T>
class DCCubeVariation : public ParametricVariation<T>
{
public:
	DCCubeVariation(T weight = 1.0) : ParametricVariation<T>("dc_cube", eVariationId::VAR_DC_CUBE, weight)
	{
		Init();
	}

	PARVARCOPY(DCCubeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x, y, z;
		T p = 2 * rand.Frand01<T>() - 1;
		T q = 2 * rand.Frand01<T>() - 1;
		uint i = rand.Rand(3);
		uint j = rand.RandBit();

		switch (i)
		{
			case 0:
				x = m_Weight * (j ? -1 : 1);
				y = m_Weight * p;
				z = m_Weight * q;

				if (j)
					outPoint.m_ColorX = m_ClampC1;
				else
					outPoint.m_ColorX = m_ClampC2;

				break;

			case 1:
				x = m_Weight * p;
				y = m_Weight * (j ? -1 : 1);
				z = m_Weight * q;

				if (j)
					outPoint.m_ColorX = m_ClampC3;
				else
					outPoint.m_ColorX = m_ClampC4;

				break;

			case 2:
			default:
				x = m_Weight * p;
				y = m_Weight * q;
				z = m_Weight * (j ? -1 : 1);

				if (j)
					outPoint.m_ColorX = m_ClampC5;
				else
					outPoint.m_ColorX = m_ClampC6;

				break;
		}

		helper.Out.x = x * m_DcCubeX;
		helper.Out.y = y * m_DcCubeY;
		helper.Out.z = z * m_DcCubeZ;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string cubeC1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeC2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeC3 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeC4 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeC5 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeC6 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cubeZ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string clampC1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string clampC2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string clampC3 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string clampC4 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string clampC5 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string clampC6 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x, y, z;\n"
		   << "\t\treal_t p = 2 * MwcNext01(mwc) - 1;\n"
		   << "\t\treal_t q = 2 * MwcNext01(mwc) - 1;\n"
		   << "\t\tuint i = MwcNextRange(mwc, 3u);\n"
		   << "\t\tuint j = MwcNext(mwc) & 1u;\n"
		   << "\n"
		   << "\t\tswitch (i)\n"
		   << "\t\t{\n"
		   << "\t\t	case 0:\n"
		   << "\t\t		x = " << weight << " * (j ? -1 : 1);\n"
		   << "\t\t		y = " << weight << " * p;\n"
		   << "\t\t		z = " << weight << " * q;\n"
		   << "\n"
		   << "\t\t		if (j)\n"
		   << "\t\t			outPoint->m_ColorX = " << clampC1 << ";\n"
		   << "\t\t		else\n"
		   << "\t\t			outPoint->m_ColorX = " << clampC2 << ";\n"
		   << "\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 1:\n"
		   << "\t\t		x =" << weight << " * p;\n"
		   << "\t\t		y =" << weight << " * (j ? -1 : 1);\n"
		   << "\t\t		z =" << weight << " * q;\n"
		   << "\n"
		   << "\t\t		if (j)\n"
		   << "\t\t			outPoint->m_ColorX = " << clampC3 << ";\n"
		   << "\t\t		else\n"
		   << "\t\t			outPoint->m_ColorX = " << clampC4 << ";\n"
		   << "\n"
		   << "\t\t		break;\n"
		   << "\t\t	case 2:\n"
		   << "\t\t	default:\n"
		   << "\t\t		x = " << weight << " * p;\n"
		   << "\t\t		y = " << weight << " * q;\n"
		   << "\t\t		z = " << weight << " * (j ? -1 : 1);\n"
		   << "\n"
		   << "\t\t		if (j)\n"
		   << "\t\t			outPoint->m_ColorX = " << clampC5 << ";\n"
		   << "\t\t		else\n"
		   << "\t\t			outPoint->m_ColorX = " << clampC6 << ";\n"
		   << "\n"
		   << "\t\t		break;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = x * " << cubeX << ";\n"
		   << "\t\tvOut.y = y * " << cubeY << ";\n"
		   << "\t\tvOut.z = z * " << cubeZ << ";\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_ClampC1 = Clamp<T>(m_DcCubeC1, 0, 1);
		m_ClampC2 = Clamp<T>(m_DcCubeC2, 0, 1);
		m_ClampC3 = Clamp<T>(m_DcCubeC3, 0, 1);
		m_ClampC4 = Clamp<T>(m_DcCubeC4, 0, 1);
		m_ClampC5 = Clamp<T>(m_DcCubeC5, 0, 1);
		m_ClampC6 = Clamp<T>(m_DcCubeC6, 0, 1);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_DcCubeC1, prefix + "dc_cube_c1"));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeC2, prefix + "dc_cube_c2"));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeC3, prefix + "dc_cube_c3"));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeC4, prefix + "dc_cube_c4"));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeC5, prefix + "dc_cube_c5"));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeC6, prefix + "dc_cube_c6"));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeX, prefix + "dc_cube_x", 1));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeY, prefix + "dc_cube_y", 1));
		m_Params.push_back(ParamWithName<T>(&m_DcCubeZ, prefix + "dc_cube_z", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_ClampC1, prefix + "dc_cube_clamp_c1"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_ClampC2, prefix + "dc_cube_clamp_c2"));
		m_Params.push_back(ParamWithName<T>(true, &m_ClampC3, prefix + "dc_cube_clamp_c3"));
		m_Params.push_back(ParamWithName<T>(true, &m_ClampC4, prefix + "dc_cube_clamp_c4"));
		m_Params.push_back(ParamWithName<T>(true, &m_ClampC5, prefix + "dc_cube_clamp_c5"));
		m_Params.push_back(ParamWithName<T>(true, &m_ClampC6, prefix + "dc_cube_clamp_c6"));
	}

private:
	T m_DcCubeC1;
	T m_DcCubeC2;
	T m_DcCubeC3;
	T m_DcCubeC4;
	T m_DcCubeC5;
	T m_DcCubeC6;
	T m_DcCubeX;
	T m_DcCubeY;
	T m_DcCubeZ;
	T m_ClampC1;//Precalc.
	T m_ClampC2;
	T m_ClampC3;
	T m_ClampC4;
	T m_ClampC5;
	T m_ClampC6;
};

/// <summary>
/// DC Cylinder.
/// This accesses the summed output point in a rare and different way.
/// </summary>
template <typename T>
class DCCylinderVariation : public ParametricVariation<T>
{
public:
	DCCylinderVariation(T weight = 1.0) : ParametricVariation<T>("dc_cylinder", eVariationId::VAR_DC_CYLINDER, weight)
	{
		Init();
	}

	PARVARCOPY(DCCylinderVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T temp = rand.Frand01<T>() * M_2PI;
		T sr = std::sin(temp);
		T cr = std::cos(temp);
		T r = m_Blur * (rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() - 2);
		helper.Out.x = m_Weight * std::sin(helper.In.x + r * sr) * m_X;
		helper.Out.y = r + helper.In.y * m_Y;
		helper.Out.z = m_Weight * std::cos(helper.In.x + r * cr);
		T sumX, sumY;

		if (m_VarType == eVariationType::VARTYPE_PRE)
		{
			sumX = helper.In.x;
			sumY = helper.In.y;
		}
		else
		{
			sumX = outPoint.m_X;
			sumY = outPoint.m_Y;
		}

		T tempX = helper.Out.x + sumX;
		T tempY = helper.Out.y + sumY;
		outPoint.m_ColorX = fmod(std::abs(T(0.5) * (m_Ldcs * ((m_Cosa * tempX + m_Sina * tempY + m_Offset)) + 1)), T(1.0));
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string offset = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blur = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sina = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string cosa = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ldcs = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ldca = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t temp = MwcNext(mwc) * M_2PI;\n"
		   << "\t\treal_t sr = sin(temp);\n"
		   << "\t\treal_t cr = cos(temp);\n"
		   << "\t\treal_t r = " << blur << " * (MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) - 2);\n"
		   << "\t\tvOut.x = " << weight << " * sin(vIn.x + r * sr)* " << x << ";\n"
		   << "\t\tvOut.y = r + vIn.y * " << y << ";\n"
		   << "\t\tvOut.z = " << weight << " * cos(vIn.x + r * cr);\n"
		   << "\n"
		   << "\t\treal_t sumX, sumY;\n\n";

		if (m_VarType == eVariationType::VARTYPE_PRE)
		{
			ss
					<< "\t\tsumX = vIn.x;\n"
					<< "\t\tsumY = vIn.y;\n";
		}
		else
		{
			ss
					<< "\t\tsumX = outPoint->m_X;\n"
					<< "\t\tsumY = outPoint->m_Y;\n";
		}

		ss
				<< "\t\treal_t tempX = vOut.x + sumX;\n"
				<< "\t\treal_t tempY = vOut.y + sumY;\n"
				<< "\n"
				<< "\t\toutPoint->m_ColorX = fmod(fabs((real_t)(0.5) * (" << ldcs << " * ((" << cosa << " * tempX + " << sina << " * tempY + " << offset << ")) + (real_t)(1.0))), (real_t)(1.0));\n"
				<< "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		sincos(m_Angle, &m_Sina, &m_Cosa);
		m_Ldcs = 1 / (m_Scale == 0.0 ? T(10E-6) : m_Scale);
		m_Ldca = m_Offset * T(M_PI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Offset, prefix + "dc_cylinder_offset"));
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "dc_cylinder_angle"));//Original used a prefix of dc_cyl_, which is incompatible with Ember's design.
		m_Params.push_back(ParamWithName<T>(&m_Scale, prefix + "dc_cylinder_scale", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "dc_cylinder_x", T(0.125)));//Original used a prefix of cyl_, which is incompatible with Ember's design.
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "dc_cylinder_y", T(0.125)));
		m_Params.push_back(ParamWithName<T>(&m_Blur, prefix + "dc_cylinder_blur", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Sina, prefix + "dc_cylinder_sina"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cosa, prefix + "dc_cylinder_cosa"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ldcs, prefix + "dc_cylinder_ldcs"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ldca, prefix + "dc_cylinder_ldca"));
	}

private:
	T m_Offset;
	T m_Angle;
	T m_Scale;
	T m_X;
	T m_Y;
	T m_Blur;
	T m_Sina;//Precalc.
	T m_Cosa;
	T m_Ldcs;
	T m_Ldca;
};

/// <summary>
/// DC GridOut.
/// </summary>
template <typename T>
class DCGridOutVariation : public Variation<T>
{
public:
	DCGridOutVariation(T weight = 1.0) : Variation<T>("dc_gridout", eVariationId::VAR_DC_GRIDOUT, weight) { }

	VARCOPY(DCGridOutVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = VarFuncs<T>::LRint(helper.In.x);
		T y = VarFuncs<T>::LRint(helper.In.y);
		T c = outPoint.m_ColorX;

		if (y <= 0)
		{
			if (x > 0)
			{
				if (-y >= x)
				{
					helper.Out.x = m_Weight * (helper.In.x + 1);
					helper.Out.y = m_Weight * helper.In.y;
					c += T(0.25);
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y + 1);
					c += T(0.75);
				}
			}
			else
			{
				if (y <= x)
				{
					helper.Out.x = m_Weight * (helper.In.x + 1);
					helper.Out.y = m_Weight * helper.In.y;
					c += T(0.25);
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y - 1);
					c += T(0.75);
				}
			}
		}
		else
		{
			if (x > 0)
			{
				if (y >= x)
				{
					helper.Out.x = m_Weight * (helper.In.x - 1);
					helper.Out.y = m_Weight * helper.In.y;
					c += T(0.25);
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y + 1);
					c += T(0.75);
				}
			}
			else
			{
				if (y > -x)
				{
					helper.Out.x = m_Weight * (helper.In.x - 1);
					helper.Out.y = m_Weight * helper.In.y;
					c += T(0.25);
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y - 1);
					c += T(0.75);
				}
			}
		}

		helper.Out.z = DefaultZ(helper);
		outPoint.m_ColorX = fmod(c, T(1.0));
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t x = LRint(vIn.x);\n"
		   << "\t\treal_t y = LRint(vIn.y);\n"
		   << "\t\treal_t c = outPoint->m_ColorX;\n"
		   << "\n"
		   << "\t\tif (y <= 0)\n"
		   << "\t\t{\n"
		   << "\t\t	if (x > 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		if (-y >= x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * (vIn.x + 1);\n"
		   << "\t\t			vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t			c += (real_t)(0.25);\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t			vOut.y = " << weight << " * (vIn.y + 1);\n"
		   << "\t\t			c += (real_t)(0.75);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (y <= x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * (vIn.x + 1);\n"
		   << "\t\t			vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t			c += (real_t)(0.25);\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t			vOut.y = " << weight << " * (vIn.y - 1);\n"
		   << "\t\t			c += (real_t)(0.75);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (x > 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		if (y >= x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * (vIn.x - 1);\n"
		   << "\t\t			vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t			c += (real_t)(0.25);\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t			vOut.y = " << weight << " * (vIn.y + 1);\n"
		   << "\t\t			c += (real_t)(0.75);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (y > -x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * (vIn.x - 1);\n"
		   << "\t\t			vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t			c += (real_t)(0.25);\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t			vOut.y = " << weight << " * (vIn.y - 1);\n"
		   << "\t\t			c += (real_t)(0.75);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t\toutPoint->m_ColorX = fmod(c, (real_t)(1.0));\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "LRint" };
	}
};

/// <summary>
/// DC Linear.
/// This accesses the summed output point in a rare and different way.
/// </summary>
template <typename T>
class DCLinearVariation : public ParametricVariation<T>
{
public:
	DCLinearVariation(T weight = 1.0) : ParametricVariation<T>("dc_linear", eVariationId::VAR_DC_LINEAR, weight)
	{
		Init();
	}

	PARVARCOPY(DCLinearVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * helper.In.x;
		helper.Out.y = m_Weight * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
		T sumX, sumY;

		if (m_VarType == eVariationType::VARTYPE_PRE)
		{
			sumX = helper.In.x;
			sumY = helper.In.y;
		}
		else
		{
			sumX = outPoint.m_X;
			sumY = outPoint.m_Y;
		}

		T tempX = helper.Out.x + sumX;
		T tempY = helper.Out.y + sumY;
		outPoint.m_ColorX = fmod(std::abs(T(0.5) * (m_Ldcs * ((m_Cosa * tempX + m_Sina * tempY + m_Offset)) + T(1.0))), T(1.0));
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string offset = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ldcs = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string ldca = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sina = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosa = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * vIn.x;\n"
		   << "\t\tvOut.y = " << weight << " * vIn.y;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\n"
		   << "\t\treal_t sumX, sumY;\n\n";

		if (m_VarType == eVariationType::VARTYPE_PRE)
		{
			ss
					<< "\t\tsumX = vIn.x;\n"
					<< "\t\tsumY = vIn.y;\n";
		}
		else
		{
			ss
					<< "\t\tsumX = outPoint->m_X;\n"
					<< "\t\tsumY = outPoint->m_Y;\n";
		}

		ss
				<< "\t\treal_t tempX = vOut.x + sumX;\n"
				<< "\t\treal_t tempY = vOut.y + sumY;\n"
				<< "\n"
				<< "\t\toutPoint->m_ColorX = fmod(fabs((real_t)(0.5) * (" << ldcs << " * ((" << cosa << " * tempX + " << sina << " * tempY + " << offset << ")) + (real_t)(1.0))), (real_t)(1.0));\n"
				<< "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Ldcs = 1 / (m_Scale == 0 ? T(10E-6) : m_Scale);
		m_Ldca = m_Offset * T(M_PI);
		sincos(m_Angle, &m_Sina, &m_Cosa);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Offset, prefix + "dc_linear_offset"));
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "dc_linear_angle"));
		m_Params.push_back(ParamWithName<T>(&m_Scale, prefix + "dc_linear_scale", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Ldcs, prefix + "dc_linear_ldcs"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Ldca, prefix + "dc_linear_ldca"));
		m_Params.push_back(ParamWithName<T>(true, &m_Sina, prefix + "dc_linear_sina"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cosa, prefix + "dc_linear_cosa"));
	}

private:
	T m_Offset;
	T m_Angle;
	T m_Scale;
	T m_Ldcs;//Precalc.
	T m_Ldca;
	T m_Sina;
	T m_Cosa;
};

/// <summary>
/// DC Triangle.
/// </summary>
template <typename T>
class DCTriangleVariation : public ParametricVariation<T>
{
public:
	DCTriangleVariation(T weight = 1.0) : ParametricVariation<T>("dc_triangle", eVariationId::VAR_DC_TRIANGLE, weight)
	{
		Init();
	}

	PARVARCOPY(DCTriangleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		// set up triangle
		const T
		xx = m_Xform->m_Affine.A(), xy = m_Xform->m_Affine.B(),  // X
		yx = m_Xform->m_Affine.C() * -1, yy = m_Xform->m_Affine.D() * -1,  // Y
		ox = m_Xform->m_Affine.E(), oy = m_Xform->m_Affine.F(),  // O
		px = helper.In.x - ox, py = helper.In.y - oy; // P
		// calculate dot products
		const T dot00 = xx * xx + xy * xy; // X * X
		const T dot01 = xx * yx + xy * yy; // X * Y
		const T dot02 = xx * px + xy * py; // X * P
		const T dot11 = yx * yx + yy * yy; // Y * Y
		const T dot12 = yx * px + yy * py; // Y * P
		// calculate barycentric coordinates
		const T denom = (dot00 * dot11 - dot01 * dot01);
		const T num_u = (dot11 * dot02 - dot01 * dot12);
		const T num_v = (dot00 * dot12 - dot01 * dot02);
		// u, v must not be constant
		T u = num_u / denom;
		T v = num_v / denom;
		int inside = 0, f = 1;

		// case A - point escapes edge XY
		if (u + v > 1)
		{
			f = -1;

			if (u > v)
			{
				ClampLteRef<T>(u, 1);
				v = 1 - u;
			}
			else
			{
				ClampLteRef<T>(v, 1);
				u = 1 - v;
			}
		}
		else if ((u < 0) || (v < 0))// case B - point escapes either edge OX or OY
		{
			ClampRef<T>(u, 0, 1);
			ClampRef<T>(v, 0, 1);
		}
		else
		{
			inside = 1;// case C - point is in triangle
		}

		// handle outside points
		if (m_ZeroEdges && !inside)
		{
			u = v = 0;
		}
		else if (!inside)
		{
			u = (u + rand.Frand01<T>() * m_A * f);
			v = (v + rand.Frand01<T>() * m_A * f);
			ClampRef<T>(u, -1, 1);
			ClampRef<T>(v, -1, 1);

			if ((u + v > 1) && (m_A > 0))
			{
				if (u > v)
				{
					ClampLteRef<T>(u, 1);
					v = 1 - u;
				}
				else
				{
					ClampLteRef<T>(v, 1);
					u = 1 - v;
				}
			}
		}

		// set output
		helper.Out.x = m_Weight * (ox + u * xx + v * yx);
		helper.Out.y = m_Weight * (oy + u * xy + v * yy);
		helper.Out.z = m_Weight * helper.In.z;
		outPoint.m_ColorX = fmod(std::abs(u + v), T(1.0));
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string scatterArea = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string zeroEdges = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		ss << "\t{\n"
		   << "\t\tconst real_t\n"
		   << "\t\txx = xform->m_A, xy = xform->m_B,\n"
		   << "\t\tyx = xform->m_C * -1, yy = xform->m_D * -1,\n"
		   << "\t\tox = xform->m_E, oy = xform->m_F,\n"
		   << "\t\tpx = vIn.x - ox, py = vIn.y - oy;\n"
		   << "\n"
		   << "\t\tconst real_t dot00 = xx * xx + xy * xy;\n"
		   << "\t\tconst real_t dot01 = xx * yx + xy * yy;\n"
		   << "\t\tconst real_t dot02 = xx * px + xy * py;\n"
		   << "\t\tconst real_t dot11 = yx * yx + yy * yy;\n"
		   << "\t\tconst real_t dot12 = yx * px + yy * py;\n"
		   << "\n"
		   << "\t\tconst real_t denom = (dot00 * dot11 - dot01 * dot01);\n"
		   << "\t\tconst real_t num_u = (dot11 * dot02 - dot01 * dot12);\n"
		   << "\t\tconst real_t num_v = (dot00 * dot12 - dot01 * dot02);\n"
		   << "\n"
		   << "\t\treal_t u = num_u / denom;\n"
		   << "\t\treal_t v = num_v / denom;\n"
		   << "\t\tint inside = 0, f = 1;\n"
		   << "\n"
		   << "\t\tif (u + v > 1)\n"
		   << "\t\t{\n"
		   << "\t\t	f = -1;\n"
		   << "\n"
		   << "\t\t	if (u > v)\n"
		   << "\t\t	{\n"
		   << "\t\t		u = u > 1 ? 1 : u;\n"
		   << "\t\t		v = 1 - u;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		v = v > 1 ? 1 : v;\n"
		   << "\t\t		u = 1 - v;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse if ((u < 0) || (v < 0))\n"
		   << "\t\t{\n"
		   << "\t\t	u = u < 0 ? 0 : u > 1 ? 1 : u;\n"
		   << "\t\t	v = v < 0 ? 0 : v > 1 ? 1 : v;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	inside = 1;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tif (" << zeroEdges << " != 0.0 && !inside)\n"
		   << "\t\t{\n"
		   << "\t\t	u = v = 0;\n"
		   << "\t\t}\n"
		   << "\t\telse if (!inside)\n"
		   << "\t\t{\n"
		   << "\t\t	u = (u + MwcNext01(mwc) * " << a << " * f);\n"
		   << "\t\t	v = (v + MwcNext01(mwc) * " << a << " * f);\n"
		   << "\t\t	u = u < -1 ? -1 : u > 1 ? 1 : u;\n"
		   << "\t\t	v = v < -1 ? -1 : v > 1 ? 1 : v;\n"
		   << "\n"
		   << "\t\t	if ((u + v > 1) && (" << a << " > 0))\n"
		   << "\t\t	{\n"
		   << "\t\t		if (u > v)\n"
		   << "\t\t		{\n"
		   << "\t\t			u = u > 1 ? 1 : u;\n"
		   << "\t\t			v = 1 - u;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			v = v > 1 ? 1 : v;\n"
		   << "\t\t			u = 1 - v;\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (ox + u * xx + v * yx);\n"
		   << "\t\tvOut.y = " << weight << " * (oy + u * xy + v * yy);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t\toutPoint->m_ColorX = fmod(fabs(u + v), (real_t)(1.0));\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_A = Clamp<T>(m_ScatterArea, -1, 1);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_ScatterArea, prefix + "dc_triangle_scatter_area", 0, eParamType::REAL, -1, 1));
		m_Params.push_back(ParamWithName<T>(&m_ZeroEdges, prefix + "dc_triangle_zero_edges", 0, eParamType::INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_A, prefix + "dc_triangle_a"));//Precalc.
	}

private:
	T m_ScatterArea;
	T m_ZeroEdges;
	T m_A;//Precalc.
};

/// <summary>
/// DC Transl.
/// The original used dc_ztransl and post_dcztransl incompatible with Ember's design.
/// These will follow the same naming convention as all other variations.
/// </summary>
template <typename T>
class DCZTranslVariation : public ParametricVariation<T>
{
public:
	DCZTranslVariation(T weight = 1.0) : ParametricVariation<T>("dc_ztransl", eVariationId::VAR_DC_ZTRANSL, weight)
	{
		Init();
	}

	PARVARCOPY(DCZTranslVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T zf = m_Factor * (outPoint.m_ColorX - m_X0_) / m_X1_m_x0;

		if (m_Clamp != 0)
			ClampRef<T>(zf, 0, 1);

		helper.Out.x = m_Weight * helper.In.x;
		helper.Out.y = m_Weight * helper.In.y;

		if (m_Overwrite == 0)
			helper.Out.z = m_Weight * helper.In.z * zf;
		else
			helper.Out.z = m_Weight * zf;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string x0 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string factor = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string overwrite = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string clamp = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x0_ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x1_ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x1_m_x0 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t zf = " << factor << " * (outPoint->m_ColorX - " << x0_ << ") / " << x1_m_x0 << ";\n"
		   << "\n"
		   << "\t\tif (" << clamp << " != 0)\n"
		   << "\t\t	zf = zf < 0 ? 0 : zf > 1 ? 1 : zf;\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * vIn.x;\n"
		   << "\t\tvOut.y = " << weight << " * vIn.y;\n"
		   << "\n"
		   << "\t\tif (" << overwrite << " == 0)\n"
		   << "\t\t	vOut.z = " << weight << " * vIn.z * zf;\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.z = " << weight << " * zf;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_X0_ = m_X0 < m_X1 ? m_X0 : m_X1;
		m_X1_ = m_X0 > m_X1 ? m_X0 : m_X1;
		m_X1_m_x0 = Zeps(m_X1_ - m_X0_);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X0, prefix + "dc_ztransl_x0", 0, eParamType::REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_X1, prefix + "dc_ztransl_x1", 1, eParamType::REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Factor, prefix + "dc_ztransl_factor", 1));
		m_Params.push_back(ParamWithName<T>(&m_Overwrite, prefix + "dc_ztransl_overwrite", 1, eParamType::INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Clamp, prefix + "dc_ztransl_clamp", 0, eParamType::INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_X0_, prefix + "dc_ztransl_x0_"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_X1_, prefix + "dc_ztransl_x1_"));
		m_Params.push_back(ParamWithName<T>(true, &m_X1_m_x0, prefix + "dc_ztransl_x1_m_x0"));
	}

private:
	T m_X0;
	T m_X1;
	T m_Factor;
	T m_Overwrite;
	T m_Clamp;
	T m_X0_;//Precalc.
	T m_X1_;
	T m_X1_m_x0;
};

#define SHAPE_SQUARE 0
#define SHAPE_DISC 1
#define SHAPE_BLUR 2

#define MAP_FLAT 0
#define MAP_SPHERICAL 1
#define MAP_HSPHERE 2
#define MAP_QSPHERE 3
#define MAP_BUBBLE 4
#define MAP_BUBBLE2 5

/// <summary>
/// dc_perlin.
/// </summary>
template <typename T>
class DCPerlinVariation : public ParametricVariation <T>
{
public:
	DCPerlinVariation(T weight = 1.0) : ParametricVariation<T>("dc_perlin", eVariationId::VAR_DC_PERLIN, weight)
	{
		Init();
	}

	PARVARCOPY(DCPerlinVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		v3T v;
		T vx, vy, col, r, theta, s, c, p, e;
		int t = 0, iShape = int(m_Shape), iMap = int(m_Map), iOctaves = int(m_Octaves), iBailout = int(m_SelectBailout);

		do
		{
			// Default edge value
			e = 0;

			// Assign vx, vy according to shape
			switch (iShape)
			{
				case SHAPE_SQUARE:
					vx = (1 + m_Edge) * (rand.Frand01<T>() - T(0.5));
					vy = (1 + m_Edge) * (rand.Frand01<T>() - T(0.5));
					r = SQR(vx) > SQR(vy) ? std::sqrt(SQR(vx)) : std::sqrt(SQR(vy));

					if (r > 1 - m_Edge)
						e = T(0.5) * (r - 1 + m_Edge) / m_Edge;

					break;

				case SHAPE_DISC:
					r = rand.Frand01<T>() + rand.Frand01<T>();
					r = (r > 1) ? 2 - r : r;
					r *= (1 + m_Edge);

					if (r > 1 - m_Edge)
						e = T(0.5) * (r - 1 + m_Edge) / m_Edge;

					theta = rand.Frand01<T>() * M_2PI;
					sincos(theta, &s, &c);
					vx = T(0.5) * r * s;
					vy = T(0.5) * r * c;
					break;

				case SHAPE_BLUR:
				default:
					r = (1 + m_Edge) * rand.Frand01<T>();

					if (r > 1 - m_Edge)
						e = T(0.5) * (r - 1 + m_Edge) / m_Edge;

					theta = rand.Frand01<T>() * M_2PI;
					sincos(theta, &s, &c);
					vx = T(0.5) * r * s;
					vy = T(0.5) * r * c;
					break;
			}

			// Assign V for noise vector position according to map
			switch (iMap)
			{
				case MAP_FLAT:
					v.x = m_Scale * vx;
					v.y = m_Scale * vy;
					v.z = m_Scale * m_Z;
					break;

				case MAP_SPHERICAL:
					r = 1 / Zeps<T>(SQR(vx) + SQR(vy));
					v.x = m_Scale * vx * r;
					v.y = m_Scale * vy * r;
					v.z = m_Scale * m_Z;
					break;

				case MAP_HSPHERE:
					r = 1 / (SQR(vx) + SQR(vy) + T(0.5));
					v.x = m_Scale * vx * r;
					v.y = m_Scale * vy * r;
					v.z = m_Scale * m_Z;
					break;

				case MAP_QSPHERE:
					r = 1 / (SQR(vx) + SQR(vy) + T(0.25));
					v.x = m_Scale * vx * r;
					v.y = m_Scale * vy * r;
					v.z = m_Scale * m_Z;
					break;

				case MAP_BUBBLE:
					r = T(0.25) - (SQR(vx) + SQR(vy));

					if (r < 0)
						r = std::sqrt(-r);
					else
						r = std::sqrt(r);

					v.x = m_Scale * vx;
					v.y = m_Scale * vy;
					v.z = m_Scale * (r + m_Z);
					break;

				case MAP_BUBBLE2:
				default:
					r = T(0.25) - (SQR(vx) + SQR(vy));

					if (r < 0)
						r = std::sqrt(-r);
					else
						r = std::sqrt(r);

					v.x = m_Scale * vx;
					v.y = m_Scale * vy;
					v.z = m_Scale * (2 * r + m_Z);
					break;
			}

			p = m_VarFuncs->PerlinNoise3D(v, m_Amps, m_Freqs, iOctaves);

			// Add edge effects
			if (p > 0)
				e = p * (1 + e * e * 20) + 2 * e;
			else
				e = p * (1 + e * e * 20) - 2 * e;
		}
		while ((e < m_NotchBottom || e > m_NotchTop) && t++ < iBailout);

		// Add blur effect to transform
		helper.Out.x = m_Weight * vx;
		helper.Out.y = m_Weight * vy;
		helper.Out.z = DefaultZ(helper);
		col = m_Centre + m_Range * p;
		outPoint.m_ColorX = col - Floor<T>(col);
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps", "Sqr", "SimplexNoise3D", "PerlinNoise3D" };
	}

	virtual vector<string> OpenCLGlobalDataNames() const override
	{
		return vector<string> { "NOISE_INDEX", "NOISE_POINTS" };
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber();
		string weight = WeightDefineString();
		string index = ss2.str() + "]";
		string shape = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string map = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string selectCentre = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string selectRange = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string centre = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string range = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string edge = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string octaves = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amps = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqs = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string selectBailout = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string notchBottom = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string notchTop = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal4 v;\n"
		   << "\t\treal_t vx, vy, col, r, theta, s, c, p, e;\n"
		   << "\t\tint t = 0, iShape = (int)" << shape << ", iMap = (int)" << map << ", iOctaves = (int)" << octaves << ", iBailout = (int)" << selectBailout << ";\n"
		   << "\n"
		   << "\t\tdo\n"
		   << "\t\t{\n"
		   << "\t\t	e = 0;\n"
		   << "\n"
		   << "\t\t	switch (iShape)\n"
		   << "\t\t	{\n"
		   << "\t\t		case " << SHAPE_SQUARE << ": \n"
		   << "\t\t			vx = (1 + " << edge << ") * (MwcNext01(mwc) - 0.5); \n"
		   << "\t\t			vy = (1 + " << edge << ") * (MwcNext01(mwc) - 0.5); \n"
		   << "\t\t			r = SQR(vx) > SQR(vy) ? sqrt(SQR(vx)) : sqrt(SQR(vy)); \n"
		   << "\n"
		   << "\t\t			if (r > 1 - " << edge << ")\n"
		   << "\t\t				e = 0.5 * (r - 1 + " << edge << ") / " << edge << "; \n"
		   << "\n"
		   << "\t\t			break; \n"
		   << "\n"
		   << "\t\t		case " << SHAPE_DISC << ": \n"
		   << "\t\t			r = MwcNext01(mwc) + MwcNext01(mwc); \n"
		   << "\t\t			r = (r > 1) ? 2 - r : r; \n"
		   << "\t\t			r *= (1 + " << edge << "); \n"
		   << "\n"
		   << "\t\t			if (r > 1 - " << edge << ")\n"
		   << "\t\t				e = 0.5 * (r - 1 + " << edge << ") / " << edge << "; \n"
		   << "\n"
		   << "\t\t			theta = MwcNext01(mwc) * M_2PI; \n"
		   << "\t\t			s = sincos(theta, &c); \n"
		   << "\t\t			vx = 0.5 * r * s; \n"
		   << "\t\t			vy = 0.5 * r * c; \n"
		   << "\t\t			break; \n"
		   << "\n"
		   << "\t\t		case " << SHAPE_BLUR << ": \n"
		   << "\t\t		default: \n"
		   << "\t\t			r = (1 + " << edge << ") * MwcNext01(mwc); \n"
		   << "\n"
		   << "\t\t			if (r > 1 - " << edge << ")\n"
		   << "\t\t				e = 0.5 * (r - 1 + " << edge << ") / " << edge << "; \n"
		   << "\n"
		   << "\t\t			theta = MwcNext01(mwc) * M_2PI; \n"
		   << "\t\t			s = sincos(theta, &c); \n"
		   << "\t\t			vx = 0.5 * r * s; \n"
		   << "\t\t			vy = 0.5 * r * c; \n"
		   << "\t\t			break; \n"
		   << "\t\t	}\n"
		   << "\n"
		   << "\t\t	switch (iMap)\n"
		   << "\t\t	{\n"
		   << "\t\t		case " << MAP_FLAT << ": \n"
		   << "\t\t			v.x = " << scale << " * vx; \n"
		   << "\t\t			v.y = " << scale << " * vy; \n"
		   << "\t\t			v.z = " << scale << " * " << z << "; \n"
		   << "\t\t			break; \n"
		   << "\n"
		   << "\t\t		case " << MAP_SPHERICAL << ": \n"
		   << "\t\t			r = 1 / Zeps(SQR(vx) + SQR(vy)); \n"
		   << "\t\t			v.x = " << scale << " * vx * r; \n"
		   << "\t\t			v.y = " << scale << " * vy * r; \n"
		   << "\t\t			v.z = " << scale << " * " << z << "; \n"
		   << "\t\t			break; \n"
		   << "\n"
		   << "\t\t		case " << MAP_HSPHERE << ": \n"
		   << "\t\t			r = 1 / (SQR(vx) + SQR(vy) + 0.5); \n"
		   << "\t\t			v.x = " << scale << " * vx * r; \n"
		   << "\t\t			v.y = " << scale << " * vy * r; \n"
		   << "\t\t			v.z = " << scale << " * " << z << "; \n"
		   << "\t\t			break; \n"
		   << "\n"
		   << "\t\t		case " << MAP_QSPHERE << ": \n"
		   << "\t\t			r = 1 / (SQR(vx) + SQR(vy) + 0.25); \n"
		   << "\t\t			v.x = " << scale << " * vx * r; \n"
		   << "\t\t			v.y = " << scale << " * vy * r; \n"
		   << "\t\t			v.z = " << scale << " * " << z << "; \n"
		   << "\t\t			break; \n"
		   << "\n"
		   << "\t\t		case " << MAP_BUBBLE << ": \n"
		   << "\t\t			r = 0.25 - (SQR(vx) + SQR(vy)); \n"
		   << "\n"
		   << "\t\t			if (r < 0)\n"
		   << "\t\t				r = sqrt(-r); \n"
		   << "\t\t			else\n"
		   << "\t\t				r = sqrt(r); \n"
		   << "\n"
		   << "\t\t			v.x = " << scale << " * vx; \n"
		   << "\t\t			v.y = " << scale << " * vy; \n"
		   << "\t\t			v.z = " << scale << " * (r + " << z << "); \n"
		   << "\t\t			break; \n"
		   << "\n"
		   << "\t\t		case " << MAP_BUBBLE2 << ": \n"
		   << "\t\t		default: \n"
		   << "\t\t			r = 0.25 - (SQR(vx) + SQR(vy)); \n"
		   << "\n"
		   << "\t\t			if (r < 0)\n"
		   << "\t\t				r = sqrt(-r); \n"
		   << "\t\t			else\n"
		   << "\t\t				r = sqrt(r); \n"
		   << "\n"
		   << "\t\t			v.x = " << scale << " * vx; \n"
		   << "\t\t			v.y = " << scale << " * vy; \n"
		   << "\t\t			v.z = " << scale << " * (2 * r + " << z << "); \n"
		   << "\t\t			break; \n"
		   << "\t\t	}\n"
		   << "\n"
		   << "\t\t	p = PerlinNoise3D(&v, globalShared + NOISE_INDEX, globalShared + NOISE_POINTS, " << amps << ", " << freqs << ", iOctaves); \n"
		   << "\n"
		   << "\t\t	if (p > 0)\n"
		   << "\t\t		e = p * (1 + e * e * 20) + 2 * e; \n"
		   << "\t\t	else\n"
		   << "\t\t		e = p * (1 + e * e * 20) - 2 * e; \n"
		   << "\t\t}\n"
		   << "\t\twhile ((e < " << notchBottom << " || e > " << notchTop << ") && t++ < iBailout); \n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * vx; \n"
		   << "\t\tvOut.y = " << weight << " * vy; \n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t\tcol = " << centre << " + " << range << " * p; \n"
		   << "\t\toutPoint->m_ColorX = col - floor(col); \n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_NotchBottom = m_SelectCentre - m_SelectRange;
		m_NotchBottom = (m_NotchBottom > T(0.75)) ? T(0.75) : m_NotchBottom;
		m_NotchBottom = (m_NotchBottom < -2) ? -3 : m_NotchBottom;
		m_NotchTop = m_SelectCentre + m_SelectRange;
		m_NotchTop = (m_NotchTop < T(-0.75)) ? T(-0.75) : m_NotchTop;
		m_NotchTop = (m_NotchTop > 3) ? 3 : m_NotchTop;
	}
protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.reserve(15);
		m_Params.push_back(ParamWithName<T>(&m_Shape, prefix + "dc_perlin_shape", 0, eParamType::INTEGER, 0, 2));
		m_Params.push_back(ParamWithName<T>(&m_Map, prefix + "dc_perlin_map", 0, eParamType::INTEGER, 0, 5));
		m_Params.push_back(ParamWithName<T>(&m_SelectCentre, prefix + "dc_perlin_select_centre", 0, eParamType::REAL, -1, 1));
		m_Params.push_back(ParamWithName<T>(&m_SelectRange, prefix + "dc_perlin_select_range", 1, eParamType::REAL, T(0.1), 2));
		m_Params.push_back(ParamWithName<T>(&m_Centre, prefix + "dc_perlin_centre", T(0.25)));
		m_Params.push_back(ParamWithName<T>(&m_Range, prefix + "dc_perlin_range", T(0.25)));
		m_Params.push_back(ParamWithName<T>(&m_Edge, prefix + "dc_perlin_edge"));
		m_Params.push_back(ParamWithName<T>(&m_Scale, prefix + "dc_perlin_scale", 1));
		m_Params.push_back(ParamWithName<T>(&m_Octaves, prefix + "dc_perlin_octaves", 2, eParamType::INTEGER, 1, 5));
		m_Params.push_back(ParamWithName<T>(&m_Amps, prefix + "dc_perlin_amps", 2));
		m_Params.push_back(ParamWithName<T>(&m_Freqs, prefix + "dc_perlin_freqs", 2));
		m_Params.push_back(ParamWithName<T>(&m_Z, prefix + "dc_perlin_z"));
		m_Params.push_back(ParamWithName<T>(&m_SelectBailout, prefix + "dc_perlin_select_bailout", 10, eParamType::INTEGER, 2, 1000));
		m_Params.push_back(ParamWithName<T>(true, &m_NotchBottom, prefix + "dc_perlin_notch_bottom"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_NotchTop, prefix + "dc_perlin_notch_top"));
	}
private:
	T m_Shape;
	T m_Map;
	T m_SelectCentre;
	T m_SelectRange;
	T m_Centre;
	T m_Range;
	T m_Edge;
	T m_Scale;
	T m_Octaves;
	T m_Amps;
	T m_Freqs;
	T m_Z;
	T m_SelectBailout;
	T m_NotchBottom;//Precalc.
	T m_NotchTop;
	shared_ptr<VarFuncs<T>> m_VarFuncs = VarFuncs<T>::Instance();
};

/// <summary>
/// randCubes.
/// </summary>
template <typename T>
class RandCubesVariation : public ParametricVariation<T>
{
public:
	RandCubesVariation(T weight = 1.0) : ParametricVariation<T>("randCubes", eVariationId::VAR_RAND_CUBES, weight)
	{
		Init();
	}

	PARVARCOPY(RandCubesVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		auto blockx = Floor<T>(std::log(rand.Frand01<T>()) * (rand.RandBit() ? m_Spread : -m_Spread));
		auto blocky = Floor<T>(std::log(rand.Frand01<T>()) * (rand.RandBit() ? m_Spread : -m_Spread));
		T z = VarFuncs<T>::Hash(int(blockx * m_Seed)) + VarFuncs<T>::Hash(int(blockx + blocky * m_Seed)); //random height for each block

		if (m_VarType == eVariationType::VARTYPE_REG)
		{
			outPoint.m_X = 0;//This variation intentionally assigns instead of summing.
			outPoint.m_Y = 0;
			outPoint.m_Z = 0;
		}

		helper.Out.x = (blockx * m_Density + rand.Frand01<T>()) * m_BlockSize;
		helper.Out.y = (blocky * m_Density + rand.Frand01<T>()) * m_BlockSize;
		helper.Out.z = m_BlockHeight * z * std::pow(rand.Frand01<T>(), T(0.125)); //fade out down
		outPoint.m_ColorX = z / 2;//block height -> palette location
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string blocksize = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blockheight = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spread = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string density = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t blockx = floor(log(MwcNext01(mwc)) * ((MwcNext(mwc) & 1) ? " << spread << " : -" << spread << "));\n"
		   << "\t\treal_t blocky = floor(log(MwcNext01(mwc)) * ((MwcNext(mwc) & 1) ? " << spread << " : -" << spread << "));\n"
		   << "\t\treal_t z = Hash(blockx * " << seed << ") + Hash(fma(blocky, " << seed << ", blockx));\n";

		if (m_VarType == eVariationType::VARTYPE_REG)
		{
			ss << "\t\toutPoint->m_X = 0;\n"
			   << "\t\toutPoint->m_Y = 0;\n"
			   << "\t\toutPoint->m_Z = 0;\n";
		}

		ss << "\t\tvOut.x = fma(blockx, " << density << ", MwcNext01(mwc)) * " << blocksize << ";\n"
		   << "\t\tvOut.y = fma(blocky, " << density << ", MwcNext01(mwc)) * " << blocksize << ";\n"
		   << "\t\tvOut.z = " << blockheight << " * z * pow(MwcNext01(mwc), (real_t)(0.125));\n"
		   << "\t\toutPoint->m_ColorX = z / 2;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Hash" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_BlockSize, prefix + "randCubes_blockSize", 1));
		m_Params.push_back(ParamWithName<T>(&m_BlockHeight, prefix + "randCubes_blockHeight", 1));
		m_Params.push_back(ParamWithName<T>(&m_Spread, prefix + "randCubes_spread", 3));
		m_Params.push_back(ParamWithName<T>(&m_Seed, prefix + "randCubes_seed", 1, eParamType::INTEGER));
		m_Params.push_back(ParamWithName<T>(&m_Density, prefix + "randCubes_density", 1));
	}

private:
	T m_BlockSize;
	T m_BlockHeight;
	T m_Spread;
	T m_Seed;
	T m_Density;
};

/// <summary>
/// pixel_flow.
/// </summary>
template <typename T>
class PixelFlowVariation : public ParametricVariation<T>
{
public:
	PixelFlowVariation(T weight = 1.0) : ParametricVariation<T>("pixel_flow", eVariationId::VAR_PIXEL_FLOW, weight)
	{
		Init();
	}

	PARVARCOPY(PixelFlowVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xl, yl;
		sincos(m_Rad, &yl, &xl);
        auto blockx = int(Floor(helper.In.x * m_Width));//Calculate which block we're in.
		blockx += int(2 - 4 * VarFuncs<T>::Hash(int(blockx * m_Seed + 1)));//Varying width and length.
        auto blocky = int(Floor(helper.In.y * m_Width));
		blocky += int(2 - 4 * VarFuncs<T>::Hash(int(blocky * m_Seed + 1)));
		T fLen = (VarFuncs<T>::Hash(int(blocky + blockx * -m_Seed)) + VarFuncs<T>::Hash(int(blockx + blocky * m_Seed * T(0.5)))) * T(0.5); //doesnt matter just needs to be random enough
		T r01 = rand.Frand01<T>();
		T fade = fLen * r01 * r01 * r01 * r01;//Fading effect.
		helper.Out.x = m_Weight * m_Len * xl * fade;
		helper.Out.y = m_Weight * m_Len * yl * fade;
		helper.Out.z = DefaultZ(helper);

		if (m_EnableDC)
			outPoint.m_ColorX = r01;//Direct color.
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string len   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string width = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dc    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rad   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t xl, yl;\n"
		   << "\t\tyl = sincos(" << rad << ", &xl);\n"
		   << "\t\tint blockx = (int)floor(vIn.x * " << width << ");\n"
		   << "\t\tblockx += (int)(2 - 4 * Hash((int)(blockx * (int)" << seed << " + 1)));\n"
		   << "\t\tint blocky = (int)floor(vIn.y * " << width << ");\n"
		   << "\t\tblocky += (int)(2 - 4 * Hash((int)(blocky * (int)" << seed << " + 1)));\n"
		   << "\t\treal_t fLen = (Hash((int)(blocky + blockx * -(int)" << seed << ")) + Hash((int)(blockx + blocky * (int)" << seed << " * (real_t)0.5))) * (real_t)0.5;\n"
		   << "\t\treal_t r01 = MwcNext01(mwc);\n"
		   << "\t\treal_t fade = fLen * r01 * r01 * r01 * r01;\n"
		   << "\t\tvOut.x = " << weight << " * " << len << " * xl * fade;\n"
		   << "\t\tvOut.y = " << weight << " * " << len << " * yl * fade;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t\tif (" << dc << ")\n"
		   << "\t\t\toutPoint->m_ColorX = r01;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Rad = m_Angle * DEG_2_RAD_T;
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Hash" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "pixel_flow_angle", 90));
		m_Params.push_back(ParamWithName<T>(&m_Len, prefix + "pixel_flow_len", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_Width, prefix + "pixel_flow_width", 200));
		m_Params.push_back(ParamWithName<T>(&m_Seed, prefix + "pixel_flow_seed", 42, eParamType::INTEGER));
		m_Params.push_back(ParamWithName<T>(&m_EnableDC, prefix + "pixel_flow_enable_dc", 0, eParamType::INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Rad, prefix + "pixel_flow_rad"));//Precalc.
	}

private:
	T m_Angle;
	T m_Len;
	T m_Width;
	T m_Seed;
	T m_EnableDC;
	T m_Rad;//Precalc.
};

MAKEPREPOSTPARVAR(DCBubble, dc_bubble, DC_BUBBLE)
MAKEPREPOSTPARVAR(DCCarpet, dc_carpet, DC_CARPET)
MAKEPREPOSTPARVARASSIGN(DCCube, dc_cube, DC_CUBE, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(DCCylinder, dc_cylinder, DC_CYLINDER)
MAKEPREPOSTVAR(DCGridOut, dc_gridout, DC_GRIDOUT)
MAKEPREPOSTPARVAR(DCLinear, dc_linear, DC_LINEAR)
MAKEPREPOSTPARVAR(DCTriangle, dc_triangle, DC_TRIANGLE)
MAKEPREPOSTPARVAR(DCZTransl, dc_ztransl, DC_ZTRANSL)
MAKEPREPOSTPARVAR(DCPerlin, dc_perlin, DC_PERLIN)
MAKEPREPOSTPARVARASSIGN(RandCubes, randCubes, RAND_CUBES, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(PixelFlow, pixel_flow, PIXEL_FLOW)
}
