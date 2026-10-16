#pragma once

#include "Variation.h"

namespace EmberNs
{
/// <summary>
/// splits3D.
/// </summary>
template <typename T>
class Splits3DVariation : public ParametricVariation<T>
{
public:
	Splits3DVariation(T weight = 1.0) : ParametricVariation<T>("splits3D", eVariationId::VAR_SPLITS3D, weight)
	{
		Init();
	}

	PARVARCOPY(Splits3DVariation)

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

		if (helper.In.z >= 0)
			helper.Out.z = m_Weight * (helper.In.z + m_Z);
		else
			helper.Out.z = m_Weight * (helper.In.z - m_Z);
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
		   << "\t\tif (vIn.z >= 0)\n"
		   << "\t\t	vOut.z = " << weight << " * (vIn.z + " << z << ");\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.z = " << weight << " * (vIn.z - " << z << ");\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		m_X = rand.Frand11<T>();
		m_Y = rand.Frand11<T>();
		m_Z = rand.Frand11<T>();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "splits3D_x"));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "splits3D_y"));
		m_Params.push_back(ParamWithName<T>(&m_Z, prefix + "splits3D_z"));
	}

private:
	T m_X;
	T m_Y;
	T m_Z;
};

/// <summary>
/// waves2b.
/// Note that _j1() is not implemented in OpenCL, so that conditional is skipped
/// when running on the GPU. The results might look different.
/// </summary>
template <typename T>
class Waves2BVariation : public ParametricVariation<T>
{
public:
	Waves2BVariation(T weight = 1.0) : ParametricVariation<T>("waves2b", eVariationId::VAR_WAVES2B, weight)
	{
		Init();
	}

	PARVARCOPY(Waves2BVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T CsX = 1;
		T CsY = 1;
		T jcbSn = 0, jcbCn, jcbDn;
		CsX = VarFuncs<T>::SafeDivInv(m_Unity, (m_Unity + Sqr(helper.In.x)));
		CsX = CsX * m_Six + m_Scaleinfx;
		CsY = VarFuncs<T>::SafeDivInv(m_Unity, (m_Unity + Sqr(helper.In.y)));
		CsY = CsY * m_Siy + m_Scaleinfy;

		if (m_Pwx >= 0 && m_Pwx < 1e-4)
		{
			VarFuncs<T>::JacobiElliptic(helper.In.y * m_Freqx, m_Jacok, jcbSn, jcbCn, jcbDn);
			helper.Out.x = m_Weight * (helper.In.x + CsX * jcbSn);
		}
		else if (m_Pwx < 0 && m_Pwx > -1e-4)
#ifdef _WIN32
			helper.Out.x = m_Weight * (helper.In.x + CsX * T(_j1(helper.In.y * m_Freqx)));//This is not implemented in OpenCL.

#else
			helper.Out.x = m_Weight * (helper.In.x + CsX * T(j1(helper.In.y * m_Freqx)));//This is not implemented in OpenCL.
#endif
		else
			helper.Out.x = m_Weight * (helper.In.x + CsX * std::sin(VarFuncs<T>::SignNz(helper.In.y) * std::pow(Zeps(std::abs(helper.In.y)), m_Pwx) * m_Freqx));

		if (m_Pwy >= 0 && m_Pwy < 1e-4)
		{
			VarFuncs<T>::JacobiElliptic(helper.In.x * m_Freqy, m_Jacok, jcbSn, jcbCn, jcbDn);
			helper.Out.y = m_Weight * (helper.In.y + CsY * jcbSn);
		}
		else if (m_Pwy < 0 && m_Pwy > -1e-4)
#ifdef _WIN32
			helper.Out.y = m_Weight * (helper.In.y + CsY * T(_j1(helper.In.x * m_Freqy)));

#else
			helper.Out.y = m_Weight * (helper.In.y + CsY * T(j1(helper.In.x * m_Freqy)));
#endif
		else
			helper.Out.y = m_Weight * (helper.In.y + CsY * std::sin(VarFuncs<T>::SignNz(helper.In.x) * std::pow(Zeps(std::abs(helper.In.x)), m_Pwy) * m_Freqy));

		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string freqx     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqy     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pwx       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string pwy       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scalex    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleinfx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaley    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleinfy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string unity     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string jacok     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string six       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string siy       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t CsX = 1;\n"
		   << "\t\treal_t CsY = 1;\n"
		   << "\t\treal_t jcbSn = 0, jcbCn, jcbDn;\n"
		   << "\t\tCsX = SafeDivInv(" << unity << ", fma(vIn.x, vIn.x, " << unity << "));\n"
		   << "\t\tCsX = fma(CsX, " << six << ", " << scaleinfx << ");\n"
		   << "\t\tCsY = SafeDivInv(" << unity << ", fma(vIn.y, vIn.y, " << unity << "));\n"
		   << "\t\tCsY = fma(CsY, " << siy << ", " << scaleinfy << ");\n"
		   << "\n"
		   << "\t\tif (" << pwx << " >= 0 && " << pwx << " < 1e-4)\n"
		   << "\t\t{\n"
		   << "\t\t	JacobiElliptic(vIn.y * " << freqx << ", " << jacok << ", &jcbSn, &jcbCn, &jcbDn);\n"
		   << "\t\t	vOut.x = " << weight << " * fma(CsX, jcbSn, vIn.x);\n"
		   << "\t\t}\n"
		   << "\t\telse if (" << pwx << " < 0 && " << pwx << " > -1e-4)\n"
		   << "\t\t	vOut.x = " << weight << " * (vIn.x + CsX * J1(vIn.y * " << freqx << ", globalShared + P1, globalShared + Q1, globalShared + P2, globalShared + Q2, globalShared + PC, globalShared + QC, globalShared + PS, globalShared + QS));\n"//J1 is manually implemented in OpenCL.
		   << "\t\telse\n"
		   << "\t\t	vOut.x = " << weight << " * fma(CsX, sin(SignNz(vIn.y) * pow(Zeps(fabs(vIn.y)), " << pwx << ") * " << freqx << "), vIn.x);\n"
		   << "\n"
		   << "\t\tif (" << pwy << " >= 0 && " << pwy << " < 1e-4)\n"
		   << "\t\t{\n"
		   << "\t\t	JacobiElliptic(vIn.x * " << freqy << ", " << jacok << ", &jcbSn, &jcbCn, &jcbDn);\n"
		   << "\t\t	vOut.y = " << weight << " * fma(CsY, jcbSn, vIn.y);\n"
		   << "\t\t}\n"
		   << "\t\telse if (" << pwy << " < 0 && " << pwy << " > -1e-4)\n"
		   << "\t\t	vOut.y = " << weight << " * (vIn.y + CsY * J1(vIn.x * " << freqy << ", globalShared + P1, globalShared + Q1, globalShared + P2, globalShared + Q2, globalShared + PC, globalShared + QC, globalShared + PS, globalShared + QS));\n"//J1 is manually implemented in OpenCL.
		   << "\t\telse\n"
		   << "\t\t	vOut.y = " << weight << " * fma(CsY, sin(SignNz(vIn.x) * pow(Zeps(fabs(vIn.x)), " << pwy << ") * " << freqy << "), vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps", "SignNz", "SafeDivInv", "JacobiElliptic", "EvalRational", "J1" };
	}

	virtual vector<string> OpenCLGlobalDataNames() const override
	{
		return vector<string> { "P1", "Q1", "P2", "Q2", "PC", "QC", "PS", "QS" };
	}

	virtual void Precalc() override
	{
		m_Six = m_Scalex - m_Scaleinfx;
		m_Siy = m_Scaley - m_Scaleinfy;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Freqx,     prefix + "waves2b_freqx", 2));
		m_Params.push_back(ParamWithName<T>(&m_Freqy,     prefix + "waves2b_freqy", 2));
		m_Params.push_back(ParamWithName<T>(&m_Pwx,       prefix + "waves2b_pwx", 1, eParamType::REAL, -10, 10));
		m_Params.push_back(ParamWithName<T>(&m_Pwy,       prefix + "waves2b_pwy", 1, eParamType::REAL, -10, 10));
		m_Params.push_back(ParamWithName<T>(&m_Scalex,    prefix + "waves2b_scalex", 1));
		m_Params.push_back(ParamWithName<T>(&m_Scaleinfx, prefix + "waves2b_scaleinfx", 1));
		m_Params.push_back(ParamWithName<T>(&m_Scaley,    prefix + "waves2b_scaley", 1));
		m_Params.push_back(ParamWithName<T>(&m_Scaleinfy, prefix + "waves2b_scaleinfy", 1));
		m_Params.push_back(ParamWithName<T>(&m_Unity,     prefix + "waves2b_unity", 1));
		m_Params.push_back(ParamWithName<T>(&m_Jacok,     prefix + "waves2b_jacok", T(0.25), eParamType::REAL, -1, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Six, prefix + "waves2b_six"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Siy, prefix + "waves2b_siy"));
	}

private:
	T m_Freqx;
	T m_Freqy;
	T m_Pwx;
	T m_Pwy;
	T m_Scalex;
	T m_Scaleinfx;
	T m_Scaley;
	T m_Scaleinfy;
	T m_Unity;
	T m_Jacok;
	T m_Six;//Precalc.
	T m_Siy;
};

/// <summary>
/// jac_cn.
/// </summary>
template <typename T>
class JacCnVariation : public ParametricVariation<T>
{
public:
	JacCnVariation(T weight = 1.0) : ParametricVariation<T>("jac_cn", eVariationId::VAR_JAC_CN, weight)
	{
		Init();
	}

	PARVARCOPY(JacCnVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T snx, cnx, dnx;
		T sny, cny, dny;
		T numX, numY, denom;
		VarFuncs<T>::JacobiElliptic(helper.In.x, m_K, snx, cnx, dnx);
		VarFuncs<T>::JacobiElliptic(helper.In.y, 1 - m_K, sny, cny, dny);
		numX = cnx * cny;
		numY = -dnx * snx * dny * sny;
		denom = SQR(snx) * SQR(sny) * m_K + SQR(cny);
		denom = m_Weight / Zeps(denom);
		helper.Out.x = denom * numX;
		helper.Out.y = denom * numY;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string k = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t snx, cnx, dnx;\n"
		   << "\t\treal_t sny, cny, dny;\n"
		   << "\t\treal_t numX, numY, denom;\n"
		   << "\t\tJacobiElliptic(vIn.x, " << k << ", &snx, &cnx, &dnx);\n"
		   << "\t\tJacobiElliptic(vIn.y, 1 - " << k << ", &sny, &cny, &dny);\n"
		   << "\t\tnumX = cnx * cny;\n"
		   << "\t\tnumY = -dnx * snx * dny * sny;\n"
		   << "\t\tdenom = fma(SQR(snx) * SQR(sny), " << k << ", SQR(cny));\n"
		   << "\t\tdenom = " << weight << " / Zeps(denom);\n"
		   << "\t\tvOut.x = denom * numX;\n"
		   << "\t\tvOut.y = denom * numY;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps", "JacobiElliptic" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_K, prefix + "jac_cn_k", T(0.5), eParamType::REAL, -1, 1));
	}

private:
	T m_K;
};

/// <summary>
/// jac_dn.
/// </summary>
template <typename T>
class JacDnVariation : public ParametricVariation<T>
{
public:
	JacDnVariation(T weight = 1.0) : ParametricVariation<T>("jac_dn", eVariationId::VAR_JAC_DN, weight)
	{
		Init();
	}

	PARVARCOPY(JacDnVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T snx, cnx, dnx;
		T sny, cny, dny;
		T numX, numY, denom;
		VarFuncs<T>::JacobiElliptic(helper.In.x, m_K, snx, cnx, dnx);
		VarFuncs<T>::JacobiElliptic(helper.In.y, 1 - m_K, sny, cny, dny);
		numX = dnx * cny * dny;
		numY = -cnx * snx * sny * m_K;
		denom = SQR(snx) * SQR(sny) * m_K + SQR(cny);
		denom = m_Weight / Zeps(denom);
		helper.Out.x = denom * numX;
		helper.Out.y = denom * numY;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string k = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t snx, cnx, dnx;\n"
		   << "\t\treal_t sny, cny, dny;\n"
		   << "\t\treal_t numX, numY, denom;\n"
		   << "\t\tJacobiElliptic(vIn.x, " << k << ", &snx, &cnx, &dnx);\n"
		   << "\t\tJacobiElliptic(vIn.y, 1 - " << k << ", &sny, &cny, &dny);\n"
		   << "\t\tnumX = dnx * cny * dny;\n"
		   << "\t\tnumY = -cnx * snx * sny * " << k << ";\n"
		   << "\t\tdenom = fma(SQR(snx) * SQR(sny), " << k << ", SQR(cny));\n"
		   << "\t\tdenom = " << weight << " / Zeps(denom);\n"
		   << "\t\tvOut.x = denom * numX;\n"
		   << "\t\tvOut.y = denom * numY;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps", "JacobiElliptic" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_K, prefix + "jac_dn_k", T(0.5), eParamType::REAL, -1, 1));
	}

private:
	T m_K;
};

/// <summary>
/// jac_sn.
/// </summary>
template <typename T>
class JacSnVariation : public ParametricVariation<T>
{
public:
	JacSnVariation(T weight = 1.0) : ParametricVariation<T>("jac_sn", eVariationId::VAR_JAC_SN, weight)
	{
		Init();
	}

	PARVARCOPY(JacSnVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T snx, cnx, dnx;
		T sny, cny, dny;
		T numX, numY, denom;
		VarFuncs<T>::JacobiElliptic(helper.In.x, m_K, snx, cnx, dnx);
		VarFuncs<T>::JacobiElliptic(helper.In.y, 1 - m_K, sny, cny, dny);
		numX = snx * dny;
		numY = cnx * dnx * cny * sny;
		denom = SQR(snx) * SQR(sny) * m_K + SQR(cny);
		denom = m_Weight / Zeps(denom);
		helper.Out.x = denom * numX;
		helper.Out.y = denom * numY;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string k = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t snx, cnx, dnx;\n"
		   << "\t\treal_t sny, cny, dny;\n"
		   << "\t\treal_t numX, numY, denom;\n"
		   << "\t\tJacobiElliptic(vIn.x, " << k << ", &snx, &cnx, &dnx);\n"
		   << "\t\tJacobiElliptic(vIn.y, 1 - " << k << ", &sny, &cny, &dny);\n"
		   << "\t\tnumX = snx * dny;\n"
		   << "\t\tnumY = cnx * dnx * cny * sny;\n"
		   << "\t\tdenom = fma(SQR(snx) * SQR(sny), " << k << ", SQR(cny));\n"
		   << "\t\tdenom = " << weight << " / Zeps(denom);\n"
		   << "\t\tvOut.x = denom * numX;\n"
		   << "\t\tvOut.y = denom * numY;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps", "JacobiElliptic" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_K, prefix + "jac_sn_k", T(0.5), eParamType::REAL, -1, 1));
	}

private:
	T m_K;
};

/// <summary>
/// pressure_wave.
/// </summary>
template <typename T>
class PressureWaveVariation : public ParametricVariation<T>
{
public:
	PressureWaveVariation(T weight = 1.0) : ParametricVariation<T>("pressure_wave", eVariationId::VAR_PRESSURE_WAVE, weight)
	{
		Init();
	}

	PARVARCOPY(PressureWaveVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (helper.In.x + (1 / Zeps(m_X * M_2PI)) * std::sin(m_X * M_2PI * helper.In.x));
		helper.Out.y = m_Weight * (helper.In.y + (1 / Zeps(m_Y * M_2PI)) * std::sin(m_Y * M_2PI * helper.In.y));
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
		   << "\t\tvOut.x = " << weight << " * fma(((real_t)(1.0) / Zeps(" << x << " * M_2PI)), sin(" << x << " * M_2PI * vIn.x), vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * fma(((real_t)(1.0) / Zeps(" << y << " * M_2PI)), sin(" << y << " * M_2PI * vIn.y), vIn.y);\n"
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
		m_Params.push_back(ParamWithName<T>(&m_X, prefix + "pressure_wave_x_freq", 1));
		m_Params.push_back(ParamWithName<T>(&m_Y, prefix + "pressure_wave_y_freq", 1));
	}

private:
	T m_X;
	T m_Y;
};

/// <summary>
/// gamma.
/// </summary>
template <typename T>
class GammaVariation : public Variation<T>
{
public:
	GammaVariation(T weight = 1.0) : Variation<T>("gamma", eVariationId::VAR_GAMMA, weight, true, true, false, false, true)
	{
	}

	VARCOPY(GammaVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * std::lgamma(helper.m_PrecalcSqrtSumSquares);
		helper.Out.y = m_Weight * helper.m_PrecalcAtanyx;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		string weight = WeightDefineString();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * lgamma(precalcSqrtSumSquares);\n"
		   << "\t\tvOut.y = " << weight << " * precalcAtanyx;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// prose3D.
/// </summary>
template <typename T>
class PRose3DVariation : public ParametricVariation<T>
{
public:
	PRose3DVariation(T weight = 1.0) : ParametricVariation<T>("pRose3D", eVariationId::VAR_PROSE3D, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(PRose3DVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		int posNeg = 1;
		T th = 0;
		T sth, cth, pang, wig, wag, wag2, wag3, wag12 = 0, waggle = 0;
		T rad = helper.m_PrecalcSqrtSumSquares;
		T curve1 = rad / m_L;
		T curve2 = Sqr(curve1);
		T curve3 = (rad - m_L * T(0.5)) / m_L;
		T curve4 = Sqr(curve2);
		th = helper.m_PrecalcAtanyx;
		sincos(th, &sth, &cth);

		if (rand.Frand01<T>() < T(0.5))
			posNeg = -1;

		pang = th / Zeps(m_Cycle);
		wig = pang * m_Freq * T(0.5) + m_Offset * m_Cycle;

		if (m_OptDir < 0)
		{
			wag = std::sin(curve1 * T(M_PI) * m_AbsOptSc) + m_Wagsc * T(0.4) * rad + m_Crvsc * T(0.5) * (std::sin(curve2 * T(M_PI)));
			wag3 = std::sin(curve4 * T(M_PI) * m_AbsOptSc) + m_Wagsc * SQR(rad) * T(0.4) + m_Crvsc * T(0.5) * (std::cos(curve3 * T(M_PI)));
		}
		else
		{
			wag = std::sin(curve1 * T(M_PI) * m_AbsOptSc) + m_Wagsc * T(0.4) * rad + m_Crvsc * T(0.5) * (std::cos(curve3 * T(M_PI)));
			wag3 = std::sin(curve4 * T(M_PI) * m_AbsOptSc) + m_Wagsc * SQR(rad) * T(0.4) + m_Crvsc * T(0.5) * (std::sin(curve2 * T(M_PI)));
		}

		wag2 = std::sin(curve2 * T(M_PI) * m_AbsOptSc) + m_Wagsc * T(0.4) * rad + m_Crvsc * T(0.5) * (std::cos(curve3 * T(M_PI)));

		if (m_Smooth12 <= 1)
			wag12 = wag;
		else if (m_Smooth12 <= 2 && m_Smooth12 > 1)
			wag12 = wag2 * (1 - m_AntiOpt1) + wag * m_AntiOpt1;
		else if (m_Smooth12 > 2)
			wag12 = wag2;

		if (m_Smooth3 == 0)
			waggle = wag12;
		else if (m_Smooth3 > 0)
			waggle = wag12 * (1 - m_Smooth3) + wag3 * m_Smooth3;

		if (rand.Frand01<T>() < m_Ghost)
		{
			if (posNeg < 0)
			{
				helper.Out.x = m_Weight * T(0.5) * m_RefSc * (m_L * std::cos(m_NumPetals * th + m_C)) * cth;
				helper.Out.y = m_Weight * T(0.5) * m_RefSc * (m_L * std::cos(m_NumPetals * th + m_C)) * sth;
				helper.Out.z = m_Weight * T(-0.5) * ((m_Z2 * waggle + Sqr(rad * T(0.5)) * std::sin(wig) * m_WigScale) + m_Dist);
			}
			else
			{
				helper.Out.x = m_Weight * T(0.5) * (m_L * std::cos(m_NumPetals * th + m_C)) * cth;
				helper.Out.y = m_Weight * T(0.5) * (m_L * std::cos(m_NumPetals * th + m_C)) * sth;
				helper.Out.z = m_Weight * T(0.5) * ((m_Z1 * waggle + Sqr(rad * T(0.5)) * std::sin(wig) * m_WigScale) + m_Dist);
			}
		}
		else
		{
			if (posNeg < 0)
			{
				helper.Out.x = m_Weight * T(0.5) * (m_L * std::cos(m_NumPetals * th + m_C)) * cth;
				helper.Out.y = m_Weight * T(0.5) * (m_L * std::cos(m_NumPetals * th + m_C)) * sth;
				helper.Out.z = m_Weight * T(0.5) * ((m_Z1 * waggle + Sqr(rad * T(0.5)) * std::sin(wig) * m_WigScale) + m_Dist);
			}
			else
			{
				helper.Out.x = m_Weight * T(0.5) * (m_L * std::cos(m_NumPetals * th + m_C)) * cth;
				helper.Out.y = m_Weight * T(0.5) * (m_L * std::cos(m_NumPetals * th + m_C)) * sth;
				helper.Out.z = m_Weight * T(0.5) * ((m_Z1 * waggle + Sqr(rad * T(0.5)) * std::sin(wig) * m_WigScale) + m_Dist);
			}
		}
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string l          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string k          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string z1         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string z2         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string refSc      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string opt        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string optSc      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string opt3       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string transp     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string wagsc      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string crvsc      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string f          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string wigsc      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string offset     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cycle      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;//Precalc.
		string optDir     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string petalsSign = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string numPetals  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string absOptSc   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string smooth12   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string smooth3    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string antiOpt1   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ghost      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freq       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string wigScale   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tint posNeg = 1;\n"
		   << "\t\treal_t th = 0;\n"
		   << "\t\treal_t sth, cth, pang, wig, wag, wag2, wag3, wag12, waggle;\n"
		   << "\t\treal_t rad = precalcSqrtSumSquares;\n"
		   << "\t\treal_t curve1 = rad / " << l << ";\n"
		   << "\t\treal_t curveTwo = Sqr(curve1);\n"
		   << "\t\treal_t curve3 = (rad - " << l << " * 0.5) / " << l << ";\n"
		   << "\t\treal_t curve4 = Sqr(curveTwo);\n"
		   << "\t\tth = precalcAtanyx;\n"
		   << "\t\tsth = sincos(th, &cth);\n"
		   << "\n"
		   << "\t\tif (MwcNext01(mwc) < 0.5)\n"
		   << "\t\t	posNeg = -1;\n"
		   << "\n"
		   << "\t\tpang = th / Zeps(" << cycle << ");\n"
		   << "\t\twig = fma(pang, " << freq << " * (real_t)(0.5), " << offset << " * " << cycle << ");\n"
		   << "\t\treal_t rad04 = (real_t)(0.4) * rad;\n"
		   << "\n"
		   << "\t\tif (" << optDir << " < 0)\n"
		   << "\t\t{\n"
		   << "\t\t	wag  = sin(curve1* MPI * " << absOptSc << ") + fma(" << wagsc << ", rad04, " << crvsc << " * (real_t)(0.5) * sin(curveTwo * MPI)); \n"
		   << "\t\t	wag3 = sin(curve4* MPI * " << absOptSc << ") + fma(" << wagsc << ", SQR(rad) * (real_t)(0.4), " << crvsc << " * (real_t)(0.5) * cos(curve3 * MPI)); \n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	wag  = sin(curve1* MPI * " << absOptSc << ") + fma(" << wagsc << ", rad04, " << crvsc << " * (real_t)(0.5) * cos(curve3 * MPI)); \n"
		   << "\t\t	wag3 = sin(curve4* MPI * " << absOptSc << ") + fma(" << wagsc << ", SQR(rad) * (real_t)(0.4), " << crvsc << " * (real_t)(0.5) * sin(curveTwo * MPI)); \n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\twag2 = sin(curveTwo * MPI * " << absOptSc << ") + fma(" << wagsc << ", rad04, " << crvsc << " * (real_t)(0.5) * cos(curve3 * MPI)); \n"
		   << "\n"
		   << "\t\tif (" << smooth12 << " <= 1)\n"
		   << "\t\t	wag12 = wag; \n"
		   << "\t\telse if (" << smooth12 << " <= 2 && " << smooth12 << " > 1)\n"
		   << "\t\t	wag12 = fma(wag2, ((real_t)(1.0) - " << antiOpt1 << "), wag * " << antiOpt1 << "); \n"
		   << "\t\telse if (" << smooth12 << " > 2)\n"
		   << "\t\t	wag12 = wag2; \n"
		   << "\n"
		   << "\t\tif (" << smooth3 << " == 0)\n"
		   << "\t\t	waggle = wag12; \n"
		   << "\t\telse if (" << smooth3 << " > 0)\n"
		   << "\t\t	waggle = fma(wag12, ((real_t)(1.0) - " << smooth3 << "), wag3 * " << smooth3 << "); \n"
		   << "\n"
		   << "\t\treal_t cospetthcl = " << weight << " * (real_t)(0.5) * " << "cos(fma(" << numPetals << ", th, " << c << ")) * " << l << "; \n"
		   << "\n"
		   << "\t\tif (MwcNext01(mwc) < " << ghost << ")\n"
		   << "\t\t{\n"
		   << "\t\t	if (posNeg < 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = " << refSc << "* cospetthcl * cth; \n"
		   << "\t\t		vOut.y = " << refSc << "* cospetthcl * sth; \n"
		   << "\t\t		vOut.z = " << weight << " * (real_t)(-0.5) * (fma(" << z2 << ", waggle, Sqr(rad * (real_t)(0.5)) * sin(wig) * " << wigScale << ") + " << dist << "); \n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = cospetthcl * cth; \n"
		   << "\t\t		vOut.y = cospetthcl * sth; \n"
		   << "\t\t		vOut.z = " << weight << " * (real_t)(0.5) * (fma(" << z1 << ", waggle, Sqr(rad * (real_t)(0.5)) * sin(wig) * " << wigScale << ") + " << dist << "); \n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (posNeg < 0)\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = cospetthcl * cth; \n"
		   << "\t\t		vOut.y = cospetthcl * sth; \n"
		   << "\t\t		vOut.z = " << weight << " * (real_t)(0.5) * (fma(" << z1 << ", waggle, Sqr(rad * (real_t)(0.5)) * sin(wig) * " << wigScale << ") + " << dist << "); \n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = cospetthcl * cth; \n"
		   << "\t\t		vOut.y = cospetthcl * sth; \n"
		   << "\t\t		vOut.z = " << weight << " * (real_t)(0.5) * (fma(" << z1 << ", waggle, Sqr(rad * (real_t)(0.5)) * sin(wig) * " << wigScale << ") + " << dist << "); \n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Sqr", "Zeps" };
	}

	virtual void Precalc() override
	{
		m_Cycle = M_2PI / Zeps(m_K);
		m_NumPetals = m_K;
		m_Ghost = Sqr(m_Transp);
		m_Freq = m_F * M_2PI;
		m_WigScale = m_Wigsc * T(0.5);

		if (std::abs(m_NumPetals) < T(0.0001))
			m_NumPetals = T(0.0001) * m_PetalsSign;

		m_Smooth12 = std::abs(m_Opt);
		m_Smooth3 = std::abs(m_Opt3);
		m_AbsOptSc = std::abs(m_OptSc);

		if (m_Smooth12 > 2)
			m_Smooth12 = 2;

		m_AntiOpt1 = 2 - m_Smooth12;

		if (m_Smooth3 > 1)
			m_Smooth3 = 1;

		m_OptDir = std::copysign(T(1.0), m_Opt);
		m_PetalsSign = std::copysign(T(1.0), m_K);

		if (m_Opt3 < 0)
			m_OptDir = -1;

		if (m_OptDir > 0)
			m_Ghost = 0;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_L, prefix + "pRose3D_l", 1, eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_K, prefix + "pRose3D_k", 3));
		m_Params.push_back(ParamWithName<T>(&m_C, prefix + "pRose3D_c"));
		m_Params.push_back(ParamWithName<T>(&m_Z1, prefix + "pRose3D_z1", 1));
		m_Params.push_back(ParamWithName<T>(&m_Z2, prefix + "pRose3D_z2", 1));
		m_Params.push_back(ParamWithName<T>(&m_RefSc, prefix + "pRose3D_refSc", 1));
		m_Params.push_back(ParamWithName<T>(&m_Opt, prefix + "pRose3D_opt", 1));
		m_Params.push_back(ParamWithName<T>(&m_OptSc, prefix + "pRose3D_optSc", 1));
		m_Params.push_back(ParamWithName<T>(&m_Opt3, prefix + "pRose3D_opt3"));
		m_Params.push_back(ParamWithName<T>(&m_Transp, prefix + "pRose3D_transp", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Dist, prefix + "pRose3D_dist", 1));
		m_Params.push_back(ParamWithName<T>(&m_Wagsc, prefix + "pRose3D_wagsc", 0));
		m_Params.push_back(ParamWithName<T>(&m_Crvsc, prefix + "pRose3D_crvsc", 0));
		m_Params.push_back(ParamWithName<T>(&m_F, prefix + "pRose3D_f", 3));
		m_Params.push_back(ParamWithName<T>(&m_Wigsc, prefix + "pRose3D_wigsc"));
		m_Params.push_back(ParamWithName<T>(&m_Offset, prefix + "pRose3D_offset"));
		m_Params.push_back(ParamWithName<T>(true, &m_Cycle, prefix + "pRose3D_cycle"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_OptDir, prefix + "pRose3D_opt_dir"));
		m_Params.push_back(ParamWithName<T>(true, &m_PetalsSign, prefix + "pRose3D_petals_sign"));
		m_Params.push_back(ParamWithName<T>(true, &m_NumPetals, prefix + "pRose3D_num_petals"));
		m_Params.push_back(ParamWithName<T>(true, &m_AbsOptSc, prefix + "pRose3D_abs_optSc"));
		m_Params.push_back(ParamWithName<T>(true, &m_Smooth12, prefix + "pRose3D_smooth12"));
		m_Params.push_back(ParamWithName<T>(true, &m_Smooth3, prefix + "pRose3D_smooth3"));
		m_Params.push_back(ParamWithName<T>(true, &m_AntiOpt1, prefix + "pRose3D_anti_opt1"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ghost, prefix + "pRose3D_ghost"));
		m_Params.push_back(ParamWithName<T>(true, &m_Freq, prefix + "pRose3D_freq"));
		m_Params.push_back(ParamWithName<T>(true, &m_WigScale, prefix + "pRose3D_wig_scale"));
	}

private:
	T m_L;
	T m_K;
	T m_C;
	T m_Z1;
	T m_Z2;
	T m_RefSc;
	T m_Opt;
	T m_OptSc;
	T m_Opt3;
	T m_Transp;
	T m_Dist;
	T m_Wagsc;
	T m_Crvsc;
	T m_F;
	T m_Wigsc;
	T m_Offset;
	T m_Cycle;//Precalc.
	T m_OptDir;
	T m_PetalsSign;
	T m_NumPetals;
	T m_AbsOptSc;
	T m_Smooth12;
	T m_Smooth3;
	T m_AntiOpt1;
	T m_Ghost;
	T m_Freq;
	T m_WigScale;
};

/// <summary>
/// log_db.
/// By dark-beam, taken from JWildfire.
/// http://jwildfire.org/forum/viewtopic.php?f=23&t=1450&p=2692#p2692
/// </summary>
template <typename T>
class LogDBVariation : public ParametricVariation<T>
{
public:
	LogDBVariation(T weight = 1.0) : ParametricVariation<T>("log_db", eVariationId::VAR_LOG_DB, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(LogDBVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		int i, adp;
		T atanPeriod = 0;

		for (i = 0; i < 7; i++)
		{
			adp = rand.Rand(10) - 5;

			if (std::abs(adp) >= 3)
				adp = 0;

			atanPeriod += adp;
		}

		atanPeriod *= m_FixPe;
		helper.Out.x = m_Denom * std::log(helper.m_PrecalcSumSquares);
		helper.Out.y = m_Weight * (helper.m_PrecalcAtanyx + atanPeriod);
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
		string fixPeriod = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string denom = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fixPe = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tint i, adp;\n"
		   << "\t\treal_t atanPeriod = 0;\n"
		   << "\n"
		   << "\t\tfor (i = 0; i < 7; i++)\n"
		   << "\t\t{\n"
		   << "\t\t	adp = MwcNextRange(mwc, 10u) - 5;\n"
		   << "\n"
		   << "\t\t	if (abs(adp) >= 3)\n"
		   << "\t\t		adp = 0;\n"
		   << "\n"
		   << "\t\t	atanPeriod += adp;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tatanPeriod *= " << fixPe << ";\n"
		   << "\t\tvOut.x = " << denom << " * log(precalcSumSquares);\n"
		   << "\t\tvOut.y = " << weight << " * (precalcAtanyx + atanPeriod);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Denom = T(0.5);

		if (m_Base > EPS)
			m_Denom = m_Denom / std::log(T(M_E) * m_Base);

		m_Denom *= m_Weight;
		m_FixPe = T(M_PI);

		if (m_FixPeriod > EPS)
			m_FixPe *= m_FixPeriod;
	}
protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Base, prefix + "log_db_base", 1));
		m_Params.push_back(ParamWithName<T>(&m_FixPeriod, prefix + "log_db_fix_period", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Denom, prefix + "log_db_denom"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_FixPe, prefix + "log_db_fix_pe"));
	}
