#pragma once

#include "Utils.h"

/// <summary>
/// VarFuncs class.
/// </summary>

namespace EmberNs
{
/// <summary>
/// Central coordinating place for code and data common to some variations.
/// This class is a singleton since all of its data is shared and read-only.
/// </summary>
template <typename T>
class EMBER_API VarFuncs : public Singleton<VarFuncs<T>>
{
public:
	/// <summary>
	/// Return -1 if the value is less than 0, 1 if it's greater and
	/// 0 if it's equal to 0.
	/// </summary>
	/// <param name="v">The value to inspect</param>
	/// <returns>-1, 0 or 1</returns>
	static inline T Sign(T v)
	{
		return (v < 0) ? static_cast<T>(-1) : (v > 0) ? static_cast<T>(1) : static_cast<T>(0);
	}

	/// <summary>
	/// Return -1 if the value is less than 0, 1 if it's greater.
	/// This differs from Sign() in that it doesn't return 0.
	/// </summary>
	/// <param name="v">The value to inspect</param>
	/// <returns>-1 or 1</returns>
	static inline T SignNz(T v)
	{
		return (v < 0) ? static_cast<T>(-1) : static_cast<T>(1);
	}

	/// <summary>
	/// Thin wrapper around a call to modf that discards the integer portion
	/// and returns the signed fractional portion.
	/// </summary>
	/// <param name="v">The value to retrieve the signed fractional portion of.</param>
	/// <returns>The signed fractional portion of v.</returns>
	static inline T Fabsmod(T v)
	{
		T dummy;
		return modf(v, &dummy);
	}

	/// <summary>
	/// Return the fractional part of a real number.
	/// </summary>
	/// <param name="v">The real number whose fractional part will be returned</param>
	/// <returns>The fractional part of the value passed in</returns>
	static inline T Fract(T x)
	{
		return x - T(Floor(x));
	}

	/// <summary>
	/// Unsure.
	/// </summary>
	/// <param name="p">Unsure.</param>
	/// <param name="amp">Unsure.</param>
	/// <param name="ph">Unsure.</param>
	/// <returns>Unsure.</returns>
	static inline T Fosc(T p, T amp, T ph)
	{
		return T(0.5) - std::cos(p * amp + ph) * T(0.5);
	}

	/// <summary>
	/// Unsure.
	/// </summary>
	/// <param name="p">Unsure.</param>
	/// <param name="ph">Unsure.</param>
	/// <returns>Unsure.</returns>
	static inline T Foscn(T p, T ph)
	{
		return T(0.5) - std::cos(p + ph) * T(0.5);
	}

	/// <summary>
	/// Log scale from Apophysis.
	/// </summary>
	/// <param name="x">The value to log scale</param>
	/// <returns>The log scaled value</returns>
	static inline T LogScale(T x)
	{
		return x == 0 ? 0 : std::log((fabs(x) + 1) * T(M_E)) * SignNz(x) / T(M_E);
	}

	/// <summary>
	/// Log map from Apophysis.
	/// </summary>
	/// <param name="x">The value to log map</param>
	/// <returns>The log mapped value</returns>
	static inline T LogMap(T x)
	{
		return x == 0 ? 0 : (T(M_E) + std::log(x * T(M_E))) * T(0.25) * SignNz(x);
	}

	/// <summary>
	/// Taking the square root of numbers close to zero is dangerous.  If x is negative
	/// due to floating point errors, it can return NaN results.
	/// </summary>
	static inline T SafeSqrt(T x)
	{
		if (x <= 0)
			return 0;

		return std::sqrt(x);
	}

	/// <summary>
	/// If r < EPS, return 1 / r.
	/// Else, return q / r.
	/// </summary>
	/// <param name="q">The numerator</param>
	/// <param name="r">The denominator</param>
	/// <returns>The quotient</returns>
	static inline T SafeDivInv(T q, T r)
	{
		if (r < EPS)
			return 1 / r;

		return q / r;
	}

	/// <summary>
	/// Return the hypotenuse of the passed in values.
	/// </summary>
	/// <param name="x">The x distance</param>
	/// <param name="y">The y distance</param>
	/// <returns>The hypotenuse</returns>
	static inline T Hypot(T x, T y)
	{
		return std::sqrt(SQR(x) + SQR(y));
	}

	/// <summary>
	/// Spread the values.
	/// </summary>
	/// <param name="x">The x distance</param>
	/// <param name="y">The y distance</param>
	/// <returns>The spread</returns>
	static inline T Spread(T x, T y)
	{
		return Hypot(x, y) * ((x) > 0 ? T(1) : T(-1));
	}

	/// <summary>
	/// Unsure.
	/// </summary>
	/// <param name="x">The x distance</param>
	/// <param name="y">The y distance</param>
	/// <returns>The powq4</returns>
	static inline T Powq4(T x, T y)
	{
		return std::pow(std::abs(x), y) * SignNz(x);
	}

	/// <summary>
	/// Unsure.
	/// </summary>
	/// <param name="x">The x distance</param>
	/// <param name="y">The y distance</param>
	/// <returns>The powq4c</returns>
	static inline T Powq4c(T x, T y)
	{
		return y == 1 ? x : Powq4(x, y);
	}

	/// <summary>
	/// Special rounding for certain variations, gotten from Apophysis.
	/// </summary>
	/// <param name="x">The value to round</param>
	/// <returns>The rounded value</returns>
	static inline float LRint(float x)
	{
		int temp = (x >= 0 ? static_cast<int>(x + 0.5f) : static_cast<int>(x - 0.5f));
		return static_cast<float>(temp);
	}

	/// <summary>
	/// Special rounding for certain variations, gotten from Apophysis.
	/// </summary>
	/// <param name="x">The value to round</param>
	/// <returns>The rounded value</returns>
	static inline double LRint(double x)
	{
		glm::int64_t temp = (x >= 0 ? static_cast<int64_t>(x + 0.5) : static_cast<int64_t>(x - 0.5));
		return static_cast<double>(temp);
	}

	/// <summary>
	/// Integer hash function from http://burtleburtle.net/bob/hash/integer.html
	/// </summary>
	/// <param name="a">The value to hash</param>
	/// <returns>The hashed value</returns>
	static inline T Hash(int a)
	{
		a = (a ^ 61) ^ (a >> 16);
		a = a + (a << 3);
		a = a ^ (a >> 4);
		a = a * 0x27d4eb2d;
		a = a ^ (a >> 15);
		return T(a) / std::numeric_limits<int>::max();
	}

	/// <summary>
	/// Hash function gotten from Chaotica, which takes an x,y pair and hashes it.
	/// Written by Thomas Ludwig and Tatyana Zabanova.
	/// </summary>
	/// <param name="x">The x value to hash</param>
	/// <param name="y">The y value to hash</param>
	/// <param name="seed">The seed to hash with</param>
	/// <returns>The hashed value</returns>
	static inline T HashShadertoy(T x, T y, T seed)
	{
		return Fract(std::sin(x * T(12.9898) + y * T(78.233) + seed) * T(43758.5453));
	}

	/// <summary>
	/// For the vibration2 variation.
	/// </summary>
	/// <returns>T</returns>
	static inline T Modulate(T amp, T freq, T x)
	{
		return amp * std::cos(x * freq * M_2PI);
	}

	/// <summary>
	/// Divide real by complex.
	/// </summary>
	/// <param name="x">The real number</param>
	/// <param name="a">The complex number</param>
	/// <returns>x / a</returns>
	static v2T RealDivComplex(T x, v2T a)
	{
		T s = x / Zeps(a.x * a.x + a.y * a.y);
		return v2T(a.x * s, -a.y * s);
	}

	/// <summary>
	/// Divide complex by complex.
	/// </summary>
	/// <param name="x">The first complex number</param>
	/// <param name="a">The secondcomplex number</param>
	/// <returns>a / b</returns>
	static v2T ComplexDivComplex(v2T a, v2T b)
	{
		T s = T(1.0) / Zeps(b.x * b.x + b.y * b.y);
		return v2T(a.x * b.x + a.y * b.y, a.y * b.x - a.x * b.y) * s;
	}

	/// <summary>
	/// Multiple complex by real.
	/// </summary>
	/// <param name="a">The complex number</param>
	/// <param name="x">The real number</param>
	/// <returns>a * x</returns>
	static v2T ComplexMultReal(v2T a, T x)
	{
		return v2T(a.x * x, a.y * x);
	}

