#pragma once

#include "Variation.h"

namespace EmberNs
{
/// <summary>
/// Hemisphere.
/// </summary>
template <typename T>
class HemisphereVariation : public Variation<T>
{
public:
	HemisphereVariation(T weight = 1.0) : Variation<T>("hemisphere", eVariationId::VAR_HEMISPHERE, weight, true) { }

	VARCOPY(HemisphereVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t = m_Weight / std::sqrt(helper.m_PrecalcSumSquares + 1);
		helper.Out.x = helper.In.x * t;
		helper.Out.y = helper.In.y * t;
		helper.Out.z = t;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t t = " << weight << " / sqrt(precalcSumSquares + (real_t)(1.0));\n"
		   << "\n"
		   << "\t\tvOut.x = vIn.x * t;\n"
		   << "\t\tvOut.y = vIn.y * t;\n"
		   << "\t\tvOut.z = t;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Epispiral.
/// </summary>
template <typename T>
class EpispiralVariation : public ParametricVariation<T>
{
public:
	EpispiralVariation(T weight = 1.0) : ParametricVariation<T>("epispiral", eVariationId::VAR_EPISPIRAL, weight, false, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(EpispiralVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T theta = helper.m_PrecalcAtanyx;
		T t = (!m_ThicknessWeight ? m_Weight : m_ThicknessWeight * rand.Frand01<T>()) / std::cos(m_N * theta) - m_HolesWeight;

		if (std::abs(t) != 0)
		{
			helper.Out.x = t * std::cos(theta);
			helper.Out.y = t * std::sin(theta);
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
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index           = ss2.str();
		string weight          = WeightDefineString();
		string n               = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thickness       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string holes           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string thicknessweight = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string holesweight     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t theta = precalcAtanyx;\n"
		   << "\t\treal_t t = (" << thicknessweight << " == (real_t)(0.0) ? " << weight << " : MwcNext01(mwc) * " << thicknessweight << ") / cos(" << n << " * theta) - " << holesweight << ";\n"
		   << "\n"
		   << "\t\tif (fabs(t) != 0)\n"
		   << "\t\t{\n"
		   << "\t\t\tvOut.x = t * cos(theta);\n"
		   << "\t\t\tvOut.y = t * sin(theta);\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t\tvOut.x = 0;\n"
		   << "\t\t\tvOut.y = 0;\n"
		   << "\t\t}\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_ThicknessWeight = m_Thickness * m_Weight;
		m_HolesWeight = m_Holes * m_Weight;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_N, prefix + "epispiral_n", 6));
		m_Params.push_back(ParamWithName<T>(&m_Thickness, prefix + "epispiral_thickness"));
		m_Params.push_back(ParamWithName<T>(&m_Holes, prefix + "epispiral_holes", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_ThicknessWeight, prefix + "epispiral_thickness_weight"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_HolesWeight, prefix + "epispiral_holes_weight"));
	}

private:
	T m_N;
	T m_Thickness;
	T m_Holes;
	T m_ThicknessWeight;//Precalc.
	T m_HolesWeight;
};

/// <summary>
/// Bwraps.
/// Note, this is the same as bwraps7, which is the same as bwraps2 except for the precalc function.
/// </summary>
template <typename T>
class BwrapsVariation : public ParametricVariation<T>
{
public:
	BwrapsVariation(T weight = 1.0) : ParametricVariation<T>("bwraps", eVariationId::VAR_BWRAPS, weight)
	{
		Init();
	}

	PARVARCOPY(BwrapsVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_BwrapsCellsize == 0)
		{
			helper.Out.x = m_Weight * helper.In.x;
			helper.Out.y = m_Weight * helper.In.y;
		}
		else
		{
			T vx = helper.In.x;
			T vy = helper.In.y;
			T cx = (Floor<T>(vx / m_BwrapsCellsize) + T(0.5)) * m_BwrapsCellsize;
			T cy = (Floor<T>(vy / m_BwrapsCellsize) + T(0.5)) * m_BwrapsCellsize;
			T lx = vx - cx;
			T ly = vy - cy;

			if ((SQR(lx) + SQR(ly)) > m_R2)
			{
				helper.Out.x = m_Weight * helper.In.x;
				helper.Out.y = m_Weight * helper.In.y;
			}
			else
			{
				lx *= m_G2;
				ly *= m_G2;
				T r = m_Rfactor / Zeps((SQR(lx) + SQR(ly)) / 4 + 1);
				lx *= r;
				ly *= r;
				r = (SQR(lx) + SQR(ly)) / m_R2;
				T theta = m_BwrapsInnerTwist * (1 - r) + m_BwrapsOuterTwist * r;
				T s = std::sin(theta);
				T c = std::cos(theta);
				vx = cx + c * lx + s * ly;
				vy = cy - s * lx + c * ly;
				helper.Out.x = m_Weight * vx;
				helper.Out.y = m_Weight * vy;
			}
		}

		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index            = ss2.str();
		string weight           = WeightDefineString();
		string bwrapsCellsize   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bwrapsSpace      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bwrapsGain       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bwrapsInnerTwist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bwrapsOuterTwist = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string g2               = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string r2               = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rfactor          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tif (" << bwrapsCellsize << " == 0)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t	vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real_t vx = vIn.x;\n"
		   << "\t\t	real_t vy = vIn.y;\n"
		   << "\t\t	real_t cx = (floor(vx / " << bwrapsCellsize << ") + (real_t)(0.5)) * " << bwrapsCellsize << ";\n"
		   << "\t\t	real_t cy = (floor(vy / " << bwrapsCellsize << ") + (real_t)(0.5)) * " << bwrapsCellsize << ";\n"
		   << "\t\t	real_t lx = vx - cx;\n"
		   << "\t\t	real_t ly = vy - cy;\n"
		   << "\n"
		   << "\t\t	if ((SQR(lx) + SQR(ly)) > " << r2 << ")\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t		vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		lx *= " << g2 << ";\n"
		   << "\t\t		ly *= " << g2 << ";\n"
		   << "\n"
		   << "\t\t		real_t r = " << rfactor << " / Zeps(fma(lx, lx, SQR(ly)) / 4 + 1);\n"
		   << "\n"
		   << "\t\t		lx *= r;\n"
		   << "\t\t		ly *= r;\n"
		   << "\t\t		r = fma(lx, lx, SQR(ly)) / " << r2 << ";\n"
		   << "\n"
		   << "\t\t		real_t theta = fma(" << bwrapsInnerTwist << ", (1 - r), " << bwrapsOuterTwist << " * r);\n"
		   << "\t\t		real_t s = sin(theta);\n"
		   << "\t\t		real_t c = cos(theta);\n"
		   << "\n"
		   << "\t\t		vx = fma(s, ly, fma(c, lx, cx));\n"
		   << "\t\t		vy = cy - s * lx + c * ly;\n"
		   << "\n"
		   << "\t\t		vOut.x = " << weight << " * vx;\n"
		   << "\t\t		vOut.y = " << weight << " * vy;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T radius = T(0.5) * (m_BwrapsCellsize / (1 + SQR(m_BwrapsSpace)));
		m_G2 = SQR(m_BwrapsGain) + T(1.0e-6);
		T maxBubble = m_G2 * radius;

		if (maxBubble > 2)
			maxBubble = 1;
		else
			maxBubble *= (1 / (SQR(maxBubble) / 4 + 1));

		m_R2 = Zeps(SQR(radius));
		m_Rfactor = radius / maxBubble;
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
		m_Params.push_back(ParamWithName<T>(&m_BwrapsCellsize, prefix + "bwraps_cellsize", 1));
		m_Params.push_back(ParamWithName<T>(&m_BwrapsSpace, prefix + "bwraps_space"));
		m_Params.push_back(ParamWithName<T>(&m_BwrapsGain, prefix + "bwraps_gain", 1));
		m_Params.push_back(ParamWithName<T>(&m_BwrapsInnerTwist, prefix + "bwraps_inner_twist"));
		m_Params.push_back(ParamWithName<T>(&m_BwrapsOuterTwist, prefix + "bwraps_outer_twist"));
		m_Params.push_back(ParamWithName<T>(true, &m_G2, prefix + "bwraps_g2"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_R2, prefix + "bwraps_r2"));
		m_Params.push_back(ParamWithName<T>(true, &m_Rfactor, prefix + "bwraps_rfactor"));
	}

private:
	T m_BwrapsCellsize;
	T m_BwrapsSpace;
	T m_BwrapsGain;
	T m_BwrapsInnerTwist;
	T m_BwrapsOuterTwist;
	T m_G2;//Precalc.
	T m_R2;
	T m_Rfactor;
};

/// <summary>
/// bwraps_rand.
/// By tatasz.
/// </summary>
template <typename T>
class BwrapsRandVariation : public ParametricVariation<T>
{
public:
	BwrapsRandVariation(T weight = 1.0) : ParametricVariation<T>("bwraps_rand", eVariationId::VAR_BWRAPS_RAND, weight)
	{
		Init();
	}

	PARVARCOPY(BwrapsRandVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_CellSize == 0)
		{
			helper.Out.x = helper.In.x * m_Weight;
			helper.Out.y = helper.In.y * m_Weight;
		}
		else
		{
			T Cx = (Floor<T>(helper.In.x * m_InvCellSize) + T(0.5)) * m_CellSize;
			T Cy = (Floor<T>(helper.In.y * m_InvCellSize) + T(0.5)) * m_CellSize;
			T Lx = helper.In.x - Cx;
			T Ly = helper.In.y - Cy;
			T radius;

			if (m_Symm == 0)
				radius = m_HalfCellSizeOver1pSpaceSq * VarFuncs<T>::HashShadertoy(Cx, Cy, m_Seed);
			else
				radius = m_HalfCellSizeOver1pSpaceSq * VarFuncs<T>::HashShadertoy(SQR(Cx), SQR(Cy), m_Seed);

			T mb = m_G2 * radius;
			T max_bubble;

			if (mb > 2)
				max_bubble = 1;
			else
				max_bubble = mb / (mb * mb * T(0.25) + 1);

			T r2 = SQR(radius);

			if (SQR(Lx) + SQR(Ly) > r2)
			{
				helper.Out.x = helper.In.x * m_Weight;
				helper.Out.y = helper.In.y * m_Weight;
			}
			else
			{
				T rfactor = radius / Zeps(max_bubble);
				T Lx2 = Lx * m_G2;
				T Ly2 = Ly * m_G2;
				T r = rfactor / ((SQR(Lx2) + SQR(Ly2)) * T(0.25) + 1);
				T Lx3 = Lx2 * r;
				T Ly3 = Ly2 * r;
				T r_2 = (SQR(Lx3) + SQR(Ly3)) / Zeps(r2);
				T theta = m_InnerTwist * (1 - r_2) + m_OuterTwist * r_2;
				T ct = std::cos(theta);
				T st = std::sin(theta);
				helper.Out.x = (Cx + ct * Lx3 + st * Ly3) * m_Weight;
				helper.Out.y = (Cy - st * Lx3 + ct * Ly3) * m_Weight;
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
		string cellsize                  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string space                     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string gain                      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string innertwist                = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string outertwist                = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string symm                      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed                      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invcellsize               = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string g2                        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spacesq                   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfcellsizeover1pspacesq = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tif (" << cellsize << " == 0)\n"
		   << "\t\t{\n"
		   << "\t\t    vOut.x = vIn.x * " << weight << ";\n"
		   << "\t\t    vOut.y = vIn.y * " << weight << ";\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t    real_t Cx = (floor(vIn.x * " << invcellsize << ") + 0.5) * " << cellsize << ";\n"
		   << "\t\t    real_t Cy = (floor(vIn.y * " << invcellsize << ") + 0.5) * " << cellsize << ";\n"
		   << "\t\t    real_t Lx = vIn.x - Cx;\n"
		   << "\t\t    real_t Ly = vIn.y - Cy;\n"
		   << "\t\t    real_t radius;\n"
		   << "\n"
		   << "\t\t    if (" << symm << " == 0)\n"
		   << "\t\t        radius = " << halfcellsizeover1pspacesq << " * HashShadertoy(Cx, Cy, " << seed << ");\n"
		   << "\t\t    else\n"
		   << "\t\t        radius = " << halfcellsizeover1pspacesq << " * HashShadertoy(SQR(Cx), SQR(Cy), " << seed << ");\n"
		   << "\n"
		   << "\t\t    real_t mb = " << g2 << " * radius;\n"
		   << "\t\t    real_t max_bubble;\n"
		   << "\n"
		   << "\t\t    if (mb > 2)\n"
		   << "\t\t        max_bubble = 1;\n"
		   << "\t\t    else\n"
		   << "\t\t        max_bubble = mb / fma(SQR(mb), (real_t)(0.25), (real_t)(1.0));\n"
		   << "\n"
		   << "\t\t    real_t r2 = SQR(radius);\n"
		   << "\n"
		   << "\t\t    if (SQR(Lx) + SQR(Ly) > r2)\n"
		   << "\t\t    {\n"
		   << "\t\t        vOut.x = vIn.x * " << weight << ";\n"
		   << "\t\t        vOut.y = vIn.y * " << weight << ";\n"
		   << "\t\t    }\n"
		   << "\t\t    else\n"
		   << "\t\t    {\n"
		   << "\t\t        real_t rfactor = radius / Zeps(max_bubble);\n"
		   << "\t\t        real_t Lx2 = Lx * " << g2 << ";\n"
		   << "\t\t        real_t Ly2 = Ly * " << g2 << ";\n"
		   << "\t\t        real_t r = rfactor / fma(fma(Lx2, Lx2, SQR(Ly2)), (real_t)(0.25), (real_t)(1.0));\n"
		   << "\t\t        real_t Lx3 = Lx2 * r;\n"
		   << "\t\t        real_t Ly3 = Ly2 * r;\n"
		   << "\t\t        real_t r_2 = fma(Lx3, Lx3, SQR(Ly3)) / Zeps(r2);\n"
		   << "\t\t        real_t theta = " << innertwist << " * (1.0 - r_2) + " << outertwist << " * r_2;\n"
		   << "\t\t        real_t ct = cos(theta);\n"
		   << "\t\t        real_t st = sin(theta);\n"
		   << "\n"
		   << "\t\t        vOut.x = (Cx + ct * Lx3 + st * Ly3) * " << weight << ";\n"
		   << "\t\t        vOut.y = (Cy - st * Lx3 + ct * Ly3) * " << weight << ";\n"
		   << "\t\t    }\n"
		   << "\t\t}\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_G2 = SQR(m_Gain);
		m_InvCellSize = 1 / Zeps(m_CellSize);
		m_SpaceSq = SQR(m_Space);
		m_HalfCellSizeOver1pSpaceSq = T(0.5) * (m_CellSize / (1 + m_SpaceSq));
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps", "Fract", "HashShadertoy" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_CellSize,                        prefix + "bwraps_rand_cellsize", 1));
		m_Params.push_back(ParamWithName<T>(&m_Space,                           prefix + "bwraps_rand_space"));
		m_Params.push_back(ParamWithName<T>(&m_Gain,                            prefix + "bwraps_rand_gain", 2));
		m_Params.push_back(ParamWithName<T>(&m_InnerTwist,                      prefix + "bwraps_rand_inner_twist"));
		m_Params.push_back(ParamWithName<T>(&m_OuterTwist,                      prefix + "bwraps_rand_outer_twist"));
		m_Params.push_back(ParamWithName<T>(&m_Symm,                            prefix + "bwraps_rand_symm"));
		m_Params.push_back(ParamWithName<T>(&m_Seed,                            prefix + "bwraps_rand_seed", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_InvCellSize,               prefix + "bwraps_rand_inv_cellsize"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_G2,                        prefix + "bwraps_rand_g2"));
		m_Params.push_back(ParamWithName<T>(true, &m_SpaceSq,                   prefix + "bwraps_rand_space_sq"));
		m_Params.push_back(ParamWithName<T>(true, &m_HalfCellSizeOver1pSpaceSq, prefix + "bwraps_rand_half_cellsize_over_1_plus_space_sq"));
	}

private:
	T m_CellSize;
	T m_Space;
	T m_Gain;
	T m_InnerTwist;
	T m_OuterTwist;
	T m_Symm;
	T m_Seed;
	T m_InvCellSize;//Precalc.
	T m_G2;
	T m_SpaceSq;
	T m_HalfCellSizeOver1pSpaceSq;
};

/// <summary>
/// BlurCircle.
/// </summary>
template <typename T>
class BlurCircleVariation : public Variation<T>
{
public:
	BlurCircleVariation(T weight = 1.0) : Variation<T>("blur_circle", eVariationId::VAR_BLUR_CIRCLE, weight) { }

	VARCOPY(BlurCircleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = 2 * rand.Frand01<T>() - 1;
		T y = 2 * rand.Frand01<T>() - 1;
		T absx = x;
		T absy = y;
		T side, perimeter;

		if (absx < 0)
			absx = absx * -1;

		if (absy < 0)
			absy = absy * -1;

		if (absx >= absy)
		{
			if (x >= absy)
				perimeter = absx + y;
			else
				perimeter = 5 * absx - y;

			side = absx;
		}
		else
		{
			if (y >= absx)
				perimeter = 3 * absy - x;
			else
				perimeter = 7 * absy + x;

			side = absy;
		}

		T r = m_Weight * side;
		T val = T(M_PI_4) * perimeter / side - T(M_PI_4);
		T sina = std::sin(val);
		T cosa = std::cos(val);
		helper.Out.x = r * cosa;
		helper.Out.y = r * sina;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		string weight = WeightDefineString();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		ss << "\t{\n"
		   << "\t\treal_t x = fma((real_t)(2.0), MwcNext01(mwc), (real_t)(-1.0));\n"
		   << "\t\treal_t y = fma((real_t)(2.0), MwcNext01(mwc), (real_t)(-1.0));\n"
		   << "\t\treal_t absx = x;\n"
		   << "\t\treal_t absy = y;\n"
		   << "\t\treal_t side, perimeter;\n"
		   << "\t\t\n"
		   << "\t\tif (absx < 0)\n"
		   << "\t\t	absx = absx * -1;\n"
		   << "\n"
		   << "\t\tif (absy < 0)\n"
		   << "\t\t	absy = absy * -1;\n"
		   << "\n"
		   << "\t\tif (absx >= absy)\n"
		   << "\t\t{\n"
		   << "\t\t	if (x >= absy)\n"
		   << "\t\t		perimeter = absx + y;\n"
		   << "\t\t	else\n"
		   << "\t\t		perimeter = fma((real_t)(5.0), absx, -y);\n"
		   << "\n"
		   << "\t\t	side = absx;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (y >= absx)\n"
		   << "\t\t		perimeter = fma((real_t)(3.0), absy, -x);\n"
		   << "\t\t	else\n"
		   << "\t\t		perimeter = fma((real_t)(7.0), absy, x);\n"
		   << "\n"
		   << "\t\t	side = absy;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t r = " << weight << " * side;\n"
		   << "\t\treal_t val = MPI4 * perimeter / side - MPI4;\n"
		   << "\t\treal_t sina = sin(val);\n"
		   << "\t\treal_t cosa = cos(val);\n"
		   << "\n"
		   << "\t\tvOut.x = r * cosa;\n"
		   << "\t\tvOut.y = r * sina;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// BlurZoom.
/// </summary>
template <typename T>
class BlurZoomVariation : public ParametricVariation<T>
{
public:
	BlurZoomVariation(T weight = 1.0) : ParametricVariation<T>("blur_zoom", eVariationId::VAR_BLUR_ZOOM, weight)
	{
		Init();
	}

	PARVARCOPY(BlurZoomVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T z = 1 + m_BlurZoomLength * rand.Frand01<T>();
		helper.Out.x = m_Weight * ((helper.In.x - m_BlurZoomX) * z + m_BlurZoomX);
		helper.Out.y = m_Weight * ((helper.In.y + m_BlurZoomY) * z - m_BlurZoomY);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string blurZoomLength = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurZoomX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurZoomY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t z = fma(" << blurZoomLength << ", MwcNext01(mwc), 1);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma((vIn.x - " << blurZoomX << "), z, " << blurZoomX << ");\n"
		   << "\t\tvOut.y = " << weight << " * fma((vIn.y + " << blurZoomY << "), z, -" << blurZoomY << ");\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_BlurZoomLength, prefix + "blur_zoom_length"));
		m_Params.push_back(ParamWithName<T>(&m_BlurZoomX, prefix + "blur_zoom_x"));
		m_Params.push_back(ParamWithName<T>(&m_BlurZoomY, prefix + "blur_zoom_y"));
	}

private:
	T m_BlurZoomLength;
	T m_BlurZoomX;
	T m_BlurZoomY;
};

/// <summary>
/// BlurPixelize.
/// </summary>
template <typename T>
class BlurPixelizeVariation : public ParametricVariation<T>
{
public:
	BlurPixelizeVariation(T weight = 1.0) : ParametricVariation<T>("blur_pixelize", eVariationId::VAR_BLUR_PIXELIZE, weight)
	{
		Init();
	}

	PARVARCOPY(BlurPixelizeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = T(Floor<T>(helper.In.x * m_InvSize));
		T y = T(Floor<T>(helper.In.y * m_InvSize));
		helper.Out.x = m_V * (x + m_BlurPixelizeScale * (rand.Frand01<T>() - T(0.5)) + T(0.5));
		helper.Out.y = m_V * (y + m_BlurPixelizeScale * (rand.Frand01<T>() - T(0.5)) + T(0.5));
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string blurPixelizeSize = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurPixelizeScale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string v = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invSize = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x = floor(vIn.x * " << invSize << ");\n"
		   << "\t\treal_t y = floor(vIn.y * " << invSize << ");\n"
		   << "\n"
		   << "\t\tvOut.x = " << v << " * fma(" << blurPixelizeScale << ", (MwcNext01(mwc) - (real_t)(0.5)), x + (real_t)(0.5));\n"
		   << "\t\tvOut.y = " << v << " * fma(" << blurPixelizeScale << ", (MwcNext01(mwc) - (real_t)(0.5)), y + (real_t)(0.5));\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		auto zsize = Zeps(m_BlurPixelizeSize);
		m_V = m_Weight * zsize;
		m_InvSize = 1 / zsize;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_BlurPixelizeSize, prefix + "blur_pixelize_size", T(0.1), eParamType::REAL, EPS));
		m_Params.push_back(ParamWithName<T>(&m_BlurPixelizeScale, prefix + "blur_pixelize_scale", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_V, prefix + "blur_pixelize_v"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_InvSize, prefix + "blur_pixelize_inv_size"));
	}

private:
	T m_BlurPixelizeSize;
	T m_BlurPixelizeScale;
	T m_V;//Precalc.
	T m_InvSize;
};

/// <summary>
/// Crop.
/// </summary>
template <typename T>
class CropVariation : public ParametricVariation<T>
{
public:
	CropVariation(T weight = 1.0) : ParametricVariation<T>("crop", eVariationId::VAR_CROP, weight)
	{
		Init();
	}

