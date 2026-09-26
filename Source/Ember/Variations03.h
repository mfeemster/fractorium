#pragma once

#include "Variation.h"

namespace EmberNs
{
/// <summary>
/// Funnel.
/// </summary>
template <typename T>
class FunnelVariation : public ParametricVariation<T>
{
public:
	FunnelVariation(T weight = 1.0) : ParametricVariation<T>("funnel", eVariationId::VAR_FUNNEL, weight)
	{
		Init();
	}

	PARVARCOPY(FunnelVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T temp = 1 / Zeps(std::cos(helper.In.y)) + m_Effect * T(M_PI);
		helper.Out.x = m_Weight * (std::tanh(helper.In.x) * temp);
		helper.Out.y = m_Weight * (std::tanh(helper.In.y) * temp);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string effect = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t temp = fma(" << effect << ", MPI, (real_t)(1.0) / Zeps(cos(vIn.y)));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (tanh(vIn.x) * temp);\n"
		   << "\t\tvOut.y = " << weight << " * (tanh(vIn.y) * temp);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Effect, prefix + "funnel_effect", 8, eParamType::INTEGER));
	}

private:
	T m_Effect;
};

/// <summary>
/// Linear3D.
/// </summary>
template <typename T>
class Linear3DVariation : public Variation<T>
{
public:
	Linear3DVariation(T weight = 1.0) : Variation<T>("linear3D", eVariationId::VAR_LINEAR3D, weight) { }

	VARCOPY(Linear3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * helper.In.x;
		helper.Out.y = m_Weight * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * vIn.x;\n"
		   << "\t\tvOut.y = " << weight << " * vIn.y;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// PowBlock.
/// </summary>
template <typename T>
class PowBlockVariation : public ParametricVariation<T>
{
public:
	PowBlockVariation(T weight = 1.0) : ParametricVariation<T>("pow_block", eVariationId::VAR_POW_BLOCK, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(PowBlockVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = std::pow(helper.m_PrecalcSumSquares, m_Power * T(0.5)) * m_Weight;
		T ran = (helper.m_PrecalcAtanyx / Zeps(m_Denominator) + (m_Root * M_2PI * Floor<T>(rand.Frand01<T>() * m_Denominator) / Zeps(m_Denominator))) * m_Numerator;
		helper.Out.x = r2 * std::cos(ran);
		helper.Out.y = r2 * std::sin(ran);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string numerator = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string denominator = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string root = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string correctN = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string correctD = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r2 = pow(precalcSumSquares, " << power << " * (real_t)(0.5)) * " << weight << ";\n"
		   << "\t\treal_t ran = (precalcAtanyx / Zeps(" << denominator << ") + (" << root << " * M_2PI * floor(MwcNext01(mwc) * " << denominator << ") / Zeps(" << denominator << "))) * " << numerator << ";\n"
		   << "\n"
		   << "\t\tvOut.x = r2 * cos(ran);\n"
		   << "\t\tvOut.y = r2 * sin(ran);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

	virtual void Precalc() override
	{
		m_Power = m_Numerator / Zeps(m_Denominator * m_Correctn * (1 / m_Correctd));
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Numerator, prefix + "pow_block_numerator", 3));//Original used a prefix of pow_, which is incompatible with Ember's design.
		m_Params.push_back(ParamWithName<T>(&m_Denominator, prefix + "pow_block_denominator", 2));
		m_Params.push_back(ParamWithName<T>(&m_Root, prefix + "pow_block_root", 1));
		m_Params.push_back(ParamWithName<T>(&m_Correctn, prefix + "pow_block_correctn", 1));
		m_Params.push_back(ParamWithName<T>(&m_Correctd, prefix + "pow_block_correctd", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Power, prefix + "pow_block_power"));//Precalc.
	}

private:
	T m_Numerator;
	T m_Denominator;
	T m_Root;
	T m_Correctn;
	T m_Correctd;
	T m_Power;//Precalc.
};

/// <summary>
/// Squirrel.
/// </summary>
template <typename T>
class SquirrelVariation : public ParametricVariation<T>
{
public:
	SquirrelVariation(T weight = 1.0) : ParametricVariation<T>("squirrel", eVariationId::VAR_SQUIRREL, weight)
	{
		Init();
	}

	PARVARCOPY(SquirrelVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T u = std::sqrt(ClampGte0<T>(Zeps(m_A) * SQR(helper.In.x) + Zeps(m_B) * SQR(helper.In.y)));//Original did not clamp.
		helper.Out.x = std::cos(u) * SafeTan<T>(helper.In.x) * m_Weight;
		helper.Out.y = std::sin(u) * SafeTan<T>(helper.In.y) * m_Weight;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string a = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t u = sqrt(ClampGte(fma(Zeps(" << a << "), SQR(vIn.x), Zeps(" << b << ") * SQR(vIn.y)), (real_t)(0.0)));\n"
		   << "\n"
		   << "\t\tvOut.x = cos(u) * tan(vIn.x) * " << weight << ";\n"
		   << "\t\tvOut.y = sin(u) * tan(vIn.y) * " << weight << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "ClampGte", "Zeps" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "squirrel_a", 1));
		m_Params.push_back(ParamWithName<T>(&m_B, prefix + "squirrel_b", 1));
	}

private:
	T m_A;
	T m_B;
};

/// <summary>
/// Ennepers.
/// </summary>
template <typename T>
class EnnepersVariation : public Variation<T>
{
public:
	EnnepersVariation(T weight = 1.0) : Variation<T>("ennepers", eVariationId::VAR_ENNEPERS, weight) { }

	VARCOPY(EnnepersVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T inxsq = SQR(helper.In.x);
		T inysq = SQR(helper.In.y);
		helper.Out.x = m_Weight * (helper.In.x - ((inxsq * helper.In.x) / 3)) + helper.In.x * inysq;
		helper.Out.y = m_Weight * (helper.In.y - ((inysq * helper.In.y) / 3)) + helper.In.y * inxsq;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = fma(" << weight << ", (vIn.x - ((SQR(vIn.x) * vIn.x) / (real_t)(3.0))), vIn.x * SQR(vIn.y));\n"
		   << "\t\tvOut.y = fma(" << weight << ", (vIn.y - ((SQR(vIn.y) * vIn.y) / (real_t)(3.0))), vIn.y * SQR(vIn.x));\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// SphericalN.
/// </summary>
template <typename T>
class SphericalNVariation : public ParametricVariation<T>
{
public:
	SphericalNVariation(T weight = 1.0) : ParametricVariation<T>("SphericalN", eVariationId::VAR_SPHERICALN, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(SphericalNVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = Zeps(std::pow(helper.m_PrecalcSqrtSumSquares, m_Dist));
		intmax_t n = Floor<T>(m_Power * rand.Frand01<T>());
		T alpha = helper.m_PrecalcAtanyx + n * M_2PI / Zeps<T>(T(Floor<T>(m_Power)));
		T sina = std::sin(alpha);
		T cosa = std::cos(alpha);

		if (Compat::m_Compat)
		{
			helper.Out.x = m_Weight * cosa / r;
			helper.Out.y = m_Weight * sina / r;
		}
		else
		{
			helper.Out.x = m_Weight * sina / r;
			helper.Out.y = m_Weight * cosa / r;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t weight = " << weight << ";\n"
		   << "\t\treal_t power  = " << power << ";\n"
		   << "\t\treal_t r = Zeps(pow(precalcSqrtSumSquares, " << dist << "));\n"
		   << "\t\tint n = floor(power * MwcNext01(mwc));\n"
		   << "\t\treal_t alpha = fma(n, M_2PI / Zeps(floor(power)), precalcAtanyx);\n"
		   << "\t\treal_t sina = sin(alpha);\n"
		   << "\t\treal_t cosa = cos(alpha);\n"
		   << "\n";

		if (Compat::m_Compat)
		{
			ss << "\t\tvOut.x = weight * cosa / r;\n"
			   << "\t\tvOut.y = weight * sina / r;\n";
		}
		else
		{
			ss << "\t\tvOut.x = weight * sina / r;\n"
			   << "\t\tvOut.y = weight * cosa / r;\n";
		}

		ss << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "SphericalN_Power", 1));
		m_Params.push_back(ParamWithName<T>(&m_Dist, prefix + "SphericalN_Dist", 1));
	}

private:
	T m_Power;
	T m_Dist;
};

/// <summary>
/// Kaleidoscope.
/// </summary>
template <typename T>
class KaleidoscopeVariation : public ParametricVariation<T>
{
public:
	KaleidoscopeVariation(T weight = 1.0) : ParametricVariation<T>("Kaleidoscope", eVariationId::VAR_KALEIDOSCOPE, weight)
	{
		Init();
	}

	PARVARCOPY(KaleidoscopeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sin45 = std::sin(45 * DEG_2_RAD_T);//Was 45 radians? They probably meant to convert this from degrees.
		T cos45 = std::cos(45 * DEG_2_RAD_T);
		helper.Out.x = ((m_Rotate * helper.In.x) * cos45 - helper.In.y * sin45 + m_LineUp) + m_X;

		//The if function splits the plugin in two.
		if (helper.In.y > 0)
			helper.Out.y = ((m_Rotate * helper.In.y) * cos45 + helper.In.x * sin45 + m_Pull + m_LineUp) + m_Y;
		else
			helper.Out.y = (m_Rotate * helper.In.y) * cos45 + helper.In.x * sin45 - m_Pull - m_LineUp;

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string pull   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotate = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string lineUp = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t sin45 = sin(45 * DEG_2_RAD);\n"
		   << "\t\treal_t cos45 = cos(45 * DEG_2_RAD);\n"
		   << "\n"
		   << "\t\tvOut.x = fma(" << rotate << " * vIn.x, cos45, -(vIn.y * sin45) + " << lineUp << ") + " << x << ";\n"
		   << "\n"
		   << "\t\tif (vIn.y > 0)\n"
		   << "\t\t	vOut.y = fma(" << rotate << " * vIn.y, cos45, fma(vIn.x, sin45, " << pull << " + " << lineUp << ")) + " << y << ";\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = fma(" << rotate << " * vIn.y, cos45, fma(vIn.x, sin45, -" << pull << " - " << lineUp << "));\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Pull, prefix + "Kaleidoscope_pull"));
		m_Params.push_back(ParamWithName<T>(&m_Rotate, prefix + "Kaleidoscope_rotate", 1));
		m_Params.push_back(ParamWithName<T>(&m_LineUp, prefix + "Kaleidoscope_line_up", 1));
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "Kaleidoscope_x"));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "Kaleidoscope_y"));
	}

private:
	T m_Pull;//Pulls apart the 2 sections of the plugin.
	T m_Rotate;//Rotates both halves of the plugin.
	T m_LineUp;
	T m_X;//Changes x co-ordinates.
	T m_Y;//Changes y co-ordinates for 1 part of the plugin.
};

/// <summary>
/// GlynnSim1.
/// </summary>
template <typename T>
class GlynnSim1Variation : public ParametricVariation<T>
{
public:
	GlynnSim1Variation(T weight = 1.0) : ParametricVariation<T>("GlynnSim1", eVariationId::VAR_GLYNNSIM1, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(GlynnSim1Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x, y, z;

		if (helper.m_PrecalcSqrtSumSquares < m_Radius)//Object generation.
		{
			Circle(rand, &x, &y);
			helper.Out.x = m_Weight * x;
			helper.Out.y = m_Weight * y;
		}
		else
		{
			T alpha = std::abs(m_Radius / Zeps(helper.m_PrecalcSqrtSumSquares));//Original did not std::abs().

			if (rand.Frand01<T>() > m_Contrast * std::pow(alpha, m_Pow))
			{
				x = helper.In.x;
				y = helper.In.y;
			}
			else
			{
				auto a2 = SQR(alpha);
				x = a2 * helper.In.x;
				y = a2 * helper.In.y;
			}

			z = Sqr(x - m_X1) + Sqr(y - m_Y1);

			if (z < SQR(m_Radius1))//Object generation.
			{
				Circle(rand, &x, &y);
				helper.Out.x = m_Weight * x;
				helper.Out.y = m_Weight * y;
			}
			else
			{
				helper.Out.x = m_Weight * x;
				helper.Out.y = m_Weight * y;
			}
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string radius    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius1   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phi1      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thickness = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string contrast  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pow       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x1        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y1        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x, y, z;\n"
		   << "\n"
		   << "\t\tif (precalcSqrtSumSquares < " << radius << ")\n"
		   << "\t\t{\n"
		   << "\t\t	GlynnSim1Circle(&" << radius1 << ", &" << thickness << ", &" << x1 << ", &" << y1 << ", mwc, &x, &y);\n"
		   << "\t\t	vOut.x = " << weight << " * x;\n"
		   << "\t\t	vOut.y = " << weight << " * y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real_t alpha = fabs(" << radius << " / Zeps(precalcSqrtSumSquares));\n"
		   << "\n"
		   << "\t\t	if (MwcNext01(mwc) > " << contrast << " * pow(alpha, " << pow << "))\n"
		   << "\t\t	{\n"
		   << "\t\t		x = vIn.x;\n"
		   << "\t\t		y = vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		real_t a2 = SQR(alpha);\n"
		   << "\t\t		x = a2 * vIn.x;\n"
		   << "\t\t		y = a2 * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\n"
		   << "\t\t	z = Sqr(x - " << x1 << ") + Sqr(y - " << y1 << ");\n"
		   << "\n"
		   << "\t\t	if (z < SQR(" << radius1 << "))\n"
		   << "\t\t	{\n"
		   << "\t\t		GlynnSim1Circle(&" << radius1 << ", &" << thickness << ", &" << x1 << ", &" << y1 << ", mwc, &x, &y);\n"
		   << "\t\t		vOut.x = " << weight << " * x;\n"
		   << "\t\t		vOut.y = " << weight << " * y;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << weight << " * x;\n"
		   << "\t\t		vOut.y = " << weight << " * y;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Sqr", "Zeps" };
	}

	virtual string OpenCLFuncsString() const override
	{
		return
			"void GlynnSim1Circle(__constant real_t* radius1, __constant real_t* thickness, __constant real_t* x1, __constant real_t* y1, uint2* mwc, real_t* x, real_t* y)\n"
			"{\n"
			"	real_t r = *radius1 * (*thickness + ((real_t)(1.0) - *thickness) * MwcNext01(mwc));\n"
			"	real_t phi = M_2PI * MwcNext01(mwc);\n"
			"	real_t sinPhi = sin(phi);\n"
			"	real_t cosPhi = cos(phi);\n"
			"\n"
			"	*x = fma(r, cosPhi, *x1);\n"
			"	*y = fma(r, sinPhi, *y1);\n"
			"}\n"
			"\n";
	}

	virtual void Precalc() override
	{
		T val = DEG_2_RAD_T * m_Phi1;
		T sinPhi1 = std::sin(val);
		T cosPhi1 = std::cos(val);
		m_Pow = std::abs(m_Pow);
		m_X1 = m_Radius * cosPhi1;
		m_Y1 = m_Radius * sinPhi1;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "GlynnSim1_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_Radius1, prefix + "GlynnSim1_radius1", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_Phi1, prefix + "GlynnSim1_phi1"));
		m_Params.push_back(ParamWithName<T>(&m_Thickness, prefix + "GlynnSim1_thickness", T(0.1), eParamType::REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Contrast, prefix + "GlynnSim1_contrast", T(1.5)));
		m_Params.push_back(ParamWithName<T>(&m_Pow, prefix + "GlynnSim1_pow", T(0.5), eParamType::REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_X1, prefix + "GlynnSim1_x1"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Y1, prefix + "GlynnSim1_y1"));
	}

private:
	void Circle(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand, T* x, T* y)
	{
		T r = m_Radius1 * (m_Thickness + (1 - m_Thickness) * rand.Frand01<T>());
		T phi = M_2PI * rand.Frand01<T>();
		T sinPhi = std::sin(phi);
		T cosPhi = std::cos(phi);
		*x = r * cosPhi + m_X1;
		*y = r * sinPhi + m_Y1;
	}

	T m_Radius;
	T m_Radius1;
	T m_Phi1;
	T m_Thickness;
	T m_Contrast;
	T m_Pow;
	T m_X1;//Precalc.
	T m_Y1;
};

/// <summary>
/// GlynnSim2.
/// </summary>
template <typename T>
class GlynnSim2Variation : public ParametricVariation<T>
{
public:
	GlynnSim2Variation(T weight = 1.0) : ParametricVariation<T>("GlynnSim2", eVariationId::VAR_GLYNNSIM2, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(GlynnSim2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x, y;

		if (helper.m_PrecalcSqrtSumSquares < m_Radius)
		{
			Circle(rand, &x, &y);
			helper.Out.x = m_Weight * x;
			helper.Out.y = m_Weight * y;
		}
		else
		{
			T alpha = std::abs(m_Radius / Zeps(helper.m_PrecalcSqrtSumSquares));//Original did not std::abs().

			if (rand.Frand01<T>() > m_Contrast * std::pow(alpha, m_Pow))
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
			}
			else
			{
				helper.Out.x = m_Weight * SQR(alpha) * helper.In.x;
				helper.Out.y = m_Weight * SQR(alpha) * helper.In.y;
			}
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string radius = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thickness = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string contrast = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pow = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phi1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phi2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phi10 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phi20 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string gamma = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string delta = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x, y;\n"
		   << "\n"
		   << "\t\tif (precalcSqrtSumSquares < " << radius << ")\n"
		   << "\t\t{\n"
		   << "\t\t	GlynnSim2Circle(&" << radius << ", &" << thickness << ", &" << phi10 << ", &" << delta << ", &" << gamma << ", mwc, &x,&y);\n"
		   << "\t\t	vOut.x = " << weight << " * x;\n"
		   << "\t\t	vOut.y = " << weight << " * y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real_t alpha = fabs(" << radius << " / Zeps(precalcSqrtSumSquares));\n"
		   << "\n"
		   << "\t\t	if (MwcNext01(mwc) > " << contrast << " * pow(alpha, " << pow << "))\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t		vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << weight << " * SQR(alpha) * vIn.x;\n"
		   << "\t\t		vOut.y = " << weight << " * SQR(alpha) * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

	virtual string OpenCLFuncsString() const override
	{
		return
			"void GlynnSim2Circle(__constant real_t* radius, __constant real_t* thickness, __constant real_t* phi10, __constant real_t* delta, __constant real_t* gamma, uint2* mwc, real_t* x, real_t* y)\n"
			"{\n"
			"	real_t r = *radius + *thickness - *gamma * MwcNext01(mwc);\n"
			"	real_t phi = fma(*delta, MwcNext01(mwc), *phi10);\n"
			"	real_t sinPhi = sin(phi);\n"
			"	real_t cosPhi = cos(phi);\n"
			"\n"
			"	*x = r * cosPhi;\n"
			"	*y = r * sinPhi;\n"
			"}\n"
			"\n";
	}

	virtual void Precalc() override
	{
		m_Pow = std::abs(m_Pow);
		m_Phi10 = T(M_PI) * m_Phi1 / 180;
		m_Phi20 = T(M_PI) * m_Phi2 / 180;
		m_Gamma = m_Thickness * (2 * m_Radius + m_Thickness) / Zeps(m_Radius + m_Thickness);
		m_Delta = m_Phi20 - m_Phi10;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "GlynnSim2_radius", 1, eParamType::REAL, 0));
		m_Params.push_back(ParamWithName<T>(&m_Thickness, prefix + "GlynnSim2_thickness", T(0.1), eParamType::REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Contrast, prefix + "GlynnSim2_contrast", T(0.5), eParamType::REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Pow, prefix + "GlynnSim2_pow", T(1.5)));
		m_Params.push_back(ParamWithName<T>(&m_Phi1, prefix + "GlynnSim2_Phi1"));
		m_Params.push_back(ParamWithName<T>(&m_Phi2, prefix + "GlynnSim2_Phi2", 360));
		m_Params.push_back(ParamWithName<T>(true, &m_Phi10, prefix + "GlynnSim2_Phi10"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Phi20, prefix + "GlynnSim2_Phi20"));
		m_Params.push_back(ParamWithName<T>(true, &m_Gamma, prefix + "GlynnSim2_Gamma"));
		m_Params.push_back(ParamWithName<T>(true, &m_Delta, prefix + "GlynnSim2_Delta"));
	}

private:
	void Circle(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand, T* x, T* y)
	{
		T r = m_Radius + m_Thickness - m_Gamma * rand.Frand01<T>();
		T phi = m_Phi10 + m_Delta * rand.Frand01<T>();
		T sinPhi = std::sin(phi);
		T cosPhi = std::cos(phi);
		*x = r * cosPhi;
		*y = r * sinPhi;
	}