	/// <summary>
	/// Multiply complex by complex.
	/// </summary>
	/// <param name="a">The first complex number</param>
	/// <param name="b">The second complex number</param>
	/// <returns>a * b</returns>
	static v2T ComplexMultComplex(v2T a, v2T b)
	{
		return v2T(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
	}

	/// <summary>
	/// Add complex to complex.
	/// </summary>
	/// <param name="a">The first complex number</param>
	/// <param name="b">The second complex number</param>
	/// <returns>a + b</returns>
	static v2T ComplexPlusComplex(v2T a, v2T b)
	{
		return v2T(a.x + b.x, a.y + b.y);
	}

	/// <summary>
	/// Add complex to real.
	/// </summary>
	/// <param name="a">The complex number</param>
	/// <param name="x">The real number</param>
	/// <returns>a + x</returns>
	static v2T ComplexPlusReal(v2T a, T x)
	{
		return v2T(a.x + x, a.y);
	}

	/// <summary>
	/// Subtract complex from complex.
	/// </summary>
	/// <param name="a">The first complex number</param>
	/// <param name="b">The second complex number</param>
	/// <returns>a - b</returns>
	static v2T ComplexMinusComplex(v2T a, v2T b)
	{
		return v2T(a.x - b.x, a.y - b.y);
	}

	/// <summary>
	/// Subtract real from complex.
	/// </summary>
	/// <param name="a">The complex number</param>
	/// <param name="x">The real number</param>
	/// <returns>a - x</returns>
	static v2T ComplexMinusReal(v2T a, T x)
	{
		return v2T(a.x - x, a.y);
	}

	/// <summary>
	/// Compute the square root of a complex number.
	/// </summary>
	/// <param name="a">The complex number</param>
	/// <returns>sqrt(a)</returns>
	static v2T ComplexSqrt(v2T a)
	{
		T mag = Hypot(a.x, a.y);
		return ComplexMultReal(v2T(std::sqrt(mag + a.x), Sign(a.y) * std::sqrt(mag - a.x)), T(0.5) * std::sqrt(T(2.0)));
	}

	/// <summary>
	/// Compute the natural logarithm of a complex number.
	/// </summary>
	/// <param name="a">The complex number</param>
	/// <returns>log(a)</returns>
	static v2T ComplexLog(v2T a)
	{
		return v2T(T(0.5) * std::log(a.x * a.x + a.y * a.y), std::atan2(a.y, a.x));
	}

	/// <summary>
	/// Compute the inverse of the natural logarithm of a complex number.
	/// </summary>
	/// <param name="a">The complex number</param>
	/// <returns>exp(a)</returns>
	static v2T ComplexExp(v2T a)
	{
		return v2T(std::cos(a.y), std::sin(a.y)) * std::exp(a.x);
	}

	/// <summary>
	/// Retrieve information about a piece of shared data by looking
	/// up its name.
	/// </summary>
	/// <param name="name">The name of the shared data to retrieve</param>
	/// <returns>A pointer to the beginning of the data and its size in terms of sizeof(T)</returns>
	std::pair<const T*, size_t>* GetSharedData(const string& name)
	{
		const auto& data = m_GlobalMap.find(name);

		if (data != m_GlobalMap.end())
			return &data->second;
		else
			return nullptr;
	}

	/// <summary>
	/// The size of the index array.
	/// </summary>
	/// <returns>size_t</returns>
	inline size_t IndexCount() const
	{
		return m_P.size();
	}

	/// <summary>
	/// Get the index value at the specified index.
	/// </summary>
	/// <param name="i">The index to retrieve</param>
	/// <returns>int</returns>
	inline int Index(int i) const
	{
		return m_P[i];
	}

	/// <summary>
	/// Get a const reference to the 3 component vector at the specified index.
	/// </summary>
	/// <param name="i">The index to retrieve</param>
	/// <returns>v3T&</returns>
	inline v3T& Grad(int i)
	{
		return m_Grad[i];
	}

	/// <summary>
	/// Get the size of the gradient vector.
	/// </summary>
	/// <returns>size_t</returns>
	inline size_t GradCount() const
	{
		return m_Grad.size();
	}

	/// <summary>
	/// Get a pointer to the floating point index values.
	/// </summary>
	/// <returns>T*</returns>
	inline T* IndexFloats() const
	{
		return const_cast<T*>(m_PFloats.data());
	}

	/// <summary>
	/// Compute 3D simplex nosie value based on the 3 component vector passed in.
	/// </summary>
	/// <param name="v">The vector to use to compute the value</param>
	/// <returns>T</returns>
	T SimplexNoise3D(const v3T& v)
	{
		v3T c[4]; // Co-ordinates of four simplex shape corners in (x,y,z)
		T n = 0; // Noise total value
		int gi[4];      // Hashed grid index for each corner, used to determine gradient
		// Convert input co-ordinates ( x, y, z ) to
		// integer-based simplex grid ( i, j, k )
		T skewIn = (v.x + v.y + v.z) * T(0.333333);
		intmax_t i = Floor<T>(v.x + skewIn);
		intmax_t j = Floor<T>(v.y + skewIn);
		intmax_t k = Floor<T>(v.z + skewIn);
		T t = (i + j + k) * T(0.1666666);
		// Cell origin co-ordinates in input space (x,y,z)
		T x0 = i - t;
		T y0 = j - t;
		T z0 = k - t;
		// This value of t finished with, not used later . . .
		// Point offset within cell, in input space (x,y,z)
		c[0].x = v.x - x0;
		c[0].y = v.y - y0;
		c[0].z = v.z - z0;
		// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
		// The nested logic determines which simplex we are in, and therefore in which
		// order to get gradients for the four corners
		int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
		int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

		// The fourth corner is always i3 = 1, j3 = 1, k3 = 1, so no need to
		// calculate values
		if (c[0].x >= c[0].y)
		{
			if (c[0].y >= c[0].z)
			{
				i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
			}
			else   // y0<z0
			{
				if (c[0].x >= c[0].z)
				{
					i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1;
				}
				else
				{
					i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1;
				}
			}
		}
		else   // x0<y0
		{
			if (c[0].y < c[0].z)
			{
				i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1;
			}
			else
			{
				if (c[0].x < c[0].z)
				{
					i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1;
				}
				else
				{
					i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
				}
			}
		}

		// A step of 1i in (i,j,k) is a step of (1-T(0.16666), -T(0.16666), -T(0.16666)) in (x,y,z),
		// and this is similar for j and k . . .
		// Offsets for second corner in (x,y,z) coords
		c[1].x = c[0].x - i1 + T(0.1666666);
		c[1].y = c[0].y - j1 + T(0.1666666);
		c[1].z = c[0].z - k1 + T(0.1666666);
		// Offsets for third corner in (x,y,z) coords
		c[2].x = c[0].x - i2 + 2 * T(0.1666666);
		c[2].y = c[0].y - j2 + 2 * T(0.1666666);
		c[2].z = c[0].z - k2 + 2 * T(0.1666666);
		// Offsets for last corner in (x,y,z) coords
		c[3].x = c[0].x - 1 + 3 * T(0.1666666);
		c[3].y = c[0].y - 1 + 3 * T(0.1666666);
		c[3].z = c[0].z - 1 + 3 * T(0.1666666);
		// Work out the hashed gradient indices of the four simplex corners
		int ii = i & 0x3ff;
		int jj = j & 0x3ff;
		int kk = k & 0x3ff;
		gi[0] = m_P[ii + m_P[jj + m_P[kk]]];
		gi[1] = m_P[ii + i1 + m_P[jj + j1 + m_P[kk + k1]]];
		gi[2] = m_P[ii + i2 + m_P[jj + j2 + m_P[kk + k2]]];
		gi[3] = m_P[ii + 1 + m_P[jj + 1 + m_P[kk + 1]]];

		// Calculate the contribution from the four corners, and add to total
		for (uint corner = 0u; corner < 4u; corner++)
		{
			t = T(0.6) - Sqr(c[corner].x) - Sqr(c[corner].y) - Sqr(c[corner].z);

			if (t > 0)
			{
				v3T u = m_Grad[gi[corner]];
				t *= t;
				n += t * t * (u.x * c[corner].x + u.y * c[corner].y + u.z * c[corner].z);
			}
		}

		// The result is scaled be fit -1.0 to 1.0
		return 32 * n;
	}

	/// <summary>
	/// Compute a perlin noise value based on the values passed in.
	/// This will iteratively call SimplexNoise3D().
	/// </summary>
	/// <param name="v">The vector</param>
	/// <param name="aScale">A value to scale a by</param>
	/// <param name="fScale">A value to scale f by</param>
	/// <param name="octaves">The number of iterations to perform</param>
	/// <returns>T</returns>
	T PerlinNoise3D(v3T& v, T aScale, T fScale, int octaves)
	{
		T n = 0, a = 1;
		v3T u = v;

		for (int i = 0; i < octaves; i++)
		{
			n += SimplexNoise3D(u) / Zeps(a);
			a *= aScale;
			u.x *= fScale;
			u.y *= fScale;
			u.x *= fScale;
		}

		return n;
	}

	/// <summary>
	/// Find the element in p which is closest to u and return
	/// the index of that element.
	/// </summary>
	/// <param name="p">The vector of points to examine</param>
	/// <param name="n">The number of points in p to examine</param>
	/// <param name="u">The point to compare p gainst</param>
	/// <returns>Integer index in p which contained the closest point</returns>
	static int Closest(v2T* p, int n, v2T& u)
	{
		T d2;
		T d2min = TMAX;
		int i, j = 0;

		for (i = 0; i < n; i++)
		{
			d2 = Sqr<T>(p[i].x - u.x) + Sqr<T>(p[i].y - u.y);

			if (d2 < d2min)
			{
				d2min = d2;
				j = i;
			}
		}

		return j;
	}

	/// <summary>
	/// Unsure.
	/// </summary>
	static T Vratio(const v2T& p, const v2T& q, const v2T& u)
	{
		v2T pmq = p - q;

		if (pmq.x == 0 && pmq.y == 0)
			return 1;

		return 2 * ((u.x - q.x) * pmq.x + (u.y - q.y) * pmq.y) / Zeps(SQR(pmq.x) + SQR(pmq.y));
	}

	/// <summary>
	/// Unsure.
	/// </summary>
	static T Voronoi(v2T* p, int n, int q, const v2T& u)
	{
		T ratio, ratiomax = TLOW;

		for (int i = 0; i < n; i++)
		{
			if (i != q)
			{
				ratio = Vratio(p[i], p[q], u);

				if (ratio > ratiomax)
					ratiomax = ratio;
			}
		}

		return ratiomax;
	}

	/// <summary>
	/// Used in the jac_* variations.
	/// </summary>
	static void JacobiElliptic(T uu, T emmc, T& sn, T& cn, T& dn)
	{
		//Code is taken from IROIRO++ library,
		//released under CC share-alike license.
		//Less accurate for faster rendering (still very precise).
		T const CA = T(0.0003);//The accuracy is the square of CA.
		T a, b, c, d = 1, em[13], en[13];
		int bo;
		int l;
		int ii;
		int i;
		T emc = emmc;
		T u = uu;

		if (emc != 0)
		{
			bo = 0;

			if (emc < 0)
				bo = 1;

			if (bo != 0)
			{
				d = 1 - emc;
				emc = -emc / d;
				d = std::sqrt(d);
				u = d * u;
			}

			a = 1;
			dn = 1;

			for (i = 0; i < 8; i++)
			{
				l = i;
				em[i] = a;
				emc = std::sqrt(emc);
				en[i] = emc;
				c = T(0.5) * (a + emc);

				if (std::abs(a - emc) <= CA * a)
					break;

				emc = a * emc;
				a = c;
			}

			u = c * u;
			sincos(u, &sn, &cn);

			if (sn != 0)
			{
				a = cn / sn;
				c = a * c;

				for (ii = l; ii >= 0; --ii)
				{
					b = em[ii];
					a = c * a;
					c = dn * c;
					dn = (en[ii] + a) / (b + a);
					a = c / b;
				}

				a = 1 / std::sqrt(c * c + 1);

				if (sn < 0)
					sn = -a;
				else
					sn = a;

				cn = c * sn;
			}

			if (bo != 0)
			{
				a = dn;
				dn = cn;
				cn = a;
				sn = sn / d;
			}
		}
		else
		{
			cn = 1 / std::cosh(u);
			dn = cn;
			sn = std::tanh(u);
		}
	}

	SINGLETON_DERIVED_IMPL_T(VarFuncs, T);

private:
	/// <summary>
	/// Constructor which initializes data and adds information about them to a global map.
	/// </summary>
	VarFuncs()
	{
		m_P = InitInts();
		m_Grad = InitGrad();
		m_Offsets = InitOffsets();
		m_P1 = InitP1();
		m_Q1 = InitQ1();
		m_P2 = InitP2();
		m_Q2 = InitQ2();
		m_PC = InitPC();
		m_QC = InitQC();
		m_PS = InitPS();
		m_QS = InitQS();
		m_GlobalMap["NOISE_INDEX"] = make_pair(m_PFloats.data(), m_PFloats.size());
		m_GlobalMap["NOISE_POINTS"] = make_pair(static_cast<T*>(&(m_Grad[0].x)), SizeOf(m_Grad) / sizeof(T));
		m_GlobalMap["OFFSETS"] = make_pair(static_cast<T*>(&(m_Offsets[0].x)), SizeOf(m_Offsets) / sizeof(T));
		m_GlobalMap["P1"] = make_pair(m_P1.data(), m_P1.size());
		m_GlobalMap["Q1"] = make_pair(m_Q1.data(), m_Q1.size());
		m_GlobalMap["P2"] = make_pair(m_P2.data(), m_P2.size());
		m_GlobalMap["Q2"] = make_pair(m_Q2.data(), m_Q2.size());
		m_GlobalMap["PC"] = make_pair(m_PC.data(), m_PC.size());
		m_GlobalMap["QC"] = make_pair(m_QC.data(), m_QC.size());
		m_GlobalMap["PS"] = make_pair(m_PS.data(), m_PS.size());
		m_GlobalMap["QS"] = make_pair(m_QS.data(), m_QS.size());
	}

	/// <summary>
	/// Initializes integer indices via initializer list.
	/// Called once from the constructor.
	/// </summary>
	/// <returns>A copy of the locally declared vector</returns>
	std::vector<int> InitInts()
	{
		std::vector<int> p =
		{
			127, 71, 882, 898, 798, 463, 517, 451, 454, 634, 578, 695, 728, 742, 325, 350, 684, 153, 340,
			311, 992, 706, 218, 285, 96, 486, 160, 98, 686, 288, 193, 119, 410, 246, 536, 415, 953, 417,
			784, 573, 734, 1, 136, 381, 177, 678, 773, 22, 301, 51, 874, 844, 775, 744, 633, 468, 1019,
			287, 475, 78, 294, 724, 519, 17, 323, 191, 187, 446, 262, 212, 170, 33, 7, 227, 566, 526, 264,
			556, 717, 477, 815, 671, 225, 207, 692, 663, 969, 393, 658, 877, 353, 788, 128, 303, 614, 501,
			490, 387, 53, 941, 951, 736, 539, 102, 163, 175, 584, 988, 35, 347, 442, 649, 642, 198, 727,
			939, 913, 811, 894, 858, 181, 412, 307, 830, 154, 479, 704, 326, 681, 619, 698, 621, 552, 598,
			74, 890, 299, 922, 701, 481, 867, 214, 817, 731, 768, 673, 315, 338, 576, 222, 484, 305, 623,
			239, 269, 46, 748, 608, 546, 537, 125, 667, 998, 714, 529, 823, 247, 289, 771, 808, 973, 735,
			516, 974, 702, 636, 357, 455, 600, 80, 336, 696, 963, 297, 92, 980, 670, 958, 625, 712, 406,
			173, 19, 763, 470, 793, 283, 655, 59, 421, 1016, 219, 13, 105, 840, 111, 38, 408, 945, 242,
			559, 206, 443, 331, 737, 580, 767, 1020, 220, 31, 968, 15, 527, 833, 139, 129, 859, 739, 418,
			783, 933, 49, 789, 178, 124, 772, 627, 0, 23, 388, 950, 976, 940, 485, 685, 21, 523, 723, 244,
			637, 488, 835, 379, 342, 452, 862, 295, 765, 897, 507, 370, 567, 416, 100, 914, 300, 120, 392,
			694, 94, 265, 791, 171, 200, 787, 441, 868, 672, 769, 983, 911, 427, 82, 69, 224, 176, 920,
			500, 462, 263, 513, 797, 293, 322, 645, 469, 635, 40, 215, 687, 960, 818, 826, 34, 603, 316,
			994, 611, 511, 93, 899, 114, 73, 241, 585, 327, 674, 280, 957, 471, 24, 502, 355, 159, 1017,
			855, 270, 538, 521, 162, 880, 334, 986, 740, 719, 266, 820, 97, 41, 52, 750, 893, 838, 616, 83,
			896, 777, 464, 562, 183, 362, 411, 478, 398, 384, 912, 599, 587, 609, 822, 243, 504, 753, 857,
			157, 964, 65, 261, 81, 371, 435, 924, 885, 884, 863, 613, 721, 669, 121, 639, 989, 487, 238,
			448, 216, 852, 643, 713, 676, 277, 879, 133, 123, 304, 547, 396, 70, 141, 909, 848, 900, 318,
			146, 356, 802, 4, 807, 558, 764, 545, 588, 872, 554, 467, 544, 505, 149, 62, 901, 64, 45, 813,
			27, 109, 718, 803, 853, 996, 1014, 476, 575, 28, 199, 688, 6, 482, 703, 560, 395, 66, 341, 794,
			422, 376, 601, 76, 14, 569, 480, 39, 1011, 1001, 854, 55, 89, 335, 761, 363, 419, 252, 799,
			358, 324, 1012, 152, 312, 496, 235, 916, 582, 615, 979, 1005, 891, 1013, 641, 18, 148, 185,
			512, 378, 58, 211, 495, 594, 87, 762, 366, 660, 449, 520, 424, 886, 819, 281, 147, 290, 390,
			32, 572, 993, 720, 683, 309, 254, 607, 568, 256, 533, 394, 620, 429, 67, 831, 103, 423, 668,
			693, 518, 551, 697, 253, 949, 54, 875, 116, 434, 743, 644, 590, 279, 843, 589, 11, 647, 586,
			806, 549, 375, 226, 851, 499, 450, 978, 29, 982, 189, 107, 508, 373, 796, 20, 700, 110, 26,
			461, 782, 591, 828, 57, 904, 847, 328, 122, 630, 711, 44, 397, 404, 209, 365, 84, 194, 1021,
			675, 135, 965, 329, 557, 691, 79, 352, 498, 629, 869, 90, 921, 233, 622, 871, 755, 439, 955,
			228, 63, 825, 43, 943, 438, 144, 961, 359, 330, 682, 626, 425, 259, 249, 801, 754, 1003, 230,
			377, 217, 878, 1007, 313, 2, 915, 550, 271, 437, 846, 548, 145, 715, 346, 251, 372, 99, 543,
			16, 47, 195, 679, 174, 905, 188, 804, 169, 785, 231, 726, 814, 339, 531, 420, 258, 1009, 134,
			972, 458, 234, 690, 260, 666, 646, 142, 184, 91, 628, 987, 10, 210, 926, 348, 386, 161, 60,
			409, 680, 204, 164, 444, 708, 276, 68, 383, 491, 382, 42, 816, 483, 699, 150, 9, 565, 555, 433,
			593, 86, 952, 839, 618, 751, 889, 108, 361, 595, 677, 407, 856, 255, 604, 85, 648, 928, 824,
			213, 192, 267, 902, 792, 656, 631, 403, 389, 493, 333, 756, 602, 925, 113, 632, 354, 37, 873,
			577, 56, 278, 930, 367, 428, 332, 317, 530, 364, 800, 774, 497, 1023, 12, 137, 845, 653, 101,
			888, 542, 167, 48, 158, 1002, 745, 292, 944, 456, 990, 574, 25, 1018, 937, 298, 966, 430, 400,
			349, 860, 689, 320, 117, 778, 104, 314, 786, 205, 606, 440, 936, 457, 932, 934, 948, 168, 445,
			931, 757, 291, 571, 919, 360, 284, 509, 296, 245, 836, 166, 3, 257, 50, 282, 151, 810, 344,
			947, 236, 946, 865, 752, 77, 610, 967, 795, 131, 302, 760, 781, 190, 938, 61, 1022, 652, 138,
			984, 832, 202, 140, 985, 5, 657, 997, 401, 319, 431, 662, 405, 275, 650, 651, 887, 310, 1004,
			368, 208, 596, 248, 758, 8, 126, 730, 489, 343, 337, 506, 515, 432, 232, 250, 532, 954, 524,
			115, 229, 522, 908, 729, 186, 561, 995, 156, 196, 118, 805, 399, 918, 991, 849, 273, 747, 640,
			143, 321, 624, 268, 306, 30, 722, 540, 534, 710, 130, 155, 883, 716, 525, 426, 812, 345, 929,
			975, 472, 837, 605, 664, 391, 581, 272, 746, 112, 659, 665, 780, 240, 841, 474, 563, 36, 579,
			286, 436, 907, 369, 201, 402, 962, 106, 749, 172, 494, 88, 466, 473, 414, 597, 374, 942, 308,
			766, 459, 821, 592, 881, 380, 759, 866, 779, 809, 876, 541, 829, 528, 999, 221, 661, 927, 413,
			977, 182, 583, 733, 892, 741, 570, 351, 617, 956, 72, 709, 850, 732, 770, 870, 95, 935, 223,
			179, 861, 917, 447, 385, 132, 827, 923, 75, 465, 612, 460, 725, 492, 553, 1008, 910, 981, 503,
			165, 895, 834, 1000, 180, 638, 906, 510, 274, 776, 971, 564, 738, 903, 654, 864, 959, 1015,
			453, 535, 237, 197, 1006, 790, 514, 842, 970, 705, 707, 1010, 203,

			// 1k Block repeats here

			127, 71, 882, 898, 798, 463, 517, 451, 454, 634, 578, 695, 728, 742, 325, 350, 684, 153, 340,
			311, 992, 706, 218, 285, 96, 486, 160, 98, 686, 288, 193, 119, 410, 246, 536, 415, 953, 417,
			784, 573, 734, 1, 136, 381, 177, 678, 773, 22, 301, 51, 874, 844, 775, 744, 633, 468, 1019,
			287, 475, 78, 294, 724, 519, 17, 323, 191, 187, 446, 262, 212, 170, 33, 7, 227, 566, 526, 264,
			556, 717, 477, 815, 671, 225, 207, 692, 663, 969, 393, 658, 877, 353, 788, 128, 303, 614, 501,
			490, 387, 53, 941, 951, 736, 539, 102, 163, 175, 584, 988, 35, 347, 442, 649, 642, 198, 727,
			939, 913, 811, 894, 858, 181, 412, 307, 830, 154, 479, 704, 326, 681, 619, 698, 621, 552, 598,
			74, 890, 299, 922, 701, 481, 867, 214, 817, 731, 768, 673, 315, 338, 576, 222, 484, 305, 623,
			239, 269, 46, 748, 608, 546, 537, 125, 667, 998, 714, 529, 823, 247, 289, 771, 808, 973, 735,
			516, 974, 702, 636, 357, 455, 600, 80, 336, 696, 963, 297, 92, 980, 670, 958, 625, 712, 406,
			173, 19, 763, 470, 793, 283, 655, 59, 421, 1016, 219, 13, 105, 840, 111, 38, 408, 945, 242,
			559, 206, 443, 331, 737, 580, 767, 1020, 220, 31, 968, 15, 527, 833, 139, 129, 859, 739, 418,
			783, 933, 49, 789, 178, 124, 772, 627, 0, 23, 388, 950, 976, 940, 485, 685, 21, 523, 723, 244,
			637, 488, 835, 379, 342, 452, 862, 295, 765, 897, 507, 370, 567, 416, 100, 914, 300, 120, 392,
			694, 94, 265, 791, 171, 200, 787, 441, 868, 672, 769, 983, 911, 427, 82, 69, 224, 176, 920,
			500, 462, 263, 513, 797, 293, 322, 645, 469, 635, 40, 215, 687, 960, 818, 826, 34, 603, 316,
			994, 611, 511, 93, 899, 114, 73, 241, 585, 327, 674, 280, 957, 471, 24, 502, 355, 159, 1017,
			855, 270, 538, 521, 162, 880, 334, 986, 740, 719, 266, 820, 97, 41, 52, 750, 893, 838, 616, 83,
			896, 777, 464, 562, 183, 362, 411, 478, 398, 384, 912, 599, 587, 609, 822, 243, 504, 753, 857,
			157, 964, 65, 261, 81, 371, 435, 924, 885, 884, 863, 613, 721, 669, 121, 639, 989, 487, 238,
			448, 216, 852, 643, 713, 676, 277, 879, 133, 123, 304, 547, 396, 70, 141, 909, 848, 900, 318,
			146, 356, 802, 4, 807, 558, 764, 545, 588, 872, 554, 467, 544, 505, 149, 62, 901, 64, 45, 813,
			27, 109, 718, 803, 853, 996, 1014, 476, 575, 28, 199, 688, 6, 482, 703, 560, 395, 66, 341, 794,
			422, 376, 601, 76, 14, 569, 480, 39, 1011, 1001, 854, 55, 89, 335, 761, 363, 419, 252, 799,
			358, 324, 1012, 152, 312, 496, 235, 916, 582, 615, 979, 1005, 891, 1013, 641, 18, 148, 185,
			512, 378, 58, 211, 495, 594, 87, 762, 366, 660, 449, 520, 424, 886, 819, 281, 147, 290, 390,
			32, 572, 993, 720, 683, 309, 254, 607, 568, 256, 533, 394, 620, 429, 67, 831, 103, 423, 668,
			693, 518, 551, 697, 253, 949, 54, 875, 116, 434, 743, 644, 590, 279, 843, 589, 11, 647, 586,
			806, 549, 375, 226, 851, 499, 450, 978, 29, 982, 189, 107, 508, 373, 796, 20, 700, 110, 26,
			461, 782, 591, 828, 57, 904, 847, 328, 122, 630, 711, 44, 397, 404, 209, 365, 84, 194, 1021,
			675, 135, 965, 329, 557, 691, 79, 352, 498, 629, 869, 90, 921, 233, 622, 871, 755, 439, 955,
			228, 63, 825, 43, 943, 438, 144, 961, 359, 330, 682, 626, 425, 259, 249, 801, 754, 1003, 230,
			377, 217, 878, 1007, 313, 2, 915, 550, 271, 437, 846, 548, 145, 715, 346, 251, 372, 99, 543,
			16, 47, 195, 679, 174, 905, 188, 804, 169, 785, 231, 726, 814, 339, 531, 420, 258, 1009, 134,
			972, 458, 234, 690, 260, 666, 646, 142, 184, 91, 628, 987, 10, 210, 926, 348, 386, 161, 60,
			409, 680, 204, 164, 444, 708, 276, 68, 383, 491, 382, 42, 816, 483, 699, 150, 9, 565, 555, 433,
			593, 86, 952, 839, 618, 751, 889, 108, 361, 595, 677, 407, 856, 255, 604, 85, 648, 928, 824,
			213, 192, 267, 902, 792, 656, 631, 403, 389, 493, 333, 756, 602, 925, 113, 632, 354, 37, 873,
			577, 56, 278, 930, 367, 428, 332, 317, 530, 364, 800, 774, 497, 1023, 12, 137, 845, 653, 101,
			888, 542, 167, 48, 158, 1002, 745, 292, 944, 456, 990, 574, 25, 1018, 937, 298, 966, 430, 400,
			349, 860, 689, 320, 117, 778, 104, 314, 786, 205, 606, 440, 936, 457, 932, 934, 948, 168, 445,
			931, 757, 291, 571, 919, 360, 284, 509, 296, 245, 836, 166, 3, 257, 50, 282, 151, 810, 344,
			947, 236, 946, 865, 752, 77, 610, 967, 795, 131, 302, 760, 781, 190, 938, 61, 1022, 652, 138,
			984, 832, 202, 140, 985, 5, 657, 997, 401, 319, 431, 662, 405, 275, 650, 651, 887, 310, 1004,
			368, 208, 596, 248, 758, 8, 126, 730, 489, 343, 337, 506, 515, 432, 232, 250, 532, 954, 524,
			115, 229, 522, 908, 729, 186, 561, 995, 156, 196, 118, 805, 399, 918, 991, 849, 273, 747, 640,
			143, 321, 624, 268, 306, 30, 722, 540, 534, 710, 130, 155, 883, 716, 525, 426, 812, 345, 929,
			975, 472, 837, 605, 664, 391, 581, 272, 746, 112, 659, 665, 780, 240, 841, 474, 563, 36, 579,
			286, 436, 907, 369, 201, 402, 962, 106, 749, 172, 494, 88, 466, 473, 414, 597, 374, 942, 308,
			766, 459, 821, 592, 881, 380, 759, 866, 779, 809, 876, 541, 829, 528, 999, 221, 661, 927, 413,
			977, 182, 583, 733, 892, 741, 570, 351, 617, 956, 72, 709, 850, 732, 770, 870, 95, 935, 223,
			179, 861, 917, 447, 385, 132, 827, 923, 75, 465, 612, 460, 725, 492, 553, 1008, 910, 981, 503,
			165, 895, 834, 1000, 180, 638, 906, 510, 274, 776, 971, 564, 738, 903, 654, 864, 959, 1015,
			453, 535, 237, 197, 1006, 790, 514, 842, 970, 705, 707, 1010, 203,

			// 2k block overlaps by two items here . . . (to allow for over-runs caused by taking
			//    "next item in sequence")

			127, 71
		};
		//Make a copy of all ints as floats. This is used
		//when passed to the OpenCL since the global shared array
		//is of type T.
		m_PFloats.clear();
		m_PFloats.reserve(p.size());

		for (size_t i = 0; i < p.size(); i++)
			m_PFloats.push_back(T(p[i]));

		return p;
	}

	/// <summary>
	/// Initializes the gradient texture.
	/// </summary>
	/// <returns>A copy of the locally declared vector</returns>
	std::vector<v3T> InitGrad()
	{
		std::vector<v3T> g =
		{
			v3T{ 0.79148875, 0.11986299, -0.59931496 }, v3T{ 0.51387411, -0.61170974, 0.60145208 }, v3T{ -0.95395128, -0.21599571, 0.20814132 }, v3T{ 0.59830026, 0.67281067, 0.43515813 },
			v3T{ -0.93971346, 0.16019818, -0.30211777 }, v3T{ -0.74549699, -0.35758846, 0.56246309 }, v3T{ -0.78850321, -0.29060783, 0.54204223 }, v3T{ 0.61332339, 0.38915256, 0.68730976 },
			v3T{ -0.64370632, -0.40843865, 0.64716307 }, v3T{ -0.23922684, 0.70399949, -0.66869667 }, v3T{ -0.82882802, -0.00130741, 0.55950192 }, v3T{ 0.07987672, 0.62439350, -0.77701510 },
			v3T{ -0.46863456, -0.57517073, 0.67049257 }, v3T{ 0.30792870, 0.42464616, -0.85138449 }, v3T{ -0.06972001, 0.30439513, 0.94999091 }, v3T{ 0.58798450, -0.00151777, 0.80887077 },
			v3T{ -0.32757867, 0.51578941, 0.79161449 }, v3T{ -0.44745031, 0.86883688, 0.21192142 }, v3T{ -0.38042636, 0.71222019, 0.58993066 }, v3T{ -0.32616370, 0.61421101, -0.71858339 },
			v3T{ 0.45483340, 0.19928843, -0.86799234 }, v3T{ -0.81020233, -0.05930352, 0.58314259 }, v3T{ 0.81994145, 0.39825895, 0.41120046 }, v3T{ 0.49257662, 0.74240487, 0.45409612 },
			v3T{ 0.95124863, -0.26667257, -0.15495734 }, v3T{ -0.95745656, 0.09203090, -0.27350914 }, v3T{ 0.20842499, -0.82482150, -0.52557446 }, v3T{ 0.46829293, -0.47740985, -0.74349282 },
			v3T{ -0.65000311, -0.74754355, 0.13665502 }, v3T{ 0.83566743, 0.53294928, -0.13275921 }, v3T{ 0.90454761, -0.35449497, -0.23691126 }, v3T{ -0.64270969, 0.21532175, 0.73522839 },
			v3T{ -0.39693478, -0.17553935, -0.90090439 }, v3T{ 0.45073049, 0.65155528, 0.61017845 }, v3T{ 0.69618384, -0.07989842, 0.71340333 }, v3T{ 0.09059934, 0.85274641, -0.51440773 },
			v3T{ -0.00560267, 0.69197466, 0.72190005 }, v3T{ 0.23586856, -0.95830502, 0.16129945 }, v3T{ 0.20354340, -0.96925430, -0.13826128 }, v3T{ -0.45516395, 0.63885905, 0.62022970 },
			v3T{ 0.80792021, 0.47917579, 0.34300946 }, v3T{ 0.40886670, -0.32579857, -0.85245722 }, v3T{ -0.83819701, -0.30910810, 0.44930831 }, v3T{ -0.57602641, -0.75801200, 0.30595978 },
			v3T{ -0.16591524, -0.96579983, -0.19925569 }, v3T{ 0.27174061, 0.93638167, -0.22214053 }, v3T{ -0.45758922, 0.73185326, -0.50497812 }, v3T{ -0.18029934, -0.78067110, -0.59836843 },
			v3T{ 0.14087163, -0.39189764, -0.90915974 }, v3T{ -0.03534787, -0.02750024, 0.99899663 }, v3T{ 0.91016878, 0.06772570, 0.40866370 }, v3T{ 0.70142578, 0.70903193, 0.07263332 },
			v3T{ -0.49486157, -0.54111502, -0.67993129 }, v3T{ -0.26972486, -0.84418773, -0.46324462 }, v3T{ 0.91931005, 0.03121901, 0.39229378 }, v3T{ -0.15332070, -0.87495538, 0.45928842 },
			v3T{ -0.59010107, -0.66883868, 0.45214549 }, v3T{ 0.51964273, -0.78565398, -0.33573688 }, v3T{ -0.25845001, 0.87348329, -0.41259003 }, v3T{ -0.64741807, -0.59846669, 0.47189773 },
			v3T{ -0.79348688, -0.32782128, -0.51274923 }, v3T{ -0.86280237, -0.14342378, -0.48476972 }, v3T{ 0.19469709, -0.76349966, 0.61576076 }, v3T{ 0.39371236, -0.70742193, -0.58697938 },
			v3T{ 0.62103834, -0.50000004, -0.60358209 }, v3T{ -0.19652824, -0.51508695, 0.83430335 }, v3T{ -0.96016549, -0.26826630, -0.07820118 }, v3T{ 0.52655683, 0.84118729, 0.12305219 },
			v3T{ 0.56222101, 0.70557745, -0.43135599 }, v3T{ 0.06395307, 0.99025162, -0.12374061 }, v3T{ -0.65379289, 0.52521996, 0.54470070 }, v3T{ 0.81206590, -0.38643765, 0.43728128 },
			v3T{ -0.69449067, -0.71926243, -0.01855435 }, v3T{ 0.33968533, 0.75504287, 0.56082452 }, v3T{ -0.52402654, -0.70537870, -0.47732282 }, v3T{ -0.65379327, -0.46369816, 0.59794512 },
			v3T{ -0.08582021, -0.01217948, 0.99623619 }, v3T{ -0.66287577, 0.49604924, 0.56083051 }, v3T{ 0.70911302, 0.68748287, -0.15660789 }, v3T{ -0.58662137, -0.46475685, 0.66323181 },
			v3T{ -0.76681755, 0.63310950, -0.10565607 }, v3T{ 0.68601816, -0.59353001, 0.42083395 }, v3T{ 0.64792478, -0.72668696, 0.22829704 }, v3T{ 0.68756542, -0.69062543, 0.22425499 },
			v3T{ -0.46901797, -0.72307343, -0.50713604 }, v3T{ -0.71418521, -0.11738817, 0.69004312 }, v3T{ 0.50880449, -0.80611081, 0.30216445 }, v3T{ 0.27793962, -0.58372922, -0.76289565 },
			v3T{ -0.39417207, 0.91575060, -0.07764800 }, v3T{ -0.84724113, -0.47860304, 0.23048124 }, v3T{ 0.67628991, 0.54362408, -0.49709638 }, v3T{ 0.65073821, -0.09420630, 0.75343544 },
			v3T{ 0.66910202, 0.73566783, -0.10533437 }, v3T{ 0.72191995, -0.00305613, 0.69196983 }, v3T{ -0.00313125, 0.06634333, 0.99779194 }, v3T{ -0.06908811, 0.28990653, -0.95455803 },
			v3T{ 0.17507626, 0.73870621, 0.65089280 }, v3T{ -0.57470594, 0.75735703, 0.31003777 }, v3T{ -0.91870733, 0.08883536, 0.38481830 }, v3T{ -0.27399536, 0.39846316, 0.87530203 },
			v3T{ 0.99772699, -0.05473919, 0.03929993 }, v3T{ 0.22663907, 0.97393801, -0.00891541 }, v3T{ 0.62338001, 0.59656797, -0.50547405 }, v3T{ 0.59177247, 0.49473684, -0.63642816 },
			v3T{ -0.24457664, -0.31345545, 0.91756632 }, v3T{ -0.44691491, -0.89198404, -0.06805539 }, v3T{ -0.83115967, -0.44685014, 0.33090566 }, v3T{ -0.39940345, 0.67719937, -0.61796270 },
			v3T{ 0.55460272, -0.63265953, -0.54051619 }, v3T{ 0.82284412, 0.14794174, -0.54867185 }, v3T{ -0.39887172, -0.82890906, -0.39218761 }, v3T{ 0.28591109, 0.71270085, 0.64055628 },
			v3T{ -0.15438831, 0.66966606, 0.72643762 }, v3T{ -0.75134796, 0.54289699, 0.37515211 }, v3T{ 0.32016243, 0.77691605, -0.54212311 }, v3T{ 0.50884942, 0.15171482, -0.84738119 },
			v3T{ 0.08945627, 0.73684807, 0.67011379 }, v3T{ -0.68792851, -0.71885270, -0.10002580 }, v3T{ 0.02292266, -0.07249674, 0.99710520 }, v3T{ 0.94083723, -0.10191422, 0.32316993 },
			v3T{ -0.81053204, 0.43703808, 0.38991733 }, v3T{ -0.19558496, -0.07485841, 0.97782552 }, v3T{ 0.68911052, -0.49915226, -0.52533200 }, v3T{ 0.19796974, 0.93342057, 0.29922235 },
			v3T{ -0.79540501, -0.26473293, 0.54520395 }, v3T{ -0.27945416, -0.91288360, 0.29757168 }, v3T{ 0.82074194, 0.43648314, 0.36859889 }, v3T{ -0.20594999, -0.70696486, -0.67659832 },
			v3T{ -0.05687654, -0.70968577, 0.70221874 }, v3T{ -0.26280466, 0.69993747, -0.66409430 }, v3T{ -0.54551347, -0.78469719, 0.29438983 }, v3T{ 0.90609571, 0.39319111, 0.15617717 },
			v3T{ 0.69129692, 0.67317351, 0.26257571 }, v3T{ 0.98391565, -0.05206160, 0.17087883 }, v3T{ 0.63806303, 0.67740288, -0.36606134 }, v3T{ -0.50096077, 0.83542684, -0.22605378 },
			v3T{ 0.65237128, 0.35509583, 0.66956603 }, v3T{ -0.85711882, -0.19885856, 0.47518691 }, v3T{ 0.79383271, -0.12451513, 0.59525256 }, v3T{ -0.63301076, 0.07907192, 0.77009416 },
			v3T{ 0.57925311, -0.49077742, 0.65084818 }, v3T{ 0.14070842, 0.97298117, 0.18305403 }, v3T{ -0.59601232, 0.69646383, -0.39963413 }, v3T{ -0.68205637, -0.47455943, 0.55641033 },
			v3T{ 0.47997775, -0.84805982, -0.22453484 }, v3T{ 0.83562547, -0.48273957, 0.26209270 }, v3T{ 0.59180830, 0.36411758, 0.71915320 }, v3T{ 0.66057023, -0.66033264, 0.35722231 },
			v3T{ 0.53319130, 0.75511965, 0.38144639 }, v3T{ -0.21631797, -0.12712992, 0.96801060 }, v3T{ -0.23971441, 0.89928294, -0.36582400 }, v3T{ -0.72825564, 0.27377922, -0.62824252 },
			v3T{ 0.02135570, 0.73882696, 0.67355672 }, v3T{ 0.48112026, 0.78759215, 0.38499597 }, v3T{ -0.58250985, -0.09956878, 0.80670213 }, v3T{ 0.21323385, 0.36856735, 0.90481459 },
			v3T{ -0.36459960, -0.93062781, -0.03160697 }, v3T{ -0.68684541, 0.17314748, -0.70587771 }, v3T{ 0.68032531, -0.07909205, -0.72863017 }, v3T{ 0.25007484, -0.61882132, 0.74466284 },
			v3T{ 0.77055613, 0.59380162, 0.23160935 }, v3T{ 0.67996118, -0.03835970, 0.73224403 }, v3T{ 0.43079959, 0.38901749, -0.81429547 }, v3T{ 0.76815116, -0.63831184, 0.05001794 },
			v3T{ -0.13601015, 0.75596033, -0.64033211 }, v3T{ 0.36884321, -0.45188838, -0.81225093 }, v3T{ 0.79562623, -0.43647179, 0.42008485 }, v3T{ -0.65875496, 0.39126701, -0.64261344 },
			v3T{ -0.68899899, 0.44217527, 0.57424858 }, v3T{ 0.25292617, 0.96620732, -0.04971687 }, v3T{ -0.68558843, -0.70460233, 0.18304118 }, v3T{ 0.86382379, 0.29507865, 0.40833448 },
			v3T{ 0.13627838, 0.31500179, 0.93925613 }, v3T{ 0.67187940, 0.64336667, 0.36695693 }, v3T{ 0.37977583, 0.31123423, 0.87115072 }, v3T{ -0.03326050, -0.99451574, -0.09915731 },
			v3T{ -0.66427749, -0.01424397, -0.74735033 }, v3T{ 0.68859558, 0.44744486, -0.57063931 }, v3T{ -0.56738045, 0.30154774, -0.76625608 }, v3T{ -0.58488004, 0.63357146, 0.50646080 },
			v3T{ 0.38842469, 0.92016339, 0.04925032 }, v3T{ 0.15316057, -0.97495961, -0.16123153 }, v3T{ 0.57623375, 0.51659393, 0.63331301 }, v3T{ 0.32392581, -0.79816566, -0.50794059 },
			v3T{ 0.73136440, -0.54179646, 0.41420129 }, v3T{ -0.58929886, -0.58690534, -0.55521975 }, v3T{ 0.64030162, 0.32487137, -0.69604054 }, v3T{ 0.80502987, -0.00635101, 0.59320028 },
			v3T{ 0.46595373, 0.62005710, -0.63120227 }, v3T{ 0.83612498, 0.53677947, 0.11297261 }, v3T{ -0.60753284, -0.29028728, -0.73934913 }, v3T{ -0.45583848, 0.84488003, 0.27998037 },
			v3T{ -0.27320563, -0.39709327, 0.87617100 }, v3T{ 0.84893256, -0.09000823, 0.52078021 }, v3T{ -0.35708766, -0.73203774, 0.58018027 }, v3T{ 0.10507148, -0.71032871, 0.69598355 },
			v3T{ 0.68468508, 0.26788814, -0.67782172 }, v3T{ -0.94602428, -0.13594737, -0.29420466 }, v3T{ 0.27104088, 0.95431757, 0.12575696 }, v3T{ -0.55840113, 0.14909310, 0.81606337 },
			v3T{ 0.47553129, 0.80729730, 0.34948685 }, v3T{ -0.01891509, -0.97526220, 0.22024047 }, v3T{ -0.65760518, -0.45924250, -0.59720327 }, v3T{ -0.70549425, 0.70862555, 0.01129989 },
			v3T{ -0.88864223, 0.43707946, -0.13883994 }, v3T{ 0.49252849, -0.43814774, 0.75195894 }, v3T{ -0.01398277, 0.69598571, 0.71791947 }, v3T{ -0.67265622, 0.27228276, -0.68803758 },
			v3T{ -0.91724038, -0.01083918, -0.39818663 }, v3T{ -0.24468025, 0.75690032, 0.60599792 }, v3T{ -0.49070434, -0.48530058, 0.72366608 }, v3T{ 0.67110346, -0.55453760, -0.49204492 },
			v3T{ -0.95532877, -0.26328211, -0.13427388 }, v3T{ -0.66012945, 0.41730904, 0.62456567 }, v3T{ 0.96822786, -0.03273592, 0.24791766 }, v3T{ 0.91952853, 0.23575545, -0.31446248 },
			v3T{ 0.63712542, 0.06762652, 0.76778763 }, v3T{ -0.21680947, 0.65843559, 0.72073312 }, v3T{ 0.06143588, 0.47272235, -0.87906724 }, v3T{ 0.70541616, -0.21884659, 0.67416186 },
			v3T{ -0.04396589, -0.67487644, -0.73661984 }, v3T{ -0.65032618, 0.75012744, 0.11993615 }, v3T{ -0.78840054, 0.58187068, -0.19962741 }, v3T{ 0.99318416, 0.11467779, 0.02083796 },
			v3T{ 0.76775820, 0.46845611, -0.43714554 }, v3T{ -0.70891635, -0.54302381, -0.45006972 }, v3T{ 0.55548849, -0.71825576, -0.41897638 }, v3T{ -0.62167600, 0.77500231, 0.11353575 },
			v3T{ 0.38413022, -0.79687865, 0.46629218 }, v3T{ -0.56271512, 0.54186596, -0.62428597 }, v3T{ 0.62019121, -0.70563211, -0.34270424 }, v3T{ 0.85913131, 0.50529005, 0.08108862 },
			v3T{ 0.54973106, -0.66129569, -0.51037612 }, v3T{ -0.74254469, -0.49670185, -0.44934914 }, v3T{ -0.75780366, 0.59195518, -0.27444976 }, v3T{ -0.40050287, 0.04302113, -0.91528500 },
			v3T{ -0.60859484, 0.35063171, 0.71180736 }, v3T{ -0.57297537, 0.81938865, -0.01736289 }, v3T{ 0.98721933, 0.09373543, -0.12888621 }, v3T{ 0.30397213, 0.87942861, 0.36634172 },
			v3T{ 0.32615126, -0.64515144, -0.69094498 }, v3T{ 0.83015604, 0.30783918, 0.46483974 }, v3T{ 0.42822875, -0.04288671, -0.90265213 }, v3T{ 0.16585965, 0.53714643, 0.82702133 },
			v3T{ -0.37193298, 0.88497229, 0.28016051 }, v3T{ 0.73544877, 0.67744273, 0.01365471 }, v3T{ -0.66150496, 0.09327263, -0.74411787 }, v3T{ 0.41664753, -0.23786298, -0.87739731 },
			v3T{ -0.78513086, -0.42653313, 0.44904233 }, v3T{ 0.08029855, 0.84803303, 0.52382451 }, v3T{ -0.09507221, 0.50524394, -0.85772364 }, v3T{ 0.66939507, -0.17805679, 0.72125309 },
			v3T{ -0.76923153, 0.41652205, -0.48455364 }, v3T{ 0.51989556, 0.79632686, 0.30914743 }, v3T{ 0.85617969, -0.51024476, 0.08128121 }, v3T{ 0.71830013, 0.03208003, 0.69499337 },
			v3T{ -0.96000528, -0.11640072, -0.25463844 }, v3T{ 0.66084196, -0.19355993, 0.72513617 }, v3T{ -0.57661819, -0.54757438, 0.60636109 }, v3T{ 0.65123443, -0.64818909, -0.39464494 },
			v3T{ 0.36952748, -0.22540306, -0.90146708 }, v3T{ 0.34048182, -0.33515083, 0.87849078 }, v3T{ 0.11132435, -0.75280467, 0.64876191 }, v3T{ 0.67563520, 0.64934616, -0.34909404 },
			v3T{ 0.23316576, 0.69276343, -0.68243135 }, v3T{ 0.30368064, -0.87532007, 0.37628825 }, v3T{ -0.27080673, -0.74246398, 0.61270789 }, v3T{ -0.21655683, -0.49565083, -0.84109060 },
			v3T{ -0.98776592, -0.14473189, 0.05806181 }, v3T{ 0.64562720, 0.38598860, 0.65892209 }, v3T{ -0.63746045, -0.57205546, 0.51613635 }, v3T{ 0.06117405, -0.78423981, -0.61743474 },
			v3T{ 0.74829362, 0.59119862, 0.30090006 }, v3T{ -0.42571462, 0.51302568, -0.74536683 }, v3T{ -0.56331794, 0.48608227, -0.66812943 }, v3T{ -0.75919788, -0.64885422, 0.05105673 },
			v3T{ 0.14385006, -0.53933953, 0.82971081 }, v3T{ -0.77031548, -0.28344830, 0.57120148 }, v3T{ -0.98358057, 0.17900745, 0.02292584 }, v3T{ -0.25051205, 0.10358351, 0.96255606 },
			v3T{ -0.32867861, -0.83176115, -0.44737430 }, v3T{ -0.36281449, -0.92995082, -0.05964161 }, v3T{ -0.53796595, -0.03614791, 0.84219117 }, v3T{ 0.92960703, 0.10461247, 0.35339354 },
			v3T{ 0.64021850, 0.61360003, 0.46218532 }, v3T{ 0.22343529, 0.69409296, 0.68433299 }, v3T{ 0.01781074, 0.89088149, 0.45388648 }, v3T{ -0.63004672, -0.26934609, 0.72835007 },
			v3T{ 0.48560056, -0.35192051, -0.80021500 }, v3T{ 0.62050161, 0.57366872, 0.53467931 }, v3T{ 0.00265452, 0.71539198, -0.69871830 }, v3T{ 0.64229521, 0.41380752, 0.64515130 },
			v3T{ 0.23080049, -0.43573115, 0.86998247 }, v3T{ 0.14620517, 0.61171896, -0.77744708 }, v3T{ -0.27436021, -0.61900378, 0.73590814 }, v3T{ 0.69959023, 0.71050058, 0.07591065 },
			v3T{ 0.70362024, 0.62044755, -0.34635731 }, v3T{ -0.29622242, -0.71700405, -0.63099721 }, v3T{ 0.31094340, -0.84299864, -0.43893905 }, v3T{ 0.07704196, -0.46344069, -0.88277248 },
			v3T{ -0.94533514, -0.04418570, 0.32309301 }, v3T{ 0.65845027, -0.36172634, -0.65999795 }, v3T{ 0.76069300, -0.18013255, 0.62361721 }, v3T{ 0.18607691, -0.45751624, -0.86951382 },
			v3T{ -0.67626808, -0.39178398, -0.62383235 }, v3T{ -0.58782719, 0.55645189, -0.58721418 }, v3T{ 0.37531624, 0.80640206, 0.45700485 }, v3T{ 0.32610790, -0.50457786, 0.79940905 },
			v3T{ 0.62915643, 0.76094546, -0.15850616 }, v3T{ 0.62803678, -0.75273385, -0.19738681 }, v3T{ 0.42539119, -0.89094420, 0.15893638 }, v3T{ 0.17668676, -0.40626331, 0.89651096 },
			v3T{ 0.02778178, -0.78957083, -0.61303024 }, v3T{ -0.25950053, -0.16244258, 0.95198313 }, v3T{ -0.44117714, 0.73727502, -0.51165249 }, v3T{ -0.30827444, 0.94136275, 0.13712420 },
			v3T{ 0.97572111, -0.04258044, -0.21483768 }, v3T{ 0.55607688, 0.60474525, -0.57014181 }, v3T{ -0.67430479, 0.12532345, 0.72774109 }, v3T{ -0.31325824, -0.81393777, -0.48925921 },
			v3T{ -0.34811982, -0.70956566, 0.61264114 }, v3T{ 0.22583632, 0.72502572, -0.65064250 }, v3T{ 0.76936493, 0.63742123, -0.04209247 }, v3T{ -0.55303394, -0.38417341, -0.73929984 },
			v3T{ -0.20953448, -0.92686077, -0.31148742 }, v3T{ -0.18786352, 0.39920999, 0.89740664 }, v3T{ 0.46307517, -0.88470611, 0.05344618 }, v3T{ -0.70328479, 0.30353783, 0.64284935 },
			v3T{ 0.85916171, 0.15710234, 0.48699077 }, v3T{ -0.26398391, 0.42122173, 0.86768932 }, v3T{ 0.82468427, 0.55134621, 0.12614757 }, v3T{ 0.05993298, 0.63414584, 0.77088721 },
			v3T{ -0.57291678, 0.81909656, -0.02910645 }, v3T{ 0.64075141, 0.74416542, -0.18882655 }, v3T{ 0.67112660, -0.55747979, -0.48867716 }, v3T{ 0.89932863, 0.23426637, -0.36922525 },
			v3T{ 0.59146340, -0.44386974, 0.67316469 }, v3T{ 0.46684506, 0.19781570, -0.86193076 }, v3T{ 0.18536399, 0.76259887, 0.61974443 }, v3T{ 0.84144446, -0.53500771, -0.07574940 },
			v3T{ 0.31212800, 0.82898453, -0.46406977 }, v3T{ -0.88440729, -0.27020677, -0.38054178 }, v3T{ 0.20051055, 0.77523319, 0.59900670 }, v3T{ 0.48749115, 0.44082691, -0.75367368 },
			v3T{ 0.24971103, -0.88242146, 0.39871892 }, v3T{ -0.29777449, -0.95158243, -0.07629705 }, v3T{ -0.37776905, -0.58777023, 0.71541366 }, v3T{ 0.22179317, 0.14730715, -0.96390269 },
			v3T{ 0.58348153, 0.68630504, 0.43420582 }, v3T{ -0.96759942, 0.14572096, 0.20619593 }, v3T{ -0.15181654, 0.47495708, 0.86681458 }, v3T{ 0.26580537, 0.74350537, -0.61363447 },
			v3T{ -0.39189499, 0.72950601, 0.56057051 }, v3T{ -0.01888074, 0.73557245, -0.67718290 }, v3T{ 0.73486517, 0.20569655, -0.64626783 }, v3T{ -0.26354754, -0.23595215, -0.93534447 },
			v3T{ -0.62584298, -0.65116585, 0.42930594 }, v3T{ -0.66666701, 0.61406968, 0.42246127 }, v3T{ 0.71799877, 0.67101619, 0.18497305 }, v3T{ 0.80098282, -0.45681211, -0.38697444 },
			v3T{ 0.13205975, 0.91574792, -0.37942847 }, v3T{ 0.68891728, 0.72389791, -0.03694308 }, v3T{ 0.50346408, 0.46323331, -0.72934136 }, v3T{ 0.84557323, 0.53378861, -0.00869685 },
			v3T{ 0.08666773, -0.81879883, 0.56750082 }, v3T{ -0.50044423, 0.65858460, -0.56198033 }, v3T{ 0.35669785, 0.32248792, -0.87679427 }, v3T{ -0.97346629, -0.22237373, -0.05397509 },
			v3T{ -0.53358835, -0.29312069, -0.79332448 }, v3T{ 0.12615748, 0.47083230, 0.87315591 }, v3T{ -0.97022570, 0.19065350, 0.14937651 }, v3T{ -0.57777643, 0.36008023, 0.73247295 },
			v3T{ 0.60132454, 0.72398065, 0.33802488 }, v3T{ 0.19047827, -0.94729649, -0.25757988 }, v3T{ -0.45904437, 0.69100108, 0.55838676 }, v3T{ 0.39148612, -0.51878308, 0.76000180 },
			v3T{ 0.04137949, -0.75662546, -0.65253786 }, v3T{ 0.20020542, -0.76439245, -0.61288006 }, v3T{ 0.07933739, -0.21074410, 0.97431643 }, v3T{ -0.40807425, 0.80614533, 0.42849166 },
			v3T{ -0.95397962, -0.09342040, -0.28494828 }, v3T{ -0.31365384, 0.14377778, -0.93858895 }, v3T{ 0.84618575, -0.39191761, 0.36106822 }, v3T{ -0.90177404, 0.07825801, -0.42506385 },
			v3T{ -0.19689944, -0.97296956, 0.12066831 }, v3T{ 0.61145370, 0.51715369, -0.59889601 }, v3T{ -0.57329050, -0.80450317, -0.15528251 }, v3T{ -0.27749150, -0.76245284, 0.58452044 },
			v3T{ -0.74877628, 0.66124357, 0.04572758 }, v3T{ 0.60284514, 0.58208119, 0.54567318 }, v3T{ 0.17695878, -0.67360184, 0.71759748 }, v3T{ -0.83953853, 0.41240184, 0.35369447 },
			v3T{ 0.37802442, -0.60322405, 0.70229501 }, v3T{ 0.51050450, -0.42970396, 0.74480847 }, v3T{ -0.48366785, -0.20902730, -0.84992529 }, v3T{ -0.87971286, -0.14820690, -0.45181855 },
			v3T{ -0.11520437, -0.59044778, -0.79881123 }, v3T{ 0.38877393, 0.92116844, -0.01742240 }, v3T{ 0.94330646, -0.27385756, -0.18754989 }, v3T{ -0.66585548, 0.46928680, -0.58000550 },
			v3T{ 0.20659390, -0.97226278, -0.10965425 }, v3T{ 0.70114934, 0.70875543, -0.07781609 }, v3T{ 0.50683262, 0.81003447, 0.29489803 }, v3T{ -0.75501572, 0.56485827, -0.33299610 },
			v3T{ -0.43930454, -0.48824131, 0.75407688 }, v3T{ -0.43442626, 0.51174617, 0.74120826 }, v3T{ -0.97139119, -0.22722375, 0.06905442 }, v3T{ -0.27189670, 0.51890879, -0.81043559 },
			v3T{ 0.34109465, 0.91412005, -0.21917797 }, v3T{ 0.23216825, -0.66497033, 0.70986785 }, v3T{ 0.87281521, 0.48669099, 0.03640737 }, v3T{ -0.60266004, -0.34235001, -0.72083101 },
			v3T{ -0.01994494, -0.52747354, 0.84933731 }, v3T{ -0.27000504, -0.77679344, -0.56893693 }, v3T{ -0.12330809, 0.85744248, -0.49958734 }, v3T{ -0.69270982, 0.61145042, -0.38246763 },
			v3T{ -0.60277814, 0.55015465, 0.57791727 }, v3T{ 0.64946165, -0.22132925, -0.72747023 }, v3T{ 0.24257305, 0.26557728, 0.93307397 }, v3T{ -0.66814908, 0.64881591, -0.36416303 },
			v3T{ -0.74538727, -0.44634982, -0.49514609 }, v3T{ 0.25115903, 0.38535072, -0.88793241 }, v3T{ -0.61584597, -0.69782826, -0.36574509 }, v3T{ 0.13745929, 0.92666227, 0.34985995 },
			v3T{ -0.50342245, -0.82980249, -0.24081874 }, v3T{ 0.11249648, 0.99333196, -0.02522230 }, v3T{ 0.83241096, 0.21922825, -0.50895085 }, v3T{ 0.50175590, 0.86108612, 0.08229039 },
			v3T{ -0.35527286, -0.56925625, -0.74143679 }, v3T{ 0.31441654, -0.91653449, 0.24719782 }, v3T{ 0.62936968, 0.70222610, 0.33282475 }, v3T{ 0.77755375, -0.56236234, -0.28135169 },
			v3T{ -0.80098254, -0.37712493, 0.46497715 }, v3T{ 0.59310190, -0.68181911, -0.42819720 }, v3T{ 0.15392285, -0.98282954, 0.10175390 }, v3T{ -0.96618662, 0.25781497, 0.00385483 },
			v3T{ 0.33750940, -0.86020836, 0.38226820 }, v3T{ -0.09597976, -0.40348179, -0.90993974 }, v3T{ -0.70910783, 0.60681107, -0.35909108 }, v3T{ 0.41726791, -0.90380775, 0.09496860 },
			v3T{ -0.03646000, 0.99581799, -0.08376873 }, v3T{ 0.35348135, -0.70899268, 0.61022972 }, v3T{ 0.66002017, 0.74115740, -0.12271547 }, v3T{ 0.18515044, 0.96534454, -0.18392727 },
			v3T{ -0.29364182, -0.88826809, -0.35320572 }, v3T{ 0.99692330, 0.02436644, -0.07449968 }, v3T{ -0.13529570, 0.35908874, 0.92344483 }, v3T{ -0.76888326, -0.29702475, 0.56621095 },
			v3T{ -0.31931644, 0.72859881, 0.60595444 }, v3T{ 0.52827199, -0.82385659, 0.20539968 }, v3T{ -0.83281688, -0.27413556, 0.48090097 }, v3T{ -0.76899198, 0.23377782, 0.59497837 },
			v3T{ -0.60599231, 0.54438401, -0.58001670 }, v3T{ -0.59616975, -0.18605791, 0.78100198 }, v3T{ -0.83753036, 0.32458912, -0.43952794 }, v3T{ 0.62016934, 0.71285793, 0.32745011 },
			v3T{ -0.62489231, 0.01790151, 0.78050570 }, v3T{ -0.44050813, -0.31396367, 0.84105850 }, v3T{ 0.82831903, 0.51349534, 0.22407615 }, v3T{ -0.54638365, -0.42878084, -0.71945250 },
			v3T{ -0.30690837, -0.54588407, -0.77962673 }, v3T{ -0.51419246, 0.49668914, 0.69921814 }, v3T{ 0.12759508, 0.79794754, 0.58906640 }, v3T{ 0.59812622, 0.53597438, 0.59579904 },
			v3T{ 0.75450428, 0.31026344, 0.57832507 }, v3T{ -0.34806954, -0.09710281, 0.93242621 }, v3T{ -0.40140375, -0.85287390, 0.33388792 }, v3T{ 0.57290191, 0.32347021, -0.75309390 },
			v3T{ -0.53362688, -0.81285892, 0.23345818 }, v3T{ -0.74679447, 0.64927639, 0.14400758 }, v3T{ -0.80251380, -0.59638095, 0.01736004 }, v3T{ -0.56868668, 0.61763086, -0.54325646 },
			v3T{ -0.72976559, 0.04179896, -0.68241852 }, v3T{ 0.57244144, -0.09255805, -0.81470474 }, v3T{ 0.97741613, 0.07186077, -0.19873032 }, v3T{ 0.72298477, 0.06613486, 0.68769121 },
			v3T{ -0.42596585, -0.65375247, -0.62542850 }, v3T{ 0.64840687, 0.16136696, -0.74399545 }, v3T{ 0.34352050, -0.92950264, 0.13423304 }, v3T{ 0.74687236, 0.45351768, -0.48631613 },
			v3T{ -0.51873425, -0.73762481, -0.43223191 }, v3T{ 0.29790392, 0.44209023, 0.84605525 }, v3T{ -0.67740274, 0.46717430, -0.56821977 }, v3T{ -0.36224935, -0.42773177, 0.82814307 },
			v3T{ -0.44192484, 0.73919980, 0.50821855 }, v3T{ -0.92680658, -0.18163204, -0.32869343 }, v3T{ -0.71384582, -0.70014113, 0.01505111 }, v3T{ 0.70600729, -0.70152253, 0.09705589 },
			v3T{ 0.90031692, -0.36943663, 0.23010002 }, v3T{ 0.25264659, -0.65121757, -0.71560141 }, v3T{ 0.96727807, 0.19056552, 0.16750499 }, v3T{ -0.65770755, -0.65887301, 0.36511251 },
			v3T{ 0.05208955, -0.10417910, 0.99319353 }, v3T{ -0.65282932, -0.40832320, 0.63803294 }, v3T{ -0.00628739, -0.99502463, -0.09943061 }, v3T{ -0.51900794, -0.62993523, 0.57776497 },
			v3T{ 0.83046729, -0.16527060, 0.53198657 }, v3T{ 0.66869945, -0.56606479, -0.48209097 }, v3T{ -0.54299772, -0.48639669, -0.68452300 }, v3T{ 0.52407156, -0.42268239, 0.73938393 },
			v3T{ 0.71446999, -0.30844019, -0.62801057 }, v3T{ -0.67320882, 0.39978543, 0.62206228 }, v3T{ -0.53289859, -0.05079670, -0.84465306 }, v3T{ 0.67708925, -0.71979254, 0.15313018 },
			v3T{ -0.61369683, 0.65230332, 0.44483321 }, v3T{ -0.26453665, -0.69129417, -0.67240816 }, v3T{ 0.85045794, 0.03075140, 0.52514345 }, v3T{ -0.76757885, -0.10940324, 0.63154861 },
			v3T{ 0.72754104, -0.17450402, -0.66350011 }, v3T{ -0.34075755, -0.67303082, 0.65644026 }, v3T{ 0.70044829, 0.13095479, -0.70158609 }, v3T{ 0.43950040, -0.88211196, 0.16946353 },
			v3T{ -0.35706397, 0.48041126, 0.80106825 }, v3T{ -0.77687193, 0.33320308, -0.53427120 }, v3T{ 0.51274543, 0.77662232, 0.36599165 }, v3T{ 0.33380578, 0.79591657, 0.50506486 },
			v3T{ -0.76587225, -0.03670574, 0.64194422 }, v3T{ -0.23491078, 0.43695339, -0.86826762 }, v3T{ 0.25698923, -0.62346599, 0.73840822 }, v3T{ 0.13009757, -0.93331414, -0.33466303 },
			v3T{ -0.54841950, 0.64297666, -0.53461861 }, v3T{ 0.69823865, 0.51710521, -0.49504039 }, v3T{ -0.64058874, -0.76638614, -0.04794108 }, v3T{ -0.99383538, 0.10829476, 0.02373760 },
			v3T{ 0.53702674, -0.26620457, -0.80046075 }, v3T{ 0.95618706, 0.14762618, 0.25280983 }, v3T{ 0.46882627, -0.32353926, -0.82190284 }, v3T{ 0.37771393, -0.17580406, -0.90907927 },
			v3T{ -0.38046162, 0.14393199, -0.91352752 }, v3T{ 0.99319923, -0.09757638, -0.06351493 }, v3T{ 0.50851715, 0.83898531, 0.19368521 }, v3T{ 0.32506349, -0.66811447, 0.66929574 },
			v3T{ -0.48035988, -0.63636898, -0.60356351 }, v3T{ -0.06435942, 0.26733173, 0.96145286 }, v3T{ 0.60598929, -0.04278909, 0.79432114 }, v3T{ -0.24869997, 0.88809619, -0.38656626 },
			v3T{ 0.37370464, 0.04464997, -0.92647246 }, v3T{ -0.48971589, -0.59472073, 0.63756224 }, v3T{ 0.69752714, 0.12358938, 0.70581978 }, v3T{ 0.52787180, 0.64468756, -0.55292794 },
			v3T{ -0.10489693, 0.16880171, -0.98005235 }, v3T{ -0.63336451, -0.45121552, -0.62869226 }, v3T{ 0.54866356, 0.65678858, 0.51729785 }, v3T{ -0.85968969, 0.49557488, -0.12385145 },
			v3T{ -0.47320716, -0.15150042, 0.86782637 }, v3T{ 0.19900943, -0.10259966, 0.97461200 }, v3T{ -0.52893938, 0.84740153, 0.04619294 }, v3T{ 0.65121421, -0.49243156, -0.57743503 },
			v3T{ 0.45693424, 0.73751862, 0.49726994 }, v3T{ -0.47661222, -0.77374319, -0.41732752 }, v3T{ -0.04808540, 0.90050093, 0.43218730 }, v3T{ 0.91129978, -0.31013948, 0.27082507 },
			v3T{ 0.58778939, -0.42668247, -0.68734686 }, v3T{ 0.82297839, -0.34772114, -0.44921773 }, v3T{ 0.29494223, -0.86544442, -0.40498769 }, v3T{ -0.39161493, 0.79055212, 0.47081322 },
			v3T{ 0.79434783, -0.59398096, -0.12727195 }, v3T{ 0.77174313, 0.29796481, 0.56180915 }, v3T{ 0.78482345, -0.44974833, 0.42635500 }, v3T{ -0.58988658, -0.54565594, 0.59522551 },
			v3T{ -0.97115669, 0.13450224, 0.19688532 }, v3T{ 0.42988246, 0.15513097, -0.88945796 }, v3T{ -0.30013401, -0.45617888, 0.83774722 }, v3T{ 0.50990724, -0.38026491, -0.77161727 },
			v3T{ -0.68923129, 0.29274099, -0.66276914 }, v3T{ -0.81531731, -0.42344291, -0.39490984 }, v3T{ 0.26048163, -0.96468719, -0.03908901 }, v3T{ 0.32147033, 0.32614689, -0.88897977 },
			v3T{ 0.70055924, -0.70700997, 0.09671429 }, v3T{ -0.58890140, -0.17999683, 0.78790626 }, v3T{ 0.70222863, 0.69308083, -0.16283095 }, v3T{ -0.75366081, -0.65098223, -0.09065052 },
			v3T{ -0.19053922, -0.78772343, -0.58582130 }, v3T{ -0.58846812, 0.34955220, 0.72905317 }, v3T{ -0.60563594, -0.40529546, -0.68479244 }, v3T{ -0.71315551, 0.69904447, 0.05240265 },
			v3T{ -0.45479055, 0.81186703, -0.36611129 }, v3T{ -0.29059626, 0.05377439, 0.95533352 }, v3T{ 0.56290473, 0.78501299, 0.25863657 }, v3T{ -0.43010366, -0.47609705, 0.76703484 },
			v3T{ 0.63372606, -0.06214270, -0.77105744 }, v3T{ 0.28788198, -0.78226752, -0.55243234 }, v3T{ -0.55506056, 0.67832002, -0.48144545 }, v3T{ -0.47229498, 0.84794057, -0.24069533 },
			v3T{ -0.27628326, 0.87423025, -0.39923556 }, v3T{ 0.97754921, -0.01429369, -0.21022189 }, v3T{ -0.78483628, 0.30941478, -0.53693064 }, v3T{ -0.35769150, -0.53057471, 0.76847073 },
			v3T{ 0.56804560, 0.59946775, -0.56388173 }, v3T{ 0.80328735, -0.57298006, -0.16255243 }, v3T{ -0.34327107, -0.35133498, -0.87105034 }, v3T{ 0.80357102, -0.01979284, -0.59487970 },
			v3T{ -0.87804782, 0.46346126, 0.11931336 }, v3T{ -0.11872912, -0.93845057, 0.32436695 }, v3T{ 0.68065237, 0.69467363, 0.23268195 }, v3T{ -0.71974506, -0.36713686, 0.58921776 },
			v3T{ 0.52822234, 0.82314813, -0.20834663 }, v3T{ -0.67654042, -0.73158271, 0.08414148 }, v3T{ -0.39062516, 0.89358947, -0.22115571 }, v3T{ -0.62142505, 0.43386674, -0.65237302 },
			v3T{ -0.48099381, -0.18611372, -0.85674188 }, v3T{ 0.05036514, -0.74987003, 0.65966528 }, v3T{ -0.49984895, -0.80920390, -0.30877188 }, v3T{ 0.50496868, 0.85618105, 0.10936472 },
			v3T{ -0.54084761, 0.24485715, 0.80469176 }, v3T{ -0.81973873, -0.50777759, 0.26493457 }, v3T{ 0.72082268, -0.43713926, -0.53788839 }, v3T{ 0.91725234, -0.15187152, 0.36821621 },
			v3T{ -0.17151325, 0.57985483, 0.79646192 }, v3T{ -0.74076471, 0.06061813, -0.66902398 }, v3T{ 0.32541463, -0.08200506, 0.94200875 }, v3T{ -0.10818362, 0.99402161, -0.01474260 },
			v3T{ -0.61710380, -0.78296663, 0.07839742 }, v3T{ -0.38878719, -0.57916742, 0.71652608 }, v3T{ 0.37911419, 0.92170992, 0.08199541 }, v3T{ -0.60810067, -0.43108035, 0.66662082 },
			v3T{ -0.11745691, 0.38395577, 0.91585034 }, v3T{ 0.47694470, -0.81050760, 0.34000174 }, v3T{ 0.40287244, 0.88894800, 0.21786522 }, v3T{ 0.46780815, -0.54966937, 0.69211207 },
			v3T{ 0.07109649, 0.79259959, -0.60558333 }, v3T{ -0.52073054, -0.06778631, 0.85102569 }, v3T{ -0.36866700, 0.77676019, -0.51061556 }, v3T{ -0.71702100, -0.35727116, 0.59853004 },
			v3T{ -0.59010862, -0.73536014, -0.33319257 }, v3T{ -0.66875911, 0.58597660, 0.45759445 }, v3T{ -0.59798034, -0.69169805, 0.40493619 }, v3T{ -0.20490060, 0.79048994, 0.57718402 },
			v3T{ 0.48765302, 0.85851673, 0.15856717 }, v3T{ 0.88918101, 0.10371433, 0.44564612 }, v3T{ 0.48664272, 0.83596000, 0.25367252 }, v3T{ -0.24554119, 0.50230038, -0.82909822 },
			v3T{ 0.03554055, -0.41884154, -0.90736356 }, v3T{ -0.03701100, -0.61772404, 0.78552352 }, v3T{ 0.42824046, 0.20582938, -0.87991158 }, v3T{ -0.06839464, -0.43555129, -0.89756183 },
			v3T{ -0.40866952, -0.70331213, -0.58167111 }, v3T{ -0.74822692, 0.38256599, 0.54203297 }, v3T{ 0.71541445, 0.51615594, 0.47091953 }, v3T{ 0.60759905, -0.70288934, -0.36982423 },
			v3T{ -0.01648745, -0.13394229, -0.99085197 }, v3T{ -0.64568452, -0.13342451, 0.75185730 }, v3T{ -0.42008783, 0.33302268, 0.84416948 }, v3T{ -0.63557180, -0.46817632, 0.61388877 },
			v3T{ -0.82478405, -0.45636029, 0.33386606 }, v3T{ -0.66628051, 0.24058840, 0.70582399 }, v3T{ -0.60499178, -0.78374178, -0.14047697 }, v3T{ 0.63041860, -0.60894989, -0.48140672 },
			v3T{ -0.07945150, -0.91288865, -0.40040202 }, v3T{ -0.66942344, 0.18523930, 0.71941550 }, v3T{ -0.00849762, -0.47038898, 0.88241827 }, v3T{ 0.66223413, -0.33585751, 0.66981019 },
			v3T{ 0.82512667, -0.23099667, -0.51556427 }, v3T{ -0.75186864, 0.65450118, -0.07950940 }, v3T{ 0.87383910, 0.08193441, 0.47926192 }, v3T{ -0.26554211, 0.78650504, 0.55758158 },
			v3T{ -0.49574252, 0.70523568, 0.50683527 }, v3T{ -0.49212635, -0.64694353, 0.58247381 }, v3T{ 0.32264136, 0.78159510, -0.53386482 }, v3T{ 0.71510371, -0.22498049, 0.66182359 },
			v3T{ 0.61434883, -0.51790453, 0.59527340 }, v3T{ -0.82551670, -0.14228251, -0.54614821 }, v3T{ -0.46251954, 0.64306734, -0.61036060 }, v3T{ -0.52117891, -0.69061769, 0.50141773 },
			v3T{ 0.27468699, -0.88951139, -0.36512537 }, v3T{ 0.65713642, -0.75365863, -0.01305358 }, v3T{ 0.94136220, -0.21960140, -0.25614924 }, v3T{ -0.85554460, 0.30842011, -0.41583708 },
			v3T{ -0.35233681, -0.15379949, 0.92314922 }, v3T{ -0.74432132, 0.44164975, -0.50093040 }, v3T{ 0.53994954, -0.79953954, -0.26304184 }, v3T{ 0.42964607, 0.11880600, 0.89514769 },
			v3T{ -0.87921789, 0.18018271, 0.44103298 }, v3T{ -0.80353079, 0.36514238, 0.47011628 }, v3T{ 0.50404538, 0.65465655, -0.56334986 }, v3T{ -0.92083981, -0.30381360, -0.24444087 },
			v3T{ 0.13956423, -0.96009192, -0.24237437 }, v3T{ -0.71698508, 0.68682212, 0.11919639 }, v3T{ -0.76698836, 0.61675487, -0.17703754 }, v3T{ -0.21874818, -0.57847904, -0.78581883 },
			v3T{ 0.55494484, -0.79971185, 0.22912262 }, v3T{ 0.79660662, -0.41090893, 0.44336412 }, v3T{ 0.66489466, 0.00947646, -0.74687703 }, v3T{ -0.59920476, 0.36935905, 0.71030103 },
			v3T{ -0.57524868, -0.51402380, -0.63629277 }, v3T{ 0.20536135, -0.69296940, 0.69110066 }, v3T{ -0.05544564, -0.99802158, 0.02964287 }, v3T{ 0.13201661, 0.16519726, -0.97738502 },
			v3T{ 0.46510187, 0.64584669, -0.60544390 }, v3T{ -0.80108393, -0.59762086, 0.03337417 }, v3T{ -0.39806873, -0.44410006, -0.80269323 }, v3T{ 0.95136791, -0.21916666, -0.21648342 },
			v3T{ -0.82086395, 0.17982074, 0.54207645 }, v3T{ 0.79513089, 0.37056075, 0.48005374 }, v3T{ 0.77112323, 0.56616567, 0.29124800 }, v3T{ 0.81176337, -0.24837815, 0.52853432 },
			v3T{ -0.81842091, 0.50060656, 0.28209979 }, v3T{ -0.38248924, -0.72602893, 0.57147525 }, v3T{ 0.46198573, 0.37950267, 0.80159024 }, v3T{ -0.59524911, 0.04222053, 0.80243126 },
			v3T{ -0.52273882, 0.79497643, -0.30782561 }, v3T{ -0.79922245, 0.45390541, 0.39397125 }, v3T{ 0.38051244, -0.76512679, 0.51941436 }, v3T{ 0.83818590, 0.22605420, 0.49633043 },
			v3T{ 0.63218067, 0.48127057, 0.60722832 }, v3T{ 0.59242495, 0.18424992, -0.78427333 }, v3T{ 0.85249021, -0.48552132, 0.19372531 }, v3T{ -0.43548364, -0.58439144, 0.68471939 },
			v3T{ 0.73179011, 0.29594379, -0.61392223 }, v3T{ -0.45280534, -0.80755156, 0.37792566 }, v3T{ 0.55557939, 0.30092870, -0.77509578 }, v3T{ 0.42575514, 0.70893498, 0.56226662 },
			v3T{ 0.60528173, -0.51550786, 0.60653580 }, v3T{ -0.51076670, 0.84729685, -0.14562083 }, v3T{ -0.33474095, 0.69713420, -0.63399716 }, v3T{ -0.48650711, 0.74561924, 0.45537104 },
			v3T{ -0.41670009, -0.87381546, -0.25061440 }, v3T{ 0.92586094, -0.34254116, -0.15952140 }, v3T{ -0.10682502, 0.59910669, 0.79351092 }, v3T{ -0.44718479, -0.59299328, 0.66961536 },
			v3T{ 0.69862855, -0.48858264, 0.52269031 }, v3T{ -0.74718902, 0.51933770, -0.41472512 }, v3T{ -0.56931667, 0.42835158, 0.70170753 }, v3T{ 0.05154068, 0.16647211, 0.98469823 },
			v3T{ 0.74568360, -0.66371406, 0.05864824 }, v3T{ 0.64686641, 0.41668704, 0.63869849 }, v3T{ 0.27796256, -0.73021674, 0.62411563 }, v3T{ 0.77079499, -0.62615383, 0.11750087 },
			v3T{ -0.06833979, 0.90160690, 0.42712371 }, v3T{ -0.98003087, -0.09480635, 0.17478914 }, v3T{ -0.85191651, 0.47279136, 0.22518122 }, v3T{ 0.52473004, -0.19693989, -0.82817454 },
			v3T{ 0.16081399, 0.75081437, -0.64063768 }, v3T{ 0.71441816, 0.52488995, -0.46270642 }, v3T{ -0.23333515, -0.88652173, 0.39954216 }, v3T{ 0.54760612, -0.74897952, -0.37303782 },
			v3T{ 0.48186221, -0.57810371, 0.65848683 }, v3T{ -0.21255857, -0.53489421, -0.81774509 }, v3T{ 0.77930308, 0.57549405, -0.24797842 }, v3T{ 0.60279872, -0.76604104, -0.22319235 },
			v3T{ 0.37230136, -0.52720909, 0.76383393 }, v3T{ -0.13321231, -0.92277683, 0.36157627 }, v3T{ -0.47833070, -0.49076061, -0.72825392 }, v3T{ 0.28828612, -0.93601402, 0.20191301 },
			v3T{ -0.66460360, -0.65589055, 0.35792406 }, v3T{ 0.90686144, 0.30403802, 0.29182738 }, v3T{ -0.00682204, 0.42199214, 0.90657382 }, v3T{ -0.33221520, 0.26584830, -0.90496284 },
			v3T{ -0.59515132, 0.55081686, 0.58514588 }, v3T{ 0.77123373, 0.59869357, -0.21625109 }, v3T{ -0.69765329, -0.61042387, 0.37505011 }, v3T{ 0.02426772, -0.55656860, -0.83044715 },
			v3T{ 0.65180023, 0.75814507, 0.01930051 }, v3T{ -0.01531784, -0.78276243, 0.62213209 }, v3T{ 0.63847163, 0.03936370, 0.76863807 }, v3T{ 0.40703600, -0.09783879, -0.90815707 },
			v3T{ -0.46223121, -0.64783550, -0.60551753 }, v3T{ 0.82788442, -0.46539053, 0.31307993 }, v3T{ -0.75467147, 0.24001984, 0.61062382 }, v3T{ -0.70062375, -0.69087941, 0.17835919 },
			v3T{ 0.35457466, 0.88605939, -0.29862279 }, v3T{ 0.20159504, -0.88658663, -0.41632150 }, v3T{ -0.32096612, 0.72494426, -0.60945597 }, v3T{ 0.14147986, 0.53949815, -0.83001518 },
			v3T{ 0.28297638, 0.93772862, 0.20146813 }, v3T{ 0.67192636, 0.43759891, -0.59751332 }, v3T{ 0.98497844, 0.01967209, 0.17155312 }, v3T{ 0.60388215, -0.68969665, 0.39955586 },
			v3T{ 0.41200242, 0.85002960, 0.32818240 }, v3T{ -0.83375884, 0.39266173, -0.38815328 }, v3T{ -0.70938505, -0.58502714, -0.39308535 }, v3T{ -0.63048972, 0.77513872, 0.04053013 },
			v3T{ 0.10261233, -0.69355480, -0.71305852 }, v3T{ 0.65702752, -0.38976767, -0.64528753 }, v3T{ -0.41388260, 0.33890875, 0.84489174 }, v3T{ 0.03028400, -0.46424256, -0.88519022 },
			v3T{ 0.45068344, -0.52775066, -0.71997478 }, v3T{ 0.48930093, 0.41323002, -0.76800101 }, v3T{ 0.28350070, 0.66390322, 0.69199701 }, v3T{ 0.42450922, -0.60916900, 0.66985450 },
			v3T{ 0.67306932, 0.51724488, -0.52861652 }, v3T{ 0.31095891, 0.94487804, -0.10251852 }, v3T{ -0.25569777, 0.90632689, -0.33643754 }, v3T{ -0.21431592, 0.07778980, -0.97366187 },
			v3T{ 0.27676605, -0.87464593, 0.39798876 }, v3T{ 0.00288072, -0.88726140, -0.46125796 }, v3T{ 0.51138622, 0.12353356, 0.85042554 }, v3T{ 0.59734197, 0.76052363, 0.25453168 },
			v3T{ -0.43336730, -0.76588813, 0.47498227 }, v3T{ 0.34180565, -0.68750195, -0.64071052 }, v3T{ -0.65078280, 0.51803512, 0.55508681 }, v3T{ -0.89824124, 0.40466264, -0.17149586 },
			v3T{ 0.54253116, 0.81082175, -0.21960883 }, v3T{ -0.53994336, 0.54836630, 0.63855741 }, v3T{ 0.68778819, 0.33483595, -0.64407475 }, v3T{ -0.63530446, -0.39864092, 0.66141792 },
			v3T{ 0.80728009, -0.58358794, -0.08788616 }, v3T{ 0.94835277, 0.26419320, 0.17558181 }, v3T{ -0.15823843, -0.51165316, 0.84449490 }, v3T{ 0.17510951, -0.22389002, 0.95875436 },
			v3T{ 0.13697442, -0.88598087, 0.44303037 }, v3T{ -0.73457485, -0.23332652, -0.63714874 }, v3T{ 0.95521505, -0.11801760, 0.27135964 }, v3T{ -0.40184319, -0.90170455, -0.15953355 },
			v3T{ 0.16857866, -0.70975159, -0.68398386 }, v3T{ -0.55230772, 0.37144476, 0.74631426 }, v3T{ 0.29875717, -0.61848962, -0.72678383 }, v3T{ 0.62465217, -0.76131685, 0.17379963 },
			v3T{ 0.75759704, 0.19352541, 0.62337360 }, v3T{ -0.10375594, 0.61563856, 0.78116827 }, v3T{ 0.52725731, 0.25296549, 0.81117704 }, v3T{ -0.71292545, -0.53989924, -0.44748867 },
			v3T{ 0.78246146, 0.54867457, 0.29446609 }, v3T{ 0.31458005, 0.63401883, -0.70644145 }, v3T{ -0.09360697, -0.99481997, -0.03963538 }, v3T{ -0.59000956, 0.10880136, -0.80003186 },
			v3T{ 0.49713243, 0.77379744, -0.39255173 }, v3T{ -0.92985377, 0.17383167, 0.32427537 }, v3T{ 0.73574353, -0.63730495, -0.22918086 }, v3T{ -0.04383386, -0.80273910, -0.59471719 },
			v3T{ 0.68411849, 0.52929683, -0.50182344 }, v3T{ -0.19561815, -0.57428906, -0.79493749 }, v3T{ 0.90257811, -0.06366895, -0.42579222 }, v3T{ 0.62294256, 0.39027502, -0.67795868 },
			v3T{ -0.39046281, -0.70398950, 0.59324327 }, v3T{ 0.70990020, 0.62433400, -0.32595821 }, v3T{ -0.99157404, 0.01300690, 0.12888658 }, v3T{ -0.55765988, -0.46179257, 0.68975581 },
			v3T{ -0.53736280, -0.34635255, -0.76894807 }, v3T{ 0.25083685, 0.44726649, -0.85850659 }, v3T{ 0.45758528, 0.86982087, -0.18446507 }, v3T{ -0.18615519, 0.23441065, -0.95414773 },
			v3T{ 0.56359579, -0.41325118, -0.71525048 }, v3T{ -0.48542469, 0.59678985, -0.63890903 }, v3T{ -0.72243931, -0.40815930, 0.55811059 }, v3T{ -0.23748605, 0.68466361, -0.68908354 },
			v3T{ -0.69257361, 0.27959985, -0.66495543 }, v3T{ -0.10352601, -0.17369566, -0.97934273 }, v3T{ 0.00192480, -0.09194122, 0.99576258 }, v3T{ 0.36297645, 0.86362173, 0.34986513 },
			v3T{ -0.71118388, -0.10242990, 0.69550385 }, v3T{ 0.45146824, 0.43080300, 0.78139952 }, v3T{ -0.13265094, -0.68773403, -0.71374059 }, v3T{ 0.56016516, -0.56270148, -0.60793259 },
			v3T{ -0.95871022, -0.27465634, -0.07374694 }, v3T{ -0.84169709, 0.06533746, -0.53598230 }, v3T{ 0.69711911, -0.61618111, -0.36653212 }, v3T{ -0.01620384, 0.59778204, -0.80149490 },
			v3T{ -0.34911215, 0.65899531, -0.66621760 }, v3T{ -0.19279427, -0.50540811, -0.84106659 }, v3T{ -0.60506152, 0.72292944, 0.33357695 }, v3T{ 0.79789244, -0.59553505, 0.09330415 },
			v3T{ -0.48173680, -0.74189415, 0.46639331 }, v3T{ 0.84140763, 0.31839867, 0.43664115 }, v3T{ 0.79614481, 0.60391839, -0.03789486 }, v3T{ 0.19384456, 0.57096572, 0.79776089 },
			v3T{ 0.83441754, -0.25078854, -0.49076723 }, v3T{ -0.62605441, 0.72550166, 0.28583776 }, v3T{ 0.55337866, -0.75558589, 0.35051679 }, v3T{ 0.80543476, -0.01571309, 0.59247611 },
			v3T{ -0.00851542, 0.98991715, 0.14139139 }, v3T{ -0.94076275, -0.29730096, -0.16302633 }, v3T{ -0.75465549, -0.41353736, -0.50939371 }, v3T{ 0.37739255, -0.63080384, 0.67798332 },
			v3T{ 0.47325376, -0.73145333, -0.49092453 }, v3T{ 0.12930721, -0.49066326, -0.86170135 }, v3T{ 0.71173142, -0.11663112, 0.69270165 }, v3T{ 0.41952295, -0.63051086, -0.65303641 },
			v3T{ 0.85916103, 0.42641569, 0.28286390 }, v3T{ 0.54792224, -0.66418740, 0.50856299 }, v3T{ 0.28479416, 0.43856869, 0.85237890 }, v3T{ -0.59050384, -0.68486024, -0.42693285 },
			v3T{ 0.54884141, 0.60847988, 0.57317130 }, v3T{ 0.87567478, 0.25649070, -0.40915304 }, v3T{ 0.02961573, 0.33496172, 0.94176619 }, v3T{ 0.67428181, 0.70665199, 0.21444580 },
			v3T{ 0.23609059, -0.51982231, 0.82100305 }, v3T{ 0.93726653, 0.00671493, 0.34854893 }, v3T{ -0.39891590, -0.91536143, -0.05458531 }, v3T{ 0.93359117, -0.35793085, 0.01711843 },
			v3T{ 0.53572079, -0.56879583, 0.62407896 }, v3T{ -0.61516933, -0.36856434, -0.69694119 }, v3T{ 0.74630703, -0.65946218, -0.09019675 }, v3T{ 0.50607373, -0.59204544, -0.62719342 },
			v3T{ -0.89793356, 0.43675114, 0.05444050 }, v3T{ -0.91682171, 0.07126199, 0.39288634 }, v3T{ -0.61178292, -0.15203616, -0.77627744 }, v3T{ -0.14028895, 0.63023583, 0.76362413 },
			v3T{ 0.71475895, -0.54060748, 0.44369268 }, v3T{ -0.31764961, 0.92630790, -0.20261391 }, v3T{ 0.59833443, -0.58864018, -0.54359788 }, v3T{ -0.81450219, 0.22699691, -0.53390879 },
			v3T{ 0.00452737, -0.06652318, 0.99777461 }, v3T{ 0.59311614, 0.19797584, -0.78039657 }, v3T{ -0.71375488, -0.02586188, 0.69991795 }, v3T{ -0.75600145, -0.26384588, -0.59903853 },
			v3T{ 0.25716644, 0.77480857, -0.57752671 }, v3T{ 0.71712423, 0.61984999, -0.31862018 }, v3T{ -0.28194922, -0.55108799, 0.78537040 }, v3T{ 0.57068285, -0.67066160, 0.47385030 },
			v3T{ 0.48969101, -0.22604767, -0.84208382 }, v3T{ -0.93763991, -0.34062289, 0.06933579 }, v3T{ -0.67376035, 0.15110895, -0.72333469 }, v3T{ -0.72414406, -0.65877431, -0.20403872 },
			v3T{ -0.71204285, 0.41163046, -0.56881926 }, v3T{ 0.23641604, -0.86280490, 0.44685026 }, v3T{ 0.84208951, 0.19949878, -0.50108432 }, v3T{ -0.67481860, 0.67904385, -0.28899707 },
			v3T{ 0.52167146, 0.66360202, 0.53618211 }, v3T{ -0.49330390, -0.48590434, 0.72149029 }, v3T{ -0.18240720, 0.04137646, -0.98235208 }, v3T{ 0.30714395, 0.55170433, 0.77542564 },
			v3T{ -0.14577549, 0.95376355, -0.26283949 }, v3T{ -0.54373260, -0.69781662, -0.46626905 }, v3T{ 0.01799205, -0.81833182, 0.57446437 }, v3T{ 0.51019037, -0.56615200, -0.64743934 },
			v3T{ 0.48463473, 0.59436639, 0.64176146 }, v3T{ 0.09115853, -0.52830175, -0.84414891 }, v3T{ -0.62962436, -0.38408030, -0.67531880 }, v3T{ 0.50864721, -0.48401592, -0.71204396 },
			v3T{ -0.69669235, -0.63427804, -0.33512853 }, v3T{ 0.60735178, -0.18339351, 0.77297518 }, v3T{ 0.74102699, 0.67064566, 0.03336744 }, v3T{ -0.47352242, -0.76145583, -0.44267543 },
			v3T{ 0.47751531, -0.79737827, -0.36900816 }, v3T{ 0.74175025, -0.64892413, 0.16942269 }, v3T{ 0.65484829, -0.70924167, -0.26105549 }, v3T{ 0.60455058, -0.64392987, -0.46890608 },
			v3T{ -0.61878613, -0.77223405, 0.14407742 }, v3T{ -0.72376655, -0.65562529, 0.21521492 }, v3T{ 0.24420910, -0.52118606, -0.81775731 }, v3T{ 0.61291622, 0.39870471, -0.68217906 },
			v3T{ 0.67751893, 0.65970488, 0.32520389 }, v3T{ -0.04366879, -0.96113671, 0.27259726 }, v3T{ 0.36541094, 0.62808212, 0.68701361 }, v3T{ -0.92572867, 0.10611717, -0.36299528 },
			v3T{ 0.80766374, -0.02031352, -0.58929335 }, v3T{ -0.82117076, 0.53034081, 0.21075390 }, v3T{ -0.62778197, -0.51872129, 0.58036025 }, v3T{ 0.37696186, 0.57743439, -0.72420251 },
			v3T{ -0.56818895, -0.47089866, -0.67484500 }, v3T{ -0.61126182, -0.69853192, 0.37203783 }, v3T{ 0.57901952, 0.81284241, -0.06343191 }, v3T{ -0.53287943, 0.70445351, 0.46881208 },
			v3T{ 0.22300157, -0.93258969, 0.28380764 }, v3T{ -0.63832115, -0.40157013, -0.65672486 }, v3T{ -0.22074780, 0.50999380, 0.83137040 }, v3T{ -0.59081050, -0.13684815, -0.79511982 },
			v3T{ -0.79824305, 0.52060475, -0.30295004 }, v3T{ -0.56871170, 0.76435226, 0.30386284 }, v3T{ 0.12786983, -0.64236825, -0.75565358 }, v3T{ -0.17631562, -0.76167939, -0.62350405 },
			v3T{ 0.34713709, 0.61125835, -0.71123770 }, v3T{ -0.39238887, -0.52886732, 0.75254922 }, v3T{ 0.38116332, 0.71358998, -0.58779577 }, v3T{ -0.72949527, -0.67040404, 0.13562844 },
			v3T{ -0.62057913, 0.45165344, -0.64100757 }, v3T{ -0.10668918, -0.98309252, -0.14881706 }, v3T{ 0.59490400, -0.46196716, -0.65778079 }, v3T{ 0.22433782, 0.49054463, 0.84204424 },
			v3T{ 0.77498791, -0.57220981, 0.26827165 }, v3T{ 0.26474565, 0.93986866, -0.21576987 }, v3T{ -0.01328623, 0.99975439, 0.01773780 }, v3T{ 0.53097408, 0.47771884, 0.69989373 },
			v3T{ 0.24635212, -0.37499947, -0.89369236 }, v3T{ 0.31300988, -0.54171955, 0.78010560 }, v3T{ 0.77494650, -0.52634980, 0.34987684 }, v3T{ 0.65518408, 0.51410661, -0.55355958 },
			v3T{ 0.78000762, -0.61855443, -0.09475515 }, v3T{ 0.58176976, 0.62638121, 0.51883574 }, v3T{ -0.62371886, -0.59433046, 0.50768699 }, v3T{ 0.85206333, 0.17478222, -0.49339564 },
			v3T{ 0.69974170, -0.42963013, 0.57077098 }, v3T{ -0.44953934, 0.62956163, -0.63369277 }, v3T{ 0.63562255, 0.51965998, -0.57090935 }, v3T{ -0.02766532, -0.52812789, -0.84871406 },
			v3T{ 0.78698609, 0.04742916, -0.61514500 }, v3T{ 0.37827449, 0.78614098, 0.48876454 }, v3T{ 0.90534508, -0.25600916, -0.33883565 }, v3T{ -0.37701605, 0.47347359, -0.79604124 },
			v3T{ -0.43802429, 0.40756165, -0.80126664 }, v3T{ -0.87945568, -0.47372426, -0.04629300 }, v3T{ -0.22787901, -0.82242670, 0.52123457 }, v3T{ 0.48721529, 0.74652617, -0.45312243 },
			v3T{ -0.68473990, -0.68222429, 0.25632263 }, v3T{ -0.33289944, 0.62102263, -0.70958358 }, v3T{ -0.07838790, -0.85438083, -0.51370101 }, v3T{ 0.18575601, 0.96209034, 0.19969195 },
			v3T{ 0.09048656, -0.68256793, -0.72519874 }, v3T{ 0.29506068, -0.68306389, -0.66810397 }, v3T{ -0.94937153, -0.17748927, 0.25921277 }, v3T{ -0.38725072, 0.16372291, 0.90732116 },
			v3T{ -0.02691563, 0.81898594, 0.57318198 }, v3T{ -0.65244629, -0.52276924, -0.54865851 }, v3T{ 0.15270967, -0.00097578, 0.98827061 }, v3T{ 0.39108739, 0.55471383, -0.73439990 },
			v3T{ 0.85379797, -0.05140234, 0.51806064 }, v3T{ 0.31443713, 0.14998906, -0.93735403 }, v3T{ -0.44277186, -0.56474741, -0.69642907 }, v3T{ -0.31521736, 0.37268196, 0.87278071 },
			v3T{ 0.97997903, -0.16829529, 0.10638514 }, v3T{ -0.25174419, -0.84939324, 0.46384910 }, v3T{ 0.03867740, -0.72044135, 0.69243651 }, v3T{ -0.80207202, 0.48047131, 0.35472214 },
			v3T{ 0.48200634, -0.48413492, 0.73026246 }, v3T{ -0.41800015, 0.44068588, -0.79440029 }, v3T{ 0.58661859, -0.43233611, 0.68480955 }, v3T{ 0.40830998, -0.53710845, 0.73810397 },
			v3T{ 0.61242611, -0.72220206, -0.32149407 }, v3T{ -0.34159283, -0.62199145, -0.70458567 }, v3T{ -0.29885191, 0.58492128, -0.75402562 }, v3T{ -0.62924060, 0.77130626, -0.09561862 },
			v3T{ 0.91118189, 0.27762192, 0.30442344 }, v3T{ 0.08064464, -0.99213777, -0.09570315 }, v3T{ 0.93083382, -0.34928416, -0.10746612 }, v3T{ 0.66101659, -0.67569323, 0.32633681 },
			v3T{ 0.07148482, -0.97619739, -0.20476469 }, v3T{ 0.30440743, -0.78193565, -0.54397863 }, v3T{ -0.35656518, -0.19962907, 0.91269355 }, v3T{ 0.82151650, -0.31061678, 0.47815045 },
			v3T{ -0.69709423, -0.71173375, -0.08657198 }, v3T{ -0.46044170, -0.78565215, -0.41321197 }, v3T{ -0.70275364, -0.21121895, 0.67935548 }, v3T{ 0.38087769, 0.63933041, 0.66797366 }
		};
		return g;
	}

	/// <summary>
	/// Initializes the offsets used in the crackle variation.
	/// </summary>
	/// <returns>A copy of the locally declared vector</returns>
	std::vector<v2T> InitOffsets()
	{
		std::vector<v2T> g =
		{
			{ -1, -1 }, { -1, 0 }, { -1, 1 },
			{ 0, -1 }, { 0, 0 }, { 0, 1 },
			{ 1, -1 }, { 1, 0 }, { 1, 1 }
		};
		return g;
	}

	/// <summary>
	/// Initializes the P1 vector used in J1().
	/// Note J1() comes with std in C++, but needed to be manually implemented in OpenCL.
	/// </summary>
	/// <returns>A copy of the locally declared vector</returns>
	std::vector<T> InitP1()
	{
		std::vector<T> v =
		{
			T(-1.4258509801366645672e+11),
			T(6.6781041261492395835e+09 ),
			T(-1.1548696764841276794e+08),
			T(9.8062904098958257677e+05 ),
			T(-4.4615792982775076130e+03),
			T(1.0650724020080236441e+01 ),
			T(-1.0767857011487300348e-02)
		};
		return v;
	}

	/// <summary>
	/// Initializes the Q1 vector used in J1().
	/// Note J1() comes with std in C++, but needed to be manually implemented in OpenCL.
	/// </summary>
	/// <returns>A copy of the locally declared vector</returns>
	std::vector<T> InitQ1()
	{
		std::vector<T> v =
		{
			T(4.1868604460820175290e+12),
			T(4.2091902282580133541e+10),
			T(2.0228375140097033958e+08),
			T(5.9117614494174794095e+05),
			T(1.0742272239517380498e+03),
			T(1.0),
			T(0.0)
		};
		return v;
	}

	/// <summary>
	/// Initializes the P2 vector used in J1().
	/// Note J1() comes with std in C++, but needed to be manually implemented in OpenCL.
	/// </summary>
	/// <returns>A copy of the locally declared vector</returns>
	std::vector<T> InitP2()
	{
		std::vector<T> v =
		{
			T(-1.7527881995806511112e+16),
			T(1.6608531731299018674e+15 ),
			T(-3.6658018905416665164e+13),
			T(3.5580665670910619166e+11 ),
			T(-1.8113931269860667829e+09),
			T(5.0793266148011179143e+06 ),
			T(-7.5023342220781607561e+03),
			T(4.6179191852758252278e+00)
		};
		return v;
	}

	/// <summary>
	/// Initializes the Q2 vector used in J1().
	/// Note J1() comes with std in C++, but needed to be manually implemented in OpenCL.
	/// </summary>
	/// <returns>A copy of the locally declared vector</returns>
	std::vector<T> InitQ2()
	{
		std::vector<T> v =
		{
			T(1.7253905888447681194e+18),
			T(1.7128800897135812012e+16),
			T(8.4899346165481429307e+13),
			T(2.7622777286244082666e+11),
			T(6.4872502899596389593e+08),
			T(1.1267125065029138050e+06),
			T(1.3886978985861357615e+03),
			T(1.0)
		};
		return v;
	}

	/// <summary>
	/// Initializes the PC vector used in J1().
	/// Note J1() comes with std in C++, but needed to be manually implemented in OpenCL.
	/// </summary>
	/// <returns>A copy of the locally declared vector</returns>
	std::vector<T> InitPC()
	{
		std::vector<T> v =
		{
			T(-4.4357578167941278571e+06),
			T(-9.9422465050776411957e+06),
			T(-6.6033732483649391093e+06),
			T(-1.5235293511811373833e+06),
			T(-1.0982405543459346727e+05),
			T(-1.6116166443246101165e+03),
			T(0.0)
		};
		return v;
	}

	/// <summary>
	/// Initializes the QC vector used in J1().
	/// Note J1() comes with std in C++, but needed to be manually implemented in OpenCL.
	/// </summary>
	/// <returns>A copy of the locally declared vector</returns>
	std::vector<T> InitQC()
	{
		std::vector<T> v =
		{
			T(-4.4357578167941278568e+06),
			T(-9.9341243899345856590e+06),
			T(-6.5853394797230870728e+06),
			T(-1.5118095066341608816e+06),
			T(-1.0726385991103820119e+05),
			T(-1.4550094401904961825e+03),
			T(1.0)
		};
		return v;
	}

	/// <summary>
	/// Initializes the PS vector used in J1().
	/// Note J1() comes with std in C++, but needed to be manually implemented in OpenCL.
	/// </summary>
	/// <returns>A copy of the locally declared vector</returns>
	std::vector<T> InitPS()
	{
		std::vector<T> v =
		{
			T(3.3220913409857223519e+04),
			T(8.5145160675335701966e+04),
			T(6.6178836581270835179e+04),
			T(1.8494262873223866797e+04),
			T(1.7063754290207680021e+03),
			T(3.5265133846636032186e+01),
			T(0.0)
		};
		return v;
	}

	/// <summary>
	/// Initializes the QS vector used in J1().
	/// Note J1() comes with std in C++, but needed to be manually implemented in OpenCL.
	/// </summary>
	/// <returns>A copy of the locally declared vector</returns>
	std::vector<T> InitQS()
	{
		std::vector<T> v =
		{
			T(7.0871281941028743574e+05),
			T(1.8194580422439972989e+06),
			T(1.4194606696037208929e+06),
			T(4.0029443582266975117e+05),
			T(3.7890229745772202641e+04),
			T(8.6383677696049909675e+02),
			T(1.0)

		};
		return v;
	}

	std::vector<int> m_P;
	std::vector<T> m_PFloats;
	std::vector<T> m_P1;
	std::vector<T> m_Q1;
	std::vector<T> m_P2;
	std::vector<T> m_Q2;
	std::vector<T> m_PC;
	std::vector<T> m_QC;
	std::vector<T> m_PS;
	std::vector<T> m_QS;
	std::vector<v2T> m_Offsets;
	std::vector<v3T> m_Grad;
	std::unordered_map<string, pair<const T*, size_t>> m_GlobalMap;
};
}