	PARVARCOPY(CropVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = helper.In.x;
		T y = helper.In.y;

		if (((x < m_X0_) || (x > m_X1_) || (y < m_Y0_) || (y > m_Y1_)) && m_Z != 0)
		{
			x = 0;
			y = 0;
		}
		else
		{
			if (x < m_X0_)
				x = m_X0_ + rand.Frand01<T>() * m_W;
			else if (x > m_X1_)
				x = m_X1_ - rand.Frand01<T>() * m_W;

			if (y < m_Y0_)
				y = m_Y0_ + rand.Frand01<T>() * m_H;
			else if (y > m_Y1_)
				y = m_Y1_ - rand.Frand01<T>() * m_H;
		}

		helper.Out.x = m_Weight * x;
		helper.Out.y = m_Weight * y;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string x0 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y0 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x0_ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y0_ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x1_ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y1_ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string w = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string h = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x = vIn.x;\n"
		   << "\t\treal_t y = vIn.y;\n"
		   << "\n"
		   << "\t\tif (((x < " << x0_ << ") || (x > " << x1_ << ") || (y < " << y0_ << ") || (y > " << y1_ << ")) && " << z << " != 0)\n"
		   << "\t\t{\n"
		   << "\t\t	x = 0;\n"
		   << "\t\t	y = 0;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (x < " << x0_ << ")\n"
		   << "\t\t		x = fma(MwcNext01(mwc), " << w << ", " << x0_ << ");\n"
		   << "\t\t	else if (x > " << x1_ << ")\n"
		   << "\t\t		x = " << x1_ << " - MwcNext01(mwc) * " << w << ";\n"
		   << "\t\t\n"
		   << "\t\t	if (y < " << y0_ << ")\n"
		   << "\t\t		y = fma(MwcNext01(mwc), " << h << ", " << y0_ << ");\n"
		   << "\t\t	else if (y > " << y1_ << ")\n"
		   << "\t\t		y = " << y1_ << " - MwcNext01(mwc) * " << h << ";\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * x;\n"
		   << "\t\tvOut.y = " << weight << " * y;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		if (m_X0 < m_X1)
		{
			m_X0_ = m_X0;
			m_X1_ = m_X1;
		}
		else
		{
			m_X0_ = m_X1;
			m_X1_ = m_X0;
		}

		if (m_Y0 < m_Y1)
		{
			m_Y0_ = m_Y0;
			m_Y1_ = m_Y1;
		}
		else
		{
			m_Y0_ = m_Y1;
			m_Y1_ = m_Y0;
		}

		m_W = (m_X1_ - m_X0_) * T(0.5) * m_S;
		m_H = (m_Y1_ - m_Y0_) * T(0.5) * m_S;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X0, prefix + "crop_left", -1));
		m_Params.push_back(ParamWithName<T>(&m_Y0, prefix + "crop_top", -1));
		m_Params.push_back(ParamWithName<T>(&m_X1, prefix + "crop_right", 1));
		m_Params.push_back(ParamWithName<T>(&m_Y1, prefix + "crop_bottom", 1));
		m_Params.push_back(ParamWithName<T>(&m_S, prefix + "crop_scatter_area", 0, eParamType::REAL));
		m_Params.push_back(ParamWithName<T>(&m_Z, prefix + "crop_zero", 0, eParamType::INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_X0_, prefix + "crop_x0_"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Y0_, prefix + "crop_y0_"));
		m_Params.push_back(ParamWithName<T>(true, &m_X1_, prefix + "crop_x1_"));
		m_Params.push_back(ParamWithName<T>(true, &m_Y1_, prefix + "crop_y1_"));
		m_Params.push_back(ParamWithName<T>(true, &m_W, prefix + "crop_w"));
		m_Params.push_back(ParamWithName<T>(true, &m_H, prefix + "crop_h"));
	}

private:
	T m_X0;
	T m_Y0;
	T m_X1;
	T m_Y1;
	T m_S;
	T m_Z;
	T m_X0_;//Precalc.
	T m_Y0_;
	T m_X1_;
	T m_Y1_;
	T m_W;
	T m_H;
};

/// <summary>
/// BCircle.
/// </summary>
template <typename T>
class BCircleVariation : public ParametricVariation<T>
{
public:
	BCircleVariation(T weight = 1.0) : ParametricVariation<T>("bcircle", eVariationId::VAR_BCIRCLE, weight)
	{
		Init();
	}

	PARVARCOPY(BCircleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = helper.In.x * m_Scale;
		T y = helper.In.y * m_Scale;
		T r = std::sqrt(SQR(x) + SQR(y));

		if (r <= 1)
		{
			helper.Out.x = m_Weight * x;
			helper.Out.y = m_Weight * y;
		}
		else
		{
			if (m_Bcbw != 0)
			{
				T ang = std::atan2(y, x);
				T omega = (m_BcbwFifth * rand.Frand01<T>()) + 1;
				T temp = m_Weight * omega;
				helper.Out.x = temp * std::cos(ang);
				helper.Out.y = temp * std::sin(ang);
			}
			else
			{
				helper.Out.x = 0;
				helper.Out.y = 0;
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
		string scale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string borderWidth = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bcbw = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bcbwfifth = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t weight = " << weight << ";\n"
		   << "\t\treal_t scale = " << scale << ";\n"
		   << "\t\treal_t x = vIn.x * scale;\n"
		   << "\t\treal_t y = vIn.y * scale;\n"
		   << "\t\treal_t r = sqrt(fma(x, x, SQR(y)));\n"
		   << "\n"
		   << "\t\tif (r <= 1)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = weight * x;\n"
		   << "\t\t	vOut.y = weight * y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (" << bcbw << " != 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		real_t ang = atan2(y, x);\n"
		   << "\t\t		real_t omega = fma(" << bcbwfifth << ", MwcNext01(mwc), (real_t)(1.0));\n"
		   << "\t\t		real_t temp = weight * omega;\n"
		   << "\t\t		vOut.x = temp * cos(ang);\n"
		   << "\t\t		vOut.y = temp * sin(ang);\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = (real_t)0.0;\n"
		   << "\t\t		vOut.y = (real_t)0.0;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Bcbw = std::abs(m_BorderWidth);
		m_BcbwFifth = T(0.2) * m_Bcbw;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Scale, prefix + "bcircle_scale", 1));
		m_Params.push_back(ParamWithName<T>(&m_BorderWidth, prefix + "bcircle_borderwidth"));
		m_Params.push_back(ParamWithName<T>(true, &m_Bcbw, prefix + "bcircle_bcbw"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_BcbwFifth, prefix + "bcircle_bcbw_fifth"));//Precalc.
	}

private:
	T m_Scale;
	T m_BorderWidth;
	T m_Bcbw;//Precalc.
	T m_BcbwFifth;
};

/// <summary>
/// BlurLinear.
/// </summary>
template <typename T>
class BlurLinearVariation : public ParametricVariation<T>
{
public:
	BlurLinearVariation(T weight = 1.0) : ParametricVariation<T>("blur_linear", eVariationId::VAR_BLUR_LINEAR, weight)
	{
		Init();
	}

	PARVARCOPY(BlurLinearVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_BlurLinearLength * rand.Frand01<T>();
		helper.Out.x = m_Weight * (helper.In.x + r * m_C);
		helper.Out.y = m_Weight * (helper.In.y + r * m_S);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string blurLinearLength = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurLinearAngle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = " << blurLinearLength << " * MwcNext01(mwc);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(r, " << c << ", vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * fma(r, " << s << ", vIn.y);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		sincos(m_BlurLinearAngle, &m_S, &m_C);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_BlurLinearLength, prefix + "blur_linear_length"));
		m_Params.push_back(ParamWithName<T>(&m_BlurLinearAngle, prefix + "blur_linear_angle", 0, eParamType::REAL_CYCLIC, 0, T(M_2PI)));
		m_Params.push_back(ParamWithName<T>(true, &m_S, prefix + "blur_linear_s"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_C, prefix + "blur_linear_c"));
	}

private:
	T m_BlurLinearLength;
	T m_BlurLinearAngle;
	T m_S;//Precalc.
	T m_C;
};

/// <summary>
/// BlurSquare.
/// </summary>
template <typename T>
class BlurSquareVariation : public ParametricVariation<T>
{
public:
	BlurSquareVariation(T weight = 1.0) : ParametricVariation<T>("blur_square", eVariationId::VAR_BLUR_SQUARE, weight)
	{
		Init();
	}

	PARVARCOPY(BlurSquareVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_V * (rand.Frand01<T>() - T(0.5));
		helper.Out.y = m_V * (rand.Frand01<T>() - T(0.5));
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string v = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.
		ss << "\t{\n"
		   << "\t\tvOut.x = " << v << " * (MwcNext01(mwc) - (real_t)(0.5));\n"
		   << "\t\tvOut.y = " << v << " * (MwcNext01(mwc) - (real_t)(0.5));\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_V = m_Weight * 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_V, prefix + "blur_square_v"));//Precalcs only, no params.
	}

private:
	T m_V;
};

/// <summary>
/// Flatten.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class FlattenVariation : public Variation<T>
{
public:
	FlattenVariation(T weight = 1.0) : Variation<T>("flatten", eVariationId::VAR_FLATTEN, weight) { }

	VARCOPY(FlattenVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_VarType == eVariationType::VARTYPE_REG)//Rare and different usage of in/out.
		{
			helper.Out.x = helper.Out.y = helper.Out.z = 0;
			outPoint.m_Z = 0;
		}
		else
		{
			helper.Out.x = helper.In.x;
			helper.Out.y = helper.In.y;
			helper.Out.z = 0;
		}
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;

		if (m_VarType == eVariationType::VARTYPE_REG)
		{
			ss << "\t{\n"
			   << "\t\tvOut.x = 0;\n"
			   << "\t\tvOut.y = 0;\n"
			   << "\t\tvOut.z = 0;\n"
			   << "\t\toutPoint->m_Z = 0;\n"
			   << "\t}\n";
		}
		else
		{
			ss << "\t{\n"
			   << "\t\tvOut.x = vIn.x;\n"
			   << "\t\tvOut.y = vIn.y;\n"
			   << "\t\tvOut.z = 0;\n"
			   << "\t}\n";
		}

		return ss.str();
	}
};