	T m_Radius;
	T m_Thickness;
	T m_Contrast;
	T m_Pow;
	T m_Phi1;
	T m_Phi2;
	T m_Phi10;//Precalc.
	T m_Phi20;
	T m_Gamma;
	T m_Delta;
};

/// <summary>
/// GlynnSim3.
/// </summary>
template <typename T>
class GlynnSim3Variation : public ParametricVariation<T>
{
public:
	GlynnSim3Variation(T weight = 1.0) : ParametricVariation<T>("GlynnSim3", eVariationId::VAR_GLYNNSIM3, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(GlynnSim3Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x, y;

		if (helper.m_PrecalcSqrtSumSquares < m_Radius1)
		{
			Circle(rand, &x, &y);
			helper.Out.x = m_Weight * x;
			helper.Out.y = m_Weight * y;
		}
		else
		{
			T alpha = std::abs(m_Radius / Zeps(helper.m_PrecalcSqrtSumSquares));//Original did not std::abs().

			if (rand.Frand01<T>() > m_Contrast * std::pow(alpha, m_Pow))
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
			}
			else
			{
				helper.Out.x = m_Weight * SQR(alpha) * helper.In.x;
				helper.Out.y = m_Weight * SQR(alpha) * helper.In.y;
			}
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string radius = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thickness = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thickness2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string contrast = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pow = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string gamma = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x, y;\n"
		   << "\n"
		   << "\t\tif (precalcSqrtSumSquares < " << radius1 << ")\n"
		   << "\t\t{\n"
		   << "\t\t	GlynnSim3Circle(&" << radius << ", &" << radius1 << ", &" << radius2 << ", &" << thickness << ", &" << gamma << ", mwc, &x,&y);\n"
		   << "\t\t	vOut.x = " << weight << " * x;\n"
		   << "\t\t	vOut.y = " << weight << " * y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t real_t alpha = fabs(" << radius << " / Zeps(precalcSqrtSumSquares));\n"
		   << "\n"
		   << "\t\t	if (MwcNext01(mwc) > " << contrast << " * pow(alpha, " << pow << "))\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t		vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << weight << " * SQR(alpha) * vIn.x;\n"
		   << "\t\t		vOut.y = " << weight << " * SQR(alpha) * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

	virtual string OpenCLFuncsString() const override
	{
		return
			"void GlynnSim3Circle(__constant real_t* radius, __constant real_t* radius1, __constant real_t* radius2, __constant real_t* thickness, __constant real_t* gamma, uint2* mwc, real_t* x, real_t* y)\n"
			"{\n"
			"	real_t r = *radius + *thickness - *gamma * MwcNext01(mwc);\n"
			"	real_t phi = M_2PI * MwcNext01(mwc);\n"
			"	real_t sinPhi = sin(phi);\n"
			"	real_t cosPhi = cos(phi);\n"
			"\n"
			"	if (MwcNext01(mwc) < *gamma)\n"
			"		r = *radius1;\n"
			"	else\n"
			"		r = *radius2;\n"
			"\n"
			"	*x = r * cosPhi;\n"
			"	*y = r * sinPhi;\n"
			"}\n"
			"\n";
	}

	virtual void Precalc() override
	{
		m_Radius1 = m_Radius + m_Thickness;
		m_Radius2 = SQR(m_Radius) / Zeps(m_Radius1);
		m_Gamma = m_Radius1 / Zeps(m_Radius1 + m_Radius2);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "GlynnSim3_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_Thickness, prefix + "GlynnSim3_thickness", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_Thickness2, prefix + "GlynnSim3_thickness2", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_Contrast, prefix + "GlynnSim3_contrast", T(0.5), eParamType::REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Pow, prefix + "GlynnSim3_pow", T(1.5)));
		m_Params.push_back(ParamWithName<T>(true, &m_Radius1, prefix + "GlynnSim3_radius1"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Radius2, prefix + "GlynnSim3_radius2"));
		m_Params.push_back(ParamWithName<T>(true, &m_Gamma, prefix + "GlynnSim3_Gamma"));
	}

private:
	void Circle(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand, T* x, T* y)
	{
		T r = m_Radius + m_Thickness - m_Gamma * rand.Frand01<T>();
		T phi = M_2PI * rand.Frand01<T>();
		T sinPhi = std::sin(phi);
		T cosPhi = std::cos(phi);

		if (rand.Frand01<T>() < m_Gamma)
			r = m_Radius1;
		else
			r = m_Radius2;

		*x = r * cosPhi;
		*y = r * sinPhi;
	}

	T m_Radius;
	T m_Thickness;
	T m_Thickness2;
	T m_Contrast;
	T m_Pow;
	T m_Radius1;//Precalc.
	T m_Radius2;
	T m_Gamma;
};

/// <summary>
/// GlynnSim4.
/// </summary>
template <typename T>
class GlynnSim4Variation : public ParametricVariation<T>
{
public:
	GlynnSim4Variation(T weight = 1.0) : ParametricVariation<T>("GlynnSim4", eVariationId::VAR_GLYNNSIM4, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(GlynnSim4Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.m_PrecalcSqrtSumSquares < m_Radius)
		{
			helper.Out.x = m_Weight * m_X;
			helper.Out.y = m_Weight * m_Y;
		}
		else
		{
			T alpha = std::abs(m_Radius / Zeps(helper.m_PrecalcSqrtSumSquares));

			if (rand.Frand01<T>() > m_Contrast * std::pow(alpha, m_Pow))
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
			}
			else
			{
				helper.Out.x = m_Weight * SQR(alpha) * helper.In.x;
				helper.Out.y = m_Weight * SQR(alpha) * helper.In.y;
			}
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string radius   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string contrast = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pow      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tif (precalcSqrtSumSquares < " << radius << ")\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * " << x << ";\n"
		   << "\t\t	vOut.y = " << weight << " * " << y << ";\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real_t alpha = fabs(" << radius << " / Zeps(precalcSqrtSumSquares));\n"
		   << "\n"
		   << "\t\t	if (MwcNext01(mwc) > " << contrast << " * pow(alpha, " << pow << "))\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t		vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << weight << " * SQR(alpha) * vIn.x;\n"
		   << "\t\t		vOut.y = " << weight << " * SQR(alpha) * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Radius,   prefix + "GlynnSim4_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_Contrast, prefix + "GlynnSim4_contrast", T(0.5), eParamType::REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Pow,      prefix + "GlynnSim4_pow", T(1.5)));
		m_Params.push_back(ParamWithName<T>(&m_X,        prefix + "GlynnSim4_X"));
		m_Params.push_back(ParamWithName<T>(&m_Y,        prefix + "GlynnSim4_Y"));
	}

private:
	T m_Radius;
	T m_Contrast;
	T m_Pow;
	T m_X;
	T m_Y;
};

/// <summary>
/// GlynnSim5.
/// </summary>
template <typename T>
class GlynnSim5Variation : public ParametricVariation<T>
{
public:
	GlynnSim5Variation(T weight = 1.0) : ParametricVariation<T>("GlynnSim5", eVariationId::VAR_GLYNNSIM5, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(GlynnSim5Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.m_PrecalcSqrtSumSquares < m_Radius1)
		{
			helper.Out.x = 0;
			helper.Out.y = 0;
		}
		else
		{
			T alpha = std::abs(m_Radius / Zeps(helper.m_PrecalcSqrtSumSquares));

			if (rand.Frand01<T>() > m_Contrast * std::pow(alpha, m_Pow))
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
			}
			else
			{
				helper.Out.x = m_Weight * SQR(alpha);
				helper.Out.y = m_Weight * SQR(alpha);
			}
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string radius = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thickness = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string contrast = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pow = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tif (precalcSqrtSumSquares < " << radius1 << ")\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = 0;\n"
		   << "\t\t	vOut.y = 0;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real_t alpha = fabs(" << radius << " / Zeps(precalcSqrtSumSquares));\n"
		   << "\n"
		   << "\t\t	if (MwcNext01(mwc) > " << contrast << " * pow(alpha, " << pow << "))\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t		vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << weight << " * SQR(alpha);\n"
		   << "\t\t		vOut.y = " << weight << " * SQR(alpha);\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

	virtual void Precalc() override
	{
		m_Radius1 = m_Radius + m_Thickness;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "GlynnSim5_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_Thickness, prefix + "GlynnSim5_thickness", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_Contrast, prefix + "GlynnSim5_contrast", T(0.5), eParamType::REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Pow, prefix + "GlynnSim5_pow", T(1.5)));
		m_Params.push_back(ParamWithName<T>(true, &m_Radius1, prefix + "GlynnSim5_radius1"));//Precalc.
	}

private:
	T m_Radius;
	T m_Thickness;
	T m_Contrast;
	T m_Pow;
	T m_Radius1;//Precalc.
};

/// <summary>
/// Starblur.
/// </summary>
template <typename T>
class StarblurVariation : public ParametricVariation<T>
{
public:
	StarblurVariation(T weight = 1.0) : ParametricVariation<T>("starblur", eVariationId::VAR_STARBLUR, weight)
	{
		Init();
	}

	PARVARCOPY(StarblurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T f = rand.Frand01<T>() * m_Power * 2;
		int intangle = int(f);
		T angle = T(intangle);
		f -= angle;
		T x = f * m_Length;
		T z = std::sqrt(1 + SQR(x) - 2 * x * std::cos(m_Alpha));

		if (intangle & 1)
			angle = M_2PI / m_Power * (intangle / 2) + std::asin(std::sin(m_Alpha) * x / z);
		else
			angle = M_2PI / m_Power * (intangle / 2) - std::asin(std::sin(m_Alpha) * x / z);

		z *= std::sqrt(rand.Frand01<T>());
		T temp = angle - T(M_PI_2);
		helper.Out.x = m_Weight * z * std::cos(temp);
		helper.Out.y = m_Weight * z * std::sin(temp);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string range  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string length = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string alpha  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t f = MwcNext01(mwc) * " << power << " * 2;\n"
		   << "\t\tint intangle = (int)(f);\n"
		   << "\t\treal_t angle = (real_t)intangle;\n"
		   << "\n"
		   << "\t\tf -= angle;\n"
		   << "\n"
		   << "\t\treal_t x = f * " << length << ";\n"
		   << "\t\treal_t z = sqrt(fma(x, x, (real_t)(1.0)) - (real_t)(2.0) * x * cos(" << alpha << "));\n"
		   << "\n"
		   << "\t\tif (intangle & 1)\n"
		   << "\t\t	angle = fma(M_2PI / " << power << ", (real_t)(intangle / 2), asin(sin(" << alpha << ") * x / z));\n"
		   << "\t\telse\n"
		   << "\t\t	angle = fma(M_2PI / " << power << ", (real_t)(intangle / 2), -asin(sin(" << alpha << ") * x / z));\n"
		   << "\n"
		   << "\t\tz *= sqrt(MwcNext01(mwc));\n"
		   << "\n"
		   << "\t\treal_t temp = angle - MPI2;\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * z * cos(temp);\n"
		   << "\t\tvOut.y = " << weight << " * z * sin(temp);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Alpha = T(M_PI) / m_Power;
		m_Length = std::sqrt(1 + SQR(m_Range) - 2 * m_Range * std::cos(m_Alpha));
		m_Alpha = std::asin(std::sin(m_Alpha) * m_Range / m_Length);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power,        prefix + "starblur_power", 5));
		m_Params.push_back(ParamWithName<T>(&m_Range,        prefix + "starblur_range", T(0.4016228317)));
		m_Params.push_back(ParamWithName<T>(true, &m_Length, prefix + "starblur_length"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Alpha,  prefix + "starblur_alpha"));
	}

private:
	T m_Power;
	T m_Range;
	T m_Length;//Precalc.
	T m_Alpha;
};

/// <summary>
/// Sineblur.
/// </summary>
template <typename T>
class SineblurVariation : public ParametricVariation<T>
{
public:
	SineblurVariation(T weight = 1.0) : ParametricVariation<T>("sineblur", eVariationId::VAR_SINEBLUR, weight)
	{
		Init();
	}

	PARVARCOPY(SineblurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T ang = rand.Frand01<T>() * M_2PI;
		T s = std::sin(ang);
		T c = std::cos(ang);
		T r = m_Weight * (m_Power == 1 ? std::acos(rand.Frand01<T>() * 2 - 1) / T(M_PI) : std::acos(std::exp(std::log(rand.Frand01<T>()) * m_Power) * 2 - 1) / T(M_PI));
		helper.Out.x = r * c;
		helper.Out.y = r * s;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t ang = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t s = sin(ang);\n"
		   << "\t\treal_t c = cos(ang);\n"
		   << "\t\treal_t r = " << weight << " * (" << power << " == 1 ? acos(MwcNext01(mwc) * 2 - 1) / MPI : acos(exp(log(MwcNext01(mwc)) * " << power << ") * 2 - 1) / MPI);\n"
		   << "\n"
		   << "\t\tvOut.x = r * c;\n"
		   << "\t\tvOut.y = r * s;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "sineblur_power", 1, eParamType::REAL, 0));
	}

private:
	T m_Power;
};

/// <summary>
/// Circleblur.
/// </summary>
template <typename T>
class CircleblurVariation : public Variation<T>
{
public:
	CircleblurVariation(T weight = 1.0) : Variation<T>("circleblur", eVariationId::VAR_CIRCLEBLUR, weight) { }

	VARCOPY(CircleblurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T rad = std::sqrt(rand.Frand01<T>());
		T temp = rand.Frand01<T>() * M_2PI;
		helper.Out.x = m_Weight * std::cos(temp) * rad;
		helper.Out.y = m_Weight * std::sin(temp) * rad;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t rad = sqrt(MwcNext01(mwc));\n"
		   << "\t\treal_t temp = MwcNext01(mwc) * M_2PI;\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * cos(temp) * rad;\n"
		   << "\t\tvOut.y = " << weight << " * sin(temp) * rad;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Depth.
/// </summary>
template <typename T>
class DepthVariation : public ParametricVariation<T>
{
public:
	DepthVariation(T weight = 1.0) : ParametricVariation<T>("depth", eVariationId::VAR_DEPTH, weight)
	{
		Init();
	}

	PARVARCOPY(DepthVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T coeff = std::abs(helper.In.z);

		if (coeff != 0 && m_Power != 1)
			coeff = std::exp(std::log(coeff) * m_Power);

		helper.Out.x = m_Weight * (helper.m_TransX + helper.In.x * coeff);
		helper.Out.y = m_Weight * (helper.m_TransY + helper.In.y * coeff);
		helper.Out.z = m_Weight * (helper.m_TransZ + helper.In.z * coeff);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t coeff = fabs(vIn.z);\n"
		   << "\n"
		   << "\t\tif (coeff != 0 && " << power << " != 1)\n"
		   << "\t\t	coeff = exp(log(coeff) * " << power << ");\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(vIn.x, coeff, transX);\n"
		   << "\t\tvOut.y = " << weight << " * fma(vIn.y, coeff, transY);\n"
		   << "\t\tvOut.z = " << weight << " * fma(vIn.z, coeff, transZ);\n"
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "depth_power", 1));
	}

private:
	T m_Power;
};

/// <summary>
/// CropN.
/// By zy0rg.
/// </summary>
template <typename T>
class CropNVariation : public ParametricVariation<T>
{
public:
	using Variation<T>::m_NeedPrecalcSumSquares;
	using Variation<T>::m_NeedPrecalcAngles;
	CropNVariation(T weight = 1.0) : ParametricVariation<T>("cropn", eVariationId::VAR_CROPN, weight, true, true, true, false, true)
	{
		Init();
		m_NeedPrecalcSumSquares = !Compat::m_Compat;
		m_NeedPrecalcAngles = Compat::m_Compat;
	}