private:
	T m_Base;
	T m_FixPeriod;
	T m_Denom;
	T m_FixPe;
};

/// <summary>
/// circlesplit.
/// By tatasz.
/// http://fav.me/dapecsh
/// </summary>
template <typename T>
class CircleSplitVariation : public ParametricVariation<T>
{
public:
	CircleSplitVariation(T weight = 1.0) : ParametricVariation<T>("circlesplit", eVariationId::VAR_CIRCLESPLIT, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(CircleSplitVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x1, y1;

		if (helper.m_PrecalcSqrtSumSquares < (m_Radius - m_Split))
		{
			x1 = helper.In.x;
			y1 = helper.In.y;
		}
		else
		{
			T a = helper.m_PrecalcAtanyx;
			T len = helper.m_PrecalcSqrtSumSquares + m_Split;
			x1 = std::cos(a) * len;
			y1 = std::sin(a) * len;
		}

		helper.Out.x = m_Weight * x1;
		helper.Out.y = m_Weight * y1;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string cs_radius = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cs_split = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x1, y1;\n"
		   << "\n"
		   << "\t\tif (precalcSqrtSumSquares < (" << cs_radius << " - " << cs_split << "))\n"
		   << "\t\t{\n"
		   << "\t\t\tx1 = vIn.x;\n"
		   << "\t\t\ty1 = vIn.y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t\treal_t a = precalcAtanyx;\n"
		   << "\t\t\treal_t len = precalcSqrtSumSquares + " << cs_split << ";\n"
		   << "\t\t\tx1 = cos(a) * len;\n"
		   << "\t\t\ty1 = sin(a) * len;\n"
		   << "\t\t}"
		   << "\t\tvOut.x = " << weight << " * x1;\n"
		   << "\t\tvOut.y = " << weight << " * y1;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "circlesplit_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_Split, prefix + "circlesplit_split", T(0.5)));
	}

private:
	T m_Radius;
	T m_Split;
};

/// <summary>
/// cylinder2.
/// By tatasz.
/// http://fav.me/dapecsh
/// </summary>
template <typename T>
class Cylinder2Variation : public Variation<T>
{
public:
	Cylinder2Variation(T weight = 1.0) : Variation<T>("cylinder2", eVariationId::VAR_CYLINDER2, weight) { }

	VARCOPY(Cylinder2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (helper.In.x / Zeps(std::sqrt(SQR(helper.In.x) + 1)));
		helper.Out.y = m_Weight * helper.In.y;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (vIn.x / Zeps(sqrt(fma(vIn.x, vIn.x, (real_t)1.0))));\n"
		   << "\t\tvOut.y = " << weight << " * vIn.y;\n"
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
/// tile_hlp.
/// By zy0rg.
/// </summary>
template <typename T>
class TileHlpVariation : public ParametricVariation<T>
{
public:
	TileHlpVariation(T weight = 1.0) : ParametricVariation<T>("tile_hlp", eVariationId::VAR_TILE_HLP, weight)
	{
		Init();
	}

	PARVARCOPY(TileHlpVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T temp, x = helper.In.x / m_Width;
		bool pos = x > 0;

        if (std::cos((pos ? x - int(x) : x + int(x)) * T(M_PI)) < rand.Frand11<T>())
			temp = pos ? -m_Vwidth : m_Vwidth;
		else
			temp = 0;

		helper.Out.x = m_Weight * helper.In.x + temp;
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
		string width  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string vwidth = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t temp, x = vIn.x / Zeps(" << width << ");\n"
		   << "\t\tbool pos = x > 0;\n"
		   << "\n"
		   << "\t\tif (cos((pos ? x - (int)x : x + (int)x) * MPI) < MwcNextNeg1Pos1(mwc))\n"
		   << "\t\t	temp = pos ? -" << vwidth << " : " << vwidth << ";\n"
		   << "\t\telse\n"
		   << "\t\t	temp = 0;\n"
		   << "\t\tvOut.x = fma(" << weight << ", vIn.x, temp);\n"
		   << "\t\tvOut.y = " << weight << " * vIn.y;\n"
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
		m_Vwidth = m_Width * m_Weight;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Width,        prefix + "tile_hlp_width", 1, eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(true, &m_Vwidth, prefix + "tile_hlp_v_width"));//Precalc.
	}

private:
	T m_Width;
	T m_Vwidth;//Precalc.
};

/// <summary>
/// tile_log.
/// By zy0rg.
/// </summary>
template <typename T>
class TileLogVariation : public ParametricVariation<T>
{
public:
	TileLogVariation(T weight = 1.0) : ParametricVariation<T>("tile_log", eVariationId::VAR_TILE_LOG, weight)
	{
		Init();
	}

	PARVARCOPY(TileLogVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T temp = Round(std::log(rand.Frand01<T>()) * (rand.RandBit() ? m_Spread : -m_Spread));
		helper.Out.x = m_Weight * (helper.In.x + temp);
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
		string spread = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t temp = (real_t) (Round(log(MwcNext01(mwc)) * (MwcNext(mwc) & 1 ? " << spread << " : -" << spread << ")));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (vIn.x + temp);\n"
		   << "\t\tvOut.y = " << weight << " * vIn.y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Round" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Spread, prefix + "tile_log_spread", 1));
	}

private:
	T m_Spread;
};

/// <summary>
/// Truchet_fill.
/// By tatasz.
/// http://fav.me/dapecsh
/// </summary>
template <typename T>
class TruchetFillVariation : public ParametricVariation<T>
{
public:
	TruchetFillVariation(T weight = 1.0) : ParametricVariation<T>("Truchet_fill", eVariationId::VAR_TRUCHET_FILL, weight)
	{
		Init();
	}

	PARVARCOPY(TruchetFillVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T modbase = T(65535);
		T multiplier = T(32747); //1103515245;
		T offset = T(12345);
		T x = helper.In.x * m_Scale;
		T y = helper.In.y * m_Scale;
		T intx = Round(x);
		T inty = Round(y);
		T r = x - intx;

		if (r < 0)
			x = 1 + r;
		else
			x = r;

		r = y - inty;

		if (r < 0)
			y = 1 + r;
		else
			y = r;

		T tiletype = 0;

		if (m_Seed != 0)
		{
			if (m_Seed == 1)
			{
				tiletype = m_Seed;
			}
			else
			{
				T xrand = helper.In.x;
				T yrand = helper.In.y;
				xrand = Round(std::abs(xrand)) * m_Seed2;
				yrand = Round(std::abs(yrand)) * m_Seed2;
				T niter = xrand + yrand + (xrand * yrand);
				T randint = (m_Seed + niter) * m_Seed2 * T(0.5);
				randint = std::fmod((randint * multiplier + offset), modbase);
				tiletype = std::fmod(randint, T(2));
			}
		}

		T xval1, xval2;

		if (tiletype < 1)
		{
			xval1 = x;
			xval2 = x - 1;
		}
		else
		{
			xval1 = x - 1;
			xval2 = x;
		}

		T r0 = std::pow((std::pow(std::fabs(xval1), m_FinalExponent) + std::pow(std::fabs(y), m_FinalExponent)), m_OneOverEx);
		T r1 = std::pow((std::pow(std::fabs(xval2), m_FinalExponent) + std::pow(std::fabs(y - 1), m_FinalExponent)), m_OneOverEx);
		T x1, y1;
		T r00 = fabs(r0 - T(0.5)) / m_Rmax;

		if (r00 < 1)
		{
			x1 = 2 * (x + Floor(helper.In.x));
			y1 = 2 * (y + Floor(helper.In.y));
		}
		else
		{
			x1 = 0;
			y1 = 0;
		}

		T r11 = std::fabs(r1 - T(0.5)) / m_Rmax;

		if (r11 < 1)
		{
			helper.Out.x = x1 + (2 * (x + Floor(helper.In.x))) - helper.In.x;
			helper.Out.y = y1 + (2 * (y + Floor(helper.In.y))) - helper.In.y;
		}
		else
		{
			helper.Out.x = x1 - helper.In.x;
			helper.Out.y = y1 - helper.In.y;
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
		string exponent      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string arcWidth      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string finalexponent = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneOverEx     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string width         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed2         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rmax          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t modbase = 65535;\n"
		   << "\t\treal_t multiplier = 32747;\n"
		   << "\t\treal_t offset = 12345;\n"
		   << "\t\treal_t sscale = " << scale << ";\n"
		   << "\t\treal_t seed = " << seed << ";\n"
		   << "\t\treal_t seed2 = " << seed2 << ";\n"
		   << "\t\treal_t finalexponent = " << finalexponent << ";\n"
		   << "\t\treal_t oneOverEx = " << oneOverEx << ";\n"
		   << "\t\treal_t rmax = " << rmax << ";\n"
		   << "\t\treal_t x = vIn.x * " << scale << ";\n"
		   << "\t\treal_t y = vIn.y * " << scale << ";\n"
		   << "\t\treal_t intx = Round(x);\n"
		   << "\t\treal_t inty = Round(y);\n"
		   << "\n"
		   << "\t\treal_t r = x - intx;\n"
		   << "\n"
		   << "\t\tif (r < 0)\n"
		   << "\t\t\tx = r + 1;\n"
		   << "\t\telse\n"
		   << "\t\t\tx = r;\n"
		   << "\n"
		   << "\t\tr = y - inty;\n"
		   << "\n"
		   << "\t\tif (r < 0)\n"
		   << "\t\t\ty = r + 1;\n"
		   << "\t\telse\n"
		   << "\t\t\ty = r;\n"
		   << "\n"
		   << "\t\treal_t tiletype = 0;\n"
		   << "\n"
		   << "\t\tif (seed != 0)\n"
		   << "\t\t{\n"
		   << "\t\t\tif (seed == 1)\n"
		   << "\t\t\t{\n"
		   << "\t\t\t\ttiletype = seed;\n"
		   << "\t\t\t}\n"
		   << "\t\t\telse\n"
		   << "\t\t\t{\n"
		   << "\t\t\t\treal_t xrand = vIn.x;\n"
		   << "\t\t\t\treal_t yrand = vIn.y;\n"
		   << "\n"
		   << "\t\t\t\txrand = Round(fabs(xrand)) * seed2;\n"
		   << "\t\t\t\tyrand = Round(fabs(yrand)) * seed2;\n"
		   << "\n"
		   << "\t\t\t\treal_t niter = fma(xrand, yrand, xrand + yrand);\n"
		   << "\t\t\t\treal_t randint = (seed + niter) * seed2 * (real_t)0.5;\n"
		   << "\n"
		   << "\t\t\t\trandint = fmod(fma(randint, multiplier, offset), modbase);\n"
		   << "\t\t\t\ttiletype = fmod(randint, 2);\n"
		   << "\t\t\t}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t xval1, xval2;\n"
		   << "\n"
		   << "\t\tif (tiletype < 1)\n"
		   << "\t\t{\n"
		   << "\t\t	xval1 = x;\n"
		   << "\t\t	xval2 = x - 1;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	xval1 = x - 1;\n"
		   << "\t\t	xval2 = x;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\t\treal_t r0 = pow((pow(fabs(xval1), finalexponent) + pow(fabs(y), finalexponent)), oneOverEx);\n"
		   << "\t\t\treal_t r1 = pow((pow(fabs(xval2), finalexponent) + pow(fabs(y-1), finalexponent)), oneOverEx);\n"
		   << "\n"
		   << "\t\treal_t x1, y1;\n"
		   << "\t\treal_t r00 = fabs(r0 - (real_t)0.5) / rmax;\n"
		   << "\n"
		   << "\t\tif (r00 < 1.0)\n"
		   << "\t\t{\n"
		   << "\t\t\tx1 = 2 * (x + floor(vIn.x));\n"
		   << "\t\t\ty1 = 2 * (y + floor(vIn.y));\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t\tx1 = 0;\n"
		   << "\t\t\ty1 = 0;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t r11 = fabs(r1 - (real_t)0.5) / rmax;\n"
		   << "\n"
		   << "\t\tif (r11 < 1)\n"
		   << "\t\t{\n"
		   << "\t\t\tvOut.x = x1 + (2 * (x + floor(vIn.x))) - vIn.x;\n"
		   << "\t\t\tvOut.y = y1 + (2 * (y + floor(vIn.y))) - vIn.y;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t\tvOut.x = x1 - vIn.x;\n"
		   << "\t\t\tvOut.y = y1 - vIn.y;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Round" };
	}

	virtual void Precalc() override
	{
		m_FinalExponent = m_Exponent > T(2) ? T(2) : (m_Exponent < T(0.001) ? T(0.001) : m_Exponent);
		m_OneOverEx = T(1) / m_FinalExponent;
		m_Width = m_ArcWidth > T(1) ? T(1) : (m_ArcWidth < T(0.001) ? T(0.001) : m_ArcWidth);
		m_Seed2 = std::sqrt(m_Seed * T(1.5)) / Zeps(m_Seed * T(0.5)) * T(0.25);
		m_Rmax = Zeps(T(0.5) * (std::pow(T(2), m_OneOverEx) - T(1)) * m_Width);
		m_Scale = T(1) / m_Weight;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Exponent,            prefix + "Truchet_fill_exponent", 2, eParamType::REAL_CYCLIC, T(0.001), 2));
		m_Params.push_back(ParamWithName<T>(&m_ArcWidth,            prefix + "Truchet_fill_arc_width", T(0.5), eParamType::REAL_CYCLIC, T(0.001), 1));
		m_Params.push_back(ParamWithName<T>(&m_Seed,                prefix + "Truchet_fill_seed"));
		m_Params.push_back(ParamWithName<T>(true, &m_FinalExponent, prefix + "Truchet_fill_final_exponent"));//Precalc
		m_Params.push_back(ParamWithName<T>(true, &m_OneOverEx,     prefix + "Truchet_fill_oneoverex"));
		m_Params.push_back(ParamWithName<T>(true, &m_Width,         prefix + "Truchet_fill_width"));
		m_Params.push_back(ParamWithName<T>(true, &m_Seed2,         prefix + "Truchet_fill_seed2"));
		m_Params.push_back(ParamWithName<T>(true, &m_Rmax,          prefix + "Truchet_fill_rmax"));
		m_Params.push_back(ParamWithName<T>(true, &m_Scale,         prefix + "Truchet_fill_scale"));
	}

private:
	T m_Exponent;
	T m_ArcWidth;
	T m_Seed;
	T m_FinalExponent;//Precalc.
	T m_OneOverEx;
	T m_Width;
	T m_Seed2;
	T m_Rmax;
	T m_Scale;
};

/// <summary>
/// Truchet_hex_fill.
/// By tatasz.
/// http://fav.me/dd9ay2c
/// </summary>
template <typename T>
class TruchetHexFillVariation : public ParametricVariation<T>
{
public:
	TruchetHexFillVariation(T weight = 1.0) : ParametricVariation<T>("Truchet_hex_fill", eVariationId::VAR_TRUCHET_HEX_FILL, weight)
	{
		Init();
	}

	PARVARCOPY(TruchetHexFillVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		//round
		T rx = T(Floor<T>(std::log(rand.Frand01<T>()) * (rand.Frand01<T>() < T(0.5) ? m_Spreadx : -m_Spreadx)));
		T rz = T(Floor<T>(std::log(rand.Frand01<T>()) * (rand.Frand01<T>() < T(0.5) ? m_Spready : -m_Spready)));
		T FX_h = M_SQRT3 * rx + M_SQRT3_2 * (rand.Frand01<T>() < T(0.5) ? rz : -rz);
		T FY_h = T(1.5) * rz;
		bool add = true;

		if (m_Seed == 1)
		{
			if (!(int(rx) & 1) && !(int(rz) & 1))
				add = false;
		}
		else if (m_Seed >= 2)
		{
			T hash_f = std::sin(FX_h * T(12.9898) + FY_h * T(78.233) + m_Seed) * T(43758.5453);
			hash_f = hash_f - Floor<T>(hash_f);

			if (hash_f < T(0.5))
				add = false; //pi/3
		}

		//exponential to make a tiled circle
		T rangle = Floor<T>(rand.Frand01<T>() * m_3nPrecalc) * M_2PI * m_13nPrecalc;
		T y_aux = m_Flipy ? (add ? helper.In.y : -helper.In.y) : helper.In.y;
		T x_aux = m_Flipx ? (add ? helper.In.x : -helper.In.x) : helper.In.x;
		T FY = y_aux * m_13nPrecalc;
		T FX = x_aux * m_13nPrecalc;
		T ang = FY * T(M_PI) + rangle;
		T a = 1 + FX * T(M_PI);
		FX = a * std::cos(ang);
		FY = a * std::sin(ang);
		//split
		T A = std::atan2(FY, FX);

		if (A < 0)
			A += M_2PI;

		A = T(Floor<T>(T(1.5) * A * T(M_1_PI)));
		ang = (T(M_PI) + A * M_2PI) / 3;
		T FX_new = FX - std::cos(ang) * 2;
		T FY_new = FY - std::sin(ang) * 2;

		//rotate by 30 to fit the hex
		if (add)
		{
			FX = M_SQRT3_2 * FX_new - T(0.5) * FY_new;
			FY = T(0.5) * FX_new + M_SQRT3_2 * FY_new;
		}
		else
		{
			FX = M_SQRT3_2 * FX_new + T(0.5) * FY_new;
			FY = -T(0.5) * FX_new + M_SQRT3_2 * FY_new;
		}

		helper.Out.x = m_Weight * (FX * T(0.5) + FX_h);
		helper.Out.y = m_Weight * (FY * T(0.5) + FY_h);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string n           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string flipx       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string flipy       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spreadx     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spready     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string p3nprecalc  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string p13nprecalc = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\t//round\n"
		   << "\t\treal_t rx = floor(log(MwcNext01(mwc)) * (MwcNext01(mwc) < (real_t)(0.5) ? " << spreadx << " : -" << spreadx << "));\n"
		   << "\t\treal_t rz = floor(log(MwcNext01(mwc)) * (MwcNext01(mwc) < (real_t)(0.5) ? " << spready << " : -" << spready << "));\n"
		   << "\t\treal_t FX_h = M_SQRT3 * rx + M_SQRT3_2 * (MwcNext01(mwc) < (real_t)(0.5) ? rz : -rz);\n"
		   << "\t\treal_t FY_h = 1.5 * rz;\n"
		   << "\n"
		   << "\t\tbool add = true;\n"
		   << "\n"
		   << "\t\tif (" << seed << " == (real_t)(1.0))\n"
		   << "\t\t{\n"
		   << "\t\t	if (!((int)rx & 1) && !((int)rz & 1))\n"
		   << "\t\t		add = false;\n"
		   << "\t\t}\n"
		   << "\t\telse if (" << seed << " >= 2)\n"
		   << "\t\t{\n"
		   << "\t\t	real_t hash_f = sin(fma(FX_h, (real_t)(12.9898), fma(FY_h, (real_t)(78.233), " << seed << "))) * (real_t)(43758.5453);\n"
		   << "\t\t	hash_f = hash_f - floor(hash_f);\n"
		   << "\n"
		   << "\t\t	if (hash_f < (real_t)(0.5))\n"
		   << "\t\t		add = false; //pi/3\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\t//exponential to make a tiled circle\n"
		   << "\t\treal_t rangle = floor(MwcNext01(mwc) * " << p3nprecalc << ") * M_2PI * " << p13nprecalc << ";\n"
		   << "\t\treal_t y_aux = " << flipy << " != (real_t)(0.0) ? (add ? vIn.y : -vIn.y) : vIn.y;\n"
		   << "\t\treal_t x_aux = " << flipx << " != (real_t)(0.0) ? (add ? vIn.x : -vIn.x) : vIn.x;\n"
		   << "\t\treal_t FY = y_aux * " << p13nprecalc << ";\n"
		   << "\t\treal_t FX = x_aux * " << p13nprecalc << ";\n"
		   << "\t\treal_t ang = fma(FY, MPI, rangle);\n"
		   << "\t\treal_t a = fma(FX, MPI, (real_t)(1.0));\n"
		   << "\t\tFX = a * cos(ang);\n"
		   << "\t\tFY = a * sin(ang);\n"
		   << "\t\t//split\n"
		   << "\t\treal_t A = atan2(FY, FX);\n"
		   << "\n"
		   << "\t\tif (A < 0) A += M_2PI;\n"
		   << "\t\t	A = floor((real_t)(1.5) * A * M1PI);\n"
		   << "\n"
		   << "\t\tang = fma(A, M_2PI, MPI) / (real_t)(3.0);\n"
		   << "\t\treal_t FX_new = FX - cos(ang) * (real_t)(2.0);\n"
		   << "\t\treal_t FY_new = FY - sin(ang) * (real_t)(2.0);\n"
		   << "\t\t//rotate by 30 to fit the hex\n"
		   << "\t\tif (add)\n"
		   << "\t\t{\n"
		   << "\t\t	FX = fma(M_SQRT3_2, FX_new, -((real_t)(0.5) * FY_new));\n"
		   << "\t\t	FY = fma((real_t)(0.5), FX_new, M_SQRT3_2 * FY_new);\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	FX = fma(M_SQRT3_2, FX_new, (real_t)(0.5) * FY_new);\n"
		   << "\t\t	FY = fma(-(real_t)(0.5), FX_new, M_SQRT3_2 * FY_new);\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(FX, (real_t)(0.5), FX_h);\n"
		   << "\t\tvOut.y = " << weight << " * fma(FY, (real_t)(0.5), FY_h);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_3nPrecalc = 3 * m_N;
		m_13nPrecalc = 1 / Zeps(m_3nPrecalc);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_N,                prefix + "Truchet_hex_fill_n", 3, eParamType::INTEGER));
		m_Params.push_back(ParamWithName<T>(&m_Flipx,            prefix + "Truchet_hex_fill_flipx", 1, eParamType::INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Flipy,            prefix + "Truchet_hex_fill_flipy", 1, eParamType::INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Spreadx,          prefix + "Truchet_hex_fill_spreadx", 1));
		m_Params.push_back(ParamWithName<T>(&m_Spready,          prefix + "Truchet_hex_fill_spready", 1));
		m_Params.push_back(ParamWithName<T>(&m_Seed,             prefix + "Truchet_hex_fill_seed", 0, eParamType::INTEGER));
		m_Params.push_back(ParamWithName<T>(true, &m_3nPrecalc,  prefix + "Truchet_hex_fill_3n_precalc"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_13nPrecalc, prefix + "Truchet_hex_fill_1_3n_precalc"));
	}

private:
	T m_N;
	T m_Flipx;
	T m_Flipy;
	T m_Spreadx;
	T m_Spready;
	T m_Seed;
	T m_3nPrecalc;//Precalc.
	T m_13nPrecalc;
};

/// <summary>
/// Truchet_hex_crop.
/// By tatasz.
/// http://fav.me/dd9ay2c
/// </summary>
template <typename T>
class TruchetHexCropVariation : public ParametricVariation<T>
{
public:
	TruchetHexCropVariation(T weight = 1.0) : ParametricVariation<T>("Truchet_hex_crop", eVariationId::VAR_TRUCHET_HEX_CROP, weight)
	{
		Init();
	}

	PARVARCOPY(TruchetHexCropVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		//get hex
		T yover3 = helper.In.y / 3;
		T x = M_SQRT3_3 * helper.In.x - yover3;
		T z = T(2.0) * yover3;
		T y = -x - z;
		//round
		T rx = std::round(x);
		T ry = std::round(y);
		T rz = std::round(z);
		T x_diff = std::abs(rx - x);
		T y_diff = std::abs(ry - y);
		T z_diff = std::abs(rz - z);

		if ((x_diff > y_diff) && (x_diff > z_diff))
			rx = -ry - rz;
		else if (y_diff > z_diff)
			ry = -rx - rz;
		else
			rz = -rx - ry;

		T FX_h = M_SQRT3 * rx + M_SQRT3_2 * rz;
		T FY_h = T(1.5) * rz;
		T FX = helper.In.x - FX_h;
		T FY = helper.In.y - FY_h;
		T add = 0.0;

		if (m_Seed == 1)
		{
			if (!(int(rx) & 1) && !(int(rz) & 1))
				add = M_PI3;
		}
		else if (m_Seed >= 2)
		{
			T hash_f = std::sin(FX_h * T(12.9898) + FY_h * T(78.233) + m_Seed) * T(43758.5453);
			hash_f = hash_f - Floor<T>(hash_f);

			if (hash_f < T(0.5))
				add = M_PI3;
		}

		T angle = std::atan2(FY, FX) + M_PI6 - add;
		T coef = T(0.47746482927568600730665129011754); //1.5 / pi
		T angle2 = Floor<T>(angle * coef) / coef + M_PI6 + add; //or subtract T(0.5)
		T x0 = std::cos(angle2);
		T y0 = std::sin(angle2);
		T dist = std::sqrt(Sqr(FX - x0) + Sqr(FY - y0));

		if (m_Inv)
		{
			if ((dist > m_D1Precalc) || (dist < m_D2Precalc))
			{
				if (m_Mode < T(0.5))
				{
					FX = 0;
					FY = 0;
				}
				else
				{
					if (m_Mode < 1.5)
					{
						FX = x0;
						FY = y0;
					}
					else
					{
						T rangle = std::atan2(FY - y0, FX - x0);
						T D;

						if (rand.Frand01<T>() < T(0.5))
							D = m_D1Precalc;
						else
							D = m_D2Precalc;

						FX = x0 + std::cos(rangle) * D;
						FY = y0 + std::sin(rangle) * D;
					}
				}
			}
		}
		else
		{
			if ((dist < m_D1Precalc) && (dist > m_D2Precalc))
			{
				if (m_Mode < T(0.5))
				{
					FX = 0;
					FY = 0;
				}
				else
				{
					if (m_Mode < 1.5)
					{
						FX = x0;
						FY = y0;
					}
					else
					{
						T rangle = std::atan2(FY - y0, FX - x0);
						T D;

						if (rand.Frand01<T>() < T(0.5))
							D = m_D1Precalc;
						else
							D = m_D2Precalc;

						FX = x0 + std::cos(rangle) * D;
						FY = y0 + std::sin(rangle) * D;
					}
				}
			}
		}

		helper.Out.x = (FX + FX_h) * m_Weight;
		helper.Out.y = (FY + FY_h) * m_Weight;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string wd        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mode      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string inv       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d1precalc = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d2precalc = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t yover3 = vIn.y / (real_t)(3.0);\n"
		   << "\t\treal_t x = fma(M_SQRT3_3, vIn.x, -yover3);\n"
		   << "\t\treal_t z = (real_t)(2.0) * yover3;\n"
		   << "\t\treal_t y = -x - z;\n"
		   << "\n"
		   << "\t\treal_t rx = round(x);\n"
		   << "\t\treal_t ry = round(y);\n"
		   << "\t\treal_t rz = round(z);\n"
		   << "\n"
		   << "\t\treal_t x_diff = fabs(rx - x);\n"
		   << "\t\treal_t y_diff = fabs(ry - y);\n"
		   << "\t\treal_t z_diff = fabs(rz - z);\n"
		   << "\n"
		   << "\t\tif ((x_diff > y_diff) && (x_diff > z_diff))\n"
		   << "\t\t	rx = -ry - rz;\n"
		   << "\t\telse if (y_diff > z_diff)\n"
		   << "\t\t	ry = -rx - rz;\n"
		   << "\t\telse\n"
		   << "\t\t	rz = -rx - ry;\n"
		   << "\n"
		   << "\t\treal_t FX_h = fma(M_SQRT3, rx, M_SQRT3_2 * rz);\n"
		   << "\t\treal_t FY_h = (real_t)(1.5) * rz;\n"
		   << "\t\treal_t FX = vIn.x - FX_h;\n"
		   << "\t\treal_t FY = vIn.y - FY_h;\n"
		   << "\t\treal_t add = 0;\n"
		   << "\n"
		   << "\t\tif (" << seed << " == 1)\n"
		   << "\t\t{\n"
		   << "\t\t	if (!((int)rx & 1) && !((int)rz & 1)) \n"
		   << "\t\t		add = MPI3;\n"
		   << "\t\t}\n"
		   << "\t\telse if (" << seed << " >= 2)\n"
		   << "\t\t{\n"
		   << "\t\t	real_t hash_f = sin(FX_h * (real_t)(12.9898) + FY_h * (real_t)(78.233) + " << seed << ") * (real_t)(43758.5453);\n"
		   << "\t\t	hash_f = hash_f - floor(hash_f);\n"
		   << "\n"
		   << "\t\t	if (hash_f < (real_t)(0.5))\n"
		   << "\t\t		add = MPI3;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t angle = atan2(FY, FX) + MPI6 - add;\n"
		   << "\t\treal_t coef = (real_t)(0.47746482927568600730665129011754);\n"
		   << "\t\treal_t angle2 = floor(angle * coef) / coef + MPI6 + add;\n"
		   << "\t\treal_t x0 = cos(angle2);\n"
		   << "\t\treal_t y0 = sin(angle2);\n"
		   << "\t\treal_t dist = sqrt(Sqr(FX - x0) + Sqr(FY - y0));\n"
		   << "\n"
		   << "\t\tif (" << inv << " != (real_t)(0.0))\n"
		   << "\t\t{\n"
		   << "\t\t	if ((dist > " << d1precalc << ") || (dist < " << d2precalc << "))\n"
		   << "\t\t	{\n"
		   << "\t\t		if (" << mode << " < (real_t)(0.5))\n"
		   << "\t\t		{\n"
		   << "\t\t			FX = 0.0;\n"
		   << "\t\t			FY = 0.0;\n"
		   << "\t\t		}\n"
		   << "\t\t		else if (" << mode << " < 1.5)\n"
		   << "\t\t		{\n"
		   << "\t\t			FX = x0;\n"
		   << "\t\t			FY = y0;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			real_t rangle = atan2(FY - y0, FX - x0);\n"
		   << "\t\t			real_t D;\n"
		   << "\n"
		   << "\t\t			if (MwcNext01(mwc) < (real_t)(0.5))\n"
		   << "\t\t				D = " << d1precalc << ";\n"
		   << "\t\t			else\n"
		   << "\t\t				D = " << d2precalc << ";\n"
		   << "\n"
		   << "\t\t			FX = fma(cos(rangle), D, x0);\n"
		   << "\t\t			FY = fma(sin(rangle), D, y0);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if ((dist < " << d1precalc << ") && (dist > " << d2precalc << "))\n"
		   << "\t\t {\n"
		   << "\t\t		if (" << mode << " < (real_t)(0.5))\n"
		   << "\t\t		{\n"
		   << "\t\t			FX = 0.0;\n"
		   << "\t\t			FY = 0.0;\n"
		   << "\t\t		}\n"
		   << "\t\t		else if (" << mode << " < 1.5)\n"
		   << "\t\t		{\n"
		   << "\t\t			FX = x0;\n"
		   << "\t\t			FY = y0;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			real_t rangle = atan2(FY - y0, FX - x0);\n"
		   << "\t\t			real_t D;\n"
		   << "\n"
		   << "\t\t			if (MwcNext01(mwc) < (real_t)(0.5))\n"
		   << "\t\t				D = " << d1precalc << ";\n"
		   << "\t\t			else\n"
		   << "\t\t				D = " << d2precalc << ";\n"
		   << "\n"
		   << "\t\t			FX = fma(cos(rangle), D, x0);\n"
		   << "\t\t			FY = fma(sin(rangle), D, y0);\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = (FX + FX_h) * " << weight << ";\n"
		   << "\t\tvOut.y = (FY + FY_h) * " << weight << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_D1Precalc = T(0.5) + m_Wd;
		m_D2Precalc = T(0.5) - m_Wd;
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Sqr" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Wd,              prefix + "Truchet_hex_crop_wd", T(0.2), eParamType::REAL, 0, 5));
		m_Params.push_back(ParamWithName<T>(&m_Mode,            prefix + "Truchet_hex_crop_mode", 0, eParamType::INTEGER, 0, 2));
		m_Params.push_back(ParamWithName<T>(&m_Inv,             prefix + "Truchet_hex_crop_inv", 0, eParamType::INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Seed,            prefix + "Truchet_hex_crop_seed", 0, eParamType::INTEGER));
		m_Params.push_back(ParamWithName<T>(true, &m_D1Precalc, prefix + "Truchet_hex_crop_d1_precalc"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_D2Precalc, prefix + "Truchet_hex_crop_d2_precalc"));
	}

private:
	T m_Wd;
	T m_Mode;
	T m_Inv;
	T m_Seed;
	T m_D1Precalc;//Precalc.
	T m_D2Precalc;
};