/// <summary>
/// Zblur.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class ZblurVariation : public Variation<T>
{
public:
	ZblurVariation(T weight = 1.0) : Variation<T>("zblur", eVariationId::VAR_ZBLUR, weight) { }

	VARCOPY(ZblurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = helper.Out.y = 0;
		helper.Out.z = m_Weight * (rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() - 2);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = vOut.y = 0;\n"
		   << "\t\tvOut.z = " << weight << " * (MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) - (real_t)(2.0));\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// ZScale.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class ZScaleVariation : public Variation<T>
{
public:
	ZScaleVariation(T weight = 1.0) : Variation<T>("zscale", eVariationId::VAR_ZSCALE, weight) { }

	VARCOPY(ZScaleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = helper.Out.y = 0;
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = vOut.y = 0;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// ZTranslate.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class ZTranslateVariation : public Variation<T>
{
public:
	ZTranslateVariation(T weight = 1.0) : Variation<T>("ztranslate", eVariationId::VAR_ZTRANSLATE, weight) { }

	VARCOPY(ZTranslateVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = helper.Out.y = 0;
		helper.Out.z = m_Weight;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = vOut.y = 0;\n"
		   << "\t\tvOut.z = " << weight << ";\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// zcone.
/// This uses in/out in a rare and different way.
/// </summary>
template <typename T>
class ZConeVariation : public Variation<T>
{
public:
	ZConeVariation(T weight = 1.0) : Variation<T>("zcone", eVariationId::VAR_ZCONE, weight, true, true) { }

	VARCOPY(ZConeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_VarType == eVariationType::VARTYPE_REG)//Rare and different usage of in/out.
		{
			helper.Out.x = helper.Out.y = 0;
		}
		else
		{
			helper.Out.x = helper.In.x;
			helper.Out.y = helper.In.y;
		}

		helper.Out.z = m_Weight * helper.m_PrecalcSqrtSumSquares;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n";

		if (m_VarType == eVariationType::VARTYPE_REG)
		{
			ss << "\t\tvOut.x = vOut.y = 0;\n";
		}
		else
		{
			ss << "\t\tvOut.x = vIn.x;\n"
			   << "\t\tvOut.y = vIn.y;\n";
		}

		ss << "\t\tvOut.z = " << weight << " * precalcSqrtSumSquares;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Blur3D.
/// </summary>
template <typename T>
class Blur3DVariation : public Variation<T>
{
public:
	Blur3DVariation(T weight = 1.0) : Variation<T>("blur3D", eVariationId::VAR_BLUR3D, weight) { }

	VARCOPY(Blur3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = rand.Frand01<T>() * M_2PI;
		T r = m_Weight * (rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() + rand.Frand01<T>() - 2);
		T angle2 = rand.Frand01<T>() * T(M_PI);
		T sina = std::sin(angle);
		T cosa = std::cos(angle);
		T sinb = std::sin(angle2);
		T cosb = std::cos(angle2);
		helper.Out.x = r * sinb * cosa;
		helper.Out.y = r * sinb * sina;
		helper.Out.z = r * cosb;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t angle = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t r = " << weight << " * (MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) + MwcNext01(mwc) - (real_t)(2.0));\n"
		   << "\t\treal_t angle2 = MwcNext01(mwc) * MPI;\n"
		   << "\t\treal_t sina = sin(angle);\n"
		   << "\t\treal_t cosa = cos(angle);\n"
		   << "\t\treal_t sinb = sin(angle2);\n"
		   << "\t\treal_t cosb = cos(angle2);\n"
		   << "\n"
		   << "\t\tvOut.x = r * sinb * cosa;\n"
		   << "\t\tvOut.y = r * sinb * sina;\n"
		   << "\t\tvOut.z = r * cosb;\n"
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Spherical3D.
/// </summary>
template <typename T>
class Spherical3DVariation : public Variation<T>
{
public:
	Spherical3DVariation(T weight = 1.0) : Variation<T>("Spherical3D", eVariationId::VAR_SPHERICAL3D, weight, true) { }

	VARCOPY(Spherical3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = m_Weight / Zeps(helper.m_PrecalcSumSquares + SQR(helper.In.z));
		helper.Out.x = r2 * helper.In.x;
		helper.Out.y = r2 * helper.In.y;
		helper.Out.z = r2 * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r2 = " << weight << " / Zeps(fma(vIn.z, vIn.z, precalcSumSquares));\n"
		   << "\n"
		   << "\t\tvOut.x = r2 * vIn.x;\n"
		   << "\t\tvOut.y = r2 * vIn.y;\n"
		   << "\t\tvOut.z = r2 * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}
};

/// <summary>
/// Curl3D.
/// </summary>
template <typename T>
class Curl3DVariation : public ParametricVariation<T>
{
public:
	Curl3DVariation(T weight = 1.0) : ParametricVariation<T>("curl3D", eVariationId::VAR_CURL3D, weight, true)
	{
		Init();
	}

	PARVARCOPY(Curl3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = helper.m_PrecalcSumSquares + SQR(helper.In.z);
		T r = m_Weight / Zeps(r2 * m_C2 + m_C2x * helper.In.x - m_C2y * helper.In.y + m_C2z * helper.In.z + 1);
		helper.Out.x = r * (helper.In.x + m_Cx * r2);
		helper.Out.y = r * (helper.In.y - m_Cy * r2);
		helper.Out.z = r * (helper.In.z + m_Cz * r2);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string cx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cz = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r2 = fma(vIn.z, vIn.z, precalcSumSquares);\n"
		   << "\t\treal_t r = " << weight << " / Zeps(r2 * " << c2 << " + " << c2x << " * vIn.x - " << c2y << " * vIn.y + " << c2z << " * vIn.z + (real_t)(1.0));\n"
		   << "\n"
		   << "\t\tvOut.x = r * fma(" << cx << ", r2, vIn.x);\n"
		   << "\t\tvOut.y = r * (vIn.y - " << cy << " * r2);\n"
		   << "\t\tvOut.z = r * fma(" << cz << ", r2, vIn.z);\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

	virtual void Precalc() override
	{
		m_C2x = 2 * m_Cx;
		m_C2y = 2 * m_Cy;
		m_C2z = 2 * m_Cz;
		m_C2 = SQR(m_Cx) + SQR(m_Cy) + SQR(m_Cz);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Cx, prefix + "curl3D_cx"));
		m_Params.push_back(ParamWithName<T>(&m_Cy, prefix + "curl3D_cy"));
		m_Params.push_back(ParamWithName<T>(&m_Cz, prefix + "curl3D_cz"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2, prefix + "curl3D_c2"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_C2x, prefix + "curl3D_c2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2y, prefix + "curl3D_c2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2z, prefix + "curl3D_c2z"));
	}

private:
	T m_Cx;
	T m_Cy;
	T m_Cz;
	T m_C2;//Precalc.
	T m_C2x;
	T m_C2y;
	T m_C2z;
};

/// <summary>
/// Disc3D.
/// </summary>
template <typename T>
class Disc3DVariation : public ParametricVariation<T>
{
public:
	Disc3DVariation(T weight = 1.0) : ParametricVariation<T>("disc3d", eVariationId::VAR_DISC3D, weight, true, true, false, true, false)
	{
		Init();
	}

	PARVARCOPY(Disc3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = helper.m_PrecalcSqrtSumSquares;
		T temp = r * m_Pi;
		T sr = std::sin(temp);
		T cr = std::cos(temp);
		T vv = m_Weight * helper.m_PrecalcAtanxy / Zeps(m_Pi);
		helper.Out.x = vv * sr;
		helper.Out.y = vv * cr;
		helper.Out.z = vv * (r * std::cos(helper.In.z));
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string pi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = precalcSqrtSumSquares;\n"
		   << "\t\treal_t temp = r * " << pi << ";\n"
		   << "\t\treal_t sr = sin(temp);\n"
		   << "\t\treal_t cr = cos(temp);\n"
		   << "\t\treal_t vv = " << weight << " * precalcAtanxy / Zeps(" << pi << ");\n"
		   << "\n"
		   << "\t\tvOut.x = vv * sr;\n"
		   << "\t\tvOut.y = vv * cr;\n"
		   << "\t\tvOut.z = vv * (r * cos(vIn.z));\n"
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
		m_Params.push_back(ParamWithName<T>(&m_Pi, prefix + "disc3d_pi", T(M_PI)));
	}

private:
	T m_Pi;
};

/// <summary>
/// Boarders2.
/// </summary>
template <typename T>
class Boarders2Variation : public ParametricVariation<T>
{
public:
	Boarders2Variation(T weight = 1.0) : ParametricVariation<T>("boarders2", eVariationId::VAR_BOARDERS2, weight)
	{
		Init();
	}

	PARVARCOPY(Boarders2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T roundX = T(int(helper.In.x >= 0 ? int(helper.In.x + T(0.5)) : int(helper.In.x - T(0.5))));
		T roundY = T(int(helper.In.y >= 0 ? int(helper.In.y + T(0.5)) : int(helper.In.y - T(0.5))));
		T offsetX = helper.In.x - roundX;
		T offsetY = helper.In.y - roundY;

		if (rand.Frand01<T>() >= m_Cr)
		{
			helper.Out.x = m_Weight * (offsetX * m_AbsC + roundX);
			helper.Out.y = m_Weight * (offsetY * m_AbsC + roundY);
		}
		else
		{
			if (std::abs(offsetX) >= std::abs(offsetY))
			{
				if (offsetX >= 0)
				{
					helper.Out.x = m_Weight * (offsetX * m_AbsC + roundX + m_Cl);
					helper.Out.y = m_Weight * (offsetY * m_AbsC + roundY + m_Cl * offsetY / offsetX);
				}
				else
				{
					helper.Out.x = m_Weight * (offsetX * m_AbsC + roundX - m_Cl);
					helper.Out.y = m_Weight * (offsetY * m_AbsC + roundY - m_Cl * offsetY / offsetX);
				}
			}
			else
			{
				if (offsetY >= 0)
				{
					helper.Out.y = m_Weight * (offsetY * m_AbsC + roundY + m_Cl);
					helper.Out.x = m_Weight * (offsetX * m_AbsC + roundX + offsetX / offsetY * m_Cl);
				}
				else
				{
					helper.Out.y = m_Weight * (offsetY * m_AbsC + roundY - m_Cl);
					helper.Out.x = m_Weight * (offsetX * m_AbsC + roundX - offsetX / offsetY * m_Cl);
				}
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
		string c = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string l = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string r = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absc = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cl = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cr = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t roundX = (real_t)(int)(vIn.x >= 0 ? (int)(vIn.x + (real_t)(0.5)) : (int)(vIn.x - (real_t)(0.5)));\n"
		   << "\t\treal_t roundY = (real_t)(int)(vIn.y >= 0 ? (int)(vIn.y + (real_t)(0.5)) : (int)(vIn.y - (real_t)(0.5)));\n"
		   << "\t\treal_t offsetX = vIn.x - roundX;\n"
		   << "\t\treal_t offsetY = vIn.y - roundY;\n"
		   << "\n"
		   << "\t\tif (MwcNext01(mwc) >= " << cr << ")\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * fma(offsetX, " << absc << ", roundX);\n"
		   << "\t\t	vOut.y = " << weight << " * fma(offsetY, " << absc << ", roundY);\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (fabs(offsetX) >= fabs(offsetY))\n"
		   << "\t\t	{\n"
		   << "\t\t		if (offsetX >= 0)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * fma(offsetX, " << absc << ", roundX + " << cl << ");\n"
		   << "\t\t			vOut.y = " << weight << " * (fma(offsetY, " << absc << ", roundY) + " << cl << " * offsetY / offsetX);\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * fma(offsetX, " << absc << ", roundX - " << cl << ");\n"
		   << "\t\t			vOut.y = " << weight << " * (fma(offsetY, " << absc << ", roundY) - " << cl << " * offsetY / offsetX);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if(offsetY >= 0)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.y = " << weight << " * fma(offsetY, " << absc << ", roundY + " << cl << ");\n"
		   << "\t\t			vOut.x = " << weight << " * (fma(offsetX, " << absc << ", roundX) + offsetX / offsetY * " << cl << ");\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.y = " << weight << " * fma(offsetY, " << absc << ", roundY - " << cl << ");\n"
		   << "\t\t			vOut.x = " << weight << " * (fma(offsetX, " << absc << ", roundX) - offsetX / offsetY * " << cl << ");\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T c = Zeps(std::abs(m_C));
		T cl = Zeps(std::abs(m_Left));
		T cr = Zeps(std::abs(m_Right));
		m_AbsC = c;
		m_Cl = c * cl;
		m_Cr = c + (c * cr);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_C, prefix + "boarders2_c", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Left, prefix + "boarders2_left", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Right, prefix + "boarders2_right", T(0.5)));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsC, prefix + "boarders2_cabs"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cl, prefix + "boarders2_cl"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cr, prefix + "boarders2_cr"));
	}

private:
	T m_C;
	T m_Left;
	T m_Right;
	T m_AbsC;//Precalc.
	T m_Cl;
	T m_Cr;
};

/// <summary>
/// Cardioid.
/// </summary>
template <typename T>
class CardioidVariation : public ParametricVariation<T>
{
public:
	CardioidVariation(T weight = 1.0) : ParametricVariation<T>("cardioid", eVariationId::VAR_CARDIOID, weight, true, true, true, false, true)
	{
		Init();
	}

	PARVARCOPY(CardioidVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight * std::sqrt(helper.m_PrecalcSumSquares + std::sin(helper.m_PrecalcAtanyx * m_A) + 1);
		helper.Out.x = r * helper.m_PrecalcCosa;
		helper.Out.y = r * helper.m_PrecalcSina;
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
		   << "\t\treal_t r = " << weight << " * sqrt(precalcSumSquares + sin(precalcAtanyx * " << a << ") + 1);\n"
		   << "\n"
		   << "\t\tvOut.x = r * precalcCosa;\n"
		   << "\t\tvOut.y = r * precalcSina;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "cardioid_a", 1));
	}

private:
	T m_A;
};

/// <summary>
/// Checks.
/// </summary>
template <typename T>
class ChecksVariation : public ParametricVariation<T>
{
public:
	ChecksVariation(T weight = 1.0) : ParametricVariation<T>("checks", eVariationId::VAR_CHECKS, weight)
	{
		Init();
	}

	PARVARCOPY(ChecksVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T dx, dy;
		T rnx = m_Rnd * rand.Frand01<T>();
		T rny = m_Rnd * rand.Frand01<T>();
		int isXY = int(VarFuncs<T>::LRint(helper.In.x * m_Cs) + VarFuncs<T>::LRint(helper.In.y * m_Cs));

		if (isXY & 1)
		{
			dx = m_Ncx + rnx;
			dy = m_Ncy;
		}
		else
		{
			dx = m_Cx;
			dy = m_Cy + rny;
		}

		helper.Out.x = m_Weight * (helper.In.x + dx);
		helper.Out.y = m_Weight * (helper.In.y + dy);
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
		string size = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rnd = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cs = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ncx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ncy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t dx, dy;\n"
		   << "\t\treal_t rnx = " << rnd << " * MwcNext01(mwc);\n"
		   << "\t\treal_t rny = " << rnd << " * MwcNext01(mwc);\n"
		   << "\n"
		   << "\t\tint isXY = (int)(LRint(vIn.x * " << cs << ") + LRint(vIn.y * " << cs << "));\n"
		   << "\n"
		   << "\t\tif (isXY & 1)\n"
		   << "\t\t{\n"
		   << "\t\t	dx = " << ncx << " + rnx;\n"
		   << "\t\t	dy = " << ncy << ";\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	dx = " << cx << ";\n"
		   << "\t\t	dy = " << cy << " + rny;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (vIn.x + dx);\n"
		   << "\t\tvOut.y = " << weight << " * (vIn.y + dy);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "LRint" };
	}

	virtual void Precalc() override
	{
		m_Cs = 1 / Zeps(m_Size);
		m_Cx = m_X;
		m_Cy = m_Y;
		m_Ncx = -m_X;
		m_Ncy = -m_Y;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "checks_x", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "checks_y", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Size, prefix + "checks_size", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Rnd, prefix + "checks_rnd"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cs, prefix + "checks_cs"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cx, prefix + "checks_cx"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cy, prefix + "checks_cy"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ncx, prefix + "checks_ncx"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ncy, prefix + "checks_ncy"));
	}

private:
	T m_X;
	T m_Y;
	T m_Size;
	T m_Rnd;
	T m_Cs;//Precalc.
	T m_Cx;
	T m_Cy;
	T m_Ncx;
	T m_Ncy;
};

/// <summary>
/// Circlize.
/// </summary>
template <typename T>
class CirclizeVariation : public ParametricVariation<T>
{
public:
	CirclizeVariation(T weight = 1.0) : ParametricVariation<T>("circlize", eVariationId::VAR_CIRCLIZE, weight)
	{
		Init();
	}

	PARVARCOPY(CirclizeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T side;
		T perimeter;
		T r, val;
		T absx = std::abs(helper.In.x);
		T absy = std::abs(helper.In.y);

		if (absx >= absy)
		{
			if (helper.In.x >= absy)
				perimeter = absx + helper.In.y;
			else
				perimeter = 5 * absx - helper.In.y;

			side = absx;
		}
		else
		{
			if (helper.In.y >= absx)
				perimeter = 3 * absy - helper.In.x;
			else
				perimeter = 7 * absy + helper.In.x;

			side = absy;
		}

		r = m_Vvar4Pi * side + m_Hole;
		val = T(M_PI_4) * perimeter / side - T(M_PI_4);
		helper.Out.x = r * std::cos(val);
		helper.Out.y = r * std::sin(val);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string hole = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vvar4pi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t side;\n"
		   << "\t\treal_t perimeter;\n"
		   << "\t\treal_t absx = fabs(vIn.x);\n"
		   << "\t\treal_t absy = fabs(vIn.y);\n"
		   << "\n"
		   << "\t\tif (absx >= absy)\n"
		   << "\t\t{\n"
		   << "\t\t	if (vIn.x >= absy)\n"
		   << "\t\t		perimeter = absx + vIn.y;\n"
		   << "\t\t	else\n"
		   << "\t\t		perimeter = fma((real_t)(5.0), absx, -vIn.y);\n"
		   << "\n"
		   << "\t\t	side = absx;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (vIn.y >= absx)\n"
		   << "\t\t		perimeter = fma((real_t)(3.0), absy, -vIn.x);\n"
		   << "\t\t	else\n"
		   << "\t\t		perimeter = fma((real_t)(7.0), absy, vIn.x);\n"
		   << "\n"
		   << "\t\t	side = absy;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t r = fma(" << vvar4pi << ", side, " << hole << ");\n"
		   << "\t\treal_t val = MPI4 * perimeter / side - MPI4;\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(val);\n"
		   << "\t\tvOut.y = r * sin(val);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Vvar4Pi = m_Weight / T(M_PI_4);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Hole, prefix + "circlize_hole"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vvar4Pi, prefix + "circlize_vvar4pi"));//Precalc.
	}

private:
	T m_Hole;
	T m_Vvar4Pi;//Precalc.
};

/// <summary>
/// Circlize2.
/// </summary>
template <typename T>
class Circlize2Variation : public ParametricVariation<T>
{
public:
	Circlize2Variation(T weight = 1.0) : ParametricVariation<T>("circlize2", eVariationId::VAR_CIRCLIZE2, weight)
	{
		Init();
	}

	PARVARCOPY(Circlize2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T side;
		T perimeter;
		T absx = std::abs(helper.In.x);
		T absy = std::abs(helper.In.y);

		if (absx >= absy)
		{
			if (helper.In.x >= absy)
				perimeter = absx + helper.In.y;
			else
				perimeter = 5 * absx - helper.In.y;

			side = absx;
		}
		else
		{
			if (helper.In.y >= absx)
				perimeter = 3 * absy - helper.In.x;
			else
				perimeter = 7 * absy + helper.In.x;

			side = absy;
		}

		T r = m_Weight * (side + m_Hole);
		T val = T(M_PI_4) * perimeter / side - T(M_PI_4);
		helper.Out.x = r * std::cos(val);
		helper.Out.y = r * std::sin(val);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string hole = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t side;\n"
		   << "\t\treal_t perimeter;\n"
		   << "\t\treal_t absx = fabs(vIn.x);\n"
		   << "\t\treal_t absy = fabs(vIn.y);\n"
		   << "\n"
		   << "\t\tif (absx >= absy)\n"
		   << "\t\t{\n"
		   << "\t\t	if (vIn.x >= absy)\n"
		   << "\t\t		perimeter = absx + vIn.y;\n"
		   << "\t\t	else\n"
		   << "\t\t		perimeter = fma((real_t)(5.0), absx, -vIn.y);\n"
		   << "\n"
		   << "\t\t	side = absx;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (vIn.y >= absx)\n"
		   << "\t\t		perimeter = fma((real_t)(3.0), absy, -vIn.x);\n"
		   << "\t\t	else\n"
		   << "\t\t		perimeter = fma((real_t)(7.0), absy, vIn.x);\n"
		   << "\n"
		   << "\t\t	side = absy;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t r = " << weight << " * (side + " << hole << ");\n"
		   << "\t\treal_t val = MPI4 * perimeter / side - MPI4;\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(val);\n"
		   << "\t\tvOut.y = r * sin(val);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Hole, prefix + "circlize2_hole"));
	}

private:
	T m_Hole;
};

/// <summary>
/// CosWrap.
/// </summary>
template <typename T>
class CosWrapVariation : public ParametricVariation<T>
{
public:
	CosWrapVariation(T weight = 1.0) : ParametricVariation<T>("coswrap", eVariationId::VAR_COS_WRAP, weight)
	{
		Init();
	}

	PARVARCOPY(CosWrapVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = T(0.5) * helper.In.x + T(0.5);
		T y = T(0.5) * helper.In.y + T(0.5);
		T bx = VarFuncs<T>::Fabsmod(m_Fr * x);
		T by = VarFuncs<T>::Fabsmod(m_Fr * y);
		T oscnapx = VarFuncs<T>::Foscn(m_AmountX, m_Px);
		T oscnapy = VarFuncs<T>::Foscn(m_AmountY, m_Py);
		helper.Out.x = -1 + m_Vv2 * Lerp<T>(Lerp<T>(x, VarFuncs<T>::Fosc(x, T(4), m_Px), oscnapx), VarFuncs<T>::Fosc(bx, T(4), m_Px), oscnapx);//Original did a direct assignment to outPoint, which is incompatible with Ember's design.
		helper.Out.y = -1 + m_Vv2 * Lerp<T>(Lerp<T>(y, VarFuncs<T>::Fosc(y, T(4), m_Py), oscnapy), VarFuncs<T>::Fosc(by, T(4), m_Py), oscnapy);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string repeat = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amountX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amountY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phaseX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phaseY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ax = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ay = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string px = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string py = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fr = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vv2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x = fma((real_t)(0.5), vIn.x, (real_t)(0.5));\n"
		   << "\t\treal_t y = fma((real_t)(0.5), vIn.y, (real_t)(0.5));\n"
		   << "\t\treal_t bx = Fabsmod(" << fr << " * x);\n"
		   << "\t\treal_t by = Fabsmod(" << fr << " * y);\n"
		   << "\t\treal_t oscnapx = Foscn(" << amountX << ", " << px << ");\n"
		   << "\t\treal_t oscnapy = Foscn(" << amountY << ", " << py << ");\n"
		   << "\n"
		   << "\t\tvOut.x = -1 + " << vv2 << " * Lerp(Lerp(x, Fosc(x, 4, " << px << "), oscnapx), Fosc(bx, 4, " << px << "), oscnapx);\n"
		   << "\t\tvOut.y = -1 + " << vv2 << " * Lerp(Lerp(y, Fosc(y, 4, " << py << "), oscnapy), Fosc(by, 4, " << py << "), oscnapy);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Fabsmod", "Fosc", "Foscn", "Lerp" };
	}

	virtual void Precalc() override
	{
		m_Ax = M_2PI * std::abs(m_AmountX);
		m_Ay = M_2PI * std::abs(m_AmountY);
		m_Px = T(M_PI)  * m_PhaseX;
		m_Py = T(M_PI)  * m_PhaseY;
		m_Fr = std::abs(m_Repeat);
		m_Vv2 = 2 * m_Weight;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Repeat, prefix + "coswrap_repeat", 1, eParamType::INTEGER_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_AmountX, prefix + "coswrap_amount_x"));
		m_Params.push_back(ParamWithName<T>(&m_AmountY, prefix + "coswrap_amount_y"));
		m_Params.push_back(ParamWithName<T>(&m_PhaseX, prefix + "coswrap_phase_x", 0, eParamType::REAL_CYCLIC, -1, 1));
		m_Params.push_back(ParamWithName<T>(&m_PhaseY, prefix + "coswrap_phase_y", 0, eParamType::REAL_CYCLIC, -1, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Ax, prefix + "coswrap_ax"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Ay, prefix + "coswrap_ay"));
		m_Params.push_back(ParamWithName<T>(true, &m_Px, prefix + "coswrap_px"));
		m_Params.push_back(ParamWithName<T>(true, &m_Py, prefix + "coswrap_py"));
		m_Params.push_back(ParamWithName<T>(true, &m_Fr, prefix + "coswrap_fr"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vv2, prefix + "coswrap_vv2"));
	}

private:
	T m_Repeat;
	T m_AmountX;
	T m_AmountY;
	T m_PhaseX;
	T m_PhaseY;
	T m_Ax;//Precalc.
	T m_Ay;
	T m_Px;
	T m_Py;
	T m_Fr;
	T m_Vv2;
};

/// <summary>
/// DeltaA.
/// The original in deltaA.c in Apophysis used a precalc variable named v, but
/// was unused in the calculation. So this remains a non-parametric variation with
/// that precalc variable omitted.
/// </summary>
template <typename T>
class DeltaAVariation : public Variation<T>
{
public:
	DeltaAVariation(T weight = 1.0) : Variation<T>("deltaa", eVariationId::VAR_DELTA_A, weight) { }
	VARCOPY(DeltaAVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T s, c;
		T avgr = m_Weight * (std::sqrt(SQR(helper.In.y) + SQR(helper.In.x + 1)) / std::sqrt(SQR(helper.In.y) + SQR(helper.In.x - 1)));
		T avga = (std::atan2(helper.In.y, helper.In.x - 1) - std::atan2(helper.In.y, helper.In.x + 1)) / 2;
		sincos(avga, &s, &c);
		helper.Out.x = avgr * c;
		helper.Out.y = avgr * s;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t xp1 = vIn.x + (real_t)(1.0);\n"
		   << "\t\treal_t xm1 = vIn.x - (real_t)(1.0);\n"
		   << "\t\treal_t avgr = " << weight << " * (sqrt(fma(vIn.y, vIn.y, SQR(xp1))) / sqrt(fma(vIn.y, vIn.y, SQR(xm1))));\n"
		   << "\t\treal_t avga = (atan2(vIn.y, xm1) - atan2(vIn.y, xp1)) / 2;\n"
		   << "\t\treal_t s = sin(avga);\n"
		   << "\t\treal_t c = cos(avga);\n"
		   << "\n"
		   << "\t\tvOut.x = avgr * c;\n"
		   << "\t\tvOut.y = avgr * s;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Expo.
/// </summary>
template <typename T>
class ExpoVariation : public ParametricVariation<T>
{
public:
	ExpoVariation(T weight = 1.0) : ParametricVariation<T>("expo", eVariationId::VAR_EXPO, weight)
	{
		Init();
	}

	PARVARCOPY(ExpoVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T expor = std::exp(helper.In.x * m_K - helper.In.y * m_T);
		T temp = helper.In.x * m_T + helper.In.y * m_K;
		T snv = std::sin(temp);
		T csv = std::cos(temp);
		helper.Out.x = m_Weight * expor * csv;
		helper.Out.y = m_Weight * expor * snv;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string real = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string imag = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string k = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string t = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t expor = exp(fma(vIn.x, " << k << ", -(vIn.y * " << t << ")));\n"
		   << "\t\treal_t temp = fma(vIn.x, " << t << ", (vIn.y * " << k << "));\n"
		   << "\t\treal_t snv = sin(temp);\n"
		   << "\t\treal_t csv = cos(temp);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * expor * csv;\n"
		   << "\t\tvOut.y = " << weight << " * expor * snv;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_K = T(0.5) * std::log(Zeps(SQR(m_Real) + SQR(m_Imag)));//Original used 1e-300, which isn't representable with a float.
		m_T = std::atan2(m_Imag, m_Real);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Real, prefix + "expo_real", -1));
		m_Params.push_back(ParamWithName<T>(&m_Imag, prefix + "expo_imaginary", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_K, prefix + "expo_k"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_T, prefix + "expo_t"));
	}

private:
	T m_Real;
	T m_Imag;
	T m_K;//Precalc.
	T m_T;
};

/// <summary>
/// Extrude.
/// </summary>
template <typename T>
class ExtrudeVariation : public ParametricVariation<T>
{
public:
	ExtrudeVariation(T weight = 1.0) : ParametricVariation<T>("extrude", eVariationId::VAR_EXTRUDE, weight)
	{
		Init();
	}

	PARVARCOPY(ExtrudeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_VarType == eVariationType::VARTYPE_REG)
		{
			helper.Out.x = helper.Out.y = helper.Out.z = 0;

			if (rand.Frand01<T>() < m_RootFace)
				outPoint.m_Z = ClampGte0(m_Weight);
			else
				outPoint.m_Z = m_Weight * rand.Frand01<T>();
		}
		else
		{
			helper.Out.x = helper.In.x;
			helper.Out.y = helper.In.y;

			if (rand.Frand01<T>() < m_RootFace)
				helper.Out.z = ClampGte0(m_Weight);
			else
				helper.Out.z = m_Weight * rand.Frand01<T>();
		}
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string rootFace = "parVars[" + ToUpper(m_Params[i++].Name()) + index;

		if (m_VarType == eVariationType::VARTYPE_REG)
		{
			ss << "\t{\n"
			   << "\t\tvOut.x = vOut.y = vOut.z = 0;\n"
			   << "\n"
			   << "\t\tif (MwcNext01(mwc) < " << rootFace << ")\n"
			   << "\t\t	outPoint->m_Z = max(" << weight << ", (real_t)(0.0));\n"
			   << "\t\telse\n"
			   << "\t\t	outPoint->m_Z = " << weight << " * MwcNext01(mwc);\n"
			   << "\t}\n";
		}
		else
		{
			ss << "\t{\n"
			   << "\t\tvOut.x = vIn.x;\n"
			   << "\t\tvOut.y = vIn.y;\n"
			   << "\n"
			   << "\t\tif (MwcNext01(mwc) < " << rootFace << ")\n"
			   << "\t\t	vOut.z = max(" << weight << ", (real_t)(0.0));\n"
			   << "\t\telse\n"
			   << "\t\t	vOut.z = " << weight << " * MwcNext01(mwc);\n"
			   << "\t}\n";
		}

		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_RootFace, prefix + "extrude_root_face", T(0.5)));
	}

private:
	T m_RootFace;
};

/// <summary>
/// fdisc.
/// </summary>
template <typename T>
class FDiscVariation : public Variation<T>
{
public:
	FDiscVariation(T weight = 1.0) : Variation<T>("fdisc", eVariationId::VAR_FDISC, weight, true, true, false, false, true) { }

	VARCOPY(FDiscVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T c, s;
		T a = M_2PI / (helper.m_PrecalcSqrtSumSquares + 1);
		T r = (helper.m_PrecalcAtanyx * T(M_1_PI) + 1) * T(0.5);
		sincos(a, &s, &c);
		helper.Out.x = m_Weight * r * c;
		helper.Out.y = m_Weight * r * s;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t a = M_2PI / (precalcSqrtSumSquares + 1);\n"
		   << "\t\treal_t r = fma(precalcAtanyx, M1PI, (real_t)(1.0)) * (real_t)(0.5);\n"
		   << "\t\treal_t s = sin(a);\n"
		   << "\t\treal_t c = cos(a);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * r * c;\n"
		   << "\t\tvOut.y = " << weight << " * r * s;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Fibonacci.
/// </summary>
template <typename T>
class FibonacciVariation : public ParametricVariation<T>
{
public:
	FibonacciVariation(T weight = 1.0) : ParametricVariation<T>("fibonacci", eVariationId::VAR_FIBONACCI, weight)
	{
		Init();
	}

	PARVARCOPY(FibonacciVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T snum1, cnum1, snum2, cnum2;
		T temp = helper.In.y * m_NatLog;
		sincos(temp, &snum1, &cnum1);
		temp = (helper.In.x * T(M_PI) + helper.In.y * m_NatLog) * -1;
		sincos(temp, &snum2, &cnum2);
		T eradius1 = std::exp(helper.In.x * m_NatLog);
		T eradius2 = std::exp((helper.In.x * m_NatLog - helper.In.y * T(M_PI)) * -1);
		helper.Out.x = m_Weight * (eradius1 * cnum1 - eradius2 * cnum2) * m_Five;
		helper.Out.y = m_Weight * (eradius1 * snum1 - eradius2 * snum2) * m_Five;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string five = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.
		string natLog = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t temp = vIn.y * " << natLog << ";\n"
		   << "\t\treal_t snum1 = sin(temp);\n"
		   << "\t\treal_t cnum1 = cos(temp);\n"
		   << "\t\ttemp = fma(vIn.x, MPI, vIn.y * " << natLog << ") * (real_t)(-1.0);\n"
		   << "\t\treal_t snum2 = sin(temp);\n"
		   << "\t\treal_t cnum2 = cos(temp);\n"
		   << "\t\treal_t eradius1 = exp(vIn.x * " << natLog << ");\n"
		   << "\t\treal_t eradius2 = exp(fma(vIn.x, " << natLog << ", -(vIn.y * MPI)) * (real_t)(-1.0));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (eradius1 * cnum1 - eradius2 * cnum2) * " << five << ";\n"
		   << "\t\tvOut.y = " << weight << " * (eradius1 * snum1 - eradius2 * snum2) * " << five << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Five = 1 / M_SQRT5;
		m_NatLog = std::log(M_PHI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_Five, prefix + "fibonacci_five"));//Precalcs only, no params.
		m_Params.push_back(ParamWithName<T>(true, &m_NatLog, prefix + "fibonacci_nat_log"));
	}

private:
	T m_Five;//Precalcs only, no params.
	T m_NatLog;
};

/// <summary>
/// Fibonacci2.
/// </summary>
template <typename T>
class Fibonacci2Variation : public ParametricVariation<T>
{
public:
	Fibonacci2Variation(T weight = 1.0) : ParametricVariation<T>("fibonacci2", eVariationId::VAR_FIBONACCI2, weight)
	{
		Init();
	}