	PARVARCOPY(CropNVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			T xang = (helper.m_PrecalcAtanyx + T(M_PI)) / m_Alpha;
			xang = (xang - int(xang)) * m_Alpha;
			xang = std::cos((xang < m_AlphaDiv2) ? xang : m_Alpha - xang);
			T xr = xang > 0 ? m_Radius / xang : 1;

			if ((helper.m_PrecalcSqrtSumSquares > xr) == (m_Power > 0))
			{
				if (m_Zero == 1)
				{
					helper.Out.x = helper.Out.y = 0;
				}
				else
				{
					T rdc = xr + (rand.Frand01<T>() * T(0.5) * m_ScatterDist);
					helper.Out.x = m_Weight * rdc * helper.m_PrecalcCosa;
					helper.Out.y = m_Weight * rdc * helper.m_PrecalcSina;
				}
			}
			else
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
			}
		}
		else
		{
			T r2 = helper.m_PrecalcSumSquares;
			T aModAlpha = std::fmod(helper.m_PrecalcAtanyx + T(M_PI), m_Alpha);
			T xang = std::cos(aModAlpha < m_AlphaDiv2 ? aModAlpha : m_Alpha - aModAlpha);
			T xr = xang > 0 ? m_Radius / Zeps(xang) : T(1);
			T xr2 = xr * xr;

			if ((r2 > xr2) == (m_Power > 0))
			{
				if (m_Zero == 0)
				{
					helper.Out.x = helper.Out.y = 0;
				}
				else
				{
					T temp = (xr + rand.Frand01<T>() * m_ScatterDist) * m_Weight / Zeps(std::sqrt(r2));
					helper.Out.x = temp * helper.In.x;
					helper.Out.y = temp * helper.In.y;
				}
			}
			else
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
			}
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index       = ss2.str();
		string weight      = WeightDefineString();
		string power       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scatterDist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string zero        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string workPower   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string alpha       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string alphaDiv2   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t weight = " << weight << ";\n"
		   << "\t\treal_t alpha = " << alpha << ";\n";

		if (Compat::m_Compat)
		{
			ss << "\t\treal_t xang = (precalcAtanyx + MPI) / alpha;\n"
			   << "\n"
			   << "\t\txang = (xang - (int) xang) * alpha;\n"
			   << "\t\txang = cos((xang < " << alphaDiv2 << ") ? xang : alpha - xang);\n"
			   << "\n"
			   << "\t\treal_t xr = xang > 0 ? " << radius << " / xang : 1;\n"
			   << "\n"
			   << "\t\tif ((precalcSqrtSumSquares > xr) == (" << power << " > 0))\n"
			   << "\t\t{\n"
			   << "\t\t	if (" << zero << " == 1)\n"
			   << "\t\t	{\n"
			   << "\t\t		vOut.x = vOut.y = 0;\n"
			   << "\t\t	}\n"
			   << "\t\t	else\n"
			   << "\t\t	{\n"
			   << "\t\t		real_t rdc = fma(MwcNext01(mwc), (real_t)(0.5) * " << scatterDist << ", xr);\n"
			   << "\n"
			   << "\t\t		vOut.x = weight * rdc * precalcCosa;\n"
			   << "\t\t		vOut.y = weight * rdc * precalcSina;\n"
			   << "\t\t	}\n"
			   << "\t\t}\n"
			   << "\t\telse\n"
			   << "\t\t{\n"
			   << "\t\t	vOut.x = weight * vIn.x;\n"
			   << "\t\t	vOut.y = weight * vIn.y;\n"
			   << "\t\t}\n"
			   << "\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
		}
		else
		{
			ss << "\t\treal_t r2 = precalcSumSquares;\n"
			   << "\t\treal_t aModAlpha = fmod(precalcAtanyx + MPI, alpha);\n"
			   << "\t\treal_t xang = cos(aModAlpha < " << alphaDiv2 << " ? aModAlpha : alpha - aModAlpha);\n"
			   << "\t\treal_t xr = xang > 0 ? " << radius << " / Zeps(xang) : (real_t)1.0;\n"
			   << "\t\treal_t xr2 = xr * xr;\n"
			   << "\n"
			   << "\t\tif ((r2 > xr2) == (" << power << " > 0))\n"
			   << "\t\t{\n"
			   << "\t\t	if (" << zero << " == 0)\n"
			   << "\t\t	{\n"
			   << "\t\t		vOut.x = vOut.y = 0;\n"
			   << "\t\t	}\n"
			   << "\t\t	else\n"
			   << "\t\t	{\n"
			   << "\t\t		real_t temp = fma(MwcNext01(mwc), " << scatterDist << ", xr) * weight / Zeps(sqrt(r2));\n"
			   << "\n"
			   << "\t\t		vOut.x = temp * vIn.x;\n"
			   << "\t\t		vOut.y = temp * vIn.y;\n"
			   << "\t\t	}\n"
			   << "\t\t}\n"
			   << "\t\telse\n"
			   << "\t\t{\n"
			   << "\t\t	vOut.x = weight * vIn.x;\n"
			   << "\t\t	vOut.y = weight * vIn.y;\n"
			   << "\t\t}\n"
			   << "\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
		}

		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		if (Compat::m_Compat)
			return vector<string> { };
		else
			return vector<string> { "Zeps" };
	}

	virtual void Precalc() override
	{
		if (Compat::m_Compat)
		{
			bool mode = m_Power > 0;
			m_WorkPower = mode ? m_Power : -m_Power;
			ClampGteRef<T>(m_WorkPower, 2);
			m_Alpha = M_2PI / m_WorkPower;
		}
		else
		{
			m_Alpha = M_2PI / std::max(T(2), std::abs(m_Power));
		}

		m_AlphaDiv2 = m_Alpha / 2;
		m_NeedPrecalcSumSquares = !Compat::m_Compat;
		m_NeedPrecalcAngles = Compat::m_Compat;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "cropn_power", 5));
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "cropn_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_ScatterDist, prefix + "cropn_scatterdist"));
		m_Params.push_back(ParamWithName<T>(&m_Zero, prefix + "cropn_zero"));
		m_Params.push_back(ParamWithName<T>(true, &m_WorkPower, prefix + "cropn_workpower"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Alpha, prefix + "cropn_alpha"));
		m_Params.push_back(ParamWithName<T>(true, &m_AlphaDiv2, prefix + "cropn_alpha_div_2"));
	}

private:
	T m_Power;
	T m_Radius;
	T m_ScatterDist;
	T m_Zero;
	T m_WorkPower;//Precalc.
	T m_Alpha;
	T m_AlphaDiv2;
};

/// <summary>
/// ShredRad.
/// </summary>
template <typename T>
class ShredRadVariation : public ParametricVariation<T>
{
public:
	ShredRadVariation(T weight = 1.0) : ParametricVariation<T>("shredrad", eVariationId::VAR_SHRED_RAD, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(ShredRadVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xang = (helper.m_PrecalcAtanyx + M_3PI + m_Alpha / 2) / m_Alpha;
		T zang = ((xang - int(xang)) * m_Width + int(xang)) * m_Alpha - T(M_PI) - m_Alpha / 2 * m_Width;
		helper.Out.x = m_Weight * helper.m_PrecalcSqrtSumSquares * std::cos(zang);
		helper.Out.y = m_Weight * helper.m_PrecalcSqrtSumSquares * std::sin(zang);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string n     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string width = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string alpha = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t xang = (precalcAtanyx + M_3PI + " << alpha << " / 2) / " << alpha << ";\n"
		   << "\t\treal_t zang = fma((xang - (int)xang) * " << width << " + (int)xang, " << alpha << ", -MPI) - " << alpha << " / 2 * " << width << ";\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * precalcSqrtSumSquares * cos(zang);\n"
		   << "\t\tvOut.y = " << weight << " * precalcSqrtSumSquares * sin(zang);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Alpha = M_2PI / m_N;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_N, prefix + "shredrad_n", 4, eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Width, prefix + "shredrad_width", T(0.5), eParamType::REAL, -1, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Alpha, prefix + "shredrad_alpha"));//Precalc.
	}

private:
	T m_N;
	T m_Width;
	T m_Alpha;//Precalc.
};

/// <summary>
/// Blob2.
/// </summary>
template <typename T>
class Blob2Variation : public ParametricVariation<T>
{
public:
	Blob2Variation(T weight = 1.0) : ParametricVariation<T>("blob2", eVariationId::VAR_BLOB2, weight, true, true, true, false, true)
	{
		Init();
	}

	PARVARCOPY(Blob2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.m_PrecalcSqrtSumSquares < m_Radius)
		{
			helper.Out.x = m_Weight * helper.In.x;
			helper.Out.y = m_Weight * helper.In.y;
		}
		else
		{
			T delta = (std::sin(helper.m_PrecalcAtanyx * m_N) + m_Symmetry) / m_DeltaHelp;
			T positive = 1 - T(delta < 0 ? 1 : 0) * 2;

			if (m_Mode != 0)
				delta = std::exp(m_Prescale * std::log(delta * positive)) * m_Postscale * m_Mode;
			else
				delta = std::exp(m_Prescale * std::log(delta * positive)) * m_Postscale * positive;

			T rad = m_Radius + (helper.m_PrecalcSqrtSumSquares - m_Radius) * delta;
			helper.Out.x = m_Weight * rad * helper.m_PrecalcCosa;
			helper.Out.y = m_Weight * rad * helper.m_PrecalcSina;
			helper.Out.z = m_Weight * helper.In.z;
			//helper.m_TransZ += m_Weight * outPoint.m_Z;//Original had this which is probably wrong.
		}
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index     = ss2.str();
		string weight    = WeightDefineString();
		string mode      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string prescale  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string postscale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string symmetry  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string comp      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dataHelp  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tif (precalcSqrtSumSquares < " << radius << ")\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t	vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real_t delta = (sin(precalcAtanyx * " << n << ") + " << symmetry << ") / " << dataHelp << ";\n"
		   << "\t\t	real_t positive = 1 - (real_t)(delta < 0 ? 1 : 0) * 2;\n"
		   << "\n"
		   << "\t\t	if (" << mode << " != 0)\n"
		   << "\t\t		delta = exp(" << prescale << " * log(delta * positive)) * " << postscale << " * " << mode << ";\n"
		   << "\t\t	else\n"
		   << "\t\t		delta = exp(" << prescale << " * log(delta * positive)) * " << postscale << " * positive;\n"
		   << "\n"
		   << "\t\t	real_t rad = fma(precalcSqrtSumSquares - " << radius << ", delta, " << radius << ");\n"
		   << "\n"
		   << "\t\t	vOut.x = " << weight << " * rad * precalcCosa;\n"
		   << "\t\t	vOut.y = " << weight << " * rad * precalcSina;\n"
		   << "\t\t	vOut.z = " << weight << " * vIn.z;\n"
		   //<< "\t\t	transZ += " << weight << " * outPoint->m_Z;\n"//Original had this which is probably wrong.
		   << "\t\t}\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_DeltaHelp = 1 + m_Compensation * m_Symmetry * (1 - (m_Symmetry < 0) * 2);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Mode, prefix + "blob2_mode", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_N, prefix + "blob2_n", 1));
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "blob2_radius"));
		m_Params.push_back(ParamWithName<T>(&m_Prescale, prefix + "blob2_prescale", 1));
		m_Params.push_back(ParamWithName<T>(&m_Postscale, prefix + "blob2_postscale", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Symmetry, prefix + "blob2_symmetry", 0, eParamType::REAL, -1, 1));
		m_Params.push_back(ParamWithName<T>(&m_Compensation, prefix + "blob2_compensation", 0));
		m_Params.push_back(ParamWithName<T>(true, &m_DeltaHelp, prefix + "blob2_deltahelp"));//Precalc.
	}

private:
	T m_Mode;
	T m_N;
	T m_Radius;
	T m_Prescale;
	T m_Postscale;
	T m_Symmetry;
	T m_Compensation;
	T m_DeltaHelp;//Precalc.
};

/// <summary>
/// Julia3D.
/// </summary>
template <typename T>
class Julia3DVariation : public ParametricVariation<T>
{
public:
	Julia3DVariation(T weight = 1.0) : ParametricVariation<T>("julia3D", eVariationId::VAR_JULIA3D, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Julia3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T z = helper.In.z / m_AbsN;
		T r = m_Weight * std::pow(helper.m_PrecalcSumSquares + SQR(z), m_Cn);
		T tmp = r * helper.m_PrecalcSqrtSumSquares;
		T ang = (helper.m_PrecalcAtanyx + M_2PI * rand.Rand(uint(m_AbsN))) / m_N;
		helper.Out.x = tmp * std::cos(ang);
		helper.Out.y = tmp * std::sin(ang);
		helper.Out.z = r * z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string n    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absn = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cn   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t z = vIn.z / " << absn << ";\n"
		   << "\t\treal_t r = " << weight << " * pow(fma(z, z, precalcSumSquares), " << cn << ");\n"
		   << "\t\treal_t tmp = r * precalcSqrtSumSquares;\n"
		   << "\t\treal_t ang = fma(M_2PI, (real_t)MwcNextRange(mwc, (uint)" << absn << "), precalcAtanyx) / " << n << ";\n"
		   << "\n"
		   << "\t\tvOut.x = tmp * cos(ang);\n"
		   << "\t\tvOut.y = tmp * sin(ang);\n"
		   << "\t\tvOut.z = r * z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_AbsN = std::abs(m_N);
		m_Cn = (1 / m_N - 1) / 2;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_N = T(rand.Rand(5) + 2);

		if (rand.Rand(2) == 0)
			m_N = -m_N;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_N, prefix + "julia3D_power", 2, eParamType::INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsN, prefix + "julia3D_absn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix + "julia3D_cn"));
	}

private:
	T m_N;
	T m_AbsN;//Precalc.
	T m_Cn;
};

/// <summary>
/// Julia3Dz.
/// </summary>
template <typename T>
class Julia3DzVariation : public ParametricVariation<T>
{
public:
	Julia3DzVariation(T weight = 1.0) : ParametricVariation<T>("julia3Dz", eVariationId::VAR_JULIA3DZ, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Julia3DzVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight * std::pow(helper.m_PrecalcSumSquares, m_Cn);
		T temp = (helper.m_PrecalcAtanyx + M_2PI * rand.Rand(uint(m_AbsN))) / m_N;
		helper.Out.x = r * std::cos(temp);
		helper.Out.y = r * std::sin(temp);
		helper.Out.z = r * helper.In.z / Zeps(helper.m_PrecalcSqrtSumSquares * m_AbsN);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string n = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absn = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cn = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = " << weight << " * pow(precalcSumSquares, " << cn << ");\n"
		   << "\t\treal_t temp = fma(M_2PI, (real_t)MwcNextRange(mwc, (uint)" << absn << "), precalcAtanyx) / " << n << ";\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(temp);\n"
		   << "\t\tvOut.y = r * sin(temp);\n"
		   << "\t\tvOut.z = r * vIn.z / Zeps(precalcSqrtSumSquares * " << absn << ");\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

	virtual void Precalc() override
	{
		m_AbsN = std::abs(m_N);
		m_Cn = 1 / m_N / 2;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_N = T(rand.Rand(5) + 2);

		if (rand.Rand(2) == 0)
			m_N = -m_N;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_N, prefix + "julia3Dz_power", 2, eParamType::INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsN, prefix + "julia3Dz_absn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix + "julia3Dz_cn"));
	}

private:
	T m_N;
	T m_AbsN;//Precalc.
	T m_Cn;
};

/// <summary>
/// LinearT.
/// </summary>
template <typename T>
class LinearTVariation : public ParametricVariation<T>
{
public:
	LinearTVariation(T weight = 1.0) : ParametricVariation<T>("linearT", eVariationId::VAR_LINEAR_T, weight)
	{
		Init();
	}

	PARVARCOPY(LinearTVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = VarFuncs<T>::SignNz(helper.In.x) * std::pow(std::abs(helper.In.x), m_PowX) * m_Weight;
		helper.Out.y = VarFuncs<T>::SignNz(helper.In.y) * std::pow(std::abs(helper.In.y), m_PowY) * m_Weight;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string powx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string powy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = SignNz(vIn.x) * pow(fabs(vIn.x), " << powx << ") * " << weight << ";\n"
		   << "\t\tvOut.y = SignNz(vIn.y) * pow(fabs(vIn.y), " << powy << ") * " << weight << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "SignNz" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_PowX, prefix + "linearT_powX", 1));//Original used a prefix of lT, which is incompatible with Ember's design.
		m_Params.push_back(ParamWithName<T>(&m_PowY, prefix + "linearT_powY", 1));
	}

private:
	T m_PowX;
	T m_PowY;
};

/// <summary>
/// LinearT3D.
/// </summary>
template <typename T>
class LinearT3DVariation : public ParametricVariation<T>
{
public:
	LinearT3DVariation(T weight = 1.0) : ParametricVariation<T>("linearT3D", eVariationId::VAR_LINEAR_T3D, weight)
	{
		Init();
	}

	PARVARCOPY(LinearT3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = T(helper.In.x < 0 ? -1 : 1) * std::pow(std::abs(helper.In.x), m_PowX) * m_Weight;
		helper.Out.y = T(helper.In.y < 0 ? -1 : 1) * std::pow(std::abs(helper.In.y), m_PowY) * m_Weight;
		helper.Out.z = T(helper.In.z < 0 ? -1 : 1) * std::pow(std::abs(helper.In.z), m_PowZ) * m_Weight;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string powx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string powy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string powz = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = (real_t)(vIn.x < 0 ? -1 : 1) * pow(fabs(vIn.x), " << powx << ") * " << weight << ";\n"
		   << "\t\tvOut.y = (real_t)(vIn.y < 0 ? -1 : 1) * pow(fabs(vIn.y), " << powy << ") * " << weight << ";\n"
		   << "\t\tvOut.z = (real_t)(vIn.z < 0 ? -1 : 1) * pow(fabs(vIn.z), " << powz << ") * " << weight << ";\n"
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_PowX, prefix + "linearT3D_powX", 1));
		m_Params.push_back(ParamWithName<T>(&m_PowY, prefix + "linearT3D_powY", 1));
		m_Params.push_back(ParamWithName<T>(&m_PowZ, prefix + "linearT3D_powZ", 1));
	}

private:
	T m_PowX;
	T m_PowY;
	T m_PowZ;
};

/// <summary>
/// Ovoid.
/// </summary>
template <typename T>
class OvoidVariation : public ParametricVariation<T>
{
public:
	OvoidVariation(T weight = 1.0) : ParametricVariation<T>("ovoid", eVariationId::VAR_OVOID, weight, true)
	{
		Init();
	}

	PARVARCOPY(OvoidVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight / Zeps(helper.m_PrecalcSumSquares);
		helper.Out.x = helper.In.x * r * m_X;
		helper.Out.y = helper.In.y * r * m_Y;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = " << weight << " / Zeps(precalcSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * r * " << x << ";\n"
		   << "\t\tvOut.y = vIn.y * r * " << y << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "ovoid_x", 1));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "ovoid_y", 1));
	}

private:
	T m_X;
	T m_Y;
};

