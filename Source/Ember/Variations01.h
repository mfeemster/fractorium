#pragma once

#include "Variation.h"
#include "Xform.h"

namespace EmberNs
{
//template <typename T> class Xform;

/// <summary>
/// Linear:
/// nx = tx;
/// ny = ty;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class LinearVariation : public Variation<T>
{
public:
	LinearVariation(T weight = 1.0) : Variation<T>("linear", eVariationId::VAR_LINEAR, weight) { }

	VARCOPY(LinearVariation)

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
/// Sinusoidal:
/// nx = sin(tx);
/// ny = sin(ty);
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class SinusoidalVariation : public Variation<T>
{
public:
	SinusoidalVariation(T weight = 1.0) : Variation<T>("sinusoidal", eVariationId::VAR_SINUSOIDAL, weight) { }

	VARCOPY(SinusoidalVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * std::sin(helper.In.x);
		helper.Out.y = m_Weight * std::sin(helper.In.y);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * sin(vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * sin(vIn.y);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Spherical:
/// T r2 = tx * tx + ty * ty + 1e-6;
/// nx = tx / r2;
/// ny = ty / r2;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <class T>
class SphericalVariation : public Variation<T>
{
public:
	SphericalVariation(T weight = 1.0) : Variation<T>("spherical", eVariationId::VAR_SPHERICAL, weight, true) { }

	VARCOPY(SphericalVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = m_Weight / Zeps(helper.m_PrecalcSumSquares);
		helper.Out.x = r2 * helper.In.x;
		helper.Out.y = r2 * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r2 = " << weight << " / Zeps(precalcSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = r2 * vIn.x;\n"
		   << "\t\tvOut.y = r2 * vIn.y;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Swirl:
/// double r2 = tx * tx + ty * ty;
/// double c1 = sin(r2);
/// double c2 = cos(r2);
/// nx = c1 * tx - c2 * ty;
/// ny = c2 * tx + c1 * ty;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class SwirlVariation : public Variation<T>
{
public:
	SwirlVariation(T weight = 1.0) : Variation<T>("swirl", eVariationId::VAR_SWIRL, weight, true) { }

	VARCOPY(SwirlVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T c1, c2;
		sincos(helper.m_PrecalcSumSquares, &c1, &c2);
		helper.Out.x = m_Weight * (c1 * helper.In.x - c2 * helper.In.y);
		helper.Out.y = m_Weight * (c2 * helper.In.x + c1 * helper.In.y);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t c1 = sin(precalcSumSquares);\n"
		   << "\t\treal_t c2 = cos(precalcSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(c1, vIn.x, -(c2 * vIn.y));\n"
		   << "\t\tvOut.y = " << weight << " * fma(c2, vIn.x, c1 * vIn.y);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// swirl3.
/// By Zy0rg.
/// </summary>
template <typename T>
class Swirl3Variation : public ParametricVariation<T>
{
public:
	Swirl3Variation(T weight = 1.0) : ParametricVariation<T>("swirl3", eVariationId::VAR_SWIRL3, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Swirl3Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T rad = helper.m_PrecalcSqrtSumSquares;
		T ang = helper.m_PrecalcAtanyx + std::log(rad) * m_Shift;
		helper.Out.x = m_Weight * rad * std::cos(ang);
		helper.Out.y = m_Weight * rad * std::sin(ang);
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
		   << "\t\treal_t rad = precalcSqrtSumSquares;\n"
		   << "\t\treal_t ang = fma(log(rad), " << shift << ", precalcAtanyx);\n"
		   << "\t\tvOut.x = " << weight << " * rad * cos(ang);\n"
		   << "\t\tvOut.y = " << weight << " * rad * sin(ang);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Shift, prefix + "swirl3_shift", T(0.5)));
	}

private:
	T m_Shift;
};

/// <summary>
/// swirl3r.
/// By Zy0rg.
/// </summary>
template <typename T>
class Swirl3rVariation : public ParametricVariation<T>
{
public:
	Swirl3rVariation(T weight = 1.0) : ParametricVariation<T>("swirl3r", eVariationId::VAR_SWIRL3R, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Swirl3rVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T rad = helper.m_PrecalcSqrtSumSquares;
		T ang = helper.m_PrecalcAtanyx;
		T ang2;

		if (rad < m_Minr)
			ang2 = ang + m_Mina;
		else if (rad > m_Maxr)
			ang2 = ang + m_Maxa;
		else
			ang2 = ang + std::log(rad) * m_Shift;

		helper.Out.x = m_Weight * rad * std::cos(ang2);
		helper.Out.y = m_Weight * rad * std::sin(ang2);
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
		string mmin  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mmax  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string minr  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string maxr  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mina  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string maxa  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t rad = precalcSqrtSumSquares;\n"
		   << "\t\treal_t ang2, ang = precalcAtanyx;\n"
		   << "\n"
		   << "\t\tif (rad < " << minr << ")\n"
		   << "\t\t	   ang2 = ang + " << mina << ";\n"
		   << "\t\telse if (rad > " << maxr << ")\n"
		   << "\t\t	ang2 = ang + " << maxa << ";\n"
		   << "\t\telse\n"
		   << "\t\t	ang2 = ang + log(rad) * " << shift << ";\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * rad * cos(ang);\n"
		   << "\t\tvOut.y = " << weight << " * rad * sin(ang);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Minr = std::min(m_Min, m_Max);
		m_Maxr = std::max(m_Min, m_Max);
		m_Mina = m_Minr > 0 ? std::log(m_Minr) * m_Shift : 0;
		m_Maxa = m_Maxr > 0 ? std::log(m_Maxr) * m_Shift : 0;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Shift, prefix + "swirl3r_shift", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Min, prefix + "swirl3r_min", T(0.5), eParamType::REAL, 0));
		m_Params.push_back(ParamWithName<T>(&m_Max, prefix + "swirl3r_max", 1, eParamType::REAL, 0));
		m_Params.push_back(ParamWithName<T>(true, &m_Minr, prefix + "swirl3r_minr"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Maxr, prefix + "swirl3r_maxr"));
		m_Params.push_back(ParamWithName<T>(true, &m_Mina, prefix + "swirl3r_mina"));
		m_Params.push_back(ParamWithName<T>(true, &m_Maxa, prefix + "swirl3r_maxa"));
	}

private:
	T m_Shift;
	T m_Min;
	T m_Max;
	T m_Minr;//Precalc.
	T m_Maxr;
	T m_Mina;
	T m_Maxa;
};

/// <summary>
/// Horseshoe:
/// a = atan2(tx, ty);
/// c1 = sin(a);
/// c2 = cos(a);
/// nx = c1 * tx - c2 * ty;
/// ny = c2 * tx + c1 * ty;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class HorseshoeVariation : public Variation<T>
{
public:
	HorseshoeVariation(T weight = 1.0) : Variation<T>("horseshoe", eVariationId::VAR_HORSESHOE, weight, true, true) { }

	VARCOPY(HorseshoeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight / Zeps(helper.m_PrecalcSqrtSumSquares);
		helper.Out.x = (helper.In.x - helper.In.y) * (helper.In.x + helper.In.y) * r;
		helper.Out.y = 2 * helper.In.x * helper.In.y * r;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r = " << weight << " / Zeps(precalcSqrtSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = (vIn.x - vIn.y) * (vIn.x + vIn.y) * r;\n"
		   << "\t\tvOut.y = (real_t)(2.0) * vIn.x * vIn.y * r;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Polar:
/// nx = atan2(tx, ty) / M_PI;
/// ny = std::sqrt(tx * tx + ty * ty) - 1.0;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class PolarVariation : public Variation<T>
{
public:
	PolarVariation(T weight = 1.0) : Variation<T>("polar", eVariationId::VAR_POLAR, weight, true, true, false, true, false) { }

	VARCOPY(PolarVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (helper.m_PrecalcAtanxy * T(M_1_PI));
		helper.Out.y = m_Weight * (helper.m_PrecalcSqrtSumSquares - 1);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * (precalcAtanxy * M1PI);\n"
		   << "\t\tvOut.y = " << weight << " * (precalcSqrtSumSquares - (real_t)(1.0));\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Handkerchief:
/// a = atan2(tx, ty);
/// r = std::sqrt(tx * tx + ty * ty);
/// p[0] += weight * sin(a + r) * r;
/// p[1] += weight * cos(a - r) * r;
/// </summary>
template <typename T>
class HandkerchiefVariation : public Variation<T>
{
public:
	HandkerchiefVariation(T weight = 1.0) : Variation<T>("handkerchief", eVariationId::VAR_HANDKERCHIEF, weight, true, true, false, true) { }

	VARCOPY(HandkerchiefVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * helper.m_PrecalcSqrtSumSquares * std::sin(helper.m_PrecalcAtanxy + helper.m_PrecalcSqrtSumSquares);
		helper.Out.y = m_Weight * helper.m_PrecalcSqrtSumSquares * std::cos(helper.m_PrecalcAtanxy - helper.m_PrecalcSqrtSumSquares);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * precalcSqrtSumSquares * sin(precalcAtanxy + precalcSqrtSumSquares);\n"
		   << "\t\tvOut.y = " << weight << " * precalcSqrtSumSquares * cos(precalcAtanxy - precalcSqrtSumSquares);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Heart:
/// a = atan2(tx, ty);
/// r = std::sqrt(tx * tx + ty * ty);
/// a *= r;
/// p[0] += weight * sin(a) * r;
/// p[1] += weight * cos(a) * -r;
/// </summary>
template <typename T>
class HeartVariation : public Variation<T>
{
public:
	HeartVariation(T weight = 1.0) : Variation<T>("heart", eVariationId::VAR_HEART, weight, true, true, false, true) { }

	VARCOPY(HeartVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcSqrtSumSquares * helper.m_PrecalcAtanxy;
		T r = m_Weight * helper.m_PrecalcSqrtSumSquares;
		helper.Out.x = r * std::sin(a);
		helper.Out.y = (-r) * std::cos(a);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t a = precalcSqrtSumSquares * precalcAtanxy;\n"
		   << "\t\treal_t r = " << weight << " * precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = r * sin(a);\n"
		   << "\t\tvOut.y = (-r) * cos(a);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Disc:
/// nx = tx * M_PI;
/// ny = ty * M_PI;
/// a = atan2(nx, ny);
/// r = std::sqrt(nx * nx + ny * ny);
/// p[0] += weight * sin(r) * a / M_PI;
/// p[1] += weight * cos(r) * a / M_PI;
/// </summary>
template <typename T>
class DiscVariation : public ParametricVariation<T>
{
public:
	DiscVariation(T weight = 1.0) : ParametricVariation<T>("disc", eVariationId::VAR_DISC, weight, true, true, false, true)
	{
		Init();
	}

	PARVARCOPY(DiscVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T val = T(M_PI) * helper.m_PrecalcSqrtSumSquares;
		T r = m_WeightByPI * helper.m_PrecalcAtanxy;
		helper.Out.x = std::sin(val) * r;
		helper.Out.y = std::cos(val) * r;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string weightByPI = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.
		ss << "\t{\n"
		   << "\t\treal_t val = MPI * precalcSqrtSumSquares;\n"
		   << "\t\treal_t r = " << weightByPI << " * precalcAtanxy;\n"
		   << "\n"
		   << "\t\tvOut.x = sin(val) * r;\n"
		   << "\t\tvOut.y = cos(val) * r;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_WeightByPI = m_Weight * T(M_1_PI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_WeightByPI, prefix + "disc_weight_by_pi"));//Precalcs only, no params.
	}

private:
	T m_WeightByPI;//Precalcs only, no params.
};

/// <summary>
/// Spiral:
/// a = atan2(tx, ty);
/// r = std::sqrt(tx * tx + ty * ty) + 1e-6;
/// p[0] += weight * (cos(a) + sin(r)) / r;
/// p[1] += weight * (sin(a) - cos(r)) / r;
/// </summary>
template <typename T>
class SpiralVariation : public Variation<T>
{
public:
	SpiralVariation(T weight = 1.0) : Variation<T>("spiral", eVariationId::VAR_SPIRAL, weight, true, true, true) { }

	VARCOPY(SpiralVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = Zeps(helper.m_PrecalcSqrtSumSquares);
		T r1 = m_Weight / r;
		helper.Out.x = r1 * (helper.m_PrecalcSina + std::sin(r));//Intentionally flipped.
		helper.Out.y = r1 * (helper.m_PrecalcCosa - std::cos(r));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r = Zeps(precalcSqrtSumSquares);\n"
		   << "\t\treal_t r1 = " << weight << " / r;\n"
		   << "\n"
		   << "\t\tvOut.x = r1 * (precalcSina + sin(r));\n"
		   << "\t\tvOut.y = r1 * (precalcCosa - cos(r));\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Hyperbolic:
/// a = atan2(tx, ty);
/// r = std::sqrt(tx * tx + ty * ty) + 1e-6;
/// p[0] += weight * sin(a) / r;
/// p[1] += weight * cos(a) * r;
/// </summary>
template <typename T>
class HyperbolicVariation : public Variation<T>
{
public:
	HyperbolicVariation(T weight = 1.0) : Variation<T>("hyperbolic", eVariationId::VAR_HYPERBOLIC, weight, true, true, true) { }

	VARCOPY(HyperbolicVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = Zeps(helper.m_PrecalcSqrtSumSquares);
		helper.Out.x = m_Weight * helper.m_PrecalcCosa / r;//Flipped from flam3 because flam3 had them erroneously flipped.
		helper.Out.y = m_Weight * helper.m_PrecalcSina * r;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r = Zeps(precalcSqrtSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * precalcCosa / r;\n"
		   << "\t\tvOut.y = " << weight << " * precalcSina * r;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Diamond:
/// a = atan2(tx, ty);
/// r = std::sqrt(tx * tx + ty * ty);
/// p[0] += weight * sin(a) * cos(r);
/// p[1] += weight * cos(a) * sin(r);
/// </summary>
template <typename T>
class DiamondVariation : public Variation<T>
{
public:
	DiamondVariation(T weight = 1.0) : Variation<T>("diamond", eVariationId::VAR_DIAMOND, weight, true, true, true) { }

	VARCOPY(DiamondVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * helper.m_PrecalcCosa * std::cos(helper.m_PrecalcSqrtSumSquares);//Flipped from flam3 because flam3 had them erroneously flipped.
		helper.Out.y = m_Weight * helper.m_PrecalcSina * std::sin(helper.m_PrecalcSqrtSumSquares);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * precalcCosa * cos(precalcSqrtSumSquares);\n"
		   << "\t\tvOut.y = " << weight << " * precalcSina * sin(precalcSqrtSumSquares);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Ex:
/// a = atan2(tx, ty);
/// r = std::sqrt(tx * tx + ty * ty);
/// n0 = sin(a + r);
/// n1 = cos(a - r);
/// m0 = n0 * n0 * n0 * r;
/// m1 = n1 * n1 * n1 * r;
/// p[0] += weight * (m0 + m1);
/// p[1] += weight * (m0 - m1);
/// </summary>
template <typename T>
class ExVariation : public Variation<T>
{
public:
	ExVariation(T weight = 1.0) : Variation<T>("ex", eVariationId::VAR_EX, weight, true, true, false, true) { }

	VARCOPY(ExVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanxy;
		T r = helper.m_PrecalcSqrtSumSquares;
		T n0 = std::sin(a + r);
		T n1 = std::cos(a - r);
		T m0 = n0 * n0 * n0 * r;
		T m1 = n1 * n1 * n1 * r;
		helper.Out.x = m_Weight * (m0 + m1);
		helper.Out.y = m_Weight * (m0 - m1);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanxy;\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\t\treal_t n0 = sin(a + r);\n"
		   << "\t\treal_t n1 = cos(a - r);\n"
		   << "\t\treal_t m0 = n0 * n0 * n0 * r;\n"
		   << "\t\treal_t m1 = n1 * n1 * n1 * r;\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (m0 + m1);\n"
		   << "\t\tvOut.y = " << weight << " * (m0 - m1);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Julia:
/// a = atan2(tx, ty)/2.0;
/// if (random bit()) a += M_PI;
/// r = pow(tx*tx + ty*ty, 0.25);
/// nx = r * cos(a);
/// ny = r * sin(a);
/// p[0] += v * nx;
/// p[1] += v * ny;
/// </summary>
template <typename T>
class JuliaVariation : public Variation<T>
{
public:
	using Variation<T>::m_NeedPrecalcSqrtSumSquares;
	JuliaVariation(T weight = 1.0) : Variation<T>("julia", eVariationId::VAR_JULIA, weight, true, false, false, true)
	{
		m_NeedPrecalcSqrtSumSquares = Compat::m_Compat;
	}

	VARCOPY(JuliaVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r;

		if (Compat::m_Compat)
			r = m_Weight * std::sqrt(helper.m_PrecalcSqrtSumSquares);
		else
			r = m_Weight * std::exp(T(0.25) * std::log(helper.m_PrecalcSumSquares));

		T a = T(0.5) * helper.m_PrecalcAtanxy;

		if (rand.RandBit())
			a += T(M_PI);

		helper.Out.x = r * std::cos(a);
		helper.Out.y = r * std::sin(a);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n";

		if (Compat::m_Compat)
			ss << "\t\treal_t r = " << weight << " * sqrt(precalcSqrtSumSquares);\n";
		else
			ss << "\t\treal_t r = " << weight << " * exp((real_t)(0.25) * log(precalcSumSquares));\n";

		ss << "\t\treal_t a = (real_t)(0.5) * precalcAtanxy;\n"
		   << "\n"
		   << "\t\tif (MwcNext(mwc) & 1)\n"
		   << "\t\t	a += MPI;\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_NeedPrecalcSqrtSumSquares = Compat::m_Compat;
	}
};

/// <summary>
/// Bent:
/// nx = tx;
/// ny = ty;
/// if (nx < 0.0) nx = nx * 2.0;
/// if (ny < 0.0) ny = ny / 2.0;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class BentVariation : public Variation<T>
{
public:
	BentVariation(T weight = 1.0) : Variation<T>("bent", eVariationId::VAR_BENT, weight) { }

	VARCOPY(BentVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T nx = helper.In.x < T(0.0) ? helper.In.x * 2 : helper.In.x;
		T ny = helper.In.y < T(0.0) ? helper.In.y / 2 : helper.In.y;
		helper.Out.x = m_Weight * nx;
		helper.Out.y = m_Weight * ny;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t nx = vIn.x < (real_t)(0.0) ? (vIn.x * (real_t)(2.0)) : vIn.x;\n"
		   << "\t\treal_t ny = vIn.y < (real_t)(0.0) ? (vIn.y / (real_t)(2.0)) : vIn.y;\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * nx;\n"
		   << "\t\tvOut.y = " << weight << " * ny;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Waves:
/// dx = coef[2][0];
/// dy = coef[2][1];
/// nx = tx + coef[1][0] * sin(ty / ((dx * dx) + EPS));
/// ny = ty + coef[1][1] * sin(tx / ((dy * dy) + EPS));
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// Special case here, use parametric for precalcs, but no regular params.
/// </summary>
template <typename T>
class WavesVariation : public ParametricVariation<T>
{
public:
	WavesVariation(T weight = 1.0) : ParametricVariation<T>("waves", eVariationId::VAR_WAVES, weight)
	{
		Init();
	}

	PARVARCOPY(WavesVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T c10 = m_Xform->m_Affine.B();
		T c11 = m_Xform->m_Affine.E();
		T nx = helper.In.x + c10 * std::sin(helper.In.y * m_Dx2);
		T ny = helper.In.y + c11 * std::sin(helper.In.x * m_Dy2);
		helper.Out.x = m_Weight * nx;
		helper.Out.y = m_Weight * ny;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string dx2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.
		string dy2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t c10 = xform->m_B;\n"
		   << "\t\treal_t c11 = xform->m_E;\n"
		   << "\t\treal_t nx = fma(c10, sin(vIn.y * " << dx2 << "), vIn.x);\n"
		   << "\t\treal_t ny = fma(c11, sin(vIn.x * " << dy2 << "), vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.x = (" << weight << " * nx);\n"
		   << "\t\tvOut.y = (" << weight << " * ny);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		if (m_Xform)//If this variation exists by itself and hasn't been added to an xform yet, m_Xform will be nullptr.
		{
			T dx = m_Xform->m_Affine.C();
			T dy = m_Xform->m_Affine.F();
			m_Dx2 = 1 / Zeps(dx * dx);
			m_Dy2 = 1 / Zeps(dy * dy);
		}
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_Dx2, prefix + "waves_dx2"));//Precalcs only, no params.
		m_Params.push_back(ParamWithName<T>(true, &m_Dy2, prefix + "waves_dy2"));
	}

private:
	T m_Dx2;//Precalcs only, no params.
	T m_Dy2;
};

/// <summary>
/// Fisheye:
/// a = atan2(tx, ty);
/// r = std::sqrt(tx * tx + ty * ty);
/// r = 2 * r / (r + 1);
/// nx = r * cos(a);
/// ny = r * sin(a);
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class FisheyeVariation : public Variation<T>
{
public:
	FisheyeVariation(T weight = 1.0) : Variation<T>("fisheye", eVariationId::VAR_FISHEYE, weight, true, true) { }

	VARCOPY(FisheyeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = 2 * m_Weight / (helper.m_PrecalcSqrtSumSquares + 1);
		helper.Out.x = r * helper.In.y;
		helper.Out.y = r * helper.In.x;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r = 2 * " << weight << " / (precalcSqrtSumSquares + 1);\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.y;\n"
		   << "\t\tvOut.y = r * vIn.x;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Popcorn:
/// dx = tan(3 * ty);
/// dy = tan(3 * tx);
/// nx = tx + coef[2][0] * sin(dx);
/// ny = ty + coef[2][1] * sin(dy);
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class PopcornVariation : public Variation<T>
{
public:
	PopcornVariation(T weight = 1.0) : Variation<T>("popcorn", eVariationId::VAR_POPCORN, weight) { }

	VARCOPY(PopcornVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T dx = SafeTan<T>(3 * helper.In.y);
		T dy = SafeTan<T>(3 * helper.In.x);
		T nx = helper.In.x + m_Xform->m_Affine.C() * std::sin(dx);
		T ny = helper.In.y + m_Xform->m_Affine.F() * std::sin(dy);
		helper.Out.x = m_Weight * nx;
		helper.Out.y = m_Weight * ny;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t dx = tan(3 * vIn.y);\n"
		   << "\t\treal_t dy = tan(3 * vIn.x);\n"
		   << "\t\treal_t nx = fma(xform->m_C, sin(dx), vIn.x);\n"
		   << "\t\treal_t ny = fma(xform->m_F, sin(dy), vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * nx;\n"
		   << "\t\tvOut.y = " << weight << " * ny;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Exponential:
/// dx = exp(tx - 1.0);
/// dy = M_PI * ty;
/// nx = cos(dy) * dx;
/// ny = sin(dy) * dx;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class ExponentialVariation : public Variation<T>
{
public:
	ExponentialVariation(T weight = 1.0) : Variation<T>("exponential", eVariationId::VAR_EXPONENTIAL, weight) { }

	VARCOPY(ExponentialVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T dx = m_Weight * std::exp(helper.In.x - 1);
		T dy = T(M_PI) * helper.In.y;
		helper.Out.x = dx * std::cos(dy);
		helper.Out.y = dx * std::sin(dy);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t dx = " << weight << " * exp(vIn.x - (real_t)(1.0));\n"
		   << "\t\treal_t dy = MPI * vIn.y;\n"
		   << "\n"
		   << "\t\tvOut.x = dx * cos(dy);\n"
		   << "\t\tvOut.y = dx * sin(dy);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Power:
/// a = atan2(tx, ty);
/// sa = sin(a);
/// r = std::sqrt(tx * tx + ty * ty);
/// r = pow(r, sa);
/// nx = r * precalc_cosa;
/// ny = r * sa;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class PowerVariation : public Variation<T>
{
public:
	PowerVariation(T weight = 1.0) : Variation<T>("power", eVariationId::VAR_POWER, weight, true, true, true) { }

	VARCOPY(PowerVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight * std::pow(helper.m_PrecalcSqrtSumSquares, helper.m_PrecalcCosa);//Flipped from flam3.
		helper.Out.x = r * helper.m_PrecalcSina;//Intentionally flipped.
		helper.Out.y = r * helper.m_PrecalcCosa;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r = " << weight << " * pow(precalcSqrtSumSquares, precalcCosa);\n"
		   << "\n"
		   << "\t\tvOut.x = r * precalcSina;\n"
		   << "\t\tvOut.y = r * precalcCosa;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Cosine:
/// nx = cos(tx * M_PI) * cosh(ty);
/// ny = -sin(tx * M_PI) * sinh(ty);
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class CosineVariation : public Variation<T>
{
public:
	CosineVariation(T weight = 1.0) : Variation<T>("cosine", eVariationId::VAR_COSINE, weight) { }

	VARCOPY(CosineVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.In.x * T(M_PI);
		T nx = std::cos(a) * std::cosh(helper.In.y);
		T ny = -std::sin(a) * std::sinh(helper.In.y);
		helper.Out.x = m_Weight * nx;
		helper.Out.y = m_Weight * ny;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t a = vIn.x * MPI;\n"
		   << "\t\treal_t nx = cos(a) * cosh(vIn.y);\n"
		   << "\t\treal_t ny = -sin(a) * sinh(vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * nx;\n"
		   << "\t\tvOut.y = " << weight << " * ny;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Rings:
/// dx = coef[2][0];
/// dx = dx * dx + EPS;
/// r = std::sqrt(tx * tx + ty * ty);
/// r = fmod(r + dx, 2 * dx) - dx + r * (1 - dx);
/// a = atan2(tx, ty);
/// nx = cos(a) * r;
/// ny = sin(a) * r;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class RingsVariation : public Variation<T>
{
public:
	RingsVariation(T weight = 1.0) : Variation<T>("rings", eVariationId::VAR_RINGS, weight, true, true, true) { }

	VARCOPY(RingsVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T dx = Zeps(m_Xform->m_Affine.C() * m_Xform->m_Affine.C());
		T r = helper.m_PrecalcSqrtSumSquares;
		r = m_Weight * (fmod(r + dx, 2 * dx) - dx + r * (1 - dx));
		helper.Out.x = r * helper.m_PrecalcSina;//Intentionally flipped, also flipped from flam3 because flam3 had them erroneously flipped.
		helper.Out.y = r * helper.m_PrecalcCosa;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t dx = Zeps(xform->m_C * xform->m_C);\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tr = " << weight << " * (fmod(r + dx, 2 * dx) + fma(r, ((real_t)(1.0) - dx), -dx));\n"
		   << "\t\tvOut.x = r * precalcSina;\n"
		   << "\t\tvOut.y = r * precalcCosa;\n"
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
/// Fan:
/// dx = coef[2][0];
/// dy = coef[2][1];
/// dx = M_PI * (dx * dx + EPS);
/// dx2 = dx / 2;
/// a = atan(tx, ty);
/// r = std::sqrt(tx * tx + ty * ty);
/// a += (fmod(a + dy, dx) > dx2) ? -dx2 : dx2;
/// nx = cos(a) * r;
/// ny = sin(a) * r;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class FanVariation : public Variation<T>
{
public:
	FanVariation(T weight = 1.0) : Variation<T>("fan", eVariationId::VAR_FAN, weight, true, true, false, true) { }

	VARCOPY(FanVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T dx = T(M_PI) * Zeps(m_Xform->m_Affine.C() * m_Xform->m_Affine.C());
		T dy = m_Xform->m_Affine.F();
		T dx2 = T(0.5) * dx;
		T a = helper.m_PrecalcAtanxy;
		T r = m_Weight * helper.m_PrecalcSqrtSumSquares;
		a += (fmod(a + dy, dx) > dx2) ? -dx2 : dx2;
		helper.Out.x = r * std::cos(a);
		helper.Out.y = r * std::sin(a);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t dx = MPI * Zeps(xform->m_C * xform->m_C);\n"
		   << "\t\treal_t dy = xform->m_F;\n"
		   << "\t\treal_t dx2 = (real_t)(0.5) * dx;\n"
		   << "\t\treal_t a = precalcAtanxy + ((fmod(precalcAtanxy + dy, dx) > dx2) ? -dx2 : dx2);\n"
		   << "\t\treal_t r = " << weight << " * precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
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
/// Blob:
/// a = atan2(tx, ty);
/// r = std::sqrt(tx * tx + ty * ty);
/// r = r * (bloblow + (blobhigh - bloblow) * (0.5 + 0.5 * sin(blobwaves * a)));
/// nx = sin(a) * r;
/// ny = cos(a) * r;
///
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class BlobVariation : public ParametricVariation<T>
{
public:
	BlobVariation(T weight = 1.0) : ParametricVariation<T>("blob", eVariationId::VAR_BLOB, weight, true, true, true, true)
	{
		Init();
	}

	PARVARCOPY(BlobVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = helper.m_PrecalcSqrtSumSquares * (m_BlobLow + m_BlobDiff * (T(0.5) + T(0.5) * std::sin(m_BlobWaves * helper.m_PrecalcAtanxy)));
		helper.Out.x = m_Weight * helper.m_PrecalcCosa * r;//Flipped from flam3 because flam3 had them erroneously flipped.
		helper.Out.y = m_Weight * helper.m_PrecalcSina * r;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index     = ss2.str();
		string weight    = WeightDefineString();
		string blobLow   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blobHigh  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blobWaves = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blobDiff  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = precalcSqrtSumSquares * fma(" << blobDiff << ", fma((real_t)(0.5), sin(" << blobWaves << " * precalcAtanxy), (real_t)(0.5)), " << blobLow << ");\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * precalcCosa * r;\n"
		   << "\t\tvOut.y = " << weight << " * precalcSina * r;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_BlobDiff = m_BlobHigh - m_BlobLow;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_BlobLow = T(0.2) + T(0.5) * rand.Frand01<T>();
		m_BlobHigh = T(0.8) + T(0.4) * rand.Frand01<T>();
		m_BlobWaves = T(int(2 + 5 * rand.Frand01<T>()));
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_BlobLow, prefix + "blob_low"));
		m_Params.push_back(ParamWithName<T>(&m_BlobHigh, prefix + "blob_high", 1));
		m_Params.push_back(ParamWithName<T>(&m_BlobWaves, prefix + "blob_waves", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_BlobDiff, prefix + "blob_diff"));//Precalc.
	}

private:
	T m_BlobLow;
	T m_BlobHigh;
	T m_BlobWaves;
	T m_BlobDiff;//Precalc.
};

/// <summary>
/// Pdj:
/// nx1 = cos(pdjb * tx);
/// nx2 = sin(pdjc * tx);
/// ny1 = sin(pdja * ty);
/// ny2 = cos(pdjd * ty);
///
/// p[0] += weight * (ny1 - nx1);
/// p[1] += weight * (nx2 - ny2);
/// </summary>
template <typename T>
class PdjVariation : public ParametricVariation<T>
{
public:
	PdjVariation(T weight = 1.0) : ParametricVariation<T>("pdj", eVariationId::VAR_PDJ, weight)
	{
		Init();
	}

	PARVARCOPY(PdjVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T nx1 = std::cos(m_PdjB * helper.In.x);
		T nx2 = std::sin(m_PdjC * helper.In.x);
		T ny1 = std::sin(m_PdjA * helper.In.y);
		T ny2 = std::cos(m_PdjD * helper.In.y);
		helper.Out.x = m_Weight * (ny1 - nx1);
		helper.Out.y = m_Weight * (nx2 - ny2);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string pdjA = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pdjB = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pdjC = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pdjD = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t nx1 = cos(" << pdjB << " * vIn.x)" << ";\n"
		   << "\t\treal_t nx2 = sin(" << pdjC << " * vIn.x)" << ";\n"
		   << "\t\treal_t ny1 = sin(" << pdjA << " * vIn.y)" << ";\n"
		   << "\t\treal_t ny2 = cos(" << pdjD << " * vIn.y)" << ";\n"
		   << "\n"
		   << "\t\tvOut.x = (" << weight << " * (ny1 - nx1));\n"
		   << "\t\tvOut.y = (" << weight << " * (nx2 - ny2));\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_PdjA = 3 * rand.Frand11<T>();
		m_PdjB = 3 * rand.Frand11<T>();
		m_PdjC = 3 * rand.Frand11<T>();
		m_PdjD = 3 * rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_PdjA, prefix + "pdj_a"));
		m_Params.push_back(ParamWithName<T>(&m_PdjB, prefix + "pdj_b"));
		m_Params.push_back(ParamWithName<T>(&m_PdjC, prefix + "pdj_c"));
		m_Params.push_back(ParamWithName<T>(&m_PdjD, prefix + "pdj_d"));
	}

private:
	T m_PdjA;
	T m_PdjB;
	T m_PdjC;
	T m_PdjD;
};

/// <summary>
/// Fan2:
/// a = precalc_atan;
/// r = precalc_sqrt;
///
/// dy = fan2y;
/// dx = M_PI * (fan2x * fan2x + EPS);
/// dx2 = dx / 2.0;
///
/// t = a + dy - dx * (int)((a + dy) / dx);
///
/// if (t > dx2)
///     a = a - dx2;
/// else
///     a = a + dx2;
///
/// nx = sin(a) * r;
/// ny = cos(a) * r;
///
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class Fan2Variation : public ParametricVariation<T>
{
public:
	Fan2Variation(T weight = 1.0) : ParametricVariation<T>("fan2", eVariationId::VAR_FAN2, weight, true, true, false, true)
	{
		Init();
	}

	PARVARCOPY(Fan2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanxy;
		T r = m_Weight * helper.m_PrecalcSqrtSumSquares;

		if (std::fmod(a + m_Fan2Y, m_Fan2Dx) > m_Fan2Dx2)
			a = a - m_Fan2Dx2;
		else
			a = a + m_Fan2Dx2;

		if (Compat::m_Compat)
		{
			helper.Out.x = r * std::sin(a);
			helper.Out.y = r * std::cos(a);
		}
		else
		{
			helper.Out.x = r * std::cos(a);
			helper.Out.y = r * std::sin(a);
		}

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string fan2X = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fan2Y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dx2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanxy;\n"
		   << "\t\treal_t weight = " << weight << ";\n"
		   << "\t\treal_t r = weight * precalcSqrtSumSquares;\n"
		   << "\t\treal_t dx2 = " << dx2 << ";\n"
		   << "\n"
		   << "\t\tif (fmod(a + " << fan2Y << ", " << dx << ") > dx2)\n"
		   << "\t\t	a = a - dx2;\n"
		   << "\t\telse\n"
		   << "\t\t	a = a + dx2;\n"
		   << "\n";

		if (Compat::m_Compat)
			ss << "\t\tvOut.x = r * sin(a);\n"
			   << "\t\tvOut.y = r * cos(a);\n"
			   << "\t\tvOut.z = weight * vIn.z;\n"
			   << "\t}\n";
		else
			ss << "\t\tvOut.x = r * cos(a);\n"
			   << "\t\tvOut.y = r * sin(a);\n"
			   << "\t\tvOut.z = weight * vIn.z;\n"
			   << "\t}\n";

		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Fan2Dx = T(M_PI) * Zeps(SQR(m_Fan2X));
		m_Fan2Dx2 = T(0.5) * m_Fan2Dx;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Fan2X = rand.Frand11<T>();
		m_Fan2Y = rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Fan2X, prefix + "fan2_x", T(0.70710678118654752440084436210485)));
		m_Params.push_back(ParamWithName<T>(&m_Fan2Y, prefix + "fan2_y", T(-0.70710678118654752440084436210485)));
		m_Params.push_back(ParamWithName<T>(true, &m_Fan2Dx, prefix + "fan2_dx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Fan2Dx2, prefix + "fan2_dx2"));
	}

private:
	T m_Fan2X;
	T m_Fan2Y;
	T m_Fan2Dx;//Precalc.
	T m_Fan2Dx2;
};

/// <summary>
/// Rings2:
/// r = precalc_sqrt;
/// dx = rings2val * rings2val + EPS;
/// r += dx - 2.0 * dx * (int)((r + dx)/(2.0 * dx)) - dx + r * (1.0 - dx);
/// nx = precalc_sina * r;
/// ny = precalc_cosa * r;
/// p[0] += weight * nx;
/// p[1] += weight * ny;
/// </summary>
template <typename T>
class Rings2Variation : public ParametricVariation<T>
{
public:
	Rings2Variation(T weight = 1.0) : ParametricVariation<T>("rings2", eVariationId::VAR_RINGS2, weight, true, true, true)
	{
		Init();
	}

	PARVARCOPY(Rings2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = helper.m_PrecalcSqrtSumSquares;
		r += -2 * m_Rings2Val2 * int((r + m_Rings2Val2) / (2 * m_Rings2Val2)) + r * (1 - m_Rings2Val2);
		helper.Out.x = m_Weight * helper.m_PrecalcCosa * r;//Flipped from flam3 because flam3 had them erroneously flipped.
		helper.Out.y = m_Weight * helper.m_PrecalcSina * r;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string rings2Val = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rings2Val2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tr += fma((real_t)(-2.0) * " << rings2Val2 << ", (real_t)(int)((r + " << rings2Val2 << ") / ((real_t)(2.0) * " << rings2Val2 << ")), r * ((real_t)(1.0) - " << rings2Val2 << "));\n"
		   //<< "\t\tr += -(real_t)(2.0) * " << rings2Val2 << " * (int)((r + " << rings2Val2 << ") / ((real_t)(2.0) * " << rings2Val2 << ")) + r * ((real_t)(1.0) - " << rings2Val2 << ");\n"
		   << "\t\tvOut.x = (" << weight << " * precalcCosa * r);\n"
		   << "\t\tvOut.y = (" << weight << " * precalcSina * r);\n"
		   << "\t\tvOut.z =  " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Rings2Val2 = Zeps(SQR(m_Rings2Val));
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Rings2Val = 2 * rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Rings2Val, prefix + "rings2_val", 1));//This differs from the original which used zero. Use 1 instead to avoid getting too close to dividing by zero.
		m_Params.push_back(ParamWithName<T>(true, &m_Rings2Val2, prefix + "rings2_val2"));//Precalc.
	}

private:
	T m_Rings2Val;
	T m_Rings2Val2;//Precalc.
};

/// <summary>
/// Eyefish:
/// r = 2.0 * weight / (precalc_sqrt + 1.0);
/// p[0] += r * tx;
/// p[1] += r * ty;
/// </summary>
template <typename T>
class EyefishVariation : public Variation<T>
{
public:
	EyefishVariation(T weight = 1.0) : Variation<T>("eyefish", eVariationId::VAR_EYEFISH, weight, true, true) { }

	VARCOPY(EyefishVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = 2 * m_Weight / (helper.m_PrecalcSqrtSumSquares + 1);
		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r = (" << weight << " * (real_t)(2.0)) / (precalcSqrtSumSquares + (real_t)(1.0));\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * vIn.y;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Bubble.
/// </summary>
template <typename T>
class BubbleVariation : public Variation<T>
{
public:
	BubbleVariation(T weight = 1.0) : Variation<T>("bubble", eVariationId::VAR_BUBBLE, weight, true) { }

	VARCOPY(BubbleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T denom = T(0.25) * helper.m_PrecalcSumSquares + 1;
		T r = m_Weight / denom;
		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * helper.In.y;
		helper.Out.z = m_Weight * (2 / Zeps(denom - 1));
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t denom = fma((real_t)(0.25), precalcSumSquares, (real_t)(1.0));\n"
		   << "\t\treal_t r = " << weight << " / denom;\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * vIn.y;\n"
		   << "\t\tvOut.z = " << weight << " * (2 / Zeps(denom - 1));\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Cylinder.
/// </summary>
template <typename T>
class CylinderVariation : public Variation<T>
{
public:
	CylinderVariation(T weight = 1.0) : Variation<T>("cylinder", eVariationId::VAR_CYLINDER, weight) { }

	VARCOPY(CylinderVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * std::sin(helper.In.x);
		helper.Out.y = m_Weight * helper.In.y;
		helper.Out.z = m_Weight * std::cos(helper.In.x);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * sin(vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * vIn.y;\n"
		   << "\t\tvOut.z = " << weight << " * cos(vIn.x);\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Perspective.
/// </summary>
template <typename T>
class PerspectiveVariation : public ParametricVariation<T>
{
public:
	PerspectiveVariation(T weight = 1.0) : ParametricVariation<T>("perspective", eVariationId::VAR_PERSPECTIVE, weight)
	{
		Init();
	}

	PARVARCOPY(PerspectiveVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T d = m_Weight / Zeps(m_Dist - helper.In.y * m_Vsin);
		helper.Out.x = d * m_Dist * helper.In.x;
		helper.Out.y = d * m_VfCos * helper.In.y;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vSin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string vfCos = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t d = " << weight << " / Zeps(" << dist << " - vIn.y * " << vSin << "); \n"
		   << "\n"
		   << "\t\tvOut.x = d * " << dist << " * vIn.x;\n"
		   << "\t\tvOut.y = d * " << vfCos << " * vIn.y;\n"
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
		T angle = m_Angle * T(M_PI) / 2;
		m_Vsin = std::sin(angle);
		m_VfCos = m_Dist * std::cos(angle);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Angle = rand.Frand01<T>();
		m_Dist = 2 * rand.Frand01<T>() + 1;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "perspective_angle"));
		m_Params.push_back(ParamWithName<T>(&m_Dist, prefix + "perspective_dist"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vsin, prefix + "perspective_vsin"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_VfCos, prefix + "perspective_vfcos"));
	}

private:
	T m_Angle;
	T m_Dist;
	T m_Vsin;//Precalc.
	T m_VfCos;
};

/// <summary>
/// Noise.
/// </summary>
template <typename T>
class NoiseVariation : public Variation<T>
{
public:
	NoiseVariation(T weight = 1.0) : Variation<T>("noise", eVariationId::VAR_NOISE, weight) { }

	VARCOPY(NoiseVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tempr = rand.Frand01<T>() * M_2PI;
		T r = rand.Frand01<T>() * m_Weight;
		helper.Out.x = helper.In.x * r * std::cos(tempr);
		helper.Out.y = helper.In.y * r * std::sin(tempr);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t tempr = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t r = MwcNext01(mwc) * " << weight << ";\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * r * cos(tempr);\n"
		   << "\t\tvOut.y = vIn.y * r * sin(tempr);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// JuliaN.
/// </summary>
template <typename T>
class JuliaNGenericVariation : public ParametricVariation<T>
{
public:
	JuliaNGenericVariation(T weight = 1.0) : ParametricVariation<T>("julian", eVariationId::VAR_JULIAN, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(JuliaNGenericVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a;
		T r = m_Weight * std::pow(helper.m_PrecalcSumSquares, m_Cn);

		if (Compat::m_Compat)
		{
			a = (helper.m_PrecalcAtanyx + M_2PI * rand.Rand(size_t(m_Rn))) / m_Power;
		}
		else
		{
            int root = int(m_Power * rand.Frand01<T>());
			a = (helper.m_PrecalcAtanyx + root * M_2PI) / m_Power;
		}

		helper.Out.x = r * std::cos(a);
		helper.Out.y = r * std::sin(a);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string cn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = " << weight << " * pow(precalcSumSquares, " << cn << ");\n";

		if (Compat::m_Compat)
		{
			ss << "\t\treal_t a = (precalcAtanyx + M_2PI * (int)(" << rn << " * MwcNext01(mwc))) / " << power << ";\n";
		}
		else
		{
			ss << "\t\tint root = (int)(" << power << " * MwcNext01(mwc));\n"
			   << "\t\treal_t a = (precalcAtanyx + root * M_2PI) / " << power << ";\n";
		}

		ss << "\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Power = Zeps(m_Power);
		m_Rn = std::abs(m_Power);
		m_Cn = m_Dist / m_Power * T(0.5);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Dist = 1;
		m_Power = T(int(5 * rand.Frand01<T>() + 2));
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Dist, prefix + "julian_dist", 1));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "julian_power", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Rn, prefix + "julian_rn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix + "julian_cn"));
	}

private:
	T m_Dist;
	T m_Power;
	T m_Rn;//Precalc.
	T m_Cn;
};

/// <summary>
/// JuliaScope.
/// </summary>
template <typename T>
class JuliaScopeVariation : public ParametricVariation<T>
{
public:
	JuliaScopeVariation(T weight = 1.0) : ParametricVariation<T>("juliascope", eVariationId::VAR_JULIASCOPE, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(JuliaScopeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a, r = m_Weight * std::pow(helper.m_PrecalcSumSquares, m_Cn);

		if (Compat::m_Compat)
		{
			int rnd = int(m_Rn * rand.Frand01<T>());

			if ((rnd & 1) == 0)
				a = (M_2PI * rnd + helper.m_PrecalcAtanyx) / m_Power;
			else
				a = (M_2PI * rnd - helper.m_PrecalcAtanyx) / m_Power;
		}
		else
		{
			auto root = Floor(m_Power * rand.Frand01<T>());
			auto root_div2 = Floor(root * T(0.5));
			auto root_mod2 = root - root_div2 * 2;
			T root_scale = T(root_mod2 == 1 ? -1 : 1);
			a = (helper.m_PrecalcAtanyx * root_scale + root * M_2PI) / Zeps(m_Power);
		}

		helper.Out.x = r * std::cos(a);
		helper.Out.y = r * std::sin(a);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string cn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a;\n"
		   << "\t\treal_t r = " << weight << " * pow(precalcSumSquares, " << cn << ");\n"
		   << "\t\treal_t power = " << power << ";\n"
		   << "\n";

		if (Compat::m_Compat)
		{
			ss << "\t\tint rnd = (int)(" << rn << " * MwcNext01(mwc));\n"
			   << "\n"
			   << "\t\tif ((rnd & 1) == 0)\n"
			   << "\t\t	a = fma(M_2PI, (real_t)rnd, precalcAtanyx) / power;\n"
			   << "\t\telse\n"
			   << "\t\t	a = fma(M_2PI, (real_t)rnd, -precalcAtanyx) / power;\n"
			   << "\n";
		}
		else
		{
			ss << "\t\tint root = (int)floor(power * MwcNext01(mwc));\n"
			   << "\t\tint root_div2 = (int)floor(root * (real_t)(0.5));\n"
			   << "\t\tint root_mod2 = root - root_div2 * 2;\n"
			   << "\t\tint root_scale = (real_t)(root_mod2 == 1 ? -1.0 : 1.0);\n"
			   << "\t\ta = (precalcAtanyx * root_scale + root * M_2PI) / power;\n";
		}

		ss << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Power = Zeps(m_Power);
		m_Rn = std::abs(m_Power);
		m_Cn = m_Dist / m_Power / 2;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Dist = 1;
		m_Power = T(int(5 * rand.Frand01<T>() + 2));
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Dist, prefix + "juliascope_dist", 1));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "juliascope_power", 1, eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_Rn, prefix + "juliascope_rn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix + "juliascope_cn"));
	}

private:
	T m_Dist;
	T m_Power;
	T m_Rn;//Precalc.
	T m_Cn;
};

/// <summary>
/// Blur.
/// This is somewhat different than the original functionality in that blur used
/// the code below, but pre_blur used gaussian_blur.
/// If the original pre_blur functionality is needed, use pre_gaussian_blur.
/// </summary>
template <typename T>
class BlurVariation : public Variation<T>
{
public:
	BlurVariation(T weight = 1.0) : Variation<T>("blur", eVariationId::VAR_BLUR, weight) { }

	VARCOPY(BlurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tempr = rand.Frand01<T>() * M_2PI;
		T r = m_Weight * rand.Frand01<T>();
		helper.Out.x = r * std::cos(tempr);
		helper.Out.y = r * std::sin(tempr);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t tmpr = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t r = " << weight << " * MwcNext01(mwc);\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(tmpr);\n"
		   << "\t\tvOut.y = r * sin(tmpr);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Gaussian blur.
/// </summary>
template <typename T>
class GaussianBlurVariation : public Variation<T>
{
public:
	GaussianBlurVariation(T weight = 1.0) : Variation<T>("gaussian_blur", eVariationId::VAR_GAUSSIAN_BLUR, weight) { }

	VARCOPY(GaussianBlurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = rand.Frand01<T>() * M_2PI;
		T r = m_Weight * (rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() - 2);
		helper.Out.x = r * std::cos(angle);
		helper.Out.y = r * std::sin(angle);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t angle = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t r = " << weight << " * (MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) - (real_t)(2.0));\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(angle);\n"
		   << "\t\tvOut.y = r * sin(angle);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Gaussian.
/// </summary>
template <typename T>
class GaussianVariation : public Variation<T>
{
public:
	GaussianVariation(T weight = 1.0) : Variation<T>("gaussian", eVariationId::VAR_GAUSSIAN, weight) { }

	VARCOPY(GaussianVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = rand.Frand01<T>() * M_2PI;
		T r = m_Weight * (rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() - 2);
		helper.Out.x = r * std::cos(angle) + helper.In.x;
		helper.Out.y = r * std::sin(angle) + helper.In.y;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t angle = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t r = " << weight << " * (MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) - (real_t)(2.0));\n"
		   << "\n"
		   << "\t\tvOut.x = fma(r, cos(angle), vIn.x);\n"
		   << "\t\tvOut.y = fma(r, sin(angle), vIn.y);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Radial blur.
/// </summary>
template <typename T>
class RadialBlurVariation : public ParametricVariation<T>
{
public:
	RadialBlurVariation(T weight = 1.0) : ParametricVariation<T>("radial_blur", eVariationId::VAR_RADIAL_BLUR, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(RadialBlurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		//Get pseudo-gaussian.
		T rndG = m_Weight * (rand.Frand01<T>() + rand.Frand01<T>()
							 + rand.Frand01<T>() + rand.Frand01<T>() - 2);
		//Calculate angle & zoom.
		T ra = helper.m_PrecalcSqrtSumSquares;
		T tempa = helper.m_PrecalcAtanyx + m_Spin * rndG;
		T rz = m_Zoom * rndG - 1;
		helper.Out.x = ra * std::cos(tempa) + rz * helper.In.x;
		helper.Out.y = ra * std::sin(tempa) + rz * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string zoom = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t rndG = " << weight << " * (MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) - (real_t)(2.0));\n"
		   << "\t\treal_t ra = precalcSqrtSumSquares;\n"
		   << "\t\treal_t tempa = fma(" << spin << ", rndG, precalcAtanyx);\n"
		   << "\t\treal_t rz = fma(" << zoom << ", rndG, (real_t)(-1.0));\n"
		   << "\n"
		   << "\t\tvOut.x = fma(ra, cos(tempa), rz * vIn.x);\n"
		   << "\t\tvOut.y = fma(ra, sin(tempa), rz * vIn.y);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		sincos(m_Angle * T(M_PI) / 2, &m_Spin, &m_Zoom);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Angle = (2 * rand.Frand01<T>() - 1);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "radial_blur_angle"));
		m_Params.push_back(ParamWithName<T>(true, &m_Spin, prefix + "radial_blur_spin"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Zoom, prefix + "radial_blur_zoom"));
	}

private:
	T m_Angle;
	T m_Spin;//Precalc.
	T m_Zoom;
};

/// <summary>
/// Pie.
/// </summary>
template <typename T>
class PieVariation : public ParametricVariation<T>
{
public:
	PieVariation(T weight = 1.0) : ParametricVariation<T>("pie", eVariationId::VAR_PIE, weight)
	{
		Init();
	}

	PARVARCOPY(PieVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		auto sl = rand.Rand(size_t(m_Slices));
		T a = m_Rotation + m_Pi2Slices * (sl + m_Thickness * rand.Frand01<T>());
		T r = m_Weight * rand.Frand01<T>();
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
		string slices = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotation = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thickness = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pi2Slices = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tuint sl = MwcNextRange(mwc, (uint)" << slices << ");\n"
		   << "\t\treal_t a = fma(" << pi2Slices << ", fma(" << thickness << ", MwcNext01(mwc), sl), " << rotation << ");\n"
		   << "\t\treal_t r = " << weight << " * MwcNext01(mwc);\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Pi2Slices = M_2PI / m_Slices;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Params[0].Set(10 * rand.Frand01<T>());//Slices.
		m_Params[1].Set(M_2PI * rand.Frand11<T>());//Rotation.
		m_Thickness = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Slices, prefix + "pie_slices", 6, eParamType::INTEGER_NONZERO, 1));
		m_Params.push_back(ParamWithName<T>(&m_Rotation, prefix + "pie_rotation", T(0.5), eParamType::REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Thickness, prefix + "pie_thickness", T(0.5), eParamType::REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Pi2Slices, prefix + "pie_pi2_slices"));//Precalc.
	}

private:
	T m_Slices;
	T m_Rotation;
	T m_Thickness;
	T m_Pi2Slices;//Precalc
};

/// <summary>
/// Ngon.
/// </summary>
template <typename T>
class NgonVariation : public ParametricVariation<T>
{
public:
	NgonVariation(T weight = 1.0) : ParametricVariation<T>("ngon", eVariationId::VAR_NGON, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(NgonVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T rFactor;

		if ((helper.In.x == 0) && (helper.In.y == 0))
			rFactor = 0;
		else
			rFactor = std::pow(helper.m_PrecalcSumSquares, m_CPower);

		T phi = helper.m_PrecalcAtanyx - m_CSides * Floor<T>(helper.m_PrecalcAtanyx * m_CSidesInv);

		if (phi > T(0.5) * m_CSides)
			phi -= m_CSides;

		T amp = (m_Corners * (1 / std::cos(phi) - 1) + m_Circle) * m_Weight * rFactor;
		helper.Out.x = amp * helper.In.x;
		helper.Out.y = amp * helper.In.y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string sides     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string circle    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string corners   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string csides    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string csidesinv = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cpower    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t rFactor;\n"
		   << "\n"
		   << "\t\tif ((vIn.x == (real_t)(0.0)) && (vIn.y == (real_t)(0.0)))\n"
		   << "\t\t	rFactor = (real_t)(0.0);\n"
		   << "\t\telse\n"
		   << "\t\t	rFactor = pow(precalcSumSquares, " << cpower << ");\n"
		   << "\n"
		   << "\t\treal_t phi = precalcAtanyx - " << csides << " * floor(precalcAtanyx * " << csidesinv << ");\n"
		   << "\n"
		   << "\t\tif (phi > (real_t)(0.5) * " << csides << ")\n"
		   << "\t\t	phi -= " << csides << ";\n"
		   << "\n"
		   << "\t\treal_t amp = fma(" << corners << ", (1 / cos(phi) - 1), " << circle << ") * " << weight << " * rFactor;\n"
		   << "\n"
		   << "\t\tvOut.x = amp * vIn.x;\n"
		   << "\t\tvOut.y = amp * vIn.y;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_CPower = -T(0.5) * m_Power;
		m_CSides = 2 * T(M_PI) / m_Sides;
		m_CSidesInv = 1 / m_CSides;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Sides = T(int(rand.Frand01<T>() * 10 + 3));
		m_Power = 3 * rand.Frand01<T>() + 1;
		m_Circle = 3 * rand.Frand01<T>();
		m_Corners = 2 * rand.Frand01<T>() * m_Circle;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Sides, prefix + "ngon_sides", 5, eParamType::INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "ngon_power", 3));
		m_Params.push_back(ParamWithName<T>(&m_Circle, prefix + "ngon_circle", 1));
		m_Params.push_back(ParamWithName<T>(&m_Corners, prefix + "ngon_corners", 2));
		m_Params.push_back(ParamWithName<T>(true, &m_CSides, prefix + "ngon_csides"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_CSidesInv, prefix + "ngon_csides_inv"));
		m_Params.push_back(ParamWithName<T>(true, &m_CPower, prefix + "ngon_cpower"));
	}

private:
	T m_Sides;
	T m_Power;
	T m_Circle;
	T m_Corners;
	T m_CSides;
	T m_CSidesInv;
	T m_CPower;
};

/// <summary>
/// Curl.
/// Note that in Apophysis, curl and post_curl differed slightly.
/// Using what post_curl did here gave bad results, so sticking with the original
/// curl code.
/// </summary>
template <typename T>
class CurlVariation : public ParametricVariation<T>
{
public:
	CurlVariation(T weight = 1.0) : ParametricVariation<T>("curl", eVariationId::VAR_CURL, weight)
	{
		Init();
	}

	PARVARCOPY(CurlVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T re = 1 + m_C1 * helper.In.x + m_C2 * (SQR(helper.In.x) - SQR(helper.In.y));
		T im = (m_C1 + m_C22 * helper.In.x) * helper.In.y;
		T r = m_Weight / Zeps(SQR(re) + SQR(im));
		helper.Out.x = (helper.In.x * re + helper.In.y * im) * r;
		helper.Out.y = (helper.In.y * re - helper.In.x * im) * r;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string c1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c22 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t re = (real_t)(1.0) + " << c1 << " * vIn.x + " << c2 << " * fma(vIn.x, vIn.x, -SQR(vIn.y));\n"
		   << "\t\treal_t im = fma(" << c22 << ", vIn.x, " << c1 << ") * vIn.y;\n"
		   << "\t\treal_t r = " << weight << " / Zeps(fma(re, re, SQR(im)));\n"
		   << "\n"
		   << "\t\tvOut.x = fma(vIn.x, re, vIn.y * im) * r;\n"
		   << "\t\tvOut.y = fma(vIn.y, re, -(vIn.x * im)) * r;\n"
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
		m_C22 = 2 * m_C2;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_C1 = rand.Frand01<T>();
		m_C2 = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_C1, prefix + "curl_c1", 1));
		m_Params.push_back(ParamWithName<T>(&m_C2, prefix + "curl_c2"));
		m_Params.push_back(ParamWithName<T>(true, &m_C22, prefix + "curl_c22"));//Precalc.
	}

private:
	T m_C1;
	T m_C2;
	T m_C22;//Precalc.
};

/// <summary>
/// Rectangles.
/// </summary>
template <typename T>
class RectanglesVariation : public ParametricVariation<T>
{
public:
	RectanglesVariation(T weight = 1.0) : ParametricVariation<T>("rectangles", eVariationId::VAR_RECTANGLES, weight)
	{
		Init();
	}

	PARVARCOPY(RectanglesVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_X == 0)
			helper.Out.x = m_Weight * helper.In.x;
		else
			helper.Out.x = m_Weight * ((2 * Floor<T>(helper.In.x / m_X) + 1) * m_X - helper.In.x);

		if (m_Y == 0)
			helper.Out.y = m_Weight * helper.In.y;
		else
			helper.Out.y = m_Weight * ((2 * Floor<T>(helper.In.y / m_Y) + 1) * m_Y - helper.In.y);

		helper.Out.z = m_Weight * helper.In.z;
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
		   << "\t\tif (" << x << " == 0)\n"
		   << "\t\t	vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = " << weight << " * fma(fma((real_t)(2.0), floor(vIn.x / " << x << "), 1), " << x << ", -vIn.x);\n"
		   << "\n"
		   << "\t\tif (" << y << " == 0)\n"
		   << "\t\t	vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = " << weight << " * fma(fma((real_t)(2.0), floor(vIn.y / " << y << "), 1), " << y << ", -vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = rand.Frand01<T>();
		m_Y = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "rectangles_x", 1));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "rectangles_y", 1));
	}

private:
	T m_X;
	T m_Y;
};

/// <summary>
/// Arch.
/// </summary>
template <typename T>
class ArchVariation : public Variation<T>
{
public:
	ArchVariation(T weight = 1.0) : Variation<T>("arch", eVariationId::VAR_ARCH, weight) { }

	VARCOPY(ArchVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = rand.Frand01<T>() * m_Weight * T(M_PI);
		T sinr, cosr;
		sincos(angle, &sinr, &cosr);
		helper.Out.x = m_Weight * sinr;
		helper.Out.y = m_Weight * (sinr * sinr) / cosr;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t angle = MwcNext01(mwc) * " << weight << " * MPI;\n"
		   << "\t\treal_t sinr = sin(angle);\n"
		   << "\t\treal_t cosr = cos(angle);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * sinr;\n"
		   << "\t\tvOut.y = " << weight << " * (sinr * sinr) / cosr;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Tangent.
/// </summary>
template <typename T>
class TangentVariation : public Variation<T>
{
public:
	TangentVariation(T weight = 1.0) : Variation<T>("tangent", eVariationId::VAR_TANGENT, weight) { }

	VARCOPY(TangentVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * std::sin(helper.In.x) / std::cos(helper.In.y);
		helper.Out.y = m_Weight * SafeTan<T>(helper.In.y);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * sin(vIn.x) / cos(vIn.y);\n"
		   << "\t\tvOut.y = " << weight << " * tan(vIn.y);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Square.
/// </summary>
template <typename T>
class SquareVariation : public Variation<T>
{
public:
	SquareVariation(T weight = 1.0) : Variation<T>("square", eVariationId::VAR_SQUARE, weight) { }

	VARCOPY(SquareVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (rand.Frand01<T>() - T(0.5));
		helper.Out.y = m_Weight * (rand.Frand01<T>() - T(0.5));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * (MwcNext01(mwc) - (real_t)(0.5));\n"
		   << "\t\tvOut.y = " << weight << " * (MwcNext01(mwc) - (real_t)(0.5));\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Rays.
/// </summary>
template <typename T>
class RaysVariation : public Variation<T>
{
public:
	RaysVariation(T weight = 1.0) : Variation<T>("rays", eVariationId::VAR_RAYS, weight, true) { }

	VARCOPY(RaysVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T ang = m_Weight * rand.Frand01<T>() * T(M_PI);
		T r = m_Weight / Zeps(helper.m_PrecalcSumSquares);
		T tanr = m_Weight * SafeTan<T>(ang) * r;
		helper.Out.x = tanr * std::cos(helper.In.x);
		helper.Out.y = tanr * std::sin(helper.In.y);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t ang = " << weight << " * MwcNext01(mwc) * MPI;\n"
		   << "\t\treal_t r = " << weight << " / Zeps(precalcSumSquares);\n"
		   << "\t\treal_t tanr = " << weight << " * tan(ang) * r;\n"
		   << "\n"
		   << "\t\tvOut.x = tanr * cos(vIn.x);\n"
		   << "\t\tvOut.y = tanr * sin(vIn.y);\n"
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
/// Rays1 from JWildfire by Raykoid666.
/// </summary>
template <typename T>
class Rays1Variation : public Variation<T>
{
public:
	Rays1Variation(T weight = 1.0) : Variation<T>("rays1", eVariationId::VAR_RAYS1, weight, true, true) { }

	VARCOPY(Rays1Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T u = 1 / Zeps(SafeTan<T>(helper.m_PrecalcSqrtSumSquares)) + (m_Weight * T(SQR(M_2_PI)));

		if (m_VarType == eVariationType::VARTYPE_REG)
			outPoint.m_X = outPoint.m_Y = 0;//This variation assigns, instead of summing, so order will matter.

		helper.Out.x = m_Weight * u * helper.m_PrecalcSumSquares / Zeps(helper.In.x);
		helper.Out.y = m_Weight * u * helper.m_PrecalcSumSquares / Zeps(helper.In.y);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t u = fma(" << weight << ", SQR(M2PI), 1 / Zeps(tan(precalcSqrtSumSquares)));\n";

		if (m_VarType == eVariationType::VARTYPE_REG)
			ss << "\t\toutPoint->m_X = outPoint->m_Y = 0;\n";

		ss
				<< "\t\tvOut.x = " << weight << " * u * precalcSumSquares / Zeps(vIn.x);\n"
				<< "\t\tvOut.y = " << weight << " * u * precalcSumSquares / Zeps(vIn.y);\n"
				<< "\t\tvOut.z = " << weight << " * vIn.z;\n"
				<< "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Rays2 from JWildfire by Raykoid666.
/// </summary>
template <typename T>
class Rays2Variation : public Variation<T>
{
public:
	Rays2Variation(T weight = 1.0) : Variation<T>("rays2", eVariationId::VAR_RAYS2, weight, true) { }

	VARCOPY(Rays2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T u = 1 / std::cos(helper.m_PrecalcSumSquares * std::tan(1 / Zeps(helper.m_PrecalcSumSquares)));

		if (m_VarType == eVariationType::VARTYPE_REG)
			outPoint.m_X = outPoint.m_Y = 0;//This variation assigns, instead of summing, so order will matter.

		helper.Out.x = (m_Weight / 10) * u * helper.m_PrecalcSumSquares / Zeps(helper.In.x);
		helper.Out.y = (m_Weight / 10) * u * helper.m_PrecalcSumSquares / Zeps(helper.In.y);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t u = 1 / cos(precalcSumSquares * tan(1 / Zeps(precalcSumSquares)));\n";

		if (m_VarType == eVariationType::VARTYPE_REG)
			ss << "\t\toutPoint->m_X = outPoint->m_Y = 0;\n";

		ss
				<< "\t\tvOut.x = (" << weight << " / 10) * u * precalcSumSquares / Zeps(vIn.x);\n"
				<< "\t\tvOut.y = (" << weight << " / 10) * u * precalcSumSquares / Zeps(vIn.y);\n"
				<< "\t\tvOut.z = " << weight << " * vIn.z;\n"
				<< "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Rays3 from JWildfire by Raykoid666.
/// </summary>
template <typename T>
class Rays3Variation : public Variation<T>
{
public:
	Rays3Variation(T weight = 1.0) : Variation<T>("rays3", eVariationId::VAR_RAYS3, weight, true) { }

	VARCOPY(Rays3Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sq = SQR(helper.m_PrecalcSumSquares);
		T u = 1 / std::sqrt(std::cos(std::sin(sq) * std::sin(1 / Zeps(sq))));

		if (m_VarType == eVariationType::VARTYPE_REG)
			outPoint.m_X = outPoint.m_Y = 0;//This variation assigns, instead of summing, so order will matter.

		helper.Out.x = (m_Weight / 10) * u * std::cos(helper.m_PrecalcSumSquares) * helper.m_PrecalcSumSquares / Zeps(helper.In.x);
		helper.Out.y = (m_Weight / 10) * u * std::tan(helper.m_PrecalcSumSquares) * helper.m_PrecalcSumSquares / Zeps(helper.In.y);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t sq = SQR(precalcSumSquares);\n"
		   << "\t\treal_t u = 1 / sqrt(cos(sin(sq) * sin(1 / Zeps(sq))));\n";

		if (m_VarType == eVariationType::VARTYPE_REG)
			ss << "\t\toutPoint->m_X = outPoint->m_Y = 0;\n";

		ss
				<< "\t\tvOut.x = (" << weight << " / 10) * u * cos(precalcSumSquares) * precalcSumSquares / Zeps(vIn.x);\n"
				<< "\t\tvOut.y = (" << weight << " / 10) * u * tan(precalcSumSquares) * precalcSumSquares / Zeps(vIn.y);\n"
				<< "\t\tvOut.z = " << weight << " * vIn.z;\n"
				<< "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Blade.
/// </summary>
template <typename T>
class BladeVariation : public Variation<T>
{
public:
	BladeVariation(T weight = 1.0) : Variation<T>("blade", eVariationId::VAR_BLADE, weight, true, true) { }

	VARCOPY(BladeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = rand.Frand01<T>() * m_Weight * helper.m_PrecalcSqrtSumSquares;
		T sinr, cosr;
		sincos(r, &sinr, &cosr);
		helper.Out.x = m_Weight * helper.In.x * (cosr + sinr);
		helper.Out.y = m_Weight * helper.In.x * (cosr - sinr);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r = MwcNext01(mwc) * " << weight << " * precalcSqrtSumSquares;\n"
		   << "\t\treal_t sinr = sin(r);\n"
		   << "\t\treal_t cosr = cos(r);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * vIn.x * (cosr + sinr);\n"
		   << "\t\tvOut.y = " << weight << " * vIn.x * (cosr - sinr);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Secant2.
/// </summary>
template <typename T>
class Secant2Variation : public Variation<T>
{
public:
	Secant2Variation(T weight = 1.0) : Variation<T>("secant2", eVariationId::VAR_SECANT2, weight, true, true) { }

	VARCOPY(Secant2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight * helper.m_PrecalcSqrtSumSquares;
		T cr = std::cos(r);
		T icr = 1 / cr;
		helper.Out.x = m_Weight * helper.In.x;

		if (cr < 0)
			helper.Out.y = m_Weight * (icr + 1);
		else
			helper.Out.y = m_Weight * (icr - 1);

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r = " << weight << " * precalcSqrtSumSquares;\n"
		   << "\t\treal_t cr = cos(r);\n"
		   << "\t\treal_t icr = (real_t)(1.0) / cr;\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * vIn.x;\n"
		   << "\n"
		   << "\t\tif (cr < (real_t)(0.0))\n"
		   << "\t\t	vOut.y = " << weight << " * (icr + (real_t)(1.0));\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = " << weight << " * (icr - (real_t)(1.0));\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// TwinTrian.
/// </summary>
template <typename T>
class TwinTrianVariation : public Variation<T>
{
public:
	TwinTrianVariation(T weight = 1.0) : Variation<T>("TwinTrian", eVariationId::VAR_TWINTRIAN, weight, true, true) { }

	VARCOPY(TwinTrianVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = rand.Frand01<T>() * m_Weight * helper.m_PrecalcSqrtSumSquares;
		T sinr, cosr, diff;
		sincos(r, &sinr, &cosr);
		diff = std::log10(SQR(sinr)) + cosr;

		if (BadVal(diff))
			diff = -30.0;

		helper.Out.x = m_Weight * helper.In.x * diff;
		helper.Out.y = m_Weight * helper.In.x * (diff - sinr * T(M_PI));
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r = MwcNext01(mwc) * " << weight << " * precalcSqrtSumSquares;\n"
		   << "\t\treal_t sinr = sin(r);\n"
		   << "\t\treal_t cosr = cos(r);\n"
		   << "\t\treal_t diff = log10(SQR(sinr)) + cosr;\n"
		   << "\n"
		   << "\t\tif (BadVal(diff))\n"
		   << "\t\t	diff = -(real_t)(30.0);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * vIn.x * diff;\n"
		   << "\t\tvOut.y = " << weight << " * vIn.x * (diff - sinr * MPI);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Cross.
/// </summary>
template <typename T>
class CrossVariation : public Variation<T>
{
public:
	CrossVariation(T weight = 1.0) : Variation<T>("cross", eVariationId::VAR_CROSS, weight) { }

	VARCOPY(CrossVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r;

		if (Compat::m_Compat)
		{
			r = m_Weight / Zeps(std::abs((helper.In.x - helper.In.y) * (helper.In.x + helper.In.y)));
		}
		else
		{
			T s = Zeps(Sqr(helper.In.x) - Sqr(helper.In.y));
			r = m_Weight / s;
		}

		helper.Out.x = helper.In.x * r;
		helper.Out.y = helper.In.y * r;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r;\n";

		if (Compat::m_Compat)
		{
			ss << "\t\tr = " << weight << " / Zeps(fabs((vIn.x - vIn.y) * (vIn.x + vIn.y)));\n";
		}
		else
		{
			ss << "\t\treal_t s = Zeps(SQR(vIn.x) - SQR(vIn.y));\n"
			   << "\t\tr = " << weight << " / s;\n";
		}

		ss   << "\n"
			 << "\t\tvOut.x = vIn.x * r;\n"
			 << "\t\tvOut.y = vIn.y * r;\n"
			 << "\t\tvOut.z = " << weight << " * vIn.z;\n"
			 << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Disc2.
/// </summary>
template <typename T>
class Disc2Variation : public ParametricVariation<T>
{
public:
	Disc2Variation(T weight = 1.0) : ParametricVariation<T>("disc2", eVariationId::VAR_DISC2, weight, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Disc2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r, t, sinr, cosr;
		t = m_RotTimesPi * (helper.In.x + helper.In.y);
		sincos(t, &sinr, &cosr);
		r = m_Weight * helper.m_PrecalcAtanxy / T(M_PI);
		helper.Out.x = (sinr + m_CosAdd) * r;
		helper.Out.y = (cosr + m_SinAdd) * r;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string rot = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string twist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sinAdd = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string cosAdd = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rotTimesPi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t t = " << rotTimesPi << " * (vIn.x + vIn.y);\n"
		   << "\t\treal_t sinr = sin(t);\n"
		   << "\t\treal_t cosr = cos(t);\n"
		   << "\t\treal_t r = " << weight << " * precalcAtanxy / MPI;\n"
		   << "\n"
		   << "\t\tvOut.x = (sinr + " << cosAdd << ") * r;\n"
		   << "\t\tvOut.y = (cosr + " << sinAdd << ") * r;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T k, add = m_Twist;
		m_RotTimesPi = m_Rot * T(M_PI);
		sincos(add, &m_SinAdd, &m_CosAdd);
		m_CosAdd -= 1;

		if (add > 2 * M_PI)
		{
			k = (1 + add - 2 * T(M_PI));
			m_CosAdd *= k;
			m_SinAdd *= k;
		}

		if (add < -2 * M_PI)
		{
			k = (1 + add + 2 * T(M_PI));
			m_CosAdd *= k;
			m_SinAdd *= k;
		}
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Rot = T(0.5) * rand.Frand01<T>();
		m_Twist = T(0.5) * rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Rot, prefix + "disc2_rot"));
		m_Params.push_back(ParamWithName<T>(&m_Twist, prefix + "disc2_twist"));
		m_Params.push_back(ParamWithName<T>(true, &m_SinAdd, prefix + "disc2_sin_add"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_CosAdd, prefix + "disc2_cos_add"));
		m_Params.push_back(ParamWithName<T>(true, &m_RotTimesPi, prefix + "disc2_rot_times_pi"));
	}

private:
	T m_Rot;
	T m_Twist;
	T m_SinAdd;//Precalc.
	T m_CosAdd;
	T m_RotTimesPi;
};

/// <summary>
/// SuperShape.
/// </summary>
template <typename T>
class SuperShapeVariation : public ParametricVariation<T>
{
public:
	SuperShapeVariation(T weight = 1.0) : ParametricVariation<T>("super_shape", eVariationId::VAR_SUPER_SHAPE, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(SuperShapeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T theta = m_Pm4 * helper.m_PrecalcAtanyx + T(M_PI_4);
		T t1 = std::abs(std::cos(theta));
		t1 = std::pow(t1, m_N2);
		T t2 = std::abs(std::sin(theta));
		t2 = std::pow(t2, m_N3);
		T r = m_Weight * ((m_Rnd * rand.Frand01<T>() + (1 - m_Rnd) * helper.m_PrecalcSqrtSumSquares) - m_Holes)
			  * std::pow(t1 + t2, m_PNeg1N1) / helper.m_PrecalcSqrtSumSquares;
		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * helper.In.y;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string m       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n1      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n2      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n3      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rnd     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string holes   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pm4     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string pNeg1N1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t theta = " << pm4 << " * precalcAtanyx + MPI4;\n"
		   << "\t\treal_t t1 = fabs(cos(theta));\n"
		   << "\t\tt1 = pow(t1, " << n2 << ");\n"
		   << "\t\treal_t t2 = fabs(sin(theta));\n"
		   << "\t\tt2 = pow(t2, " << n3 << ");\n"
		   << "\t\treal_t r = " << weight << " * (fma(" << rnd << ", MwcNext01(mwc), ((real_t)(1.0) - " << rnd << ") * precalcSqrtSumSquares) - " << holes << ") * pow(t1 + t2, " << pNeg1N1 << ") / precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * vIn.y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Pm4 = m_M / T(4.0);
		m_PNeg1N1 = T(-1.0) / m_N1;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Rnd = rand.Frand01<T>();
		m_M = T(int(rand.Frand01<T>() * 6));
		m_N1 = rand.Frand01<T>() * 40;
		m_N2 = rand.Frand01<T>() * 20;
		m_N3 = m_N2;
		m_Holes = 0.0;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_M, prefix + "super_shape_m"));
		m_Params.push_back(ParamWithName<T>(&m_N1, prefix + "super_shape_n1", 1));
		m_Params.push_back(ParamWithName<T>(&m_N2, prefix + "super_shape_n2", 1));
		m_Params.push_back(ParamWithName<T>(&m_N3, prefix + "super_shape_n3", 1));
		m_Params.push_back(ParamWithName<T>(&m_Rnd, prefix + "super_shape_rnd"));
		m_Params.push_back(ParamWithName<T>(&m_Holes, prefix + "super_shape_holes"));
		m_Params.push_back(ParamWithName<T>(true, &m_Pm4, prefix + "super_shape_pm4"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_PNeg1N1, prefix + "super_shape_pneg1n1"));
	}

private:
	T m_M;
	T m_N1;
	T m_N2;
	T m_N3;
	T m_Rnd;
	T m_Holes;
	T m_Pm4;//Precalc.
	T m_PNeg1N1;
};

/// <summary>
/// Flower.
/// </summary>
template <typename T>
class FlowerVariation : public ParametricVariation<T>
{
public:
	FlowerVariation(T weight = 1.0) : ParametricVariation<T>("flower", eVariationId::VAR_FLOWER, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(FlowerVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T theta = helper.m_PrecalcAtanyx;
		T r = m_Weight * (rand.Frand01<T>() - m_Holes) * std::cos(m_Petals * theta) / helper.m_PrecalcSqrtSumSquares;
		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * helper.In.y;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string petals = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string holes = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t theta = precalcAtanyx;\n"
		   << "\t\treal_t r = " << weight << " * (MwcNext01(mwc) - " << holes << ") * cos(" << petals << " * theta) / precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * vIn.y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Petals = 4 * rand.Frand01<T>();
		m_Holes = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Petals, prefix + "flower_petals"));
		m_Params.push_back(ParamWithName<T>(&m_Holes, prefix + "flower_holes"));
	}

private:
	T m_Petals;
	T m_Holes;
};

/// <summary>
/// flowerdb.
/// By dark-beam.
/// </summary>
template <typename T>
class FlowerDbVariation : public ParametricVariation<T>
{
public:
	FlowerDbVariation(T weight = 1.0) : ParametricVariation<T>("flowerdb", eVariationId::VAR_FLOWER_DB, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(FlowerDbVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight * helper.m_PrecalcSqrtSumSquares;
		T t = helper.m_PrecalcAtanyx;
		T r2 = r * (std::abs((m_Spread + std::sin(m_Petals * t)) * cos(m_PetalsSplit * t)));
		helper.Out.x = r2 * std::cos(t);
		helper.Out.y = r2 * std::sin(t);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string petals      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string split       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spread      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string petalssplit = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = " << weight << " * precalcSqrtSumSquares;\n"
		   << "\t\treal_t t = precalcAtanyx;\n"
		   << "\t\treal_t r2 = r * (fabs((" << spread << " + sin(" << petals << " * t)) * cos(" << petalssplit << " * t)));\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(t);\n"
		   << "\t\tvOut.y = r * sin(t);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_PetalsSplit = m_Petals * m_Split;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Petals,            prefix + "flowerdb_petals", 6));
		m_Params.push_back(ParamWithName<T>(&m_Split,             prefix + "flowerdb_petal_split"));
		m_Params.push_back(ParamWithName<T>(&m_Spread,            prefix + "flowerdb_petal_spread", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_PetalsSplit, prefix + "flowerdb_petal_split_petals"));//Precalc.
	}

private:
	T m_Petals;
	T m_Split;
	T m_Spread;
	T m_PetalsSplit;//Precalc.
};

/// <summary>
/// Conic.
/// </summary>
template <typename T>
class ConicVariation : public ParametricVariation<T>
{
public:
	ConicVariation(T weight = 1.0) : ParametricVariation<T>("conic", eVariationId::VAR_CONIC, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(ConicVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T z = Zeps(helper.m_PrecalcSqrtSumSquares);
		T ct = helper.In.x / z;
		T r = m_Weight * (rand.Frand01<T>() - m_Holes) *
			  m_Eccentricity / (1 + m_Eccentricity * ct) / z;
		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * helper.In.y;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string eccentricity = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string holes = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t z = Zeps(precalcSqrtSumSquares);\n"
		   << "\t\treal_t ct = vIn.x / precalcSqrtSumSquares;\n"
		   << "\t\treal_t r = " << weight << " * (MwcNext01(mwc) - " << holes << ") * " << eccentricity << " / fma(" << eccentricity << ", ct, 1) / z;\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * vIn.y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Eccentricity = rand.Frand01<T>();
		m_Holes = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Eccentricity, prefix + "conic_eccentricity", 1));
		m_Params.push_back(ParamWithName<T>(&m_Holes, prefix + "conic_holes"));
	}

private:
	T m_Eccentricity;
	T m_Holes;
};

/// <summary>
/// Parabola.
/// </summary>
template <typename T>
class ParabolaVariation : public ParametricVariation<T>
{
public:
	ParabolaVariation(T weight = 1.0) : ParametricVariation<T>("parabola", eVariationId::VAR_PARABOLA, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(ParabolaVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sr, cr;
		sincos(helper.m_PrecalcSqrtSumSquares, &sr, &cr);
		helper.Out.x = m_Height * m_Weight * sr * sr * rand.Frand01<T>();
		helper.Out.y = m_Width * m_Weight * cr * rand.Frand01<T>();
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string height = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string width = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t sr = sin(precalcSqrtSumSquares);\n"
		   << "\t\treal_t cr = cos(precalcSqrtSumSquares);\n"
		   << "\n"
		   << "\t\tvOut.x = " << height << " * (" << weight << " * sr * sr * MwcNext01(mwc));\n"
		   << "\t\tvOut.y = " << width << " * (" << weight << " * cr * MwcNext01(mwc));\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Height = T(0.5) * rand.Frand01<T>();
		m_Width = T(0.5) * rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Height, prefix + "parabola_height"));
		m_Params.push_back(ParamWithName<T>(&m_Width, prefix + "parabola_width"));
	}

private:
	T m_Height;
	T m_Width;
};

/// <summary>
/// Bent2.
/// </summary>
template <typename T>
class Bent2Variation : public ParametricVariation<T>
{
public:
	Bent2Variation(T weight = 1.0) : ParametricVariation<T>("bent2", eVariationId::VAR_BENT2, weight)
	{
		Init();
	}

	PARVARCOPY(Bent2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.In.x >= 0)
			helper.Out.x = m_Weight * helper.In.x;
		else
			helper.Out.x = m_Vx * helper.In.x;

		if (helper.In.y >= 0)
			helper.Out.y = m_Weight * helper.In.y;
		else
			helper.Out.y = m_Vy * helper.In.y;

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
		string vx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tif (vIn.x >= 0)\n"
		   << "\t\t	vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = " << vx << " * vIn.x;\n"
		   << "\n"
		   << "\t\tif (vIn.y >= 0)\n"
		   << "\t\t	vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\telse\n"
		   << "\t\tvOut.y = " << vy << " * vIn.y;\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Vx = m_X * m_Weight;
		m_Vy = m_Y * m_Weight;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = 3 * (T(-0.5) + rand.Frand01<T>());
		m_Y = 3 * (T(-0.5) + rand.Frand01<T>());
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "bent2_x", 1));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "bent2_y", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Vx, prefix + "bent2_vx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Vy, prefix + "bent2_vy"));
	}

private:
	T m_X;
	T m_Y;
	T m_Vx;//Precalc.
	T m_Vy;
};

/// <summary>
/// Bipolar.
/// </summary>
template <typename T>
class BipolarVariation : public ParametricVariation<T>
{
public:
	BipolarVariation(T weight = 1.0) : ParametricVariation<T>("bipolar", eVariationId::VAR_BIPOLAR, weight, true)
	{
		Init();
	}

	PARVARCOPY(BipolarVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		const T x2y2 = helper.m_PrecalcSumSquares;
		const T t = x2y2 + 1;
		const T x2 = 2 * helper.In.x;
		T y = T(0.5) * std::atan2(2 * helper.In.y, x2y2 - 1) + m_S;

		if (y > T(M_PI_2))
			y = -T(M_PI_2) + fmod(y + T(M_PI_2), T(M_PI));
		else if (y < -T(M_PI_2))
			y = T(M_PI_2) - fmod(T(M_PI_2) - y, T(M_PI));

		const T f = t + x2;
		const T g = t - x2;
		const T foverg = f / Zeps(g);

		if ((g == 0) || (foverg <= 0))
		{
			if (Compat::m_Compat && m_VarType == eVariationType::VARTYPE_REG)
			{
				helper.Out.x = 0;
				helper.Out.y = 0;
				helper.Out.z = 0;
			}
			else
			{
				helper.Out.x = helper.In.x;
				helper.Out.y = helper.In.y;
				helper.Out.z = helper.In.z;
			}
		}
		else
		{
			helper.Out.x = m_V4 * std::log(foverg);
			helper.Out.y = m_V * y;
			helper.Out.z = m_Weight * helper.In.z;
		}
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string shift = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string v = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string v4 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x2y2 = precalcSumSquares;\n"
		   << "\t\treal_t t = x2y2 + 1;\n"
		   << "\t\treal_t x2 = 2 * vIn.x;\n"
		   << "\t\treal_t ps = " << s << ";\n"
		   << "\t\treal_t y = (real_t)(0.5) * atan2((real_t)(2.0) * vIn.y, x2y2 - (real_t)(1.0)) + ps;\n"
		   << "\n"
		   << "\t\tif (y > MPI2)\n"
		   << "\t\t	y = -MPI2 + fmod(y + MPI2, MPI);\n"
		   << "\t\telse if (y < -MPI2)\n"
		   << "\t\t	y = MPI2 - fmod(MPI2 - y, MPI);\n"
		   << "\n"
		   << "\t\treal_t f = t + x2;\n"
		   << "\t\treal_t g = t - x2;\n"
		   << "\t\treal_t foverg = f / Zeps(g);\n"
		   << "\n";

		if (Compat::m_Compat && m_VarType == eVariationType::VARTYPE_REG)
		{
			ss << "\t\tif ((g == 0) || (foverg <= 0))\n"
			   << "\t\t{\n"
			   << "\t\t	vOut.x = 0;\n"
			   << "\t\t	vOut.y = 0;\n"
			   << "\t\t	vOut.z = 0;\n"
			   << "\t\t}\n";
		}
		else
		{
			ss << "\t\tif ((g == 0) || (foverg <= 0))\n"
			   << "\t\t{\n"
			   << "\t\t	vOut.x = vIn.x;\n"
			   << "\t\t	vOut.y = vIn.y;\n"
			   << "\t\t	vOut.z = vIn.z;\n"
			   << "\t\t}\n";
		}

		ss << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << v4 << " * log(foverg);\n"
		   << "\t\t	vOut.y = " << v << " * y;\n"
		   << "\t\t	vOut.z = " << weight << " * vIn.z;\n"
		   << "\t\t}\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_S = -T(M_PI_2) * m_Shift;
		m_V = m_Weight * T(M_2_PI);
		m_V4 = m_Weight * T(0.25) * T(M_2_PI);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Shift = 2 * rand.Frand01<T>() - 1;
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
		m_Params.push_back(ParamWithName<T>(&m_Shift, prefix + "bipolar_shift"));
		m_Params.push_back(ParamWithName<T>(true, &m_S, prefix + "bipolar_s"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_V, prefix + "bipolar_v"));
		m_Params.push_back(ParamWithName<T>(true, &m_V4, prefix + "bipolar_v4"));
	}

private:
	T m_Shift;
	T m_S;//Precalc.
	T m_V;
	T m_V4;
};

/// <summary>
/// Boarders.
/// </summary>
template <typename T>
class BoardersVariation : public Variation<T>
{
public:
	BoardersVariation(T weight = 1.0) : Variation<T>("boarders", eVariationId::VAR_BOARDERS, weight) { }

	VARCOPY(BoardersVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T roundX = std::rint(helper.In.x);
		T roundY = std::rint(helper.In.y);
		T offsetX = helper.In.x - roundX;
		T offsetY = helper.In.y - roundY;

		if (rand.Frand01<T>() >= T(0.75))
		{
			helper.Out.x = m_Weight * (offsetX * T(0.5) + roundX);
			helper.Out.y = m_Weight * (offsetY * T(0.5) + roundY);
		}
		else
		{
			if (std::abs(offsetX) >= std::abs(offsetY))
			{
				if (offsetX >= 0)
				{
					helper.Out.x = m_Weight * (offsetX * T(0.5) + roundX + T(0.25));
					helper.Out.y = m_Weight * (offsetY * T(0.5) + roundY + T(0.25) * offsetY / offsetX);
				}
				else
				{
					helper.Out.x = m_Weight * (offsetX * T(0.5) + roundX - T(0.25));
					helper.Out.y = m_Weight * (offsetY * T(0.5) + roundY - T(0.25) * offsetY / offsetX);
				}
			}
			else
			{
				if (offsetY >= 0)
				{
					helper.Out.y = m_Weight * (offsetY * T(0.5) + roundY + T(0.25));
					helper.Out.x = m_Weight * (offsetX * T(0.5) + roundX + offsetX / offsetY * T(0.25));
				}
				else
				{
					helper.Out.y = m_Weight * (offsetY * T(0.5) + roundY - T(0.25));
					helper.Out.x = m_Weight * (offsetX * T(0.5) + roundX - offsetX / offsetY * T(0.25));
				}
			}
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t roundX = rint(vIn.x);\n"
		   << "\t\treal_t roundY = rint(vIn.y);\n"
		   << "\t\treal_t offsetX = vIn.x - roundX;\n"
		   << "\t\treal_t offsetY = vIn.y - roundY;\n"
		   << "\n"
		   << "\t\tif (MwcNext01(mwc) >= (real_t)(0.75))\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * fma(offsetX, (real_t)(0.5), roundX);\n"
		   << "\t\t	vOut.y = " << weight << " * fma(offsetY, (real_t)(0.5), roundY);\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (fabs(offsetX) >= fabs(offsetY))\n"
		   << "\t\t	{\n"
		   << "\t\t		if (offsetX >= (real_t)(0.0))\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * fma(offsetX, (real_t)(0.5), roundX + (real_t)(0.25));\n"
		   << "\t\t			vOut.y = " << weight << " * (fma(offsetY, (real_t)(0.5), roundY) + (real_t)(0.25) * offsetY / offsetX);\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * fma(offsetX, (real_t)(0.5), roundX - (real_t)(0.25));\n"
		   << "\t\t			vOut.y = " << weight << " * (fma(offsetY, (real_t)(0.5), roundY) - (real_t)(0.25) * offsetY / offsetX);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (offsetY >= (real_t)(0.0))\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.y = " << weight << " * fma(offsetY, (real_t)(0.5), roundY + (real_t)(0.25));\n"
		   << "\t\t			vOut.x = " << weight << " * (fma(offsetX, (real_t)(0.5), roundX) + offsetX / offsetY * (real_t)(0.25));\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.y = " << weight << " * fma(offsetY, (real_t)(0.5), roundY - (real_t)(0.25));\n"
		   << "\t\t			vOut.x = " << weight << " * (fma(offsetX, (real_t)(0.5), roundX) - offsetX / offsetY * (real_t)(0.25));\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Butterfly.
/// </summary>
template <typename T>
class ButterflyVariation : public Variation<T>
{
public:
	ButterflyVariation(T weight = 1.0) : Variation<T>("butterfly", eVariationId::VAR_BUTTERFLY, weight) { }

	VARCOPY(ButterflyVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T wx = m_Weight * T(1.3029400317411197908970256609023);//This precision came from the original.
		T y2 = helper.In.y * 2;
		T r = wx * std::sqrt(std::abs(helper.In.y * helper.In.x) / Zeps(SQR(helper.In.x) + SQR(y2)));
		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * y2;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t wx = " << weight << " * (real_t)(1.3029400317411197908970256609023);\n"
		   << "\t\treal_t y2 = vIn.y * (real_t)(2.0);\n"
		   << "\t\treal_t r = wx * sqrt(fabs(vIn.y * vIn.x) / Zeps(fma(vIn.x, vIn.x, SQR(y2))));\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * y2;\n"
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
/// Cell.
/// </summary>
template <typename T>
class CellVariation : public ParametricVariation<T>
{
public:
	CellVariation(T weight = 1.0) : ParametricVariation<T>("cell", eVariationId::VAR_CELL, weight)
	{
		Init();
	}

	PARVARCOPY(CellVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T invCellSize = 1 / m_Size;
		T x = T(Floor(helper.In.x * invCellSize));//Calculate input cell. Note that int cast is omitted here. See below.
		T y = T(Floor(helper.In.y * invCellSize));
		T dx = helper.In.x - x * m_Size;//Offset from cell origin.
		T dy = helper.In.y - y * m_Size;

		//Interleave cells.
		if (y >= 0)
		{
			if (x >= 0)
			{
				y *= 2;
				x *= 2;
			}
			else
			{
				y *= 2;
				x = -(2 * x + 1);
			}
		}
		else
		{
			if (x >= 0)
			{
				y = -(2 * y + 1);
				x *= 2;
			}
			else
			{
				y = -(2 * y + 1);
				x = -(2 * x + 1);
			}
		}

		helper.Out.x = m_Weight * (dx + x * m_Size);
		helper.Out.y = -(m_Weight * (dy + y * m_Size));
		helper.Out.z = DefaultZ(helper);
	}

	/// <summary>
	/// Cell is very strange and will not run using integers.
	/// When using floats, it at least gives some output, however
	/// that output is slightly different than the CPU. But not by enough
	/// to change the shape of the final image.
	/// </summary>
	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string size = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t invCellSize = (real_t)(1.0) / " << size << ";\n"
		   //Float to int, orig.
		   //<< "\t\tint x = (int)floor(vIn.x * invCellSize);\n"
		   //<< "\t\tint y = (int)floor(vIn.y * invCellSize);\n"
		   //For some reason, OpenCL renders nothing if these are ints, so use floats.
		   //Note that Cuburn also omits the usage of ints.
		   << "\t\treal_t x = floor(vIn.x * invCellSize);\n"
		   << "\t\treal_t y = floor(vIn.y * invCellSize);\n"
		   << "\t\treal_t dx = vIn.x - x * " << size << ";\n"
		   << "\t\treal_t dy = vIn.y - y * " << size << ";\n"
		   << "\n"
		   << "\t\tif (y >= 0)\n"
		   << "\t\t{\n"
		   << "\t\t	if (x >= 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		y *= 2;\n"
		   << "\t\t		x *= 2;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		y *= 2;\n"
		   << "\t\t		x = -fma((real_t)(2.0), x, (real_t)(1.0));\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (x >= 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		y = -fma((real_t)(2.0), y, (real_t)(1.0));\n"
		   << "\t\t		x *= 2;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		y = -fma((real_t)(2.0), y, (real_t)(1.0));\n"
		   << "\t\t		x = -fma((real_t)(2.0), x, (real_t)(1.0));\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(x, " << size << ", dx);\n"
		   << "\t\tvOut.y = -(" << weight << " * fma(y, " << size << ", dy));\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Size = 2 * rand.Frand01<T>() + T(0.5);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Size, prefix + "cell_size", 1));
	}

private:
	T m_Size;
};

/// <summary>
/// Cpow.
/// </summary>
template <typename T>
class CpowVariation : public ParametricVariation<T>
{
public:
	CpowVariation(T weight = 1.0) : ParametricVariation<T>("cpow", eVariationId::VAR_CPOW, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(CpowVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanyx;
		T lnr = T(0.5) * std::log(helper.m_PrecalcSumSquares);
		T angle = m_C * a + m_D * lnr + m_Ang * rand.Rand(size_t(m_Power));
		T m = m_Weight * std::exp(m_C * lnr - m_D * a);
		helper.Out.x = m * std::cos(angle);
		helper.Out.y = m * std::sin(angle);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string powerR = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string powerI = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ang = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanyx;\n"
		   << "\t\treal_t lnr = (real_t)(0.5) * log(precalcSumSquares);\n"
		   << "\t\treal_t angle = fma(" << c << ", a, fma(" << d << ", lnr, " << ang << " * (real_t)MwcNextRange(mwc, (uint)" << power << ")));\n"
		   << "\t\treal_t m = " << weight << " * exp(fma(" << c << ", lnr, -(" << d << " * a)));\n"
		   << "\n"
		   << "\t\tvOut.x = m * cos(angle);\n"
		   << "\t\tvOut.y = m * sin(angle);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_C = m_PowerR / m_Power;
		m_D = m_PowerI / m_Power;
		m_Ang = 2 * T(M_PI) / m_Power;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_PowerR = 3 * rand.Frand01<T>();
		m_PowerI = rand.Frand01<T>() - T(0.5);
		m_Params[2].Set(5 * rand.Frand01<T>());//Power.
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_PowerR, prefix + "cpow_r", 1));
		m_Params.push_back(ParamWithName<T>(&m_PowerI, prefix + "cpow_i"));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "cpow_power", 1, eParamType::INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_C, prefix + "cpow_c"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_D, prefix + "cpow_d"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ang, prefix + "cpow_ang"));
	}

private:
	T m_PowerR;
	T m_PowerI;
	T m_Power;
	T m_C;//Precalc.
	T m_D;
	T m_Ang;
};

/// <summary>
/// Curve.
/// </summary>
template <typename T>
class CurveVariation : public ParametricVariation<T>
{
public:
	CurveVariation(T weight = 1.0) : ParametricVariation<T>("curve", eVariationId::VAR_CURVE, weight)
	{
		Init();
	}

	PARVARCOPY(CurveVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (helper.In.x + (m_XAmp * std::exp(-helper.In.y * helper.In.y * m_XLengthV)));
		helper.Out.y = m_Weight * (helper.In.y + (m_YAmp * std::exp(-helper.In.x * helper.In.x * m_YLengthV)));
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight   = WeightDefineString();
		string xAmp     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yAmp     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xLength  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yLength  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xLengthV = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yLengthV = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * fma(" << xAmp << ", exp(-vIn.y * vIn.y * " << xLengthV << "), vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * fma(" << yAmp << ", exp(-vIn.x * vIn.x * " << yLengthV << "), vIn.y);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		if (Compat::m_Compat)
		{
			m_XLengthV = 1 / std::max(SQR(m_XLength), T(1e-20));
			m_YLengthV = 1 / std::max(SQR(m_YLength), T(1e-20));
		}
		else
		{
			m_XLengthV = 1 / Zeps(m_XLength);
			m_YLengthV = 1 / Zeps(m_YLength);
		}
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_XAmp = 5 * (rand.Frand01<T>() - T(0.5));
		m_YAmp = 4 * (rand.Frand01<T>() - T(0.5));
		m_XLength = 2 * (rand.Frand01<T>() + T(0.5));
		m_YLength = 2 * (rand.Frand01<T>() + T(0.5));
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_XAmp, prefix + "curve_xamp"));
		m_Params.push_back(ParamWithName<T>(&m_YAmp, prefix + "curve_yamp"));
		m_Params.push_back(ParamWithName<T>(&m_XLength, prefix + "curve_xlength", 1));
		m_Params.push_back(ParamWithName<T>(&m_YLength, prefix + "curve_ylength", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_XLengthV, prefix + "curve_xlenv"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_YLengthV, prefix + "curve_ylenv"));
	}

private:
	T m_XAmp;
	T m_YAmp;
	T m_XLength;
	T m_YLength;
	T m_XLengthV;//Precalc.
	T m_YLengthV;
};

/// <summary>
/// Edisc.
/// </summary>
template <typename T>
class EdiscVariation : public Variation<T>
{
public:
	EdiscVariation(T weight = 1.0) : Variation<T>("edisc", eVariationId::VAR_EDISC, weight, true) { }

	VARCOPY(EdiscVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T tmp = helper.m_PrecalcSumSquares + 1;
		T tmp2 = 2 * helper.In.x;
		T r1 = std::sqrt(tmp + tmp2);
		T r2 = std::sqrt(tmp - tmp2);
		T xmax = Zeps((r1 + r2) * T(0.5));
		T logs = std::log(xmax + std::sqrt(xmax - 1));
		T a2;

		if (Compat::m_Compat)
			a2 = -std::acos(Clamp<T>(helper.In.x / xmax, -1, 1));
		else
			a2 = std::acos(helper.In.x / xmax);

		T w = m_Weight / T(11.57034632);//2 / e^pi.
		T snv, csv, snhu, cshu;
		sincos(logs, &snv, &csv);
		snhu = std::sinh(a2);
		cshu = std::cosh(a2);

		if (helper.In.y > 0.0)
			snv = -snv;

		helper.Out.x = w * cshu * csv;
		helper.Out.y = w * snhu * snv;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t tmp = precalcSumSquares + (real_t)(1.0);\n"
		   << "\t\treal_t tmp2 = (real_t)(2.0) * vIn.x;\n"
		   << "\t\treal_t r1 = sqrt(tmp + tmp2);\n"
		   << "\t\treal_t r2 = sqrt(tmp - tmp2);\n"
		   << "\t\treal_t xmax = Zeps((r1 + r2) * (real_t)(0.5));\n"
		   << "\t\treal_t logs = log(xmax + sqrt(xmax - (real_t)(1.0)));\n"
		   << "\t\treal_t a2;\n";

		if (Compat::m_Compat)
			ss << "\t\ta2 = -acos(clamp(vIn.x / xmax, -(real_t)(1.0), (real_t)(1.0)));\n";
		else
			ss << "\t\ta2 = acos(vIn.x / xmax);\n";

		ss << "\t\treal_t w = " << weight << " / (real_t)(11.57034632);\n"
		   << "\t\treal_t snv = sin(logs);\n"
		   << "\t\treal_t csv = cos(logs);\n"
		   << "\t\treal_t snhu = sinh(a2);\n"
		   << "\t\treal_t cshu = cosh(a2);\n"
		   << "\t\tif (vIn.y > 0)\n"
		   << "\t\t	snv = -snv;\n"
		   << "\t\tvOut.x = w * cshu * csv;\n"
		   << "\t\tvOut.y = w * snhu * snv;\n"
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
/// Elliptic.
/// </summary>
template <typename T>
class EllipticVariation : public ParametricVariation<T>
{
public:
	EllipticVariation(T weight = 1.0) : ParametricVariation<T>("elliptic", eVariationId::VAR_ELLIPTIC, weight, true)
	{
		Init();
	}

	PARVARCOPY(EllipticVariation)

	//An improved version which was posted in the Discord chat by user Claude which helps with rounding errors.
	//Note that for this to work, a "bad value" had to be changed from 1e10 and -1e10 to 1e50 and -1e50.
	//For this to be correct, it must always use double. So there is no point in switching precisions when using this variation.
	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (typeid(T) == typeid(float))
		{
			T tmp = helper.m_PrecalcSumSquares + 1;
			T x2 = 2 * helper.In.x;
			T xmax = T(0.5) * (std::sqrt(tmp + x2) + std::sqrt(tmp - x2));
			T a = helper.In.x / xmax;
			T b = 1 - a * a;
			T ssx = xmax - 1;
			const T w = m_WeightDivPiDiv2;

			if (b < 0)
				b = 0;
			else
				b = std::sqrt(b);

			if (ssx < 0)
				ssx = 0;
			else
				ssx = std::sqrt(ssx);

			helper.Out.x = w * std::atan2(a, b);

			if (helper.In.y > 0)
				helper.Out.y = w * std::log(xmax + ssx);
			else
				helper.Out.y = -(w * std::log(xmax + ssx));

			helper.Out.z = DefaultZ(helper);
		}
		else
		{
			double x2 = 2 * helper.In.x;
			double u = helper.m_PrecalcSumSquares + x2;
			double v = helper.m_PrecalcSumSquares - x2;
			double xmaxm1 = 0.5 * (Sqrt1pm1(u) + Sqrt1pm1(v));
			double a = helper.In.x / (1 + xmaxm1);
			double ssx = xmaxm1 < 0 ? 0 : std::sqrt(xmaxm1);
			helper.Out.x = T(m_WeightDivPiDiv2 * std::asin(Clamp(a, -1.0, 1.0)));

			if (helper.In.y > 0)
				helper.Out.y = T(m_WeightDivPiDiv2 * std::log1p(xmaxm1 + ssx));
			else
				helper.Out.y = T(-(m_WeightDivPiDiv2 * std::log1p(xmaxm1 + ssx)));

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
		string weightDivPiDiv2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		if (typeid(T) == typeid(float))
		{
			ss << "\t{\n"
			   << "\t\treal_t tmp = precalcSumSquares + (real_t)(1.0);\n"
			   << "\t\treal_t x2 = (real_t)(2.0) * vIn.x;\n"
			   << "\t\treal_t xmax = (real_t)(0.5) * (sqrt(tmp + x2) + sqrt(tmp - x2));\n"
			   << "\t\treal_t a = vIn.x / xmax;\n"
			   << "\t\treal_t b = (real_t)(1.0) - a * a;\n"
			   << "\t\treal_t ssx = xmax - (real_t)(1.0);\n"
			   << "\t\tconst real_t w = " << weightDivPiDiv2 << ";\n"
			   << "\n"
			   << "\t\tif (b < 0)\n"
			   << "\t\t	b = 0;\n"
			   << "\t\telse\n"
			   << "\t\t	b = sqrt(b);\n"
			   << "\n"
			   << "\t\tif (ssx < 0)\n"
			   << "\t\t	ssx = 0;\n"
			   << "\t\telse\n"
			   << "\t\t	ssx = sqrt(ssx);\n"
			   << "\n"
			   << "\t\tvOut.x = w * atan2(a, b);\n"
			   << "\n"
			   << "\t\tif (vIn.y > 0)\n"
			   << "\t\t	vOut.y = w * log(xmax + ssx);\n"
			   << "\t\telse\n"
			   << "\t\t	vOut.y = -(w * log(xmax + ssx));\n"
			   << "\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
		}
		else
		{
			ss << "\t{\n"
			   << "\t\tdouble x2 = 2.0 * vIn.x;\n"
			   << "\t\tdouble u = precalcSumSquares + x2;\n"
			   << "\t\tdouble v = precalcSumSquares - x2;\n"
			   << "\t\tdouble xmaxm1 = 0.5 * (Sqrt1pm1(u) + Sqrt1pm1(v));\n"
			   << "\t\tdouble a = vIn.x / (1 + xmaxm1);\n"
			   << "\t\tdouble ssx = xmaxm1 < 0 ? 0.0 : sqrt(xmaxm1);\n"
			   << "\t\tvOut.x = (" << weightDivPiDiv2 << " * asin(clamp(a, (double)-1.0, (double)1.0)));\n"
			   << "\n"
			   << "\t\tif (vIn.y > 0)\n"
			   << "\t\t\tvOut.y = " << weightDivPiDiv2 << " * log1p(xmaxm1 + ssx);\n"
			   << "\t\telse\n"
			   << "\t\t\tvOut.y = -(" << weightDivPiDiv2 << " * log1p(xmaxm1 + ssx));\n"
			   << "\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
		}

		return ss.str();
	}

	virtual string OpenCLFuncsString() const override
	{
		return
			"double Sqrt1pm1(double x)\n"
			"{\n"
			"	if (-0.0625 < x && x < 0.0625)\n"
			"	{\n"
			"		double num = 0;\n"
			"		double den = 0;\n"
			"		num += 1.0 / 32.0;\n"
			"		den += 1.0 / 256.0;\n"
			"		num *= x;\n"
			"		den *= x;\n"
			"		num += 5.0 / 16.0;\n"
			"		den += 5.0 / 32.0;\n"
			"		num *= x;\n"
			"		den *= x;\n"
			"		num += 3.0 / 4.0;\n"
			"		den += 15.0 / 16.0;\n"
			"		num *= x;\n"
			"		den *= x;\n"
			"		num += 1.0 / 2.0;\n"
			"		den += 7.0 / 4.0;\n"
			"		num *= x;\n"
			"		den *= x;\n"
			"		den += 1;\n"
			"		return num / den;\n"
			"	}\n"
			"\n"
			"	return sqrt(1 + x) - 1;\n"
			"}\n\n";
	}

	virtual void Precalc() override
	{
		m_WeightDivPiDiv2 = m_Weight / T(M_PI_2);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_WeightDivPiDiv2, prefix + "elliptic_weight_div_pi_div_2"));//Precalc.
	}

private:
	//An improved version which was posted in the Discord chat by user Claude which was helps with rounding errors.
	//Note that for this to work, a "bad value" had to be changed from 1e10 and -1e10 to 1e50 and -1e50.
	//For this to be correct, it must always use double. So there is no point in switching precisions when using this variation.
	double Sqrt1pm1(double x)
	{
		if (-0.0625 < x && x < 0.0625)
		{
			double num = 0;
			double den = 0;
			num += 1.0 / 32;
			den += 1.0 / 256;
			num *= x;
			den *= x;
			num += 5.0 / 16;
			den += 5.0 / 32;
			num *= x;
			den *= x;
			num += 3.0 / 4;
			den += 15.0 / 16;
			num *= x;
			den *= x;
			num += 1.0 / 2;
			den += 7.0 / 4;
			num *= x;
			den *= x;
			den += 1;
			return num / den;
		}

		return std::sqrt(1 + x) - 1;
	}

	T m_WeightDivPiDiv2;//Precalc.
};

/// <summary>
/// Escher.
/// </summary>
template <typename T>
class EscherVariation : public ParametricVariation<T>
{
public:
	using Variation<T>::m_NeedPrecalcAtanXY;
	using Variation<T>::m_NeedPrecalcAtanYX;
	EscherVariation(T weight = 1.0) : ParametricVariation<T>("escher", eVariationId::VAR_ESCHER, weight, true, false, false, true, true)
	{
		Init();

		if (Compat::m_Compat)
		{
			m_NeedPrecalcAtanXY = false;
			m_NeedPrecalcAtanYX = true;
		}
		else
		{
			m_NeedPrecalcAtanXY = true;
			m_NeedPrecalcAtanYX = false;
		}
	}

	PARVARCOPY(EscherVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = Compat::m_Compat ? helper.m_PrecalcAtanyx : helper.m_PrecalcAtanxy;
		T lnr = T(0.5) * std::log(helper.m_PrecalcSumSquares);
		T m = m_Weight * std::exp(m_C * lnr - m_D * a);
		T n = m_C * a + m_D * lnr;
		helper.Out.x = m * std::cos(n);
		helper.Out.y = m * std::sin(n);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string beta = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a = " << (Compat::m_Compat ? "precalcAtanyx" : "precalcAtanxy") << ";\n"
		   << "\t\treal_t lnr = (real_t)(0.5) * log(precalcSumSquares);\n"
		   << "\t\treal_t m = " << weight << " * exp(fma(" << c << ", lnr, -(" << d << " * a)));\n"
		   << "\t\treal_t n = fma(" << c << ", a, " << d << " * lnr);\n"
		   << "\n"
		   << "\t\tvOut.x = m * cos(n);\n"
		   << "\t\tvOut.y = m * sin(n);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		sincos(m_Beta, &m_D, &m_C);
		m_C = T(0.5) * (1 + m_C);
		m_D = T(0.5) * m_D;

		if (Compat::m_Compat)
		{
			m_NeedPrecalcAtanXY = false;
			m_NeedPrecalcAtanYX = true;
		}
		else
		{
			m_NeedPrecalcAtanXY = true;
			m_NeedPrecalcAtanYX = false;
		}
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Beta = T(M_PI) * rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Beta, prefix + "escher_beta", 3));
		m_Params.push_back(ParamWithName<T>(true, &m_C, prefix + "escher_beta_c"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_D, prefix + "escher_beta_d"));
	}

private:
	T m_Beta;
	T m_C;//Precalc.
	T m_D;
};

/// <summary>
/// Foci.
/// </summary>
template <typename T>
class FociVariation : public Variation<T>
{
public:
	FociVariation(T weight = 1.0) : Variation<T>("foci", eVariationId::VAR_FOCI, weight) { }

	VARCOPY(FociVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T expx = std::exp(helper.In.x) * T(0.5);
		T expnx = T(0.25) / Zeps(expx);
		T sn, cn, tmp;
		sincos(helper.In.y, &sn, &cn);
		tmp = m_Weight / Zeps(expx + expnx - cn);
		helper.Out.x = tmp * (expx - expnx);
		helper.Out.y = tmp * sn;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t expx = exp(vIn.x) * (real_t)(0.5);\n"
		   << "\t\treal_t expnx = (real_t)(0.25) / Zeps(expx);\n"
		   << "\t\treal_t sn = sin(vIn.y);\n"
		   << "\t\treal_t cn = cos(vIn.y);\n"
		   << "\t\treal_t tmp = Zeps(expx + expnx - cn);\n"
		   << "\n"
		   << "\t\ttmp = " << weight << " / tmp;\n"
		   << "\n"
		   << "\t\tvOut.x = tmp * (expx - expnx);\n"
		   << "\t\tvOut.y = tmp * sn;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// LazySusan.
/// </summary>
template <typename T>
class LazySusanVariation : public ParametricVariation<T>
{
public:
	LazySusanVariation(T weight = 1.0) : ParametricVariation<T>("lazysusan", eVariationId::VAR_LAZYSUSAN, weight)
	{
		Init();
	}

	PARVARCOPY(LazySusanVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = helper.In.x - m_X;
		T y = helper.In.y + m_Y;
		T r = std::sqrt(x * x + y * y);

		if (r < m_Weight)
		{
			T a = std::atan2(y, x) + m_Spin + m_Twist * (m_Weight - r);
			helper.Out.x = m_Weight * (r * std::cos(a) + m_X);//Fix to make it colapse to 0 when weight is 0.//SMOULDER
			helper.Out.y = m_Weight * (r * std::sin(a) - m_Y);
		}
		else
		{
			r = 1 + m_Space / Zeps(r);
			helper.Out.x = m_Weight * (r * x + m_X);//Fix to make it colapse to 0 when weight is 0.//SMOULDER
			helper.Out.y = m_Weight * (r * y - m_Y);
		}

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string spin  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string space = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string twist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x = vIn.x - " << x << ";\n"
		   << "\t\treal_t y = vIn.y + " << y << ";\n"
		   << "\t\treal_t r = sqrt(fma(x, x, SQR(y)));\n"
		   << "\n"
		   << "\t\tif (r < " << weight << ")\n"
		   << "\t\t{\n"
		   << "\t\t	real_t a = fma(" << twist << ", " << weight << " - r, atan2(y, x) + " << spin << ");\n"
		   << "\n"
		   << "\t\t	vOut.x = " << weight << " * fma(r, cos(a), " << x << ");\n"
		   << "\t\t	vOut.y = " << weight << " * fma(r, sin(a), -" << y << ");\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	r = (real_t)(1.0) + " << space << " / Zeps(r);\n"
		   << "\n"
		   << "\t\t	vOut.x = " << weight << " * fma(r, x, " << x << ");\n"
		   << "\t\t	vOut.y = " << weight << " * fma(r, y, -" << y << ");\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = 2 * rand.Frand11<T>();
		m_Y = 2 * rand.Frand11<T>();
		m_Spin = T(M_PI) * rand.Frand11<T>();
		m_Space = 2 * rand.Frand11<T>();
		m_Twist = 2 * rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Spin, prefix + "lazysusan_spin", T(M_PI)));
		m_Params.push_back(ParamWithName<T>(&m_Space, prefix + "lazysusan_space"));
		m_Params.push_back(ParamWithName<T>(&m_Twist, prefix + "lazysusan_twist"));
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "lazysusan_x"));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "lazysusan_y"));
	}

private:
	T m_Spin;
	T m_Space;
	T m_Twist;
	T m_X;
	T m_Y;
};

/// <summary>
/// Loonie.
/// </summary>
template <typename T>
class LoonieVariation : public ParametricVariation<T>
{
public:
	LoonieVariation(T weight = 1.0) : ParametricVariation<T>("loonie", eVariationId::VAR_LOONIE, weight, true)
	{
		Init();
	}

	PARVARCOPY(LoonieVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.m_PrecalcSumSquares < m_W2 && helper.m_PrecalcSumSquares != 0)
		{
			T r = m_Weight * std::sqrt((m_W2 / helper.m_PrecalcSumSquares) - 1);
			helper.Out.x = r * helper.In.x;
			helper.Out.y = r * helper.In.y;
		}
		else
		{
			helper.Out.x = m_Weight * helper.In.x;
			helper.Out.y = m_Weight * helper.In.y;
		}

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string w2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tif (precalcSumSquares < " << w2 << " && precalcSumSquares != 0)\n"
		   << "\t\t{\n"
		   << "\t\t	real_t r = " << weight << " * sqrt((" << w2 << " / precalcSumSquares) - (real_t)(1.0));\n"
		   << "\t\t	vOut.x = r * vIn.x;\n"
		   << "\t\t	vOut.y = r * vIn.y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t	vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_W2 = SQR(m_Weight);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_W2, prefix + "loonie_w2"));//Precalc.
	}

private:
	T m_W2;//Precalc.
};

/// <summary>
/// Modulus.
/// </summary>
template <typename T>
class ModulusVariation : public ParametricVariation<T>
{
public:
	ModulusVariation(T weight = 1.0) : ParametricVariation<T>("modulus", eVariationId::VAR_MODULUS, weight)
	{
		Init();
	}

	PARVARCOPY(ModulusVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.In.x > m_X)
			helper.Out.x = m_Weight * (-m_X + fmod(helper.In.x + m_X, m_XRange));
		else if (helper.In.x < -m_X)
			helper.Out.x = m_Weight * (m_X - fmod(m_X - helper.In.x, m_XRange));
		else
			helper.Out.x = m_Weight * helper.In.x;

		if (helper.In.y > m_Y)
			helper.Out.y = m_Weight * (-m_Y + fmod(helper.In.y + m_Y, m_YRange));
		else if (helper.In.y < -m_Y)
			helper.Out.y = m_Weight * (m_Y - fmod(m_Y - helper.In.y, m_YRange));
		else
			helper.Out.y = m_Weight * helper.In.y;

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
		string xr = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yr = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tif (vIn.x > " << x << ")\n"
		   << "\t\t	vOut.x = " << weight << " * (-" << x << " + fmod(vIn.x + " << x << ", " << xr << "));\n"
		   << "\t\telse if (vIn.x < -" << x << ")\n"
		   << "\t\t	vOut.x = " << weight << " * ( " << x << " - fmod(" << x << " - vIn.x, " << xr << "));\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = " << weight << " * vIn.x;\n"
		   << "\n"
		   << "\t\tif (vIn.y > " << y << ")\n"
		   << "\t\t	vOut.y = " << weight << " * (-" << y << " + fmod(vIn.y + " << y << ", " << yr << "));\n"
		   << "\t\telse if (vIn.y < -" << y << ")\n"
		   << "\t\t	vOut.y = " << weight << " * ( " << y << " - fmod(" << y << " - vIn.y, " << yr << "));\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = " << weight << " * vIn.y;\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_XRange = 2 * m_X;
		m_YRange = 2 * m_Y;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = rand.Frand11<T>();
		m_Y = rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "modulus_x", 1));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "modulus_y", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_XRange, prefix + "modulus_xrange"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_YRange, prefix + "modulus_yrange"));
	}

private:
	T m_X;
	T m_Y;
	T m_XRange;//Precalc.
	T m_YRange;
};

/// <summary>
/// Oscilloscope.
/// </summary>
template <typename T>
class OscilloscopeVariation : public ParametricVariation<T>
{
public:
	OscilloscopeVariation(T weight = 1.0) : ParametricVariation<T>("oscilloscope", eVariationId::VAR_OSCILLOSCOPE, weight)
	{
		Init();
	}

	PARVARCOPY(OscilloscopeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t;

		if (m_Damping == 0.0)
			t = m_Amplitude * std::cos(m_2PiFreq * helper.In.x) + m_Separation;
		else
			t = m_Amplitude * std::exp(-std::abs(helper.In.x) * m_Damping) * std::cos(m_2PiFreq * helper.In.x) + m_Separation;

		if (std::abs(helper.In.y) <= t)
		{
			helper.Out.x = m_Weight * helper.In.x;
			helper.Out.y = -(m_Weight * helper.In.y);
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
		string separation = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string frequency  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amplitude  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string damping    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string tpf        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t t;\n"
		   << "\n"
		   << "\t\tif (" << damping << " == (real_t)(0.0))\n"
		   << "\t\t	t = fma(" << amplitude << ", cos(" << tpf << " * vIn.x), " << separation << ");\n"
		   << "\t\telse\n"
		   << "\t\t	t = fma(" << amplitude << ", exp(-fabs(vIn.x) * " << damping << ") * cos(" << tpf << " * vIn.x), " << separation << ");\n"
		   << "\n"
		   << "\t\tif (fabs(vIn.y) <= t)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t	vOut.y = -(" << weight << " * vIn.y);\n"
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

	virtual void Precalc() override
	{
		m_2PiFreq = m_Frequency * T(M_2PI);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Separation = 1 + rand.Frand11<T>();
		m_Frequency = T(M_PI) * rand.Frand11<T>();
		m_Amplitude = 1 + 2 * rand.Frand01<T>();
		m_Damping = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Separation,    prefix + "oscilloscope_separation", 1));
		m_Params.push_back(ParamWithName<T>(&m_Frequency,     prefix + "oscilloscope_frequency", T(M_PI)));
		m_Params.push_back(ParamWithName<T>(&m_Amplitude,     prefix + "oscilloscope_amplitude", 1));
		m_Params.push_back(ParamWithName<T>(&m_Damping,       prefix + "oscilloscope_damping"));
		m_Params.push_back(ParamWithName<T>(true, &m_2PiFreq, prefix + "oscilloscope_2pifreq"));//Precalc.
	}

private:
	T m_Separation;
	T m_Frequency;
	T m_Amplitude;
	T m_Damping;
	T m_2PiFreq;//Precalc.
};

/// <summary>
/// Oscilloscope2.
/// By dark-beam.
/// </summary>
template <typename T>
class Oscilloscope2Variation : public ParametricVariation<T>
{
public:
	Oscilloscope2Variation(T weight = 1.0) : ParametricVariation<T>("oscilloscope2", eVariationId::VAR_OSCILLOSCOPE2, weight)
	{
		Init();
	}

	PARVARCOPY(Oscilloscope2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t;
		T pt = m_Perturbation * std::sin(m_Tpf2 * helper.In.y);

		if (!m_Damping)
			t = m_Amplitude * std::cos(m_Tpf * helper.In.x + pt) + m_Separation;
		else
			t = m_Amplitude * std::exp(-std::abs(helper.In.x) * m_Damping) * std::cos(m_Tpf * helper.In.x + pt) + m_Separation;

		if (std::abs(helper.In.y) <= t)
		{
			helper.Out.x = -(m_Weight * helper.In.x);
			helper.Out.y = -(m_Weight * helper.In.y);
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
		string separation   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string frequencyx   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string frequencyy   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amplitude    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string perturbation = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string damping      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string tpf          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string tpf2         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t t;\n"
		   << "\t\treal_t pt = " << perturbation << " * sin(" << tpf2 << " * vIn.y);\n"
		   << "\n"
		   << "\t\tif (" << damping << " == 0)\n"
		   << "\t\t	t = fma(" << amplitude << ", cos(fma(" << tpf << ", vIn.x, pt)), " << separation << ");\n"
		   << "\t\telse\n"
		   << "\t\t	t = fma(" << amplitude << ", exp(-fabs(vIn.x) * " << damping << ") * cos(fma(" << tpf << ", vIn.x, pt)), " << separation << ");\n"
		   << "\n"
		   << "\t\tif (fabs(vIn.y) <= t)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = -(" << weight << " * vIn.x);\n"
		   << "\t\t	vOut.y = -(" << weight << " * vIn.y);\n"
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

	virtual void Precalc() override
	{
		m_Tpf = M_2PI * m_FrequencyX;
		m_Tpf2 = M_2PI * m_FrequencyY;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Separation,   prefix + "oscilloscope2_separation", 1, eParamType::REAL, 0));
		m_Params.push_back(ParamWithName<T>(&m_FrequencyX,   prefix + "oscilloscope2_frequencyx", T(M_PI)));
		m_Params.push_back(ParamWithName<T>(&m_FrequencyY,   prefix + "oscilloscope2_frequencyy", T(M_PI)));
		m_Params.push_back(ParamWithName<T>(&m_Amplitude,    prefix + "oscilloscope2_amplitude", 1));
		m_Params.push_back(ParamWithName<T>(&m_Perturbation, prefix + "oscilloscope2_perturbation", 1));
		m_Params.push_back(ParamWithName<T>(&m_Damping,      prefix + "oscilloscope2_damping", 0));
		m_Params.push_back(ParamWithName<T>(true, &m_Tpf,    prefix + "oscilloscope2_tpf"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Tpf2,   prefix + "oscilloscope2_tpf2"));
	}

private:
	T m_Separation;
	T m_FrequencyX;
	T m_FrequencyY;
	T m_Amplitude;
	T m_Perturbation;
	T m_Damping;
	T m_Tpf;//Precalc.
	T m_Tpf2;
};

/// <summary>
/// Polar2.
/// </summary>
template <typename T>
class Polar2Variation : public ParametricVariation<T>
{
public:
	Polar2Variation(T weight = 1.0) : ParametricVariation<T>("polar2", eVariationId::VAR_POLAR2, weight, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Polar2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Vvar * helper.m_PrecalcAtanxy;
		helper.Out.y = m_Vvar2 * std::log(helper.m_PrecalcSumSquares);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string vvar = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vvar2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = " << vvar << " * precalcAtanxy;\n"
		   << "\t\tvOut.y = " << vvar2 << " * log(precalcSumSquares);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Vvar = m_Weight / T(M_PI);
		m_Vvar2 = m_Vvar * T(0.5);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_Vvar, prefix + "polar2_vvar"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Vvar2, prefix + "polar2_vvar2"));
	}

private:
	T m_Vvar;
	T m_Vvar2;
};

/// <summary>
/// Popcorn2.
/// </summary>
template <typename T>
class Popcorn2Variation : public ParametricVariation<T>
{
public:
	Popcorn2Variation(T weight = 1.0) : ParametricVariation<T>("popcorn2", eVariationId::VAR_POPCORN2, weight)
	{
		Init();
	}

	PARVARCOPY(Popcorn2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (helper.In.x + m_X * std::sin(SafeTan<T>(helper.In.y * m_C)));
		helper.Out.y = m_Weight * (helper.In.y + m_Y * std::sin(SafeTan<T>(helper.In.x * m_C)));
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
		string c = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * fma(" << x << ", sin(tan(vIn.y * " << c << ")), vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * fma(" << y << ", sin(tan(vIn.x * " << c << ")), vIn.y);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = T(0.2) + rand.Frand01<T>();
		m_Y = T(0.2) * rand.Frand01<T>();
		m_C = 5 * rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "popcorn2_x", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "popcorn2_y", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_C, prefix + "popcorn2_c", 3));
	}

private:
	T m_X;
	T m_Y;
	T m_C;
};

/// <summary>
/// Scry.
/// Note that scry does not multiply by weight, but as the
/// values still approach 0 as the weight approaches 0, it
/// should be ok.
/// </summary>
template <typename T>
class ScryVariation : public ParametricVariation<T>
{
public:
	ScryVariation(T weight = 1.0) : ParametricVariation<T>("scry", eVariationId::VAR_SCRY, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(ScryVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t = helper.m_PrecalcSumSquares;
		T r = 1 / Zeps(helper.m_PrecalcSqrtSumSquares * (t + m_InvWeight));
		helper.Out.x = helper.In.x * r;
		helper.Out.y = helper.In.y * r;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string invWeight = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t t = precalcSumSquares;\n"
		   << "\t\treal_t r = (real_t)(1.0) / Zeps(precalcSqrtSumSquares * (t + " << invWeight << "));\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * r;\n"
		   << "\t\tvOut.y = vIn.y * r;\n"
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
		m_InvWeight = 1 / Zeps(m_Weight);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_InvWeight, prefix + "scry_inv_weight"));//Precalcs only, no params.
	}

private:
	T m_InvWeight;//Precalcs only, no params.
};

/// <summary>
/// scry2.
/// By dark-beam, modified by tatasz to increase the speed.
/// </summary>
template <typename T>
class Scry2Variation : public ParametricVariation<T>
{
public:
	Scry2Variation(T weight = 1.0) : ParametricVariation<T>("scry2", eVariationId::VAR_SCRY2, weight, true)
	{
		Init();
	}

	PARVARCOPY(Scry2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = helper.m_PrecalcSumSquares;
		T s = 1 / Zeps(std::sqrt(r2) * (r2 + 1));
		T newX = helper.In.x * s;
		T newY = helper.In.y * s;
		T dang = (std::atan2(newY, newX) + T(M_PI)) / m_2PiOverPower;
		T rad = std::sqrt(SQR(newX) + SQR(newY));
		T zang1 = T(Floor<T>(dang));
		T xang1 = dang - zang1;
		T xang2 = xang1 > 0.5 ? 1 - xang1 : xang1;
		T zang = xang1 > 0.5 ? zang1 + 1 : zang1;
		T sign = T(xang1 > 0.5 ? -1 : 1);
		T xang = std::atan(xang2 * std::tan(m_2PiOverPower * T(0.5)) * 2) / m_2PiOverPower;
		T coeff = 1 / std::cos(xang * m_2PiOverPower);
		T ang = (zang + sign * xang) * m_2PiOverPower - T(M_PI);
		helper.Out.x = m_Weight * coeff * rad * std::cos(ang);
		helper.Out.y = m_Weight * coeff * rad * std::sin(ang);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string twopioverpower = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r2 = precalcSumSquares;\n"
		   << "\t\treal_t s = 1 / Zeps(sqrt(r2) * (r2 + 1));\n"
		   << "\t\treal_t newX = vIn.x * s;\n"
		   << "\t\treal_t newY = vIn.y * s;\n"
		   << "\t\treal_t dang = (atan2(newY, newX) + MPI) / " << twopioverpower << ";\n"
		   << "\t\treal_t rad = sqrt(SQR(newX) + SQR(newY));\n"
		   << "\t\treal_t zang1 = floor(dang);\n"
		   << "\t\treal_t xang1 = dang - zang1;\n"
		   << "\t\treal_t xang2 = xang1 > 0.5 ? 1 - xang1 : xang1;\n"
		   << "\t\treal_t zang = xang1 > 0.5 ? zang1 + 1 : zang1;\n"
		   << "\t\treal_t sign = xang1 > 0.5 ? -1.0 : 1.0;\n"
		   << "\t\treal_t xang = atan(xang2 * tan(" << twopioverpower << " * 0.5) * 2) / " << twopioverpower << ";\n"
		   << "\t\treal_t coeff = 1 / cos(xang * " << twopioverpower << ");\n"
		   << "\t\treal_t ang = (zang + sign * xang) * " << twopioverpower << " - MPI;\n"
		   << "\t\tvOut.x = " << weight << " * coeff * rad * cos(ang);\n"
		   << "\t\tvOut.y = " << weight << " * coeff * rad * sin(ang);\n"
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
		m_2PiOverPower = M_2PI / Zeps(m_Power);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "scry2_power", 4));
		m_Params.push_back(ParamWithName<T>(true, &m_2PiOverPower, prefix + "scry2_2pi_over_power"));//Precalc.
	}

private:
	T m_Power;
	T m_2PiOverPower;//Precalc.
};

/// <summary>
/// Separation.
/// </summary>
template <typename T>
class SeparationVariation : public ParametricVariation<T>
{
public:
	SeparationVariation(T weight = 1.0) : ParametricVariation<T>("separation", eVariationId::VAR_SEPARATION, weight)
	{
		Init();
	}

	PARVARCOPY(SeparationVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.In.x > 0.0)
			helper.Out.x = m_Weight * (std::sqrt(SQR(helper.In.x) + m_XX) - helper.In.x * m_XInside);
		else
			helper.Out.x = -(m_Weight * (std::sqrt(SQR(helper.In.x) + m_XX) + helper.In.x * m_XInside));

		if (helper.In.y > 0.0)
			helper.Out.y = m_Weight * (std::sqrt(SQR(helper.In.y) + m_YY) - helper.In.y * m_YInside);
		else
			helper.Out.y = -(m_Weight * (std::sqrt(SQR(helper.In.y) + m_YY) + helper.In.y * m_YInside));

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string x       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xInside = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yInside = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xx      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yy      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tif (vIn.x > (real_t)(0.0))\n"
		   << "\t\t	vOut.x = " << weight << " * (sqrt(fma(vIn.x, vIn.x, " << xx << ")) - vIn.x * " << xInside << ");\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = -(" << weight << " * fma(vIn.x, " << xInside << ", sqrt(fma(vIn.x, vIn.x, " << xx << "))));\n"
		   << "\n"
		   << "\t\tif (vIn.y > (real_t)(0.0))\n"
		   << "\t\t	vOut.y = " << weight << " * (sqrt(fma(vIn.y, vIn.y, " << yy << ")) - vIn.y * " << yInside << ");\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = -(" << weight << " * fma(vIn.y, " << yInside << ", sqrt(fma(vIn.y, vIn.y, " << yy << "))));\n"
		   << "\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_XX = SQR(m_X);
		m_YY = SQR(m_Y);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = 1 + rand.Frand11<T>();
		m_XInside = 1 + rand.Frand11<T>();
		m_Y = rand.Frand11<T>();
		m_YInside = rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "separation_x", 1));
		m_Params.push_back(ParamWithName<T>(&m_XInside, prefix + "separation_xinside"));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "separation_y", 1));
		m_Params.push_back(ParamWithName<T>(&m_YInside, prefix + "separation_yinside"));
		m_Params.push_back(ParamWithName<T>(true, &m_XX, prefix + "separation_xx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_YY, prefix + "separation_yy"));
	}

private:
	T m_X;
	T m_XInside;
	T m_Y;
	T m_YInside;
	T m_XX;//Precalc.
	T m_YY;
};

/// <summary>
/// Split.
/// </summary>
template <typename T>
class SplitVariation : public ParametricVariation<T>
{
public:
	SplitVariation(T weight = 1.0) : ParametricVariation<T>("split", eVariationId::VAR_SPLIT, weight)
	{
		Init();
	}

	PARVARCOPY(SplitVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (std::cos(helper.In.y * m_YAng) >= 0)
			helper.Out.x = m_Weight * helper.In.x;
		else
			helper.Out.x = -(m_Weight * helper.In.x);

		if (std::cos(helper.In.x * m_XAng) >= 0)
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
		string xSize = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ySize = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string xAng = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yAng = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tif (cos(vIn.y * " << yAng << ") >= 0)\n"
		   << "\t\t	vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = -(" << weight << " * vIn.x);\n"
		   << "\n"
		   << "\t\tif (cos(vIn.x * " << xAng << ") >= 0)\n"
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
		m_XAng = T(M_PI) * m_XSize;
		m_YAng = T(M_PI) * m_YSize;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_XSize = rand.Frand11<T>();
		m_YSize = rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_XSize, prefix + "split_xsize", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_YSize, prefix + "split_ysize", T(0.5)));
		m_Params.push_back(ParamWithName<T>(true, &m_XAng, prefix + "split_xang"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_YAng, prefix + "split_yang"));
	}

private:
	T m_XSize;
	T m_YSize;
	T m_XAng;//Precalc.
	T m_YAng;
};

/// <summary>
/// Splits.
/// </summary>
template <typename T>
class SplitsVariation : public ParametricVariation<T>
{
public:
	SplitsVariation(T weight = 1.0) : ParametricVariation<T>("splits", eVariationId::VAR_SPLITS, weight)
	{
		Init();
	}

	PARVARCOPY(SplitsVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (helper.In.x >= 0)
			helper.Out.x = m_Weight * (helper.In.x + m_X);
		else
			helper.Out.x = m_Weight * (helper.In.x - m_X);

		if (helper.In.y >= 0)
			helper.Out.y = m_Weight * (helper.In.y + m_Y);
		else
			helper.Out.y = m_Weight * (helper.In.y - m_Y);

		helper.Out.z = m_Weight * helper.In.z;//Original from flam3 does not have this, but the apo implementation does, so use Apo since it's more recent.
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
		   << "\t\tif (vIn.x >= 0)\n"
		   << "\t\t	vOut.x = " << weight << " * (vIn.x + " << x << ");\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = " << weight << " * (vIn.x - " << x << ");\n"
		   << "\n"
		   << "\t\tif (vIn.y >= 0)\n"
		   << "\t\t	vOut.y = " << weight << " * (vIn.y + " << y << ");\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = " << weight << " * (vIn.y - " << y << ");\n"
		   << "\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = rand.Frand11<T>();
		m_Y = rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "splits_x"));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "splits_y"));
	}

private:
	T m_X;
	T m_Y;
};

/// <summary>
/// Stripes.
/// </summary>
template <typename T>
class StripesVariation : public ParametricVariation<T>
{
public:
	StripesVariation(T weight = 1.0) : ParametricVariation<T>("stripes", eVariationId::VAR_STRIPES, weight)
	{
		Init();
	}

	PARVARCOPY(StripesVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T roundx = T(int(helper.In.x >= 0 ? (helper.In.x + T(0.5)) : (helper.In.x - T(0.5))));
		T offsetx = helper.In.x - roundx;
		helper.Out.x = m_Weight * (offsetx * (1 - m_Space) + roundx);
		helper.Out.y = m_Weight * (helper.In.y + offsetx * offsetx * m_Warp);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string space = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string warp = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t roundx = (real_t)(int)(vIn.x >= 0 ? (vIn.x + (real_t)(0.5)) : (vIn.x - (real_t)(0.5)));\n"
		   << "\t\treal_t offsetx = vIn.x - roundx;\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(offsetx, (real_t)(1.0) - " << space << ", roundx);\n"
		   << "\t\tvOut.y = " << weight << " * fma(SQR(offsetx), " << warp << ", vIn.y);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Params[0].Set(rand.Frand01<T>());//Space.
		m_Params[1].Set(5 * rand.Frand01<T>());//Warp.
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Space, prefix + "stripes_space", T(0.5), eParamType::REAL, T(0.5), 5));
		m_Params.push_back(ParamWithName<T>(&m_Warp, prefix + "stripes_warp"));
	}

private:
	T m_Space;
	T m_Warp;
};

/// <summary>
/// Wedge.
/// </summary>
template <typename T>
class WedgeVariation : public ParametricVariation<T>
{
public:
	WedgeVariation(T weight = 1.0) : ParametricVariation<T>("wedge", eVariationId::VAR_WEDGE, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(WedgeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = helper.m_PrecalcSqrtSumSquares;
		T a = helper.m_PrecalcAtanyx + m_Swirl * r;
		T c = T(Floor<T>((m_Count * a + T(M_PI)) * T(M_1_PI) * T(0.5)));
		a = a * m_CompFac + c * m_Angle;
		r = m_Weight * (r + m_Hole);
		helper.Out.x = r * std::cos(a);
		helper.Out.y = r * std::sin(a);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string angle   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string hole    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string count   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string swirl   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string compFac = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\t\treal_t a = fma(" << swirl << ", r, precalcAtanyx);\n"
		   << "\t\treal_t c = floor(fma(" << count << ", a, MPI) * M1PI * (real_t)(0.5));\n"
		   << "\n"
		   << "\t\ta = fma(a, " << compFac << ", c * " << angle << ");\n"
		   << "\t\tr = " << weight << " * (r + " << hole << ");\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_CompFac = 1 - m_Angle * m_Count * T(M_1_PI) * T(0.5);
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Angle = T(M_PI) * rand.Frand01<T>();
		m_Hole = T(0.5)  * rand.Frand11<T>();
		m_Count = T(Floor<T>(5 * rand.Frand01<T>())) + 1;
		m_Swirl = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "wedge_angle", T(M_PI_2)));
		m_Params.push_back(ParamWithName<T>(&m_Hole, prefix + "wedge_hole"));
		m_Params.push_back(ParamWithName<T>(&m_Count, prefix + "wedge_count", 2, eParamType::INTEGER, 1));
		m_Params.push_back(ParamWithName<T>(&m_Swirl, prefix + "wedge_swirl"));
		m_Params.push_back(ParamWithName<T>(true, &m_CompFac, prefix + "wedge_compfac"));//Precalc.
	}

private:
	T m_Angle;
	T m_Hole;
	T m_Count;
	T m_Swirl;
	T m_CompFac;//Precalc.
};

/// <summary>
/// Wedge julia.
/// </summary>
template <typename T>
class WedgeJuliaVariation : public ParametricVariation<T>
{
public:
	WedgeJuliaVariation(T weight = 1.0) : ParametricVariation<T>("wedge_julia", eVariationId::VAR_WEDGE_JULIA, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(WedgeJuliaVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight * std::pow(helper.m_PrecalcSumSquares, m_Cn);
		auto tRand = rand.Rand(size_t(m_Rn));
		T a = (helper.m_PrecalcAtanyx + M_2PI * tRand) / m_Power;
		T c = T(Floor<T>((m_Count * a + T(M_PI)) * T(M_1_PI) * T(0.5)));
		a = a * m_Cf + c * m_Angle;
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
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string count = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string cn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cf    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = " << weight << " * pow(precalcSumSquares, " << cn << ");\n"
		   << "\t\tuint tRand = MwcNextRange(mwc, (uint)" << rn << ");\n"
		   << "\t\treal_t a = fma(M_2PI, (real_t)tRand, precalcAtanyx) / " << power << ";\n"
		   << "\t\treal_t c = floor(fma(" << count << ", a, MPI) * M1PI * (real_t)(0.5));\n"
		   << "\n"
		   << "\t\ta = fma(a, " << cf << ", c * " << angle << ");\n"
		   << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Cf = 1 - m_Angle * m_Count * T(M_1_PI) * T(0.5);
		m_Rn = std::abs(m_Power);
		m_Cn = m_Dist / m_Power / 2;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Power = T(int(5 * rand.Frand01<T>() + 2));
		m_Dist = 1;
		m_Count = T(int(3 * rand.Frand01<T>() + 1));
		m_Angle = T(M_PI) * rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "wedge_julia_angle"));
		m_Params.push_back(ParamWithName<T>(&m_Count, prefix + "wedge_julia_count", 1));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "wedge_julia_power", 1));
		m_Params.push_back(ParamWithName<T>(&m_Dist, prefix + "wedge_julia_dist"));
		m_Params.push_back(ParamWithName<T>(true, &m_Rn, prefix + "wedge_julia_rn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix + "wedge_julia_cn"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cf, prefix + "wedge_julia_cf"));
	}

private:
	T m_Angle;
	T m_Count;
	T m_Power;
	T m_Dist;
	T m_Rn;//Precalc.
	T m_Cn;
	T m_Cf;
};

/// <summary>
/// Wedge sph.
/// </summary>
template <typename T>
class WedgeSphVariation : public ParametricVariation<T>
{
public:
	WedgeSphVariation(T weight = 1.0) : ParametricVariation<T>("wedge_sph", eVariationId::VAR_WEDGE_SPH, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(WedgeSphVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = 1 / Zeps(helper.m_PrecalcSqrtSumSquares);
		T a = helper.m_PrecalcAtanyx + m_Swirl * r;
		auto c = Floor<T>((m_Count * a + T(M_PI)) * m_C12Pi);
		a = a * m_CompFac + c * m_Angle;
		T temp = m_Weight * (r + m_Hole);
		helper.Out.x = temp * std::cos(a);
		helper.Out.y = temp * std::sin(a);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string angle   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string count   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string hole    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string swirl   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c12pi   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string compfac = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = (real_t)(1.0) / Zeps(precalcSqrtSumSquares);\n"
		   << "\t\treal_t a = fma(" << swirl << ", r, precalcAtanyx);\n"
		   << "\t\treal_t c = floor(fma(" << count << ", a, MPI) * " << c12pi << "); \n"
		   << "\n"
		   << "\t\ta = fma(a, " << compfac << ", c * " << angle << ");\n"
		   << "\t\treal_t temp = " << weight << " * (r + " << hole << ");\n"
		   << "\t\tvOut.x = temp * cos(a);\n"
		   << "\t\tvOut.y = temp * sin(a);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Angle = T(M_PI) * rand.Frand01<T>();
		m_Count = T(Floor<T>(5 * rand.Frand01<T>())) + 1;
		m_Hole = T(0.5)  * rand.Frand11<T>();
		m_Swirl = rand.Frand01<T>();
	}

	virtual void Precalc() override
	{
		m_C12Pi = T(M_1_PI) / 2;
		m_CompFac = 1 - m_Angle * m_Count * m_C12Pi;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "wedge_sph_angle", T(M_PI_2)));
		m_Params.push_back(ParamWithName<T>(&m_Count, prefix + "wedge_sph_count", 2, eParamType::INTEGER, 1));
		m_Params.push_back(ParamWithName<T>(&m_Hole, prefix + "wedge_sph_hole"));
		m_Params.push_back(ParamWithName<T>(&m_Swirl, prefix + "wedge_sph_swirl"));
		m_Params.push_back(ParamWithName<T>(true, &m_C12Pi, prefix + "wedge_sph_c1_2pi"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_CompFac, prefix + "wedge_sph_comp_fac"));
	}

private:
	T m_Angle;
	T m_Count;
	T m_Hole;
	T m_Swirl;
	T m_C12Pi;//Precalc.
	T m_CompFac;
};

/// <summary>
/// Whorl.
/// </summary>
template <typename T>
class WhorlVariation : public ParametricVariation<T>
{
public:
	WhorlVariation(T weight = 1.0) : ParametricVariation<T>("whorl", eVariationId::VAR_WHORL, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(WhorlVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a, r = helper.m_PrecalcSqrtSumSquares;

		if (r < m_Weight)
			a = helper.m_PrecalcAtanyx + m_Inside / (m_Weight - r);
		else
			a = helper.m_PrecalcAtanyx + m_Outside / Zeps(m_Weight - r);

		helper.Out.x = m_Weight * r * std::cos(a);
		helper.Out.y = m_Weight * r * std::sin(a);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string inside = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string outside = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a;\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tif (r < " << weight << ")\n"
		   << "\t\t	a = precalcAtanyx + " << inside << " / (" << weight << " - r);\n"
		   << "\t\telse\n"
		   << "\t\t	a = precalcAtanyx + " << outside << " / Zeps(" << weight << " - r);\n"
		   << "\n"
		   << "\t\tvOut.x = (" << weight << " * r * cos(a));\n"
		   << "\t\tvOut.y = (" << weight << " * r * sin(a));\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Inside = rand.Frand01<T>();
		m_Outside = rand.Frand01<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Inside, prefix + "whorl_inside", 1));
		m_Params.push_back(ParamWithName<T>(&m_Outside, prefix + "whorl_outside", 1));
	}

private:
	T m_Inside;
	T m_Outside;
};

/// <summary>
/// Waves.
/// </summary>
template <typename T>
class Waves2Variation : public ParametricVariation<T>
{
public:
	Waves2Variation(T weight = 1.0) : ParametricVariation<T>("waves2", eVariationId::VAR_WAVES2, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(Waves2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (helper.In.x + m_ScaleX * std::sin(helper.In.y * m_FreqX));
		helper.Out.y = m_Weight * (helper.In.y + m_ScaleY * std::sin(helper.In.x * m_FreqY));
		helper.Out.z = m_Weight * (helper.In.z + m_ScaleZ * std::sin(helper.m_PrecalcSqrtSumSquares * m_FreqZ));
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string freqX  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqY  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqZ  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleZ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * fma(" << scaleX << ", sin(vIn.y * " << freqX << "), vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * fma(" << scaleY << ", sin(vIn.x * " << freqY << "), vIn.y);\n"
		   << "\t\tvOut.z = " << weight << " * fma(" << scaleZ << ", sin(precalcSqrtSumSquares * " << freqZ << "), vIn.z);\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_FreqX = 4 * rand.Frand01<T>();
		m_ScaleX = T(0.5) + rand.Frand01<T>();
		m_FreqY = 4 * rand.Frand01<T>();
		m_ScaleY = T(0.5) + rand.Frand01<T>();
		m_FreqZ = 0;
		m_ScaleZ = 0;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_FreqX, prefix + "waves2_freqx", 2));
		m_Params.push_back(ParamWithName<T>(&m_ScaleX, prefix + "waves2_scalex"));
		m_Params.push_back(ParamWithName<T>(&m_FreqY, prefix + "waves2_freqy", 2));
		m_Params.push_back(ParamWithName<T>(&m_ScaleY, prefix + "waves2_scaley"));
		m_Params.push_back(ParamWithName<T>(&m_FreqZ, prefix + "waves2_freqz"));
		m_Params.push_back(ParamWithName<T>(&m_ScaleZ, prefix + "waves2_scalez"));
	}

private:
	T m_FreqX;
	T m_ScaleX;
	T m_FreqY;
	T m_ScaleY;
	T m_FreqZ;
	T m_ScaleZ;
};

/// <summary>
/// Exp.
/// </summary>
template <typename T>
class ExpVariation : public Variation<T>
{
public:
	ExpVariation(T weight = 1.0) : Variation<T>("exp", eVariationId::VAR_EXP, weight) { }

	VARCOPY(ExpVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T expe = m_Weight * std::exp(helper.In.x);
		helper.Out.x = expe * std::cos(helper.In.y);
		helper.Out.y = expe * std::sin(helper.In.y);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t expe = " << weight << " * exp(vIn.x);\n"
		   << "\n"
		   << "\t\tvOut.x = expe * cos(vIn.y);\n"
		   << "\t\tvOut.y = expe * sin(vIn.y);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Exp2.
/// By tatasz.
/// </summary>
template <typename T>
class Exp2Variation : public Variation<T>
{
public:
	Exp2Variation(T weight = 1.0) : Variation<T>("exp2", eVariationId::VAR_EXP2, weight) { }

	VARCOPY(Exp2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T ypi = helper.In.y * T(M_PI);
		T expe = m_Weight * std::exp(helper.In.x * T(M_PI));
		helper.Out.x = expe * std::cos(ypi);
		helper.Out.y = expe * std::sin(ypi);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t ypi = vIn.y * MPI;\n"
		   << "\t\treal_t expe = " << weight << " * exp(vIn.x * MPI);\n"
		   << "\n"
		   << "\t\tvOut.x = expe * cos(ypi);\n"
		   << "\t\tvOut.y = expe * sin(ypi);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Log.
/// </summary>
template <typename T>
class LogVariation : public ParametricVariation<T>
{
public:
	LogVariation(T weight = 1.0) : ParametricVariation<T>("log", eVariationId::VAR_LOG, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(LogVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = std::log(helper.m_PrecalcSumSquares) * m_Denom;
		helper.Out.y = m_Weight * helper.m_PrecalcAtanyx;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string base = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string denom = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = log(precalcSumSquares) * " << denom << ";\n"
		   << "\t\tvOut.y = " << weight << " * precalcAtanyx;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Denom = m_Weight * T(0.5) / std::log(m_Base);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Base, prefix + "log_base", T(M_E), eParamType::REAL, EPS, TMAX));
		m_Params.push_back(ParamWithName<T>(true, &m_Denom, prefix + "log_denom"));//Precalc.
	}

private:
	T m_Base;
	T m_Denom;//Precalc.
};

/// <summary>
/// Sine.
/// </summary>
template <typename T>
class SinVariation : public Variation<T>
{
public:
	SinVariation(T weight = 1.0) : Variation<T>("sin", eVariationId::VAR_SIN, weight) { }

	VARCOPY(SinVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			helper.Out.x = m_Weight * std::sin(helper.In.x) * std::cosh(helper.In.y);
			helper.Out.y = m_Weight * std::cos(helper.In.x) * std::sinh(helper.In.y);
		}
		else
		{
			T x = helper.In.x * M_PI2;
			T y = helper.In.y * M_PI2;
			helper.Out.x = m_Weight * std::sin(x) * std::cosh(y);
			helper.Out.y = m_Weight * std::cos(x) * std::sinh(y);
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		if (Compat::m_Compat)
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\tvOut.x = " << weight << " * sin(vIn.x) * cosh(vIn.y);\n"
			   << "\t\tvOut.y = " << weight << " * cos(vIn.x) * sinh(vIn.y);\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
		else
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t x = vIn.x * MPI2;\n"
			   << "\t\treal_t y = vIn.y * MPI2;\n"
			   << "\t\tvOut.x = " << weight << " * sin(x) * cosh(y);\n"
			   << "\t\tvOut.y = " << weight << " * cos(x) * sinh(y);\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
	}
};

/// <summary>
/// Cosine.
/// </summary>
template <typename T>
class CosVariation : public Variation<T>
{
public:
	CosVariation(T weight = 1.0) : Variation<T>("cos", eVariationId::VAR_COS, weight) { }

	VARCOPY(CosVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			//clamp fabs x and y to 7.104760e+002 for cosh, and |x| 7.104760e+002 for sinh
			helper.Out.x = m_Weight * std::cos(helper.In.x)   * std::cosh(helper.In.y);
			helper.Out.y = -(m_Weight * std::sin(helper.In.x) * std::sinh(helper.In.y));
		}
		else
		{
			T x = helper.In.x * M_PI2;
			T y = helper.In.y * M_PI2;
			helper.Out.x = m_Weight *  std::cos(x) * std::cosh(y);
			helper.Out.y = m_Weight * -std::sin(x) * std::sinh(y);
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		if (Compat::m_Compat)
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\tvOut.x = " << weight << " * cos(vIn.x) * cosh(vIn.y);\n"
			   << "\t\tvOut.y = -(" << weight << " * sin(vIn.x) * sinh(vIn.y));\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
		else
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t x = vIn.x * MPI2;\n"
			   << "\t\treal_t y = vIn.y * MPI2;\n"
			   << "\t\tvOut.x = " << weight << " *  cos(x) * cosh(y);\n"
			   << "\t\tvOut.y = " << weight << " * -sin(x) * sinh(y);\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
	}
};

/// <summary>
/// Tangent.
/// </summary>
template <typename T>
class TanVariation : public Variation<T>
{
public:
	TanVariation(T weight = 1.0) : Variation<T>("tan", eVariationId::VAR_TAN, weight) { }

	VARCOPY(TanVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			T tansin, tancos, tansinh, tancosh, tanden;
			sincos(2 * helper.In.x, &tansin, &tancos);
			tansinh = std::sinh(2 * helper.In.y);
			tancosh = std::cosh(2 * helper.In.y);
			tanden = 1 / Zeps(tancos + tancosh);
			helper.Out.x = m_Weight * tanden * tansin;
			helper.Out.y = m_Weight * tanden * tansinh;
		}
		else
		{
			T x = helper.In.x * M_PI2;
			T y = helper.In.y * M_PI2;
			T den = m_Weight / Zeps(std::cos(x) + std::cosh(y));
			helper.Out.x = std::sin(x) * den;
			helper.Out.y = std::sinh(y) * den;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		if (Compat::m_Compat)
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t tansin = sin((real_t)(2.0) * vIn.x);\n"
			   << "\t\treal_t tancos = cos((real_t)(2.0) * vIn.x);\n"
			   << "\t\treal_t tansinh = sinh((real_t)(2.0) * vIn.y);\n"
			   << "\t\treal_t tancosh = cosh((real_t)(2.0) * vIn.y);\n"
			   << "\t\treal_t tanden = (real_t)(1.0) / Zeps(tancos + tancosh);\n"
			   << "\n"
			   << "\t\tvOut.x = " << weight << " * tanden * tansin;\n"
			   << "\t\tvOut.y = " << weight << " * tanden * tansinh;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
		else
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t x = vIn.x * MPI2;\n"
			   << "\t\treal_t y = vIn.y * MPI2;\n"
			   << "\t\treal_t den = " << weight << " / Zeps(cos(x) + cosh(y));\n"
			   << "\n"
			   << "\t\tvOut.x = sin(x) * den;\n"
			   << "\t\tvOut.y = sinh(y) * den;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Sec.
/// </summary>
template <typename T>
class SecVariation : public Variation<T>
{
public:
	SecVariation(T weight = 1.0) : Variation<T>("sec", eVariationId::VAR_SEC, weight) { }

	VARCOPY(SecVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			T secsin, seccos, secsinh, seccosh, secden;
			sincos(helper.In.x, &secsin, &seccos);
			secsinh = std::sinh(helper.In.y);
			seccosh = std::cosh(helper.In.y);
			secden = 2 / Zeps(std::cos(2 * helper.In.x) + std::cosh(2 * helper.In.y));
			helper.Out.x = m_Weight * secden * seccos * seccosh;
			helper.Out.y = m_Weight * secden * secsin * secsinh;
		}
		else
		{
			T x = helper.In.x * T(M_PI);
			T y = helper.In.y * T(M_PI);
			T secsin, seccos, secsinh, seccosh, secden;
			sincos(x, &secsin, &seccos);
			secsinh = std::sinh(y);
			seccosh = std::cosh(y);
			secden = m_Weight * (2 / Zeps(std::cos(2 * x) + std::cosh(2 * y)));
			helper.Out.x = secden * seccos * seccosh;
			helper.Out.y = secden * secsin * secsinh;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		if (Compat::m_Compat)
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t secsin = sin(vIn.x);\n"
			   << "\t\treal_t seccos = cos(vIn.x);\n"
			   << "\t\treal_t secsinh = sinh(vIn.y);\n"
			   << "\t\treal_t seccosh = cosh(vIn.y);\n"
			   << "\t\treal_t secden = (real_t)(2.0) / Zeps(cos((real_t)(2.0) * vIn.x) + cosh((real_t)(2.0) * vIn.y));\n"
			   << "\n"
			   << "\t\tvOut.x = " << weight << " * secden * seccos * seccosh;\n"
			   << "\t\tvOut.y = " << weight << " * secden * secsin * secsinh;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
		else
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t x = vIn.x * MPI;\n"
			   << "\t\treal_t y = vIn.y * MPI;\n"
			   << "\t\treal_t secsin = sin(x);\n"
			   << "\t\treal_t seccos = cos(x);\n"
			   << "\t\treal_t secsinh = sinh(y);\n"
			   << "\t\treal_t seccosh = cosh(y);\n"
			   << "\t\treal_t secden = " << weight << " * ((real_t)(2.0) / Zeps(cos((real_t)(2.0) * x) + cosh((real_t)(2.0) * y)));\n"
			   << "\n"
			   << "\t\tvOut.x = secden * seccos * seccosh;\n"
			   << "\t\tvOut.y = secden * secsin * secsinh;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Cosecant.
/// </summary>
template <typename T>
class CscVariation : public Variation<T>
{
public:
	CscVariation(T weight = 1.0) : Variation<T>("csc", eVariationId::VAR_CSC, weight) { }

	VARCOPY(CscVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			T cscsin, csccos, cscsinh, csccosh, cscden;
			sincos(helper.In.x, &cscsin, &csccos);
			cscsinh = std::sinh(helper.In.y);
			csccosh = std::cosh(helper.In.y);
			cscden = 2 / Zeps(std::cosh(2 * helper.In.y) - std::cos(2 * helper.In.x));
			helper.Out.x = m_Weight * cscden * cscsin * csccosh;
			helper.Out.y = -(m_Weight * cscden * csccos * cscsinh);
		}
		else
		{
			T x = helper.In.x * M_PI2;
			T y = helper.In.y * M_PI2;
			T cscsin, csccos, cscsinh, csccosh, cscden, d;
			sincos(x, &cscsin, &csccos);
			cscsinh = std::sinh(y);
			csccosh = std::cosh(y);
			d = 1 + 2 * cscsinh * cscsinh - std::cos(2 * x);
			cscden = 2 * m_Weight / d;
			helper.Out.x = cscden * cscsin * csccosh;
			helper.Out.y = cscden * csccos * cscsinh;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		if (Compat::m_Compat)
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t cscsin = sin(vIn.x);\n"
			   << "\t\treal_t csccos = cos(vIn.x);\n"
			   << "\t\treal_t cscsinh = sinh(vIn.y);\n"
			   << "\t\treal_t csccosh = cosh(vIn.y);\n"
			   << "\t\treal_t cscden = (real_t)(2.0) / Zeps(cosh((real_t)(2.0) * vIn.y) - cos((real_t)(2.0) * vIn.x));\n"
			   << "\n"
			   << "\t\tvOut.x = " << weight << " * cscden * cscsin * csccosh;\n"
			   << "\t\tvOut.y = -(" << weight << " * cscden * csccos * cscsinh);\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
		else
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t x = vIn.x * MPI2;\n"
			   << "\t\treal_t y = vIn.y * MPI2;\n"
			   << "\t\treal_t cscsin = sin(x);\n"
			   << "\t\treal_t csccos = cos(x);\n"
			   << "\t\treal_t cscsinh = sinh(y);\n"
			   << "\t\treal_t csccosh = cosh(y);\n"
			   << "\t\treal_t d = Zeps(1 + 2 * cscsinh * cscsinh - cos(2 * x));\n"
			   << "\t\treal_t cscden = 2 * " << weight << " / d;\n"
			   << "\n"
			   << "\t\tvOut.x = cscden * cscsin * csccosh;\n"
			   << "\t\tvOut.y = cscden * csccos * cscsinh;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Cotangent.
/// </summary>
template <typename T>
class CotVariation : public Variation<T>
{
public:
	CotVariation(T weight = 1.0) : Variation<T>("cot", eVariationId::VAR_COT, weight) { }

	VARCOPY(CotVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			T cotsin, cotcos, cotsinh, cotcosh, cotden;
			sincos(2 * helper.In.x, &cotsin, &cotcos);
			cotsinh = std::sinh(2 * helper.In.y);
			cotcosh = std::cosh(2 * helper.In.y);
			cotden = 1 / Zeps(cotcosh - cotcos);
			helper.Out.x = m_Weight * cotden * cotsin;
			helper.Out.y = m_Weight * cotden * -1 * cotsinh;
		}
		else
		{
			T x = helper.In.x * M_PI2;
			T y = helper.In.y * M_PI2;
			T cotsin, cotcos, cotsinh, cotcosh, cotden;
			sincos(x, &cotsin, &cotcos);
			cotsinh = std::sinh(y);
			cotcosh = std::cosh(y);
			cotden = m_Weight / Zeps(cotcosh - cotcos);
			helper.Out.x = cotden * cotsin;
			helper.Out.y = cotden * cotsinh;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		if (Compat::m_Compat)
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t cotsin = sin((real_t)(2.0) * vIn.x);\n"
			   << "\t\treal_t cotcos = cos((real_t)(2.0) * vIn.x);\n"
			   << "\t\treal_t cotsinh = sinh((real_t)(2.0) * vIn.y);\n"
			   << "\t\treal_t cotcosh = cosh((real_t)(2.0) * vIn.y);\n"
			   << "\t\treal_t cotden = (real_t)(1.0) / Zeps(cotcosh - cotcos);\n"
			   << "\n"
			   << "\t\tvOut.x = " << weight << " * cotden * cotsin;\n"
			   << "\t\tvOut.y = " << weight << " * cotden * -1 * cotsinh;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
		else
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t x = vIn.x * MPI2;\n"
			   << "\t\treal_t y = vIn.y * MPI2;\n"
			   << "\t\treal_t cotsin = sin(x);\n"
			   << "\t\treal_t cotcos = cos(x);\n"
			   << "\t\treal_t cotsinh = sinh(y);\n"
			   << "\t\treal_t cotcosh = cosh(y);\n"
			   << "\t\treal_t cotden = " << weight << " / Zeps(cotcosh - cotcos);\n"
			   << "\n"
			   << "\t\tvOut.x = cotden * cotsin;\n"
			   << "\t\tvOut.y = cotden * cotsinh;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Sinh.
/// </summary>
template <typename T>
class SinhVariation : public Variation<T>
{
public:
	SinhVariation(T weight = 1.0) : Variation<T>("sinh", eVariationId::VAR_SINH, weight) { }

	VARCOPY(SinhVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			T sinhsin, sinhcos, sinhsinh, sinhcosh;
			sincos(helper.In.y, &sinhsin, &sinhcos);
			sinhsinh = std::sinh(helper.In.x);
			sinhcosh = std::cosh(helper.In.x);
			helper.Out.x = m_Weight * sinhsinh * sinhcos;
			helper.Out.y = m_Weight * sinhcosh * sinhsin;
		}
		else
		{
			T sinhsin, sinhcos, sinhsinh, sinhcosh;
			T x = helper.In.x * M_PI4;
			T y = helper.In.y * M_PI4;
			sincos(y, &sinhsin, &sinhcos);
			sinhsinh = std::sinh(x);
			sinhcosh = std::cosh(x);
			helper.Out.x = m_Weight * sinhsinh * sinhcos;
			helper.Out.y = m_Weight * sinhcosh * sinhsin;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		if (Compat::m_Compat)
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t sinhsin = sin(vIn.y);\n"
			   << "\t\treal_t sinhcos = cos(vIn.y);\n"
			   << "\t\treal_t sinhsinh = sinh(vIn.x);\n"
			   << "\t\treal_t sinhcosh = cosh(vIn.x);\n"
			   << "\n"
			   << "\t\tvOut.x = " << weight << " * sinhsinh * sinhcos;\n"
			   << "\t\tvOut.y = " << weight << " * sinhcosh * sinhsin;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
		else
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t x = vIn.x * MPI4;\n"
			   << "\t\treal_t y = vIn.y * MPI4;\n"
			   << "\t\treal_t sinhsin = sin(y);\n"
			   << "\t\treal_t sinhcos = cos(y);\n"
			   << "\t\treal_t sinhsinh = sinh(x);\n"
			   << "\t\treal_t sinhcosh = cosh(x);\n"
			   << "\n"
			   << "\t\tvOut.x = " << weight << " * sinhsinh * sinhcos;\n"
			   << "\t\tvOut.y = " << weight << " * sinhcosh * sinhsin;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
	}
};

/// <summary>
/// Cosh.
/// </summary>
template <typename T>
class CoshVariation : public Variation<T>
{
public:
	CoshVariation(T weight = 1.0) : Variation<T>("cosh", eVariationId::VAR_COSH, weight) { }

	VARCOPY(CoshVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			T coshsin, coshcos, coshsinh, coshcosh;
			sincos(helper.In.y, &coshsin, &coshcos);
			coshsinh = std::sinh(helper.In.x);
			coshcosh = std::cosh(helper.In.x);
			helper.Out.x = m_Weight * coshcosh * coshcos;
			helper.Out.y = m_Weight * coshsinh * coshsin;
		}
		else
		{
			T coshsin, coshcos, coshsinh, coshcosh;
			T x = helper.In.x * M_PI2;
			T y = helper.In.y * M_PI2;
			sincos(y, &coshsin, &coshcos);
			coshsinh = std::sinh(x);
			coshcosh = std::cosh(x);
			helper.Out.x = m_Weight * coshcosh * coshcos;
			helper.Out.y = m_Weight * coshsinh * coshsin;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		if (Compat::m_Compat)
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t coshsin = sin(vIn.y);\n"
			   << "\t\treal_t coshcos = cos(vIn.y);\n"
			   << "\t\treal_t coshsinh = sinh(vIn.x);\n"
			   << "\t\treal_t coshcosh = cosh(vIn.x);\n"
			   << "\n"
			   << "\t\tvOut.x = " << weight << " * coshcosh * coshcos;\n"
			   << "\t\tvOut.y = " << weight << " * coshsinh * coshsin;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
		else
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t x = vIn.x * MPI2;\n"
			   << "\t\treal_t y = vIn.y * MPI2;\n"
			   << "\t\treal_t coshsin = sin(y);\n"
			   << "\t\treal_t coshcos = cos(y);\n"
			   << "\t\treal_t coshsinh = sinh(x);\n"
			   << "\t\treal_t coshcosh = cosh(x);\n"
			   << "\n"
			   << "\t\tvOut.x = " << weight << " * coshcosh * coshcos;\n"
			   << "\t\tvOut.y = " << weight << " * coshsinh * coshsin;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
	}
};

/// <summary>
/// Tanh.
/// </summary>
template <typename T>
class TanhVariation : public Variation<T>
{
public:
	TanhVariation(T weight = 1.0) : Variation<T>("tanh", eVariationId::VAR_TANH, weight) { }

	VARCOPY(TanhVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			T tanhsin, tanhcos, tanhsinh, tanhcosh, tanhden;
			sincos(2 * helper.In.y, &tanhsin, &tanhcos);
			tanhsinh = std::sinh(2 * helper.In.x);
			tanhcosh = std::cosh(2 * helper.In.x);
			tanhden = 1 / Zeps(tanhcos + tanhcosh);
			helper.Out.x = m_Weight * tanhden * tanhsinh;
			helper.Out.y = m_Weight * tanhden * tanhsin;
		}
		else
		{
			T tanhsin, tanhcos, tanhsinh, tanhcosh, tanhden;
			T x = helper.In.x * M_PI2;
			T y = helper.In.y * M_PI2;
			sincos(y, &tanhsin, &tanhcos);
			tanhsinh = std::sinh(x);
			tanhcosh = std::cosh(x);
			tanhden = m_Weight / Zeps(tanhcos + tanhcosh);
			helper.Out.x = tanhden * tanhsinh;
			helper.Out.y = tanhden * tanhsin;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		if (Compat::m_Compat)
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t tanhsin = sin((real_t)(2.0) * vIn.y);\n"
			   << "\t\treal_t tanhcos = cos((real_t)(2.0) * vIn.y);\n"
			   << "\t\treal_t tanhsinh = sinh((real_t)(2.0) * vIn.x);\n"
			   << "\t\treal_t tanhcosh = cosh((real_t)(2.0) * vIn.x);\n"
			   << "\t\treal_t tanhden = (real_t)(1.0) / Zeps(tanhcos + tanhcosh);\n"
			   << "\n"
			   << "\t\tvOut.x = " << weight << " * tanhden * tanhsinh;\n"
			   << "\t\tvOut.y = " << weight << " * tanhden * tanhsin;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
		else
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t x = vIn.x * MPI2;\n"
			   << "\t\treal_t y = vIn.y * MPI2;\n"
			   << "\t\treal_t tanhsin = sin(y);\n"
			   << "\t\treal_t tanhcos = cos(y);\n"
			   << "\t\treal_t tanhsinh = sinh(x);\n"
			   << "\t\treal_t tanhcosh = cosh(x);\n"
			   << "\t\treal_t tanhden = " << weight << " / Zeps(tanhcos + tanhcosh);\n"
			   << "\n"
			   << "\t\tvOut.x = tanhden * tanhsinh;\n"
			   << "\t\tvOut.y = tanhden * tanhsin;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// tanh_spiral.
/// </summary>
template <typename T>
class TanhSpiralVariation : public ParametricVariation<T>
{
public:
	TanhSpiralVariation(T weight = 1.0) : ParametricVariation<T>("tanh_spiral", eVariationId::VAR_TANH_SPIRAL, weight)
	{
		Init();
	}

	PARVARCOPY(TanhSpiralVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t2 = (rand.Frand01<T>() - T(0.5)) * M_2PI;
		T aux = Zeps(std::cos(m_A * t2) + std::cosh(t2));
		helper.Out.x = m_Weight * (std::sinh(t2) / aux);
		helper.Out.y = m_Weight * (std::sin(m_A * t2) / aux);
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
		ss << "\t{\n"
		   << "\t\treal_t t2 = (MwcNext01(mwc) - 0.5) * M_2PI;\n"
		   << "\t\treal_t aux = Zeps(cos(" << a << " * t2) + cosh(t2));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (sinh(t2) / aux);\n"
		   << "\t\tvOut.y = " << weight << " * (sin(" << a << " * t2) / aux);\n"
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
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "tanh_spiral_a", 4));
	}

private:
	T m_A;
};

/// <summary>
/// Sech
/// </summary>
template <typename T>
class SechVariation : public Variation<T>
{
public:
	SechVariation(T weight = 1.0) : Variation<T>("sech", eVariationId::VAR_SECH, weight) { }

	VARCOPY(SechVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			T sechsin, sechcos, sechsinh, sechcosh, sechden;
			sincos(helper.In.y, &sechsin, &sechcos);
			sechsinh = std::sinh(helper.In.x);
			sechcosh = std::cosh(helper.In.x);
			sechden = 2 / Zeps(std::cos(2 * helper.In.y) + std::cosh(2 * helper.In.x));
			helper.Out.x = m_Weight * sechden * sechcos * sechcosh;
			helper.Out.y = -(m_Weight * sechden * sechsin * sechsinh);
		}
		else
		{
			T sechsin, sechcos, sechsinh, sechcosh, sechden;
			T x = helper.In.x * M_PI4;
			T y = helper.In.y * M_PI4;
			sincos(y, &sechsin, &sechcos);
			sechsinh = std::sinh(x);
			sechcosh = std::cosh(x);
			sechden = m_Weight * (2 / Zeps(std::cos(y * 2) + std::cosh(x * 2)));
			helper.Out.x = sechden * sechcos * sechcosh;
			helper.Out.y = sechden * sechsin * sechsinh;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		if (Compat::m_Compat)
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t sechsin = sin(vIn.y);\n"
			   << "\t\treal_t sechcos = cos(vIn.y);\n"
			   << "\t\treal_t sechsinh = sinh(vIn.x);\n"
			   << "\t\treal_t sechcosh = cosh(vIn.x);\n"
			   << "\t\treal_t sechden = (real_t)(2.0) / Zeps(cos((real_t)(2.0) * vIn.y) + cosh((real_t)(2.0) * vIn.x));\n"
			   << "\n"
			   << "\t\tvOut.x = " << weight << " * sechden * sechcos * sechcosh;\n"
			   << "\t\tvOut.y = -(" << weight << " * sechden * sechsin * sechsinh);\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
		else
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t x = vIn.x * MPI4;\n"
			   << "\t\treal_t y = vIn.y * MPI4;\n"
			   << "\t\treal_t sechsin = sin(y);\n"
			   << "\t\treal_t sechcos = cos(y);\n"
			   << "\t\treal_t sechsinh = sinh(x);\n"
			   << "\t\treal_t sechcosh = cosh(x);\n"
			   << "\t\treal_t sechden = " << weight << " * ((real_t)(2.0) / Zeps(cos(y * (real_t)(2.0)) + cosh(x * (real_t)(2.0))));\n"
			   << "\n"
			   << "\t\tvOut.x = sechden * sechcos * sechcosh;\n"
			   << "\t\tvOut.y = sechden * sechsin * sechsinh;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Csch.
/// </summary>
template <typename T>
class CschVariation : public Variation<T>
{
public:
	CschVariation(T weight = 1.0) : Variation<T>("csch", eVariationId::VAR_CSCH, weight) { }

	VARCOPY(CschVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			T cschsin, cschcos, cschsinh, cschcosh, cschden;
			sincos(helper.In.y, &cschsin, &cschcos);
			cschsinh = std::sinh(helper.In.x);
			cschcosh = std::cosh(helper.In.x);
			cschden = 2 / Zeps(std::cosh(2 * helper.In.x) - std::cos(2 * helper.In.y));
			helper.Out.x = m_Weight * cschden * cschsinh * cschcos;
			helper.Out.y = -(m_Weight * cschden * cschcosh * cschsin);
		}
		else
		{
			T cschsin, cschcos, cschsinh, cschcosh, cschden;
			T x = helper.In.x * M_PI4;
			T y = helper.In.y * M_PI4;
			sincos(y, &cschsin, &cschcos);
			cschsinh = std::sinh(x);
			cschcosh = std::cosh(x);
			cschden = m_Weight * (2 / Zeps(std::cosh(2 * x) - std::cos(2 * y)));
			helper.Out.x = cschden * cschsinh * cschcos;
			helper.Out.y = cschden * cschcosh * cschsin;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		if (Compat::m_Compat)
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t cschsin = sin(vIn.y);\n"
			   << "\t\treal_t cschcos = cos(vIn.y);\n"
			   << "\t\treal_t cschsinh = sinh(vIn.x);\n"
			   << "\t\treal_t cschcosh = cosh(vIn.x);\n"
			   << "\t\treal_t cschden = (real_t)(2.0) / Zeps(cosh((real_t)(2.0) * vIn.x) - cos((real_t)(2.0) * vIn.y));\n"
			   << "\n"
			   << "\t\tvOut.x = " << weight << " * cschden * cschsinh * cschcos;\n"
			   << "\t\tvOut.y = -(" << weight << " * cschden * cschcosh * cschsin);\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
		else
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t x = vIn.x * MPI4;\n"
			   << "\t\treal_t y = vIn.y * MPI4;\n"
			   << "\t\treal_t cschsin = sin(y);\n"
			   << "\t\treal_t cschcos = cos(y);\n"
			   << "\t\treal_t cschsinh = sinh(x);\n"
			   << "\t\treal_t cschcosh = cosh(x);\n"
			   << "\t\treal_t cschden = " << weight << " * ((real_t)(2.0) / Zeps(cosh((real_t)(2.0) * x) - cos((real_t)(2.0) * y)));\n"
			   << "\n"
			   << "\t\tvOut.x = cschden * cschsinh * cschcos;\n"
			   << "\t\tvOut.y = cschden * cschcosh * cschsin;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Coth.
/// </summary>
template <typename T>
class CothVariation : public Variation<T>
{
public:
	CothVariation(T weight = 1.0) : Variation<T>("coth", eVariationId::VAR_COTH, weight) { }

	VARCOPY(CothVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			T cothsin, cothcos, cothsinh, cothcosh, cothden;
			sincos(2 * helper.In.y, &cothsin, &cothcos);
			cothsinh = std::sinh(2 * helper.In.x);
			cothcosh = std::cosh(2 * helper.In.x);
			cothden = 1 / Zeps(cothcosh - cothcos);
			helper.Out.x = m_Weight * cothden * cothsinh;
			helper.Out.y = m_Weight * cothden * cothsin;
		}
		else
		{
			T cothsin, cothcos, cothsinh, cothcosh, cothden;
			T x = helper.In.x * M_PI2;
			T y = helper.In.y * M_PI2;
			sincos(y, &cothsin, &cothcos);
			cothsinh = std::sinh(x);
			cothcosh = std::cosh(x);
			cothden = m_Weight / Zeps(cothcosh - cothcos);
			helper.Out.x = cothden * cothsinh;
			helper.Out.y = cothden * cothsin;
		}

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		if (Compat::m_Compat)
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t cothsin = sin((real_t)(2.0) * vIn.y);\n"
			   << "\t\treal_t cothcos = cos((real_t)(2.0) * vIn.y);\n"
			   << "\t\treal_t cothsinh = sinh((real_t)(2.0) * vIn.x);\n"
			   << "\t\treal_t cothcosh = cosh((real_t)(2.0) * vIn.x);\n"
			   << "\t\treal_t cothden = (real_t)(1.0) / Zeps(cothcosh - cothcos);\n"
			   << "\n"
			   << "\t\tvOut.x = " << weight << " * cothden * cothsinh;\n"
			   << "\t\tvOut.y = " << weight << " * cothden * cothsin;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
		else
		{
			ostringstream ss;
			string weight = WeightDefineString();
			ss << "\t{\n"
			   << "\t\treal_t x = vIn.x * MPI2;\n"
			   << "\t\treal_t y = vIn.y * MPI2;\n"
			   << "\t\treal_t cothsin = sin(y);\n"
			   << "\t\treal_t cothcos = cos(y);\n"
			   << "\t\treal_t cothsinh = sinh(x);\n"
			   << "\t\treal_t cothcosh = cosh(x);\n"
			   << "\t\treal_t cothden = " << weight << " / Zeps(cothcosh - cothcos);\n"
			   << "\n"
			   << "\t\tvOut.x = cothden * cothsinh;\n"
			   << "\t\tvOut.y = cothden * cothsin;\n"
			   << "\t\tvOut.z = " << DefaultZCl()
			   << "\t}\n";
			return ss.str();
		}
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Auger.
/// </summary>
template <typename T>
class AugerVariation : public ParametricVariation<T>
{
public:
	AugerVariation(T weight = 1.0) : ParametricVariation<T>("auger", eVariationId::VAR_AUGER, weight)
	{
		Init();
	}

	PARVARCOPY(AugerVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T s = std::sin(m_Freq * helper.In.x);
		T t = std::sin(m_Freq * helper.In.y);
		T dx = helper.In.x + m_AugerWeight * (m_HalfScale * t + std::abs(helper.In.x) * t);
		T dy = helper.In.y + m_AugerWeight * (m_HalfScale * s + std::abs(helper.In.y) * s);
		helper.Out.x = m_Weight * (helper.In.x + m_Symmetry * (dx - helper.In.x));
		helper.Out.y = m_Weight * dy;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string symmetry    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string augerWeight = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freq        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfscale   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t s = sin(" << freq << " * vIn.x);\n"
		   << "\t\treal_t t = sin(" << freq << " * vIn.y);\n"
		   << "\t\treal_t dx = fma(" << augerWeight << ", fma(" << halfscale << ", t, fabs(vIn.x) * t), vIn.x);\n"
		   << "\t\treal_t dy = fma(" << augerWeight << ", fma(" << halfscale << ", s, fabs(vIn.y) * s), vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(" << symmetry << ", (dx - vIn.x), vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * dy;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Symmetry = 0;
		m_AugerWeight = T(0.5) + rand.Frand01<T>() / 2;
		m_Freq = T(Floor<T>(5 * rand.Frand01<T>())) + 1;
		m_Scale = rand.Frand01<T>();
	}

	virtual void Precalc() override
	{
		m_HalfScale = m_Scale / 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Symmetry, prefix + "auger_sym"));
		m_Params.push_back(ParamWithName<T>(&m_AugerWeight, prefix + "auger_weight", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Freq, prefix + "auger_freq", 5));
		m_Params.push_back(ParamWithName<T>(&m_Scale, prefix + "auger_scale", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_HalfScale, prefix + "auger_half_scale"));
	}

private:
	T m_Symmetry;
	T m_AugerWeight;
	T m_Freq;
	T m_Scale;
	T m_HalfScale;
};

/// <summary>
/// Flux.
/// </summary>
template <typename T>
class FluxVariation : public ParametricVariation<T>
{
public:
	FluxVariation(T weight = 1.0) : ParametricVariation<T>("flux", eVariationId::VAR_FLUX, weight)
	{
		Init();
	}

	PARVARCOPY(FluxVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xpw = helper.In.x + m_Weight;
		T xmw = helper.In.x - m_Weight;
		T yy = SQR(helper.In.y);
		T frac = std::sqrt(yy + SQR(xmw));

		if (frac == 0)
			frac = 1;

		T avgr = m_Weight * (m_Spr * std::sqrt(std::sqrt(yy + SQR(xpw)) / frac));
		T avga = (std::atan2(helper.In.y, xmw) - std::atan2(helper.In.y, xpw)) * T(0.5);
		helper.Out.x = avgr * std::cos(avga);
		helper.Out.y = avgr * std::sin(avga);
		helper.Out.z = helper.In.z;//Apo does not use weight, sums only z. Sum here for reg, else assign.
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string spread = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spr = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t xpw = vIn.x + " << weight << ";\n"
		   << "\t\treal_t xmw = vIn.x - " << weight << ";\n"
		   << "\t\treal_t yy = SQR(vIn.y);\n"
		   << "\t\treal_t frac = sqrt(fma(xmw, xmw, yy));\n"
		   << "\n"
		   << "\t\tif (frac == (real_t)(0.0))\n"
		   << "\t\t	frac = (real_t)(1.0);\n"
		   << "\n"
		   << "\t\treal_t avgr = " << weight << " * (" << spr << " * sqrt(sqrt(fma(xpw, xpw, yy)) / frac));\n"
		   << "\t\treal_t avga = (atan2(vIn.y, xmw) - atan2(vIn.y, xpw)) * (real_t)(0.5);\n"
		   << "\n"
		   << "\t\tvOut.x = avgr * cos(avga);\n"
		   << "\t\tvOut.y = avgr * sin(avga);\n"
		   << "\t\tvOut.z = vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Spr = 2 + m_Spread;
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_Spread = T(0.5) + rand.Frand01<T>() / 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Spread, prefix + "flux_spread"));
		m_Params.push_back(ParamWithName<T>(true, &m_Spr, prefix + "flux_spr"));//Precalc.
	}

private:
	T m_Spread;
	T m_Spr;//Precalc.
};

MAKEPREPOSTVAR(Linear, linear, LINEAR)
MAKEPREPOSTVAR(Sinusoidal, sinusoidal, SINUSOIDAL)
MAKEPREPOSTVAR(Spherical, spherical, SPHERICAL)
MAKEPREPOSTVAR(Swirl, swirl, SWIRL)
MAKEPREPOSTPARVAR(Swirl3, swirl3, SWIRL3)
MAKEPREPOSTPARVAR(Swirl3r, swirl3r, SWIRL3R)
MAKEPREPOSTVAR(Horseshoe, horseshoe, HORSESHOE)
MAKEPREPOSTVAR(Polar, polar, POLAR)
MAKEPREPOSTVAR(Handkerchief, handkerchief, HANDKERCHIEF)
MAKEPREPOSTVAR(Heart, heart, HEART)
MAKEPREPOSTPARVAR(Disc, disc, DISC)
MAKEPREPOSTVAR(Spiral, spiral, SPIRAL)
MAKEPREPOSTVAR(Hyperbolic, hyperbolic, HYPERBOLIC)
MAKEPREPOSTVAR(Diamond, diamond, DIAMOND)
MAKEPREPOSTVAR(Ex, ex, EX)
MAKEPREPOSTVAR(Julia, julia, JULIA)
MAKEPREPOSTVAR(Bent, bent, BENT)
MAKEPREPOSTPARVAR(Waves, waves, WAVES)
MAKEPREPOSTVAR(Fisheye, fisheye, FISHEYE)
MAKEPREPOSTVAR(Popcorn, popcorn, POPCORN)
MAKEPREPOSTVAR(Exponential, exponential, EXPONENTIAL)
MAKEPREPOSTVAR(Power, power, POWER)
MAKEPREPOSTVAR(Cosine, cosine, COSINE)
MAKEPREPOSTVAR(Rings, rings, RINGS)
MAKEPREPOSTVAR(Fan, fan, FAN)
MAKEPREPOSTPARVAR(Blob, blob, BLOB)
MAKEPREPOSTPARVAR(Pdj, pdj, PDJ)
MAKEPREPOSTPARVAR(Fan2, fan2, FAN2)
MAKEPREPOSTPARVAR(Rings2, rings2, RINGS2)
MAKEPREPOSTVAR(Eyefish, eyefish, EYEFISH)
MAKEPREPOSTVAR(Bubble, bubble, BUBBLE)
MAKEPREPOSTVAR(Cylinder, cylinder, CYLINDER)
MAKEPREPOSTPARVAR(Perspective, perspective, PERSPECTIVE)
MAKEPREPOSTVAR(Noise, noise, NOISE)
MAKEPREPOSTPARVAR(JuliaNGeneric, julian, JULIAN)
MAKEPREPOSTPARVAR(JuliaScope, juliascope, JULIASCOPE)
MAKEPREPOSTVARASSIGN(Blur, blur, BLUR, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTVARASSIGN(GaussianBlur, gaussian_blur, GAUSSIAN_BLUR, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTVAR(Gaussian, gaussian, GAUSSIAN)
MAKEPREPOSTPARVAR(RadialBlur, radial_blur, RADIAL_BLUR)
//MAKEPREPOSTPARVAR(RadialGaussian, radial_gaussian, RADIAL_GAUSSIAN)
MAKEPREPOSTPARVARASSIGN(Pie, pie, PIE, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(Ngon, ngon, NGON)
MAKEPREPOSTPARVAR(Curl, curl, CURL)
MAKEPREPOSTPARVAR(Rectangles, rectangles, RECTANGLES)
MAKEPREPOSTVARASSIGN(Arch, arch, ARCH, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTVAR(Tangent, tangent, TANGENT)
MAKEPREPOSTVARASSIGN(Square, square, SQUARE, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTVAR(Rays, rays, RAYS)
MAKEPREPOSTVAR(Rays1, rays1, RAYS1)
MAKEPREPOSTVAR(Rays2, rays2, RAYS2)
MAKEPREPOSTVAR(Rays3, rays3, RAYS3)
MAKEPREPOSTVAR(Blade, blade, BLADE)
MAKEPREPOSTVAR(Secant2, secant2, SECANT2)
MAKEPREPOSTVAR(TwinTrian, TwinTrian, TWINTRIAN)
MAKEPREPOSTVAR(Cross, cross, CROSS)
MAKEPREPOSTPARVAR(Disc2, disc2, DISC2)
MAKEPREPOSTPARVAR(SuperShape, super_shape, SUPER_SHAPE)
MAKEPREPOSTPARVAR(Flower, flower, FLOWER)
MAKEPREPOSTPARVAR(FlowerDb, flowerdb, FLOWER_DB)
MAKEPREPOSTPARVAR(Conic, conic, CONIC)
MAKEPREPOSTPARVAR(Parabola, parabola, PARABOLA)
MAKEPREPOSTPARVAR(Bent2, bent2, BENT2)
MAKEPREPOSTPARVAR(Bipolar, bipolar, BIPOLAR)
MAKEPREPOSTVAR(Boarders, boarders, BOARDERS)
MAKEPREPOSTVAR(Butterfly, butterfly, BUTTERFLY)
MAKEPREPOSTPARVAR(Cell, cell, CELL)
MAKEPREPOSTPARVAR(Cpow, cpow, CPOW)
MAKEPREPOSTPARVAR(Curve, curve, CURVE)
MAKEPREPOSTVAR(Edisc, edisc, EDISC)
MAKEPREPOSTPARVAR(Elliptic, elliptic, ELLIPTIC)
MAKEPREPOSTPARVAR(Escher, escher, ESCHER)
MAKEPREPOSTVAR(Foci, foci, FOCI)
MAKEPREPOSTPARVAR(LazySusan, lazysusan, LAZYSUSAN)
MAKEPREPOSTPARVAR(Loonie, loonie, LOONIE)
MAKEPREPOSTPARVAR(Modulus, modulus, MODULUS)
MAKEPREPOSTPARVAR(Oscilloscope, oscilloscope, OSCILLOSCOPE)
MAKEPREPOSTPARVAR(Oscilloscope2, oscilloscope2, OSCILLOSCOPE2)
MAKEPREPOSTPARVAR(Polar2, polar2, POLAR2)
MAKEPREPOSTPARVAR(Popcorn2, popcorn2, POPCORN2)
MAKEPREPOSTPARVAR(Scry, scry, SCRY)
MAKEPREPOSTPARVAR(Scry2, scry2, SCRY2)
MAKEPREPOSTPARVAR(Separation, separation, SEPARATION)
MAKEPREPOSTPARVAR(Split, split, SPLIT)
MAKEPREPOSTPARVAR(Splits, splits, SPLITS)
MAKEPREPOSTPARVAR(Stripes, stripes, STRIPES)
MAKEPREPOSTPARVAR(Wedge, wedge, WEDGE)
MAKEPREPOSTPARVAR(WedgeJulia, wedge_julia, WEDGE_JULIA)
MAKEPREPOSTPARVAR(WedgeSph, wedge_sph, WEDGE_SPH)
MAKEPREPOSTPARVAR(Whorl, whorl, WHORL)
MAKEPREPOSTPARVAR(Waves2, waves2, WAVES2)
MAKEPREPOSTVAR(Exp, exp, EXP)
MAKEPREPOSTVAR(Exp2, exp2, EXP2)
MAKEPREPOSTPARVAR(Log, log, LOG)
MAKEPREPOSTVAR(Sin, sin, SIN)
MAKEPREPOSTVAR(Cos, cos, COS)
MAKEPREPOSTVAR(Tan, tan, TAN)
MAKEPREPOSTVAR(Sec, sec, SEC)
MAKEPREPOSTVAR(Csc, csc, CSC)
MAKEPREPOSTVAR(Cot, cot, COT)
MAKEPREPOSTVAR(Sinh, sinh, SINH)
MAKEPREPOSTVAR(Cosh, cosh, COSH)
MAKEPREPOSTVAR(Tanh, tanh, TANH)
MAKEPREPOSTPARVAR(TanhSpiral, tanh_spiral, TANH_SPIRAL)
MAKEPREPOSTVAR(Sech, sech, SECH)
MAKEPREPOSTVAR(Csch, csch, CSCH)
MAKEPREPOSTVAR(Coth, coth, COTH)
MAKEPREPOSTPARVAR(Auger, auger, AUGER)
MAKEPREPOSTPARVAR(Flux, flux, FLUX)
}