	PARVARCOPY(Fibonacci2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T snum1, cnum1, snum2, cnum2;
		T temp = helper.In.y * m_NatLog;
		sincos(temp, &snum1, &cnum1);
		temp = (helper.In.x * T(M_PI) + helper.In.y * m_NatLog) * -1;
		sincos(temp, &snum2, &cnum2);
		T eradius1 = m_Sc * std::exp(m_Sc2 * (helper.In.x * m_NatLog));
		T eradius2 = m_Sc * std::exp(m_Sc2 * ((helper.In.x * m_NatLog - helper.In.y * T(M_PI)) * -1));
		helper.Out.x = m_Weight * (eradius1 * cnum1 - eradius2 * cnum2) * m_Five;
		helper.Out.y = m_Weight * (eradius1 * snum1 - eradius2 * snum2) * m_Five;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string sc = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sc2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string five = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string natLog = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t temp = vIn.y * " << natLog << ";\n"
		   << "\t\treal_t snum1 = sin(temp);\n"
		   << "\t\treal_t cnum1 = cos(temp);\n"
		   << "\t\ttemp = fma(vIn.x, MPI, vIn.y * " << natLog << ") * -1;\n"
		   << "\t\treal_t snum2 = sin(temp);\n"
		   << "\t\treal_t cnum2 = cos(temp);\n"
		   << "\t\treal_t eradius1 = " << sc << " * exp(" << sc2 << " * (vIn.x * " << natLog << "));\n"
		   << "\t\treal_t eradius2 = " << sc << " * exp(" << sc2 << " * (fma(vIn.x, " << natLog << ", -(vIn.y * MPI)) * (real_t)(-1.0)));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(eradius1, cnum1, -(eradius2 * cnum2)) * " << five << ";\n"
		   << "\t\tvOut.y = " << weight << " * fma(eradius1, snum1, -(eradius2 * snum2)) * " << five << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Five = 1 / M_SQRT5;
		m_NatLog = std::log(M_PHI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Sc, prefix + "fibonacci2_sc", 1));
		m_Params.push_back(ParamWithName<T>(&m_Sc2, prefix + "fibonacci2_sc2", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Five, prefix + "fibonacci2_five"));//Precalcs.
		m_Params.push_back(ParamWithName<T>(true, &m_NatLog, prefix + "fibonacci2_nat_log"));
	}

private:
	T m_Sc;
	T m_Sc2;
	T m_Five;//Precalcs.
	T m_NatLog;
};

/// <summary>
/// Glynnia.
/// </summary>
template <typename T>
class GlynniaVariation : public ParametricVariation<T>
{
public:
	GlynniaVariation(T weight = 1.0) : ParametricVariation<T>("glynnia", eVariationId::VAR_GLYNNIA, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(GlynniaVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (Compat::m_Compat)
		{
			T d, r = helper.m_PrecalcSqrtSumSquares;

			if (r > 1)
			{
				if (rand.Frand01<T>() > T(0.5))
				{
					d = std::sqrt(r + helper.In.x);
					helper.Out.x = m_V2 * d;
					helper.Out.y = -(m_V2 / d * helper.In.y);
				}
				else
				{
					d = r + helper.In.x;
					r = m_Weight / std::sqrt(r * (SQR(helper.In.y) + SQR(d)));
					helper.Out.x = r * d;
					helper.Out.y = r * helper.In.y;
				}
			}
			else
			{
				if (rand.Frand01<T>() > T(0.5))
				{
					d = Zeps(std::sqrt(r + helper.In.x));
					helper.Out.x = -(m_V2 * d);
					helper.Out.y = -(m_V2 / d * helper.In.y);
				}
				else
				{
					d = r + helper.In.x;
					r = m_Weight / Zeps(std::sqrt(r * (SQR(helper.In.y) + SQR(d))));
					helper.Out.x = -(r * d);
					helper.Out.y = r * helper.In.y;
				}
			}
		}
		else
		{
			T r2 = helper.m_PrecalcSumSquares;
			T sr = helper.m_PrecalcSqrtSumSquares;
			T aux = r2 > 1 ? T(1) : T(-1);
			T temp = std::sqrt(sr + helper.In.x);

			if (rand.Frand01<T>() < T(0.5))
			{
				helper.Out.x = m_V2 * aux * temp;
				helper.Out.y = m_V2 * (-helper.In.y / Zeps(temp));
			}
			else
			{
				T tempweight = m_V2 / Zeps(sr);
				helper.Out.x = tempweight * aux * temp;
				helper.Out.y = tempweight * (helper.In.y / Zeps(temp));
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
		string v2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.

		if (Compat::m_Compat)
		{
			ss << "\t{\n"
			   << "\t\treal_t d, r = precalcSqrtSumSquares;\n"
			   << "\n"
			   << "\t\tif (r > 1)\n"
			   << "\t\t{\n"
			   << "\t\t	if (MwcNext01(mwc) > (real_t)(0.5))\n"
			   << "\t\t	{\n"
			   << "\t\t		d = sqrt(r + vIn.x);\n"
			   << "\t\t		vOut.x = " << v2 << " * d;\n"
			   << "\t\t		vOut.y = -(" << v2 << " / d * vIn.y);\n"
			   << "\t\t	}\n"
			   << "\t\t	else\n"
			   << "\t\t	{\n"
			   << "\t\t		d = r + vIn.x;\n"
			   << "\t\t		r = " << weight << " / sqrt(r * fma(vIn.y, vIn.y, SQR(d)));\n"
			   << "\t\t		vOut.x = r * d;\n"
			   << "\t\t		vOut.y = r * vIn.y;\n"
			   << "\t\t	}\n"
			   << "\t\t}\n"
			   << "\t\telse\n"
			   << "\t\t{\n"
			   << "\t\t	if (MwcNext01(mwc) > (real_t)(0.5))\n"
			   << "\t\t	{\n"
			   << "\t\t		d = Zeps(sqrt(r + vIn.x));\n"
			   << "\t\t		vOut.x = -(" << v2 << " * d);\n"
			   << "\t\t		vOut.y = -(" << v2 << " / d * vIn.y);\n"
			   << "\t\t	}\n"
			   << "\t\t	else\n"
			   << "\t\t	{\n"
			   << "\t\t		d = r + vIn.x;\n"
			   << "\t\t		r = " << weight << " / sqrt(r * fma(vIn.y, vIn.y, SQR(d)));\n"
			   << "\t\t		vOut.x = -(r * d);\n"
			   << "\t\t		vOut.y = r * vIn.y;\n"
			   << "\t\t	}\n"
			   << "\t\t}\n";
		}
		else
		{
			ss << "\t{\n"
			   << "\t\treal_t r2 = precalcSumSquares;\n"
			   << "\t\treal_t sr = precalcSqrtSumSquares;\n"
			   << "\t\treal_t aux = r2 > 1 ? (real_t)(1.0) : (real_t)(-1.0);\n"
			   << "\t\treal_t temp = sqrt(sr + vIn.x);\n"
			   << "\n"
			   << "\t\tif (MwcNext01(mwc) < (real_t)(0.5))\n"
			   << "\t\t{\n"
			   << "\t\t	vOut.x = " << v2 << " * aux * temp;\n"
			   << "\t\t	vOut.y = " << v2 << " * (-vIn.y / Zeps(temp));\n"
			   << "\t\t}\n"
			   << "\t\telse\n"
			   << "\t\t{\n"
			   << "\t\t	real_t tempweight = " << v2 << " / Zeps(sr);\n"
			   << "\t\t	vOut.x = tempweight * aux * temp;\n"
			   << "\t\t	vOut.y = tempweight * (vIn.y / Zeps(temp));\n"
			   << "\t\t}\n";
		}

		ss << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_V2 = m_Weight * (std::sqrt(T(2)) / 2);
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
		m_Params.push_back(ParamWithName<T>(true, &m_V2, prefix + "glynnia_v2"));//Precalcs only, no params.
	}

private:
	T m_V2;//Precalcs only, no params.
};

/// <summary>
/// Glynnia2.
/// By guagapunyaimel.
/// </summary>
template <typename T>
class Glynnia2Variation : public ParametricVariation<T>
{
public:
	Glynnia2Variation(T weight = 1.0) : ParametricVariation<T>("glynnia2", eVariationId::VAR_GLYNNIA2, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(Glynnia2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T d, r = helper.m_PrecalcSqrtSumSquares;

		if (r > 0 && helper.In.y > 0)
		{
			if (rand.Frand01<T>() > T(0.5))
			{
				d = std::sqrt(r + helper.In.x);
				helper.Out.x = m_V2 * d;
				helper.Out.y = -(m_V2 / d * helper.In.y);
			}
			else
			{
				d = r + helper.In.x;
				r = m_Weight / std::sqrt(r * (SQR(helper.In.y) + SQR(d)));
				helper.Out.x = r * d;
				helper.Out.y = r * helper.In.y;
			}
		}
		else
		{
			if (rand.Frand01<T>() > T(0.5))
			{
				d = Zeps(std::sqrt(r + helper.In.x));
				helper.Out.x = -(m_V2 * d);
				helper.Out.y = -(m_V2 / d * helper.In.y);
			}
			else
			{
				d = r + helper.In.x;
				r = m_Weight / Zeps(std::sqrt(r * (SQR(helper.In.y) + SQR(d))));
				helper.Out.x = -(r * d);
				helper.Out.y = r * helper.In.y;
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
		string v2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.
		ss << "\t{\n"
		   << "\t\treal_t d, r = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tif (r > 0 && vIn.y > 0)\n"
		   << "\t\t{\n"
		   << "\t\t	if (MwcNext01(mwc) > (real_t)(0.5))\n"
		   << "\t\t	{\n"
		   << "\t\t		d = sqrt(r + vIn.x);\n"
		   << "\t\t		vOut.x = " << v2 << " * d;\n"
		   << "\t\t		vOut.y = -(" << v2 << " / d * vIn.y);\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		d = r + vIn.x;\n"
		   << "\t\t		r = " << weight << " / sqrt(r * fma(vIn.y, vIn.y, SQR(d)));\n"
		   << "\t\t		vOut.x = r * d;\n"
		   << "\t\t		vOut.y = r * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (MwcNext01(mwc) > (real_t)(0.5))\n"
		   << "\t\t	{\n"
		   << "\t\t		d = Zeps(sqrt(r + vIn.x));\n"
		   << "\t\t		vOut.x = -(" << v2 << " * d);\n"
		   << "\t\t		vOut.y = -(" << v2 << " / d * vIn.y);\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		d = r + vIn.x;\n"
		   << "\t\t		r = " << weight << " / Zeps(sqrt(r * fma(vIn.y, vIn.y, SQR(d))));\n"
		   << "\t\t		vOut.x = -(r * d);\n"
		   << "\t\t		vOut.y = r * vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_V2 = m_Weight * std::sqrt(T(2)) / 2;
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
		m_Params.push_back(ParamWithName<T>(true, &m_V2, prefix + "glynnia2_v2"));//Precalcs only, no params.
	}

private:
	T m_V2;//Precalcs only, no params.
};

/// <summary>
/// GridOut.
/// </summary>
template <typename T>
class GridOutVariation : public Variation<T>
{
public:
	GridOutVariation(T weight = 1.0) : Variation<T>("gridout", eVariationId::VAR_GRIDOUT, weight) { }

	VARCOPY(GridOutVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = VarFuncs<T>::LRint(helper.In.x);
		T y = VarFuncs<T>::LRint(helper.In.y);

		if (y <= 0)
		{
			if (x > 0)
			{
				if (-y >= x)
				{
					helper.Out.x = m_Weight * (helper.In.x + 1);
					helper.Out.y = m_Weight * helper.In.y;
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y + 1);
				}
			}
			else
			{
				if (y <= x)
				{
					helper.Out.x = m_Weight * (helper.In.x + 1);
					helper.Out.y = m_Weight * helper.In.y;
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y - 1);
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
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y + 1);
				}
			}
			else
			{
				if (y > -x)
				{
					helper.Out.x = m_Weight * (helper.In.x - 1);
					helper.Out.y = m_Weight * helper.In.y;
				}
				else
				{
					helper.Out.x = m_Weight * helper.In.x;
					helper.Out.y = m_Weight * (helper.In.y - 1);
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
		   << "\t\treal_t x = LRint(vIn.x);\n"
		   << "\t\treal_t y = LRint(vIn.y);\n"
		   << "\n"
		   << "\t\tif (y <= 0)\n"
		   << "\t\t{\n"
		   << "\t\t	if (x > 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		if (-y >= x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * (vIn.x + 1);\n"
		   << "\t\t			vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t			vOut.y = " << weight << " * (vIn.y + 1);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (y <= x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * (vIn.x + 1);\n"
		   << "\t\t			vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t			vOut.y = " << weight << " * (vIn.y - 1);\n"
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
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t			vOut.y = " << weight << " * (vIn.y + 1);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (y > -x)\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * (vIn.x - 1);\n"
		   << "\t\t			vOut.y = " << weight << " * vIn.y;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			vOut.x = " << weight << " * vIn.x;\n"
		   << "\t\t			vOut.y = " << weight << " * (vIn.y - 1);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "LRint" };
	}
};

/// <summary>
/// Hole.
/// </summary>
template <typename T>
class HoleVariation : public ParametricVariation<T>
{
public:
	HoleVariation(T weight = 1.0) : ParametricVariation<T>("hole", eVariationId::VAR_HOLE, weight, true, true, true, false, true)
	{
		Init();
	}

	PARVARCOPY(HoleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r, delta = std::pow(helper.m_PrecalcAtanyx / T(M_PI) + 1, m_A);

		if (m_Inside != 0)
			r = m_Weight * delta / (helper.m_PrecalcSqrtSumSquares + delta);
		else
			r = m_Weight * helper.m_PrecalcSqrtSumSquares + delta;

		helper.Out.x = r * helper.m_PrecalcCosa;
		helper.Out.y = r * helper.m_PrecalcSina;
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
		string inside = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r, delta = pow(precalcAtanyx / MPI + 1, " << a << ");\n"
		   << "\n"
		   << "\t\tif (" << inside << " != 0)\n"
		   << "\t\t	r = " << weight << " * delta / (precalcSqrtSumSquares + delta);\n"
		   << "\t\telse\n"
		   << "\t\t	r = fma(" << weight << ", precalcSqrtSumSquares, delta);\n"
		   << "\n"
		   << "\t\tvOut.x = r * precalcCosa;\n"
		   << "\t\tvOut.y = r * precalcSina;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "hole_a", 1));
		m_Params.push_back(ParamWithName<T>(&m_Inside, prefix + "hole_inside", 0, eParamType::INTEGER, 0, 1));
	}

private:
	T m_A;
	T m_Inside;
};

/// <summary>
/// Hypertile.
/// </summary>
template <typename T>
class HypertileVariation : public ParametricVariation<T>
{
public:
	HypertileVariation(T weight = 1.0) : ParametricVariation<T>("hypertile", eVariationId::VAR_HYPERTILE, weight)
	{
		Init();
	}

	PARVARCOPY(HypertileVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.In.x + m_Real;
		T b = helper.In.y - m_Imag;
		T c = m_Real * helper.In.x - m_Imag * helper.In.y + 1;
		T d = m_Real * helper.In.y + m_Imag * helper.In.x;
		T vr = m_Weight / (SQR(c) + SQR(d));
		helper.Out.x = vr * (a * c + b * d);
		helper.Out.y = vr * (b * c - a * d);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string p    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string real = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string imag = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a = vIn.x + " << real << ";\n"
		   << "\t\treal_t b = vIn.y - " << imag << ";\n"
		   << "\t\treal_t c = fma(" << real << ", vIn.x, -(" << imag << " * vIn.y)) + 1;\n"
		   << "\t\treal_t d = fma(" << real << ", vIn.y, " << imag << " * vIn.x);\n"
		   << "\t\treal_t vr = " << weight << " / fma(c, c, SQR(d));\n"
		   << "\n"
		   << "\t\tvOut.x = vr * fma(a, c, b * d);\n"
		   << "\t\tvOut.y = vr * fma(b, c, -(a * d));\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T pa = M_2PI / Zeps(m_P);
		T cs = std::cos(pa);
		T r2 = (T(1) - cs) / (cs + cos(M_2PI / Zeps(m_Q))) + 1;
		T r  = (r2 > 0) ? T(1) / sqrt(r2) : T(1);
		T a  = m_N * pa;
		m_Real = r * std::cos(a);
		m_Imag = r * std::sin(a);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "hypertile_p", 3));
		m_Params.push_back(ParamWithName<T>(&m_Q, prefix + "hypertile_q", 7));
		m_Params.push_back(ParamWithName<T>(&m_N, prefix + "hypertile_n", 0));
		m_Params.push_back(ParamWithName<T>(true, &m_Real, prefix + "hypertile_real"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Imag, prefix + "hypertile_imag"));
	}

private:
	T m_P;
	T m_Q;
	T m_N;
	T m_Real;//Precalc.
	T m_Imag;
};

/// <summary>
/// Hypertile1.
/// </summary>
template <typename T>
class Hypertile1Variation : public ParametricVariation<T>
{
public:
	Hypertile1Variation(T weight = 1.0) : ParametricVariation<T>("hypertile1", eVariationId::VAR_HYPERTILE1, weight)
	{
		Init();
	}

	PARVARCOPY(Hypertile1Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T temp;

		if (Compat::m_Compat)
			temp = rand.Crand() * m_Pa;
		else
			temp = Floor(rand.Frand01<T>() * m_IP) * m_Pa;

		T sina = std::sin(temp);
		T cosa = std::cos(temp);
		T re = m_R * cosa;
		T im = m_R * sina;
		T a = helper.In.x + re;
		T b = helper.In.y - im;
		T c = re * helper.In.x - im * helper.In.y + 1;
		T d = re * helper.In.y + im * helper.In.x;
		T vr = m_Weight / Zeps(SQR(c) + SQR(d));
		helper.Out.x = vr * (a * c + b * d);
		helper.Out.y = vr * (b * c - a * d);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string p  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pa = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string r  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ip = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n";

		if (Compat::m_Compat)
			ss << "\t\treal_t temp = MwcNextCrand(mwc) * " << pa << ";\n";
		else
			ss << "\t\treal_t temp = floor(MwcNext01(mwc) * " << ip << ") * " << pa << ";\n";

		ss << "\t\treal_t sina = sin(temp);\n"
		   << "\t\treal_t cosa = cos(temp);\n"
		   << "\t\treal_t r = " << r << ";\n"
		   << "\t\treal_t re = r * cosa;\n"
		   << "\t\treal_t im = r * sina;\n"
		   << "\t\treal_t a = vIn.x + re;\n"
		   << "\t\treal_t b = vIn.y - im;\n"
		   << "\t\treal_t c = fma(re, vIn.x, -(im * vIn.y)) + 1;\n"
		   << "\t\treal_t d = fma(re, vIn.y, im * vIn.x);\n"
		   << "\t\treal_t vr = " << weight << " / Zeps(fma(c, c, SQR(d)));\n"
		   << "\n"
		   << "\t\tvOut.x = vr * fma(a, c, b * d);\n"
		   << "\t\tvOut.y = vr * fma(b, c, -(a * d));\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Pa = M_2PI / Zeps(m_P);
		T cs = std::cos(m_Pa);
		T r2 = T(1) - (cs - 1) / (cs + std::cos(M_2PI / Zeps(m_Q)));
		m_R  = (r2 > 0) ? T(1) / std::sqrt(r2) : T(1);
        m_IP = T(int(m_P));
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
		m_Params.push_back(ParamWithName<T>(&m_P,        prefix + "hypertile1_p", 3));
		m_Params.push_back(ParamWithName<T>(&m_Q,        prefix + "hypertile1_q", 7));
		m_Params.push_back(ParamWithName<T>(true, &m_Pa, prefix + "hypertile1_pa"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_R,  prefix + "hypertile1_r"));
		m_Params.push_back(ParamWithName<T>(true, &m_IP, prefix + "hypertile1_ip"));
	}

private:
	T m_P;
	T m_Q;
	T m_Pa;//Precalc.
	T m_R;
	T m_IP;
};

/// <summary>
/// Hypertile2.
/// </summary>
template <typename T>
class Hypertile2Variation : public ParametricVariation<T>
{
public:
	Hypertile2Variation(T weight = 1.0) : ParametricVariation<T>("hypertile2", eVariationId::VAR_HYPERTILE2, weight)
	{
		Init();
	}

	PARVARCOPY(Hypertile2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.In.x + m_R;
		T b = helper.In.y;
		T c = m_R * helper.In.x + 1;
		T d = m_R * helper.In.y;
		T x = (a * c + b * d);
		T y = (b * c - a * d);
		T vr   = m_Weight / Zeps(SQR(c) + SQR(d));
		T temp = rand.Crand() * m_Pa;
		T sina = std::sin(temp);
		T cosa = std::cos(temp);
		helper.Out.x = vr * (x * cosa + y * sina);
		helper.Out.y = vr * (y * cosa - x * sina);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string p = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pa = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string r = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r = " << r << ";\n"
		   << "\t\treal_t a = vIn.x + r;\n"
		   << "\t\treal_t b = vIn.y;\n"
		   << "\t\treal_t c = fma(r, vIn.x, (real_t)(1.0));\n"
		   << "\t\treal_t d = r * vIn.y;\n"
		   << "\t\treal_t x = fma(a, c, b * d);\n"
		   << "\t\treal_t y = fma(b, c, -(a * d));\n"
		   << "\t\treal_t vr = " << weight << " / Zeps(fma(c, c, SQR(d)));\n"
		   << "\t\treal_t temp = MwcNextCrand(mwc) * " << pa << ";\n"
		   << "\t\treal_t sina = sin(temp);\n"
		   << "\t\treal_t cosa = cos(temp);\n"
		   << "\n"
		   << "\t\tvOut.x = vr * fma(x, cosa, y * sina);\n"
		   << "\t\tvOut.y = vr * fma(y, cosa, -(x * sina));\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Pa = T(M_2PI) / Zeps(m_P);
		T cs = std::cos(m_Pa);
		T r2 = T(1) - (cs - T(1)) / (cs + std::cos(T(M_2PI) / Zeps(m_Q)));
		m_R = (r2 > 0) ? T(1) / sqrt(r2) : T(1);
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
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "hypertile2_p", 3));
		m_Params.push_back(ParamWithName<T>(&m_Q, prefix + "hypertile2_q", 7));
		m_Params.push_back(ParamWithName<T>(true, &m_Pa, prefix + "hypertile2_pa"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_R, prefix + "hypertile2_r"));
	}

private:
	T m_P;
	T m_Q;
	T m_Pa;//Precalc.
	T m_R;
};

/// <summary>
/// Hypertile3D.
/// </summary>
template <typename T>
class Hypertile3DVariation : public ParametricVariation<T>
{
public:
	Hypertile3DVariation(T weight = 1.0) : ParametricVariation<T>("hypertile3D", eVariationId::VAR_HYPERTILE3D, weight, true)
	{
		Init();
	}

	PARVARCOPY(Hypertile3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = helper.m_PrecalcSumSquares + helper.In.z;
		T x2cx = m_C2x * helper.In.x;
		T y2cy = m_C2y * helper.In.y;
		T d = m_Weight / Zeps(m_C2 * r2 + x2cx - y2cy + 1);
		helper.Out.x = d * (helper.In.x * m_S2x - m_Cx * (y2cy - r2 - 1));
		helper.Out.y = d * (helper.In.y * m_S2y + m_Cy * (-x2cx - r2 - 1));
		helper.Out.z = d * (helper.In.z * m_S2z);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string p   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cy  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cz  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r2 = precalcSumSquares + vIn.z;\n"
		   << "\t\treal_t x2cx = " << c2x << " * vIn.x;\n"
		   << "\t\treal_t y2cy = " << c2y << " * vIn.y;\n"
		   << "\t\treal_t d = " << weight << " / Zeps(fma(" << c2 << ", r2, (x2cx - y2cy) + 1));\n"
		   << "\n"
		   << "\t\tvOut.x = d * fma(vIn.x, " << s2x << ", -(" << cx << " * (y2cy - r2 - 1)));\n"
		   << "\t\tvOut.y = d * fma(vIn.y, " << s2y << ", " << cy << " * (-x2cx - r2 - 1));\n"
		   << "\t\tvOut.z = d * (vIn.z * " << s2z << ");\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T pa = 2 * T(M_PI) / Zeps(m_P);
		T qa = 2 * T(M_PI) / Zeps(m_Q);
		T r = -(std::cos(pa) - 1) / Zeps(std::cos(pa) + std::cos(qa));
		T na = m_N * pa;

		if (r > 0)
			r = 1 / Zeps(std::sqrt(1 + r));
		else
			r = 1;

		m_Cx = r * std::cos(na);
		m_Cy = r * std::sin(na);
		m_C2 = SQR(m_Cx) + SQR(m_Cy);
		m_C2x = 2 * m_Cx;
		m_C2y = 2 * m_Cy;
		m_S2x = 1 + SQR(m_Cx) - SQR(m_Cy);
		m_S2y = 1 + SQR(m_Cy) - SQR(m_Cx);
		m_S2z = 1 - SQR(m_Cy) - SQR(m_Cx);
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
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "hypertile3D_p", 3, eParamType::INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(&m_Q, prefix + "hypertile3D_q", 7, eParamType::INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(&m_N, prefix + "hypertile3D_n", 0, eParamType::INTEGER));
		m_Params.push_back(ParamWithName<T>(true, &m_Cx, prefix + "hypertile3D_cx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cy, prefix + "hypertile3D_cy"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cz, prefix + "hypertile3D_cz"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2x, prefix + "hypertile3D_s2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2y, prefix + "hypertile3D_s2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2z, prefix + "hypertile3D_s2z"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2x, prefix + "hypertile3D_c2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2y, prefix + "hypertile3D_c2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2z, prefix + "hypertile3D_c2z"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2, prefix + "hypertile3D_c2"));
	}

private:
	T m_P;
	T m_Q;
	T m_N;
	T m_Cx;//Precalc.
	T m_Cy;
	T m_Cz;
	T m_S2x;
	T m_S2y;
	T m_S2z;
	T m_C2x;
	T m_C2y;
	T m_C2z;
	T m_C2;
};

/// <summary>
/// Hypertile3D1.
/// </summary>
template <typename T>
class Hypertile3D1Variation : public ParametricVariation<T>
{
public:
	Hypertile3D1Variation(T weight = 1.0) : ParametricVariation<T>("hypertile3D1", eVariationId::VAR_HYPERTILE3D1, weight, true)
	{
		Init();
	}

	PARVARCOPY(Hypertile3D1Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T temp = rand.Crand() * m_Pa;
		T cx = m_R * std::cos(temp);
		T cy = m_R * std::sin(temp);
		T s2x = 1 + SQR(cx) - SQR(cy);
		T s2y = 1 + SQR(cy) - SQR(cx);
		T r2 = helper.m_PrecalcSumSquares + SQR(helper.In.z);
		T x2cx = 2 * cx * helper.In.x;
		T y2cy = 2 * cy * helper.In.y;
		T d = m_Weight / Zeps(m_C2 * r2 + x2cx - y2cy + 1);
		helper.Out.x = d * (helper.In.x * s2x - cx * (y2cy - r2 - 1));
		helper.Out.y = d * (helper.In.y * s2y + cy * (-x2cx - r2 - 1));
		helper.Out.z = d * (helper.In.z * m_S2z);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string p   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pa  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string r   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t temp = MwcNextCrand(mwc) * " << pa << ";\n"
		   << "\t\treal_t cx = " << r << " * cos(temp);\n"
		   << "\t\treal_t cy = " << r << " * sin(temp);\n"
		   << "\t\treal_t s2x = fma(cx, cx, (real_t)(1.0)) - SQR(cy);\n"
		   << "\t\treal_t s2y = fma(cy, cy, (real_t)(1.0)) - SQR(cx);\n"
		   << "\t\treal_t r2 = precalcSumSquares + SQR(vIn.z);\n"
		   << "\t\treal_t x2cx = 2 * cx * vIn.x;\n"
		   << "\t\treal_t y2cy = 2 * cy * vIn.y;\n"
		   << "\t\treal_t d = " << weight << " / Zeps(fma(" << c2 << ", r2, (x2cx - y2cy) + 1)); \n"
		   << "\n"
		   << "\t\tvOut.x = d * fma(vIn.x, s2x, -(cx * (y2cy - r2 - 1)));\n"
		   << "\t\tvOut.y = d * fma(vIn.y, s2y, cy * (-x2cx - r2 - 1));\n"
		   << "\t\tvOut.z = d * (vIn.z * " << s2z << ");\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T pa = M_2PI / Zeps(m_P);
		T qa = M_2PI / Zeps(m_Q);
		T r = -(std::cos(pa) - 1) / Zeps(std::cos(pa) + std::cos(qa));

		if (r > 0)
			r = 1 / Zeps(std::sqrt(1 + r));
		else
			r = 1;

		m_Pa = pa;
		m_R = r;
		m_C2 = SQR(r);
		m_S2z = 1 - m_C2;
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
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "hypertile3D1_p", 3, eParamType::INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(&m_Q, prefix + "hypertile3D1_q", 7, eParamType::INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(true, &m_Pa, prefix + "hypertile3D1_pa"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_R, prefix + "hypertile3D1_r"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2, prefix + "hypertile3D1_c2"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2z, prefix + "hypertile3D1_s2z"));
	}

private:
	T m_P;
	T m_Q;
	T m_Pa;//Precalc.
	T m_R;
	T m_C2;
	T m_S2z;
};

/// <summary>
/// Hypertile3D2.
/// </summary>
template <typename T>
class Hypertile3D2Variation : public ParametricVariation<T>
{
public:
	Hypertile3D2Variation(T weight = 1.0) : ParametricVariation<T>("hypertile3D2", eVariationId::VAR_HYPERTILE3D2, weight, true)
	{
		Init();
	}

	PARVARCOPY(Hypertile3D2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = helper.m_PrecalcSumSquares + SQR(helper.In.z);
		T x2cx = m_C2x * helper.In.x;
		T x = helper.In.x * m_S2x - m_Cx * (-r2 - 1);
		T y = helper.In.y * m_S2y;
		T vr = m_Weight / (m_C2 * r2 + x2cx + 1);
		T temp = rand.Crand() * m_Pa;
		T sina = std::sin(temp);
		T cosa = std::cos(temp);
		helper.Out.x = vr * (x * cosa + y * sina);
		helper.Out.y = vr * (y * cosa - x * sina);
		helper.Out.z = vr * (helper.In.z * m_S2z);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string p   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pa  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r2 = precalcSumSquares + SQR(vIn.z);\n"
		   << "\t\treal_t x2cx = " << c2x << " * vIn.x;\n"
		   << "\t\treal_t x = fma(vIn.x, " << s2x << ", -(" << cx << " * (-r2 - (real_t)(1.0))));\n"
		   << "\t\treal_t y = vIn.y * " << s2y << ";\n"
		   << "\t\treal_t vr = " << weight << " / fma(" << c2 << ", r2, x2cx + (real_t)(1.0));\n"
		   << "\t\treal_t temp = MwcNextCrand(mwc) * " << pa << ";\n"
		   << "\t\treal_t sina = sin(temp);\n"
		   << "\t\treal_t cosa = cos(temp);\n"
		   << "\n"
		   << "\t\tvOut.x = vr * fma(x, cosa, y * sina);\n"
		   << "\t\tvOut.y = vr * fma(y, cosa, -(x * sina));\n"
		   << "\t\tvOut.z = vr * (vIn.z * " << s2z << ");\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T pa = M_2PI / Zeps(m_P);
		T qa = M_2PI / Zeps(m_Q);
		T r = -(std::cos(pa) - 1) / Zeps(std::cos(pa) + std::cos(qa));

		if (r > 0)
			r = 1 / Zeps(std::sqrt(1 + r));
		else
			r = 1;

		m_Pa = pa;
		m_Cx = r;
		m_C2 = SQR(m_Cx);
		m_C2x = 2 * m_Cx;
		m_S2x = 1 + SQR(m_Cx);
		m_S2y = 1 - SQR(m_Cx);
		m_S2z = 1 - SQR(m_Cx);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "hypertile3D2_p", 3, eParamType::INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(&m_Q, prefix + "hypertile3D2_q", 7, eParamType::INTEGER, 3, T(0x7fffffff)));
		m_Params.push_back(ParamWithName<T>(true, &m_Pa, prefix + "hypertile3D2_pa"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cx, prefix + "hypertile3D2_cx"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2, prefix + "hypertile3D2_c2"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2x, prefix + "hypertile3D2_c2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2x, prefix + "hypertile3D2_s2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2y, prefix + "hypertile3D2_s2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2z, prefix + "hypertile3D2_s2z"));
	}

private:
	T m_P;
	T m_Q;
	T m_Pa;//Precalc.
	T m_Cx;
	T m_C2;
	T m_C2x;
	T m_S2x;
	T m_S2y;
	T m_S2z;
};

/// <summary>
/// IDisc.
/// </summary>
template <typename T>
class IDiscVariation : public ParametricVariation<T>
{
public:
	IDiscVariation(T weight = 1.0) : ParametricVariation<T>("idisc", eVariationId::VAR_IDISC, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(IDiscVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = T(M_PI) / (helper.m_PrecalcSqrtSumSquares + 1);
		T s = std::sin(a);
		T c = std::cos(a);
		T r = helper.m_PrecalcAtanyx * m_V;
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
		string v = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.
		ss << "\t{\n"
		   << "\t\treal_t a = MPI / (precalcSqrtSumSquares + 1);\n"
		   << "\t\treal_t s = sin(a);\n"
		   << "\t\treal_t c = cos(a);\n"
		   << "\t\treal_t r = precalcAtanyx * " << v << ";\n"
		   << "\n"
		   << "\t\tvOut.x = r * c;\n"
		   << "\t\tvOut.y = r * s;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_V = m_Weight * T(M_1_PI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_V, prefix + "idisc_v"));//Precalcs only, no params.
	}

private:
	T m_V;//Precalcs only, no params.
};

/// <summary>
/// Julian2.
/// </summary>
template <typename T>
class Julian2Variation : public ParametricVariation<T>
{
public:
	Julian2Variation(T weight = 1.0) : ParametricVariation<T>("julian2", eVariationId::VAR_JULIAN2, weight)
	{
		Init();
	}

	PARVARCOPY(Julian2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a;
		T x = m_A * helper.In.x + m_B * helper.In.y + m_E;
		T y = m_C * helper.In.x + m_D * helper.In.y + m_F;
		T r = m_Weight * std::pow(SQR(x) + SQR(y), m_Cn);

		if (Compat::m_Compat)//You need to implement this on the gpu and make sure this is even what the original does. Also search all julias that use "root".
		{
			a = (std::atan2(y, x) + M_2PI * rand.Rand(size_t(m_AbsN))) / m_Power;
		}
		else
		{
            auto root = int(m_AbsN * rand.Frand01<T>());
			a = (std::atan2(y, x) + root * M_2PI) / m_Power;
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
		string a     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string e     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string f     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absn  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cn    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a;\n"
		   << "\t\treal_t x = fma(" << a << ", vIn.x, fma(" << b << ", vIn.y, " << e << "));\n"
		   << "\t\treal_t y = fma(" << c << ", vIn.x, fma(" << d << ", vIn.y, " << f << "));\n"
		   << "\t\treal_t r = " << weight << " * pow(fma(x, x, SQR(y)), " << cn << ");\n";

		if (Compat::m_Compat)
			ss << "\t\ta = fma(M_2PI, (real_t)MwcNextRange(mwc, (uint)" << absn << "), atan2(y, x)) / " << power << ";\n";
		else
			ss << "\t\tint root = (int)(" << absn << " * MwcNext01(mwc));\n"
			   << "\t\ta = (atan2(y, x) + root + M_2PI) / " << power << ";\n";

		ss << "\t\tvOut.x = r * cos(a);\n"
		   << "\t\tvOut.y = r * sin(a);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		if (m_Power == 0)
			m_Power = 2;

		if (Compat::m_Compat)
			m_AbsN = T(int(abs(m_Power)));
		else
			m_AbsN = std::abs(m_Power);

		m_Cn = m_Dist / m_Power / 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "julian2_a", 1));
		m_Params.push_back(ParamWithName<T>(&m_B, prefix + "julian2_b"));
		m_Params.push_back(ParamWithName<T>(&m_C, prefix + "julian2_c"));
		m_Params.push_back(ParamWithName<T>(&m_D, prefix + "julian2_d", 1));
		m_Params.push_back(ParamWithName<T>(&m_E, prefix + "julian2_e"));
		m_Params.push_back(ParamWithName<T>(&m_F, prefix + "julian2_f"));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "julian2_power", 2, eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Dist, prefix + "julian2_dist", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsN, prefix + "julian2_absn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix + "julian2_cn"));
	}

private:
	T m_A;
	T m_B;
	T m_C;
	T m_D;
	T m_E;
	T m_F;
	T m_Power;
	T m_Dist;
	T m_AbsN;//Precalc.
	T m_Cn;
};

/// <summary>
/// JuliaQ.
/// </summary>
template <typename T>
class JuliaQVariation : public ParametricVariation<T>
{
public:
	JuliaQVariation(T weight = 1.0) : ParametricVariation<T>("juliaq", eVariationId::VAR_JULIAQ, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(JuliaQVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanyx * m_InvPower + rand.Crand() * m_InvPower2pi;
		T sina = std::sin(a);
		T cosa = std::cos(a);
		T r = m_Weight * std::pow(helper.m_PrecalcSumSquares, m_HalfInvPower);
		helper.Out.x = r * cosa;
		helper.Out.y = r * sina;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string divisor      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfInvPower = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invPower     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invPower2Pi  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a = fma(precalcAtanyx, " << invPower << ", MwcNextCrand(mwc) * " << invPower2Pi << ");\n"
		   << "\t\treal_t sina = sin(a);\n"
		   << "\t\treal_t cosa = cos(a);\n"
		   << "\t\treal_t r = " << weight << " * pow(precalcSumSquares, " << halfInvPower << ");\n"
		   << "\n"
		   << "\t\tvOut.x = r * cosa;\n"
		   << "\t\tvOut.y = r * sina;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		auto p = Zeps(m_Power);
		m_HalfInvPower = T(0.5) * (m_Divisor / p);
		m_InvPower = m_Divisor / p;
		m_InvPower2pi = M_2PI / p;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "juliaq_power", 3, eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Divisor, prefix + "juliaq_divisor", 2, eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_HalfInvPower, prefix + "juliaq_half_inv_power"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_InvPower, prefix + "juliaq_inv_power"));
		m_Params.push_back(ParamWithName<T>(true, &m_InvPower2pi, prefix + "juliaq_inv_power_2pi"));
	}

private:
	T m_Power;
	T m_Divisor;
	T m_HalfInvPower;//Precalc.
	T m_InvPower;
	T m_InvPower2pi;
};

/// <summary>
/// Murl.
/// </summary>
template <typename T>
class MurlVariation : public ParametricVariation<T>
{
public:
	MurlVariation(T weight = 1.0) : ParametricVariation<T>("murl", eVariationId::VAR_MURL, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(MurlVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = helper.m_PrecalcAtanyx * m_Power;
		T sina = std::sin(angle);
		T cosa = std::cos(angle);
		T r = m_Cp * std::pow(helper.m_PrecalcSumSquares, m_P2);
		T re = r * cosa + 1;
		T im = r * sina;
		T r1 = m_Vp / (SQR(re) + SQR(im));
		helper.Out.x = r1 * (helper.In.x * re + helper.In.y * im);
		helper.Out.y = r1 * (helper.In.y * re - helper.In.x * im);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string c = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cp = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string p2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vp = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t angle = precalcAtanyx * " << power << ";\n"
		   << "\t\treal_t sina = sin(angle);\n"
		   << "\t\treal_t cosa = cos(angle);\n"
		   << "\t\treal_t r = " << cp << " * pow(precalcSumSquares, " << p2 << ");\n"
		   << "\t\treal_t re = fma(r, cosa, (real_t)(1.0));\n"
		   << "\t\treal_t im = r * sina;\n"
		   << "\t\treal_t r1 = " << vp << " / fma(re, re, SQR(im));\n"
		   << "\n"
		   << "\t\tvOut.x = r1 * fma(vIn.x, re, vIn.y * im);\n"
		   << "\t\tvOut.y = r1 * fma(vIn.y, re, -(vIn.x * im));\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		if (m_Power != 1)
			m_Cp = m_C / (m_Power - 1);
		else
			m_Cp = m_C;

		m_P2 = m_Power / 2;
		m_Vp = m_Weight * (m_Cp + 1);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_C, prefix + "murl_c"));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "murl_power", 2, eParamType::REAL, 2));
		m_Params.push_back(ParamWithName<T>(true, &m_Cp, prefix + "murl_cp"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_P2, prefix + "murl_p2"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vp, prefix + "murl_vp"));
	}

private:
	T m_C;
	T m_Power;
	T m_Cp;//Precalc.
	T m_P2;
	T m_Vp;
};

/// <summary>
/// Murl2.
/// </summary>
template <typename T>
class Murl2Variation : public ParametricVariation<T>
{
public:
	Murl2Variation(T weight = 1.0) : ParametricVariation<T>("murl2", eVariationId::VAR_MURL2, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Murl2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = helper.m_PrecalcAtanyx * m_Power;
		T sina = std::sin(angle);
		T cosa = std::cos(angle);
		T r = m_C * std::pow(helper.m_PrecalcSumSquares, m_P2);
		T re = r * cosa + 1;
		T im = r * sina;
		r = std::pow(SQR(re) + SQR(im), m_InvP);
		angle = std::atan2(im, re) * m_InvP2;
		sina = std::sin(angle);
		cosa = std::cos(angle);
		re = r * cosa;
		im = r * sina;
		T r1 = m_Vp / SQR(r);
		helper.Out.x = r1 * (helper.In.x * re + helper.In.y * im);
		helper.Out.y = r1 * (helper.In.y * re - helper.In.x * im);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string c = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string p2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invp = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invp2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vp = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t angle = precalcAtanyx * " << power << ";\n"
		   << "\t\treal_t sina = sin(angle);\n"
		   << "\t\treal_t cosa = cos(angle);\n"
		   << "\t\treal_t r = " << c << " * pow(precalcSumSquares, " << p2 << ");\n"
		   << "\t\treal_t re = fma(r, cosa, (real_t)(1.0));\n"
		   << "\t\treal_t im = r * sina;\n"
		   << "\n"
		   << "\t\tr = pow(fma(re, re, SQR(im)), " << invp << ");\n"
		   << "\t\tangle = atan2(im, re) * " << invp2 << ";\n"
		   << "\t\tsina = sin(angle);\n"
		   << "\t\tcosa = cos(angle);\n"
		   << "\t\tre = r * cosa;\n"
		   << "\t\tim = r * sina;\n"
		   << "\n"
		   << "\t\treal_t r1 = " << vp << " / SQR(r);\n"
		   << "\n"
		   << "\t\tvOut.x = r1 * fma(vIn.x, re, vIn.y * im);\n"
		   << "\t\tvOut.y = r1 * fma(vIn.y, re, -(vIn.x * im));\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_P2 = m_Power / 2;
		m_InvP = 1 / m_Power;
		m_InvP2 = 2 / m_Power;

		if (m_C == -1)
			m_Vp = 0;
		else
			m_Vp = m_Weight * std::pow(m_C + 1, 2 / m_Power);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_C, prefix + "murl2_c", 0, eParamType::REAL, -1, 1));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "murl2_power", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_P2, prefix + "murl2_p2"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_InvP, prefix + "murl2_invp"));
		m_Params.push_back(ParamWithName<T>(true, &m_InvP2, prefix + "murl2_invp2"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vp, prefix + "murl2_vp"));
	}