/// <summary>
/// Ovoid3D.
/// </summary>
template <typename T>
class Ovoid3DVariation : public ParametricVariation<T>
{
public:
	Ovoid3DVariation(T weight = 1.0) : ParametricVariation<T>("ovoid3d", eVariationId::VAR_OVOID3D, weight, true)
	{
		Init();
	}

	PARVARCOPY(Ovoid3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight / Zeps(helper.m_PrecalcSumSquares + SQR(helper.In.z));
		helper.Out.x = helper.In.x * r * m_X;
		helper.Out.y = helper.In.y * r * m_Y;
		helper.Out.z = helper.In.z * r * m_Z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = " << weight << " / Zeps(fma(vIn.z, vIn.z, precalcSumSquares));\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * r * " << x << ";\n"
		   << "\t\tvOut.y = vIn.y * r * " << y << ";\n"
		   << "\t\tvOut.z = vIn.z * r * " << z << ";\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "ovoid3d_x", 1));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "ovoid3d_y", 1));
		m_Params.push_back(ParamWithName<T>(&m_Z, prefix + "ovoid3d_z", 1));
	}

private:
	T m_X;
	T m_Y;
	T m_Z;
};

/// <summary>
/// Spirograph.
/// </summary>
template <typename T>
class SpirographVariation : public ParametricVariation<T>
{
public:
	SpirographVariation(T weight = 1.0) : ParametricVariation<T>("Spirograph", eVariationId::VAR_SPIROGRAPH, weight)
	{
		Init();
	}

	PARVARCOPY(SpirographVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t = (m_TMax - m_TMin) * rand.Frand01<T>() + m_TMin;
		T y = (m_YMax - m_YMin) * rand.Frand01<T>() + m_YMin;
		T ab = m_A + m_B;
		T abdivbt = ab / m_B * t;
		T x1 = ab * std::cos(t) - m_C1 * std::cos(abdivbt);
		T y1 = ab * std::sin(t) - m_C2 * std::sin(abdivbt);
		helper.Out.x = m_Weight * (x1 + m_D * std::cos(t) + y);
		helper.Out.y = m_Weight * (y1 + m_D * std::sin(t) + y);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string a    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string tmin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ymin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string tmax = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ymax = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t t = fma(" << tmax << " - " << tmin << ", MwcNext01(mwc), " << tmin << ");\n"
		   << "\t\treal_t y = fma(" << ymax << " - " << ymin << ", MwcNext01(mwc), " << ymin << ");\n"
		   << "\t\treal_t ab = " << a << " + " << b << ";\n"
		   << "\t\treal_t abdivbt = ab / " << b << " * t;\n"
		   << "\t\treal_t x1 = fma(ab, cos(t), -(" << c1 << " * cos(abdivbt)));\n"
		   << "\t\treal_t y1 = fma(ab, sin(t), -(" << c2 << " * sin(abdivbt)));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(" << d << ", cos(t), x1 + y);\n"
		   << "\t\tvOut.y = " << weight << " * fma(" << d << ", sin(t), y1 + y);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "Spirograph_a", 3));
		m_Params.push_back(ParamWithName<T>(&m_B, prefix + "Spirograph_b", 2));
		m_Params.push_back(ParamWithName<T>(&m_D, prefix + "Spirograph_d", 1));
		m_Params.push_back(ParamWithName<T>(&m_TMin, prefix + "Spirograph_tmin", -1));
		m_Params.push_back(ParamWithName<T>(&m_YMin, prefix + "Spirograph_ymin", -1));
		m_Params.push_back(ParamWithName<T>(&m_TMax, prefix + "Spirograph_tmax", 1));
		m_Params.push_back(ParamWithName<T>(&m_YMax, prefix + "Spirograph_ymax", 1));
		m_Params.push_back(ParamWithName<T>(&m_C1, prefix + "Spirograph_c1", 0));
		m_Params.push_back(ParamWithName<T>(&m_C2, prefix + "Spirograph_c2", 0));
	}

private:
	T m_A;
	T m_B;
	T m_D;
	T m_TMin;
	T m_YMin;
	T m_TMax;
	T m_YMax;
	T m_C1;
	T m_C2;
};

/// <summary>
/// Petal.
/// </summary>
template <typename T>
class PetalVariation : public Variation<T>
{
public:
	PetalVariation(T weight = 1.0) : Variation<T>("petal", eVariationId::VAR_PETAL, weight) { }

	VARCOPY(PetalVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sinX = std::sin(helper.In.x);
		T cosX = std::cos(helper.In.x);
		T cosY = std::cos(helper.In.y);
		T bx = Cube(cosX * cosY);
		T by = Cube(sinX * cosY);
		helper.Out.x = m_Weight * cosX * bx;
		helper.Out.y = m_Weight * cosX * by;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t sinX = sin(vIn.x);\n"
		   << "\t\treal_t cosX = cos(vIn.x);\n"
		   << "\t\treal_t sinY = sin(vIn.y);\n"
		   << "\t\treal_t cosY = cos(vIn.y);\n"
		   << "\t\treal_t bx = Cube(cosX*cosY);\n"
		   << "\t\treal_t by = Cube(sinX*cosY);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * cosX * bx;\n"
		   << "\t\tvOut.y = " << weight << " * cosX * by;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Cube" };
	}
};

/// <summary>
/// Spher.
/// </summary>
template <typename T>
class SpherVariation : public Variation<T>
{
public:
	SpherVariation(T weight = 1.0) : Variation<T>("spher", eVariationId::VAR_SPHER, weight, true) { }