/// <summary>
/// waves2_radial.
/// By tatasz.
/// http://fav.me/dapecsh
/// </summary>
template <typename T>
class Waves2RadialVariation : public ParametricVariation<T>
{
public:
	Waves2RadialVariation(T weight = 1.0) : ParametricVariation<T>("waves2_radial", eVariationId::VAR_WAVES2_RADIAL, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(Waves2RadialVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x0 = helper.In.x;
		T y0 = helper.In.y;
		T dist = helper.m_PrecalcSqrtSumSquares;
		T factor = (dist < m_Distance) ? (dist - m_Null) / Zeps(m_Distance - m_Null) : T(1);
		factor = (dist < m_Null) ? T(0) : factor;
		helper.Out.x = m_Weight * (x0 + factor * std::sin(y0 * m_Freqx) * m_Scalex);
		helper.Out.y = m_Weight * (y0 + factor * std::sin(x0 * m_Freqy) * m_Scaley);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string freqX    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleX   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqY    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaleY   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string nullVar  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string distance = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t x0 = vIn.x;\n"
		   << "\t\treal_t y0 = vIn.y;\n"
		   << "\n"
		   << "\t\treal_t dist = precalcSqrtSumSquares;\n"
		   << "\t\treal_t factor = (dist < " << distance << ") ? (dist - " << nullVar << ") / Zeps(" << distance << "-" << nullVar << ") : (real_t)(1.0);\n"
		   << "\t\tfactor = (dist < " << nullVar << ") ? (real_t) 0.0 : factor;\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(factor * " << scaleX << ", sin(y0 * " << freqX << "), x0);\n"
		   << "\t\tvOut.y = " << weight << " * fma(factor * " << scaleY << ", sin(x0 * " << freqY << "), y0);\n"
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
		m_Params.push_back(ParamWithName<T>(&m_Freqx, prefix + "waves2_radial_freqx", 7));
		m_Params.push_back(ParamWithName<T>(&m_Scalex, prefix + "waves2_radial_scalex", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_Freqy, prefix + "waves2_radial_freqy", 13));
		m_Params.push_back(ParamWithName<T>(&m_Scaley, prefix + "waves2_radial_scaley", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_Null, prefix + "waves2_radial_null", 2));
		m_Params.push_back(ParamWithName<T>(&m_Distance, prefix + "waves2_radial_distance", 10));
	}

private:
	T m_Freqx;
	T m_Scalex;
	T m_Freqy;
	T m_Scaley;
	T m_Null;
	T m_Distance;
};

/// <summary>
/// panorama1.
/// </summary>
template <typename T>
class Panorama1Variation : public Variation<T>
{
public:
	Panorama1Variation(T weight = 1.0) : Variation<T>("panorama1", eVariationId::VAR_PANORAMA1, weight, true)
	{
	}