private:
	T m_C;
	T m_Power;
	T m_P2;//Precalc.
	T m_InvP;
	T m_InvP2;
	T m_Vp;
};

/// <summary>
/// NPolar.
/// </summary>
template <typename T>
class NPolarVariation : public ParametricVariation<T>
{
public:
	NPolarVariation(T weight = 1.0) : ParametricVariation<T>("npolar", eVariationId::VAR_NPOLAR, weight, true, false, false, true, true)
	{
		Init();
	}

	PARVARCOPY(NPolarVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x, y;

		if (Compat::m_Compat)
		{
			if (m_IsOdd != 0)
			{
				T angle = (helper.m_PrecalcAtanyx + M_2PI * rand.Rand(size_t(m_AbsN))) / m_Nnz;
				T r = m_Weight * std::pow(helper.m_PrecalcSumSquares, m_Cn) * m_Parity;
				x = std::cos(angle) * r;
				y = std::sin(angle) * r;
			}
			else
			{
				x = m_Vvar * helper.m_PrecalcAtanxy;
				y = m_Vvar2 * std::log(helper.m_PrecalcSumSquares);
				T angle = (std::atan2(y, x) + M_2PI * rand.Rand(size_t(m_AbsN))) / m_Nnz;
				T r = m_Weight * std::pow(SQR(x) + SQR(y), m_Cn);
				T sina = std::sin(angle) * r;
				T cosa = std::cos(angle) * r;
				x = m_Vvar2 * std::log(SQR(cosa) + SQR(sina));
				y = m_Vvar * std::atan2(cosa, sina);
			}
		}
		else
		{
			if (m_IsOdd != 0)
			{
				T angle = (helper.m_PrecalcAtanyx + M_2PI * rand.Rand(size_t(m_AbsN))) / m_Nnz;
				T r = m_Weight * std::pow(helper.m_PrecalcSumSquares, m_Cn);
				x = std::cos(angle) * r;
				y = std::sin(angle) * r;
			}
			else
			{
				x = m_Vvar * helper.m_PrecalcAtanxy;
				y = m_Vvar2 * std::log(helper.m_PrecalcSumSquares);
				T angle = (std::atan2(y, x) + M_2PI * rand.Rand(size_t(m_AbsN))) / m_Nnz;
				T r = m_Weight * std::pow(SQR(x) + SQR(y), m_Cn) * m_Parity;
				T sina = std::sin(angle) * r;
				T cosa = std::cos(angle) * r;
				x = m_Vvar2 * 2 * std::log(r);
				y = m_Vvar * std::atan2(cosa, sina);
			}
		}

		helper.Out.x = x;
		helper.Out.y = y;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string parity = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string nnz    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vvar   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vvar2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absn   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cn     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string isOdd  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x, y;\n"
		   << "\t\treal_t vvar = " << vvar << ";\n"
		   << "\t\treal_t vvar2 = " << vvar2 << ";\n"
		   << "\n";

		if (Compat::m_Compat)
		{
			ss << "\t\tif (" << isOdd << " != 0)\n"
			   << "\t\t{\n"
			   << "\t\t	real_t angle = (precalcAtanyx + M_2PI * MwcNextRange(mwc, (uint)" << absn << ")) / " << nnz << ";\n"
			   << "\t\t	real_t r = " << weight << " * pow(precalcSumSquares, " << cn << ") * " << parity << ";\n"
			   << "\t\t	x = cos(angle) * r;\n"
			   << "\t\t	y = sin(angle) * r;\n"
			   << "\t\t}\n"
			   << "\t\telse\n"
			   << "\t\t{\n"
			   << "\t\t	x = vvar * precalcAtanxy;\n"
			   << "\t\t	y = vvar2 * log(precalcSumSquares);\n"
			   << "\t\t	real_t angle = (atan2(y, x) + M_2PI * MwcNextRange(mwc, (uint)" << absn << ")) / " << nnz << ";\n"
			   << "\t\t	real_t r = " << weight << " * pow(SQR(x) + SQR(y), " << cn << ");\n"
			   << "\t\t	real_t sina = sin(angle) * r;\n"
			   << "\t\t	real_t cosa = cos(angle) * r;\n"
			   << "\t\t	x = vvar2 * log(SQR(cosa) + SQR(sina));\n"
			   << "\t\t	y = vvar * atan2(cosa, sina);\n"
			   << "\t\t}\n";
		}
		else
		{
			ss << "\t\tif (" << isOdd << " != 0)\n"
			   << "\t\t{\n"
			   << "\t\t	real_t angle = (precalcAtanyx + M_2PI * MwcNextRange(mwc, (uint)" << absn << ")) / " << nnz << ";\n"
			   << "\t\t	real_t r = " << weight << " * pow(precalcSumSquares, " << cn << ");\n"
			   << "\t\t	x = cos(angle) * r;\n"
			   << "\t\t	y = sin(angle) * r;\n"
			   << "\t\t}\n"
			   << "\t\telse\n"
			   << "\t\t{\n"
			   << "\t\t	x = vvar * precalcAtanxy;\n"
			   << "\t\t	y = vvar2 * log(precalcSumSquares);\n"
			   << "\t\t	real_t angle = (atan2(y, x) + M_2PI * MwcNextRange(mwc, (uint)" << absn << ")) / " << nnz << ";\n"
			   << "\t\t	real_t r = " << weight << " * pow(SQR(x) + SQR(y), " << cn << ") * " << parity << ";\n"
			   << "\t\t	real_t sina = sin(angle) * r;\n"
			   << "\t\t	real_t cosa = cos(angle) * r;\n"
			   << "\t\t	x = vvar2 * (real_t)(2.0) * log(r);\n"
			   << "\t\t	y = vvar * atan2(cosa, sina);\n"
			   << "\t\t}\n";
		}

		ss  << "\n"
			<< "\t\tvOut.x = x;\n"
			<< "\t\tvOut.y = y;\n"
			<< "\t\tvOut.z = " << DefaultZCl()
			<< "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Nnz = m_N == 0 ? 1 : m_N;
		m_Vvar = m_Weight / T(M_PI);
		m_Vvar2 = m_Vvar * T(0.5);
		m_AbsN = std::abs(m_Nnz);
		m_Cn = 1 / m_Nnz / 2;
		m_IsOdd = std::fmod(std::abs(m_Parity), T(2.0));
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Parity, prefix + "npolar_parity", 0));
		m_Params.push_back(ParamWithName<T>(&m_N, prefix + "npolar_n", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Nnz, prefix + "npolar_nnz"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Vvar, prefix + "npolar_vvar"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vvar2, prefix + "npolar_vvar_2"));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsN, prefix + "npolar_absn"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix + "npolar_cn"));
		m_Params.push_back(ParamWithName<T>(true, &m_IsOdd, prefix + "npolar_isodd"));
	}