	VARCOPY(SpherVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = Zeps(helper.m_PrecalcSumSquares);

		if (r2 > 1)
		{
			if (rand.Frand01<T>() < 0.5)
			{
				helper.Out.x = (m_Weight / r2) * helper.In.x;
				helper.Out.y = (m_Weight / r2) * helper.In.y;
			}
			else
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
			}
		}
		else
		{
			helper.Out.x = 0;
			helper.Out.y = 0;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r2 = Zeps(precalcSumSquares);\n"
		   << "\n"
		   << "\t\tif (r2 > 1)\n"
		   << "\t\t{\n"
		   << "\t\t	if (MwcNext01(mwc) < 0.5)\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = (" << weight << " / r2) * vIn.x;\n"
		   << "\t\t		vOut.y = (" << weight << " / r2) * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t		vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = 0;\n"
		   << "\t\t	vOut.y = 0;\n"
		   << "\t\t}\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// RoundSpher.
/// </summary>
template <typename T>
class RoundSpherVariation : public Variation<T>
{
public:
	RoundSpherVariation(T weight = 1.0) : Variation<T>("roundspher", eVariationId::VAR_ROUNDSPHER, weight, true) { }

	VARCOPY(RoundSpherVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T e = 1 / Zeps(helper.m_PrecalcSumSquares) + SQR(T(M_2_PI));
		T temp = m_Weight / Zeps(helper.m_PrecalcSumSquares);
		helper.Out.x = m_Weight * (temp * helper.In.x / e);
		helper.Out.y = m_Weight * (temp * helper.In.y / e);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t e = fma(M2PI, M2PI, 1 / Zeps(precalcSumSquares));\n"
		   << "\n"
		   << "\t\treal_t temp = " << weight << " / Zeps(precalcSumSquares);\n"
		   << "\t\tvOut.x = " << weight << " * (temp * vIn.x / e);\n"
		   << "\t\tvOut.y = " << weight << " * (temp * vIn.y / e);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// roundSpher3D.
/// </summary>
template <typename T>
class RoundSpher3DVariation : public Variation<T>
{
public:
	RoundSpher3DVariation(T weight = 1.0) : Variation<T>("roundspher3D", eVariationId::VAR_ROUNDSPHER3D, weight, true, true) { }

	VARCOPY(RoundSpher3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T inZ, otherZ, tempTz, tempPz;
		inZ = helper.In.z;

		if (m_VarType == eVariationType::VARTYPE_PRE)
			otherZ = helper.m_TransZ;
		else
			otherZ = outPoint.m_Z;

		if (inZ == 0)
			tempTz = std::cos(helper.m_PrecalcSqrtSumSquares);
		else
			tempTz = helper.In.z;

		if (otherZ == 0)
		{
			tempPz = std::cos(helper.m_PrecalcSqrtSumSquares);

			if (m_VarType == eVariationType::VARTYPE_PRE)
				helper.m_TransZ = 0;
			else
				outPoint.m_Z = 0;
		}
		else
		{
			if (m_VarType == eVariationType::VARTYPE_PRE)
			{
				tempPz = helper.m_TransZ;
				helper.m_TransZ = 0;
			}
			else
			{
				tempPz = outPoint.m_Z;
				outPoint.m_Z = 0;
			}
		}

		T d = helper.m_PrecalcSumSquares + SQR(tempTz);
		T e = 1 / d + SQR(T(M_2_PI));
		helper.Out.x = m_Weight * (m_Weight / d * helper.In.x / e);
		helper.Out.y = m_Weight * (m_Weight / d * helper.In.y / e);
		helper.Out.z = tempPz + m_Weight * (m_Weight / d * tempTz / e);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t inZ, otherZ, tempTz, tempPz;\n"
		   << "\t\tinZ = vIn.z;\n"
		   << "\n";

		if (m_VarType == eVariationType::VARTYPE_PRE)
			ss << "\t\totherZ = transZ;\n";
		else
			ss << "\t\totherZ = outPoint->m_Z;\n";

		ss
				<< "\n"
				<< "\t\tif (inZ == 0)\n"
				<< "\t\t	tempTz = cos(precalcSqrtSumSquares);\n"
				<< "\t\telse\n"
				<< "\t\t	tempTz = vIn.z;\n"
				<< "\n"
				<< "\t\tif (otherZ == 0)\n"
				<< "\t\t{\n"
				<< "\t\t	tempPz = cos(precalcSqrtSumSquares);\n"
				<< "\n";

		if (m_VarType == eVariationType::VARTYPE_PRE)
			ss << "\t\t	transZ = 0;\n";
		else
			ss << "\t\t	outPoint->m_Z = 0;\n";

		ss
				<< "\t\t}\n"
				<< "\t\telse\n"
				<< "\t\t{\n";

		if (m_VarType == eVariationType::VARTYPE_PRE)
		{
			ss
					<< "\t\t		tempPz = transZ;\n"
					<< "\t\t		transZ = 0;\n";
		}
		else
		{
			ss
					<< "\t\t		tempPz = outPoint->m_Z;\n"
					<< "\t\t		outPoint->m_Z = 0;\n";
		}

		ss
				<< "\t\t}\n"
				<< "\n"
				<< "\t\treal_t d = fma(tempTz, tempTz, precalcSumSquares);\n"
				<< "\t\treal_t e = fma(M2PI, M2PI, 1 / d);\n"
				<< "\n"
				<< "\t\tvOut.x = " << weight << " * (" << weight << " / d * vIn.x / e);\n"
				<< "\t\tvOut.y = " << weight << " * (" << weight << " / d * vIn.y / e);\n"
				<< "\t\tvOut.z = tempPz + " << weight << " * (" << weight << " / d * tempTz / e);\n"
				<< "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// SpiralWing.
/// </summary>
template <typename T>
class SpiralWingVariation : public Variation<T>
{
public:
	SpiralWingVariation(T weight = 1.0) : Variation<T>("spiralwing", eVariationId::VAR_SPIRAL_WING, weight, true) { }

	VARCOPY(SpiralWingVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T d = Zeps(helper.m_PrecalcSumSquares);
		T c1 = Zeps(SQR(helper.In.x));
		T c2 = Zeps(SQR(helper.In.y));
		helper.Out.x = m_Weight * ((1 / d) * std::cos(c1) * std::sin(c2));
		helper.Out.y = m_Weight * ((1 / d) * std::sin(c1) * std::sin(c2));
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t d = Zeps(precalcSumSquares);\n"
		   << "\t\treal_t c1 = Zeps(SQR(vIn.x));\n"
		   << "\t\treal_t c2 = Zeps(SQR(vIn.y));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (((real_t)(1.0) / d) * cos(c1) * sin(c2));\n"
		   << "\t\tvOut.y = " << weight << " * (((real_t)(1.0) / d) * sin(c1) * sin(c2));\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Squarize.
/// </summary>
template <typename T>
class SquarizeVariation : public Variation<T>
{
public:
	SquarizeVariation(T weight = 1.0) : Variation<T>("squarize", eVariationId::VAR_SQUARIZE, weight, true, true, false, false, true) { }

	VARCOPY(SquarizeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanyx;

		if (a < 0)
			a += M_2PI;

		T p = 4 * helper.m_PrecalcSqrtSumSquares * a * T(M_1_PI);

		if (p <= 1 * helper.m_PrecalcSqrtSumSquares)
		{
			helper.Out.x = m_Weight * helper.m_PrecalcSqrtSumSquares;
			helper.Out.y = m_Weight * p;
		}
		else if (p <= 3 * helper.m_PrecalcSqrtSumSquares)
		{
			helper.Out.x = m_Weight * (2 * helper.m_PrecalcSqrtSumSquares - p);
			helper.Out.y = m_Weight * helper.m_PrecalcSqrtSumSquares;
		}
		else if (p <= 5 * helper.m_PrecalcSqrtSumSquares)
		{
			helper.Out.x = -(m_Weight * helper.m_PrecalcSqrtSumSquares);
			helper.Out.y = m_Weight * (4 * helper.m_PrecalcSqrtSumSquares - p);
		}
		else if (p <= 7 * helper.m_PrecalcSqrtSumSquares)
		{
			helper.Out.x = -(m_Weight * (6 * helper.m_PrecalcSqrtSumSquares - p));
			helper.Out.y = -(m_Weight * helper.m_PrecalcSqrtSumSquares);
		}
		else
		{
			helper.Out.x = m_Weight * helper.m_PrecalcSqrtSumSquares;
			helper.Out.y = -(m_Weight * (8 * helper.m_PrecalcSqrtSumSquares - p));
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanyx;\n"
		   << "\n"
		   << "\t\tif (a < 0)\n"
		   << "\t\t	a += M_2PI;\n"
		   << "\n"
		   << "\t\treal_t p = 4 * precalcSqrtSumSquares * a * M1PI;\n"
		   << "\n"
		   << "\t\tif (p <= 1 * precalcSqrtSumSquares)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * precalcSqrtSumSquares;\n"
		   << "\t\t	vOut.y = " << weight << " * p;\n"
		   << "\t\t}\n"
		   << "\t\telse if (p <= 3 * precalcSqrtSumSquares)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * (2 * precalcSqrtSumSquares - p);\n"
		   << "\t\t	vOut.y = " << weight << " * precalcSqrtSumSquares;\n"
		   << "\t\t}\n"
		   << "\t\telse if (p <= 5 * precalcSqrtSumSquares)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = -(" << weight << " * precalcSqrtSumSquares);\n"
		   << "\t\t	vOut.y = " << weight << " * (4 * precalcSqrtSumSquares - p);\n"
		   << "\t\t}\n"
		   << "\t\telse if (p <= 7 * precalcSqrtSumSquares)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = -(" << weight << " * (6 * precalcSqrtSumSquares - p));\n"
		   << "\t\t	vOut.y = -(" << weight << " * precalcSqrtSumSquares);\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * precalcSqrtSumSquares;\n"
		   << "\t\t	vOut.y = -(" << weight << " * (8 * precalcSqrtSumSquares - p));\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Sschecks.
/// </summary>
template <typename T>
class SschecksVariation : public ParametricVariation<T>
{
public:
	SschecksVariation(T weight = 1.0) : ParametricVariation<T>("sschecks", eVariationId::VAR_SSCHECKS, weight, true)
	{
		Init();
	}

	PARVARCOPY(SschecksVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T dx, dy, r = m_Weight / Zeps(helper.m_PrecalcSumSquares);
		int isXY = int(VarFuncs<T>::LRint(helper.In.x * m_InvSize) + VarFuncs<T>::LRint(helper.In.y * m_InvSize));

		if (isXY & 1)
		{
			dx = -m_X + m_Rand * rand.Frand01<T>();
			dy = -m_Y;
		}
		else
		{
			dx = m_X;
			dy = m_Y + m_Rand * rand.Frand01<T>();
		}

		helper.Out.x = m_Weight * (std::sin(helper.In.x) * r + dx);
		helper.Out.y = m_Weight * (std::sin(helper.In.y) * r + dy);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string size = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rand = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invSize = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		ss << "\t{\n"
		   << "\t\treal_t dx, dy, r = " << weight << " / Zeps(precalcSumSquares);\n"
		   << "\t\tint isXY = LRint(vIn.x * " << invSize << ") + LRint(vIn.y * " << invSize << ");\n"
		   << "\n"
		   << "\t\tif (isXY & 1)\n"
		   << "\t\t{\n"
		   << "\t\t	dx = fma(" << rand << ", MwcNext01(mwc), -" << x << ");\n"
		   << "\t\t	dy = -" << y << ";\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	dx = " << x << ";\n"
		   << "\t\t	dy = fma(" << rand << ", MwcNext01(mwc), " << y << ");\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(sin(vIn.x), r, dx);\n"
		   << "\t\tvOut.y = " << weight << " * fma(sin(vIn.y), r, dy);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "LRint", "Zeps" };
	}

	virtual void Precalc() override
	{
		m_InvSize = 1 / Zeps(m_Size);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "sschecks_x", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "sschecks_y", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Size, prefix + "sschecks_size", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Rand, prefix + "sschecks_rnd"));
		m_Params.push_back(ParamWithName<T>(true, &m_InvSize, prefix + "sschecks_inv_size"));//Precalc.
	}

private:
	T m_X;
	T m_Y;
	T m_Size;
	T m_Rand;
	T m_InvSize;//Precalc.
};

/// <summary>
/// PhoenixJulia.
/// </summary>
template <typename T>
class PhoenixJuliaVariation : public ParametricVariation<T>
{
public:
	PhoenixJuliaVariation(T weight = 1.0) : ParametricVariation<T>("phoenix_julia", eVariationId::VAR_PHOENIX_JULIA, weight, true)
	{
		Init();
	}

	PARVARCOPY(PhoenixJuliaVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T preX = helper.In.x * (m_XDistort + 1);
		T preY = helper.In.y * (m_YDistort + 1);

		if (Compat::m_Compat)
		{
			T temp = std::atan2(preY, preX) * m_InvN + rand.Crand() * m_Inv2PiN;
			T r = m_Weight * std::pow(helper.m_PrecalcSumSquares, m_Cn);
			helper.Out.x = r * std::cos(temp);
			helper.Out.y = r * std::sin(temp);
			helper.Out.z = m_Weight * helper.In.z;
		}
		else
		{
            int root = int(rand.Frand01<T>() * m_Power);
			T a = std::atan2(preY, preX) * m_InvN + root * m_Inv2PiN;
			T r = m_Weight * std::pow(helper.m_PrecalcSumSquares, m_Cn);
			helper.Out.x = r * std::cos(a);
			helper.Out.y = r * std::sin(a);
			helper.Out.z = DefaultZ(helper);
		}
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xDistort = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yDistort = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cN       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invN     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string inv2PiN  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t weight = " << weight << ";\n"
		   << "\t\treal_t preX = vIn.x * (" << xDistort << " + 1);\n"
		   << "\t\treal_t preY = vIn.y * (" << yDistort << " + 1);\n";

		if (Compat::m_Compat)
		{
			ss << "\t\treal_t temp = fma(atan2(preY, preX), " << invN << ", MwcNextCrand(mwc) * " << inv2PiN << ");\n"
			   << "\t\treal_t r = weight * pow(precalcSumSquares, " << cN << ");\n"
			   << "\n"
			   << "\t\tvOut.x = r * cos(temp);\n"
			   << "\t\tvOut.y = r * sin(temp);\n"
			   << "\t\tvOut.z = weight * vIn.z;\n"
			   << "\t}\n";
		}
		else
		{
			ss << "\t\tint root = (int)(MwcNext01(mwc) * " << power << ");\n"
			   << "\t\treal_t temp = fma(atan2(preY, preX), " << invN << ", root * " << inv2PiN << ");\n"
			   << "\t\treal_t r = weight * pow(precalcSumSquares, " << cN << ");\n"
			   << "\n"
			   << "\t\tvOut.x = r * cos(temp);\n"
			   << "\t\tvOut.y = r * sin(temp);\n"
			   << "\t\tvOut.z = weight * vIn.z;\n"
			   << "\t}\n";
		}

		return ss.str();
	}

	virtual void Precalc() override
	{
		auto zp = Zeps(m_Power);
		m_InvN = m_Dist / zp;
		m_Inv2PiN = M_2PI / zp;
		m_Cn = m_InvN / 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "phoenix_julia_power", 2));//Original omitted _julia.
		m_Params.push_back(ParamWithName<T>(&m_Dist, prefix + "phoenix_julia_dist", 1));
		m_Params.push_back(ParamWithName<T>(&m_XDistort, prefix + "phoenix_julia_x_distort", T(-0.5)));//Original omitted phoenix_ prefix.
		m_Params.push_back(ParamWithName<T>(&m_YDistort, prefix + "phoenix_julia_y_distort"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix + "phoenix_julia_cn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_InvN, prefix + "phoenix_julia_invn"));
		m_Params.push_back(ParamWithName<T>(true, &m_Inv2PiN, prefix + "phoenix_julia_inv2pin"));
	}

private:
	T m_Power;
	T m_Dist;
	T m_XDistort;
	T m_YDistort;
	T m_Cn;//Precalc.
	T m_InvN;
	T m_Inv2PiN;
};

/// <summary>
/// Mobius.
/// </summary>
template <typename T>
class MobiusVariation : public ParametricVariation<T>
{
public:
	MobiusVariation(T weight = 1.0) : ParametricVariation<T>("Mobius", eVariationId::VAR_MOBIUS, weight)
	{
		Init();
	}

	PARVARCOPY(MobiusVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T uRe = m_Re_A * helper.In.x - m_Im_A * helper.In.y + m_Re_B;
		T uIm = m_Re_A * helper.In.y + m_Im_A * helper.In.x + m_Im_B;
		T vRe = m_Re_C * helper.In.x - m_Im_C * helper.In.y + m_Re_D;
		T vIm = m_Re_C * helper.In.y + m_Im_C * helper.In.x + m_Im_D;
		T vDenom = Zeps(SQR(vRe) + SQR(vIm));
		helper.Out.x = m_Weight * (uRe * vRe + uIm * vIm) / vDenom;
		helper.Out.y = m_Weight * (uIm * vRe - uRe * vIm) / vDenom;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string reA = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string imA = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string reB = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string imB = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string reC = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string imC = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string reD = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string imD = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t uRe = fma(" << reA << ", vIn.x, -(" << imA << " * vIn.y) + " << reB << ");\n"
		   << "\t\treal_t uIm = fma(" << reA << ", vIn.y, fma(" << imA << ", vIn.x, " << imB << "));\n"
		   << "\t\treal_t vRe = fma(" << reC << ", vIn.x, -(" << imC << " * vIn.y) + " << reD << ");\n"
		   << "\t\treal_t vIm = fma(" << reC << ", vIn.y, fma(" << imC << ", vIn.x, " << imD << "));\n"
		   << "\t\treal_t vDenom = Zeps(fma(vRe, vRe, SQR(vIm)));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(uRe, vRe, uIm * vIm) / vDenom;\n"
		   << "\t\tvOut.y = " << weight << " * fma(uIm, vRe, -(uRe * vIm)) / vDenom;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Re_A, prefix + "Mobius_Re_A", 1));//Original omitted Mobius_ prefix, which is incompatible with Ember's design.
		m_Params.push_back(ParamWithName<T>(&m_Im_A, prefix + "Mobius_Im_A"));
		m_Params.push_back(ParamWithName<T>(&m_Re_B, prefix + "Mobius_Re_B"));
		m_Params.push_back(ParamWithName<T>(&m_Im_B, prefix + "Mobius_Im_B"));
		m_Params.push_back(ParamWithName<T>(&m_Re_C, prefix + "Mobius_Re_C"));
		m_Params.push_back(ParamWithName<T>(&m_Im_C, prefix + "Mobius_Im_C"));
		m_Params.push_back(ParamWithName<T>(&m_Re_D, prefix + "Mobius_Re_D", 1));
		m_Params.push_back(ParamWithName<T>(&m_Im_D, prefix + "Mobius_Im_D"));
	}

private:
	T m_Re_A;
	T m_Im_A;
	T m_Re_B;
	T m_Im_B;
	T m_Re_C;
	T m_Im_C;
	T m_Re_D;
	T m_Im_D;
};

/// <summary>
/// MobiusN.
/// </summary>
template <typename T>
class MobiusNVariation : public ParametricVariation<T>
{
public:
	MobiusNVariation(T weight = 1.0) : ParametricVariation<T>("MobiusN", eVariationId::VAR_MOBIUSN, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(MobiusNVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		intmax_t n;
		T z = 4 * m_Dist / m_Power;
		T r = std::pow(helper.m_PrecalcSqrtSumSquares, z);
		T alpha = helper.m_PrecalcAtanyx * m_Power;
		T x = r * std::cos(alpha);
		T y = r * std::sin(alpha);
		T reU = m_Re_A * x - m_Im_A * y + m_Re_B;
		T imU = m_Re_A * y + m_Im_A * x + m_Im_B;
		T reV = m_Re_C * x - m_Im_C * y + m_Re_D;
		T imV = m_Re_C * y + m_Im_C * x + m_Im_D;
		T radV = reV * reV + imV * imV;
		x = (reU * reV + imU * imV) / radV;
		y = (imU * reV - reU * imV) / radV;
		z = 1 / z;
		r = std::pow(std::sqrt(SQR(x) + SQR(y)), z);
		n = Floor<T>(m_Power * rand.Frand01<T>());
		alpha = (std::atan2(y, x) + n * M_2PI) / Floor<T>(m_Power);
		helper.Out.x = m_Weight * r * std::cos(alpha);
		helper.Out.y = m_Weight * r * std::sin(alpha);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string reA   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string imA   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string reB   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string imB   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string reC   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string imC   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string reD   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string imD   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tint n;\n"
		   << "\n"
		   << "\t\treal_t z = (real_t)(4.0) * " << dist << " / " << power << ";\n"
		   << "\t\treal_t r = pow(precalcSqrtSumSquares, z);\n"
		   << "\t\treal_t alpha = precalcAtanyx * " << power << ";\n"
		   << "\t\treal_t x = r * cos(alpha);\n"
		   << "\t\treal_t y = r * sin(alpha);\n"
		   << "\t\treal_t reU = fma(" << reA << ", x, -(" << imA << " * y) + " << reB << ");\n"
		   << "\t\treal_t imU = fma(" << reA << ", y, fma(" << imA << ", x, " << imB << "));\n"
		   << "\t\treal_t reV = fma(" << reC << ", x, -(" << imC << " * y) + " << reD << ");\n"
		   << "\t\treal_t imV = fma(" << reC << ", y, fma(" << imC << ", x, " << imD << "));\n"
		   << "\t\treal_t radV = fma(reV, reV, SQR(imV));\n"
		   << "\n"
		   << "\t\tx = fma(reU, reV, imU * imV) / radV;\n"
		   << "\t\ty = fma(imU, reV, -(reU * imV)) / radV;\n"
		   << "\n"
		   << "\t\tz = (real_t)(1.0) / z;\n"
		   << "\t\tr = pow(sqrt(fma(x, x, SQR(y))), z);\n"
		   << "\t\tn = (int)floor(" << power << " * MwcNext01(mwc));\n"
		   << "\t\talpha = fma((real_t)n, M_2PI, atan2(y, x)) / floor(" << power << ");\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * r * cos(alpha);\n"
		   << "\t\tvOut.y = " << weight << " * r * sin(alpha);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		if (std::abs(m_Power) < 1)
			m_Power = 1;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Re_A, prefix + "MobiusNRe_A", 1));
		m_Params.push_back(ParamWithName<T>(&m_Im_A, prefix + "MobiusNIm_A"));
		m_Params.push_back(ParamWithName<T>(&m_Re_B, prefix + "MobiusNRe_B"));
		m_Params.push_back(ParamWithName<T>(&m_Im_B, prefix + "MobiusNIm_B"));
		m_Params.push_back(ParamWithName<T>(&m_Re_C, prefix + "MobiusNRe_C"));
		m_Params.push_back(ParamWithName<T>(&m_Im_C, prefix + "MobiusNIm_C"));
		m_Params.push_back(ParamWithName<T>(&m_Re_D, prefix + "MobiusNRe_D", 1));
		m_Params.push_back(ParamWithName<T>(&m_Im_D, prefix + "MobiusNIm_D"));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "MobiusN_Power", 2));
		m_Params.push_back(ParamWithName<T>(&m_Dist, prefix + "MobiusN_Dist", 1));
	}

private:
	T m_Re_A;
	T m_Im_A;
	T m_Re_B;
	T m_Im_B;
	T m_Re_C;
	T m_Im_C;
	T m_Re_D;
	T m_Im_D;
	T m_Power;
	T m_Dist;
};

/// <summary>
/// mobius_strip.
/// Original was "mobius", which conflicts with the other mobius variation.
/// Rename this mobius_strip for deconfliction, which breaks backward compatibility.
/// </summary>
template <typename T>
class MobiusStripVariation : public ParametricVariation<T>
{
public:
	MobiusStripVariation(T weight = 1.0) : ParametricVariation<T>("mobius_strip", eVariationId::VAR_MOBIUS_STRIP, weight)
	{
		Init();
	}

	PARVARCOPY(MobiusStripVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T s, t, mx, my, mz, rx, ry, rz;
		T deltaT, deltaS;
		t = helper.In.x;

		//Put t in range -rectX to +rectX, then map that to 0 - 2pi.
		if (m_RectX == 0)
		{
			t = 0;
		}
		else
		{
			deltaT = (t + m_RectX) / (2 * m_RectX);
			deltaT -= Floor<T>(deltaT);
			t = M_2PI * deltaT;
		}

		s = helper.In.y;

		//Put s in range -rectY to +rectY, then map that to -width to +width.
		if (m_RectY == 0)
		{
			s = 0;
		}
		else
		{
			deltaS = (s + m_RectY) / (2 * m_RectY);
			deltaS -= Floor<T>(deltaS);
			s = 2 * m_Width * deltaS - m_Width;
		}

		//Initial "object" co-ordinates.
		mx = (m_Radius + s * std::cos(t / 2)) * std::cos(t);
		my = (m_Radius + s * std::cos(t / 2)) * std::sin(t);
		mz = s * sin(t / 2);
		//Rotate around X axis (change y & z) and store temporarily in R variables.
		rx = mx;
		ry = my * m_RotyCos + mz * m_RotySin;
		rz = mz * m_RotyCos - my * m_RotySin;
		//Rotate around Y axis (change x & z) and store back in M variables.
		mx = rx * m_RotxCos - rz * m_RotxSin;
		my = ry;
		mz = rz * m_RotxCos + rx * m_RotxSin;
		//Add final values in to variations totals.
		helper.Out.x = m_Weight * mx;
		helper.Out.y = m_Weight * my;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string radius  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string width   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rectX   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rectY   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotateX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotateY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotxSin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotxCos = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotySin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotyCos = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t s, t, mx, my, mz, rx, ry, rz;\n"
		   << "\t\treal_t deltaT, deltaS;\n"
		   << "\n"
		   << "\t\tt = vIn.x;\n"
		   << "\n"
		   << "\t\tif (" << rectX << " == 0)\n"
		   << "\t\t{\n"
		   << "\t\t	t = 0;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	deltaT = (t + " << rectX << ") / (2 * " << rectX << ");\n"
		   << "\t\t	deltaT -= floor(deltaT);\n"
		   << "\t\t	t = M_2PI * deltaT;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\ts = vIn.y;\n"
		   << "\n"
		   << "\t\tif (" << rectY << " == 0)\n"
		   << "\t\t{\n"
		   << "\t\t	s = 0;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	deltaS = (s + " << rectY << ") / (2 * " << rectY << ");\n"
		   << "\t\t	deltaS -= floor(deltaS);\n"
		   << "\t\t	s = fma((real_t)(2.0) * " << width << ", deltaS, -" << width << ");\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tmx = fma(s, cos(t / (real_t)(2.0)), " << radius << ") * cos(t);\n"
		   << "\t\tmy = fma(s, cos(t / (real_t)(2.0)), " << radius << ") * sin(t);\n"
		   << "\t\tmz = s * sin(t / 2);\n"
		   << "\n"
		   << "\t\trx = mx;\n"
		   << "\t\try = fma(my, " << rotyCos << ", mz * " << rotySin << ");\n"
		   << "\t\trz = fma(mz, " << rotyCos << ", -(my * " << rotySin << "));\n"
		   << "\n"
		   << "\t\tmx = fma(rx, " << rotxCos << ", -(rz * " << rotxSin << "));\n"
		   << "\t\tmy = ry;\n"
		   << "\t\tmz = fma(rz, " << rotxCos << ", rx * " << rotxSin << ");\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * mx;\n"
		   << "\t\tvOut.y = " << weight << " * my;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_RotxSin = std::sin(m_RotateX * M_2PI);
		m_RotxCos = std::cos(m_RotateX * M_2PI);
		m_RotySin = std::sin(m_RotateY * M_2PI);
		m_RotyCos = std::cos(m_RotateY * M_2PI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "mobius_strip_radius", 2));
		m_Params.push_back(ParamWithName<T>(&m_Width, prefix + "mobius_strip_width", 1));
		m_Params.push_back(ParamWithName<T>(&m_RectX, prefix + "mobius_strip_rect_x", M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_RectY, prefix + "mobius_strip_rect_y", 1));
		m_Params.push_back(ParamWithName<T>(&m_RotateX, prefix + "mobius_strip_rotate_x"));
		m_Params.push_back(ParamWithName<T>(&m_RotateY, prefix + "mobius_strip_rotate_y"));
		m_Params.push_back(ParamWithName<T>(true, &m_RotxSin, prefix + "mobius_strip_rotxsin"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_RotxCos, prefix + "mobius_strip_rotxcos"));
		m_Params.push_back(ParamWithName<T>(true, &m_RotySin, prefix + "mobius_strip_rotysin"));
		m_Params.push_back(ParamWithName<T>(true, &m_RotyCos, prefix + "mobius_strip_rotycos"));
	}

private:
	T m_Radius;
	T m_Width;
	T m_RectX;
	T m_RectY;
	T m_RotateX;
	T m_RotateY;
	T m_RotxSin;//Precalc.
	T m_RotxCos;
	T m_RotySin;
	T m_RotyCos;
};

/// <summary>
/// Lissajous.
/// </summary>
template <typename T>
class LissajousVariation : public ParametricVariation<T>
{
public:
	LissajousVariation(T weight = 1.0) : ParametricVariation<T>("Lissajous", eVariationId::VAR_LISSAJOUS, weight)
	{
		Init();
	}

	PARVARCOPY(LissajousVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t = (m_Max - m_Min) * rand.Frand01<T>() + m_Min;
		T y = rand.Frand01<T>() - T(0.5);
		T x1 = sin(m_A * t + m_D);
		T y1 = sin(m_B * t);
		helper.Out.x = m_Weight * (x1 + m_C * t + m_E * y);
		helper.Out.y = m_Weight * (y1 + m_C * t + m_E * y);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string min = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string max = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string e   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t t = fma(" << max << " - " << min << ", MwcNext01(mwc), " << min << ");\n"
		   << "\t\treal_t y = MwcNext01(mwc) - (real_t)(0.5);\n"
		   << "\t\treal_t x1 = sin(fma(" << a << ", t, " << d << "));\n"
		   << "\t\treal_t y1 = sin(" << b << " * t);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(" << c << ", t, fma(" << e << ", y, x1));\n"
		   << "\t\tvOut.y = " << weight << " * fma(" << c << ", t, fma(" << e << ", y, y1));\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Min, prefix + "Lissajous_tmin", -T(M_PI)));
		m_Params.push_back(ParamWithName<T>(&m_Max, prefix + "Lissajous_tmax", T(M_PI)));
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "Lissajous_a", 3));
		m_Params.push_back(ParamWithName<T>(&m_B, prefix + "Lissajous_b", 2));
		m_Params.push_back(ParamWithName<T>(&m_C, prefix + "Lissajous_c"));
		m_Params.push_back(ParamWithName<T>(&m_D, prefix + "Lissajous_d"));
		m_Params.push_back(ParamWithName<T>(&m_E, prefix + "Lissajous_e"));
	}

private:
	T m_Min;
	T m_Max;
	T m_A;
	T m_B;
	T m_C;
	T m_D;
	T m_E;
};

/// <summary>
/// svf.
/// </summary>
template <typename T>
class SvfVariation : public ParametricVariation<T>
{
public:
	SvfVariation(T weight = 1.0) : ParametricVariation<T>("svf", eVariationId::VAR_SVF, weight)
	{
		Init();
	}

	PARVARCOPY(SvfVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T cn = std::cos(m_N * helper.In.y);
		T sx = std::sin(helper.In.x);
		T cx = std::cos(helper.In.x);
		T sy = std::sin(helper.In.y);
		T cy = std::cos(helper.In.y);
		helper.Out.x = m_Weight * (cy * (cn * cx));
		helper.Out.y = m_Weight * (cy * (cn * sx));
		helper.Out.z = m_Weight * (sy * cn);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string n = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t cn = cos(" << n << " * vIn.y);\n"
		   << "\t\treal_t sx = sin(vIn.x);\n"
		   << "\t\treal_t cx = cos(vIn.x);\n"
		   << "\t\treal_t sy = sin(vIn.y);\n"
		   << "\t\treal_t cy = cos(vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (cy * (cn * cx));\n"
		   << "\t\tvOut.y = " << weight << " * (cy * (cn * sx));\n"
		   << "\t\tvOut.z = " << weight << " * (sy * cn);\n"
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_N, prefix + "svf_n", 2));
	}

private:
	T m_N;
};

/// <summary>
/// target.
/// </summary>
template <typename T>
class TargetVariation : public ParametricVariation<T>
{
public:
	TargetVariation(T weight = 1.0) : ParametricVariation<T>("target", eVariationId::VAR_TARGET, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(TargetVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanyx;
		T t = std::log(helper.m_PrecalcSqrtSumSquares);

		if (t < 0)
			t -= m_SizeDiv2;

		t = fmod(std::abs(t), m_Size);

		if (t < m_SizeDiv2)
			a += m_Even;
		else
			a += m_Odd;

		helper.Out.x = helper.m_PrecalcSqrtSumSquares * std::cos(a);
		helper.Out.y = helper.m_PrecalcSqrtSumSquares * std::sin(a);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string even = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string odd = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string size = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sizeDiv2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanyx;\n"
		   << "\t\treal_t t = log(precalcSqrtSumSquares);\n"
		   << "\n"
		   << "\t\tif (t < (real_t)(0.0))\n"
		   << "\t\t	t -= " << sizeDiv2 << ";\n"
		   << "\n"
		   << "\t\tt = fmod(fabs(t), " << size << ");\n"
		   << "\n"
		   << "\t\tif (t < " << sizeDiv2 << ")\n"
		   << "\t\t	a += " << even << ";\n"
		   << "\t\telse\n"
		   << "\t\t	a += " << odd << ";\n"
		   << "\n"
		   << "\t\tvOut.x = precalcSqrtSumSquares * cos(a);\n"
		   << "\t\tvOut.y = precalcSqrtSumSquares * sin(a);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_SizeDiv2 = m_Size * T(0.5);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Even,           prefix + "target_even", 0, eParamType::REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Odd,            prefix + "target_odd", 0, eParamType::REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Size,           prefix + "target_size", 1, eParamType::REAL, EPS, TMAX));
		m_Params.push_back(ParamWithName<T>(true, &m_SizeDiv2, prefix + "target_size_2"));//Precalc.
	}

private:
	T m_Even;
	T m_Odd;
	T m_Size;
	T m_SizeDiv2;//Precalc.
};

/// <summary>
/// target0.
/// By Mark Faber.
/// </summary>
template <typename T>
class Target0Variation : public ParametricVariation<T>
{
public:
	Target0Variation(T weight = 1.0) : ParametricVariation<T>("target0", eVariationId::VAR_TARGET0, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Target0Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanyx;
		T t = std::log(helper.m_PrecalcSqrtSumSquares);

		if (t < 0)
			t -= m_SizeDiv2;

		t = fmod(std::abs(t), m_Size);

		if (t < m_SizeDiv2)
			a += m_Even;
		else
			a += m_Odd;

		helper.Out.x = helper.m_PrecalcSqrtSumSquares * std::cos(a) * m_Weight;
		helper.Out.y = helper.m_PrecalcSqrtSumSquares * std::sin(a) * m_Weight;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string even     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string odd      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string size     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sizeDiv2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanyx;\n"
		   << "\t\treal_t t = log(precalcSqrtSumSquares);\n"
		   << "\n"
		   << "\t\tif (t < (real_t)(0.0))\n"
		   << "\t\t	t -= " << sizeDiv2 << ";\n"
		   << "\n"
		   << "\t\tt = fmod(fabs(t), " << size << ");\n"
		   << "\n"
		   << "\t\tif (t < " << sizeDiv2 << ")\n"
		   << "\t\t	a += " << even << ";\n"
		   << "\t\telse\n"
		   << "\t\t	a += " << odd << ";\n"
		   << "\n"
		   << "\t\tvOut.x = precalcSqrtSumSquares * cos(a) * " << weight << ";\n"
		   << "\t\tvOut.y = precalcSqrtSumSquares * sin(a) * " << weight << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_SizeDiv2 = m_Size * T(0.5);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Even,           prefix + "target0_even", 0, eParamType::REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Odd,            prefix + "target0_odd", 0, eParamType::REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Size,           prefix + "target0_size", 0, eParamType::REAL, 0));
		m_Params.push_back(ParamWithName<T>(true, &m_SizeDiv2, prefix + "target0_size_2"));//Precalc.
	}

private:
	T m_Even;
	T m_Odd;
	T m_Size;
	T m_SizeDiv2;//Precalc.
};