	VARCOPY(Panorama1Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T aux = 1 / std::sqrt(helper.m_PrecalcSumSquares + 1);
		T x1 = helper.m_TransX * aux;
		T y1 = helper.m_TransY * aux;
		aux = std::sqrt(x1 * x1 + SQR(y1));
		helper.Out.x = m_Weight * std::atan2(x1, y1) * T(M_1_PI);
		helper.Out.y = m_Weight * (aux - T(0.5));
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		string weight = WeightDefineString();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		ss << "\t{\n"
		   << "\t\treal_t aux = 1.0 / sqrt(precalcSumSquares + 1);\n"
		   << "\t\treal_t x1 = transX * aux;\n"
		   << "\t\treal_t y1 = transY * aux;\n"
		   << "\t\taux = sqrt(fma(x1, x1, SQR(y1)));\n"
		   << "\t\tvOut.x = " << weight << " * atan2(x1, y1) * M1PI;\n"
		   << "\t\tvOut.y = " << weight << " * (aux - 0.5);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// panorama2.
/// </summary>
template <typename T>
class Panorama2Variation : public Variation<T>
{
public:
	Panorama2Variation(T weight = 1.0) : Variation<T>("panorama2", eVariationId::VAR_PANORAMA2, weight, true, true)
	{
	}

	VARCOPY(Panorama2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T aux = 1 / (helper.m_PrecalcSqrtSumSquares + 1);
		T x1 = helper.m_TransX * aux;
		T y1 = helper.m_TransY * aux;
		aux = std::sqrt(x1 * x1 + SQR(y1));
		helper.Out.x = m_Weight * std::atan2(x1, y1) * T(M_1_PI);
		helper.Out.y = m_Weight * (aux - T(0.5));
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		string weight = WeightDefineString();
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		ss << "\t{\n"
		   << "\t\treal_t aux = 1.0 / (precalcSqrtSumSquares + 1);\n"
		   << "\t\treal_t x1 = transX * aux;\n"
		   << "\t\treal_t y1 = transY * aux;\n"
		   << "\t\taux = sqrt(fma(x1, x1, SQR(y1)));\n"
		   << "\t\tvOut.x = " << weight << " * atan2(x1, y1) * M1PI;\n"
		   << "\t\tvOut.y = " << weight << " * (aux - 0.5);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// helicoid.
/// </summary>
template <typename T>
class HelicoidVariation : public ParametricVariation<T>
{
public:
	HelicoidVariation(T weight = 1.0) : ParametricVariation<T>("helicoid", eVariationId::VAR_HELICOID, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(HelicoidVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T temp = helper.In.z * m_Freq2Pi + helper.m_PrecalcAtanyx;
		T weightXdist = m_Weight * helper.m_PrecalcSqrtSumSquares;
		helper.Out.x = weightXdist * std::cos(temp);
		helper.Out.y = weightXdist * std::sin(temp);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string freq    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freq2pi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t temp = fma(vIn.z, " << freq2pi << ", precalcAtanyx);\n"
		   << "\t\treal_t weightXdist = " << weight << " * precalcSqrtSumSquares;\n"
		   << "\t\tvOut.x = weightXdist * cos(temp);\n"
		   << "\t\tvOut.y = weightXdist * sin(temp);\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Freq2Pi = m_Freq * M_2PI;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Freq, prefix + "helicoid_frequency", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Freq2Pi, prefix + "helicoid_frequency_2pi"));//Precalc.
	}

private:
	T m_Freq;
	T m_Freq2Pi;//Precalc.
};

/// <summary>
/// helix.
/// </summary>
template <typename T>
class HelixVariation : public ParametricVariation<T>
{
public:
	HelixVariation(T weight = 1.0) : ParametricVariation<T>("helix", eVariationId::VAR_HELIX, weight)
	{
		Init();
	}

	PARVARCOPY(HelixVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T temp = helper.In.z * m_Freq2Pi;
		helper.Out.x = m_Weight * (helper.In.x + std::cos(temp) * m_Width);
		helper.Out.y = m_Weight * (helper.In.y + std::sin(temp) * m_Width);
		helper.Out.z = m_Weight * helper.In.z;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string freq    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string width   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freq2pi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t temp = vIn.z  * " << freq2pi << ";\n"
		   << "\t\tvOut.x = " << weight << " * (fma(cos(temp), " << width << ", vIn.x));\n"
		   << "\t\tvOut.y = " << weight << " * (fma(sin(temp), " << width << ", vIn.y));\n"
		   << "\t\tvOut.z = " << weight << " * vIn.z ;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Freq2Pi = m_Freq * M_2PI;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Freq, prefix + "helix_frequency", 1));
		m_Params.push_back(ParamWithName<T>(&m_Width, prefix + "helix_width", T(0.5)));
		m_Params.push_back(ParamWithName<T>(true, &m_Freq2Pi, prefix + "helix_frequency_2pi"));//Precalc.
	}

private:
	T m_Freq;
	T m_Width;
	T m_Freq2Pi;//Precalc.
};

/// <summary>
/// sphereblur.
/// </summary>
template <typename T>
class SphereblurVariation : public ParametricVariation<T>
{
public:
	SphereblurVariation(T weight = 1.0) : ParametricVariation<T>("sphereblur", eVariationId::VAR_SPHEREBLUR, weight)
	{
		Init();
	}

	PARVARCOPY(SphereblurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T ang = rand.Frand01<T>() * M_2PI;
		T angz = std::acos(rand.Frand01<T>() * 2 - 1);
		T r = m_Weight * std::exp(std::log(std::acos((m_Power == T(1.0) ? rand.Frand01<T>() : std::exp(std::log(rand.Frand01<T>()) * m_Power)) * 2 - 1) / T(M_PI)) / T(1.5));
		T s = std::sin(ang);
		T c = std::cos(ang);
		T sz = std::sin(angz);
		T cz = std::cos(angz);
		helper.Out.x = r * c * sz;
		helper.Out.y = r * s * sz;
		helper.Out.z = r * cz;
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
		   << "\t\treal_t angz = acos(MwcNext01(mwc) * 2 - 1);\n"
		   << "\t\treal_t r = " << weight << " * exp(log(acos((" << power << " == 1.0 ? MwcNext01(mwc) : exp(log(MwcNext01(mwc)) * " << power << ")) * 2 - 1) / MPI) / 1.5);\n"
		   << "\t\treal_t s = sin(ang);\n"
		   << "\t\treal_t c = cos(ang);\n"
		   << "\t\treal_t sz = sin(angz);\n"
		   << "\t\treal_t cz = cos(angz);\n"
		   << "\t\tvOut.x = r * c * sz;\n"
		   << "\t\tvOut.y = r * s * sz;\n"
		   << "\t\tvOut.z = r * cz;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		if (m_Power < 0)
			m_Power = 0;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "sphereblur_power", 1, eParamType::REAL, 0));
	}

private:
	T m_Power;
};

/// <summary>
/// cpow3.
/// </summary>
template <typename T>
class Cpow3Variation : public ParametricVariation<T>
{
public:
	Cpow3Variation(T weight = 1.0) : ParametricVariation<T>("cpow3", eVariationId::VAR_CPOW3, weight, true, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(Cpow3Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T a = helper.m_PrecalcAtanyx;

		if (a < 0) a += M_2PI;

		if (std::cos(a / 2) < rand.Frand11<T>())
			a -= M_2PI;

		a += (rand.RandBit() ? M_2PI : -M_2PI) * std::round(std::log(rand.Frand01<T>()) * m_Coeff);
		T lnr2 = std::log(helper.m_PrecalcSumSquares);
		T r = m_Weight * std::exp(m_HalfC * lnr2 - m_PrecalcD * a);
		T temp = m_PrecalcC * a + m_HalfD * lnr2 + m_Ang * rand.Crand();
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
		string d = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string divisor = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string spread = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string precalcc = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfc = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string precalcd = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfd = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string ang = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string coeff = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t a = precalcAtanyx;\n"
		   << "\n"
		   << "\t\tif (a < 0) a += M_2PI;\n"
		   << "\n"
		   << "\t\tif (cos(a / 2) < MwcNextNeg1Pos1(mwc))\n"
		   << "\t\ta -= M_2PI;\n"
		   << "\n"
		   << "\t\ta += ((MwcNext(mwc) & 1) ? M_2PI : -M_2PI) * round(log(MwcNext01(mwc)) * " << coeff << ");\n"
		   << "\t\treal_t lnr2 = log(precalcSumSquares);\n"
		   << "\t\treal_t r = " << weight << " * exp(fma(" << halfc << ", lnr2, -(" << precalcd << " * a)));\n"
		   << "\t\treal_t temp = fma(" << precalcc << ", a, fma(" << halfd << ", lnr2, " << ang << " * MwcNextCrand(mwc)));\n"
		   << "\t\tvOut.x = r * cos(temp);\n"
		   << "\t\tvOut.y = r * sin(temp);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Ang = M_2PI / Zeps(m_Divisor);
		T a = std::atan2((m_D < 0 ? -std::log(-m_D) : std::log(m_D)) * m_R, M_2PI);
		m_PrecalcC = std::cos(a) * m_R * std::cos(a) / Zeps(m_Divisor);
		m_PrecalcD = std::cos(a) * m_R * std::sin(a) / Zeps(m_Divisor);
		m_HalfC = m_PrecalcC / 2;
		m_HalfD = m_PrecalcD / 2;
		m_Coeff = m_PrecalcD == 0 ? 0 : T(-0.095) * m_Spread / m_PrecalcD;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_R, prefix + "cpow3_r", 1));
		m_Params.push_back(ParamWithName<T>(&m_D, prefix + "cpow3_d", 1, eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Divisor, prefix + "cpow3_divisor", 1, eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Spread, prefix + "cpow3_spread", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_PrecalcC, prefix + "cpow3_precalc_c"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_HalfC, prefix + "cpow3_half_c"));
		m_Params.push_back(ParamWithName<T>(true, &m_PrecalcD, prefix + "cpow3_precalc_d"));
		m_Params.push_back(ParamWithName<T>(true, &m_HalfD, prefix + "cpow3_half_d"));
		m_Params.push_back(ParamWithName<T>(true, &m_Ang, prefix + "cpow3_ang"));
		m_Params.push_back(ParamWithName<T>(true, &m_Coeff, prefix + "cpow3_coeff"));
	}

private:
	T m_R;
	T m_D;
	T m_Divisor;
	T m_Spread;
	T m_PrecalcC;//Precalc.
	T m_HalfC;
	T m_PrecalcD;
	T m_HalfD;
	T m_Ang;
	T m_Coeff;
};

/// <summary>
/// concentric.
/// </summary>
template <typename T>
class ConcentricVariation : public ParametricVariation<T>
{
public:
	ConcentricVariation(T weight = 1.0) : ParametricVariation<T>("concentric", eVariationId::VAR_CONCENTRIC, weight)
	{
		Init();
	}

	PARVARCOPY(ConcentricVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T lvl = T(Floor<T>(rand.Frand01<T>() * m_Density) / Zeps(m_Density)); //random level. should care if density=0 but meh, works fine
		T randa = rand.Frand01<T>() * M_2PI; //random angle
		T randr = lvl * m_Radius; //calc radius of rings

		if (m_Rblur != 0)
			randr += (std::sqrt(rand.Frand01<T>()) * 2 - 1) * m_Rblur; //blur ring. sqrt is expensive but gives nice effect

		if (m_VarType == eVariationType::VARTYPE_REG)
		{
			outPoint.m_X = 0;//This variation intentionally assigns instead of summing.
			outPoint.m_Y = 0;
			outPoint.m_Z = 0;
		}

		helper.Out.x = randr * std::cos(randa) * m_Weight; //polar to cartesian coords, origin is origin
		helper.Out.y = randr * std::sin(randa) * m_Weight;
		T zb = 0;

		if (m_Zblur != 0)
			zb = (rand.Frand01<T>() * 2 - 1) * m_Zblur;

		helper.Out.z = (-lvl + zb) * m_Weight;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string radius = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string density = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rblur = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string zblur = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t lvl = floor(MwcNext01(mwc) * " << density << ") / Zeps(" << density << ");\n"
		   << "\t\treal_t randa = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t randr = lvl * " << radius << ";\n"
		   << "\n"
		   << "\t\tif (" << rblur << " != 0)\n"
		   << "\t\t\trandr += fma(sqrt(MwcNext01(mwc)), (real_t)(2.0), (real_t)(-1.0)) * " << rblur << ";\n";

		if (m_VarType == eVariationType::VARTYPE_REG)
		{
			ss << "\t\toutPoint->m_X = 0;\n"
			   << "\t\toutPoint->m_Y = 0;\n"
			   << "\t\toutPoint->m_Z = 0;\n";
		}

		ss << "\t\tvOut.x = randr * cos(randa) * " << weight << ";\n"
		   << "\t\tvOut.y = randr * sin(randa) * " << weight << ";\n"
		   << "\t\treal_t zb = 0;\n"
		   << "\n"
		   << "\t\tif (" << zblur << " != 0)\n"
		   << "\t\t\tzb = fma(sqrt(MwcNext01(mwc)), (real_t)(2.0), (real_t)(-1.0)) * " << zblur << ";\n"
		   << "\n"
		   << "\t\tvOut.z = (-lvl + zb) * " << weight << ";\n"
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
		m_Params.push_back(ParamWithName<T>(&m_Radius, prefix + "concentric_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_Density, prefix + "concentric_density", 10, eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Rblur, prefix + "concentric_R_blur"));
		m_Params.push_back(ParamWithName<T>(&m_Zblur, prefix + "concentric_Z_blur"));
	}

private:
	T m_Radius;
	T m_Density;
	T m_Rblur;
	T m_Zblur;
};

/// <summary>
/// hypercrop.
/// </summary>
template <typename T>
class HypercropVariation : public ParametricVariation<T>
{
public:
	HypercropVariation(T weight = 1.0) : ParametricVariation<T>("hypercrop", eVariationId::VAR_HYPERCROP, weight, false, false, false, false, true)
	{
		Init();
	}

	PARVARCOPY(HypercropVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T fx = helper.In.x;
		T fy = helper.In.y;
		T fz = helper.In.z;
		T angle = Floor<T>(helper.m_PrecalcAtanyx * m_Coeff) / m_Coeff + m_A0;
		T x0 = std::cos(angle) * m_Len;
		T y0 = std::sin(angle) * m_Len;

		if (std::sqrt(Sqr(helper.In.x - x0) + Sqr(helper.In.y - y0)) < m_D)
		{
			if (m_Zero > 1.5)
			{
				fx = x0;
				fy = y0;
				fz = 0;
			}
			else
			{
				if (m_Zero > 0.5)
				{
					fx = 0;
					fy = 0;
					fz = 0;
				}
				else
				{
					T rangle = std::atan2(helper.In.y - y0, helper.In.x - x0);
					fx = x0 + std::cos(rangle) * m_D;
					fy = y0 + std::sin(rangle) * m_D;
					fz = 0;
				}
			}
		}

		helper.Out.x = fx * m_Weight;
		helper.Out.y = fy * m_Weight;
		helper.Out.z = fz * m_Weight;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string n     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rad   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string zero  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string coeff = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a0    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string len   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t fx = vIn.x;\n"
		   << "\t\treal_t fy = vIn.y;\n"
		   << "\t\treal_t fz = vIn.z;\n"
		   << "\t\treal_t angle = floor(precalcAtanyx * " << coeff << ") / " << coeff << " + " << a0 << ";\n"
		   << "\t\treal_t x0 = cos(angle) * " << len << ";\n"
		   << "\t\treal_t y0 = sin(angle) * " << len << ";\n"
		   << "\t\treal_t xmx = vIn.x - x0;\n"
		   << "\t\treal_t ymy = vIn.y - y0;\n"
		   << "\n"
		   << "\t\tif (sqrt(fma(xmx, xmx, SQR(ymy))) < " << d << ")\n"
		   << "\t\t{\n"
		   << "\t\t	if (" << zero << " > 1.5)\n"
		   << "\t\t	{\n"
		   << "\t\t		fx = x0;\n"
		   << "\t\t		fy = y0;\n"
		   << "\t\t		fz = 0;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		if (" << zero << " > 0.5)\n"
		   << "\t\t		{\n"
		   << "\t\t			fx = 0;\n"
		   << "\t\t			fy = 0;\n"
		   << "\t\t			fz = 0;\n"
		   << "\t\t		}\n"
		   << "\t\t		else\n"
		   << "\t\t		{\n"
		   << "\t\t			real_t rangle = atan2(vIn.y - y0, vIn.x - x0);\n"
		   << "\t\t			fx = fma(cos(rangle), " << d << ", x0);\n"
		   << "\t\t			fy = fma(sin(rangle), " << d << ", y0);\n"
		   << "\t\t			fz = 0;\n"
		   << "\t\t		}\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = fx * " << weight << ";\n"
		   << "\t\tvOut.y = fy * " << weight << ";\n"
		   << "\t\tvOut.z = fz * " << weight << ";\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_N = Zeps(m_N);
		m_Coeff = Zeps<T>(m_N * T(0.5) / T(M_PI));
		m_A0 = T(M_PI) / m_N;
		m_Len = 1 / Zeps(std::cos(m_A0));
		m_D = m_Rad * std::sin(m_A0) * m_Len;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_N,           prefix + "hypercrop_n", 4));
		m_Params.push_back(ParamWithName<T>(&m_Rad,         prefix + "hypercrop_rad", 1));
		m_Params.push_back(ParamWithName<T>(&m_Zero,        prefix + "hypercrop_zero"));
		m_Params.push_back(ParamWithName<T>(true, &m_Coeff, prefix + "hypercrop_coeff"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_A0,    prefix + "hypercrop_a0"));
		m_Params.push_back(ParamWithName<T>(true, &m_Len,   prefix + "hypercrop_len"));
		m_Params.push_back(ParamWithName<T>(true, &m_D,     prefix + "hypercrop_d"));
	}

private:
	T m_N;
	T m_Rad;
	T m_Zero;
	T m_Coeff;//Precalc.
	T m_A0;
	T m_Len;
	T m_D;
};

/// <summary>
/// hypershift.
/// </summary>
template <typename T>
class HypershiftVariation : public ParametricVariation<T>
{
public:
	HypershiftVariation(T weight = 1.0) : ParametricVariation<T>("hypershift", eVariationId::VAR_HYPERSHIFT, weight, true)
	{
		Init();
	}

	PARVARCOPY(HypershiftVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T rad = 1 / Zeps(helper.m_PrecalcSumSquares);
		T x = rad * helper.In.x + m_Shift;
		T y = rad * helper.In.y;
		rad = m_Weight * m_Scale / Zeps(SQR(x) + SQR(y));

		if (Compat::m_Compat && m_VarType == eVariationType::VARTYPE_REG)
			outPoint.m_X = outPoint.m_Y = outPoint.m_Z = 0;//This variation assigns, instead of summing, so order will matter.

		helper.Out.x = rad * x + m_Shift;
		helper.Out.y = rad * y;
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
		string scale = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t rad = 1 / Zeps(precalcSumSquares);\n"
		   << "\t\treal_t x = fma(rad, vIn.x, " << shift << ");\n"
		   << "\t\treal_t y = rad * vIn.y;\n"
		   << "\t\trad = " << weight << " * " << scale << " / Zeps(fma(x, x, SQR(y)));\n";

		if (Compat::m_Compat && m_VarType == eVariationType::VARTYPE_REG)
			ss << "\t\toutPoint->m_X = outPoint->m_Y = outPoint->m_Z = 0;\n";

		ss << "\t\tvOut.x = fma(rad, x, " << shift << ");\n"
		   << "\t\tvOut.y = rad * y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Scale = 1 - SQR(m_Shift);
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
		m_Params.push_back(ParamWithName<T>(&m_Shift,       prefix + "hypershift_shift", T(0.01)));
		m_Params.push_back(ParamWithName<T>(true, &m_Scale, prefix + "hypershift_scale"));//Precalc.
	}

private:
	T m_Shift;
	T m_Scale;//Precalc.
};

/// <summary>
/// hypershift2.
/// </summary>
template <typename T>
class Hypershift2Variation : public ParametricVariation<T>
{
public:
	Hypershift2Variation(T weight = 1.0) : ParametricVariation<T>("hypershift2", eVariationId::VAR_HYPERSHIFT2, weight)
	{
		Init();
	}

	PARVARCOPY(Hypershift2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T fx = helper.In.x * m_Scale2;
		T fy = helper.In.y * m_Scale2;
		T rad = 1 / Zeps(fx * fx + fy * fy);
		T x = rad * fx + m_Shift;
		T y = rad * fy;
		rad = m_Weight * m_Scale / Zeps(x * x + y * y);
		T angle = (Floor<T>(rand.Frand01<T>() * m_P) * 2 + 1) * T(M_PI) / m_P;
		T X = rad * x + m_Shift;
		T Y = rad * y;
		T cosa = std::cos(angle);
		T sina = std::sin(angle);

		if (Compat::m_Compat && m_VarType == eVariationType::VARTYPE_REG)
			outPoint.m_X = outPoint.m_Y = outPoint.m_Z = 0;//This variation assigns, instead of summing, so order will matter.

		helper.Out.x = cosa * X - sina * Y;
		helper.Out.y = sina * X + cosa * Y;
		helper.Out.z = helper.In.z * rad;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string p      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string q      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string shift  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t scale2 = " << scale2 << ";\n"
		   << "\t\treal_t fx = vIn.x * scale2;\n"
		   << "\t\treal_t fy = vIn.y * scale2;\n"
		   << "\t\treal_t rad = 1 / Zeps(fma(fx, fx, SQR(fy)));\n"
		   << "\t\treal_t x = fma(rad, fx, " << shift << ");\n"
		   << "\t\treal_t y = rad * fy;\n"
		   << "\t\trad = " << weight << " * " << scale << " / Zeps(fma(x, x, SQR(y)));\n"
		   << "\t\treal_t angle = (floor(MwcNext01(mwc) * " << p << ") * 2 + 1) * MPI / " << p << ";\n"
		   << "\t\treal_t X = fma(rad, x, " << shift << ");\n"
		   << "\t\treal_t Y = rad * y;\n"
		   << "\t\treal_t cosa = cos(angle);\n"
		   << "\t\treal_t sina = sin(angle);\n";

		if (Compat::m_Compat && m_VarType == eVariationType::VARTYPE_REG)
			ss << "\t\toutPoint->m_X = outPoint->m_Y = outPoint->m_Z = 0;\n";

		ss << "\t\tvOut.x = fma(cosa, X, -(sina * Y));\n"
		   << "\t\tvOut.y = fma(sina, X, cosa * Y);\n"
		   << "\t\tvOut.z = vIn.z * rad;\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T pq = T(M_PI) / m_Q;
		T pp = T(M_PI) / m_P;
		T spq = std::sin(pq);
		T spp = std::sin(pp);
		m_Shift = std::sin(T(M_PI) * T(0.5) - pq - pp);
		m_Shift = m_Shift / std::sqrt(1 - Sqr(spq) - Sqr(spp));
		m_Scale2 = 1 / std::sqrt(Sqr(sin(T(M_PI) / 2 + pp)) / Sqr(spq) - 1);
		m_Scale2 = m_Scale2 * (std::sin(T(M_PI) / 2 + pp) / spq - 1);
		m_Scale = 1 - SQR(m_Shift);
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
		m_Params.push_back(ParamWithName<T>(&m_P, prefix + "hypershift2_p", 3, eParamType::REAL, 2));
		m_Params.push_back(ParamWithName<T>(&m_Q, prefix + "hypershift2_q", 7, eParamType::REAL, 2));
		m_Params.push_back(ParamWithName<T>(true, &m_Shift,  prefix + "hypershift2_shift"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Scale,  prefix + "hypershift2_scale"));
		m_Params.push_back(ParamWithName<T>(true, &m_Scale2, prefix + "hypershift2_scale2"));
	}

private:
	T m_P;
	T m_Q;
	T m_Shift;//Precalc.
	T m_Scale;
	T m_Scale2;
};

/// <summary>
/// lens.
/// By tatasz.
/// </summary>
template <typename T>
class LensVariation : public Variation<T>
{
public:
	LensVariation(T weight = 1.0) : Variation<T>("lens", eVariationId::VAR_LENS, weight)
	{
	}

	VARCOPY(LensVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = T(0.5) * (SQR(helper.In.x) - SQR(helper.In.y)) * m_Weight;
		helper.Out.y = helper.In.x * helper.In.y * m_Weight;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss;
		string weight = WeightDefineString();
		ss << "\t{\n"
		   << "\t\tvOut.x = (real_t)0.5 * fma(vIn.x, vIn.x, -SQR(vIn.y)) * " << weight << ";\n"
		   << "\t\tvOut.y = vIn.x * vIn.y * " << weight << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
};

/// <summary>
/// projective.
/// By eralex61.
/// </summary>
template <typename T>
class ProjectiveVariation : public ParametricVariation<T>
{
public:
	ProjectiveVariation(T weight = 1.0) : ParametricVariation<T>("projective", eVariationId::VAR_PROJECTIVE, weight)
	{
		Init();
	}

	PARVARCOPY(ProjectiveVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T s = m_Weight / Zeps(m_A * helper.In.x + m_B * helper.In.y + m_C);
		helper.Out.x = (m_A1 * helper.In.x + m_B1 * helper.In.y + m_C1) * s;
		helper.Out.y = (m_A2 * helper.In.x + m_B2 * helper.In.y + m_C2) * s;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string a  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string b2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t s = " << weight << " / Zeps(fma(" << a << ", vIn.x, fma(" << b << ", vIn.y, " << c << ")));\n"
		   << "\t\tvOut.x = fma(" << a1 << ", vIn.x, fma(" << b1 << ", vIn.y, " << c1 << ")) * s;\n"
		   << "\t\tvOut.y = fma(" << a2 << ", vIn.x, fma(" << b2 << ", vIn.y, " << c2 << ")) * s;\n"
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
		m_Params.push_back(ParamWithName<T>(&m_A, prefix  + "projective_A"));
		m_Params.push_back(ParamWithName<T>(&m_B, prefix  + "projective_B"));
		m_Params.push_back(ParamWithName<T>(&m_C, prefix  + "projective_C", 1));
		m_Params.push_back(ParamWithName<T>(&m_A1, prefix + "projective_A1", 1));
		m_Params.push_back(ParamWithName<T>(&m_B1, prefix + "projective_B1"));
		m_Params.push_back(ParamWithName<T>(&m_C1, prefix + "projective_C1"));
		m_Params.push_back(ParamWithName<T>(&m_A2, prefix + "projective_A2"));
		m_Params.push_back(ParamWithName<T>(&m_B2, prefix + "projective_B2", 1));
		m_Params.push_back(ParamWithName<T>(&m_C2, prefix + "projective_C2"));
	}

private:
	T m_A;
	T m_B;
	T m_C;
	T m_A1;
	T m_B1;
	T m_C1;
	T m_A2;
	T m_B2;
	T m_C2;
};

/// <summary>
/// depth_blur.
/// By tatasz.
/// </summary>
template <typename T>
class DepthBlurVariation : public ParametricVariation<T>
{
public:
	DepthBlurVariation(T weight = 1.0) : ParametricVariation<T>("depth_blur", eVariationId::VAR_DEPTH_BLUR, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(DepthBlurVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T bx = 0;
		T by = 0;
		T rad = helper.m_PrecalcSqrtSumSquares;

		if (rad > m_Radius)
		{
			T f = rand.Frand01<T>() * m_Power2;
			T int_angle = std::trunc(f);
			f = f - int_angle;
			T x = f * m_Length;
			T z = std::sqrt(1 + SQR(x) - 2 * x * m_CosAlpha);
			int iangle = int(int_angle);

			if (!(iangle & 1))
				int_angle = m_2piOverPower * (iangle / 2) + std::asin(m_SinAlpha * x / Zeps(z));
			else
				int_angle = m_2piOverPower * (iangle / 2) - std::asin(m_SinAlpha * x / Zeps(z));

			z *= std::sqrt(rand.Frand01<T>());
			by = std::sin(int_angle - T(M_PI_2));
			bx = std::cos(int_angle - T(M_PI_2));
			T aux = z * m_BlurOver10 * (rad - m_Radius);
			by = aux * by;
			bx = aux * bx;
		}

		helper.Out.x = m_Weight * (helper.In.x + bx);
		helper.Out.y = m_Weight * (helper.In.y + by);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string range          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blur           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string alpha          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string length         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurover10     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power2         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string twopioverpower = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sinalpha       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosalpha       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t bx = 0;\n"
		   << "\t\treal_t by = 0;\n"
		   << "\t\treal_t rad = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tif (rad > " << radius << ")\n"
		   << "\t\t{\n"
		   << "\t\t	real_t f = MwcNext01(mwc) * " << power2 << ";\n"
		   << "\t\t	real_t int_angle = trunc(f);\n"
		   << "\t\t	f = f - int_angle;\n"
		   << "\t\t	real_t x = f * " << length << ";\n"
		   << "\t\t	real_t z = sqrt(1 + SQR(x) - 2 * x * " << cosalpha << ");\n"
		   << "\t\t	int iangle = (int)int_angle;\n"
		   << "\n"
		   << "\t\t	if (!(iangle & 1))\n"
		   << "\t\t		int_angle = " << twopioverpower << " * (iangle / 2) + asin(" << sinalpha << " * x / Zeps(z));\n"
		   << "\t\t	else\n"
		   << "\t\t		int_angle = " << twopioverpower << " * (iangle / 2) - asin(" << sinalpha << " * x / Zeps(z));\n"
		   << "\n"
		   << "\t\t	z *= sqrt(MwcNext01(mwc));\n"
		   << "\t\t	by = sin(int_angle - MPI2);\n"
		   << "\t\t	bx = cos(int_angle - MPI2);\n"
		   << "\t\t	real_t aux = z * " << blurover10 << " * (rad - " << radius << ");\n"
		   << "\t\t	by = aux * by;\n"
		   << "\t\t	bx = aux * bx;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (vIn.x + bx);\n"
		   << "\t\tvOut.y = " << weight << " * (vIn.y + by);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T alpha = T(M_PI) / Zeps(m_Power);
		m_Length = T(std::sqrt(1 + SQR(m_Range) - 2 * m_Range * std::cos(alpha)));
		m_Alpha = std::asin(std::sin(alpha) * m_Range / Zeps(m_Length));
		m_BlurOver10 = m_Blur / 10;
		m_Power2 = m_Power * 2;
		m_2piOverPower = M_2PI / Zeps(m_Power);
		m_SinAlpha = std::sin(m_Alpha);
		m_CosAlpha = std::cos(m_Alpha);
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
		m_Params.push_back(ParamWithName<T>(&m_Power,              prefix  + "depth_blur_power", 5));
		m_Params.push_back(ParamWithName<T>(&m_Range,              prefix  + "depth_blur_range", T(0.401623)));
		m_Params.push_back(ParamWithName<T>(&m_Blur,               prefix  + "depth_blur_blur", 1));
		m_Params.push_back(ParamWithName<T>(&m_Radius,             prefix + "depth_blur_radius", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Alpha,        prefix + "depth_blur_alpha"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Length,       prefix + "depth_blur_length"));
		m_Params.push_back(ParamWithName<T>(true, &m_BlurOver10,   prefix + "depth_blur_blur_over_10"));
		m_Params.push_back(ParamWithName<T>(true, &m_Power2,       prefix + "depth_blur_power2"));
		m_Params.push_back(ParamWithName<T>(true, &m_2piOverPower, prefix + "depth_blur_2pi_over_power"));
		m_Params.push_back(ParamWithName<T>(true, &m_SinAlpha,     prefix + "depth_blur_sin_alpha"));
		m_Params.push_back(ParamWithName<T>(true, &m_CosAlpha,     prefix + "depth_blur_cos_alpha"));
	}

private:
	T m_Power;
	T m_Range;
	T m_Blur;
	T m_Radius;
	T m_Alpha;//Precalc.
	T m_Length;
	T m_BlurOver10;
	T m_Power2;
	T m_2piOverPower;
	T m_SinAlpha;
	T m_CosAlpha;

};

/// <summary>
/// depth_blur2.
/// By tatasz.
/// </summary>
template <typename T>
class DepthBlur2Variation : public ParametricVariation<T>
{
public:
	DepthBlur2Variation(T weight = 1.0) : ParametricVariation<T>("depth_blur2", eVariationId::VAR_DEPTH_BLUR2, weight)
	{
		Init();
	}

	PARVARCOPY(DepthBlur2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T bx = 0;
		T by = 0;
		T rad = std::sqrt(Sqr((helper.In.x - m_X0) * m_OneOverMulX) + Sqr((helper.In.y - m_Y0) * m_OneOverMulY));

		if (rad > m_Radius)
		{
			T f = rand.Frand01<T>() * m_Power2;
			T int_angle = std::trunc(f);
			f = f - int_angle;
			T x = f * m_Length;
			T z = std::sqrt(1 + SQR(x) - 2 * x * std::cos(m_Alpha));
			int iangle = int(int_angle);

			if (iangle & 1)
				int_angle = m_2piOverPower * (iangle / 2) + std::asin(std::sin(m_Alpha) * x / Zeps(z));
			else
				int_angle = m_2piOverPower * (iangle / 2) - std::asin(std::sin(m_Alpha) * x / Zeps(z));

			z *= std::sqrt(rand.Frand01<T>());
			auto temp = int_angle - T(M_PI_2);
			by = std::sin(temp);
			bx = std::cos(temp);
			T aux = z * m_BlurOver10 * std::exp(std::log(rad - m_Radius) * m_Exp);
			by = aux * by;
			bx = aux * bx;
		}

		helper.Out.x = m_Weight * (helper.In.x + bx);
		helper.Out.y = m_Weight * (helper.In.y + by);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string range          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blur           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x0             = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y0             = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulx           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string muly           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string exp            = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string alpha          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string length         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurover10     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power2         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string twopioverpower = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneovermulx    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneovermuly    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t bx = 0;\n"
		   << "\t\treal_t by = 0;\n"
		   << "\t\treal_t rad = sqrt(Sqr((vIn.x - " << x0 << ") * " << oneovermulx << ") + Sqr((vIn.y - " << y0 << ") * " << oneovermuly << "));\n"
		   << "\n"
		   << "\t\tif (rad > " << radius << ")\n"
		   << "\t\t{\n"
		   << "\t\t	real_t f = MwcNext01(mwc) * " << power2 << ";\n"
		   << "\t\t	real_t int_angle = trunc(f);\n"
		   << "\t\t	f = f - int_angle;\n"
		   << "\t\t	real_t x = f * " << length << ";\n"
		   << "\t\t	real_t z = sqrt(1 + SQR(x) - 2 * x * cos(" << alpha << "));\n"
		   << "\t\t	int iangle = (int)int_angle;\n"
		   << "\n"
		   << "\t\t	if (iangle & 1)\n"
		   << "\t\t		int_angle = " << twopioverpower << " * (iangle / 2) + asin(sin(" << alpha << ") * x / Zeps(z));\n"
		   << "\t\t	else\n"
		   << "\t\t		int_angle = " << twopioverpower << " * (iangle / 2) - asin(sin(" << alpha << ") * x / Zeps(z));\n"
		   << "\n"
		   << "\t\t	z *= sqrt(MwcNext01(mwc));\n"
		   << "\t\t	real_t temp = int_angle - MPI2;\n"
		   << "\t\t	by = sin(temp);\n"
		   << "\t\t	bx = cos(temp);\n"
		   << "\t\t	real_t aux = z * " << blurover10 << " * exp(log(rad - " << radius << ") * " << exp << ");\n"
		   << "\t\t	by = aux * by;\n"
		   << "\t\t	bx = aux * bx;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (vIn.x + bx);\n"
		   << "\t\tvOut.y = " << weight << " * (vIn.y + by);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Alpha = T(M_PI) / Zeps(m_Power);
		m_Length = T(std::sqrt(1 + SQR(m_Range) - 2 * m_Range * std::cos(m_Alpha)));
		m_Alpha = std::asin(std::sin(m_Alpha) * m_Range / Zeps(m_Length));
		m_BlurOver10 = m_Blur / 10;
		m_Power2 = m_Power * 2;
		m_2piOverPower = M_2PI / Zeps(m_Power);
		m_OneOverMulX = 1 / Zeps(m_MulX);
		m_OneOverMulY = 1 / Zeps(m_MulY);
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps", "Sqr" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power,               prefix + "depth_blur2_power", 5));
		m_Params.push_back(ParamWithName<T>(&m_Range,               prefix + "depth_blur2_range", T(0.401623)));
		m_Params.push_back(ParamWithName<T>(&m_Blur,                prefix + "depth_blur2_blur", 1));
		m_Params.push_back(ParamWithName<T>(&m_Radius,              prefix + "depth_blur2_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_X0,                  prefix + "depth_blur2_x0", 0));
		m_Params.push_back(ParamWithName<T>(&m_Y0,                  prefix + "depth_blur2_y0", 0));
		m_Params.push_back(ParamWithName<T>(&m_MulX,                prefix + "depth_blur2_mulx", 1));
		m_Params.push_back(ParamWithName<T>(&m_MulY,                prefix + "depth_blur2_muly", 1));
		m_Params.push_back(ParamWithName<T>(&m_Exp,                 prefix + "depth_blur2_exp", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Alpha,         prefix + "depth_blur2_alpha"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Length,        prefix + "depth_blur2_length"));
		m_Params.push_back(ParamWithName<T>(true, &m_BlurOver10,    prefix + "depth_blur2_blur_over_10"));
		m_Params.push_back(ParamWithName<T>(true, &m_Power2,        prefix + "depth_blur2_power2"));
		m_Params.push_back(ParamWithName<T>(true, &m_2piOverPower,  prefix + "depth_blur2_2pi_over_power"));
		m_Params.push_back(ParamWithName<T>(true, &m_OneOverMulX,   prefix + "depth_blur2_one_over_mulx"));
		m_Params.push_back(ParamWithName<T>(true, &m_OneOverMulY,   prefix + "depth_blur2_one_over_muly"));
	}

private:
	T m_Power;
	T m_Range;
	T m_Blur;
	T m_Radius;
	T m_X0;
	T m_Y0;
	T m_MulX;
	T m_MulY;
	T m_Exp;
	T m_Alpha;//Precalc.
	T m_Length;
	T m_BlurOver10;
	T m_Power2;
	T m_2piOverPower;
	T m_OneOverMulX;
	T m_OneOverMulY;
};

/// <summary>
/// depth_gaussian.
/// By tatasz.
/// </summary>
template <typename T>
class DepthGaussianVariation : public ParametricVariation<T>
{
public:
	DepthGaussianVariation(T weight = 1.0) : ParametricVariation<T>("depth_gaussian", eVariationId::VAR_DEPTH_GAUSSIAN, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(DepthGaussianVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T bx = 0;
		T by = 0;
		T rad = helper.m_PrecalcSqrtSumSquares;

		if (rad > m_Radius)
		{
			T r = std::sqrt(-2 * log(rand.Frand01<T>()));
			T ang = rand.Frand01<T>() * M_2PI;
			bx = std::cos(ang);
			by = std::sin(ang);
			T aux = (r * m_BlurOver10 * (rad - m_Radius));
			by = aux * by;
			bx = aux * bx;
		}

		helper.Out.x = m_Weight * (helper.In.x + bx);
		helper.Out.y = m_Weight * (helper.In.y + by);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string blur       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurover10 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t bx = 0;\n"
		   << "\t\treal_t by = 0;\n"
		   << "\t\treal_t rad = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tif (rad > " << radius << ")\n"
		   << "\t\t{\n"
		   << "\t\t	real_t r = sqrt(-2 * log(MwcNext01(mwc)));\n"
		   << "\t\t	real_t ang = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\t	bx = cos(ang);\n"
		   << "\t\t	by = sin(ang);\n"
		   << "\t\t	real_t aux = (r * " << blurover10 << " * (rad - " << radius << "));\n"
		   << "\t\t	by = aux * by;\n"
		   << "\t\t	bx = aux * bx;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (vIn.x + bx);\n"
		   << "\t\tvOut.y = " << weight << " * (vIn.y + by);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_BlurOver10 = m_Blur / 10;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Blur,             prefix + "depth_gaussian_blur", 1));
		m_Params.push_back(ParamWithName<T>(&m_Radius,           prefix + "depth_gaussian_radius", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_BlurOver10, prefix + "depth_gaussian_blur_over_10"));//Precalc.
	}

private:
	T m_Blur;
	T m_Radius;
	T m_BlurOver10;//Precalc.
};

/// <summary>
/// depth_gaussian2.
/// By tatasz.
/// </summary>
template <typename T>
class DepthGaussian2Variation : public ParametricVariation<T>
{
public:
	DepthGaussian2Variation(T weight = 1.0) : ParametricVariation<T>("depth_gaussian2", eVariationId::VAR_DEPTH_GAUSSIAN2, weight)
	{
		Init();
	}

	PARVARCOPY(DepthGaussian2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T bx = 0;
		T by = 0;
		T rad = std::sqrt(Sqr((helper.In.x - m_X0) / m_OneOverMulXSq) + Sqr((helper.In.y - m_Y0) / m_OneOverMulYSq));

		if (rad > m_Radius)
		{
			T r = std::sqrt(-2 * std::log(rand.Frand01<T>()));
			T ang = rand.Frand01<T>() * M_2PI;
			bx = std::cos(ang);
			by = std::sin(ang);
			T aux = r * m_BlurOver10 * std::exp(std::log(rad - m_Radius) * m_Exp);
			by = aux * by;
			bx = aux * bx;
		}

		helper.Out.x = m_Weight * (helper.In.x + bx);
		helper.Out.y = m_Weight * (helper.In.y + by);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string blur          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x0            = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y0            = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulx          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string muly          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string exp           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurover10    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneovermulsqx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneovermulsqy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t bx = 0;\n"
		   << "\t\treal_t by = 0;\n"
		   << "\t\treal_t rad = sqrt(Sqr((vIn.x - " << x0 << ") / " << oneovermulsqx << ") + Sqr((vIn.y - " << y0 << ") / " << oneovermulsqy << "));\n"
		   << "\n"
		   << "\t\tif (rad > " << radius << ")\n"
		   << "\t\t{\n"
		   << "\t\t	real_t r = sqrt(-2 * log(MwcNext01(mwc)));\n"
		   << "\t\t	real_t ang = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\t	bx = cos(ang);\n"
		   << "\t\t	by = sin(ang);\n"
		   << "\t\t	real_t aux = r * " << blurover10 << " * exp(log(rad - " << radius << ") * " << exp << ");\n"
		   << "\t\t	by = aux * by;\n"
		   << "\t\t	bx = aux * bx;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (vIn.x + bx);\n"
		   << "\t\tvOut.y = " << weight << " * (vIn.y + by);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_BlurOver10 = m_Blur / 10;
		m_OneOverMulXSq = 1 / Zeps(SQR(m_MulX));
		m_OneOverMulYSq = 1 / Zeps(SQR(m_MulY));
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Sqr" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Blur,                prefix + "depth_gaussian2_blur", 1));
		m_Params.push_back(ParamWithName<T>(&m_Radius,              prefix + "depth_gaussian2_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_X0,                  prefix + "depth_gaussian2_x0", 0));
		m_Params.push_back(ParamWithName<T>(&m_Y0,                  prefix + "depth_gaussian2_y0", 0));
		m_Params.push_back(ParamWithName<T>(&m_MulX,                prefix + "depth_gaussian2_mulx", 1));
		m_Params.push_back(ParamWithName<T>(&m_MulY,                prefix + "depth_gaussian2_muly", 1));
		m_Params.push_back(ParamWithName<T>(&m_Exp,                 prefix + "depth_gaussian2_exp", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_BlurOver10,    prefix + "depth_gaussian2_blur_over_10"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_OneOverMulXSq, prefix + "depth_gaussian2_one_over_mulx_sq"));
		m_Params.push_back(ParamWithName<T>(true, &m_OneOverMulYSq, prefix + "depth_gaussian2_one_over_muly_sq"));
	}

private:
	T m_Blur;
	T m_Radius;
	T m_X0;
	T m_Y0;
	T m_MulX;
	T m_MulY;
	T m_Exp;
	T m_BlurOver10;//Precalc.
	T m_OneOverMulXSq;
	T m_OneOverMulYSq;
};

/// <summary>
/// depth_ngon.
/// By tatasz.
/// </summary>
template <typename T>
class DepthNgonVariation : public ParametricVariation<T>
{
public:
	DepthNgonVariation(T weight = 1.0) : ParametricVariation<T>("depth_ngon", eVariationId::VAR_DEPTH_NGON, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(DepthNgonVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T bx = 0;
		T by = 0;
		T rad = helper.m_PrecalcSqrtSumSquares;

		if (rad > m_Radius)
		{
			T ang = rand.Frand01<T>() * M_2PI;
			T phi = ang - m_Side * Floor(ang / Zeps(m_Side));

			if (phi > m_HalfSide)
				phi -= m_Side;

			phi = 1 / Zeps(std::cos(phi));
			T aux = phi * m_BlurOver10 * (rad - m_Radius);
			bx = aux * std::cos(ang);
			by = aux * std::sin(ang);
		}

		helper.Out.x = m_Weight * (helper.In.x + bx);
		helper.Out.y = m_Weight * (helper.In.y + by);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blur       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string side       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfside   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurover10 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t bx = 0;\n"
		   << "\t\treal_t by = 0;\n"
		   << "\t\treal_t rad = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tif (rad > " << radius << ")\n"
		   << "\t\t{\n"
		   << "\t\t	real_t ang = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\t	real_t phi = ang - " << side << " * floor(ang / Zeps(" << side << "));\n"
		   << "\n"
		   << "\t\t	if (phi > " << halfside << ")\n"
		   << "\t\t		phi -= " << side << ";\n"
		   << "\n"
		   << "\t\t	phi = 1 / Zeps(cos(phi));\n"
		   << "\t\t	real_t aux = phi * " << blurover10 << " * (rad - " << radius << ");\n"
		   << "\t\t	bx = aux * cos(ang);\n"
		   << "\t\t	by = aux * sin(ang);\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (vIn.x + bx);\n"
		   << "\t\tvOut.y = " << weight << " * (vIn.y + by);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Side = M_2PI / Zeps(m_Power);
		m_HalfSide = m_Side / 2;
		m_BlurOver10 = m_Blur / 10;
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
		m_Params.push_back(ParamWithName<T>(&m_Power,            prefix + "depth_ngon_power", 5));
		m_Params.push_back(ParamWithName<T>(&m_Blur,             prefix + "depth_ngon_blur", 1));
		m_Params.push_back(ParamWithName<T>(&m_Radius,           prefix + "depth_ngon_radius", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Side,       prefix + "depth_ngon_side"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_HalfSide,   prefix + "depth_ngon_half_side"));
		m_Params.push_back(ParamWithName<T>(true, &m_BlurOver10, prefix + "depth_ngon_blur_over_10"));
	}

private:
	T m_Power;
	T m_Blur;
	T m_Radius;
	T m_Side;//Precalc.
	T m_HalfSide;
	T m_BlurOver10;
};

/// <summary>
/// depth_ngon2.
/// By tatasz.
/// </summary>
template <typename T>
class DepthNgon2Variation : public ParametricVariation<T>
{
public:
	using Variation<T>::m_NeedPrecalcSumSquares;
	using Variation<T>::m_NeedPrecalcSqrtSumSquares;

	DepthNgon2Variation(T weight = 1.0) : ParametricVariation<T>("depth_ngon2", eVariationId::VAR_DEPTH_NGON2, weight)
	{
		Init();
		m_NeedPrecalcSumSquares = m_NeedPrecalcSqrtSumSquares = Compat::m_Compat;
	}

	PARVARCOPY(DepthNgon2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T bx;
		T by;

		if (Compat::m_Compat)
		{
			T rad = helper.m_PrecalcSqrtSumSquares;

			if (rad > m_Radius)
			{
				T ang = rand.Frand01<T>() * M_2PI;
				T phi = ang - m_Side * Floor(ang / Zeps(m_Side));

				if (phi > m_HalfSide)
					phi -= m_Side;

				phi = 1 / Zeps(std::cos(phi));
				T aux = phi * m_BlurOver10 * (rad - m_Radius);
				bx = aux * std::cos(ang);
				by = aux * std::sin(ang);
			}
			else
			{
				bx = 0;
				by = 0;
			}
		}
		else
		{
			T rad = std::sqrt(Sqr(helper.In.x - m_X0) * m_OneOverMulXSq + Sqr(helper.In.y - m_Y0) * m_OneOverMulYSq);

			if (rad > m_Radius)
			{
				T ang = rand.Frand01<T>() * M_2PI;
				T phi = ang - m_Side * Floor(ang / Zeps(m_Side));
				T phi2 = phi > m_HalfSide ? phi - m_Side : phi;
				T amp = 1 / Zeps(std::cos(phi2));
				T aux = amp * m_BlurOver10 * std::exp(std::log(rad - m_Radius) * m_Exp);
				bx = aux * std::cos(ang);
				by = aux * std::sin(ang);
			}
			else
			{
				bx = 0;
				by = 0;
			}
		}

		helper.Out.x = m_Weight * (helper.In.x + bx);
		helper.Out.y = m_Weight * (helper.In.y + by);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blur          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x0            = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y0            = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulx          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string muly          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string exp           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string side          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfside      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurover10    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneovermulsqx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneovermulsqy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t bx;\n"
		   << "\t\treal_t by;\n"
		   << "\t\treal_t weight = " << weight << ";\n"
		   << "\t\treal_t side = " << side << ";\n"
		   << "\t\treal_t radius = " << radius << ";\n";

		if (Compat::m_Compat)
		{
			ss << "\t\treal_t rad = precalcSqrtSumSquares;\n"
			   << "\n"
			   << "\t\tif (rad > radius)\n"
			   << "\t\t{\n"
			   << "\t\t	real_t ang = MwcNext01(mwc) * M_2PI;\n"
			   << "\t\t	real_t phi = ang - side * floor(ang / Zeps(side));\n"
			   << "\n"
			   << "\t\t	if (phi > " << halfside << ")\n"
			   << "\t\t		phi -= side;\n"
			   << "\n"
			   << "\t\t	phi = 1 / Zeps(cos(phi));\n"
			   << "\t\t	real_t aux = phi * " << blurover10 << " * (rad - radius);\n"
			   << "\t\t	bx = aux * cos(ang);\n"
			   << "\t\t	by = aux * sin(ang);\n"
			   << "\t\t}\n"
			   << "\t\telse\n"
			   << "\t\t{\n"
			   << "\t\t	bx = 0;\n"
			   << "\t\t	by = 0;\n"
			   << "\t\t}\n"
			   << "\n";
		}
		else
		{
			ss << "\t\treal_t rad = sqrt(Sqr(vIn.x - " << x0 << ") * " << oneovermulsqx << " + Sqr(vIn.y - " << y0 << ") * " << oneovermulsqy << ");\n"
			   << "\n"
			   << "\t\tif (rad > radius)\n"
			   << "\t\t{\n"
			   << "\t\t	real_t ang = MwcNext01(mwc) * M_2PI;\n"
			   << "\t\t	real_t phi = ang - side * floor(ang / Zeps(side));\n"
			   << "\n"
			   << "\t\t	if (phi > " << halfside << ")\n"
			   << "\t\t		phi -= side;\n"
			   << "\n"
			   << "\t\t	real_t amp = 1 / Zeps(cos(phi));\n"
			   << "\t\t	real_t aux = amp * " << blurover10 << " * exp(log(rad - radius) * " << exp << ");\n"
			   << "\t\t	bx = aux * cos(ang);\n"
			   << "\t\t	by = aux * sin(ang);\n"
			   << "\t\t}\n"
			   << "\t\telse\n"
			   << "\t\t{\n"
			   << "\t\t	bx = 0;\n"
			   << "\t\t	by = 0;\n"
			   << "\t\t}\n"
			   << "\n";
		}

		ss << "\t\tvOut.x = weight * (vIn.x + bx);\n"
		   << "\t\tvOut.y = weight * (vIn.y + by);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Side = M_2PI / Zeps(m_Power);
		m_HalfSide = m_Side / 2;
		m_BlurOver10 = m_Blur / 10;
		m_OneOverMulXSq = 1 / Zeps(SQR(m_MulX));
		m_OneOverMulYSq = 1 / Zeps(SQR(m_MulY));
		m_NeedPrecalcSumSquares = m_NeedPrecalcSqrtSumSquares = Compat::m_Compat;
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		if (Compat::m_Compat)
			return vector<string> { "Zeps" };
		else
			return vector<string> { "Zeps", "Sqr" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power,               prefix + "depth_ngon2_power", 5));
		m_Params.push_back(ParamWithName<T>(&m_Blur,                prefix + "depth_ngon2_blur", 1));
		m_Params.push_back(ParamWithName<T>(&m_Radius,              prefix + "depth_ngon2_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_X0,                  prefix + "depth_ngon2_x0", 0));
		m_Params.push_back(ParamWithName<T>(&m_Y0,                  prefix + "depth_ngon2_y0", 0));
		m_Params.push_back(ParamWithName<T>(&m_MulX,                prefix + "depth_ngon2_mulx", 1));
		m_Params.push_back(ParamWithName<T>(&m_MulY,                prefix + "depth_ngon2_muly", 1));
		m_Params.push_back(ParamWithName<T>(&m_Exp,                 prefix + "depth_ngon2_exp", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Side,          prefix + "depth_ngon2_side"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_HalfSide,      prefix + "depth_ngon2_half_side"));
		m_Params.push_back(ParamWithName<T>(true, &m_BlurOver10,    prefix + "depth_ngon2_blur_over_10"));
		m_Params.push_back(ParamWithName<T>(true, &m_OneOverMulXSq, prefix + "depth_ngon2_one_over_mulx_sq"));
		m_Params.push_back(ParamWithName<T>(true, &m_OneOverMulYSq, prefix + "depth_ngon2_one_over_muly_sq"));
	}

private:
	T m_Power;
	T m_Blur;
	T m_Radius;
	T m_X0;
	T m_Y0;
	T m_MulX;
	T m_MulY;
	T m_Exp;
	T m_Side;//Precalc.
	T m_HalfSide;
	T m_BlurOver10;
	T m_OneOverMulXSq;
	T m_OneOverMulYSq;
};

/// <summary>
/// depth_sine.
/// By tatasz.
/// </summary>
template <typename T>
class DepthSineVariation : public ParametricVariation<T>
{
public:
	DepthSineVariation(T weight = 1.0) : ParametricVariation<T>("depth_sine", eVariationId::VAR_DEPTH_SINE, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(DepthSineVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T bx = 0;
		T by = 0;
		T rad = helper.m_PrecalcSqrtSumSquares;

		if (rad > m_Radius)
		{
			T ang = rand.Frand01<T>() * M_2PI;
			T r = (m_Power == 1 ? std::acos(rand.Frand01<T>() * 2 - 1) / T(M_PI) : std::acos(std::exp(std::log(1 - rand.Frand01<T>()) * m_Power) * 2 - 1) / T(M_PI));
			T aux = r * m_BlurOver10 * (rad - m_Radius);
			bx = aux * std::cos(ang);
			by = aux * std::sin(ang);
		}

		helper.Out.x = m_Weight * (helper.In.x + bx);
		helper.Out.y = m_Weight * (helper.In.y + by);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blur       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurover10 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t bx = 0;\n"
		   << "\t\treal_t by = 0;\n"
		   << "\t\treal_t rad = precalcSqrtSumSquares;\n"
		   << "\n"
		   << "\t\tif (rad > " << radius << ")\n"
		   << "\t\t{\n"
		   << "\t\t	real_t ang = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\t	real_t r = (" << power << " == 1 ? acos(fma(MwcNext01(mwc), (real_t)(2.0), (real_t)(-1.0))) / MPI : acos(exp(log((real_t)(1.0) - MwcNext01(mwc)) * " << power << ") * (real_t)(2.0) - (real_t)(1.0)) / MPI);\n"
		   << "\t\t	real_t aux = r * " << blurover10 << " * (rad - " << radius << ");\n"
		   << "\t\t	bx = aux * cos(ang);\n"
		   << "\t\t	by = aux * sin(ang);\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (vIn.x + bx);\n"
		   << "\t\tvOut.y = " << weight << " * (vIn.y + by);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_BlurOver10 = m_Blur / 10;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power,            prefix + "depth_sine_power", 1, eParamType::REAL, 0));
		m_Params.push_back(ParamWithName<T>(&m_Blur,             prefix + "depth_sine_blur", 1));
		m_Params.push_back(ParamWithName<T>(&m_Radius,           prefix + "depth_sine_radius", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_BlurOver10, prefix + "depth_sine_blur_over_10"));//Precalc.
	}

private:
	T m_Power;
	T m_Blur;
	T m_Radius;
	T m_BlurOver10;//Precalc.
};

/// <summary>
/// depth_sine2.
/// By tatasz.
/// </summary>
template <typename T>
class DepthSine2Variation : public ParametricVariation<T>
{
public:
	DepthSine2Variation(T weight = 1.0) : ParametricVariation<T>("depth_sine2", eVariationId::VAR_DEPTH_SINE2, weight)
	{
		Init();
	}

	PARVARCOPY(DepthSine2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T rad;
		T bx = 0;
		T by = 0;

		if (Compat::m_Compat)
			rad = std::sqrt(Sqr((helper.In.x - m_X0) / Zeps(m_MulX)) + Sqr((helper.In.y - m_Y0) / Zeps(m_MulY)));
		else
			rad = std::sqrt(Sqr(helper.In.x - m_X0) * m_OneOverMulXSq + Sqr(helper.In.y - m_Y0) * m_OneOverMulYSq);

		if (rad > m_Radius)
		{
			T ang = rand.Frand01<T>() * M_2PI;
			T r = (m_Power == 1 ? std::acos(rand.Frand01<T>() * 2 - 1) / T(M_PI) : std::acos(std::exp(std::log(1 - rand.Frand01<T>()) * m_Power) * 2 - 1) / T(M_PI));
			T aux = r * m_BlurOver10 * std::exp(std::log(rad - m_Radius) * m_Exp);
			bx = aux * std::cos(ang);
			by = aux * std::sin(ang);
		}

		helper.Out.x = m_Weight * (helper.In.x + bx);
		helper.Out.y = m_Weight * (helper.In.y + by);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blur          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x0            = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y0            = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mulx          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string muly          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string exp           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blurover10    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneovermulsqx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string oneovermulsqy = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t bx = 0;\n"
		   << "\t\treal_t by = 0;\n"
		   << "\t\treal_t weight = " << weight << ";\n";

		if (Compat::m_Compat)
			ss << "\t\treal_t rad = sqrt(Sqr((vIn.x - " << x0 << ") / " << mulx << ") + Sqr((vIn.y - " << y0 << ") / " << muly << ")); \n";
		else
			ss << "\t\treal_t rad = sqrt(Sqr(vIn.x - " << x0 << ") * " << oneovermulsqx << " + Sqr(vIn.y - " << y0 << ") * " << oneovermulsqy << "); \n";

		ss << "\n"
		   << "\t\tif (rad > " << radius << ")\n"
		   << "\t\t{\n"
		   << "\t\t	real_t ang = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\t	real_t r = (" << power << " == 1 ? acos(fma(MwcNext01(mwc), 2, (real_t)(-1.0))) / MPI : acos(exp(log(1 - MwcNext01(mwc)) * " << power << ") * 2 - 1) / MPI);\n"
		   << "\t\t	real_t aux = r * " << blurover10 << " * exp(log(rad - " << radius << ") * " << exp << ");\n"
		   << "\t\t	bx = aux * cos(ang);\n"
		   << "\t\t	by = aux * sin(ang);\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.x = weight * (vIn.x + bx);\n"
		   << "\t\tvOut.y = weight * (vIn.y + by);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}
	virtual void Precalc() override
	{
		m_Power = std::max(T(0), m_Power);
		m_BlurOver10 = m_Blur / 10;
		m_OneOverMulXSq = 1 / Zeps(SQR(m_MulX));
		m_OneOverMulYSq = 1 / Zeps(SQR(m_MulY));
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Sqr" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power,               prefix + "depth_sine2_power", 1, eParamType::REAL, 0));
		m_Params.push_back(ParamWithName<T>(&m_Blur,                prefix + "depth_sine2_blur", 1));
		m_Params.push_back(ParamWithName<T>(&m_Radius,              prefix + "depth_sine2_radius", 1));
		m_Params.push_back(ParamWithName<T>(&m_X0,                  prefix + "depth_sine2_x0", 0));
		m_Params.push_back(ParamWithName<T>(&m_Y0,                  prefix + "depth_sine2_y0", 0));
		m_Params.push_back(ParamWithName<T>(&m_MulX,                prefix + "depth_sine2_mulx", 1));
		m_Params.push_back(ParamWithName<T>(&m_MulY,                prefix + "depth_sine2_muly", 1));
		m_Params.push_back(ParamWithName<T>(&m_Exp,                 prefix + "depth_sine2_exp", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_BlurOver10,    prefix + "depth_sine2_blur_over_10"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_OneOverMulXSq, prefix + "depth_sine2_one_over_mulx_sq"));
		m_Params.push_back(ParamWithName<T>(true, &m_OneOverMulYSq, prefix + "depth_sine2_one_over_muly_sq"));
	}
private:
	T m_Power;
	T m_Blur;
	T m_Radius;
	T m_X0;
	T m_Y0;
	T m_MulX;
	T m_MulY;
	T m_Exp;
	T m_BlurOver10;//Precalc.
	T m_OneOverMulXSq;
	T m_OneOverMulYSq;
};

/// <summary>
/// coth_spiral.
/// </summary>
template <typename T>
class CothSpiralVariation : public ParametricVariation<T>
{
public:
	CothSpiralVariation(T weight = 1.0) : ParametricVariation<T>("coth_spiral", eVariationId::VAR_COTH_SPIRAL, weight)
	{
		Init();
	}

	PARVARCOPY(CothSpiralVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T t2 = (rand.Frand01<T>() - T(0.5)) * M_2PI;
		T aux = Zeps(std::cos(m_A * t2) - std::cosh(t2));
		helper.Out.x = m_Weight * (-std::sinh(t2) / aux);
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
		   << "\t\treal_t aux = Zeps(cos(" << a << " * t2) - cosh(t2));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * (-sinh(t2) / aux);\n"
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
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "coth_spiral_a", 4));
	}

private:
	T m_A;
};

/// <summary>
/// dust.
/// By tatasz.
/// </summary>
template <typename T>
class DustVariation : public ParametricVariation<T>
{
public:
	DustVariation(T weight = 1.0) : ParametricVariation<T>("dust", eVariationId::VAR_DUST, weight)
	{
		Init();
	}

	PARVARCOPY(DustVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T X = std::fmod(T(Floor(std::abs(helper.In.x))), m_Zdens) / m_Zdens;
		T Y = std::fmod(T(Floor(std::abs(helper.In.y))), m_Zdens) / m_Zdens;
		T random_x = VarFuncs<T>::HashShadertoy(X, Y, 1);
		T random_y = VarFuncs<T>::HashShadertoy(Y, X, 2);
		T a = (X + random_x * m_Dist) * M_2PI;
		T r = std::exp(std::log(Y + random_y * m_Dist) * m_Power);
		helper.Out.x = m_Weight * std::cos(a) * r;
		helper.Out.y = m_Weight * std::sin(a) * r;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string dens  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dist  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string zdens = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t X = fmod(floor(fabs(vIn.x)), " << zdens << ") / " << zdens << ";\n"
		   << "\t\treal_t Y = fmod(floor(fabs(vIn.y)), " << zdens << ") / " << zdens << ";\n"
		   << "\t\treal_t random_x = HashShadertoy(X, Y, (real_t)(1.0));\n"
		   << "\t\treal_t random_y = HashShadertoy(Y, X, (real_t)(2.0));\n"
		   << "\t\treal_t a = fma(random_x, " << dist << ", X) * M_2PI;\n"
		   << "\t\treal_t r = exp(log(fma(random_y, " << dist << ", Y)) * " << power << ");\n"
		   << "\t\tvOut.x = " << weight << " * cos(a) * r;\n"
		   << "\t\tvOut.y = " << weight << " * sin(a) * r;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Zdens = Zeps(m_Density);
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Fract", "HashShadertoy" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Density,     prefix + "dust_density", 5, eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Dist,        prefix + "dust_distortion", 1));
		m_Params.push_back(ParamWithName<T>(&m_Power,       prefix + "dust_power", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Zdens, prefix + "dust_zdens"));//Precalc.
	}

private:
	T m_Density;
	T m_Dist;
	T m_Power;
	T m_Zdens;//Precalc.
};

/// <summary>
/// asteria.
/// By dark-beam.
/// </summary>
template <typename T>
class AsteriaVariation : public ParametricVariation<T>
{
public:
	AsteriaVariation(T weight = 1.0) : ParametricVariation<T>("asteria", eVariationId::VAR_ASTERIA, weight)
	{
		Init();
	}

	PARVARCOPY(AsteriaVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x0 = m_Weight * helper.In.x;
		T y0 = m_Weight * helper.In.y;
		T xx = x0;
		T yy = y0;
		T r = SQR(xx) + SQR(yy);
		xx = Sqr(std::abs(xx) - 1);
		yy = Sqr(std::abs(yy) - 1);
		T r2 = std::sqrt(yy + xx);
		bool in1 = r < 1;
		bool out2 = r2 < 1;

		if (in1 && out2)
			in1 = (rand.Frand01<T>() > 0.35);
		else
			in1 = !in1;

		if (in1)
		{
			helper.Out.x = x0;
			helper.Out.y = y0;
		}
		else
		{
			if (Compat::m_Compat)
				xx = x0 * m_CosAlpha - y0 * m_SinAlpha;
			else
				xx = x0 * m_CosAlpha + y0 * m_SinAlpha;

			yy = x0 * m_SinAlpha + y0 * m_CosAlpha;
			T nx = xx / std::sqrt(1 - SQR(yy)) * (1 - std::sqrt(1 - Sqr(-std::abs(yy) + 1)));
			xx =  nx * m_CosAlpha + yy * m_SinAlpha;
			yy = -nx * m_SinAlpha + yy * m_CosAlpha;
			helper.Out.x = xx;
			helper.Out.y = yy;
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
		string alpha = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sina  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosa  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t weight = " << weight << ";\n"
		   << "\t\treal_t cosa = " << cosa << ";\n"
		   << "\t\treal_t sina = " << sina << ";\n"
		   << "\t\treal_t x0 = weight * vIn.x;\n"
		   << "\t\treal_t y0 = weight * vIn.y;\n"
		   << "\t\treal_t xx = x0;\n"
		   << "\t\treal_t yy = y0;\n"
		   << "\t\treal_t r = SQR(xx) + SQR(yy);\n"
		   << "\t\txx = Sqr(fabs(xx) - 1);\n"
		   << "\t\tyy = Sqr(fabs(yy) - 1);\n"
		   << "\t\treal_t r2 = sqrt(yy + xx);\n"
		   << "\t\tbool in1 = r < 1;\n"
		   << "\t\tbool out2 = r2 < 1;\n"
		   << "\n"
		   << "\t\tif (in1 && out2)\n"
		   << "\t\t	in1 = MwcNext01(mwc) > 0.35;\n"
		   << "\t\telse\n"
		   << "\t\t	in1 = !in1;\n"
		   << "\n"
		   << "\t\tif (in1)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = x0;\n"
		   << "\t\t	vOut.y = y0;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n";

		if (Compat::m_Compat)
			ss << "\t\t	xx = x0 * cosa - y0 * sina;\n";
		else
			ss << "\t\t	xx = x0 * cosa + y0 * sina;\n";

		ss << "\t\t	yy = x0 * sina + y0 * cosa;\n"
		   << "\t\t	real_t nx = xx / sqrt(1 - SQR(yy)) * (1 - sqrt(1 - Sqr(-fabs(yy) + 1)));\n"
		   << "\t\t	xx = nx * cosa + yy * sina;\n"
		   << "\t\t	yy = -nx * sina + yy * cosa;\n"
		   << "\t\t	vOut.x = xx;\n"
		   << "\t\t	vOut.y = yy;\n"
		   << "\t\t}\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_SinAlpha = T(std::sin(M_PI * m_Alpha));
		m_CosAlpha = T(std::cos(M_PI * m_Alpha));
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Sqr" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Alpha,          prefix + "asteria_alpha", 0, eParamType::REAL_CYCLIC, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_SinAlpha, prefix + "asteria_sin_alpha"));
		m_Params.push_back(ParamWithName<T>(true, &m_CosAlpha, prefix + "asteria_cos_alpha"));
	}

private:
	T m_Alpha;
	T m_SinAlpha;//Precalc.
	T m_CosAlpha;
};

/// <summary>
/// pulse.
/// By FarDareisMai.
/// </summary>
template <typename T>
class PulseVariation : public ParametricVariation<T>
{
public:
	PulseVariation(T weight = 1.0) : ParametricVariation<T>("pulse", eVariationId::VAR_PULSE, weight)
	{
		Init();
	}

	PARVARCOPY(PulseVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (helper.In.x + m_ScaleX * std::sin(helper.In.x * m_FreqX));
		helper.Out.y = m_Weight * (helper.In.y + m_ScaleY * std::sin(helper.In.y * m_FreqY));
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string freqx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqy  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scalex = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaley = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * fma(" << scalex << ", sin(vIn.x * " << freqx << "), vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * fma(" << scaley << ", sin(vIn.y * " << freqy << "), vIn.y);\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_FreqX,  prefix + "pulse_freqx",  2));
		m_Params.push_back(ParamWithName<T>(&m_FreqY,  prefix + "pulse_freqy",  2));
		m_Params.push_back(ParamWithName<T>(&m_ScaleX, prefix + "pulse_scalex", 1));
		m_Params.push_back(ParamWithName<T>(&m_ScaleY, prefix + "pulse_scaley", 1));
	}

private:
	T m_FreqX;
	T m_FreqY;
	T m_ScaleX;
	T m_ScaleY;
};

/// <summary>
/// excinis.
/// </summary>
template <typename T>
class ExcinisVariation : public ParametricVariation<T>
{
public:
	ExcinisVariation(T weight = 1.0) : ParametricVariation<T>("excinis", eVariationId::VAR_EXCINIS, weight, true, true)
	{
		Init();
	}

	PARVARCOPY(ExcinisVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r_prior = helper.m_PrecalcSqrtSumSquares * m_LengthScalar;
		T r_sign = T(r_prior >= 0 ? 1 : -1);
		T r_eps = std::abs(r_prior) > m_Eps ? 0 : r_sign * m_Eps;
		T r = m_Weight / Zeps(r_prior + r_eps);
		T r_func;

		if (m_RadiusFunc < 1)
			r_func = r;
		else if (m_RadiusFunc < 2)
			r_func = std::cos(r);
		else if (m_RadiusFunc < 3)
			r_func = std::exp(r);
		else if (m_RadiusFunc < 4)
			r_func = std::log(std::abs(r) + std::abs(r_eps));
		else
			r_func = 1 / Zeps(r + r_eps);

		helper.Out.x = (helper.In.x - helper.In.y) * (helper.In.x + helper.In.y) * r_func;
		helper.Out.y = 2 * helper.In.x * helper.In.y * r_func;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string lengthscalar = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radiusfunc   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string eps          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t r_prior = precalcSqrtSumSquares * " << lengthscalar << ";\n"
		   << "\t\treal_t r_sign = r_prior >= 0.0 ? 1.0 : -1.0;\n"
		   << "\t\treal_t r_eps = fabs(r_prior) > " << eps << " ? 0 : r_sign * " << eps << ";\n"
		   << "\t\treal_t r = " << weight << " / Zeps(r_prior + r_eps);\n"
		   << "\t\treal_t r_func;\n"
		   << "\n"
		   << "\t\tif (" << radiusfunc << " < 1)\n"
		   << "\t\t	r_func = r;\n"
		   << "\t\telse if (" << radiusfunc << " < 2)\n"
		   << "\t\t	r_func = cos(r);\n"
		   << "\t\telse if (" << radiusfunc << " < 3)\n"
		   << "\t\t	r_func = exp(r);\n"
		   << "\t\telse if (" << radiusfunc << " < 4)\n"
		   << "\t\t	r_func = log(fabs(r) + fabs(r_eps));\n"
		   << "\t\telse\n"
		   << "\t\t	r_func = 1 / Zeps(r + r_eps);\n"
		   << "\n"
		   << "\t\tvOut.x = (vIn.x - vIn.y) * (vIn.x + vIn.y) * r_func;\n"
		   << "\t\tvOut.y = 2 * vIn.x * vIn.y * r_func;\n"
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
		m_Params.push_back(ParamWithName<T>(&m_LengthScalar, prefix + "excinis_lengthscalar", 1));
		m_Params.push_back(ParamWithName<T>(&m_RadiusFunc,   prefix + "excinis_radiusFunc", 0, eParamType::REAL, 0, 5));
		m_Params.push_back(ParamWithName<T>(&m_Eps,          prefix + "excinis_eps", T(0.1)));
	}

private:
	T m_LengthScalar;
	T m_RadiusFunc;
	T m_Eps;
};

/// <summary>
/// vibration.
/// By FarDareisMai.
/// </summary>
template <typename T>
class VibrationVariation : public ParametricVariation<T>
{
public:
	VibrationVariation(T weight = 1.0) : ParametricVariation<T>("vibration", eVariationId::VAR_VIBRATION, weight)
	{
		Init();
	}

	PARVARCOPY(VibrationVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T d_along_dir = helper.In.x * m_CosDir + helper.In.y * m_SinDir;
		T local_amp = m_Amp * std::sin((d_along_dir * m_ScaledFreq) + m_PhaseShift);
		T x = helper.In.x + local_amp * m_CosTot;
		T y = helper.In.y + local_amp * m_SinTot;
		d_along_dir = helper.In.x * m_CosDir2 + helper.In.y * m_SinDir2;
		local_amp = m_Amp2 * std::sin((d_along_dir * m_ScaledFreq2) + m_PhaseShift2);
		x += local_amp * m_CosTot2;
		y += local_amp * m_SinTot2;
		helper.Out.x = m_Weight * x;
		helper.Out.y = m_Weight * y;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string dir         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string angle       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freq        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amp         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phase       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dir2        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string angle2      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freq2       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amp2        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phase2      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosdir      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sindir      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string costot      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sintot      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaledfreq  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phaseshift  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosdir2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sindir2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string costot2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sintot2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaledfreq2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phaseshift2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t d_along_dir = fma(vIn.x, " << cosdir << ", vIn.y * " << sindir << ");\n"
		   << "\t\treal_t local_amp = " << amp << " * sin(fma(d_along_dir, " << scaledfreq << ", " << phaseshift << "));\n"
		   << "\t\treal_t x = fma(local_amp, " << costot << ", vIn.x);\n"
		   << "\t\treal_t y = fma(local_amp, " << sintot << ", vIn.y);\n"
		   << "\t\td_along_dir = fma(vIn.x, " << cosdir2 << ", vIn.y * " << sindir2 << ");\n"
		   << "\t\tlocal_amp = " << amp2 << " * sin(fma(d_along_dir, " << scaledfreq2 << ", " << phaseshift2 << "));\n"
		   << "\t\tx += local_amp * " << costot2 << ";\n"
		   << "\t\ty += local_amp * " << sintot2 << ";\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * x;\n"
		   << "\t\tvOut.y = " << weight << " * y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T totalangle = m_Angle + m_Dir;
		m_CosDir = std::cos(m_Dir);
		m_SinDir = std::sin(m_Dir);
		m_CosTot = std::cos(totalangle);
		m_SinTot = std::sin(totalangle);
		m_ScaledFreq = m_Freq * M_2PI;
		m_PhaseShift = M_2PI * m_Phase / Zeps(m_Freq);
		T totalangle2 = m_Angle2 + m_Dir2;
		m_CosDir2 = std::cos(m_Dir2);
		m_SinDir2 = std::sin(m_Dir2);
		m_CosTot2 = std::cos(totalangle2);
		m_SinTot2 = std::sin(totalangle2);
		m_ScaledFreq2 = m_Freq2 * M_2PI;
		m_PhaseShift2 = M_2PI * m_Phase2 / Zeps(m_Freq2);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Dir,               prefix + "vibration_dir", 0, eParamType::REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Angle,             prefix + "vibration_angle", T(M_PI_2), eParamType::REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Freq,              prefix + "vibration_freq", 1));
		m_Params.push_back(ParamWithName<T>(&m_Amp,               prefix + "vibration_amp", T(0.25)));
		m_Params.push_back(ParamWithName<T>(&m_Phase,             prefix + "vibration_phase", 0, eParamType::REAL_CYCLIC, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Dir2,              prefix + "vibration_dir2", T(M_PI_2), eParamType::REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Angle2,            prefix + "vibration_angle2", T(M_PI_2), eParamType::REAL_CYCLIC, 0, M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_Freq2,             prefix + "vibration_freq2", 1));
		m_Params.push_back(ParamWithName<T>(&m_Amp2,              prefix + "vibration_amp2", T(0.25)));
		m_Params.push_back(ParamWithName<T>(&m_Phase2,            prefix + "vibration_phase2", 0, eParamType::REAL_CYCLIC, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_CosDir,      prefix + "vibration_cos_dir"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_SinDir,      prefix + "vibration_sin_dir"));
		m_Params.push_back(ParamWithName<T>(true, &m_CosTot,      prefix + "vibration_cos_tot"));
		m_Params.push_back(ParamWithName<T>(true, &m_SinTot,      prefix + "vibration_sin_tot"));
		m_Params.push_back(ParamWithName<T>(true, &m_ScaledFreq,  prefix + "vibration_scaled_freq"));
		m_Params.push_back(ParamWithName<T>(true, &m_PhaseShift,  prefix + "vibration_phase_shift"));
		m_Params.push_back(ParamWithName<T>(true, &m_CosDir2,     prefix + "vibration_cos_dir2"));
		m_Params.push_back(ParamWithName<T>(true, &m_SinDir2,     prefix + "vibration_sin_dir2"));
		m_Params.push_back(ParamWithName<T>(true, &m_CosTot2,     prefix + "vibration_cos_tot2"));
		m_Params.push_back(ParamWithName<T>(true, &m_SinTot2,     prefix + "vibration_sin_tot2"));
		m_Params.push_back(ParamWithName<T>(true, &m_ScaledFreq2, prefix + "vibration_scaled_freq2"));
		m_Params.push_back(ParamWithName<T>(true, &m_PhaseShift2, prefix + "vibration_phase_shift2"));
	}

private:
	T m_Dir;
	T m_Angle;
	T m_Freq;
	T m_Amp;
	T m_Phase;
	T m_Dir2;
	T m_Angle2;
	T m_Freq2;
	T m_Amp2;
	T m_Phase2;
	T m_CosDir;
	T m_SinDir;
	T m_CosTot;
	T m_SinTot;
	T m_ScaledFreq;
	T m_PhaseShift;
	T m_CosDir2;
	T m_SinDir2;
	T m_CosTot2;
	T m_SinTot2;
	T m_ScaledFreq2;
	T m_PhaseShift2;
};

/// <summary>
/// vibration2.
/// By FarDareisMai.
/// </summary>
template <typename T>
class Vibration2Variation : public ParametricVariation<T>
{
public:
	Vibration2Variation(T weight = 1.0) : ParametricVariation<T>("vibration2", eVariationId::VAR_VIBRATION2, weight)
	{
		Init();
	}

	PARVARCOPY(Vibration2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T d_along_dir = helper.In.x * m_CosDir + helper.In.y * m_SinDir;
		T dirL = m_Dir + VarFuncs<T>::Modulate(m_Dm, m_Dmfreq, d_along_dir);
		T angleL = m_Angle + VarFuncs<T>::Modulate(m_Tm, m_Tmfreq, d_along_dir);
		T freqL = VarFuncs<T>::Modulate(m_Fm, m_Fmfreq, d_along_dir) / Zeps(m_Freq);
		T ampL = m_Amp + m_Amp * VarFuncs<T>::Modulate(m_Am, m_Amfreq, d_along_dir);
		T total_angle = angleL + dirL;
		T cos_dir = std::cos(dirL);
		T sin_dir = std::sin(dirL);
		T cos_tot = std::cos(total_angle);
		T sin_tot = std::sin(total_angle);
		d_along_dir = helper.In.x * cos_dir + helper.In.y * sin_dir;
		T local_amp = ampL * std::sin((d_along_dir * m_ScaledFreq) + freqL + m_PhaseShift);
		T x = helper.In.x + local_amp * cos_tot;
		T y = helper.In.y + local_amp * sin_tot;
		d_along_dir = helper.In.x * m_CosDir2 + helper.In.y * m_SinDir2;
		dirL = m_Dir2 + VarFuncs<T>::Modulate(m_D2m, m_D2mfreq, d_along_dir);
		angleL = m_Angle2 + VarFuncs<T>::Modulate(m_T2m, m_T2mfreq, d_along_dir);
		freqL = VarFuncs<T>::Modulate(m_F2m, m_F2mfreq, d_along_dir) / Zeps(m_Freq2);
		ampL = m_Amp2 + m_Amp2 * VarFuncs<T>::Modulate(m_A2m, m_A2mfreq, d_along_dir);
		total_angle = angleL + dirL;
		cos_dir = std::cos(dirL);
		sin_dir = std::sin(dirL);
		cos_tot = std::cos(total_angle);
		sin_tot = std::sin(total_angle);
		d_along_dir = helper.In.x * cos_dir + helper.In.y * sin_dir;
		local_amp = ampL * std::sin((d_along_dir * m_ScaledFreq2) + freqL + m_PhaseShift2);
		x += local_amp * cos_tot;
		y += local_amp * sin_tot;
		helper.Out.x = m_Weight * x;
		helper.Out.y = m_Weight * y;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string dir         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string angle       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freq        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amp         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phase       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dir2        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string angle2      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freq2       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amp2        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phase2      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dm          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string dmfreq      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string tm          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string tmfreq      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fm          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fmfreq      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string am          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amfreq      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d2m         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string d2mfreq     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string t2m         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string t2mfreq     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string f2m         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string f2mfreq     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a2m         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string a2mfreq     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosdir      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sindir      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaledfreq  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phaseshift  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosdir2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sindir2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaledfreq2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string phaseshift2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t d_along_dir = fma(vIn.x, " << cosdir << ", vIn.y * " << sindir << ");\n"
		   << "\t\treal_t dirL = " << dir << " + Modulate(" << dm << ", " << dmfreq << ", d_along_dir);\n"
		   << "\t\treal_t angleL = " << angle << " + Modulate(" << tm << ", " << tmfreq << ", d_along_dir);\n"
		   << "\t\treal_t freqL = Modulate(" << fm << ", " << fmfreq << ", d_along_dir) / Zeps(" << freq << ");\n"
		   << "\t\treal_t ampL = fma(" << amp << ", Modulate(" << am << ", " << amfreq << ", d_along_dir), " << amp << ");\n"
		   << "\t\treal_t total_angle = angleL + dirL;\n"
		   << "\t\treal_t cos_dir = cos(dirL);\n"
		   << "\t\treal_t sin_dir = sin(dirL);\n"
		   << "\t\treal_t cos_tot = cos(total_angle);\n"
		   << "\t\treal_t sin_tot = sin(total_angle);\n"
		   << "\t\td_along_dir = fma(vIn.x, cos_dir, vIn.y * sin_dir);\n"
		   << "\t\treal_t local_amp = ampL * sin(fma(d_along_dir, " << scaledfreq << ", freqL + " << phaseshift << "));\n"
		   << "\t\treal_t x = fma(local_amp, cos_tot, vIn.x);\n"
		   << "\t\treal_t y = fma(local_amp, sin_tot, vIn.y);\n"
		   << "\t\td_along_dir = fma(vIn.x, " << cosdir2 << ", vIn.y * " << sindir2 << ");\n"
		   << "\t\tdirL = " << dir2 << " + Modulate(" << d2m << ", " << d2mfreq << ", d_along_dir);\n"
		   << "\t\tangleL = " << angle2 << " + Modulate(" << t2m << ", " << t2mfreq << ", d_along_dir);\n"
		   << "\t\tfreqL = Modulate(" << f2m << ", " << f2mfreq << ", d_along_dir) / Zeps(" << freq2 << ");\n"
		   << "\t\tampL = fma(" << amp2 << ", Modulate(" << a2m << ", " << a2mfreq << ", d_along_dir), " << amp2 << ");\n"
		   << "\t\ttotal_angle = angleL + dirL;\n"
		   << "\t\tcos_dir = cos(dirL);\n"
		   << "\t\tsin_dir = sin(dirL);\n"
		   << "\t\tcos_tot = cos(total_angle);\n"
		   << "\t\tsin_tot = sin(total_angle);\n"
		   << "\t\td_along_dir = fma(vIn.x, cos_dir, vIn.y * sin_dir);\n"
		   << "\t\tlocal_amp = ampL * sin(fma(d_along_dir, " << scaledfreq2 << ", freqL + " << phaseshift2 << "));\n"
		   << "\t\tx += local_amp * cos_tot;\n"
		   << "\t\ty += local_amp * sin_tot;\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * x;\n"
		   << "\t\tvOut.y = " << weight << " * y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_CosDir = std::cos(m_Dir);
		m_SinDir = std::sin(m_Dir);
		m_ScaledFreq = m_Freq * M_2PI;
		m_PhaseShift = M_2PI * m_Phase / Zeps(m_Freq);
		m_CosDir2 = std::cos(m_Dir2);
		m_SinDir2 = std::sin(m_Dir2);
		m_ScaledFreq2 = m_Freq2 * M_2PI;
		m_PhaseShift2 = M_2PI * m_Phase2 / Zeps(m_Freq2);
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps", "Modulate" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Dir,               prefix + "vibration2_dir", 0));
		m_Params.push_back(ParamWithName<T>(&m_Angle,             prefix + "vibration2_angle", T(M_PI_2)));
		m_Params.push_back(ParamWithName<T>(&m_Freq,              prefix + "vibration2_freq", 1));
		m_Params.push_back(ParamWithName<T>(&m_Amp,               prefix + "vibration2_amp", T(0.25)));
		m_Params.push_back(ParamWithName<T>(&m_Phase,             prefix + "vibration2_phase", 0));
		m_Params.push_back(ParamWithName<T>(&m_Dir2,              prefix + "vibration2_dir2", T(M_PI_2)));
		m_Params.push_back(ParamWithName<T>(&m_Angle2,            prefix + "vibration2_angle2", T(M_PI_2)));
		m_Params.push_back(ParamWithName<T>(&m_Freq2,             prefix + "vibration2_freq2", 1));
		m_Params.push_back(ParamWithName<T>(&m_Amp2,              prefix + "vibration2_amp2", T(0.25)));
		m_Params.push_back(ParamWithName<T>(&m_Phase2,            prefix + "vibration2_phase2", 0));
		m_Params.push_back(ParamWithName<T>(&m_Dm,                prefix + "vibration2_dm"));
		m_Params.push_back(ParamWithName<T>(&m_Dmfreq,            prefix + "vibration2_dmfreq"));
		m_Params.push_back(ParamWithName<T>(&m_Tm	,             prefix + "vibration2_tm"));
		m_Params.push_back(ParamWithName<T>(&m_Tmfreq,            prefix + "vibration2_tmfreq"));
		m_Params.push_back(ParamWithName<T>(&m_Fm	,             prefix + "vibration2_fm"));
		m_Params.push_back(ParamWithName<T>(&m_Fmfreq,            prefix + "vibration2_fmfreq"));
		m_Params.push_back(ParamWithName<T>(&m_Am	,             prefix + "vibration2_am"));
		m_Params.push_back(ParamWithName<T>(&m_Amfreq,            prefix + "vibration2_amfreq"));
		m_Params.push_back(ParamWithName<T>(&m_D2m	,             prefix + "vibration2_dm2"));
		m_Params.push_back(ParamWithName<T>(&m_D2mfreq,           prefix + "vibration2_dmfreq2"));
		m_Params.push_back(ParamWithName<T>(&m_T2m	,             prefix + "vibration2_tm2"));
		m_Params.push_back(ParamWithName<T>(&m_T2mfreq,           prefix + "vibration2_tmfreq2"));
		m_Params.push_back(ParamWithName<T>(&m_F2m	,             prefix + "vibration2_fm2"));
		m_Params.push_back(ParamWithName<T>(&m_F2mfreq,           prefix + "vibration2_fmfreq2"));
		m_Params.push_back(ParamWithName<T>(&m_A2m	,             prefix + "vibration2_am2"));
		m_Params.push_back(ParamWithName<T>(&m_A2mfreq,           prefix + "vibration2_amfreq2"));
		m_Params.push_back(ParamWithName<T>(true, &m_CosDir,      prefix + "vibration2_cos_dir"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_SinDir,      prefix + "vibration2_sin_dir"));
		m_Params.push_back(ParamWithName<T>(true, &m_ScaledFreq,  prefix + "vibration2_scaled_freq"));
		m_Params.push_back(ParamWithName<T>(true, &m_PhaseShift,  prefix + "vibration2_phase_shift"));
		m_Params.push_back(ParamWithName<T>(true, &m_CosDir2,     prefix + "vibration2_cos_dir2"));
		m_Params.push_back(ParamWithName<T>(true, &m_SinDir2,     prefix + "vibration2_sin_dir2"));
		m_Params.push_back(ParamWithName<T>(true, &m_ScaledFreq2, prefix + "vibration2_scaled_freq2"));
		m_Params.push_back(ParamWithName<T>(true, &m_PhaseShift2, prefix + "vibration2_phase_shift2"));
	}

private:
	T m_Dir;
	T m_Angle;
	T m_Freq;
	T m_Amp;
	T m_Phase;
	T m_Dir2;
	T m_Angle2;
	T m_Freq2;
	T m_Amp2;
	T m_Phase2;
	T m_Dm;
	T m_Dmfreq;
	T m_Tm;
	T m_Tmfreq;
	T m_Fm;
	T m_Fmfreq;
	T m_Am;
	T m_Amfreq;
	T m_D2m;
	T m_D2mfreq;
	T m_T2m;
	T m_T2mfreq;
	T m_F2m;
	T m_F2mfreq;
	T m_A2m;
	T m_A2mfreq;
	T m_CosDir;//Precalc.
	T m_SinDir;
	T m_ScaledFreq;
	T m_PhaseShift;
	T m_CosDir2;
	T m_SinDir2;
	T m_ScaledFreq2;
	T m_PhaseShift2;
};

/// <summary>
/// arcsech.
/// By tatasz.
/// </summary>
template <typename T>
class ArcsechVariation : public ParametricVariation<T>
{
public:
	ArcsechVariation(T weight = 1.0) : ParametricVariation<T>("arcsech", eVariationId::VAR_ARCSECH, weight)
	{
		Init();
	}

	PARVARCOPY(ArcsechVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		std::complex<T> z(helper.In.x, helper.In.y);
		z = T(1.0) / z;
		std::complex<T> result = m_WeightInvPi * std::log(z + std::sqrt(z + T(1.0)) * std::sqrt(z - T(1.0)));
		helper.Out.x = result.real();
		helper.Out.y = result.imag();
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string weightinvpi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal2 z1 = (real2)(vIn.x, vIn.y);\n"
		   << "\t\treal2 z = RealDivComplex(1.0, z1);\n"
		   << "\t\treal2 result = ComplexMultReal(ComplexLog(ComplexPlusComplex(z,ComplexMultComplex(ComplexSqrt(ComplexPlusReal(z, 1.0)), ComplexSqrt(ComplexMinusReal(z, 1.0))))), " << weightinvpi << ");\n"
		   << "\n"
		   << "\t\tvOut.x = result.x;\n"
		   << "\t\tvOut.y = result.y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_WeightInvPi = m_Weight * T(M_1_PI);
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps", "Sign", "Hypot", "ComplexMultReal", "ComplexLog", "ComplexPlusReal", "ComplexPlusComplex", "ComplexMultComplex", "ComplexSqrt", "ComplexMinusReal", "RealDivComplex" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_WeightInvPi, prefix + "arcsech_weight_inv_pi"));//Precalc.
	}

private:
	T m_WeightInvPi;//Precalc only.
};

/// <summary>
/// arcsech2.
/// By tatasz.
/// </summary>
template <typename T>
class Arcsech2Variation : public ParametricVariation<T>
{
public:
	Arcsech2Variation(T weight = 1.0) : ParametricVariation<T>("arcsech2", eVariationId::VAR_ARCSECH2, weight)
	{
		Init();
	}

	PARVARCOPY(Arcsech2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		std::complex<T> z(helper.In.x, helper.In.y);
		z = T(1.0) / z;
		std::complex<T> result = m_WeightInvPi * std::log(z + std::sqrt(z + T(1.0)) * std::sqrt(z - T(1.0)));

		if (result.imag() < 0)
		{
			helper.Out.x = result.real();
			helper.Out.y = result.imag() + T(1.0);
		}
		else
		{
			helper.Out.x = -result.real();
			helper.Out.y = result.imag() - T(1.0);
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
		string weightinvpi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal2 z1 = (real2)(vIn.x, vIn.y);\n"
		   << "\t\treal2 z = RealDivComplex(1.0, z1);\n"
		   << "\t\treal2 result = ComplexMultReal(ComplexLog(ComplexPlusComplex(z,ComplexMultComplex(ComplexSqrt(ComplexPlusReal(z, 1.0)), ComplexSqrt(ComplexMinusReal(z, 1.0))))), " << weightinvpi << ");\n"
		   << "\n"
		   << "\t\tif (result.y < 0)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = result.x;\n"
		   << "\t\t	vOut.y = result.y + 1;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = -result.x;\n"
		   << "\t\t	vOut.y = result.y - 1;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_WeightInvPi = m_Weight * T(M_1_PI) * 2;
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps", "Sign", "Hypot", "ComplexMultReal", "ComplexLog", "ComplexPlusReal", "ComplexPlusComplex", "ComplexMultComplex", "ComplexSqrt", "ComplexMinusReal", "RealDivComplex" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_WeightInvPi, prefix + "arcsech2_weight_inv_pi"));//Precalc.
	}

private:
	T m_WeightInvPi;//Precalc only.
};

/// <summary>
/// arcsinh.
/// By tatasz.
/// </summary>
template <typename T>
class ArcsinhVariation : public ParametricVariation<T>
{
public:
	ArcsinhVariation(T weight = 1.0) : ParametricVariation<T>("arcsinh", eVariationId::VAR_ARCSINH, weight)
	{
		Init();
	}

	PARVARCOPY(ArcsinhVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		std::complex<T> z(helper.In.x, helper.In.y);
		std::complex<T> result = m_WeightInvPi * std::log(z + std::sqrt(z * z + T(1.0)));
		helper.Out.x = result.real();
		helper.Out.y = result.imag();
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string weightinvpi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal2 z = (real2)(vIn.x, vIn.y);\n"
		   << "\t\treal2 result = ComplexMultReal(ComplexLog(ComplexPlusComplex(z, ComplexSqrt(ComplexPlusReal(ComplexMultComplex(z, z), 1.0)))), " << weightinvpi << ");\n"
		   << "\n"
		   << "\t\tvOut.x = result.x;\n"
		   << "\t\tvOut.y = result.y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_WeightInvPi = m_Weight * T(M_2_PI);
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Sign", "Hypot", "ComplexMultReal", "ComplexLog", "ComplexPlusReal", "ComplexPlusComplex", "ComplexMultComplex", "ComplexSqrt" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_WeightInvPi, prefix + "arcsinh_weight_inv_pi"));//Precalc.
	}

private:
	T m_WeightInvPi;//Precalc only.
};

/// <summary>
/// arctanh.
/// </summary>
template <typename T>
class ArctanhVariation : public ParametricVariation<T>
{
public:
	ArctanhVariation(T weight = 1.0) : ParametricVariation<T>("arctanh", eVariationId::VAR_ARCTANH, weight)
	{
		Init();
	}

	PARVARCOPY(ArctanhVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		std::complex<T> z(helper.In.x, helper.In.y);
		std::complex<T> result = m_WeightInvPi * std::log((z + T(1.0)) / (-z + T(1.0)));
		helper.Out.x = result.real();
		helper.Out.y = result.imag();
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string weightinvpi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal2 z = (real2)(vIn.x, vIn.y);\n"
		   << "\t\treal2 zm = (real2)(-vIn.x, -vIn.y);\n"
		   << "\t\treal2 result = ComplexMultReal(ComplexLog(ComplexDivComplex(ComplexPlusReal(z, 1.0), ComplexPlusReal(zm, 1.0))), " << weightinvpi << ");\n"
		   << "\n"
		   << "\t\tvOut.x = result.x;\n"
		   << "\t\tvOut.y = result.y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_WeightInvPi = m_Weight * T(M_1_PI);
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Zeps", "ComplexMultReal", "ComplexLog", "ComplexPlusReal", "ComplexDivComplex" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(true, &m_WeightInvPi, prefix + "arctanh_weight_inv_pi"));//Precalc.
	}

private:
	T m_WeightInvPi;//Precalc only.
};

/// <summary>
/// hex_truchet.
/// By tatasz.
/// </summary>
template <typename T>
class HexTruchetVariation : public ParametricVariation<T>
{
public:
	HexTruchetVariation(T weight = 1.0) : ParametricVariation<T>("hex_truchet", eVariationId::VAR_HEX_TRUCHET, weight)
	{
		Init();
	}

	PARVARCOPY(HexTruchetVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = rand.Frand01<T>() * M_2PI;
		T r = std::sqrt(rand.Frand01<T>() * m_MaxmMin + m_Min);
		v2T xy;

		if (angle < 2.0943)
		{
			xy = v2T(1, T(0.5773));
		}
		else
		{
			if (angle < 4.1887)
				xy = v2T(-1, T(0.5773));
			else
				xy = v2T(0, T(-1.1547));
		}

		T a = angle + T(2.6179);
		T X = 2 * (Floor<T>(rand.Frand01<T>() * m_Sidem2p1) - m_Side);
		T Y = 2 * (Floor<T>(rand.Frand01<T>() * m_Sidem2p1) - m_Side);
		T xfinal = X + T(0.5) * Y;
		T yfinal = T(0.8660) * Y;
		T random = VarFuncs<T>::HashShadertoy(X, Y, m_Seed);
		T rotation = random < T(0.5) ? 0 : T(1.0471);
		T a_final = a - rotation;
		T cosa = std::cos(rotation);
		T sina = std::sin(rotation);
		v2T xy_final(xy.x * cosa + xy.y * sina, -xy.x * sina + xy.y * cosa);
		helper.Out.x = (std::cos(a_final) * r + xy_final.x + xfinal) * m_Weight;
		helper.Out.y = (std::sin(a_final) * r + xy_final.y + yfinal) * m_Weight;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string size     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string side     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string onemsize = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mn       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string maxmmin  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sidem2p1 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t angle = MwcNext01(mwc) * M_2PI;\n"
		   << "\t\treal_t r = sqrt(fma(MwcNext01(mwc), " << maxmmin << ", " << mn << "));\n"
		   << "\t\treal2 xy;\n"
		   << "\n"
		   << "\t\tif (angle < 2.0943)\n"
		   << "\t\t{\n"
		   << "\t\t	xy = (real2)(1.0, 0.5773);\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	if (angle < 4.1887)\n"
		   << "\t\t		xy = (real2)(-1.0, 0.5773);\n"
		   << "\t\t	else\n"
		   << "\t\t		xy = (real2)(0.0, -1.1547);\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t a = angle + 2.6179;\n"
		   << "\t\treal_t X = 2.0 * (floor(MwcNext01(mwc) * " << sidem2p1 << ") - " << side << ");\n"
		   << "\t\treal_t Y = 2.0 * (floor(MwcNext01(mwc) * " << sidem2p1 << ") - " << side << ");\n"
		   << "\t\treal_t xfinal = fma(0.5, Y, X);\n"
		   << "\t\treal_t yfinal = 0.8660 * Y;\n"
		   << "\t\treal_t random = HashShadertoy(X, Y, " << seed << ");\n"
		   << "\t\treal_t rotation = random < 0.5 ? 0.0 : 1.0471;\n"
		   << "\t\treal_t a_final = a - rotation;\n"
		   << "\t\treal_t cosa = cos(rotation);\n"
		   << "\t\treal_t sina = sin(rotation);\n"
		   << "\t\treal2 xy_final = (real2)(fma(xy.x, cosa, xy.y * sina), fma(-xy.x, sina, xy.y * cosa));\n"
		   << "\n"
		   << "\t\tvOut.x = fma(cos(a_final), r, xy_final.x + xfinal) * " << weight << ";\n"
		   << "\t\tvOut.y = fma(sin(a_final), r, xy_final.y + yfinal) * " << weight << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_OnemSize = (1 - m_Size) * T(0.85);
		m_Min = Sqr(T(0.15) + m_OnemSize * T(0.5));
		T mx = Sqr(1 - m_OnemSize * T(0.5));
		m_MaxmMin = mx - m_Min;
		m_Sidem2p1 = m_Side * 2 + 1;
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Fract", "HashShadertoy" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Size,           prefix + "hex_truchet_size", 1, eParamType::REAL, -1, 1));
		m_Params.push_back(ParamWithName<T>(&m_Side,           prefix + "hex_truchet_side", 1));
		m_Params.push_back(ParamWithName<T>(&m_Seed,           prefix + "hex_truchet_seed", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_OnemSize, prefix + "hex_truchet_onemsize"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Min,      prefix + "hex_truchet_min"));
		m_Params.push_back(ParamWithName<T>(true, &m_MaxmMin,  prefix + "hex_truchet_maxmmin"));
		m_Params.push_back(ParamWithName<T>(true, &m_Sidem2p1, prefix + "hex_truchet_sidem2p1"));
	}

private:
	T m_Size;
	T m_Side;
	T m_Seed;
	T m_OnemSize;//Precalc only.
	T m_Min;
	T m_MaxmMin;
	T m_Sidem2p1;
};

/// <summary>
/// hex_rand.
/// By tatasz.
/// </summary>
template <typename T>
class HexRandVariation : public ParametricVariation<T>
{
public:
	HexRandVariation(T weight = 1.0) : ParametricVariation<T>("hex_rand", eVariationId::VAR_HEX_RAND, weight)
	{
		Init();
	}

	PARVARCOPY(HexRandVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T k = rand.Frand01<T>() * 6;
		auto int_angle = Floor<T>(k);
		T x = k - int_angle;
		T z = std::sqrt(1 + x * x - x);
		T angle_sign = T((int_angle - Floor<T>(k / 2) * 2) == 1 ? 1 : -1);
		T final_angle = T(2.0943951023931954923084289221863) * (int_angle / 2) + angle_sign * std::asin(T(0.86602540378443864676372317075294) * x / z) - T(M_PI_2);
		T X = (floor(rand.Frand01<T>() * m_X * 2) - m_X);
		T Y = (floor(rand.Frand01<T>() * m_Y * 2) - m_Y);
		T xfinal = X + T(0.5) * Y;
		T yfinal = Y * T(0.86602540378443864676372317075294);
		T N = VarFuncs<T>::HashShadertoy(yfinal, xfinal, m_Seed);

		if (N < m_Density)
		{
			T z_scaled = z * std::sqrt(rand.Frand01<T>()) * T(1.1547005383792515290182975610039);//2 / sqrt(3)
			T n = VarFuncs<T>::HashShadertoy(xfinal, yfinal, m_Seed);
			T R = m_SizeOver2 * z_scaled * std::pow(n, m_Power);
			helper.Out.x = (std::cos(final_angle) * R + xfinal) * m_Weight;
			helper.Out.y = (std::sin(final_angle) * R + yfinal) * m_Weight;
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
		string index = ss2.str();
		string weight = WeightDefineString();
		string x         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string size      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string density   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sizeover2 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t seed = " << seed << ";\n"
		   << "\t\treal_t weight = " << weight << ";\n"
		   << "\t\treal_t k = MwcNext01(mwc) * 6;\n"
		   << "\t\tint int_angle = (int)floor(k);\n"
		   << "\t\treal_t x = k - int_angle;\n"
		   << "\t\treal_t z = sqrt(1 + x * x - x);\n"
		   << "\t\treal_t angle_sign = (int_angle - floor(k / 2) * 2) == 1 ? 1.0 : -1.0;\n"
		   << "\t\treal_t final_angle = 2.0943951023931954923084289221863 * (int_angle / 2) + angle_sign * asin(0.86602540378443864676372317075294 * x / z) - MPI2;\n"
		   << "\t\treal_t X = (floor(MwcNext01(mwc) * " << x << " * 2) - " << x << ");\n"
		   << "\t\treal_t Y = (floor(MwcNext01(mwc) * " << y << " * 2) - " << y << ");\n"
		   << "\t\treal_t xfinal = fma(0.5, Y, X);\n"
		   << "\t\treal_t yfinal = Y * 0.86602540378443864676372317075294;\n"
		   << "\t\treal_t N = HashShadertoy(yfinal, xfinal, seed);\n"
		   << "\n"
		   << "\t\tif (N < " << density << ")\n"
		   << "\t\t{\n"
		   << "\t\t	real_t z_scaled = z * sqrt(MwcNext01(mwc)) * 1.1547005383792515290182975610039;\n"
		   << "\t\t	real_t n = HashShadertoy(xfinal, yfinal, seed);\n"
		   << "\t\t	real_t R = " << sizeover2 << " * z_scaled * pow(n, " << power << ");\n"
		   << "\t\t	vOut.x = fma(cos(final_angle), R, xfinal) * weight;\n"
		   << "\t\t	vOut.y = fma(sin(final_angle), R, yfinal) * weight;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = (real_t)(0.0);\n"
		   << "\t\t	vOut.y = (real_t)(0.0);\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_SizeOver2 = m_Size * T(0.5);
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Fract", "HashShadertoy" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X,               prefix + "hex_rand_x", 10));
		m_Params.push_back(ParamWithName<T>(&m_Y,               prefix + "hex_rand_y", 10));
		m_Params.push_back(ParamWithName<T>(&m_Size,            prefix + "hex_rand_size", 1));
		m_Params.push_back(ParamWithName<T>(&m_Power,           prefix + "hex_rand_power", 1));
		m_Params.push_back(ParamWithName<T>(&m_Density,         prefix + "hex_rand_density", 1, eParamType::REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Seed,            prefix + "hex_rand_seed", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_SizeOver2, prefix + "hex_rand_size_over_2"));//Precalc.
	}

private:
	T m_X;
	T m_Y;
	T m_Size;
	T m_Power;
	T m_Density;
	T m_Seed;
	T m_SizeOver2;//Precalc.
};

/// <summary>
/// smartshape.
/// By Zy0rg.
/// </summary>
template <typename T>
class SmartshapeVariation : public ParametricVariation<T>
{
public:
	SmartshapeVariation(T weight = 1.0) : ParametricVariation<T>("smartshape", eVariationId::VAR_SMARTSHAPE, weight, true, true, false, false, true)
	{
		Init();
	}

	PARVARCOPY(SmartshapeVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T dang = (helper.m_PrecalcAtanyx + T(M_PI)) / m_Alpha;
		T rad = helper.m_PrecalcSqrtSumSquares;
		T zang1 = T(Floor<T>(dang));
		T xang1 = dang - zang1;
		T xang2 = xang1 > 0.5 ? 1 - xang1 : xang1;
		T zang = xang1 > 0.5 ? zang1 + 1 : zang1;
		T sign = T(xang1 > 0.5 ? -1 : 1);
		T xang;

		if (m_Comp == 1 && m_Distortion >= 1)
			xang = std::atan(xang2 * m_AlphaCoeff) / m_Alpha;
		else
			xang = xang2;

		T coeff_1;

		if (m_Distortion == 0)
		{
			coeff_1 = 1;
		}
		else
		{
			T coeff0 = 1 / std::cos(xang * m_Alpha);

			if (m_Roundstr != 0)
			{
				T wwidth;

				if (m_Roundwidth != 1)
					wwidth = std::exp(std::log(xang * 2) * m_Roundwidth) * m_RoundCoeff;
				else
					wwidth = xang * 2 * m_RoundCoeff;

				coeff_1 = std::abs((1 - wwidth) * coeff0 + wwidth);
			}
			else
				coeff_1 = coeff0;
		}

		T coeff;

		//for negative distortion and small values of coeff_1
		//this expression is numerically unstable
		//double precision strongly recommended
		if (m_Distortion != 1)
			coeff = std::exp(std::log(coeff_1) * m_Distortion);
		else
			coeff = coeff_1;

		T ang = (zang + sign * xang) * m_Alpha - T(M_PI);
		T temp = m_Weight * coeff * rad;
		helper.Out.x = std::cos(ang) * temp;
		helper.Out.y = std::sin(ang) * temp;
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
		string roundstr     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string roundwidth   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string distortion   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string compensation = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string alpha        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string alphacoeff   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string roundcoeff   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string comp         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t dang = (precalcAtanyx + MPI) / " << alpha << ";\n"
		   << "\t\treal_t rad = precalcSqrtSumSquares;\n"
		   << "\t\treal_t zang1 = floor(dang);\n"
		   << "\t\treal_t xang1 = dang - zang1;\n"
		   << "\t\treal_t xang2 = xang1 > 0.5 ? 1.0 - xang1 : xang1;\n"
		   << "\t\treal_t zang = xang1 > 0.5 ? zang1 + 1.0 : zang1;\n"
		   << "\t\treal_t sign = xang1 > 0.5 ? -1.0 : 1.0;\n"
		   << "\t\treal_t xang;\n"
		   << "\n"
		   << "\t\tif (" << comp << " == 1.0 && " << distortion << " >= 1.0)\n"
		   << "\t\t	xang = atan(xang2 * " << alphacoeff << ") / " << alpha << ";\n"
		   << "\t\telse\n"
		   << "\t\t	xang = xang2;\n"
		   << "\n"
		   << "\t\treal_t coeff_1;\n"
		   << "\n"
		   << "\t\tif (" << distortion << " == 0.0)\n"
		   << "\t\t{\n"
		   << "\t\t	coeff_1 = 1.0;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real_t coeff0 = 1.0 / cos(xang * " << alpha << ");\n"
		   << "\n"
		   << "\t\t	if (" << roundstr << " != 0.0)\n"
		   << "\t\t	{\n"
		   << "\t\t		real_t wwidth;\n"
		   << "\t\t		if (" << roundwidth << " != 1.0)\n"
		   << "\t\t			wwidth = exp(log(xang * 2) * " << roundwidth << ") * " << roundcoeff << ";\n"
		   << "\t\t		else\n"
		   << "\t\t			wwidth = xang * 2 * " << roundcoeff << ";\n"
		   << "\n"
		   << "\t\t		coeff_1 = fabs(fma(1.0 - wwidth, coeff0, wwidth));\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t		coeff_1 = coeff0;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t coeff;\n"
		   << "\n"
		   << "\t\tif (" << distortion << " != 1.0)\n"
		   << "\t\t	coeff = exp(log(coeff_1) * " << distortion << ");\n"
		   << "\t\telse\n"
		   << "\t\t	coeff = coeff_1;\n"
		   << "\n"
		   << "\t\treal_t ang = fma(sign, xang, zang) * " << alpha << " - MPI;\n"
		   << "\t\treal_t temp = " << weight << " * coeff * rad;\n"
		   << "\t\tvOut.x = cos(ang) * temp;\n"
		   << "\t\tvOut.y = sin(ang) * temp;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Power = std::max(m_Power, T(2));
		m_Alpha = T(M_2PI) / m_Power;
		m_AlphaCoeff = std::tan(m_Alpha * T(0.5)) * 2;
		m_RoundCoeff = m_Roundstr / std::sin(m_Alpha * T(0.5)) / m_Power * 2;
		m_Comp = m_Compensation <= 0 ? T(0) : T(1);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Power,            prefix + "smartshape_power", 4, eParamType::REAL, 2));
		m_Params.push_back(ParamWithName<T>(&m_Roundstr,         prefix + "smartshape_roundstr"));
		m_Params.push_back(ParamWithName<T>(&m_Roundwidth,       prefix + "smartshape_roundwidth", 1));
		m_Params.push_back(ParamWithName<T>(&m_Distortion,       prefix + "smartshape_distortion", 1));
		m_Params.push_back(ParamWithName<T>(&m_Compensation,     prefix + "smartshape_compensation", 0, eParamType::REAL, 0, 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Alpha,      prefix + "smartshape_alpha"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_AlphaCoeff, prefix + "smartshape_alphacoeff"));
		m_Params.push_back(ParamWithName<T>(true, &m_RoundCoeff, prefix + "smartshape_roundcoeff"));
		m_Params.push_back(ParamWithName<T>(true, &m_Comp,       prefix + "smartshape_comp"));
	}

private:
	T m_Power;
	T m_Roundstr;
	T m_Roundwidth;
	T m_Distortion;
	T m_Compensation;
	T m_Alpha;//Precalc.
	T m_AlphaCoeff;
	T m_RoundCoeff;
	T m_Comp;
};

/// <summary>
/// squares.
/// By tatasz.
/// </summary>
template <typename T>
class SquaresVariation : public ParametricVariation<T>
{
public:
	SquaresVariation(T weight = 1.0) : ParametricVariation<T>("squares", eVariationId::VAR_SQUARES, weight)
	{
		Init();
	}

	PARVARCOPY(SquaresVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (m_CellSize == 0)
		{
			helper.Out.x = helper.In.x * m_Weight;
			helper.Out.y = helper.In.y * m_Weight;
		}
		else
		{
			T Cx = (Floor<T>(helper.In.x / m_CellSize) + T(0.5)) * m_CellSize;
			T Cy = (Floor<T>(helper.In.y / m_CellSize) + T(0.5)) * m_CellSize;
			T Lx = helper.In.x - Cx;
			T Ly = helper.In.y - Cy;
			T aLx = std::abs(T(Floor<T>(Lx * m_Num * 2 / m_CellSize + T(0.5))));
			T aLy = std::abs(T(Floor<T>(Ly * m_Num * 2 / m_CellSize + T(0.5))));
			T m = std::max(aLx, aLy);
			T maxi = m_Max + Floor<T>(VarFuncs<T>::HashShadertoy(SQR(Cx), SQR(Cy), m_Seed) * (m_Num - m_Max));
			T level;

			if (m > maxi)
				level = 0;
			else if (m < m_Min)
				level = 0;
			else
				level = m - m_Min;

			T angle;

			if (VarFuncs<T>::HashShadertoy(SQR(Cx), SQR(Cy), m_ZeroSeed) < m_Zero)
				angle = 0;
			else
				angle = level * T(M_PI_2);

			T c = std::cos(angle);
			T s = std::sin(angle);
			helper.Out.x = (Cx + Lx * c - Ly * s) * m_Weight;
			helper.Out.y = (Cy + Lx * s + Ly * c) * m_Weight;
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
		string cellsize = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mn       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mx       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string num      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string zero     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string zeroseed = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tif (" << cellsize << " == 0)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = vIn.x * " << weight << ";\n"
		   << "\t\t	vOut.y = vIn.y * " << weight << ";\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real_t Cx = (floor(vIn.x / " << cellsize << ") + 0.5) * " << cellsize << ";\n"
		   << "\t\t	real_t Cy = (floor(vIn.y / " << cellsize << ") + 0.5) * " << cellsize << ";\n"
		   << "\t\t	real_t Lx = vIn.x - Cx;\n"
		   << "\t\t	real_t Ly = vIn.y - Cy;\n"
		   << "\t\t	real_t aLx = fabs(floor(Lx * " << num << " * 2.0 / " << cellsize << " + 0.5));\n"
		   << "\t\t	real_t aLy = fabs(floor(Ly * " << num << " * 2.0 / " << cellsize << " + 0.5));\n"
		   << "\t\t	real_t m = max(aLx, aLy);\n"
		   << "\t\t	real_t maxi = " << mx << " + floor(HashShadertoy(SQR(Cx), SQR(Cy), " << seed << ") * (" << num << " - " << mx << "));\n"
		   << "\t\t	real_t level;\n"
		   << "\n"
		   << "\t\t	if (m > maxi)\n"
		   << "\t\t		level = 0.0;\n"
		   << "\t\t	else if (m < " << mn << ")\n"
		   << "\t\t		level = 0.0;\n"
		   << "\t\t	else\n"
		   << "\t\t		level = m - " << mn << ";\n"
		   << "\n"
		   << "\t\t	real_t angle;\n"
		   << "\n"
		   << "\t\t	if (HashShadertoy(SQR(Cx), SQR(Cy), " << zeroseed << ") < " << zero << ")\n"
		   << "\t\t		angle = 0.0;\n"
		   << "\t\t	else\n"
		   << "\t\t		angle = level * MPI2;\n"
		   << "\n"
		   << "\t\t	real_t c = cos(angle);\n"
		   << "\t\t	real_t s = sin(angle);\n"
		   << "\n"
		   << "\t\t	vOut.x = (Cx + Lx * c - Ly * s) * " << weight << ";\n"
		   << "\t\t	vOut.y = (Cy + Lx * s + Ly * c) * " << weight << ";\n"
		   << "\t\t}\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Fract", "HashShadertoy" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_CellSize, prefix + "squares_cellsize"));
		m_Params.push_back(ParamWithName<T>(&m_Min,      prefix + "squares_min", 0, eParamType::REAL, 0));
		m_Params.push_back(ParamWithName<T>(&m_Max,      prefix + "squares_max", 10, eParamType::REAL, 0));
		m_Params.push_back(ParamWithName<T>(&m_Num,      prefix + "squares_num", 10));
		m_Params.push_back(ParamWithName<T>(&m_Seed,     prefix + "squares_seed", 1));
		m_Params.push_back(ParamWithName<T>(&m_Zero,     prefix + "squares_zero", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_ZeroSeed, prefix + "squares_zero_seed", 1));
	}

private:
	T m_CellSize;
	T m_Min;
	T m_Max;
	T m_Num;
	T m_Seed;
	T m_Zero;
	T m_ZeroSeed;
};

/// <summary>
/// starblur2.
/// By Zy0rg.
/// </summary>
template <typename T>
class Starblur2Variation : public ParametricVariation<T>
{
public:
	Starblur2Variation(T weight = 1.0) : ParametricVariation<T>("starblur2", eVariationId::VAR_STARBLUR2, weight)
	{
		Init();
	}

	PARVARCOPY(Starblur2Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T k = rand.Frand01<T>() * m_Power2;
		auto int_angle = Floor<T>(k);
		T f = k - int_angle;
		T ff;

		if (m_StripesAbs > 0)
		{
			T fs = std::trunc(f * m_StripesAbs);
			T dif;

			if (std::fmod(int_angle, T(2)) == 0)
				dif = m_Width * (f * m_StripesAbs - fs);
			else
				dif = 1 - m_Width * (f * m_StripesAbs - fs);

			ff = (dif + fs) / m_StripesAbs;
		}
		else
			ff = f;

		T x = ff * m_Length;
		T z = Zeps(std::sqrt(1 + x * x - x * m_CosAlpha2));
		T angle_sign = (int_angle - Floor<T>(k / 2) * 2) == 1 ? T(1) : T(-1);
		T final_angle = m_TwopiPower * (int_angle / 2) + angle_sign * std::asin(m_SinAlpha * x / z) - T(M_PI_2);
		T z_scaled = m_Weight * (z * std::sqrt(rand.Frand01<T>() * m_OnemHoleSq + m_HoleSq));
		helper.Out.x = std::cos(final_angle) * z_scaled;
		helper.Out.y = std::sin(final_angle) * z_scaled;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string power      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string range      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string hole       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string stripes    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string width      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string alpha      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string length     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string holesq     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string onemholesq = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string twopipower = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sinalpha   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cosalpha2  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string stripesabs = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t k = MwcNext01(mwc) * " << power2 << ";\n"
		   << "\t\tint int_angle = (int)floor(k);\n"
		   << "\t\treal_t f = k - int_angle;\n"
		   << "\t\treal_t stripes = " << stripesabs << ";\n"
		   << "\t\treal_t ff;\n"
		   << "\n"
		   << "\t\tif (stripes > (real_t)(0.0))\n"
		   << "\t\t{\n"
		   << "\t\t	real_t fs = trunc(f * stripes);\n"
		   << "\t\t	real_t dif;\n"
		   << "\n"
		   << "\t\t	if (fmod(int_angle, (real_t)(2.0)) == 0)\n"
		   << "\t\t		dif = " << width << " * fma(f, stripes, -fs);\n"
		   << "\t\t	else\n"
		   << "\t\t		dif = 1 - " << width << " * fma(f, stripes, -fs);\n"
		   << "\n"
		   << "\t\t	ff = (dif + fs) / stripes;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t	ff = f;\n"
		   << "\n"
		   << "\t\treal_t x = ff * " << length << ";\n"
		   << "\t\treal_t z = Zeps(sqrt(1 + x * x - x * " << cosalpha2 << "));\n"
		   << "\t\treal_t angle_sign = (int_angle - floor(k/2) * 2) == (real_t)(1.0) ? (real_t)(1.0) : (real_t)(-1.0);\n"
		   << "\t\treal_t final_angle = " << twopipower << " * (int_angle / 2) + angle_sign * asin(" << sinalpha << " * x / z) - MPI2;\n"
		   << "\t\treal_t z_scaled = " << weight << " * (z * sqrt(fma(MwcNext01(mwc), " << onemholesq << ", " << holesq << ")));\n"
		   << "\t\tvOut.x = cos(final_angle) * z_scaled;\n"
		   << "\t\tvOut.y = sin(final_angle) * z_scaled;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T p = Zeps(m_Power);
		T alpha = T(M_PI) / p;
		m_Length = std::sqrt(1 + m_Range * m_Range - 2 * m_Range * std::cos(alpha));
		m_Alpha = std::asin(std::sin(alpha) * m_Range / Zeps(m_Length));
		T holeAbs = std::min(std::abs(m_Hole), T(1));
		m_StripesAbs = Zeps(std::abs(m_Stripes));
		m_HoleSq = SQR(holeAbs);
		m_OnemHoleSq = 1 - m_HoleSq;
		m_TwopiPower = (M_2PI / p);
		m_Power2 = m_Power * 2;
		m_SinAlpha = std::sin(m_Alpha);
		m_CosAlpha2 = std::cos(m_Alpha) * 2;
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
		m_Params.push_back(ParamWithName<T>(&m_Power,            prefix + "starblur2_power", 5));
		m_Params.push_back(ParamWithName<T>(&m_Range,            prefix + "starblur2_range", T(0.40162283177245455973959534526548)));
		m_Params.push_back(ParamWithName<T>(&m_Hole,             prefix + "starblur2_hole", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Stripes,          prefix + "starblur2_stripes", 5));
		m_Params.push_back(ParamWithName<T>(&m_Width,            prefix + "starblur2_width", T(0.2)));
		m_Params.push_back(ParamWithName<T>(true, &m_Alpha,      prefix + "starblur2_alpha"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Length,     prefix + "starblur2_length"));
		m_Params.push_back(ParamWithName<T>(true, &m_HoleSq,     prefix + "starblur2_holesq"));
		m_Params.push_back(ParamWithName<T>(true, &m_OnemHoleSq, prefix + "starblur2_onem_holesq"));
		m_Params.push_back(ParamWithName<T>(true, &m_TwopiPower, prefix + "starblur2_twopi_power"));
		m_Params.push_back(ParamWithName<T>(true, &m_Power2,     prefix + "starblur2_power2"));
		m_Params.push_back(ParamWithName<T>(true, &m_SinAlpha,   prefix + "starblur2_sin_alpha"));
		m_Params.push_back(ParamWithName<T>(true, &m_CosAlpha2,  prefix + "starblur2_cos_alpha2"));
		m_Params.push_back(ParamWithName<T>(true, &m_StripesAbs, prefix + "starblur2_stripes_abs"));
	}

private:
	T m_Power;
	T m_Range;
	T m_Hole;
	T m_Stripes;
	T m_Width;
	T m_Alpha;//Precalc.
	T m_Length;
	T m_HoleSq;
	T m_OnemHoleSq;
	T m_TwopiPower;
	T m_Power2;
	T m_SinAlpha;
	T m_CosAlpha2;
	T m_StripesAbs;
};

/// <summary>
/// unicorngaloshen.
/// By tatasz and chaosfissure.
/// </summary>
template <typename T>
class UnicornGaloshenVariation : public ParametricVariation<T>
{
public:
	UnicornGaloshenVariation(T weight = 1.0) : ParametricVariation<T>("unicorngaloshen", eVariationId::VAR_UNICORNGALOSHEN, weight)
	{
		Init();
	}

	PARVARCOPY(UnicornGaloshenVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T r = m_Weight / Zeps(std::sqrt((SQR(helper.In.y) * m_MultY) + (m_MultX * SQR(helper.In.x))));
		v2T factor;

		if (m_Sine <= 0)
		{
			factor.x = m_SinXAmp;
			factor.y = m_SinYAmp;
		}
		else
		{
			v2T tmp_vec(1, 2);
			v2T scalar_xy;

			if (m_Mode <= 0)
				scalar_xy = v2T(helper.In.x, helper.In.y);
			else if (m_Mode <= 1)
				scalar_xy = v2T(std::exp(-helper.In.x), std::exp(-helper.In.y));
			else if (m_Mode <= 2)
				scalar_xy = v2T(1 / Zeps(helper.In.x), 1 / Zeps(helper.In.y));
			else if (m_Mode <= 3)
				scalar_xy = v2T(std::log(std::abs(helper.In.x)), m_SinYAmp * std::log(std::abs(helper.In.y)));
			else
				scalar_xy = v2T(std::atan2(helper.In.x, helper.In.y), std::atan2(helper.In.y, helper.In.x));

			factor = tmp_vec + v2T(m_SinXAmp * std::sin(scalar_xy.x * m_SinXFreqPi), m_SinYAmp * std::sin(scalar_xy.y * m_SinYFreqPi));
		}

		helper.Out.x = factor.x * (helper.In.x - helper.In.y) * (helper.In.x + m_MultY * helper.In.y) * r;
		helper.Out.y = factor.y * helper.In.x * helper.In.y * r;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string multx      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string multy      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sine       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sinxamp    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sinxfreq   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sinyamp    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sinyfreq   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mode       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sinxfreqpi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sinyfreqpi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t mode = " << mode << ";\n"
		   << "\t\treal_t r = " << weight << " / Zeps(sqrt((SQR(vIn.y) * " << multy << ") + (" << multx << " * SQR(vIn.x))));\n"
		   << "\t\treal2 factor;\n"
		   << "\n"
		   << "\t\tif (" << sine << " <= 0)\n"
		   << "\t\t{\n"
		   << "\t\t	factor.x = " << sinxamp << ";\n"
		   << "\t\t	factor.y = " << sinyamp << ";\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real2 tmp_vec = (real2)(1, 2);\n"
		   << "\t\t	real2 scalar_xy;\n"
		   << "\n"
		   << "\t\t	if (mode <= 0)\n"
		   << "\t\t		scalar_xy = (real2)(vIn.x, vIn.y);\n"
		   << "\t\t	else if (mode <= 1)\n"
		   << "\t\t		scalar_xy = (real2)(exp(-vIn.x), exp(-vIn.y));\n"
		   << "\t\t	else if (mode <= 2)\n"
		   << "\t\t		scalar_xy = (real2)(1 / Zeps(vIn.x), 1 / Zeps(vIn.y));\n"
		   << "\t\t	else if (mode <= 3)\n"
		   << "\t\t		scalar_xy = (real2)(log(fabs(vIn.x)), " << sinyamp << " * log(fabs(vIn.y)));\n"
		   << "\t\t	else\n"
		   << "\t\t		scalar_xy = (real2)(atan2(vIn.x, vIn.y), atan2(vIn.y, vIn.x));\n"
		   << "\n"
		   << "\t\t	factor = tmp_vec + (real2)(" << sinxamp << " * sin(scalar_xy.x * " << sinxfreqpi << "), " << sinyamp << " * sin(scalar_xy.y * " << sinyfreqpi << "));\n"
		   << "\t\t}\n"
		   << "\t\tvOut.x = factor.x * (vIn.x - vIn.y) * fma(" << multy << ", vIn.y, vIn.x) * r;\n"
		   << "\t\tvOut.y = factor.y * vIn.x * vIn.y * r;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_SinXFreqPi = T(M_PI) * m_SinXFreq;
		m_SinYFreqPi = T(M_PI) * m_SinYFreq;
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
		m_Params.push_back(ParamWithName<T>(&m_MultX,            prefix + "unicorngaloshen_mult_x", 1));
		m_Params.push_back(ParamWithName<T>(&m_MultY,            prefix + "unicorngaloshen_mult_y", 1));
		m_Params.push_back(ParamWithName<T>(&m_Sine,             prefix + "unicorngaloshen_sine"));
		m_Params.push_back(ParamWithName<T>(&m_SinXAmp,          prefix + "unicorngaloshen_sin_x_amplitude", 1));
		m_Params.push_back(ParamWithName<T>(&m_SinXFreq,         prefix + "unicorngaloshen_sin_x_freq", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_SinYAmp,          prefix + "unicorngaloshen_sin_y_amplitude", 2));
		m_Params.push_back(ParamWithName<T>(&m_SinYFreq,         prefix + "unicorngaloshen_sin_y_freq", T(0.2)));
		m_Params.push_back(ParamWithName<T>(&m_Mode,             prefix + "unicorngaloshen_mode", 0, eParamType::REAL, 0, 4));
		m_Params.push_back(ParamWithName<T>(true, &m_SinXFreqPi, prefix + "unicorngaloshen_sin_x_freq_pi"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_SinYFreqPi, prefix + "unicorngaloshen_sin_y_freq_pi"));
	}

private:
	T m_MultX;
	T m_MultY;
	T m_Sine;
	T m_SinXAmp;
	T m_SinXFreq;
	T m_SinYAmp;
	T m_SinYFreq;
	T m_Mode;
	T m_SinXFreqPi;//Precalc.
	T m_SinYFreqPi;
};

/// <summary>
/// dragonfire.
/// By tatasz and chaosfissure.
/// </summary>
template <typename T>
class DragonfireVariation : public ParametricVariation<T>
{
public:
	DragonfireVariation(T weight = 1.0) : ParametricVariation<T>("dragonfire", eVariationId::VAR_DRAGONFIRE, weight)
	{
		Init();
	}

	PARVARCOPY(DragonfireVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T exp_x = std::exp(helper.In.x) * m_C1;
		T exp_nx = m_C2 / Zeps(exp_x);
		T cos_y = std::cos(helper.In.y);
		T sin_y = std::sin(helper.In.y);
		T r = m_Weight / Zeps(exp_x + exp_nx - cos_y);
		T newFX = (exp_x - exp_nx) * r;
		T newFY = sin_y * r;

		if (m_FuzzyMode < 1)
		{
			helper.Out.x = newFX;
			helper.Out.y = newFY;
		}
		else
		{
			T fuzzyX = std::cos(m_LogScalarFuzzy / Zeps(std::log(std::abs(newFX))));
			T fuzzyY = std::cos(m_LogScalarFuzzy / Zeps(std::log(std::abs(newFY))));
			v2T newXY;

			if (m_FuzzyMode < 2)
			{
				newXY.x = newFX + fuzzyX;
				newXY.y = newFY + fuzzyY;
			}
			else if (m_FuzzyMode < 3)
			{
				T sinfx = std::sin(fuzzyX);//Duped in 2 and 3.
				T sinfy = std::sin(fuzzyY);
				newXY.x = newFX * sinfx;
				newXY.y = newFY * sinfy;
			}
			else if (m_FuzzyMode < 4)
			{
				T sinfx = std::sin(fuzzyX);
				T sinfy = std::sin(fuzzyY);
				newXY.x = newFX * std::exp(sinfx);
				newXY.y = newFY * std::exp(sinfy);
			}
			else if (m_FuzzyMode < 5)
			{
				T dirX = T(fuzzyX < 0 ? -1 : 1);//Duped in 4, 5, and 6.
				T dirY = T(fuzzyY < 0 ? -1 : 1);
				T fuzzyX1 = dirX * m_FuzzyRadius;
				T fuzzyY1 = dirY * m_FuzzyRadius;
				newXY.x = newFX + fuzzyX1;
				newXY.y = newFY + fuzzyY1;
			}
			else if (m_FuzzyMode < 6)
			{
				T dirX = T(fuzzyX < 0 ? -1 : 1);
				T dirY = T(fuzzyY < 0 ? -1 : 1);
				T fuzzyX1 = dirX * m_FuzzyRadius;
				T fuzzyY1 = dirY * m_FuzzyRadius;
				newXY.x = newFX + std::exp(fuzzyX1);
				newXY.y = newFY + std::exp(fuzzyY1);
			}
			else
			{
				T dirX = T(fuzzyX < 0 ? -1 : 1);
				T dirY = T(fuzzyY < 0 ? -1 : 1);
				T fuzzyX1 = dirX * m_FuzzyRadius;
				T fuzzyY1 = dirY * m_FuzzyRadius;
				newXY.x = newFX + std::exp(std::sin(fuzzyX1));
				newXY.y = newFY + std::exp(std::cos(fuzzyY1));
			}

			if (m_FuzzyInterpolationMode < 1)
			{
				helper.Out.x = newXY.x;
				helper.Out.y = newXY.y;
			}
			else
			{
				helper.Out.x = newFX * m_1mVal + newXY.x * m_Val;
				helper.Out.y = newFY * m_1mVal + newXY.y * m_Val;
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
		string c1             = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string c2             = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string mode           = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string radius         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string interpmode     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string interppos      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string logscalar      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string adjustedscalar = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string val            = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string onemval        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t exp_x = exp(vIn.x) * " << c1 << ";\n"
		   << "\t\treal_t exp_nx = " << c2 << " / Zeps(exp_x);\n"
		   << "\t\treal_t cos_y = cos(vIn.y);\n"
		   << "\t\treal_t sin_y = sin(vIn.y);\n"
		   << "\t\treal_t r = " << weight << " / Zeps(exp_x + exp_nx - cos_y);\n"
		   << "\t\treal_t newFX = (exp_x - exp_nx) * r;\n"
		   << "\t\treal_t newFY = sin_y * r;\n"
		   << "\n"
		   << "\t\tif (" << mode << " < 1)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = newFX;\n"
		   << "\t\t	vOut.y = newFY;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real_t fuzzyX = cos(" << logscalar << " / Zeps(log(fabs(newFX))));\n"
		   << "\t\t	real_t fuzzyY = cos(" << logscalar << " / Zeps(log(fabs(newFY))));\n"
		   << "\t\t	real2 newXY;\n"
		   << "\n"
		   << "\t\t	if (" << mode << " < 2)\n"
		   << "\t\t	{\n"
		   << "\t\t		newXY.x = newFX + fuzzyX;\n"
		   << "\t\t		newXY.y = newFY + fuzzyY;\n"
		   << "\t\t	}\n"
		   << "\t\t	else if (" << mode << " < 3)\n"
		   << "\t\t	{\n"
		   << "\t\t		real_t sinfx = sin(fuzzyX);\n"
		   << "\t\t		real_t sinfy = sin(fuzzyY);\n"
		   << "\t\t		newXY.x = newFX * sinfx;\n"
		   << "\t\t		newXY.y = newFY * sinfy;\n"
		   << "\t\t	}\n"
		   << "\t\t	else if (" << mode << " < 4)\n"
		   << "\t\t	{\n"
		   << "\t\t		real_t sinfx = sin(fuzzyX);\n"
		   << "\t\t		real_t sinfy = sin(fuzzyY);\n"
		   << "\t\t		newXY.x = newFX * exp(sinfx);\n"
		   << "\t\t		newXY.y = newFY * exp(sinfy);\n"
		   << "\t\t	}\n"
		   << "\t\t	else if (" << mode << " < 5)\n"
		   << "\t\t	{\n"
		   << "\t\t		real_t dirX = fuzzyX < 0.0 ? -1.0 : 1.0;\n"
		   << "\t\t		real_t dirY = fuzzyY < 0.0 ? -1.0 : 1.0;\n"
		   << "\t\t		real_t fuzzyX1 = dirX * " << radius << ";\n"
		   << "\t\t		real_t fuzzyY1 = dirY * " << radius << ";\n"
		   << "\t\t		newXY.x = newFX + fuzzyX1;\n"
		   << "\t\t		newXY.y = newFY + fuzzyY1;\n"
		   << "\t\t	}\n"
		   << "\t\t	else if (" << mode << " < 6)\n"
		   << "\t\t	{\n"
		   << "\t\t		real_t dirX = fuzzyX < 0.0 ? -1.0 : 1.0;\n"
		   << "\t\t		real_t dirY = fuzzyY < 0.0 ? -1.0 : 1.0;\n"
		   << "\t\t		real_t fuzzyX1 = dirX * " << radius << ";\n"
		   << "\t\t		real_t fuzzyY1 = dirY * " << radius << ";\n"
		   << "\t\t		newXY.x = newFX + exp(fuzzyX1);\n"
		   << "\t\t		newXY.y = newFY + exp(fuzzyY1);\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		real_t dirX = fuzzyX < 0.0 ? -1.0 : 1.0;\n"
		   << "\t\t		real_t dirY = fuzzyY < 0.0 ? -1.0 : 1.0;\n"
		   << "\t\t		real_t fuzzyX1 = dirX * " << radius << ";\n"
		   << "\t\t		real_t fuzzyY1 = dirY * " << radius << ";\n"
		   << "\t\t		newXY.x = newFX + exp(sin(fuzzyX1));\n"
		   << "\t\t		newXY.y = newFY + exp(cos(fuzzyY1));\n"
		   << "\t\t	}\n"
		   << "\n"
		   << "\t\t	if (" << interpmode << " < 1)\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = newXY.x;\n"
		   << "\t\t		vOut.y = newXY.y;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = fma(newFX, " << onemval << ", newXY.x * " << val << ");\n"
		   << "\t\t		vOut.y = fma(newFY, " << onemval << ", newXY.y * " << val << ");\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		T pos_mpi = m_FuzzyInterpolationPosition * T(M_PI);

		if (m_FuzzyInterpolationMode < 2)
		{
			m_Val = m_FuzzyInterpolationPosition;
		}
		else
		{
			T trigPos;

			if (m_FuzzyInterpolationMode < 3)
				trigPos = std::cos(pos_mpi);
			else if (m_FuzzyInterpolationMode < 4)
				trigPos = std::tan(pos_mpi);
			else if (m_FuzzyInterpolationMode < 5)
				trigPos = std::sinh(pos_mpi);
			else
				trigPos = std::asinh(pos_mpi);

			m_Val = (1 - trigPos) * m_AdjustedFuzzyScalar;
		}

		m_1mVal = 1 - m_Val;
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
		m_Params.push_back(ParamWithName<T>(&m_C1,                         prefix + "dragonfire_c1", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_C2,                         prefix + "dragonfire_c2", T(0.25)));
		m_Params.push_back(ParamWithName<T>(&m_FuzzyMode,                  prefix + "dragonfire_fuzzyMode", 0, eParamType::REAL, 0, 7));
		m_Params.push_back(ParamWithName<T>(&m_FuzzyRadius,                prefix + "dragonfire_fuzzyRadius", 1));
		m_Params.push_back(ParamWithName<T>(&m_FuzzyInterpolationMode,     prefix + "dragonfire_fuzzyInterpolationMode", 0, eParamType::REAL, 0, 6));
		m_Params.push_back(ParamWithName<T>(&m_FuzzyInterpolationPosition, prefix + "dragonfire_fuzzyInterpolationPosition", 1));
		m_Params.push_back(ParamWithName<T>(&m_LogScalarFuzzy,             prefix + "dragonfire_logScalarFuzzy", M_2PI));
		m_Params.push_back(ParamWithName<T>(&m_AdjustedFuzzyScalar,        prefix + "dragonfire_adjustedFuzzyScalar", T(0.5)));
		m_Params.push_back(ParamWithName<T>(true, &m_Val,                  prefix + "dragonfire_val"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_1mVal,                prefix + "dragonfire_1m_val"));
	}

private:
	T m_C1;
	T m_C2;
	T m_FuzzyMode;
	T m_FuzzyRadius;
	T m_FuzzyInterpolationMode;
	T m_FuzzyInterpolationPosition;
	T m_LogScalarFuzzy;
	T m_AdjustedFuzzyScalar;
	T m_Val;//Precalc.
	T m_1mVal;
};

/// <summary>
/// truchet_glyph.
/// By tatasz and tyrantwave.
/// </summary>
template <typename T>
class TruchetGlyphVariation : public ParametricVariation<T>
{
public:
	TruchetGlyphVariation(T weight = 1.0) : ParametricVariation<T>("Truchet_glyph", eVariationId::VAR_TRUCHET_GLYPH, weight)
	{
		Init();
	}

	PARVARCOPY(TruchetGlyphVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x1 = helper.In.x * m_Scale;
		T y1 = helper.In.y * m_Scale;
		T intx = T(Floor<T>(x1 + T(0.5)));
		T inty = T(Floor<T>(y1 + T(0.5)));
		T r_x = x1 - intx;
		T x = r_x < 0 ? 1 + r_x : r_x;
		T r_y = y1 - inty;
		T y = r_y < 0 ? 1 + r_y : r_y;
		T tiletype;

		if (m_Seed == 0)
		{
			tiletype = 0;
		}
		else if (m_Seed == 1)
		{
			tiletype = 1;
		}
		else
		{
			T xrand = T(Floor<T>(std::abs(helper.In.x) + T(0.5)));
			T yrand = T(Floor<T>(std::abs(helper.In.y) + T(0.5)));
			T randint0 = VarFuncs<T>::HashShadertoy(xrand, yrand, m_Seed);
			T randint = fmod(randint0 * T(32747) + T(12345), T(65535));
			tiletype = fmod(randint, T(2));
		}

		v2T R;
		T xval1, xval2;

		if (tiletype < 1)
		{
			xval1 = x;
			xval2 = x - 1;
		}
		else
		{
			xval1 = x - 1;
			xval2 = x;
		}

		R = v2T(std::pow(std::pow(std::abs(xval1), m_N) + std::pow(std::abs(y), m_N), m_OneN), std::pow(std::pow(std::abs(xval2), m_N) + std::pow(std::abs(y - 1), m_N), m_OneN));
		T r00 = std::abs(R.x - T(0.5)) / m_Rmax;
		T r11 = std::abs(R.y - T(0.5)) / m_Rmax;
		v2T xy;

		if (r00 < 1)
			xy = v2T((x + Floor<T>(helper.In.x)) * 2, (y + Floor<T>(helper.In.y)) * 2);
		else
			xy = v2T(0, 0);

		T cost = std::cos(m_Rads);
		T sint = std::sin(m_Rads);
		T roundx = (Floor<T>(helper.In.x / m_Amp) + T(0.5)) * m_Amp;
		T roundy = (Floor<T>(helper.In.y / m_Amp) + T(0.5)) * m_Amp;
		T vx = helper.In.x - roundx;
		T vy = helper.In.y - roundy;
		T newx = cost * vx + sint * vy;
		T newy = -sint * vx + cost * vy;

		if (std::abs(newx) + std::abs(newy) < m_HalfAmpMax)
		{
			if (r11 < 1)
			{
				helper.Out.x = xy.x + ((x + Floor<T>(helper.In.x)) * 2) - helper.In.x;
				helper.Out.y = xy.y + ((y + Floor<T>(helper.In.y)) * 2) - helper.In.y;
			}
			else
			{
				helper.Out.x = xy.x - helper.In.x;
				helper.Out.y = xy.y - helper.In.y;
			}
		}
		else
		{
			helper.Out.x = helper.In.x;
			helper.Out.y = helper.In.y;
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
		string exponent   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string arcwidth   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string amp        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string max        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string angle      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rmax       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n          = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string onen       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rads       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfampmax = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t scale = " << scale << ";\n"
		   << "\t\treal_t seed = " << seed << ";\n"
		   << "\t\treal_t rmax = " << rmax << ";\n"
		   << "\t\treal_t onen = " << onen << ";\n"
		   << "\t\treal_t amp = " << amp << ";\n"
		   << "\t\treal_t rads = " << rads << ";\n"
		   << "\t\treal_t n = " << n << ";\n"
		   << "\t\treal_t x1 = vIn.x * scale;\n"
		   << "\t\treal_t y1 = vIn.y * scale;\n"
		   << "\t\treal_t intx = floor(x1 + 0.5);\n"
		   << "\t\treal_t inty = floor(y1 + 0.5);\n"
		   << "\t\treal_t r_x = x1 - intx;\n"
		   << "\t\treal_t x = r_x < 0 ? 1 + r_x : r_x;\n"
		   << "\t\treal_t r_y = y1 - inty;\n"
		   << "\t\treal_t y = r_y < 0 ? 1 + r_y : r_y;\n"
		   << "\t\treal_t tiletype;\n"
		   << "\n"
		   << "\t\tif (seed == 0)\n"
		   << "\t\t{\n"
		   << "\t\t	tiletype = 0;\n"
		   << "\t\t}\n"
		   << "\t\telse if (seed == 1)\n"
		   << "\t\t{\n"
		   << "\t\t	tiletype = 1;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real_t xrand = floor(fabs(vIn.x) + (real_t)0.5);\n"
		   << "\t\t	real_t yrand = floor(fabs(vIn.y) + (real_t)0.5);\n"
		   << "\t\t	real_t randint0 = HashShadertoy(xrand, yrand, seed);\n"
		   << "\t\t	real_t randint = fmod(fma(randint0, (real_t)(32747.0), (real_t)(12345.0)), (real_t)(65535.0));\n"
		   << "\t\t	tiletype = fmod(randint, (real_t)2.0);\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t xval1, xval2;\n"
		   << "\n"
		   << "\t\tif (tiletype < 1)\n"
		   << "\t\t{\n"
		   << "\t\t	xval1 = x;\n"
		   << "\t\t	xval2 = x - 1;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	xval1 = x - 1;\n"
		   << "\t\t	xval2 = x;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal2 R = (real2)(pow(pow(fabs(xval1), n) + pow(fabs(y), n), onen), pow(pow(fabs(xval2), n) + pow(fabs(y - 1), n), onen));\n"
		   << "\t\treal_t r00 = fabs(R.x - (real_t)(0.5)) / rmax;\n"
		   << "\t\treal_t r11 = fabs(R.y - (real_t)(0.5)) / rmax;\n"
		   << "\t\treal2 xy;\n"
		   << "\n"
		   << "\t\tif (r00 < 1)\n"
		   << "\t\t	xy = (real2)((floor(vIn.x) + x) * (real_t)(2.0), (floor(vIn.y) + y) * (real_t)(2.0));\n"
		   << "\t\telse\n"
		   << "\t\t	xy = (real2)(0.0, 0.0);\n"
		   << "\n"
		   << "\t\treal_t cost = cos(rads);\n"
		   << "\t\treal_t sint = sin(rads);\n"
		   << "\t\treal_t roundx = (floor(vIn.x / amp) + (real_t)(0.5)) * amp;\n"
		   << "\t\treal_t roundy = (floor(vIn.y / amp) + (real_t)(0.5)) * amp;\n"
		   << "\t\treal_t vx = vIn.x - roundx;\n"
		   << "\t\treal_t vy = vIn.y - roundy;\n"
		   << "\t\treal_t newx = fma(cost, vx, sint * vy);\n"
		   << "\t\treal_t newy = fma(-sint, vx, cost * vy);\n"
		   << "\n"
		   << "\t\tif (fabs(newx) + fabs(newy) < " << halfampmax << ")\n"
		   << "\t\t{\n"
		   << "\t\t	if (r11 < 1)\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = xy.x + ((floor(vIn.x) + x) * (real_t)(2.0)) - vIn.x;\n"
		   << "\t\t		vOut.y = xy.y + ((floor(vIn.y) + y) * (real_t)(2.0)) - vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t	else\n"
		   << "\t\t	{\n"
		   << "\t\t		vOut.x = xy.x - vIn.x;\n"
		   << "\t\t		vOut.y = xy.y - vIn.y;\n"
		   << "\t\t	}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = vIn.x;\n"
		   << "\t\t	vOut.y = vIn.y;\n"
		   << "\t\t}\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_N = Clamp<T>(m_Exponent, T(0.001), T(2));
		T width = Clamp<T>(m_ArcWidth, T(0.001), T(1));
		m_OneN = 1 / Zeps(m_N);
		m_Rmax = Zeps(T(0.5) * (std::pow(T(2.0), m_OneN) - 1) * width);
		m_Scale = 1 / m_Weight;
		m_Rads = m_Angle * DEG_2_RAD_T;
		m_HalfAmpMax = m_Amp * T(0.5) * m_Max;
		m_Amp = Zeps(m_Amp);
	}

	virtual vector<string> OpenCLGlobalFuncNames() const override
	{
		return vector<string> { "Fract", "HashShadertoy" };
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Exponent,         prefix + "Truchet_glyph_exponent", 2, eParamType::REAL, 0, 2));
		m_Params.push_back(ParamWithName<T>(&m_ArcWidth,         prefix + "Truchet_glyph_arc_width", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Amp,              prefix + "Truchet_glyph_amp", 6, eParamType::REAL_NONZERO, EPS));
		m_Params.push_back(ParamWithName<T>(&m_Max,              prefix + "Truchet_glyph_max", T(0.9), eParamType::REAL, 0));
		m_Params.push_back(ParamWithName<T>(&m_Angle,            prefix + "Truchet_glyph_angle", 45));
		m_Params.push_back(ParamWithName<T>(&m_Seed,             prefix + "Truchet_glyph_seed", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Rmax,       prefix + "Truchet_glyph_rmax"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_N,          prefix + "Truchet_glyph_n"));
		m_Params.push_back(ParamWithName<T>(true, &m_OneN,       prefix + "Truchet_glyph_onen"));
		m_Params.push_back(ParamWithName<T>(true, &m_Scale,      prefix + "Truchet_glyph_scale"));
		m_Params.push_back(ParamWithName<T>(true, &m_Rads,       prefix + "Truchet_glyph_rads"));
		m_Params.push_back(ParamWithName<T>(true, &m_HalfAmpMax, prefix + "Truchet_glyph_half_amp_max"));
	}

private:
	T m_Exponent;
	T m_ArcWidth;
	T m_Amp;
	T m_Max;
	T m_Angle;
	T m_Seed;
	T m_Rmax;//Precalc.
	T m_N;
	T m_OneN;
	T m_Scale;
	T m_Rads;
	T m_HalfAmpMax;
};

/// <summary>
/// truchet_inv.
/// By tatasz and tyrantwave.
/// </summary>
template <typename T>
class TruchetInvVariation : public ParametricVariation<T>
{
public:
	TruchetInvVariation(T weight = 1.0) : ParametricVariation<T>("Truchet_inv", eVariationId::VAR_TRUCHET_INV, weight)
	{
		Init();
	}

	PARVARCOPY(TruchetInvVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T x1 = helper.In.x * m_Scale;
		T y1 = helper.In.y * m_Scale;
		T intx = T(Floor<T>(x1 + T(0.5)));
		T inty = T(Floor<T>(y1 + T(0.5)));
		T r_x = x1 - intx;
		T x = r_x < 0 ? 1 + r_x : r_x;
		T r_y = y1 - inty;
		T y = r_y < 0 ? 1 + r_y : r_y;
		T tiletype;

		if (m_Seed == 0)
		{
			tiletype = 0;
		}
		else if (m_Seed == 1)
		{
			tiletype = 1;
		}
		else
		{
			T xrand = Floor<T>(helper.In.x + T(0.5)) * m_Seed2;
			T yrand = Floor<T>(helper.In.y + T(0.5)) * m_Seed2;
			T niter = xrand + yrand + xrand * yrand;
			T randint0 = (niter + m_Seed) * m_Seed2Half;
			T randint = fmod(randint0 * 32747 + 12345, T(65535));
			tiletype = fmod(randint, T(2));
		}

		T xval1, xval2;

		if (tiletype < 1)
		{
			xval1 = x;
			xval2 = x - 1;
		}
		else
		{
			xval1 = x - 1;
			xval2 = x;
		}

		v2T R(std::pow(std::pow(std::abs(xval1), m_N) + std::pow(std::abs(y), m_N), m_OneN), std::pow(std::pow(std::abs(xval2), m_N) + std::pow(std::abs(y - 1), m_N), m_OneN));
		T r00 = std::abs(R.x - T(0.5)) / m_Rmax;
		T r11 = std::abs(R.y - T(0.5)) / m_Rmax;

		if (r00 > 1 && r11 > 1)
		{
			helper.Out.x = (x + Floor<T>(helper.In.x)) * m_Size;
			helper.Out.y = (y + Floor<T>(helper.In.y)) * m_Size;
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
		string index = ss2.str();
		string weight = WeightDefineString();
		string exponent  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string arcwidth  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rot       = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string size      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string rmax      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string n         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string onen      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed2     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string seed2half = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scale     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t scale = " << scale << ";\n"
		   << "\t\treal_t seed = " << seed << ";\n"
		   << "\t\treal_t seed2 = " << seed2 << ";\n"
		   << "\t\treal_t rmax = " << rmax << ";\n"
		   << "\t\treal_t onen = " << onen << ";\n"
		   << "\t\treal_t n = " << n << ";\n"
		   << "\t\treal_t size = " << size << ";\n"
		   << "\t\treal_t x1 = vIn.x * scale;\n"
		   << "\t\treal_t y1 = vIn.y * scale;\n"
		   << "\t\treal_t intx = floor(x1 + 0.5);\n"
		   << "\t\treal_t inty = floor(y1 + 0.5);\n"
		   << "\t\treal_t r_x = x1 - intx;\n"
		   << "\t\treal_t x = r_x < 0 ? 1 + r_x : r_x;\n"
		   << "\t\treal_t r_y = y1 - inty;\n"
		   << "\t\treal_t y = r_y < 0 ? 1 + r_y : r_y;\n"
		   << "\t\treal_t tiletype;\n"
		   << "\n"
		   << "\t\tif (seed == 0)\n"
		   << "\t\t{\n"
		   << "\t\t	tiletype = 0;\n"
		   << "\t\t}\n"
		   << "\t\telse if (seed == 1)\n"
		   << "\t\t{\n"
		   << "\t\t	tiletype = 1;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	real_t xrand = floor(vIn.x + 0.5) * seed2;\n"
		   << "\t\t	real_t yrand = floor(vIn.y + 0.5) * seed2;\n"
		   << "\t\t	real_t niter = xrand + yrand + xrand * yrand;\n"
		   << "\t\t	real_t randint0 = (niter + seed) * " << seed2half << ";\n"
		   << "\t\t	real_t randint = fmod(fma(randint0, (real_t)(32747.0), (real_t)(12345.0)), (real_t)(65535.0));\n"
		   << "\t\t	tiletype = fmod(randint, 2.0);\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal_t xval1, xval2;\n"
		   << "\n"
		   << "\t\tif (tiletype < 1)\n"
		   << "\t\t{\n"
		   << "\t\t	xval1 = x;\n"
		   << "\t\t	xval2 = x - 1;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	xval1 = x - 1;\n"
		   << "\t\t	xval2 = x;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\treal2 R = (real2)(pow(pow(fabs(xval1), n) + pow(fabs(y), n), onen), pow(pow(fabs(xval2), n) + pow(fabs(y - 1), n), onen));\n"
		   << "\t\treal_t r00 = fabs(R.x - (real_t)0.5) / rmax;\n"
		   << "\t\treal_t r11 = fabs(R.y - (real_t)0.5) / rmax;\n"
		   << "\n"
		   << "\t\tif (r00 > 1 && r11 > 1)\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = (x + floor(vIn.x)) * size;\n"
		   << "\t\t	vOut.y = (y + floor(vIn.y)) * size;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = 0;\n"
		   << "\t\t	vOut.y = 0;\n"
		   << "\t\t}\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_N = Clamp<T>(m_Exponent, T(0.001), T(2));
		T width = Clamp<T>(m_ArcWidth, T(0.001), T(1));
		m_Seed2 = std::sqrt(m_Seed * T(1.5)) / Zeps(m_Seed * T(0.5)) * T(0.25);
		m_Seed2Half = m_Seed2 * T(0.5);
		m_OneN = 1 / Zeps(m_N);
		m_Rmax = Zeps(T(0.5) * (std::pow(T(2.0), m_OneN) - 1) * width);
		m_Scale = (std::cos(m_Rot) + std::sin(m_Rot)) / m_Weight;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Exponent,        prefix + "Truchet_inv_exponent", 2, eParamType::REAL, 0, 2));
		m_Params.push_back(ParamWithName<T>(&m_ArcWidth,        prefix + "Truchet_inv_arc_width", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Rot,             prefix + "Truchet_inv_rotation"));
		m_Params.push_back(ParamWithName<T>(&m_Size,            prefix + "Truchet_inv_size", 1, eParamType::REAL, 0));
		m_Params.push_back(ParamWithName<T>(&m_Seed,            prefix + "Truchet_inv_seed", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Rmax,      prefix + "Truchet_inv_rmax"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_N,         prefix + "Truchet_inv_n"));
		m_Params.push_back(ParamWithName<T>(true, &m_OneN,      prefix + "Truchet_inv_onen"));
		m_Params.push_back(ParamWithName<T>(true, &m_Seed2,     prefix + "Truchet_inv_seed2"));
		m_Params.push_back(ParamWithName<T>(true, &m_Seed2Half, prefix + "Truchet_inv_seed2_half"));
		m_Params.push_back(ParamWithName<T>(true, &m_Scale,     prefix + "Truchet_inv_scale"));
	}

private:
	T m_Exponent;
	T m_ArcWidth;
	T m_Rot;
	T m_Size;
	T m_Seed;
	T m_Rmax;//Precalc.
	T m_N;
	T m_OneN;
	T m_Width;
	T m_Seed2;
	T m_Seed2Half;
	T m_Scale;
};

/// <summary>
/// henon.
/// By tyrantwave.
/// </summary>
template <typename T>
class HenonVariation : public ParametricVariation<T>
{
public:
	HenonVariation(T weight = 1.0) : ParametricVariation<T>("henon", eVariationId::VAR_HENON, weight)
	{
		Init();
	}

	PARVARCOPY(HenonVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = (1 - (m_A * SQR(helper.In.x)) + helper.In.y) * m_Weight;
		helper.Out.y = m_B * helper.In.x * m_Weight;
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
		   << "\t\tvOut.x = (1 - (" << a << " * SQR(vIn.x)) + vIn.y) * " << weight << ";\n"
		   << "\t\tvOut.y = " << b << " * vIn.x * " << weight << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "henon_a", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_B, prefix + "henon_b", 1));
	}

private:
	T m_A;
	T m_B;
};

/// <summary>
/// lozi.
/// By tyrantwave.
/// </summary>
template <typename T>
class LoziVariation : public ParametricVariation<T>
{
public:
	LoziVariation(T weight = 1.0) : ParametricVariation<T>("lozi", eVariationId::VAR_LOZI, weight)
	{
		Init();
	}

	PARVARCOPY(LoziVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = (1 - (m_A * std::abs(helper.In.x)) + helper.In.y) * m_Weight;
		helper.Out.y = m_B * helper.In.x * m_Weight;
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
		   << "\t\tvOut.x = (1 - (" << a << " * fabs(vIn.x)) + vIn.y) * " << weight << ";\n"
		   << "\t\tvOut.y = " << b << " * vIn.x * " << weight << ";\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_A, prefix + "lozi_a", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_B, prefix + "lozi_b", 1));
	}

private:
	T m_A;
	T m_B;
};

/// <summary>
/// point_symmetry.
/// By Fractalthew.
/// </summary>
template <typename T>
class PointSymmetryVariation : public ParametricVariation<T>
{
public:
	PointSymmetryVariation(T weight = 1.0) : ParametricVariation<T>("point_symmetry", eVariationId::VAR_POINT_SYMMETRY, weight)
	{
		Init();
	}

	PARVARCOPY(PointSymmetryVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T angle = Floor<T>(rand.Frand01<T>() * m_Order) * m_TwoPiDivOrder;
		T dx = (helper.In.x - m_X) * m_Weight;
		T dy = (helper.In.y - m_Y) * m_Weight;
		T cosa = std::cos(angle);
		T sina = std::sin(angle);
		helper.Out.x = m_X + dx * cosa + dy * sina;
		helper.Out.y = m_Y + dy * cosa - dx * sina;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string x             = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y             = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string order         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string twopidivorder = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t angle = floor(MwcNext01(mwc) * " << order << ") * " << twopidivorder << ";\n"
		   << "\t\treal_t dx = (vIn.x - " << x << ") * " << weight << ";\n"
		   << "\t\treal_t dy = (vIn.y - " << y << ") * " << weight << ";\n"
		   << "\t\treal_t cosa = cos(angle);\n"
		   << "\t\treal_t sina = sin(angle);\n"
		   << "\t\tvOut.x = " << x << " + dx * cosa + dy * sina;\n"
		   << "\t\tvOut.y = " << y << " + dy * cosa - dx * sina;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_TwoPiDivOrder = M_2PI / Zeps(m_Order);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X,                   prefix + "point_symmetry_centre_x", T(0.15)));
		m_Params.push_back(ParamWithName<T>(&m_Y,                   prefix + "point_symmetry_centre_y", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Order,               prefix + "point_symmetry_order", 3));
		m_Params.push_back(ParamWithName<T>(true, &m_TwoPiDivOrder, prefix + "point_symmetry_two_pi_div_order"));//Precalc.
	}

private:
	T m_X;
	T m_Y;
	T m_Order;
	T m_TwoPiDivOrder;//Precalc.
};

/// <summary>
/// d_spherical.
/// By tatasz.
/// </summary>
template <typename T>
class DSphericalVariation : public ParametricVariation<T>
{
public:
	DSphericalVariation(T weight = 1.0) : ParametricVariation<T>("d_spherical", eVariationId::VAR_D_SPHERICAL, weight, true)
	{
		Init();
	}

	PARVARCOPY(DSphericalVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		if (rand.Frand01<T>() < m_SphericalWeight)
		{
			T r = m_Weight / Zeps(helper.m_PrecalcSumSquares);
			helper.Out.x = helper.In.x * r;
			helper.Out.y = helper.In.y * r;
		}
		else
		{
			helper.Out.x = helper.In.x * m_Weight;
			helper.Out.y = helper.In.y * m_Weight;
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
		string sphericalweight = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tif (MwcNext01(mwc) < " << sphericalweight << ")\n"
		   << "\t\t{\n"
		   << "\t\t	real_t r = " << weight << " / Zeps(precalcSumSquares);\n"
		   << "\t\t	vOut.x = vIn.x * r;\n"
		   << "\t\t	vOut.y = vIn.y * r;\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t	vOut.x = vIn.x * " << weight << ";\n"
		   << "\t\t	vOut.y = vIn.y * " << weight << ";\n"
		   << "\t\t}\n"
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
		m_Params.push_back(ParamWithName<T>(&m_SphericalWeight, prefix + "d_spherical_weight", T(0.5)));
	}

private:
	T m_SphericalWeight;
};

/// <summary>
/// modulusx.
/// By tatasz.
/// </summary>
template <typename T>
class ModulusxVariation : public ParametricVariation<T>
{
public:
	ModulusxVariation(T weight = 1.0) : ParametricVariation<T>("modulusx", eVariationId::VAR_MODULUSX, weight)
	{
		Init();
	}

	PARVARCOPY(ModulusxVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T fx = std::fmod(helper.In.x + m_X + m_Shift, m_X2);

		if (fx >= 0)
			helper.Out.x = m_Weight * (fx - m_X);
		else
			helper.Out.x = m_Weight * (fx + m_X);

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
		string x     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string shift = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string x2    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t fx = fmod(vIn.x + " << x << " + " << shift << ", " << x2 << ");\n"
		   << "\n"
		   << "\t\tif (fx >= 0)\n"
		   << "\t\t	vOut.x = " << weight << " * (fx - " << x << ");\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.x = " << weight << " * (fx + " << x << ");\n"
		   << "\n"
		   << "\t\tvOut.y = " << weight << " * vIn.y;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_X2 = 2 * m_X;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X,        prefix + "modulusx_x", 1));
		m_Params.push_back(ParamWithName<T>(&m_Shift,    prefix + "modulusx_shift"));
		m_Params.push_back(ParamWithName<T>(true, &m_X2, prefix + "modulusx_x2"));//Precalc.
	}

private:
	T m_X;
	T m_Shift;
	T m_X2;//Precalc.
};

/// <summary>
/// modulusy.
/// By tatasz.
/// </summary>
template <typename T>
class ModulusyVariation : public ParametricVariation<T>
{
public:
	ModulusyVariation(T weight = 1.0) : ParametricVariation<T>("modulusy", eVariationId::VAR_MODULUSY, weight)
	{
		Init();
	}

	PARVARCOPY(ModulusyVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T fy = std::fmod(helper.In.y + m_Y + m_Shift, m_Y2);

		if (fy >= 0)
			helper.Out.y = m_Weight * (fy - m_Y);
		else
			helper.Out.y = m_Weight * (fy + m_Y);

		helper.Out.x = m_Weight * helper.In.x;
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string y     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string shift = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y2    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t fy = fmod(vIn.y + " << y << " + " << shift << ", " << y2 << ");\n"
		   << "\n"
		   << "\t\tif (fy >= 0)\n"
		   << "\t\t	vOut.y = " << weight << " * (fy - " << y << ");\n"
		   << "\t\telse\n"
		   << "\t\t	vOut.y = " << weight << " * (fy + " << y << ");\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * vIn.x;\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Y2 = 2 * m_Y;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Y,        prefix + "modulusy_y", 1));
		m_Params.push_back(ParamWithName<T>(&m_Shift,    prefix + "modulusy_shift"));
		m_Params.push_back(ParamWithName<T>(true, &m_Y2, prefix + "modulusy_y2"));//Precalc.
	}

private:
	T m_Y;
	T m_Shift;
	T m_Y2;
};

/// <summary>
/// rotate.
/// By tatasz.
/// </summary>
template <typename T>
class RotateVariation : public ParametricVariation<T>
{
public:
	RotateVariation(T weight = 1.0) : ParametricVariation<T>("rotate", eVariationId::VAR_ROTATE, weight)
	{
		Init();
	}

	PARVARCOPY(RotateVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (helper.In.x * m_AngleCos - helper.In.y * m_AngleSin);
		helper.Out.y = m_Weight * (helper.In.x * m_AngleSin + helper.In.y * m_AngleCos);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string angle    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string anglesin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string anglecos = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * (vIn.x * " << anglecos << " - vIn.y * " << anglesin << ");\n"
		   << "\t\tvOut.y = " << weight << " * (vIn.x * " << anglesin << " + vIn.y * " << anglecos << ");\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		auto rad = m_Angle / 180 * T(M_PI);
		m_AngleSin = std::sin(rad);
		m_AngleCos = std::cos(rad);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Angle,          prefix + "rotate_angle"));
		m_Params.push_back(ParamWithName<T>(true, &m_AngleSin, prefix + "rotate_angle_sin"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_AngleCos, prefix + "rotate_angle_cos"));
	}

private:
	T m_Angle;
	T m_AngleSin;//Precalc.
	T m_AngleCos;
};

/// <summary>
/// shift.
/// By tatasz.
/// </summary>
template <typename T>
class ShiftVariation : public ParametricVariation<T>
{
public:
	ShiftVariation(T weight = 1.0) : ParametricVariation<T>("shift", eVariationId::VAR_SHIFT, weight)
	{
		Init();
	}

	PARVARCOPY(ShiftVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		helper.Out.x = m_Weight * (helper.In.x + m_AngleCos * m_X - m_AngleSin * m_Y);
		helper.Out.y = m_Weight * (helper.In.y - m_AngleCos * m_Y - m_AngleSin * m_X);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string x        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string y        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string angle    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string anglesin = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string anglecos = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\tvOut.x = " << weight << " * (vIn.x + " << anglecos << " * " << x << " - " << anglesin << " * " << y << ");\n"
		   << "\t\tvOut.y = " << weight << " * (vIn.y - " << anglecos << " * " << y << " - " << anglesin << " * " << x << ");\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		auto rad = m_Angle / 180 * T(M_PI);
		m_AngleSin = std::sin(rad);
		m_AngleCos = std::cos(rad);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_X,              prefix + "shift_x"));
		m_Params.push_back(ParamWithName<T>(&m_Y,              prefix + "shift_y"));
		m_Params.push_back(ParamWithName<T>(&m_Angle,          prefix + "shift_angle"));
		m_Params.push_back(ParamWithName<T>(true, &m_AngleSin, prefix + "shift_angle_sin"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_AngleCos, prefix + "shift_angle_cos"));
	}

private:
	T m_X;
	T m_Y;
	T m_Angle;
	T m_AngleSin;//Precalc.
	T m_AngleCos;
};

/// <summary>
/// waves3.
/// By tatasz.
/// </summary>
template <typename T>
class Waves3Variation : public ParametricVariation<T>
{
public:
	Waves3Variation(T weight = 1.0) : ParametricVariation<T>("waves3", eVariationId::VAR_WAVES3, weight)
	{
		Init();
	}

	PARVARCOPY(Waves3Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T scalex = m_HalfScalex * (T(1.0) + std::sin(helper.In.y * m_Sxfreq));
		T scaley = m_HalfScaley * (T(1.0) + std::sin(helper.In.x * m_Syfreq));
		helper.Out.x = m_Weight * (helper.In.x + std::sin(helper.In.y * m_Freqx) * scalex);
		helper.Out.y = m_Weight * (helper.In.y + std::sin(helper.In.x * m_Freqy) * scaley);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string scalex     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaley     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqx      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqy      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string sxfreq     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string syfreq     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfscalex = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string halfscaley = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t scalex = " << halfscalex << " * ((real_t)(1.0) + sin(vIn.y * " << sxfreq << "));\n"
		   << "\t\treal_t scaley = " << halfscaley << " * ((real_t)(1.0) + sin(vIn.x * " << syfreq << "));\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(sin(vIn.y * " << freqx << "), scalex, vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * fma(sin(vIn.x * " << freqy << "), scaley, vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_HalfScalex = m_Scalex * T(0.5);
		m_HalfScaley = m_Scaley * T(0.5);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Scalex, prefix + "waves3_scalex", T(0.05)));
		m_Params.push_back(ParamWithName<T>(&m_Scaley, prefix + "waves3_scaley", T(0.05)));
		m_Params.push_back(ParamWithName<T>(&m_Freqx,  prefix + "waves3_freqx", 7));
		m_Params.push_back(ParamWithName<T>(&m_Freqy,  prefix + "waves3_freqy", 13));
		m_Params.push_back(ParamWithName<T>(&m_Sxfreq, prefix + "waves3_sx_freq"));
		m_Params.push_back(ParamWithName<T>(&m_Syfreq, prefix + "waves3_sy_freq", 2));
		m_Params.push_back(ParamWithName<T>(true, &m_HalfScalex, prefix + "waves3_half_scalex"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_HalfScaley, prefix + "waves3_half_scaley"));
	}

private:
	T m_Scalex;
	T m_Scaley;
	T m_Freqx;
	T m_Freqy;
	T m_Sxfreq;
	T m_Syfreq;
	T m_HalfScalex;//Precalc.
	T m_HalfScaley;
};

/// <summary>
/// waves4.
/// By tatasz.
/// </summary>
template <typename T>
class Waves4Variation : public ParametricVariation<T>
{
public:
	Waves4Variation(T weight = 1.0) : ParametricVariation<T>("waves4", eVariationId::VAR_WAVES4, weight)
	{
		Init();
	}