private:
	T m_Parity;
	T m_N;
	T m_Nnz;//Precalc.
	T m_Vvar;
	T m_Vvar2;
	T m_AbsN;
	T m_Cn;
	T m_IsOdd;
};

/// <summary>
/// Ortho.
/// </summary>
template <typename T>
class OrthoVariation : public ParametricVariation<T>
{
public:
	OrthoVariation(T weight = 1.0) : ParametricVariation<T>("ortho", eVariationId::VAR_ORTHO, weight, true, true, true, false, false)
	{
		Init();
	}

	PARVARCOPY(OrthoVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r, a;
		T xo;
		T ro;
		T c, s;
		T x, y, tc, ts;
		T theta;
		r = helper.m_PrecalcSumSquares;

		if (r < 1)
		{
			if (helper.In.x >= 0)
			{
				xo = (r + 1) / Zeps(2 * helper.In.x);
				ro = std::sqrt(Sqr(helper.In.x - xo) + SQR(helper.In.y));
				theta = std::atan2(T(1), ro);
				a = fmod(m_In * theta + std::atan2(helper.In.y, xo - helper.In.x) + theta, 2 * theta) - theta;
				sincos(a, &s, &c);
				helper.Out.x = m_Weight * (xo - c * ro);
				helper.Out.y = m_Weight * s * ro;
			}
			else
			{
				xo = -(r + 1) / (2 * helper.In.x);
				ro = std::sqrt(Sqr(-helper.In.x - xo) + SQR(helper.In.y));
				theta = std::atan2(T(1), ro);
				a = fmod(m_In * theta + std::atan2(helper.In.y, xo + helper.In.x) + theta, 2 * theta) - theta;
				sincos(a, &s, &c);
				helper.Out.x = -(m_Weight * (xo - c * ro));
				helper.Out.y = m_Weight * s * ro;
			}
		}
		else
		{
			r = 1 / std::sqrt(r);
			ts = helper.m_PrecalcSina;
			tc = helper.m_PrecalcCosa;
			x = r * tc;
			y = r * ts;

			if (x >= 0)
			{
				xo = (SQR(x) + SQR(y) + 1) / Zeps(2 * x);
				ro = std::sqrt(Sqr(x - xo) + SQR(y));
				theta = std::atan2(T(1), ro);
				a = fmod(m_Out * theta + std::atan2(y, xo - x) + theta, 2 * theta) - theta;
				sincos(a, &s, &c);
				x = (xo - c * ro);
				y = s * ro;
				theta = std::atan2(y, x);
				sincos(theta, &ts, &tc);
				r = 1 / std::sqrt(SQR(x) + SQR(y));
				helper.Out.x = m_Weight * r * tc;
				helper.Out.y = m_Weight * r * ts;
			}
			else
			{
				xo = -(SQR(x) + SQR(y) + 1) / (2 * x);
				ro = std::sqrt(Sqr(-x - xo) + SQR(y));
				theta = std::atan2(T(1), ro);
				a = fmod(m_Out * theta + std::atan2(y, xo + x) + theta, 2 * theta) - theta;
				sincos(a, &s, &c);
				x = (xo - c * ro);
				y = s * ro;
				theta = std::atan2(y, x);
				sincos(theta, &ts, &tc);
				r = 1 / std::sqrt(SQR(x) + SQR(y));
				helper.Out.x = -(m_Weight * r * tc);
				helper.Out.y = m_Weight * r * ts;
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
		string in = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string out = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r, a;\n"
		   << "\t\treal_t xo;\n"
		   << "\t\treal_t ro;\n"
		   << "\t\treal_t c,s;\n"
		   << "\t\treal_t x, y, tc, ts;\n"
		   << "\t\treal_t theta;\n"
		   << "\n"
		   << "\t\tr = precalcSumSquares;\n"
		   << "\n"
		   << "\t\tif (r < 1)\n"
		   << "\t\t{\n"
		   << "\t\t	real_t y2 = SQR(vIn.y);\n"
		   << "\t\t	if (vIn.x >= 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		xo = (r + 1) / Zeps(2 * vIn.x);\n"
		   << "\t\t		real_t xmx = vIn.x - xo;\n"
		   << "\t\t		ro = sqrt(fma(xmx, xmx, y2));\n"
		   << "\t\t		theta = atan2(1, ro);\n"
		   << "\t\t		a = fmod(fma(" << in << ", theta, atan2(vIn.y, xo - vIn.x) + theta), 2 * theta) - theta;\n"
		   << "\t\t		s = sin(a);\n"
		   << "\t\t		c = cos(a);\n"
		   << "\n"
		   << "\t\t		vOut.x = " << weight << " * (xo - c * ro);\n"
		   << "\t\t		vOut.y = " << weight << " * s * ro;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		xo = - (r + 1) / (2 * vIn.x);\n"
		   << "\t\t		real_t mxmx = -vIn.x - xo;\n"
		   << "\t\t		ro = sqrt(fma(mxmx, mxmx, y2));\n"
		   << "\t\t		theta = atan2(1 , ro);\n"
		   << "\t\t		a = fmod(fma(" << in << ", theta, atan2(vIn.y, xo + vIn.x) + theta), 2 * theta) - theta;\n"
		   << "\t\t		s = sin(a);\n"
		   << "\t\t		c = cos(a);\n"
		   << "\n"
		   << "\t\t		vOut.x = -(" << weight << " * (xo - c * ro));\n"
		   << "\t\t		vOut.y = " << weight << " * s * ro;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	r = 1 / sqrt(r);\n"
		   << "\t\t	ts = precalcSina;\n"
		   << "\t\t	tc = precalcCosa;\n"
		   << "\t\t	x = r * tc;\n"
		   << "\t\t	y = r * ts;\n"
		   << "\t\t	real_t x2 = SQR(x);\n"
		   << "\t\t	real_t y2 = SQR(y);\n"
		   << "\t\t	real_t x2y2 = x2 + y2;\n"
		   << "\n"
		   << "\t\t	if (x >= 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		xo = (x2y2 + 1) / Zeps(2 * x);\n"
		   << "\t\t		real_t xmx = x - xo;\n"
		   << "\t\t		ro = sqrt(fma(xmx, xmx, y2));\n"
		   << "\t\t		theta = atan2(1, ro);\n"
		   << "\t\t		a = fmod(fma(" << out << ", theta, atan2(y, xo - x) + theta), 2 * theta) - theta;\n"
		   << "\t\t		s = sin(a);\n"
		   << "\t\t		c = cos(a);\n"
		   << "\n"
		   << "\t\t		x = (xo - c * ro);\n"
		   << "\t\t		y =  s * ro;\n"
		   << "\t\t		theta = atan2(y, x);\n"
		   << "\t\t		ts = sin(theta);\n"
		   << "\t\t		tc = cos(theta);\n"
		   << "\t\t		r = 1 / sqrt(fma(x, x, SQR(y)));\n"
		   << "\n"
		   << "\t\t		vOut.x = " << weight << " * r * tc;\n"
		   << "\t\t		vOut.y = " << weight << " * r * ts;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		xo = -(x2y2 + 1) / (2 * x);\n"
		   << "\t\t		real_t mxmx = -x - xo;\n"
		   << "\t\t		ro = sqrt(fma(mxmx, mxmx, y2));\n"
		   << "\t\t		theta = atan2(1 , ro);\n"
		   << "\t\t		a = fmod(fma(" << out << ", theta, atan2(y, xo + x) + theta), 2 * theta) - theta;\n"
		   << "\t\t		s = sin(a);\n"
		   << "\t\t		c = cos(a);\n"
		   << "\n"
		   << "\t\t		x = (xo - c * ro);\n"
		   << "\t\t		y =  s * ro;\n"
		   << "\t\t		theta = atan2(y, x);\n"
		   << "\t\t		ts = sin(theta);\n"
		   << "\t\t		tc = cos(theta);\n"
		   << "\t\t		r = 1 / sqrt(fma(x, x, SQR(y)));\n"
		   << "\n"
		   << "\t\t		vOut.x = -(" << weight << " * r * tc);\n"
		   << "\t\t		vOut.y = " << weight << " * r * ts;\n"
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
		m_Params.push_back(ParamWithName<T>(&m_In, prefix + "ortho_in", 0, eParamType::REAL_CYCLIC, T(-M_PI), T(M_PI)));
		m_Params.push_back(ParamWithName<T>(&m_Out, prefix + "ortho_out", 0, eParamType::REAL_CYCLIC, T(-M_PI), T(M_PI)));
	}

private:
	T m_In;
	T m_Out;
};

/// <summary>
/// Poincare.
/// </summary>
template <typename T>
class PoincareVariation : public ParametricVariation<T>
{
public:
	PoincareVariation(T weight = 1.0) : ParametricVariation<T>("poincare", eVariationId::VAR_POINCARE, weight)
	{
		Init();
	}

	PARVARCOPY(PoincareVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xmc1x = helper.In.x - m_C1x;
		T ymc1y = helper.In.y - m_C1y;
		T den = Zeps(SQR(xmc1x) + SQR(ymc1y));
		T c1r2 = SQR(m_C1r);
		T x = m_C1x + (c1r2 * xmc1x) / den;
		T y = m_C1y + (c1r2 * ymc1y) / den;
		T xmc2x = x - m_C2x;
		T ymc2y = y - m_C2y;
		T c2r2 = SQR(m_C2r);
		den = Zeps(SQR(xmc2x) + SQR(ymc2y));
		helper.Out.x = m_C2x + (c2r2 * xmc2x) / den;
		helper.Out.y = m_C2y + (c2r2 * ymc2y) / den;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string c1r = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1a = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2r = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2a = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1d = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2d = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t xmc1x = vIn.x - " << c1x << ";\n"
		   << "\t\treal_t ymc1y = vIn.y - " << c1y << ";\n"
		   << "\t\treal_t den = Zeps(fma(xmc1x, xmc1x, SQR(ymc1y)));\n"
		   << "\t\treal_t c1r2 = SQR(" << c1r << ");\n"
		   << "\t\treal_t x = " << c1x << " + (c1r2 * xmc1x) / den;\n"
		   << "\t\treal_t y = " << c1y << " + (c1r2 * ymc1y) / den;\n"
		   << "\n"
		   << "\t\treal_t xmc2x = x - " << c2x << ";\n"
		   << "\t\treal_t ymc2y = y - " << c2y << ";\n"
		   << "\t\treal_t c2r2 = SQR(" << c2r << ");\n"
		   << "\t\tden = Zeps(fma(xmc2x, xmc2x, SQR(ymc2y)));\n"
		   << "\t\tvOut.x = " << c2x << " + (c2r2 * xmc2x) / den;\n"
		   << "\t\tvOut.y = " << c2y << " + (c2r2 * ymc2y) / den;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_C1d = std::sqrt(1 + SQR(m_C1r));
		m_C2d = std::sqrt(1 + SQR(m_C2r));
		m_C1x = m_C1d * std::cos(fmod(m_C1a, T(M_PI)));
		m_C1y = m_C1d * std::sin(fmod(m_C1a, T(M_PI)));
		m_C2x = m_C2d * std::cos(fmod(m_C2a, T(M_PI)));
		m_C2y = m_C2d * std::sin(fmod(m_C2a, T(M_PI)));
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
		m_Params.push_back(ParamWithName<T>(&m_C1r, prefix + "poincare_c1r", 1));
		m_Params.push_back(ParamWithName<T>(&m_C1a, prefix + "poincare_c1a", -1, eParamType::REAL_CYCLIC, T(-M_PI), T(M_PI)));
		m_Params.push_back(ParamWithName<T>(&m_C2r, prefix + "poincare_c2r", 1));
		m_Params.push_back(ParamWithName<T>(&m_C2a, prefix + "poincare_c2a", 1, eParamType::REAL_CYCLIC, T(-M_PI), T(M_PI)));
		m_Params.push_back(ParamWithName<T>(true, &m_C1x, prefix + "poincare_c1x"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_C1y, prefix + "poincare_c1y"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2x, prefix + "poincare_c2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2y, prefix + "poincare_c2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_C1d, prefix + "poincare_c1d"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2d, prefix + "poincare_c2d"));
	}

private:
	T m_C1r;
	T m_C1a;
	T m_C2r;
	T m_C2a;
	T m_C1x;//Precalc.
	T m_C1y;
	T m_C2x;
	T m_C2y;
	T m_C1d;
	T m_C2d;
};

/// <summary>
/// Poincare2.
/// This is intended to mimic the Poincare variation in Chaotica. But we couldn't use the same name because
/// Poincare already exists above.
/// </summary>
template <typename T>
class Poincare2Variation : public ParametricVariation<T>
{
public:
	Poincare2Variation(T weight = 1.0) : ParametricVariation<T>("poincare2", eVariationId::VAR_POINCARE2, weight)
	{
		Init();
	}

	PARVARCOPY(Poincare2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.In.x - m_Cx;
		T b = helper.In.y - m_Cy;
		T c = 1 - m_Cx * helper.In.x - m_Cy * helper.In.y;
		T d = m_Cy * helper.In.x - m_Cx * helper.In.y;
		T num = m_Weight / Zeps(c * c + d * d);
		helper.Out.x = (a * c + b * d) * num;
		helper.Out.y = (b * c - a * d) * num;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string cP = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cQ = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a = vIn.x - " << cX << ";\n"
		   << "\t\treal_t b = vIn.y - " << cY << ";\n"
		   << "\t\treal_t c = 1 - " << cX << " * vIn.x - " << cY << " * vIn.y;\n"
		   << "\t\treal_t d = fma(" << cY << ", vIn.x, -(" << cX << " * vIn.y));\n"
		   << "\t\treal_t num = " << weight << " / Zeps(fma(c, c, d * d));\n"
		   << "\n"
		   << "\t\tvOut.x = fma(a, c, b * d) * num;\n"
		   << "\t\tvOut.y = fma(b, c, -(a * d)) * num;\n"
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
		T a0 = M_2PI / m_PoincareP;
		T dist2 = 1 - (std::cos(a0) - 1) / (std::cos(a0) + std::cos(M_2PI / m_PoincareQ));
		T dist  = (dist2 > 0) ? T(1) / std::sqrt(dist2) : T(1);

		if (1 / m_PoincareP + 1 / m_PoincareQ < T(0.5))
		{
			m_Cx = std::cos(a0) * dist;
			m_Cy = std::sin(a0) * dist;
		}
		else
			m_Cx = m_Cy = 0;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_PoincareP, prefix + "poincare2_p", 3));
		m_Params.push_back(ParamWithName<T>(&m_PoincareQ, prefix + "poincare2_q", 7));
		m_Params.push_back(ParamWithName<T>(true, &m_Cx, prefix  + "poincare2_cx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cy, prefix  + "poincare2_cy"));
	}

private:
	T m_PoincareP;
	T m_PoincareQ;
	T m_Cx;//Precalc.
	T m_Cy;
};

/// <summary>
/// Poincare3D.
/// </summary>
template <typename T>
class Poincare3DVariation : public ParametricVariation<T>
{
public:
	Poincare3DVariation(T weight = 1.0) : ParametricVariation<T>("poincare3D", eVariationId::VAR_POINCARE3D, weight, true)
	{
		Init();
	}

	PARVARCOPY(Poincare3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2 = helper.m_PrecalcSumSquares + SQR(helper.In.z);
		T x2cx = m_C2x * helper.In.x;
		T y2cy = m_C2y * helper.In.y;
		T z2cz = m_C2z * helper.In.z;
		T val = Zeps(m_C2 * r2 - x2cx - y2cy - z2cz + 1);
		T d = m_Weight / val;
		helper.Out.x = d * (helper.In.x * m_S2x + m_Cx * (y2cy + z2cz - r2 - 1));
		helper.Out.y = d * (helper.In.y * m_S2y + m_Cy * (x2cx + z2cz - r2 - 1));
		helper.Out.z = d * (helper.In.z * m_S2z + m_Cz * (y2cy + x2cx - r2 - 1));
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string r   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cy  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cz  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2x = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2y = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s2z = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r2 = fma(vIn.z, vIn.z, precalcSumSquares);\n"
		   << "\t\treal_t x2cx = " << c2x << " * vIn.x;\n"
		   << "\t\treal_t y2cy = " << c2y << " * vIn.y;\n"
		   << "\t\treal_t z2cz = " << c2z << " * vIn.z;\n"
		   << "\t\treal_t val = Zeps(" << c2 << " * r2 - x2cx - y2cy - z2cz + (real_t)(1.0));\n"
		   << "\t\treal_t d = " << weight << " / val;\n"
		   << "\n"
		   << "\t\tvOut.x = d * fma(vIn.x, " << s2x << ", " << cx << " * (y2cy + z2cz - r2 - (real_t)(1.0)));\n"
		   << "\t\tvOut.y = d * fma(vIn.y, " << s2y << ", " << cy << " * (x2cx + z2cz - r2 - (real_t)(1.0)));\n"
		   << "\t\tvOut.z = d * fma(vIn.z, " << s2z << ", " << cz << " * (y2cy + x2cx - r2 - (real_t)(1.0)));\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps" };
	}

	virtual void Precalc() override
	{
		m_Cx = -m_R * std::cos(m_A * T(M_PI_2)) * std::cos(m_B * T(M_PI_2));
		m_Cy = m_R * std::sin(m_A * T(M_PI_2)) * std::cos(m_B * T(M_PI_2));
		m_Cz = -m_R * std::sin(m_B * T(M_PI_2));
		m_C2 = SQR(m_Cx) + SQR(m_Cy) + SQR(m_Cz);
		m_C2x = 2 * m_Cx;
		m_C2y = 2 * m_Cy;
		m_C2z = 2 * m_Cz;
		m_S2x = SQR(m_Cx) - SQR(m_Cy) - SQR(m_Cz) + 1;
		m_S2y = SQR(m_Cy) - SQR(m_Cx) - SQR(m_Cz) + 1;
		m_S2z = SQR(m_Cz) - SQR(m_Cy) - SQR(m_Cx) + 1;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_R, prefix + "poincare3D_r"));
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "poincare3D_a"));
		m_Params.push_back(ParamWithName<T>(&m_B, prefix + "poincare3D_b"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cx, prefix + "poincare3D_cx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cy, prefix + "poincare3D_cy"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cz, prefix + "poincare3D_cz"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2, prefix + "poincare3D_c2"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2x, prefix + "poincare3D_c2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2y, prefix + "poincare3D_c2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_C2z, prefix + "poincare3D_c2z"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2x, prefix + "poincare3D_s2x"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2y, prefix + "poincare3D_s2y"));
		m_Params.push_back(ParamWithName<T>(true, &m_S2z, prefix + "poincare3D_s2z"));
	}

private:
	T m_R;
	T m_A;
	T m_B;
	T m_Cx;//Precalc.
	T m_Cy;
	T m_Cz;
	T m_C2;
	T m_C2x;
	T m_C2y;
	T m_C2z;
	T m_S2x;
	T m_S2y;
	T m_S2z;
};