/// <summary>
/// target2.
/// By Mark Faber.
/// </summary>
template <typename T>
class Target2Variation : public ParametricVariation<T>
{
public:
	Target2Variation(T weight = 1.0) : ParametricVariation<T>("target2", eVariationId::VAR_TARGET2, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Target2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanyx;
		T t = std::log(helper.m_PrecalcSqrtSumSquares);
		t = m_Tightness * std::log(helper.m_PrecalcSqrtSumSquares) + T(M_1_PI) * (a + T(M_PI));

		if (t < 0)
			t -= m_SizeDiv2;

		t = fmod(std::abs(t), m_Size);

		if (t < m_SizeDiv2)
			a += m_Even;
		else
			a += m_Odd;

		helper.Out.x = helper.m_PrecalcSqrtSumSquares * std::cos(a) * m_Weight;
		helper.Out.y = helper.m_PrecalcSqrtSumSquares * std::sin(a) * m_Weight;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string even       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string odd        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string size       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thightness = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sizeDiv2   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanyx;\n"
		   << "\t\treal_t t = log(precalcSqrtSumSquares);\n"
		   << "\t\tt = fma(" << thightness << ", log(precalcSqrtSumSquares), M1PI * (a + MPI));\n"
		   << "\n"
		   << "\t\tif (t < (real_t)(0.0))\n"
		   << "\t\t	t -= " << sizeDiv2 << ";\n"
		   << "\n"
		   << "\t\tt = fmod(fabs(t), " << size << ");\n"
		   << "\n"
		   << "\t\tif (t < " << sizeDiv2 << ")\n"
		   << "\t\t	a += " << even << ";\n"
		   << "\t\telse\n"
		   << "\t\t	a += " << odd << ";\n"
		   << "\n"
		   << "\t\tvOut.x = precalcSqrtSumSquares * cos(a) * " << weight << ";\n"
		   << "\t\tvOut.y = precalcSqrtSumSquares * sin(a) * " << weight << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_SizeDiv2 = m_Size * T(0.5);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Even,      prefix + "target2_even", 0, eParamType::REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Odd,       prefix + "target2_odd", 0, eParamType::REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Size,      prefix + "target2_size", 1, eParamType::REAL, EPS, TMAX));
		m_Params.push_back(ParamWithName<T>(&m_Tightness, prefix + "target2_thightness"));
		m_Params.push_back(ParamWithName<T>(true, &m_SizeDiv2, prefix + "target2_size_2"));//Precalc.
	}

private:
	T m_Even;
	T m_Odd;
	T m_Size;
	T m_Tightness;
	T m_SizeDiv2;//Precalc.
};

/// <summary>
/// taurus.
/// </summary>
template <typename T>
class TaurusVariation : public ParametricVariation<T>
{
public:
	TaurusVariation(T weight = 1.0) : ParametricVariation<T>("taurus", eVariationId::VAR_TAURUS, weight)
	{
		Init();
	}

	PARVARCOPY(TaurusVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sx = std::sin(helper.In.x);
		T cx = std::cos(helper.In.x);
		T sy = std::sin(helper.In.y);
		T cy = std::cos(helper.In.y);
		T ir = m_InvTimesR + (m_1MinusInv * (m_R * std::cos(m_N * helper.In.x)));
		helper.Out.x = m_Weight * (cx * (ir + sy));
		helper.Out.y = m_Weight * (sx * (ir + sy));
		helper.Out.z = m_Weight * (m_Sor * cy) + (m_1MinusSor * helper.In.y);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string r           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string inv         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sor         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invTimesR   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneMinusInv = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneMinusSor = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t sx = sin(vIn.x);\n"
		   << "\t\treal_t cx = cos(vIn.x);\n"
		   << "\t\treal_t sy = sin(vIn.y);\n"
		   << "\t\treal_t cy = cos(vIn.y);\n"
		   << "\t\treal_t ir = fma(" << oneMinusInv << ", " << r << " * cos(" << n << " * vIn.x), " << invTimesR << ");\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (cx * (ir + sy));\n"
		   << "\t\tvOut.y = " << weight << " * (sx * (ir + sy));\n"
		   << "\t\tvOut.z = fma(" << weight << ", " << sor << " * cy, " << oneMinusSor << " * vIn.y);\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_InvTimesR = m_Inv * m_R;
		m_1MinusInv = 1 - m_Inv;
		m_1MinusSor = 1 - m_Sor;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_R, prefix + "taurus_r", 3));
		m_Params.push_back(ParamWithName<T>(&m_N, prefix + "taurus_n", 5));
		m_Params.push_back(ParamWithName<T>(&m_Inv, prefix + "taurus_inv", T(1.5)));
		m_Params.push_back(ParamWithName<T>(&m_Sor, prefix + "taurus_sor", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_InvTimesR, prefix + "taurus_inv_times_r"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_1MinusInv, prefix + "taurus_1_minus_inv"));
		m_Params.push_back(ParamWithName<T>(true, &m_1MinusSor, prefix + "taurus_1_minus_sor"));
	}

private:
	T m_R;
	T m_N;
	T m_Inv;
	T m_Sor;
	T m_InvTimesR;//Precalc.
	T m_1MinusInv;
	T m_1MinusSor;
};

/// <summary>
/// collideoscope.
/// </summary>
template <typename T>
class CollideoscopeVariation : public ParametricVariation<T>
{
public:
	CollideoscopeVariation(T weight = 1.0) : ParametricVariation<T>("collideoscope", eVariationId::VAR_COLLIDEOSCOPE, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(CollideoscopeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		int alt;
		T a = helper.m_PrecalcAtanyx;
		T r = m_Weight * helper.m_PrecalcSqrtSumSquares;

		if (a >= 0)
		{
			alt = int(a * m_KnPi);

			if ((alt & 1) == 0)
				a = alt * m_PiKn + fmod(m_KaKn + a, m_PiKn);
			else
				a = alt * m_PiKn + fmod(-m_KaKn + a, m_PiKn);
		}
		else
		{
			alt = int(-a * m_KnPi);

			if ((alt & 1) == 1)
				a = -(alt * m_PiKn + fmod(-m_KaKn - a, m_PiKn));
			else
				a = -(alt * m_PiKn + fmod(m_KaKn - a, m_PiKn));
		}

		helper.Out.x = r * std::cos(a);
		helper.Out.y = r * std::sin(a);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string a = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string num = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ka = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string knpi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string kakn = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pikn = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tint alt;\n"
		   << "\t\treal_t a = precalcAtanyx;\n"
		   << "\t\treal_t r = " << weight << " * precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tif (a >= 0)\n"
		   << "\t\t{\n"
		   << "\t\t	alt = (int)(a * " << knpi << ");\n"
		   << "\n"
		   << "\t\t	if ((alt & 1) == 0)\n"
		   << "\t\t		a = fma((real_t)alt, " << pikn << ", fmod(" << kakn << " + a, " << pikn << "));\n"
		   << "\t\t	else\n"
		   << "\t\t		a = fma((real_t)alt, " << pikn << ", fmod(-" << kakn << " + a, " << pikn << "));\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	alt = (int)(-a * " << knpi << ");\n"
		   << "\n"
		   << "\t\t	if ((alt & 1) == 1)\n"
		   << "\t\t		a = -fma((real_t)alt, " << pikn << ", fmod(-" << kakn << " - a, " << pikn << "));\n"
		   << "\t\t	else\n"
		   << "\t\t		a = -fma((real_t)alt, " << pikn << ", fmod(" << kakn << " - a, " << pikn << "));\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Num = Zeps(m_Num);
		m_KnPi = m_Num * T(M_1_PI);
		m_PiKn = T(M_PI) / m_Num;
		m_Ka = T(M_PI) * m_A;
		m_KaKn = m_Ka / m_Num;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "collideoscope_a"));
		m_Params.push_back(ParamWithName<T>(&m_Num, prefix + "collideoscope_num", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Ka, prefix + "collideoscope_ka"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_KnPi, prefix + "collideoscope_kn_pi"));
		m_Params.push_back(ParamWithName<T>(true, &m_KaKn, prefix + "collideoscope_ka_kn"));
		m_Params.push_back(ParamWithName<T>(true, &m_PiKn, prefix + "collideoscope_pi_kn"));
	}

private:
	T m_A;
	T m_Num;
	T m_Ka;//Precalc.
	T m_KnPi;
	T m_KaKn;
	T m_PiKn;
};

/// <summary>
/// bMod.
/// </summary>
template <typename T>
class BModVariation : public ParametricVariation<T>
{
public:
	BModVariation(T weight = 1.0) : ParametricVariation<T>("bMod", eVariationId::VAR_BMOD, weight)
	{
		Init();
	}

	PARVARCOPY(BModVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xp1 = helper.In.x + 1;
		T inysq = SQR(helper.In.y);
		T tau = T(0.5) * (std::log(SQR(xp1) + inysq) - std::log(Sqr(helper.In.x - 1) + inysq));
		T sigma = T(M_PI) - std::atan2(helper.In.y, xp1) - std::atan2(helper.In.y, 1 - helper.In.x);

		if (tau < m_Radius && -tau < m_Radius)
			tau = fmod(tau + m_Radius + m_Distance * m_Radius, 2 * m_Radius) - m_Radius;

		T temp = Zeps(std::cosh(tau) - std::cos(sigma));
		helper.Out.x = m_Weight * std::sinh(tau) / temp;
		helper.Out.y = m_Weight * std::sin(sigma) / temp;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string radius = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t xp1 = vIn.x + (real_t)(1.0);\n"
		   << "\t\treal_t xm1 = vIn.x - (real_t)(1.0);\n"
		   << "\t\treal_t inysq = SQR(vIn.y);\n"
		   << "\t\treal_t tau = (real_t)(0.5) * (log(fma(xp1, xp1, inysq)) - log(fma(xm1, xm1, inysq)));\n"
		   << "\t\treal_t sigma = MPI - atan2(vIn.y, xp1) - atan2(vIn.y, (real_t)(1.0) - vIn.x);\n"
		   << "\n"
		   << "\t\tif (tau < " << radius << " && -tau < " << radius << ")\n"
		   << "\t\t	tau = fmod(fma(" << dist << ", " << radius << ", tau + " << radius << "), (real_t)(2.0) * " << radius << ") - " << radius << ";\n"
		   << "\n"
		   << "\t\treal_t temp = Zeps(cosh(tau) - cos(sigma));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * sinh(tau) / temp;\n"
		   << "\t\tvOut.y = " << weight << " * sin(sigma) / temp;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "bMod_radius", 1, eParamType::REAL, 0, TMAX));
		m_Params.push_back(ParamWithName<T>(&m_Distance, prefix + "bMod_distance", 0, eParamType::REAL_CYCLIC, 0, 2));
	}

private:
	T m_Radius;
	T m_Distance;
};

/// <summary>
/// bSwirl.
/// </summary>
template <typename T>
class BSwirlVariation : public ParametricVariation<T>
{
public:
	BSwirlVariation(T weight = 1.0) : ParametricVariation<T>("bSwirl", eVariationId::VAR_BSWIRL, weight)
	{
		Init();
	}

	PARVARCOPY(BSwirlVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xp1 = helper.In.x + 1;
		T inysq = SQR(helper.In.y);
		T tau = T(0.5) * (std::log(SQR(xp1) + inysq) - std::log(Sqr(helper.In.x - 1) + inysq));
		T sigma = T(M_PI) - std::atan2(helper.In.y, xp1) - std::atan2(helper.In.y, 1 - helper.In.x);
		sigma = sigma + tau * m_Out + m_In / Zeps(tau);
		T temp = Zeps(std::cosh(tau) - std::cos(sigma));
		helper.Out.x = m_Weight * std::sinh(tau) / temp;
		helper.Out.y = m_Weight * std::sin(sigma) / temp;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string in  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string out = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t xp1 = vIn.x + (real_t)(1.0);\n"
		   << "\t\treal_t xm1 = vIn.x - (real_t)(1.0);\n"
		   << "\t\treal_t inysq = SQR(vIn.y);\n"
		   << "\t\treal_t tau = (real_t)(0.5) * (log(fma(xp1, xp1, inysq)) - log(fma(xm1, xm1, inysq)));\n"
		   << "\t\treal_t sigma = MPI - atan2(vIn.y, xp1) - atan2(vIn.y, (real_t)(1.0) - vIn.x);\n"
		   << "\n"
		   << "\t\tsigma = sigma + tau * " << out << " + " << in << " / Zeps(tau);\n"
		   << "\n"
		   << "\t\treal_t temp = Zeps(cosh(tau) - cos(sigma));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * sinh(tau) / temp;\n"
		   << "\t\tvOut.y = " << weight << " * sin(sigma) / temp;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_In, prefix + "bSwirl_in"));
		m_Params.push_back(ParamWithName<T>(&m_Out, prefix + "bSwirl_out"));
	}

private:
	T m_In;
	T m_Out;
};

/// <summary>
/// bTransform.
/// </summary>
template <typename T>
class BTransformVariation : public ParametricVariation<T>
{
public:
	BTransformVariation(T weight = 1.0) : ParametricVariation<T>("bTransform", eVariationId::VAR_BTRANSFORM, weight)
	{
		Init();
	}