	PARVARCOPY(Waves4Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T ax = T(Floor<T>(helper.In.y * m_Freqx / M_2PI));
		ax = std::sin(ax * T(12.9898) + ax * T(78.233) + T(1.0) + helper.In.y * m_Yfact001) * T(43758.5453);
        ax = ax - int(ax);

		if (m_Cont == 1) ax = (ax > T(0.5)) ? T(1.0) : T(0.0);

		helper.Out.x = m_Weight * (helper.In.x + std::sin(helper.In.y * m_Freqx) * ax * ax * m_Scalex);
		helper.Out.y = m_Weight * (helper.In.y + std::sin(helper.In.x * m_Freqy) * m_Scaley);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string scalex    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaley    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqx     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqy     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cont      = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yfact     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yfact001  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t ax = floor(vIn.y * " << freqx << " / M_2PI);\n"
		   << "\t\tax = sin(ax * (real_t)(12.9898) + ax * (real_t)(78.233) + (real_t)(1.0) + vIn.y * " << yfact001 << ") * (real_t)(43758.5453);\n"
		   << "\t\tax = ax - (int) ax;\n"
		   << "\t\tif (" << cont << " == 1) ax = (ax > (real_t)(0.5)) ? (real_t)(1.0) : (real_t)(0.0);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(sin(vIn.y * " << freqx << "), ax * ax * " << scalex << ", vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * fma(sin(vIn.x * " << freqy << "), " << scaley << ", vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Yfact001 = m_Yfact * T(0.001);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Scalex, prefix + "waves4_scalex", T(0.05)));
		m_Params.push_back(ParamWithName<T>(&m_Scaley, prefix + "waves4_scaley", T(0.05)));
		m_Params.push_back(ParamWithName<T>(&m_Freqx,  prefix + "waves4_freqx", 7));
		m_Params.push_back(ParamWithName<T>(&m_Freqy,  prefix + "waves4_freqy", 13));
		m_Params.push_back(ParamWithName<T>(&m_Cont,   prefix + "waves4_cont", 0, eParamType::INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Yfact,  prefix + "waves4_yfact", T(0.1)));
		m_Params.push_back(ParamWithName<T>(true, &m_Yfact001, prefix + "waves4_yfact001"));//Precalc.
	}

private:
	T m_Scalex;
	T m_Scaley;
	T m_Freqx;
	T m_Freqy;
	T m_Cont;
	T m_Yfact;
	T m_Yfact001;//Precalc.
};

/// <summary>
/// waves22.
/// By tatasz.
/// </summary>
template <typename T>
class Waves22Variation : public ParametricVariation<T>
{
public:
	Waves22Variation(T weight = 1.0) : ParametricVariation<T>("waves22", eVariationId::VAR_WAVES22, weight)
	{
		Init();
	}