/// <summary>
/// Polynomial.
/// </summary>
template <typename T>
class PolynomialVariation : public ParametricVariation<T>
{
public:
	PolynomialVariation(T weight = 1.0) : ParametricVariation<T>("polynomial", eVariationId::VAR_POLYNOMIAL, weight)
	{
		Init();
	}

	PARVARCOPY(PolynomialVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xp = std::pow(std::abs(m_Weight) * std::abs(helper.In.x), m_Powx);//Original did not fabs.
		T yp = std::pow(std::abs(m_Weight) * std::abs(helper.In.y), m_Powy);
		helper.Out.x = xp * VarFuncs<T>::Sign(helper.In.x) + m_Lcx * helper.In.x + m_Scx;
		helper.Out.y = yp * VarFuncs<T>::Sign(helper.In.y) + m_Lcy * helper.In.y + m_Scy;
		helper.Out.z = m_Weight * helper.In.z;
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
		string lcx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string lcy  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scy  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t xp = pow(fabs(" << weight << ") * fabs(vIn.x), " << powx << ");\n"
		   << "\t\treal_t yp = pow(fabs(" << weight << ") * fabs(vIn.y), " << powy << ");\n"
		   << "\t\treal_t zp = " << weight << " * vIn.z;\n"
		   << "\n"
		   << "\t\tvOut.x = fma(xp, Sign(vIn.x), fma(" << lcx << ", vIn.x, " << scx << "));\n"
		   << "\t\tvOut.y = fma(yp, Sign(vIn.y), fma(" << lcy << ", vIn.y, " << scy << "));\n"
		   << "\t\tvOut.z = zp;\n"
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Powx, prefix + "polynomial_powx", 1));
		m_Params.push_back(ParamWithName<T>(&m_Powy, prefix + "polynomial_powy", 1));
		m_Params.push_back(ParamWithName<T>(&m_Lcx, prefix + "polynomial_lcx"));
		m_Params.push_back(ParamWithName<T>(&m_Lcy, prefix + "polynomial_lcy"));
		m_Params.push_back(ParamWithName<T>(&m_Scx, prefix + "polynomial_scx"));
		m_Params.push_back(ParamWithName<T>(&m_Scy, prefix + "polynomial_scy"));
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Sign" };
	}

private:
	T m_Powx;
	T m_Powy;
	T m_Lcx;
	T m_Lcy;
	T m_Scx;
	T m_Scy;
};

/// <summary>
/// PSphere.
/// </summary>
template <typename T>
class PSphereVariation : public ParametricVariation<T>
{
public:
	PSphereVariation(T weight = 1.0) : ParametricVariation<T>("psphere", eVariationId::VAR_PSPHERE, weight)
	{
		Init();
	}

	PARVARCOPY(PSphereVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T c0 = helper.In.x * m_Vpi;
		T c1 = helper.In.y * m_Vpi;
		T sinc0, cosc0, sinc1, cosc1;
		sincos(c0, &sinc0, &cosc0);
		sincos(c1, &sinc1, &cosc1);
		helper.Out.x = cosc0 * -sinc1;
		helper.Out.y = sinc0 * cosc1;
		helper.Out.z = cosc1 * m_ZScale;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		int i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string zscale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vpi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t c0 = vIn.x * " << vpi << ";\n"
		   << "\t\treal_t c1 = vIn.y * " << vpi << ";\n"
		   << "\n"
		   << "\t\treal_t sinc0 = sin(c0);\n"
		   << "\t\treal_t cosc0 = cos(c0);\n"
		   << "\t\treal_t sinc1 = sin(c1);\n"
		   << "\t\treal_t cosc1 = cos(c1);\n"
		   << "\n"
		   << "\t\tvOut.x = cosc0 * -sinc1;\n"
		   << "\t\tvOut.y = sinc0 * cosc1;\n"
		   << "\t\tvOut.z = cosc1 * " << zscale << ";\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Vpi = m_Weight * T(M_PI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_ZScale, prefix + "psphere_zscale"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vpi, prefix + "psphere_vpi"));//Precalc.
	}

private:
	T m_ZScale;
	T m_Vpi;//Precalc.
};

/// <summary>
/// Rational3.
/// </summary>
template <typename T>
class Rational3Variation : public ParametricVariation<T>
{
public:
	Rational3Variation(T weight = 1.0) : ParametricVariation<T>("rational3", eVariationId::VAR_RATIONAL3, weight)
	{
		Init();
	}

	PARVARCOPY(Rational3Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T xsqr = helper.In.x * helper.In.x;
		T ysqr = helper.In.y * helper.In.y;
		T xcb = helper.In.x * helper.In.x * helper.In.x;
		T ycb = helper.In.y * helper.In.y * helper.In.y;
		T tr = m_T3 * (xcb - 3 * helper.In.x * ysqr) + m_T2 * (xsqr - ysqr) + m_T1 * helper.In.x + m_Tc;
		T ti = m_T3 * (3 * xsqr * helper.In.y - ycb) + m_T2 * 2 * helper.In.x * helper.In.y + m_T1 * helper.In.y;
		T br = m_B3 * (xcb - 3 * helper.In.x * ysqr) + m_B2 * (xsqr - ysqr) + m_B1 * helper.In.x + m_Bc;
		T bi = m_B3 * (3 * xsqr * helper.In.y - ycb) + m_B2 * 2 * helper.In.x * helper.In.y + m_B1 * helper.In.y;
		T r3den = 1 / Zeps(br * br + bi * bi);
		helper.Out.x = m_Weight * (tr * br + ti * bi) * r3den;
		helper.Out.y = m_Weight * (ti * br - tr * bi) * r3den;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string t3 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string t2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string t1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string tc = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b3 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string bc = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t xsqr = vIn.x * vIn.x;\n"
		   << "\t\treal_t ysqr = vIn.y * vIn.y;\n"
		   << "\t\treal_t xcb  = vIn.x * vIn.x * vIn.x;\n"
		   << "\t\treal_t ycb  = vIn.y * vIn.y * vIn.y;\n"
		   << "\n"
		   << "\t\treal_t tr = fma(" << t3 << ", (xcb - (real_t)(3.0) * vIn.x * ysqr), fma(" << t2 << ", (xsqr - ysqr), fma(" << t1 << ", vIn.x, " << tc << ")));\n"
		   << "\t\treal_t ti = fma(" << t3 << ", ((real_t)(3.0) * xsqr * vIn.y - ycb), fma(" << t2 << " * (real_t)(2.0), vIn.x * vIn.y, " << t1 << " * vIn.y));\n"
		   << "\n"
		   << "\t\treal_t br = fma(" << b3 << ", (xcb - (real_t)(3.0) * vIn.x * ysqr), fma(" << b2 << ", (xsqr - ysqr), fma(" << b1 << ", vIn.x, " << bc << ")));\n"
		   << "\t\treal_t bi = fma(" << b3 << ", ((real_t)(3.0) * xsqr * vIn.y - ycb), fma(" << b2 << ", (real_t)(2.0) * vIn.x * vIn.y, " << b1 << " * vIn.y));\n"
		   << "\n"
		   << "\t\treal_t r3den = 1 / Zeps(fma(br, br, bi * bi));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(tr, br, ti * bi) * r3den;\n"
		   << "\t\tvOut.y = " << weight << " * fma(ti, br, -(tr * bi)) * r3den;\n"
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
		m_Params.push_back(ParamWithName<T>(&m_T3, prefix + "rational3_t3", 1));
		m_Params.push_back(ParamWithName<T>(&m_T2, prefix + "rational3_t2"));
		m_Params.push_back(ParamWithName<T>(&m_T1, prefix + "rational3_t1"));
		m_Params.push_back(ParamWithName<T>(&m_Tc, prefix + "rational3_tc", 1));
		m_Params.push_back(ParamWithName<T>(&m_B3, prefix + "rational3_b3"));
		m_Params.push_back(ParamWithName<T>(&m_B2, prefix + "rational3_b2", 1));
		m_Params.push_back(ParamWithName<T>(&m_B1, prefix + "rational3_b1"));
		m_Params.push_back(ParamWithName<T>(&m_Bc, prefix + "rational3_bc", 1));
	}

private:
	T m_T3;
	T m_T2;
	T m_T1;
	T m_Tc;
	T m_B3;
	T m_B2;
	T m_B1;
	T m_Bc;
};

/// <summary>
/// Ripple.
/// </summary>
template <typename T>
class RippleVariation : public ParametricVariation<T>
{
public:
	RippleVariation(T weight = 1.0) : ParametricVariation<T>("ripple", eVariationId::VAR_RIPPLE, weight)
	{
		Init();
	}

	PARVARCOPY(RippleVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		//Align input x, y to given center and multiply with scale.
		T x = (helper.In.x * m_S) - m_CenterX;
		T y = (helper.In.y * m_S) + m_CenterY;
		//Calculate distance from center but constrain it to EPS.
		T d = std::max(EPS, std::sqrt(SQR(x) * SQR(y)));
		//Normalize x and y.
		T nx = x / d;
		T ny = y / d;
		//Calculate cosine wave with given frequency, velocity
		//and phase based on the distance to center.
		T wave = std::cos(m_F * d - m_Vxp);
		//Calculate the wave offsets
		T d1 = wave * m_Pxa + d;
		T d2 = wave * m_Pixa + d;
		//We got two offsets, so we also got two new positions (u,v).
		T u1 = m_CenterX + nx * d1;
		T v1 = -m_CenterY + ny * d1;
		T u2 = m_CenterX + nx * d2;
		T v2 = -m_CenterY + ny * d2;
		//Interpolate the two positions by the given phase and
		//invert the multiplication with scale from before.
		helper.Out.x = m_Weight * Lerp<T>(u1, u2, m_P) * m_Is;//Original did a direct assignment to outPoint, which is incompatible with Ember's design.
		helper.Out.y = m_Weight * Lerp<T>(v1, v2, m_P) * m_Is;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string frequency = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string velocity  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amplitude = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string centerx   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string centery   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phase     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string f         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string p         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string s         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string is        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vxp       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pxa       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pixa      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x = fma(vIn.x, " << s << ", -" << centerx << ");\n"
		   << "\t\treal_t y = fma(vIn.y, " << s << ", " << centery << ");\n"
		   << "\n"
		   << "\t\treal_t d = max(EPS, sqrt(SQR(x) * SQR(y)));\n"
		   << "\n"
		   << "\t\treal_t nx = x / d;\n"
		   << "\t\treal_t ny = y / d;\n"
		   << "\n"
		   << "\t\treal_t wave = cos(fma(" << f << ", d, -" << vxp << "));\n"
		   << "\n"
		   << "\t\treal_t d1 = fma(wave, " << pxa  << ", d);\n"
		   << "\t\treal_t d2 = fma(wave, " << pixa << ", d);\n"
		   << "\n"
		   << "\t\treal_t u1 = fma(nx, d1, " << centerx << ");\n"
		   << "\t\treal_t v1 = fma(ny, d1, -" << centery << ");\n"
		   << "\t\treal_t u2 = fma(nx, d2, " << centerx << ");\n"
		   << "\t\treal_t v2 = fma(ny, d2, -" << centery << ");\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * Lerp(u1, u2, " << p << ") * " << is << ";\n"
		   << "\t\tvOut.y = " << weight << " * Lerp(v1, v2, " << p << ") * " << is << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Lerp" };
	}

	virtual void Precalc() override
	{
		m_F = m_Frequency * 5;
		m_A = m_Amplitude * T(0.01);
		m_P = m_Phase * M_2PI - T(M_PI);
		m_S = Zeps(m_Scale);//Scale must not be zero.
		m_Is = 1 / m_S;//Need the inverse scale.
		//Pre-multiply velocity + phase, phase + amplitude and (PI - phase) + amplitude.
		m_Vxp = m_Velocity * m_P;
		m_Pxa = m_P * m_A;
		m_Pixa = (T(M_PI) - m_P) * m_A;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Frequency, prefix + "ripple_frequency", 2));
		m_Params.push_back(ParamWithName<T>(&m_Velocity, prefix + "ripple_velocity", 1));
		m_Params.push_back(ParamWithName<T>(&m_Amplitude, prefix + "ripple_amplitude", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_CenterX, prefix + "ripple_centerx"));
		m_Params.push_back(ParamWithName<T>(&m_CenterY, prefix + "ripple_centery"));
		m_Params.push_back(ParamWithName<T>(&m_Phase, prefix + "ripple_phase"));
		m_Params.push_back(ParamWithName<T>(&m_Scale, prefix + "ripple_scale", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_F, prefix + "ripple_f"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_A, prefix + "ripple_a"));
		m_Params.push_back(ParamWithName<T>(true, &m_P, prefix + "ripple_p"));
		m_Params.push_back(ParamWithName<T>(true, &m_S, prefix + "ripple_s"));
		m_Params.push_back(ParamWithName<T>(true, &m_Is, prefix + "ripple_is"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vxp, prefix + "ripple_vxp"));
		m_Params.push_back(ParamWithName<T>(true, &m_Pxa, prefix + "ripple_pxa"));
		m_Params.push_back(ParamWithName<T>(true, &m_Pixa, prefix + "ripple_pixa"));
	}

private:
	T m_Frequency;
	T m_Velocity;
	T m_Amplitude;
	T m_CenterX;
	T m_CenterY;
	T m_Phase;
	T m_Scale;
	T m_F;//Precalc.
	T m_A;
	T m_P;
	T m_S;
	T m_Is;
	T m_Vxp;
	T m_Pxa;
	T m_Pixa;
};

/// <summary>
/// Sigmoid.
/// </summary>
template <typename T>
class SigmoidVariation : public ParametricVariation<T>
{
public:
	SigmoidVariation(T weight = 1.0) : ParametricVariation<T>("sigmoid", eVariationId::VAR_SIGMOID, weight)
	{
		Init();
	}

	PARVARCOPY(SigmoidVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T c0 = m_Ax / Zeps(1 + std::exp(m_Sx * helper.In.x));
		T c1 = m_Ay / Zeps(1 + std::exp(m_Sy * helper.In.y));
		T x = (2 * (c0 - T(0.5)));
		T y = (2 * (c1 - T(0.5)));
		helper.Out.x = m_Vv * x;
		helper.Out.y = m_Vv * y;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string shiftX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string shiftY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ax = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ay = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vv = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t c0 = " << ax << " / Zeps(1 + exp(" << sx << " * vIn.x));\n"
		   << "\t\treal_t c1 = " << ay << " / Zeps(1 + exp(" << sy << " * vIn.y));\n"
		   << "\t\treal_t x = (2 * (c0 - (real_t)(0.5)));\n"
		   << "\t\treal_t y = (2 * (c1 - (real_t)(0.5)));\n"
		   << "\n"
		   << "\t\tvOut.x = " << vv << " * x;\n"
		   << "\t\tvOut.y = " << vv << " * y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Sx = m_ShiftX;
		m_Sy = m_ShiftY;
		m_Ax = 1;
		m_Ay = 1;

		if (m_Sx < 1 && m_Sx > -1)
		{
			if (m_Sx == 0)
			{
				m_Sx = EPS;
				m_Ax = 1;
			}
			else
			{
				m_Ax = T(m_Sx < 0 ? -1 : 1);
				m_Sx = 1 / m_Sx;
			}
		}

		if (m_Sy < 1 && m_Sy > -1)
		{
			if (m_Sy == 0)
			{
				m_Sy = EPS;
				m_Ay = 1;
			}
			else
			{
				m_Ay = T(m_Sy < 0 ? -1 : 1);
				m_Sy = 1 / m_Sy;
			}
		}

		m_Sx *= -5;
		m_Sy *= -5;
		m_Vv = std::abs(m_Weight);
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
		m_Params.push_back(ParamWithName<T>(&m_ShiftX, prefix + "sigmoid_shiftx", 1));
		m_Params.push_back(ParamWithName<T>(&m_ShiftY, prefix + "sigmoid_shifty", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Sx, prefix + "sigmoid_sx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Sy, prefix + "sigmoid_sy"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ax, prefix + "sigmoid_ax"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ay, prefix + "sigmoid_ay"));
		m_Params.push_back(ParamWithName<T>(true, &m_Vv, prefix + "sigmoid_vv"));
	}

private:
	T m_ShiftX;
	T m_ShiftY;
	T m_Sx;//Precalc.
	T m_Sy;
	T m_Ax;
	T m_Ay;
	T m_Vv;
};

/// <summary>
/// SinusGrid.
/// </summary>
template <typename T>
class SinusGridVariation : public ParametricVariation<T>
{
public:
	SinusGridVariation(T weight = 1.0) : ParametricVariation<T>("sinusgrid", eVariationId::VAR_SINUS_GRID, weight)
	{
		Init();
	}

	PARVARCOPY(SinusGridVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x = helper.In.x;
		T y = helper.In.y;
		T sx = -1 * std::cos(x * m_Fx);
		T sy = -1 * std::cos(y * m_Fy);
		T tx = Lerp(helper.In.x, sx, m_Ax);
		T ty = Lerp(helper.In.y, sy, m_Ay);
		helper.Out.x = m_Weight * tx;
		helper.Out.y = m_Weight * ty;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string ampX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ampY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ax = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ay = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x = vIn.x;\n"
		   << "\t\treal_t y = vIn.y;\n"
		   << "\t\treal_t sx = -1 * cos(x * " << fx << ");\n"
		   << "\t\treal_t sy = -1 * cos(y * " << fy << ");\n"
		   << "\t\treal_t tx = Lerp(vIn.x, sx, " << ax << ");\n"
		   << "\t\treal_t ty = Lerp(vIn.y, sy, " << ay << ");\n"
		   << "\t\treal_t tz = vIn.z;\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * tx;\n"
		   << "\t\tvOut.y = " << weight << " * ty;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Lerp" };
	}

	virtual void Precalc() override
	{
		m_Ax = m_AmpX;
		m_Ay = m_AmpY;
		m_Fx = Zeps(m_FreqX * M_2PI);
		m_Fy = Zeps(m_FreqY * M_2PI);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_AmpX, prefix + "sinusgrid_ampx", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_AmpY, prefix + "sinusgrid_ampy", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_FreqX, prefix + "sinusgrid_freqx", 1));
		m_Params.push_back(ParamWithName<T>(&m_FreqY, prefix + "sinusgrid_freqy", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Fx, prefix + "sinusgrid_fx"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Fy, prefix + "sinusgrid_fy"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ax, prefix + "sinusgrid_ax"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ay, prefix + "sinusgrid_ay"));
	}

private:
	T m_AmpX;
	T m_AmpY;
	T m_FreqX;
	T m_FreqY;
	T m_Fx;//Precalc.
	T m_Fy;
	T m_Ax;
	T m_Ay;
};

/// <summary>
/// Stwin.
/// </summary>
template <typename T>
class StwinVariation : public ParametricVariation<T>
{
public:
	StwinVariation(T weight = 1.0) : ParametricVariation<T>("stwin", eVariationId::VAR_STWIN, weight)
	{
		Init();
	}

	PARVARCOPY(StwinVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		const T multiplier = T(0.05);
		T x = helper.In.x * m_Weight * multiplier;
		T y = helper.In.y * m_Weight * multiplier;
		T x2 = SQR(x);
		T y2 = SQR(y);
		T xPlusy = x + y;
		T x2Minusy2 = x2 - y2;
		T x2Plusy2 = x2 + y2;
		T result = x2Minusy2 * std::sin(M_2PI * m_Distort * xPlusy);
		T divident = 1;

		if (x2Plusy2 != 0)
			divident = x2Plusy2;

		result /= divident;
		helper.Out.x = m_Weight * helper.In.x + result;
		helper.Out.y = m_Weight * helper.In.y + result;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string distort = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x = vIn.x * " << weight << " * (real_t)(0.05);\n"
		   << "\t\treal_t y = vIn.y * " << weight << " * (real_t)(0.05);\n"
		   << "\t\treal_t x2 = SQR(x);\n"
		   << "\t\treal_t y2 = SQR(y);\n"
		   << "\t\treal_t xPlusy = x + y;\n"
		   << "\t\treal_t x2Minusy2 = x2 - y2;\n"
		   << "\t\treal_t x2Plusy2 = x2 + y2;\n"
		   << "\t\treal_t result = x2Minusy2 * sin(M_2PI * " << distort << " * xPlusy);\n"
		   << "\t\treal_t divident = 1;\n"
		   << "\n"
		   << "\t\tif (x2Plusy2 != 0)\n"
		   << "\t\t	divident = x2Plusy2;\n"
		   << "\n"
		   << "\t\tresult /= divident;\n"
		   << "\n"
		   << "\t\tvOut.x = fma(" << weight << ", vIn.x, result);\n"
		   << "\t\tvOut.y = fma(" << weight << ", vIn.y, result);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Distort, prefix + "stwin_distort", 1));//Original had a misspelling of swtin, which is incompatible with Ember's design.
	}

private:
	T m_Distort;
};

/// <summary>
/// TwoFace.
/// </summary>
template <typename T>
class TwoFaceVariation : public Variation<T>
{
public:
	TwoFaceVariation(T weight = 1.0) : Variation<T>("twoface", eVariationId::VAR_TWO_FACE, weight, true) { }