	PARVARCOPY(BTransformVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xp1 = helper.In.x + 1;
		T inysq = SQR(helper.In.y);
		T tau = T(0.5) * (std::log(Sqr(xp1) + inysq) - std::log(Sqr(helper.In.x - 1) + inysq)) / m_Power + m_Move;
		T sigma = T(M_PI) - std::atan2(helper.In.y, xp1) - std::atan2(helper.In.y, 1 - helper.In.x) + m_Rotate;
		sigma = sigma / m_Power + M_2PI / m_Power * Floor<T>(rand.Frand01<T>() * m_Power);

		if (helper.In.x >= 0)
			tau += m_Split;
		else
			tau -= m_Split;

		T temp = Zeps(std::cosh(tau) - std::cos(sigma));
		helper.Out.x = m_Weight * std::sinh(tau) / temp;
		helper.Out.y = m_Weight * std::sin(sigma) / temp;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string rotate = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string move   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string split  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t xp1 = vIn.x + (real_t)(1.0);\n"
		   << "\t\treal_t xm1 = vIn.x - (real_t)(1.0);\n"
		   << "\t\treal_t inysq = SQR(vIn.y);\n"
		   << "\t\treal_t tau = (real_t)(0.5) * (log(fma(xp1, xp1, inysq)) - log(fma(xm1, xm1, inysq))) / " << power << " + " << move << ";\n"
		   << "\t\treal_t sigma = MPI - atan2(vIn.y, vIn.x + (real_t)(1.0)) - atan2(vIn.y, (real_t)(1.0) - vIn.x) + " << rotate << ";\n"
		   << "\n"
		   << "\t\tsigma = sigma / " << power << " + M_2PI / " << power << " * floor(MwcNext01(mwc) * " << power << ");\n"
		   << "\n"
		   << "\t\tif (vIn.x >= 0)\n"
		   << "\t\t	tau += " << split << ";\n"
		   << "\t\telse\n"
		   << "\t\t	tau -= " << split << ";\n"
		   << "\n"
		   << "\t\treal_t temp = Zeps(cosh(tau) - cos(sigma));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * sinh(tau) / temp;\n"
		   << "\t\tvOut.y = " << weight << " * sin(sigma) / temp;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Rotate, prefix + "bTransform_rotate"));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "bTransform_power", 0, eParamType::REAL_NONZERO, 1, T(INT_MAX)));
		m_Params.push_back(ParamWithName<T>(&m_Move, prefix + "bTransform_move"));
		m_Params.push_back(ParamWithName<T>(&m_Split, prefix + "bTransform_split"));
	}

private:
	T m_Rotate;
	T m_Power;
	T m_Move;
	T m_Split;
};

/// <summary>
/// bCollide.
/// </summary>
template <typename T>
class BCollideVariation : public ParametricVariation<T>
{
public:
	BCollideVariation(T weight = 1.0) : ParametricVariation<T>("bCollide", eVariationId::VAR_BCOLLIDE, weight)
	{
		Init();
	}

	PARVARCOPY(BCollideVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xp1 = helper.In.x + 1;
		T inysq = SQR(helper.In.y);
		T tau = T(0.5) * (std::log(Sqr(xp1) + inysq) - std::log(Sqr(helper.In.x - 1) + inysq));
		T sigma = T(M_PI) - std::atan2(helper.In.y, xp1) - std::atan2(helper.In.y, 1 - helper.In.x);
		int alt = int(sigma * m_CnPi);

		if ((alt & 1) == 0)
			sigma = alt * m_PiCn + fmod(sigma + m_CaCn, m_PiCn);
		else
			sigma = alt * m_PiCn + fmod(sigma - m_CaCn, m_PiCn);

		T temp = Zeps(std::cosh(tau) - std::cos(sigma));
		helper.Out.x = m_Weight * std::sinh(tau) / temp;
		helper.Out.y = m_Weight * std::sin(sigma) / temp;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string a    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string num  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ca   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cnPi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string caCn = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string piCn = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t xp1 = vIn.x + (real_t)(1.0);\n"
		   << "\t\treal_t xm1 = vIn.x - (real_t)(1.0);\n"
		   << "\t\treal_t inysq = SQR(vIn.y);\n"
		   << "\t\treal_t tau = (real_t)(0.5) * (log(fma(xp1, xp1, inysq)) - log(fma(xm1, xm1, inysq)));\n"
		   << "\t\treal_t sigma = MPI - atan2(vIn.y, xp1) - atan2(vIn.y, (real_t)(1.0) - vIn.x);\n"
		   << "\t\tint alt = (int)(sigma * " << cnPi << ");\n"
		   << "\n"
		   << "\t\tif ((alt & 1) == 0)\n"
		   << "\t\t	sigma = fma(alt, " << piCn << ", fmod(sigma + " << caCn << ", " << piCn << "));\n"
		   << "\t\telse\n"
		   << "\t\t	sigma = fma(alt, " << piCn << ", fmod(sigma - " << caCn << ", " << piCn << "));\n"
		   << "\n"
		   << "\t\treal_t temp = Zeps(cosh(tau) - cos(sigma));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * sinh(tau) / temp;\n"
		   << "\t\tvOut.y = " << weight << " * sin(sigma) / temp;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

	virtual void Precalc() override
	{
		m_CnPi = m_Num * T(M_1_PI);
		m_PiCn = T(M_PI) / Zeps(m_Num);
		m_Ca = T(M_PI) * m_A;
		m_CaCn = m_Ca / Zeps(m_Num);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "bCollide_a", 0, eParamType::REAL_CYCLIC, -1, 1));
		m_Params.push_back(ParamWithName<T>(&m_Num, prefix + "bCollide_num", 1, eParamType::REAL, EPS, T(INT_MAX)));
		m_Params.push_back(ParamWithName<T>(true, &m_Ca, prefix + "bCollide_ca"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_CnPi, prefix + "bCollide_cn_pi"));
		m_Params.push_back(ParamWithName<T>(true, &m_CaCn, prefix + "bCollide_ca_cn"));
		m_Params.push_back(ParamWithName<T>(true, &m_PiCn, prefix + "bCollide_pi_cn"));
	}

private:
	T m_A;
	T m_Num;
	T m_Ca;//Precalc.
	T m_CnPi;
	T m_CaCn;
	T m_PiCn;
};

/// <summary>
/// eclipse.
/// </summary>
template <typename T>
class EclipseVariation : public ParametricVariation<T>
{
public:
	EclipseVariation(T weight = 1.0) : ParametricVariation<T>("eclipse", eVariationId::VAR_ECLIPSE, weight)
	{
		Init();
	}

	PARVARCOPY(EclipseVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x, c2;

		if (std::abs(helper.In.y) <= m_Weight)
		{
			c2 = std::sqrt(SQR(m_Weight) - SQR(helper.In.y));

			if (std::abs(helper.In.x) <= c2)
			{
				x = helper.In.x + m_Shift * m_Weight;

				if (std::abs(x) >= c2)
					helper.Out.x = -(m_Weight * helper.In.x);
				else
					helper.Out.x = m_Weight * x;
			}
			else
			{
				helper.Out.x = m_Weight * helper.In.x;
			}

			helper.Out.y = m_Weight * helper.In.y;
		}
		else
		{
			helper.Out.x = m_Weight * helper.In.x;
			helper.Out.y = m_Weight * helper.In.y;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string shift = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x, c2;\n"
		   << "\n"
		   << "\t\tif (fabs(vIn.y) <= " << weight << ")\n"
		   << "\t\t{\n"
		   << "\t\t	c2 = sqrt(fma(" << weight << ", " << weight << ", - SQR(vIn.y)));\n"
		   << "\n"
		   << "\t\t	if (fabs(vIn.x) <= c2)\n"
		   << "\t\t	{\n"
		   << "\t\t		x = fma(" << shift << ", " << weight << ", vIn.x);\n"
		   << "\n"
		   << "\t\t		if (fabs(x) >= c2)\n"
		   << "\t\t			vOut.x = -(" << weight << " * vIn.x);\n"
		   << "\t\t		else\n"
		   << "\t\t			vOut.x = " << weight << " * x;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t	}\n"
		   << "\n"
		   << "\t\t	vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t	vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Shift, prefix + "eclipse_shift", 0, eParamType::REAL_CYCLIC, -2, 2));
	}

private:
	T m_Shift;
};

/// <summary>
/// flipcircle.
/// </summary>
template <typename T>
class FlipCircleVariation : public ParametricVariation<T>
{
public:
	FlipCircleVariation(T weight = 1.0) : ParametricVariation<T>("flipcircle", eVariationId::VAR_FLIP_CIRCLE, weight, true)
	{
		Init();
	}

	PARVARCOPY(FlipCircleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * helper.In.x;

		if (helper.m_PrecalcSumSquares > m_WeightSquared)
			helper.Out.y = m_Weight * helper.In.y;
		else
			helper.Out.y = -(m_Weight * helper.In.y);

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string ww = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * vIn.x;\n"
		   << "\n"
		   << "\t\tif (precalcSumSquares > " << ww << ")\n"
		   << "\t\t	vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = -(" << weight << " * vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_WeightSquared = SQR(m_Weight);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_WeightSquared, prefix + "flipcircle_weight_squared"));
	}

private:
	T m_WeightSquared;
};

/// <summary>
/// flipx.
/// By tatasz.
/// </summary>
template <typename T>
class FlipXVariation : public Variation<T>
{
public:
	FlipXVariation(T weight = 1.0) : Variation<T>("flipx", eVariationId::VAR_FLIP_X, weight) { }

	VARCOPY(FlipXVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.y = m_Weight * helper.In.y;

		if (helper.In.y > 0)
			helper.Out.x = -(m_Weight * helper.In.x);
		else
			helper.Out.x = m_Weight * helper.In.x;

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.y = " << weight << " * vIn.y;\n"
		   << "\n"
		   << "\t\tif (vIn.y > 0)\n"
		   << "\t\t	vOut.x = -(" << weight << " * vIn.x);\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = " << weight << " * vIn.x;\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// flipy.
/// </summary>
template <typename T>
class FlipYVariation : public Variation<T>
{
public:
	FlipYVariation(T weight = 1.0) : Variation<T>("flipy", eVariationId::VAR_FLIP_Y, weight) { }

	VARCOPY(FlipYVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * helper.In.x;

		if (helper.In.x > 0)
			helper.Out.y = -(m_Weight * helper.In.y);
		else
			helper.Out.y = m_Weight * helper.In.y;

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * vIn.x;\n"
		   << "\n"
		   << "\t\tif (vIn.x > 0)\n"
		   << "\t\t	vOut.y = -(" << weight << " * vIn.y);\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = " << weight << " * vIn.y;\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// eCollide.
/// </summary>
template <typename T>
class ECollideVariation : public ParametricVariation<T>
{
public:
	ECollideVariation(T weight = 1.0) : ParametricVariation<T>("eCollide", eVariationId::VAR_ECOLLIDE, weight, true)
	{
		Init();
	}

	PARVARCOPY(ECollideVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tmp = helper.m_PrecalcSumSquares + 1;
		T tmp2 = 2 * helper.In.x;
		T xmax = (VarFuncs<T>::SafeSqrt(tmp + tmp2) + VarFuncs<T>::SafeSqrt(tmp - tmp2)) * T(0.5);
		int alt;

		if (xmax < 1)
			xmax = 1;

		T nu = std::acos(Clamp<T>(helper.In.x / xmax, -1, 1)); // -Pi < nu < Pi

		if (helper.In.y > 0)
		{
			alt = int(nu * m_CnPi);

			if ((alt & 1) == 0)
				nu = alt * m_PiCn + fmod(nu + m_CaCn, m_PiCn);
			else
				nu = alt * m_PiCn + fmod(nu - m_CaCn, m_PiCn);
		}
		else
		{
			alt = int(nu * m_CnPi);

			if ((alt & 1) == 0)
				nu = alt * m_PiCn + fmod(nu + m_CaCn, m_PiCn);
			else
				nu = alt * m_PiCn + fmod(nu - m_CaCn, m_PiCn);

			nu *= -1;
		}

		helper.Out.x = m_Weight * xmax * std::cos(nu);
		helper.Out.y = m_Weight * std::sqrt(xmax - 1) * std::sqrt(xmax + 1) * std::sin(nu);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string a = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string num = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ca = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cnPi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string caCn = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string piCn = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t tmp = precalcSumSquares + 1;\n"
		   << "\t\treal_t tmp2 = 2 * vIn.x;\n"
		   << "\t\treal_t xmax = (SafeSqrt(tmp + tmp2) + SafeSqrt(tmp - tmp2)) * (real_t)(0.5);\n"
		   << "\t\tint alt;\n"
		   << "\n"
		   << "\t\tif (xmax < 1)\n"
		   << "\t\t	xmax = 1;\n"
		   << "\n"
		   << "\t\treal_t nu = acos(clamp(vIn.x / xmax, -(real_t)(1.0), (real_t)(1.0)));\n"
		   << "\n"
		   << "\t\tif (vIn.y > 0)\n"
		   << "\t\t{\n"
		   << "\t\t	alt = (int)(nu * " << cnPi << ");\n"
		   << "\n"
		   << "\t\t	if ((alt & 1) == 0)\n"
		   << "\t\t		nu = alt * " << piCn << " + fmod(nu + " << caCn << ", " << piCn << ");\n"
		   << "\t\t	else\n"
		   << "\t\t		nu = alt * " << piCn << " + fmod(nu - " << caCn << ", " << piCn << ");\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	alt = (int)(nu * " << cnPi << ");\n"
		   << "\n"
		   << "\t\t	if ((alt & 1) == 0)\n"
		   << "\t\t		nu = alt * " << piCn << " + fmod(nu + " << caCn << ", " << piCn << ");\n"
		   << "\t\t	else\n"
		   << "\t\t		nu = alt * " << piCn << " + fmod(nu - " << caCn << ", " << piCn << ");\n"
		   << "\n"
		   << "\t\t	nu *= -1;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * xmax * cos(nu);\n"
		   << "\t\tvOut.y = " << weight << " * sqrt(xmax - 1) * sqrt(xmax + 1) * sin(nu);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "SafeSqrt" };
	}

	virtual void Precalc() override
	{
		m_CnPi = m_Num * T(M_1_PI);
		m_PiCn = T(M_PI) / m_Num;
		m_Ca = T(M_PI) * m_A;
		m_CaCn = m_Ca / m_Num;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "eCollide_a", 0, eParamType::REAL_CYCLIC, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Num, prefix + "eCollide_num", 1, eParamType::INTEGER, 1, T(INT_MAX)));
		m_Params.push_back(ParamWithName<T>(true, &m_Ca, prefix + "eCollide_ca"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_CnPi, prefix + "eCollide_cn_pi"));
		m_Params.push_back(ParamWithName<T>(true, &m_CaCn, prefix + "eCollide_ca_cn"));
		m_Params.push_back(ParamWithName<T>(true, &m_PiCn, prefix + "eCollide_pi_cn"));
	}

private:
	T m_A;
	T m_Num;
	T m_Ca;//Precalc.
	T m_CnPi;
	T m_CaCn;
	T m_PiCn;
};

/// <summary>
/// eJulia.
/// </summary>
template <typename T>
class EJuliaVariation : public ParametricVariation<T>
{
public:
	EJuliaVariation(T weight = 1.0) : ParametricVariation<T>("eJulia", eVariationId::VAR_EJULIA, weight, true)
	{
		Init();
	}

	PARVARCOPY(EJuliaVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x, r2 = helper.m_PrecalcSumSquares;

		if (m_Sign == 1)
		{
			x = helper.In.x;
		}
		else
		{
			r2 = 1 / Zeps(r2);
			x = helper.In.x * r2;
		}

		T tmp = r2 + 1;
		T tmp2 = 2 * x;
		T xmax = (VarFuncs<T>::SafeSqrt(tmp + tmp2) + VarFuncs<T>::SafeSqrt(tmp - tmp2)) * T(0.5);
		ClampGteRef<T>(xmax, 1);
		T mu = std::acosh(xmax);
		T nu = std::acos(Clamp<T>(x / xmax, -1, 1));//-Pi < nu < Pi.

		if (helper.In.y < 0)
			nu *= -1;

		nu = nu / m_Power + M_2PI / m_Power * Floor<T>(rand.Frand01<T>() * m_Power);
		mu /= m_Power;
		helper.Out.x = m_Weight * std::cosh(mu) * std::cos(nu);
		helper.Out.y = m_Weight * std::sinh(mu) * std::sin(nu);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sign = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x, r2 = precalcSumSquares;\n"
		   << "\n"
		   << "\t\tif (" << sign << " == 1)\n"
		   << "\t\t{\n"
		   << "\t\t	x = vIn.x;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	r2 = 1 / Zeps(r2);\n"
		   << "\t\t	x = vIn.x * r2;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t tmp = r2 + 1;\n"
		   << "\t\treal_t tmp2 = 2 * x;\n"
		   << "\t\treal_t xmax = (SafeSqrt(tmp + tmp2) + SafeSqrt(tmp - tmp2)) * (real_t)(0.5);\n"
		   << "\n"
		   << "\t\tif (xmax < 1)\n"
		   << "\t\t	xmax = 1;\n"
		   << "\n"
		   << "\t\treal_t mu = acosh(xmax);\n"
		   << "\t\treal_t nu = acos(clamp(x / xmax, -(real_t)(1.0), (real_t)(1.0)));\n"
		   << "\n"
		   << "\t\tif (vIn.y < 0)\n"
		   << "\t\t	nu *= -1;\n"
		   << "\n"
		   << "\t\tnu = nu / " << power << " + M_2PI / " << power << " * floor(MwcNext01(mwc) * " << power << ");\n"
		   << "\t\tmu /= " << power << ";\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * cosh(mu) * cos(nu);\n"
		   << "\t\tvOut.y = " << weight << " * sinh(mu) * sin(nu);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "SafeSqrt", "Zeps" };
	}

	virtual void Precalc() override
	{
		m_Sign = 1;

		if (m_Power < 0)
			m_Sign = -1;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "eJulia_power", 2, eParamType::INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_Sign, prefix + "eJulia_sign"));//Precalc.
	}

private:
	T m_Power;
	T m_Sign;//Precalc.
};

/// <summary>
/// eMod.
/// </summary>
template <typename T>
class EModVariation : public ParametricVariation<T>
{
public:
	EModVariation(T weight = 1.0) : ParametricVariation<T>("eMod", eVariationId::VAR_EMOD, weight, true)
	{
		Init();
	}