	PARVARCOPY(Waves22Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T sinx, siny;
        int px = int(m_Powerx);
        int py = int(m_Powery);

		if (m_Modex < T(0.5))
		{
			sinx = std::sin(helper.In.y * m_Freqx);
		}
		else
		{
			sinx = T(0.5) * (T(1.0) + std::sin(helper.In.y * m_Freqx));
		}

		T offsetx = T(std::pow(sinx, px) * m_Scalex);

		if (m_Modey < T(0.5))
		{
			siny = std::sin(helper.In.x * m_Freqy);
		}
		else
		{
			siny = T(0.5) * (T(1.0) + std::sin(helper.In.x * m_Freqy));
		}

		T offsety = T(std::pow(siny, py) * m_Scaley);
		helper.Out.x = m_Weight * (helper.In.x + offsetx);
		helper.Out.y = m_Weight * (helper.In.y + offsety);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string scalex = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaley = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqx  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqy  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string modex  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string modey  = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string powerx = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string powery = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t sinx, siny;\n"
		   << "\n"
		   << "\t\tint px = (int) " << powerx << ";\n"
		   << "\t\tint py = (int) " << powery << ";\n"
		   << "\t\tif (" << modex << " < (real_t)(0.5)){\n"
		   << "\t\tsinx = sin(vIn.y * " << freqx << ");\n"
		   << "\t\t} else {\n"
		   << "\t\tsinx = (real_t)(0.5) * ((real_t)(1.0) + sin(vIn.y * " << freqx << "));\n"
		   << "\t\t}\n"
		   << "\t\treal_t offsetx = pow(sinx, px) * " << scalex << ";\n"
		   << "\t\tif (" << modey << " < (real_t)(0.5)){\n"
		   << "\t\tsiny = sin(vIn.x * " << freqy << ");\n"
		   << "\t\t} else {\n"
		   << "\t\tsiny = (real_t)(0.5) * ((real_t)(1.0) + sin(vIn.x * " << freqy << "));\n"
		   << "\t\t}\n"
		   << "\t\treal_t offsety = pow(siny, py) * " << scaley << ";\n"
		   << "\t\tvOut.x = " << weight << " * (vIn.x + offsetx);\n"
		   << "\t\tvOut.y = " << weight << " * (vIn.y + offsety);\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Scalex, prefix + "waves22_scalex", T(0.05)));
		m_Params.push_back(ParamWithName<T>(&m_Scaley, prefix + "waves22_scaley", T(0.05)));
		m_Params.push_back(ParamWithName<T>(&m_Freqx,  prefix + "waves22_freqx", 7));
		m_Params.push_back(ParamWithName<T>(&m_Freqy,  prefix + "waves22_freqy", 13));
		m_Params.push_back(ParamWithName<T>(&m_Modex,  prefix + "waves22_modex", 0, eParamType::INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Modey,  prefix + "waves22_modey", 0, eParamType::INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Powerx, prefix + "waves22_powerx", 2));
		m_Params.push_back(ParamWithName<T>(&m_Powery, prefix + "waves22_powery", 2));
	}

private:
	T m_Scalex;
	T m_Scaley;
	T m_Freqx;
	T m_Freqy;
	T m_Modex;
	T m_Modey;
	T m_Powerx;
	T m_Powery;
};

/// <summary>
/// waves23.
/// By tatasz.
/// </summary>
template <typename T>
class Waves23Variation : public ParametricVariation<T>
{
public:
	Waves23Variation(T weight = 1.0) : ParametricVariation<T>("waves23", eVariationId::VAR_WAVES23, weight)
	{
		Init();
	}