	VARCOPY(TwoFaceVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight;

		if (helper.In.x > 0)
			r /= helper.m_PrecalcSumSquares;

		helper.Out.x = r * helper.In.x;
		helper.Out.y = r * helper.In.y;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t r = " << weight << ";\n"
		   << "\n"
		   << "\t\tif (vIn.x > 0)\n"
		   << "\t\t	r /= precalcSumSquares;\n"
		   << "\n"
		   << "\t\tvOut.x = r * vIn.x;\n"
		   << "\t\tvOut.y = r * vIn.y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Unpolar.
/// </summary>
template <typename T>
class UnpolarVariation : public ParametricVariation<T>
{
public:
	UnpolarVariation(T weight = 1.0) : ParametricVariation<T>("unpolar", eVariationId::VAR_UNPOLAR, weight)
	{
		Init();
	}

	PARVARCOPY(UnpolarVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = std::exp(helper.In.y);
		T s = std::sin(helper.In.x);
		T c = std::cos(helper.In.x);
		helper.Out.x = m_Vvar2 * r * s;
		helper.Out.y = m_Vvar2 * r * c;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string vvar2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalcs only, no params.
		ss << "\t{\n"
		   << "\t\treal_t vvar2 = " << vvar2 << ";\n"
		   << "\t\treal_t r = exp(vIn.y);\n"
		   << "\t\treal_t s = sin(vIn.x);\n"
		   << "\t\treal_t c = cos(vIn.x);\n"
		   << "\t\treal_t vvar2r = vvar2 * r;\n"
		   << "\n"
		   << "\t\tvOut.x = vvar2r * s;\n"
		   << "\t\tvOut.y = vvar2r * c;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Vvar2 = (m_Weight / T(M_PI)) * T(0.5);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_Vvar2, prefix + "unpolar_vvar_2"));//Precalcs only, no params.
	}

private:
	T m_Vvar2;//Precalcs only, no params.
};

/// <summary>
/// WavesN.
/// </summary>
template <typename T>
class WavesNVariation : public ParametricVariation<T>
{
public:
	WavesNVariation(T weight = 1.0) : ParametricVariation<T>("wavesn", eVariationId::VAR_WAVESN, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(WavesNVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = (helper.m_PrecalcAtanyx + M_2PI * rand.Rand(size_t(m_AbsN))) / m_Power;
		T r = m_Weight * std::pow(helper.m_PrecalcSumSquares, m_Cn);
		T sina = std::sin(angle);
		T cosa = std::cos(angle);
		T xn = r * cosa;
		T yn = r * sina;
		T siny = std::sin(m_FreqX * yn);
		T sinx = std::sin(m_FreqY * xn);
		T dx = xn + T(0.5) * (m_ScaleX * siny + std::abs(xn) * m_IncX * siny);
		T dy = yn + T(0.5) * (m_ScaleY * sinx + std::abs(yn) * m_IncY * sinx);
		helper.Out.x = m_Weight * dx;
		helper.Out.y = m_Weight * dy;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string freqX  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqY  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleX = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleY = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string incX   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string incY   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absn   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cn     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t angle = fma(M_2PI, (real_t)MwcNextRange(mwc, (uint)" << absn << "), precalcAtanyx) / " << power << ";\n"
		   << "\t\treal_t r = " << weight << " * pow(precalcSumSquares, " << cn << ");\n"
		   << "\t\treal_t sina = sin(angle);\n"
		   << "\t\treal_t cosa = cos(angle);\n"
		   << "\t\treal_t xn = r * cosa;\n"
		   << "\t\treal_t yn = r * sina;\n"
		   << "\t\treal_t siny = sin(" << freqX << " * yn);\n"
		   << "\t\treal_t sinx = sin(" << freqY << " * xn);\n"
		   << "\t\treal_t dx = fma((real_t)(0.5), fma(" << scaleX << ", siny, fabs(xn) * " << incX << " * siny), xn);\n"
		   << "\t\treal_t dy = fma((real_t)(0.5), fma(" << scaleY << ", sinx, fabs(yn) * " << incY << " * sinx), yn);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * dx;\n"
		   << "\t\tvOut.y = " << weight << " * dy;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		if (m_Power == 0)
			m_Power = 2;

		m_AbsN = T(int(std::abs(m_Power)));
		m_Cn = 1 / m_Power / 2;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_FreqX, prefix + "wavesn_freqx", 2));
		m_Params.push_back(ParamWithName<T>(&m_FreqY, prefix + "wavesn_freqy", 2));
		m_Params.push_back(ParamWithName<T>(&m_ScaleX, prefix + "wavesn_scalex", 1));
		m_Params.push_back(ParamWithName<T>(&m_ScaleY, prefix + "wavesn_scaley", 1));
		m_Params.push_back(ParamWithName<T>(&m_IncX, prefix + "wavesn_incx"));
		m_Params.push_back(ParamWithName<T>(&m_IncY, prefix + "wavesn_incy"));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "wavesn_power", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsN, prefix + "wavesn_absn"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Cn, prefix + "wavesn_cn"));
	}

private:
	T m_FreqX;
	T m_FreqY;
	T m_ScaleX;
	T m_ScaleY;
	T m_IncX;
	T m_IncY;
	T m_Power;
	T m_AbsN;//Precalc.
	T m_Cn;
};

/// <summary>
/// XHeart.
/// </summary>
template <typename T>
class XHeartVariation : public ParametricVariation<T>
{
public:
	XHeartVariation(T weight = 1.0) : ParametricVariation<T>("xheart", eVariationId::VAR_XHEART, weight, true)
	{
		Init();
	}

	PARVARCOPY(XHeartVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r2_4 = helper.m_PrecalcSumSquares + 4;

		if (r2_4 == 0)
			r2_4 = 1;

		T bx = 4 / r2_4;
		T by = m_Rat / r2_4;
		T x = m_Cosa * (bx * helper.In.x) - m_Sina * (by * helper.In.y);
		T y = m_Sina * (bx * helper.In.x) + m_Cosa * (by * helper.In.y);

		if (x > 0)
		{
			helper.Out.x = m_Weight * x;
			helper.Out.y = m_Weight * y;
		}
		else
		{
			helper.Out.x = m_Weight * x;
			helper.Out.y = -m_Weight * y;
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
		string angle = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ratio = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosa  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sina  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rat   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r2_4 = precalcSumSquares + 4;\n"
		   << "\n"
		   << "\t\tif (r2_4 == 0)\n"
		   << "\t\t	r2_4 = 1;\n"
		   << "\n"
		   << "\t\treal_t bx = 4 / r2_4;\n"
		   << "\t\treal_t by = " << rat << " / r2_4;\n"
		   << "\t\treal_t bxx = bx * vIn.x;\n"
		   << "\t\treal_t byy = by * vIn.y;\n"
		   << "\t\treal_t x = fma(" << cosa << ", bxx, -(" << sina << " * byy));\n"
		   << "\t\treal_t y = fma(" << sina << ", bxx, " << cosa << " * byy);\n"
		   << "\n"
		   << "\t\tif (x > 0)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * x;\n"
		   << "\t\t	vOut.y = " << weight << " * y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = " << weight << " * x;\n"
		   << "\t\t	vOut.y = -" << weight << " * y;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T ang = T(M_PI_4) + (T(0.5) * T(M_PI_4) * m_Angle);
		sincos(ang, &m_Sina, &m_Cosa);
		m_Rat = 6 + 2 * m_Ratio;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle, prefix + "xheart_angle"));
		m_Params.push_back(ParamWithName<T>(&m_Ratio, prefix + "xheart_ratio"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cosa, prefix + "xheart_cosa"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Sina, prefix + "xheart_sina"));
		m_Params.push_back(ParamWithName<T>(true, &m_Rat, prefix + "xheart_rat"));
	}

private:
	T m_Angle;
	T m_Ratio;
	T m_Cosa;//Precalc.
	T m_Sina;
	T m_Rat;
};

/// <summary>
/// Barycentroid.
/// </summary>
template <typename T>
class BarycentroidVariation : public ParametricVariation<T>
{
public:
	BarycentroidVariation(T weight = 1.0) : ParametricVariation<T>("barycentroid", eVariationId::VAR_BARYCENTROID, weight)
	{
		Init();
	}

	PARVARCOPY(BarycentroidVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		//Compute dot products.
		T dot00 = SQR(m_A) + SQR(m_B);//v0 * v0.
		T dot01 = m_A * m_C + m_B * m_D;//v0 * v1.
		T dot02 = m_A * helper.In.x + m_B * helper.In.y;//v0 * v2.
		T dot11 = SQR(m_C) + SQR(m_D);//v1 * v1.
		T dot12 = m_C * helper.In.x + m_D * helper.In.y;//v1 * v2.
		//Compute inverse denomiator.
		T invDenom = 1 / Zeps(dot00 * dot11 - dot01 * dot01);
		//Now we can pull [u,v] as the barycentric coordinates of the point
		//P in the triangle [A, B, C].
		T u = (dot11 * dot02 - dot01 * dot12) * invDenom;
		T v = (dot00 * dot12 - dot01 * dot02) * invDenom;
		// now combine with input
		T um = std::sqrt(SQR(u) + SQR(helper.In.x)) * VarFuncs<T>::Sign(u);
		T vm = std::sqrt(SQR(v) + SQR(helper.In.y)) * VarFuncs<T>::Sign(v);
		helper.Out.x = m_Weight * um;
		helper.Out.y = m_Weight * vm;
		helper.Out.z = m_Weight * helper.In.z;
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
		string c = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t dot00 = fma(" << a << ", " << a << ", SQR(" << b << "));\n"
		   << "\t\treal_t dot01 = fma(" << a << ", " << c << ", " << b << " * " << d << ");\n"
		   << "\t\treal_t dot02 = fma(" << a << ", vIn.x, " << b << " * vIn.y);\n"
		   << "\t\treal_t dot11 = fma(" << c << ", " << c << ", SQR(" << d << "));\n"
		   << "\t\treal_t dot12 = fma(" << c << ", vIn.x, " << d << " * vIn.y);\n"
		   << "\t\treal_t invDenom = (real_t)(1.0) / Zeps(fma(dot00, dot11, -(dot01 * dot01)));\n"
		   << "\t\treal_t u = fma(dot11, dot02, -(dot01 * dot12)) * invDenom;\n"
		   << "\t\treal_t v = fma(dot00, dot12, -(dot01 * dot02)) * invDenom;\n"
		   << "\t\treal_t um = sqrt(fma(u, u, SQR(vIn.x))) * Sign(u);\n"
		   << "\t\treal_t vm = sqrt(fma(v, v, SQR(vIn.y))) * Sign(v);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * um;\n"
		   << "\t\tvOut.y = " << weight << " * vm;\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Sign", "Zeps" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "barycentroid_a", 1));
		m_Params.push_back(ParamWithName<T>(&m_B, prefix + "barycentroid_b"));
		m_Params.push_back(ParamWithName<T>(&m_C, prefix + "barycentroid_c"));
		m_Params.push_back(ParamWithName<T>(&m_D, prefix + "barycentroid_d", 1));
	}

private:
	T m_A;
	T m_B;
	T m_C;
	T m_D;
};

/// <summary>
/// BiSplit.
/// </summary>
template <typename T>
class BiSplitVariation : public ParametricVariation<T>
{
public:
	BiSplitVariation(T weight = 1.0) : ParametricVariation<T>("bisplit", eVariationId::VAR_BISPLIT, weight)
	{
		Init();
	}

	PARVARCOPY(BiSplitVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight01 / SafeTan<T>(helper.In.x) * std::cos(helper.In.y);
		helper.Out.y = m_Weight01 / std::sin(helper.In.x) * (-helper.In.y);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string weight01 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight01 << " / tan(vIn.x) * cos(vIn.y);\n"
		   << "\t\tvOut.y = " << weight01 << " / sin(vIn.x) * (-vIn.y);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Weight01 = m_Weight * T(0.1);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_Weight01, prefix + "bisplit_weight01"));//Precalcs only, no params.
	}

private:
	T m_Weight01;
};

/// <summary>
/// Crescents.
/// </summary>
template <typename T>
class CrescentsVariation : public Variation<T>
{
public:
	CrescentsVariation(T weight = 1.0) : Variation<T>("crescents", eVariationId::VAR_CRESCENTS, weight) { }

	VARCOPY(CrescentsVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sinx = std::sin(helper.In.x);
		T sinx2 = SQR(sinx);
		T cosx = std::cos(helper.In.x);
		T coshy1 = std::cosh(helper.In.y) + 1;
		helper.Out.x = m_Weight * sinx * coshy1 * sinx2;
		helper.Out.y = m_Weight * cosx * coshy1 * sinx2;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t sinx = sin(vIn.x);\n"
		   << "\t\treal_t sinx2 = SQR(sinx);\n"
		   << "\t\treal_t cosx = cos(vIn.x);\n"
		   << "\t\treal_t coshy1 = cosh(vIn.y) + 1.0;\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * sinx * coshy1 * sinx2;\n"
		   << "\t\tvOut.y = " << weight << " * cosx * coshy1 * sinx2;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// Mask.
/// </summary>
template <typename T>
class MaskVariation : public Variation<T>
{
public:
	MaskVariation(T weight = 1.0) : Variation<T>("mask", eVariationId::VAR_MASK, weight, true) { }

	VARCOPY(MaskVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T d = m_Weight / Zeps(helper.m_PrecalcSumSquares);
		T sinx = std::sin(helper.In.x);
		T sinx2 = SQR(sinx);
		T cosx = std::cos(helper.In.x);
		T coshy1 = std::cosh(helper.In.y) + 1;
		helper.Out.x = d * sinx * coshy1 * sinx2;
		helper.Out.y = d * cosx * coshy1 * sinx2;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\treal_t d = " << weight << " / Zeps(precalcSumSquares);\n"
		   << "\t\treal_t sinx = sin(vIn.x);\n"
		   << "\t\treal_t sinx2 = SQR(sinx);\n"
		   << "\t\treal_t cosx = cos(vIn.x);\n"
		   << "\t\treal_t coshy1 = cosh(vIn.y) + 1.0;\n"
		   << "\n"
		   << "\t\tvOut.x = d * sinx * coshy1 * sinx2;\n"
		   << "\t\tvOut.y = d * cosx * coshy1 * sinx2;\n"
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
/// Cpow2.
/// </summary>
template <typename T>
class Cpow2Variation : public ParametricVariation<T>
{
public:
	Cpow2Variation(T weight = 1.0) : ParametricVariation<T>("cpow2", eVariationId::VAR_CPOW2, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Cpow2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanyx;
		int n = rand.Rand(m_SpreadUint);

		if (a < 0)
			n++;

		a += M_2PI * n;

		if (std::cos(a * m_InvSpread) < rand.Frand11<T>())
			a -= m_FullSpread;

		T lnr2 = std::log(helper.m_PrecalcSumSquares);
		T r = m_Weight * std::exp(m_HalfC * lnr2 - m_D * a);
		T temp = m_C * a + m_HalfD * lnr2 + m_Ang * rand.Crand();
		helper.Out.x = r * std::cos(temp);
		helper.Out.y = r * std::sin(temp);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string r = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string divisor = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spread = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfC = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfD = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ang = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string invSpread = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fullSpread = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanyx;\n"
		   << "\t\tint n = MwcNextRange(mwc, (uint)" << spread << ");\n"
		   << "\n"
		   << "\t\tif (a < 0)\n"
		   << "\t\t	n++;\n"
		   << "\n"
		   << "\t\ta += M_2PI * n;\n"
		   << "\n"
		   << "\t\tif (cos(a * " << invSpread << ") < MwcNextNeg1Pos1(mwc))\n"
		   << "\t\t	a -= " << fullSpread << ";\n"
		   << "\n"
		   << "\t\treal_t lnr2 = log(precalcSumSquares);\n"
		   << "\t\treal_t r = " << weight << " * exp(fma(" << halfC << ", lnr2, -(" << d << " * a)));\n"
		   << "\t\treal_t temp = fma(" << c << ", a, fma(" << halfD << ", lnr2, " << ang << " * MwcNextCrand(mwc)));\n"
		   << "\n"
		   << "\t\tvOut.x = r * cos(temp);\n"
		   << "\t\tvOut.y = r * sin(temp);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		auto spread = Zeps(m_Spread);
		m_Ang = M_2PI / m_Divisor;
		m_C = m_R * std::cos(T(M_PI) / 2 * m_A) / m_Divisor;
		m_D = m_R * std::sin(T(M_PI) / 2 * m_A) / m_Divisor;
		m_HalfC = m_C / 2;
		m_HalfD = m_D / 2;
		m_InvSpread = T(0.5) / spread;
		m_FullSpread = M_2PI * spread;
		m_SpreadUint = uint(spread);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_R, prefix + "cpow2_r", 1));
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "cpow2_a"));
		m_Params.push_back(ParamWithName<T>(&m_Divisor, prefix + "cpow2_divisor", 1));
		m_Params.push_back(ParamWithName<T>(&m_Spread, prefix + "cpow2_spread", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_C, prefix + "cpow2_c"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_HalfC, prefix + "cpow2_halfc"));
		m_Params.push_back(ParamWithName<T>(true, &m_D, prefix + "cpow2_d"));
		m_Params.push_back(ParamWithName<T>(true, &m_HalfD, prefix + "cpow2_halfd"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ang, prefix + "cpow2_ang"));
		m_Params.push_back(ParamWithName<T>(true, &m_InvSpread, prefix + "cpow2_inv_spread"));
		m_Params.push_back(ParamWithName<T>(true, &m_FullSpread, prefix + "cpow2_full_spread"));
	}

private:
	T m_R;
	T m_A;
	T m_Divisor;
	T m_Spread;
	uint m_SpreadUint;//Precalc.
	T m_C;
	T m_HalfC;
	T m_D;
	T m_HalfD;
	T m_Ang;
	T m_InvSpread;
	T m_FullSpread;
};

MAKEPREPOSTVAR(Hemisphere, hemisphere, HEMISPHERE)
MAKEPREPOSTPARVAR(Epispiral, epispiral, EPISPIRAL)
MAKEPREPOSTPARVAR(Bwraps, bwraps, BWRAPS)
MAKEPREPOSTPARVAR(BwrapsRand, bwraps_rand, BWRAPS_RAND)
MAKEPREPOSTVARASSIGN(BlurCircle, blur_circle, BLUR_CIRCLE, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(BlurZoom, blur_zoom, BLUR_ZOOM)
MAKEPREPOSTPARVAR(BlurPixelize, blur_pixelize, BLUR_PIXELIZE)
MAKEPREPOSTPARVAR(Crop, crop, CROP)
MAKEPREPOSTPARVAR(BCircle, bcircle, BCIRCLE)
MAKEPREPOSTPARVAR(BlurLinear, blur_linear, BLUR_LINEAR)
MAKEPREPOSTPARVARASSIGN(BlurSquare, blur_square, BLUR_SQUARE, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTVAR(Flatten, flatten, FLATTEN)
MAKEPREPOSTVARASSIGN(Zblur, zblur, ZBLUR, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTVARASSIGN(Blur3D, blur3D, BLUR3D, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTVAR(ZScale, zscale, ZSCALE)
MAKEPREPOSTVARASSIGN(ZTranslate, ztranslate, ZTRANSLATE, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTVAR(ZCone, zcone, ZCONE)
MAKEPREPOSTVAR(Spherical3D, Spherical3D, SPHERICAL3D)
MAKEPREPOSTPARVAR(Curl3D, curl3D, CURL3D)
MAKEPREPOSTPARVAR(Disc3D, disc3d, DISC3D)
MAKEPREPOSTPARVAR(Boarders2, boarders2, BOARDERS2)
MAKEPREPOSTPARVAR(Cardioid, cardioid, CARDIOID)
MAKEPREPOSTPARVAR(Checks, checks, CHECKS)
MAKEPREPOSTPARVAR(Circlize, circlize, CIRCLIZE)
MAKEPREPOSTPARVAR(Circlize2, circlize2, CIRCLIZE2)
MAKEPREPOSTPARVAR(CosWrap, coswrap, COS_WRAP)
MAKEPREPOSTVAR(DeltaA, deltaa, DELTA_A)
MAKEPREPOSTPARVAR(Expo, expo, EXPO)
MAKEPREPOSTPARVAR(Extrude, extrude, EXTRUDE)
MAKEPREPOSTVAR(FDisc, fdisc, FDISC)
MAKEPREPOSTPARVAR(Fibonacci, fibonacci, FIBONACCI)
MAKEPREPOSTPARVAR(Fibonacci2, fibonacci2, FIBONACCI2)
MAKEPREPOSTPARVAR(Glynnia, glynnia, GLYNNIA)
MAKEPREPOSTPARVAR(Glynnia2, glynnia2, GLYNNIA2)
MAKEPREPOSTVAR(GridOut, gridout, GRIDOUT)
MAKEPREPOSTPARVAR(Hole, hole, HOLE)
MAKEPREPOSTPARVAR(Hypertile, hypertile, HYPERTILE)
MAKEPREPOSTPARVAR(Hypertile1, hypertile1, HYPERTILE1)
MAKEPREPOSTPARVAR(Hypertile2, hypertile2, HYPERTILE2)
MAKEPREPOSTPARVAR(Hypertile3D, hypertile3D, HYPERTILE3D)
MAKEPREPOSTPARVAR(Hypertile3D1, hypertile3D1, HYPERTILE3D1)
MAKEPREPOSTPARVAR(Hypertile3D2, hypertile3D2, HYPERTILE3D2)
MAKEPREPOSTPARVAR(IDisc, idisc, IDISC)
MAKEPREPOSTPARVAR(Julian2, julian2, JULIAN2)
MAKEPREPOSTPARVAR(JuliaQ, juliaq, JULIAQ)
MAKEPREPOSTPARVAR(Murl, murl, MURL)
MAKEPREPOSTPARVAR(Murl2, murl2, MURL2)
MAKEPREPOSTPARVAR(NPolar, npolar, NPOLAR)
MAKEPREPOSTPARVAR(Ortho, ortho, ORTHO)
MAKEPREPOSTPARVAR(Poincare, poincare, POINCARE)
MAKEPREPOSTPARVAR(Poincare2, poincare2, POINCARE2)
MAKEPREPOSTPARVAR(Poincare3D, poincare3D, POINCARE3D)
MAKEPREPOSTPARVAR(Polynomial, polynomial, POLYNOMIAL)
MAKEPREPOSTPARVAR(PSphere, psphere, PSPHERE)
MAKEPREPOSTPARVAR(Rational3, rational3, RATIONAL3)
MAKEPREPOSTPARVAR(Ripple, ripple, RIPPLE)
MAKEPREPOSTPARVAR(Sigmoid, sigmoid, SIGMOID)
MAKEPREPOSTPARVAR(SinusGrid, sinusgrid, SINUS_GRID)
MAKEPREPOSTPARVAR(Stwin, stwin, STWIN)
MAKEPREPOSTVAR(TwoFace, twoface, TWO_FACE)
MAKEPREPOSTPARVAR(Unpolar, unpolar, UNPOLAR)
MAKEPREPOSTPARVAR(WavesN, wavesn, WAVESN)
MAKEPREPOSTPARVAR(XHeart, xheart, XHEART)
MAKEPREPOSTPARVAR(Barycentroid, barycentroid, BARYCENTROID)
MAKEPREPOSTPARVAR(BiSplit, bisplit, BISPLIT)
MAKEPREPOSTVAR(Crescents, crescents, CRESCENTS)
MAKEPREPOSTVAR(Mask, mask, MASK)
MAKEPREPOSTPARVAR(Cpow2, cpow2, CPOW2)
}