	PARVARCOPY(EModVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tmp = helper.m_PrecalcSumSquares + 1;
		T tmp2 = 2 * helper.In.x;
		T xmax = (VarFuncs<T>::SafeSqrt(tmp + tmp2) + VarFuncs<T>::SafeSqrt(tmp - tmp2)) * T(0.5);
		ClampGteRef<T>(xmax, 1);
		T mu = std::acosh(xmax);
		T nu = std::acos(Clamp<T>(helper.In.x / xmax, -1, 1));//-Pi < nu < Pi.

		if (helper.In.y < 0)
			nu *= -1;

		if (mu < m_Radius && -mu < m_Radius)
		{
			if (nu > 0)
				mu = fmod(mu + m_Radius + m_Distance * m_Radius, 2 * m_Radius) - m_Radius;
			else
				mu = fmod(mu - m_Radius - m_Distance * m_Radius, 2 * m_Radius) + m_Radius;
		}

		helper.Out.x = m_Weight * std::cosh(mu) * std::cos(nu);
		helper.Out.y = m_Weight * std::sinh(mu) * std::sin(nu);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string radius = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t tmp = precalcSumSquares + 1;\n"
		   << "\t\treal_t tmp2 = 2 * vIn.x;\n"
		   << "\t\treal_t xmax = (SafeSqrt(tmp + tmp2) + SafeSqrt(tmp - tmp2)) * (real_t)(0.5);\n"
		   << "\n"
		   << "\t\tif (xmax < 1)\n"
		   << "\t\t	xmax = 1;\n"
		   << "\n"
		   << "\t\treal_t mu = acosh(xmax);\n"
		   << "\t\treal_t nu = acos(clamp(vIn.x / xmax, -(real_t)(1.0), (real_t)(1.0)));\n"
		   << "\n"
		   << "\t\tif (vIn.y < 0)\n"
		   << "\t\t	nu *= -1;\n"
		   << "\n"
		   << "\t\tif (mu < " << radius << " && -mu < " << radius << ")\n"
		   << "\t\t{\n"
		   << "\t\t	if (nu > 0)\n"
		   << "\t\t		mu = fmod(mu + " << radius << " + " << dist << " * " << radius << ", 2 * " << radius << ") -  " << radius << ";\n"
		   << "\t\t	else\n"
		   << "\t\t		mu = fmod(mu - " << radius << " - " << dist << " * " << radius << ", 2 * " << radius << ") +  " << radius << ";\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * cosh(mu) * cos(nu);\n"
		   << "\t\tvOut.y = " << weight << " * sinh(mu) * sin(nu);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "SafeSqrt" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "eMod_radius", 1, eParamType::REAL, 0, TMAX));
		m_Params.push_back(ParamWithName<T>(&m_Distance, prefix + "eMod_distance", 0, eParamType::REAL_CYCLIC, 0, 2));
	}

private:
	T m_Radius;
	T m_Distance;
};

/// <summary>
/// eMotion.
/// </summary>
template <typename T>
class EMotionVariation : public ParametricVariation<T>
{
public:
	EMotionVariation(T weight = 1.0) : ParametricVariation<T>("eMotion", eVariationId::VAR_EMOTION, weight, true)
	{
		Init();
	}

	PARVARCOPY(EMotionVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tmp = helper.m_PrecalcSumSquares + 1;
		T tmp2 = 2 * helper.In.x;
		T xmax = (VarFuncs<T>::SafeSqrt(tmp + tmp2) + VarFuncs<T>::SafeSqrt(tmp - tmp2)) * T(0.5);
		ClampGteRef<T>(xmax, 1);
		T mu = std::acosh(xmax);
		T nu = std::acos(Clamp<T>(helper.In.x / xmax, -1, 1));//-Pi < nu < Pi.

		if (helper.In.y < 0)
			nu *= -1;

		if (nu < 0)
			mu += m_Move;
		else
			mu -= m_Move;

		if (mu <= 0)
		{
			mu *= -1;
			nu *= -1;
		}

		nu += m_Rotate;
		helper.Out.x = m_Weight * std::cosh(mu) * std::cos(nu);
		helper.Out.y = m_Weight * std::sinh(mu) * std::sin(nu);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string move = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotate = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t tmp = precalcSumSquares + 1;\n"
		   << "\t\treal_t tmp2 = 2 * vIn.x;\n"
		   << "\t\treal_t xmax = (SafeSqrt(tmp + tmp2) + SafeSqrt(tmp - tmp2)) * (real_t)(0.5);\n"
		   << "\n"
		   << "\t\tif (xmax < 1)\n"
		   << "\t\t	xmax = 1;\n"
		   << "\n"
		   << "\t\treal_t mu = acosh(xmax);\n"
		   << "\t\treal_t nu = acos(clamp(vIn.x / xmax, -(real_t)(1.0), (real_t)(1.0)));\n"
		   << "\n"
		   << "\t\tif (vIn.y < 0)\n"
		   << "\t\t	nu *= -1;\n"
		   << "\n"
		   << "\t\tif (nu < 0)\n"
		   << "\t\t	mu += " << move << ";\n"
		   << "\t\telse\n"
		   << "\t\t	mu -= " << move << ";\n"
		   << "\n"
		   << "\t\tif (mu <= 0)\n"
		   << "\t\t{\n"
		   << "\t\t	mu *= -1;\n"
		   << "\t\t	nu *= -1;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tnu += " << rotate << ";\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * cosh(mu) * cos(nu);\n"
		   << "\t\tvOut.y = " << weight << " * sinh(mu) * sin(nu);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "SafeSqrt" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Move, prefix + "eMotion_move"));
		m_Params.push_back(ParamWithName<T>(&m_Rotate, prefix + "eMotion_rotate", 0, eParamType::REAL_CYCLIC, 0, M_2PI));
	}

private:
	T m_Move;
	T m_Rotate;
};

/// <summary>
/// ePush.
/// </summary>
template <typename T>
class EPushVariation : public ParametricVariation<T>
{
public:
	EPushVariation(T weight = 1.0) : ParametricVariation<T>("ePush", eVariationId::VAR_EPUSH, weight, true)
	{
		Init();
	}

	PARVARCOPY(EPushVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tmp = helper.m_PrecalcSumSquares + 1;
		T tmp2 = 2 * helper.In.x;
		T xmax = (VarFuncs<T>::SafeSqrt(tmp + tmp2) + VarFuncs<T>::SafeSqrt(tmp - tmp2)) * T(0.5);
		ClampGteRef<T>(xmax, 1);
		T mu = std::acosh(xmax);
		T nu = std::acos(Clamp<T>(helper.In.x / xmax, -1, 1));//-Pi < nu < Pi.

		if (helper.In.y < 0)
			nu *= -1;

		nu += m_Rotate;
		mu *= m_Dist;
		mu += m_Push;
		helper.Out.x = m_Weight * std::cosh(mu) * std::cos(nu);
		helper.Out.y = m_Weight * std::sinh(mu) * std::sin(nu);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string push = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotate = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t tmp = precalcSumSquares + 1;\n"
		   << "\t\treal_t tmp2 = 2 * vIn.x;\n"
		   << "\t\treal_t xmax = (SafeSqrt(tmp + tmp2) + SafeSqrt(tmp - tmp2)) * (real_t)(0.5);\n"
		   << "\n"
		   << "\t\tif (xmax < 1)\n"
		   << "\t\t	xmax = 1;\n"
		   << "\n"
		   << "\t\treal_t mu = acosh(xmax);\n"
		   << "\t\treal_t nu = acos(clamp(vIn.x / xmax, -(real_t)(1.0), (real_t)(1.0)));\n"
		   << "\n"
		   << "\t\tif (vIn.y < 0)\n"
		   << "\t\t	nu *= -1;\n"
		   << "\n"
		   << "\t\tnu += " << rotate << ";\n"
		   << "\t\tmu *= " << dist << ";\n"
		   << "\t\tmu += " << push << ";\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * cosh(mu) * cos(nu);\n"
		   << "\t\tvOut.y = " << weight << " * sinh(mu) * sin(nu);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "SafeSqrt" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Push, prefix + "ePush_push"));
		m_Params.push_back(ParamWithName<T>(&m_Dist, prefix + "ePush_dist", 1));
		m_Params.push_back(ParamWithName<T>(&m_Rotate, prefix + "ePush_rotate", 0, eParamType::REAL_CYCLIC, T(-M_PI), T(M_PI)));
	}

private:
	T m_Push;
	T m_Dist;
	T m_Rotate;
};

/// <summary>
/// eRotate.
/// </summary>
template <typename T>
class ERotateVariation : public ParametricVariation<T>
{
public:
	ERotateVariation(T weight = 1.0) : ParametricVariation<T>("eRotate", eVariationId::VAR_EROTATE, weight, true)
	{
		Init();
	}

	PARVARCOPY(ERotateVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tmp = helper.m_PrecalcSumSquares + 1;
		T tmp2 = 2 * helper.In.x;
		T xmax = (VarFuncs<T>::SafeSqrt(tmp + tmp2) + VarFuncs<T>::SafeSqrt(tmp - tmp2)) * T(0.5);

		if (xmax < 1)
			xmax = 1;

		T nu = std::acos(Clamp<T>(helper.In.x / xmax, -1, 1));//-Pi < nu < Pi.

		if (helper.In.y < 0)
			nu *= -1;

		nu = fmod(nu + m_Rotate + T(M_PI), M_2PI) - T(M_PI);
		helper.Out.x = m_Weight * xmax * std::cos(nu);
		helper.Out.y = m_Weight * std::sqrt(xmax * xmax - 1) * std::sin(nu);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string rotate = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t weight = " << weight << ";\n"
		   << "\t\treal_t tmp = precalcSumSquares + 1;\n"
		   << "\t\treal_t tmp2 = 2 * vIn.x;\n"
		   << "\t\treal_t xmax = (SafeSqrt(tmp + tmp2) + SafeSqrt(tmp - tmp2)) * (real_t)(0.5);\n"
		   << "\n"
		   << "\t\tif (xmax < 1)\n"
		   << "\t\t	xmax = 1;\n"
		   << "\n"
		   << "\t\treal_t nu = acos(clamp(vIn.x / xmax, (real_t)(-1.0), (real_t)(1.0)));\n"
		   << "\n"
		   << "\t\tif (vIn.y < 0)\n"
		   << "\t\t	nu *= -1;\n"
		   << "\n"
		   << "\t\tnu = fmod(nu + " << rotate << " + MPI, M_2PI) - MPI;\n"
		   << "\n"
		   << "\t\tvOut.x = weight * xmax * cos(nu);\n"
		   << "\t\tvOut.y = weight * sqrt(xmax * xmax - 1) * sin(nu);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "SafeSqrt" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Rotate, prefix + "eRotate_rotate", 0, eParamType::REAL_CYCLIC, T(-M_PI), T(M_PI)));
	}

private:
	T m_Rotate;
};

/// <summary>
/// eScale.
/// </summary>
template <typename T>
class EScaleVariation : public ParametricVariation<T>
{
public:
	EScaleVariation(T weight = 1.0) : ParametricVariation<T>("eScale", eVariationId::VAR_ESCALE, weight, true)
	{
		Init();
	}

	PARVARCOPY(EScaleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tmp = helper.m_PrecalcSumSquares + 1;
		T tmp2 = 2 * helper.In.x;
		T xmax = (VarFuncs<T>::SafeSqrt(tmp + tmp2) + VarFuncs<T>::SafeSqrt(tmp - tmp2)) * T(0.5);
		ClampGteRef<T>(xmax, 1);
		T mu = std::acosh(xmax);
		T nu = std::acos(Clamp<T>(helper.In.x / xmax, -1, 1));//-Pi < nu < Pi.

		if (helper.In.y < 0)
			nu *= -1;

		mu *= m_Scale;
		nu = fmod(fmod(m_Scale * (nu + T(M_PI) + m_Angle), M_2PI * m_Scale) - m_Angle - m_Scale * T(M_PI), M_2PI);

		if (nu > T(M_PI))
			nu -= M_2PI;

		if (nu < -T(M_PI))
			nu += M_2PI;

		helper.Out.x = m_Weight * std::cosh(mu) * std::cos(nu);
		helper.Out.y = m_Weight * std::sinh(mu) * std::sin(nu);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string scale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t tmp = precalcSumSquares + 1;\n"
		   << "\t\treal_t tmp2 = 2 * vIn.x;\n"
		   << "\t\treal_t xmax = (SafeSqrt(tmp + tmp2) + SafeSqrt(tmp - tmp2)) * (real_t)(0.5);\n"
		   << "\n"
		   << "\t\tif (xmax < 1)\n"
		   << "\t\t	xmax = 1;\n"
		   << "\n"
		   << "\t\treal_t mu = acosh(xmax);\n"
		   << "\t\treal_t nu = acos(clamp(vIn.x / xmax, -(real_t)(1.0), (real_t)(1.0)));\n"
		   << "\n"
		   << "\t\tif (vIn.y < 0)\n"
		   << "\t\t	nu *= -1;\n"
		   << "\n"
		   << "\t\tmu *= " << scale << ";\n"
		   << "\t\tnu = fmod(fmod(" << scale << " * (nu + MPI + " << angle << "), M_2PI * " << scale << ") - " << angle << " - " << scale << " * MPI, M_2PI);\n"
		   << "\n"
		   << "\t\tif (nu > MPI)\n"
		   << "\t\t	nu -= M_2PI;\n"
		   << "\n"
		   << "\t\tif (nu < -MPI)\n"
		   << "\t\t	nu += M_2PI;\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * cosh(mu) * cos(nu);\n"
		   << "\t\tvOut.y = " << weight << " * sinh(mu) * sin(nu);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "SafeSqrt" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Scale, prefix + "eScale_scale", 1, eParamType::REAL_NONZERO, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "eScale_angle", 0, eParamType::REAL_CYCLIC, 0, M_2PI));
	}

private:
	T m_Scale;
	T m_Angle;
};

MAKEPREPOSTPARVAR(Funnel, funnel, FUNNEL)
MAKEPREPOSTVAR(Linear3D, linear3D, LINEAR3D)
MAKEPREPOSTPARVAR(PowBlock, pow_block, POW_BLOCK)
MAKEPREPOSTPARVAR(Squirrel, squirrel, SQUIRREL)
MAKEPREPOSTVAR(Ennepers, ennepers, ENNEPERS)
MAKEPREPOSTPARVAR(SphericalN, SphericalN, SPHERICALN)
MAKEPREPOSTPARVAR(Kaleidoscope, Kaleidoscope, KALEIDOSCOPE)
MAKEPREPOSTPARVAR(GlynnSim1, GlynnSim1, GLYNNSIM1)
MAKEPREPOSTPARVAR(GlynnSim2, GlynnSim2, GLYNNSIM2)
MAKEPREPOSTPARVAR(GlynnSim3, GlynnSim3, GLYNNSIM3)
MAKEPREPOSTPARVAR(GlynnSim4, GlynnSim4, GLYNNSIM4)
MAKEPREPOSTPARVAR(GlynnSim5, GlynnSim5, GLYNNSIM5)
MAKEPREPOSTPARVARASSIGN(Starblur, starblur, STARBLUR, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTPARVARASSIGN(Sineblur, sineblur, SINEBLUR, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTVARASSIGN(Circleblur, circleblur, CIRCLEBLUR, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(Depth, depth, DEPTH)
MAKEPREPOSTPARVAR(CropN, cropn, CROPN)
MAKEPREPOSTPARVAR(ShredRad, shredrad, SHRED_RAD)
MAKEPREPOSTPARVAR(Blob2, blob2, BLOB2)
MAKEPREPOSTPARVAR(Julia3D, julia3D, JULIA3D)
MAKEPREPOSTPARVAR(Julia3Dz, julia3Dz, JULIA3DZ)
MAKEPREPOSTPARVAR(LinearT, linearT, LINEAR_T)
MAKEPREPOSTPARVAR(LinearT3D, linearT3D, LINEAR_T3D)
MAKEPREPOSTPARVAR(Ovoid, ovoid, OVOID)
MAKEPREPOSTPARVAR(Ovoid3D, ovoid3d, OVOID3D)
MAKEPREPOSTPARVARASSIGN(Spirograph, Spirograph, SPIROGRAPH, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTVAR(Petal, petal, PETAL)
MAKEPREPOSTVAR(Spher, spher, SPHER)
MAKEPREPOSTVAR(RoundSpher, roundspher, ROUNDSPHER)
MAKEPREPOSTVAR(RoundSpher3D, roundspher3D, ROUNDSPHER3D)
MAKEPREPOSTVAR(SpiralWing, spiralwing, SPIRAL_WING)
MAKEPREPOSTVAR(Squarize, squarize, SQUARIZE)
MAKEPREPOSTPARVAR(Sschecks, sschecks, SSCHECKS)
MAKEPREPOSTPARVAR(PhoenixJulia, phoenix_julia, PHOENIX_JULIA)
MAKEPREPOSTPARVAR(Mobius, Mobius, MOBIUS)
MAKEPREPOSTPARVAR(MobiusN, MobiusN, MOBIUSN)
MAKEPREPOSTPARVAR(MobiusStrip, mobius_strip, MOBIUS_STRIP)
MAKEPREPOSTPARVARASSIGN(Lissajous, Lissajous, LISSAJOUS, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(Svf, svf, SVF)
MAKEPREPOSTPARVAR(Target, target, TARGET)
MAKEPREPOSTPARVAR(Target0, target0, TARGET0)
MAKEPREPOSTPARVAR(Target2, target2, TARGET2)
MAKEPREPOSTPARVAR(Taurus, taurus, TAURUS)
MAKEPREPOSTPARVAR(Collideoscope, collideoscope, COLLIDEOSCOPE)
MAKEPREPOSTPARVAR(BMod, bMod, BMOD)
MAKEPREPOSTPARVAR(BSwirl, bSwirl, BSWIRL)
MAKEPREPOSTPARVAR(BTransform, bTransform, BTRANSFORM)
MAKEPREPOSTPARVAR(BCollide, bCollide, BCOLLIDE)
MAKEPREPOSTPARVAR(Eclipse, eclipse, ECLIPSE)
MAKEPREPOSTPARVAR(FlipCircle, flipcircle, FLIP_CIRCLE)
MAKEPREPOSTVAR(FlipX, flipx, FLIP_X)
MAKEPREPOSTVAR(FlipY, flipy, FLIP_Y)
MAKEPREPOSTPARVAR(ECollide, eCollide, ECOLLIDE)
MAKEPREPOSTPARVAR(EJulia, eJulia, EJULIA)
MAKEPREPOSTPARVAR(EMod, eMod, EMOD)
MAKEPREPOSTPARVAR(EMotion, eMotion, EMOTION)
MAKEPREPOSTPARVAR(EPush, ePush, EPUSH)
MAKEPREPOSTPARVAR(ERotate, eRotate, EROTATE)
MAKEPREPOSTPARVAR(EScale, eScale, ESCALE)
}