	PARVARCOPY(Waves23Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T mx = helper.In.y * m_Freqx12Pi;
		T fx = mx - Floor<T>(mx);

		if (fx > T(0.5)) fx = T(0.5) - fx;

		T my = helper.In.x * m_Freqy12Pi;
		T fy = my - Floor<T>(my);

		if (fy > T(0.5)) fy = T(0.5) - fy;

		helper.Out.x = m_Weight * (helper.In.x + fx * m_Scalex);
		helper.Out.y = m_Weight * (helper.In.y + fy * m_Scaley);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string scalex    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaley    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqx     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqy     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqx12pi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqy12pi = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t mx = vIn.y * " << freqx12pi << ";\n"
		   << "\t\treal_t fx = mx - floor(mx);\n"
		   << "\t\tif (fx > (real_t)(0.5)) fx = (real_t)(0.5) - fx;\n"
		   << "\t\treal_t my = vIn.x * " << freqy12pi << ";\n"
		   << "\t\treal_t fy = my - floor(my);\n"
		   << "\t\tif (fy > (real_t)(0.5)) fy = (real_t)(0.5) - fy;\n"
		   << "\t\tvOut.x = " << weight << " * fma(fx, " << scalex << ", vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * fma(fy, " << scaley << ", vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Freqx12Pi = m_Freqx * M_1_2PI;
		m_Freqy12Pi = m_Freqy * M_1_2PI;
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Scalex, prefix + "waves23_scalex", T(0.05)));
		m_Params.push_back(ParamWithName<T>(&m_Scaley, prefix + "waves23_scaley", T(0.05)));
		m_Params.push_back(ParamWithName<T>(&m_Freqx,  prefix + "waves23_freqx", 7));
		m_Params.push_back(ParamWithName<T>(&m_Freqy,  prefix + "waves23_freqy", 13));
		m_Params.push_back(ParamWithName<T>(true, &m_Freqx12Pi, prefix + "waves23_freqx_12pi"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_Freqy12Pi, prefix + "waves23_freqy_12pi"));
	}

private:
	T m_Scalex;
	T m_Scaley;
	T m_Freqx;
	T m_Freqy;
	T m_Freqx12Pi;//Precalc.
	T m_Freqy12Pi;
};

/// <summary>
/// waves42.
/// By tatasz.
/// </summary>
template <typename T>
class Waves42Variation : public ParametricVariation<T>
{
public:
	Waves42Variation(T weight = 1.0) : ParametricVariation<T>("waves42", eVariationId::VAR_WAVES42, weight)
	{
		Init();
	}

	PARVARCOPY(Waves42Variation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T ax = T(Floor<T>(helper.In.y * m_Freqx2));
		ax = std::sin(ax * T(12.9898) + ax * T(78.233) + T(1.0) + helper.In.y * m_Yfact001) * T(43758.5453);
		ax = ax - int(ax);

		if (m_Cont == 1) ax = (ax > T(0.5)) ? T(1.0) : T(0.0);

		helper.Out.x = m_Weight * (helper.In.x + std::sin(helper.In.y * m_Freqx) * ax * ax * m_Scalex);
		helper.Out.y = m_Weight * (helper.In.y + std::sin(helper.In.x * m_Freqy) * m_Scaley);
		helper.Out.z = DefaultZ(helper);
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index = ss2.str();
		string weight = WeightDefineString();
		string scalex   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string scaley   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqx    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqy    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string cont     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yfact    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string freqx2   = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string yfact001 = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t ax = floor(vIn.y * " << freqx2 << ");\n"
		   << "\t\tax = sin(ax * (real_t)(12.9898) + ax * (real_t)(78.233) + (real_t)(1.0) + vIn.y * " << yfact001 << ") * (real_t)(43758.5453);\n"
		   << "\t\tax = ax - (int) ax;\n"
		   << "\t\tif (" << cont << " == 1) ax = (ax > (real_t)(0.5)) ? (real_t)(1.0) : (real_t)(0.0);\n"
		   << "\n"
		   << "\t\tvOut.x = " << weight << " * fma(sin(vIn.y * " << freqx << "), ax * ax * " << scalex << ", vIn.x);\n"
		   << "\t\tvOut.y = " << weight << " * fma(sin(vIn.x * " << freqy << "), " << scaley << ", vIn.y);\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_Yfact001 = m_Yfact * T(0.001);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Scalex, prefix + "waves42_scalex", T(0.05)));
		m_Params.push_back(ParamWithName<T>(&m_Scaley, prefix + "waves42_scaley", T(0.05)));
		m_Params.push_back(ParamWithName<T>(&m_Freqx,  prefix + "waves42_freqx", 7));
		m_Params.push_back(ParamWithName<T>(&m_Freqy,  prefix + "waves42_freqy", 13));
		m_Params.push_back(ParamWithName<T>(&m_Cont,   prefix + "waves42_cont", 0, eParamType::INTEGER, 0, 1));
		m_Params.push_back(ParamWithName<T>(&m_Yfact,  prefix + "waves42_yfact", T(0.1)));
		m_Params.push_back(ParamWithName<T>(&m_Freqx2, prefix + "waves42_freqx2", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_Yfact001, prefix + "waves42_yfact001"));//Precalc.
	}

private:
	T m_Scalex;
	T m_Scaley;
	T m_Freqx;
	T m_Freqy;
	T m_Cont;
	T m_Yfact;
	T m_Freqx2;
	T m_Yfact001;//Precalc.
};

/// <summary>
/// vignette by Bezo97.
/// </summary>
template <typename T>
class VignetteVariation : public ParametricVariation<T>
{
public:
	VignetteVariation(T weight = 1.0) : ParametricVariation<T>("vignette", eVariationId::VAR_VIGNETTE, weight)
	{
		Init();
	}

	PARVARCOPY(VignetteVariation)

	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		T px = helper.In.x;
		T py = helper.In.y;
		T dist = std::sqrt((px - m_Posx) * (px - m_Posx) + (py - m_Posy) * (py - m_Posy));
		helper.Out.z = DefaultZ(helper);

		if (dist < m_InnerabsPrecalc)
		{
			//middle part
			if (m_Innerradius < 0.0)
			{
				helper.Out.x = 0;
				helper.Out.y = 0;
				return;
			}

			helper.Out.x = px * m_Weight;
			helper.Out.y = py * m_Weight;
			return;
		}

		//map to 0-1
		dist = (dist - m_InnerabsPrecalc) / m_FadeabsPrecalc;
		T fade = T(1.0) - std::pow(rand.Frand01<T>(), m_PowerhelperPrecalc);
		T blur_r = m_Blur * std::pow(dist * rand.Frand01<T>(), T(2.0));
		T blur_a = rand.Frand01<T>() * M_2PI;

		if (m_Faderadius > 0)
		{
			//vignette
			if (fade < dist)
			{
				px = 0;
				py = 0;
			}
			else
			{
				px += blur_r * std::cos(blur_a);
				py += blur_r * std::sin(blur_a);
			}
		}
		else
		{
			//inverse vignette
			if (1 - fade > dist)
			{
				px = 0;
				py = 0;
			}
			else
			{
				px += blur_r * std::cos(blur_a);
				py += blur_r * std::sin(blur_a);
			}
		}

		helper.Out.x = px * m_Weight;
		helper.Out.y = py * m_Weight;
	}

	virtual string OpenCLString() const override
	{
		ostringstream ss, ss2;
		intmax_t i = 0;
		ss2 << "_" << XformIndexInEmber() << "]";
		string index              = ss2.str();
		string weight             = WeightDefineString();
		string posx               = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string posy               = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string innerradius        = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string faderadius         = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string power              = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string blur               = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string innerabsprecalc    = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string fadeabsprecalc     = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		string powerhelperprecalc = "parVars[" + ToUpper(m_Params[i++].Name()) + index;
		ss << "\t{\n"
		   << "\t\treal_t px = vIn.x;\n"
		   << "\t\treal_t py = vIn.y;\n"
		   << "\t\treal_t dist = sqrt((px - " << posx << ") * (px - " << posx << ") + (py - " << posy << ") * (py - " << posy << "));\n"
		   << "\n"
		   << "\t\tvOut.z = " << DefaultZCl()
		   << "\n"
		   << "\t\tif (dist < " << innerabsprecalc << ")\n"
		   << "\t\t{\n"
		   << "\t\t\tif (" << innerradius << " < (real_t)(0.0))\n"
		   << "\t\t\t{\n"
		   << "\t\t\t\tvOut.x = (real_t)(0.0);\n"
		   << "\t\t\t\tvOut.y = (real_t)(0.0);\n"
		   << "\t\t\t}\n"
		   << "\t\t\telse\n"
		   << "\t\t\t{\n"
		   << "\t\t\t\tvOut.x = px * " << weight << ";\n"
		   << "\t\t\t\tvOut.y = py * " << weight << ";\n"
		   << "\t\t\t}\n"
		   << "\t\t}\n"
		   << "\t\telse\n"
		   << "\t\t{\n"
		   << "\t\t\tdist = (dist - " << innerabsprecalc << ") / " << fadeabsprecalc << ";\n"
		   << "\n"
		   << "\t\t\treal_t fade = (real_t)(1.0) - pow(MwcNext01(mwc), " << powerhelperprecalc << ");\n"
		   << "\t\t\treal_t blur_r = " << blur << " * pow(dist * MwcNext01(mwc), (real_t)(2.0));\n"
		   << "\t\t\treal_t blur_a = MwcNext01(mwc) * M_2PI;\n"
		   << "\n"
		   << "\t\t\tif (" << faderadius << " > 0.0)\n"
		   << "\t\t\t{\n"
		   << "\t\t\t\tif (fade < dist)\n"
		   << "\t\t\t\t{\n"
		   << "\t\t\t\t\tpx = (real_t)(0.0);\n"
		   << "\t\t\t\t\tpy = (real_t)(0.0);\n"
		   << "\t\t\t\t}\n"
		   << "\t\t\t\telse\n"
		   << "\t\t\t\t{\n"
		   << "\t\t\t\t\tpx += blur_r * cos(blur_a);\n"
		   << "\t\t\t\t\tpy += blur_r * sin(blur_a);\n"
		   << "\t\t\t\t}\n"
		   << "\t\t\t}\n"
		   << "\t\t\telse\n"
		   << "\t\t\t{\n"
		   << "\t\t\t\tif (1 - fade > dist)\n"
		   << "\t\t\t\t{\n"
		   << "\t\t\t\t\tpx = (real_t)(0.0);\n"
		   << "\t\t\t\t\tpy = (real_t)(0.0);\n"
		   << "\t\t\t\t}\n"
		   << "\t\t\t\telse\n"
		   << "\t\t\t\t{\n"
		   << "\t\t\t\t\tpx += blur_r * cos(blur_a);\n"
		   << "\t\t\t\t\tpy += blur_r * sin(blur_a);\n"
		   << "\t\t\t\t}\n"
		   << "\t\t\t}\n"
		   << "\n"
		   << "\t\t\tvOut.x = px * " << weight << ";\n"
		   << "\t\t\tvOut.y = py * " << weight << ";\n"
		   << "\t\t}\n"
		   << "\t}\n";
		return ss.str();
	}

	virtual void Precalc() override
	{
		m_InnerabsPrecalc = std::abs(m_Innerradius);
		m_FadeabsPrecalc = std::abs(m_Faderadius);
		m_PowerhelperPrecalc = T(1.0) / std::abs(m_Power);
	}

protected:
	void Init()
	{
		string prefix = Prefix();
		m_Params.clear();
		m_Params.push_back(ParamWithName<T>(&m_Posx, prefix + "vignette_posx"));
		m_Params.push_back(ParamWithName<T>(&m_Posy, prefix + "vignette_posy"));
		m_Params.push_back(ParamWithName<T>(&m_Innerradius, prefix + "vignette_inner_radius", T(0.5)));
		m_Params.push_back(ParamWithName<T>(&m_Faderadius, prefix + "vignette_fade_radius", T(0.5), eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Power, prefix + "vignette_power", 4, eParamType::REAL_NONZERO));
		m_Params.push_back(ParamWithName<T>(&m_Blur, prefix + "vignette_blur", 1));
		m_Params.push_back(ParamWithName<T>(true, &m_InnerabsPrecalc, prefix + "vignette_inner_abs_precalc"));//Precalc.
		m_Params.push_back(ParamWithName<T>(true, &m_FadeabsPrecalc, prefix + "vignette_fade_abs_precalc"));
		m_Params.push_back(ParamWithName<T>(true, &m_PowerhelperPrecalc, prefix + "vignette_power_helper_precalc"));
	}

private:
	T m_Posx;
	T m_Posy;
	T m_Innerradius;
	T m_Faderadius;
	T m_Power;
	T m_Blur;
	T m_InnerabsPrecalc;//Precalc.
	T m_FadeabsPrecalc;
	T m_PowerhelperPrecalc;
};

MAKEPREPOSTPARVAR(Splits3D, splits3D, SPLITS3D)
MAKEPREPOSTPARVAR(Waves2B, waves2b, WAVES2B)
MAKEPREPOSTPARVAR(JacCn, jac_cn, JAC_CN)
MAKEPREPOSTPARVAR(JacDn, jac_dn, JAC_DN)
MAKEPREPOSTPARVAR(JacSn, jac_sn, JAC_SN)
MAKEPREPOSTPARVAR(PressureWave, pressure_wave, PRESSURE_WAVE)
MAKEPREPOSTVAR(Gamma, gamma, GAMMA)
MAKEPREPOSTPARVAR(PRose3D, pRose3D, PROSE3D)
MAKEPREPOSTPARVAR(LogDB, log_db, LOG_DB)
MAKEPREPOSTPARVAR(CircleSplit, circlesplit, CIRCLESPLIT)
MAKEPREPOSTVAR(Cylinder2, cylinder2, CYLINDER2)
MAKEPREPOSTPARVAR(TileLog, tile_log, TILE_LOG)
MAKEPREPOSTPARVAR(TileHlp, tile_hlp, TILE_HLP)
MAKEPREPOSTPARVAR(TruchetFill, Truchet_fill, TRUCHET_FILL)
MAKEPREPOSTPARVAR(TruchetHexFill, Truchet_hex_fill, TRUCHET_HEX_FILL)
MAKEPREPOSTPARVAR(TruchetHexCrop, Truchet_hex_crop, TRUCHET_HEX_CROP)
MAKEPREPOSTPARVAR(Waves2Radial, waves2_radial, WAVES2_RADIAL)
MAKEPREPOSTVAR(Panorama1, panorama1, PANORAMA1)
MAKEPREPOSTVAR(Panorama2, panorama2, PANORAMA2)
MAKEPREPOSTPARVAR(Helicoid, helicoid, HELICOID)
MAKEPREPOSTPARVAR(Helix, helix, HELIX)
MAKEPREPOSTPARVARASSIGN(Sphereblur, sphereblur, SPHEREBLUR, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(Cpow3, cpow3, CPOW3)
MAKEPREPOSTPARVARASSIGN(Concentric, concentric, CONCENTRIC, eVariationAssignType::ASSIGNTYPE_SUM)
MAKEPREPOSTPARVAR(Hypercrop, hypercrop, HYPERCROP)
MAKEPREPOSTPARVAR(Hypershift, hypershift, HYPERSHIFT)
MAKEPREPOSTPARVAR(Hypershift2, hypershift2, HYPERSHIFT2)
MAKEPREPOSTVAR(Lens, lens, LENS)
MAKEPREPOSTPARVAR(Projective, projective, PROJECTIVE)
MAKEPREPOSTPARVAR(DepthBlur, depth_blur, DEPTH_BLUR)
MAKEPREPOSTPARVAR(DepthBlur2, depth_blur2, DEPTH_BLUR2)
MAKEPREPOSTPARVAR(DepthGaussian, depth_gaussian, DEPTH_GAUSSIAN)
MAKEPREPOSTPARVAR(DepthGaussian2, depth_gaussian2, DEPTH_GAUSSIAN2)
MAKEPREPOSTPARVAR(DepthNgon, depth_ngon, DEPTH_NGON)
MAKEPREPOSTPARVAR(DepthNgon2, depth_ngon2, DEPTH_NGON2)
MAKEPREPOSTPARVAR(DepthSine, depth_sine, DEPTH_SINE)
MAKEPREPOSTPARVAR(DepthSine2, depth_sine2, DEPTH_SINE2)
MAKEPREPOSTPARVAR(CothSpiral, coth_spiral, COTH_SPIRAL)
MAKEPREPOSTPARVAR(Dust, dust, DUST)
MAKEPREPOSTPARVAR(Asteria, asteria, ASTERIA)
MAKEPREPOSTPARVAR(Pulse, pulse, PULSE)
MAKEPREPOSTPARVAR(Excinis, excinis, EXCINIS)
MAKEPREPOSTPARVAR(Vibration, vibration, VIBRATION)
MAKEPREPOSTPARVAR(Vibration2, vibration2, VIBRATION2)
MAKEPREPOSTPARVAR(Arcsech, arcsech, ARCSECH)
MAKEPREPOSTPARVAR(Arcsech2, arcsech2, ARCSECH2)
MAKEPREPOSTPARVAR(Arcsinh, arcsinh, ARCSINH)
MAKEPREPOSTPARVAR(Arctanh, arctanh, ARCTANH)
MAKEPREPOSTPARVAR(HexTruchet, hex_truchet, HEX_TRUCHET)
MAKEPREPOSTPARVAR(HexRand, hex_rand, HEX_RAND)
MAKEPREPOSTPARVAR(Smartshape, smartshape, SMARTSHAPE)
MAKEPREPOSTPARVAR(Squares, squares, SQUARES)
MAKEPREPOSTPARVAR(Starblur2, starblur2, STARBLUR2)
MAKEPREPOSTPARVAR(UnicornGaloshen, unicorngaloshen, UNICORNGALOSHEN)
MAKEPREPOSTPARVAR(Dragonfire, dragonfire, DRAGONFIRE)
MAKEPREPOSTPARVAR(TruchetGlyph, Truchet_glyph, TRUCHET_GLYPH)
MAKEPREPOSTPARVAR(TruchetInv, Truchet_inv, TRUCHET_INV)
MAKEPREPOSTPARVAR(Henon, henon, HENON)
MAKEPREPOSTPARVAR(Lozi, lozi, LOZI)
MAKEPREPOSTPARVAR(PointSymmetry, point_symmetry, POINT_SYMMETRY)
MAKEPREPOSTPARVAR(DSpherical, d_spherical, D_SPHERICAL)
MAKEPREPOSTPARVAR(Modulusx, modulusx, MODULUSX)
MAKEPREPOSTPARVAR(Modulusy, modulusy, MODULUSY)
MAKEPREPOSTPARVAR(Rotate, rotate, ROTATE)
MAKEPREPOSTPARVAR(Shift, shift, SHIFT)
MAKEPREPOSTPARVAR(Waves22, waves22, WAVES22)
MAKEPREPOSTPARVAR(Waves23, waves23, WAVES23)
MAKEPREPOSTPARVAR(Waves42, waves42, WAVES42)
MAKEPREPOSTPARVAR(Waves3, waves3, WAVES3)
MAKEPREPOSTPARVAR(Waves4, waves4, WAVES4)
MAKEPREPOSTPARVAR(Vignette, vignette, VIGNETTE)
}
