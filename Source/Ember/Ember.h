#pragma once

#include "Curves.h"
#include "Xform.h"
#include "Palette.h"
#include "SpatialFilter.h"
#include "TemporalFilter.h"
#include "EmberMotion.h"
#include "CarToRas.h"
#include "VarFuncs.h"

/// <summary>
/// Ember class.
/// </summary>
namespace EmberNs
{
template <typename T> class Interpolater;

/// <summary>
/// Bit position specifying the presence of each type of 3D parameter.
/// One, none, some or all of these can be present.
/// </summary>
enum class eProjBits : et
{
	PROJBITS_ZPOS = 1,
	PROJBITS_PERSP = 2,
	PROJBITS_PITCH = 4,
	PROJBITS_YAW = 8,
	PROJBITS_BLUR = 16,
};

/// <summary>
/// Ember is the main class for holding all of the information required to render a fractal flame.
/// This includes a vector of xforms, a final xform, size and color information as well as an Xml edit
/// document that users can use to keep track of changes.
/// Operations will often desire operating on just the regular xforms, or the regular xforms plus the final one.
/// The word "total" is used to signify when the final xform is included in the operation.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API Ember
{
public:
	/// <summary>
	/// Default constructor which calls Init() to set default values.
	/// </summary>
	Ember()
	{
		m_Background.Reset();
		m_Curves.Init();
		m_Xforms.reserve(12);
		m_CamMat = m3T(0);
		m_ProjFunc = &EmberNs::Ember<T>::ProjectNone;
	}

	/// <summary>
	/// Copy constructor to copy an Ember object of the same type.
	/// </summary>
	/// <param name="ember">The Ember object to copy</param>
	Ember(const Ember<T>& ember)
	{
		Ember<T>::operator=<T>(ember);
	}

	/// <summary>
	/// Copy constructor to copy an Ember object of type U, where T is usually the same type as U.
	/// </summary>
	/// <param name="ember">The Ember object to copy</param>
	template <typename U>
	Ember(const Ember<U>& ember)
	{
		Ember<T>::operator=<U>(ember);
	}

	/// <summary>
	/// Destructor which clears the Xml edits.
	/// </summary>
	~Ember()
	{
		ClearEdit();
	}

	/// <summary>
	/// Assignment operator to copy an Ember object of the same type.
	/// For some reason the compiler requires Xform to define two assignment operators,
	/// however it gets confused when Ember has two.
	/// </summary>
	/// <param name="ember">The Ember object to copy</param>
	Ember<T>& operator = (const Ember<T>& ember)
	{
		if (this != &ember)
			Ember<T>::operator=<T>(ember);

		return *this;
	}

	/// <summary>
	/// Assignment operator to assign an Ember object of type U, where T is usually the same type as U.
	/// </summary>
	/// <param name="ember">The Ember object to copy.</param>
	/// <returns>Reference to updated self</returns>
	template <typename U>
	Ember<T>& operator = (const Ember<U>& ember)
	{
		m_FinalRasW			  = ember.m_FinalRasW;
		m_FinalRasH			  = ember.m_FinalRasH;
		m_OrigFinalRasW		  = ember.m_OrigFinalRasW;
		m_OrigFinalRasH		  = ember.m_OrigFinalRasH;
		m_OrigPixPerUnit	  = static_cast<T>(ember.m_OrigPixPerUnit);
		m_RandPointRange      = static_cast<T>(ember.m_RandPointRange);
		m_SubBatchSize		  = ember.m_SubBatchSize;
		m_FuseCount			  = ember.m_FuseCount;
		m_Supersample		  = ember.m_Supersample;
		m_TemporalSamples	  = ember.m_TemporalSamples;
		m_Symmetry			  = ember.m_Symmetry;
		m_Quality			  = static_cast<T>(ember.m_Quality);
		m_PixelsPerUnit		  = static_cast<T>(ember.m_PixelsPerUnit);
		m_Zoom				  = static_cast<T>(ember.m_Zoom);
		m_CamZPos			  = static_cast<T>(ember.m_CamZPos);
		m_CamPerspective	  = static_cast<T>(ember.m_CamPerspective);
		m_CamYaw			  = static_cast<T>(ember.m_CamYaw);
		m_CamPitch			  = static_cast<T>(ember.m_CamPitch);
		m_CamDepthBlur		  = static_cast<T>(ember.m_CamDepthBlur);
		m_BlurCurve           = static_cast<T>(ember.m_BlurCurve);
		m_CamMat			  = ember.m_CamMat;
		m_CenterX			  = static_cast<T>(ember.m_CenterX);
		m_CenterY			  = static_cast<T>(ember.m_CenterY);
		m_RotCenterY		  = static_cast<T>(ember.m_RotCenterY);
		m_Rotate			  = static_cast<T>(ember.m_Rotate);
		m_Brightness		  = static_cast<T>(ember.m_Brightness);
		m_Gamma				  = static_cast<T>(ember.m_Gamma);
		m_Vibrancy			  = static_cast<T>(ember.m_Vibrancy);
		m_GammaThresh		  = static_cast<T>(ember.m_GammaThresh);
		m_HighlightPower	  = static_cast<T>(ember.m_HighlightPower);
		m_K2                  = static_cast<T>(ember.m_K2);
		m_Time				  = static_cast<T>(ember.m_Time);
		m_Background		  = ember.m_Background;
		m_Interp			  = ember.m_Interp;
		m_AffineInterp		  = ember.m_AffineInterp;
		m_MinRadDE			  = static_cast<T>(ember.m_MinRadDE);
		m_MaxRadDE			  = static_cast<T>(ember.m_MaxRadDE);
		m_CurveDE			  = static_cast<T>(ember.m_CurveDE);
		m_SpatialFilterType	  = ember.m_SpatialFilterType;
		m_SpatialFilterRadius = static_cast<T>(ember.m_SpatialFilterRadius);
		m_Stagger             = ember.m_Stagger;
		m_Rotations           = ember.m_Rotations;
		m_SecondsPerRotation  = ember.m_SecondsPerRotation;
		m_RotateXformsCw      = ember.m_RotateXformsCw;
		m_BlendSeconds        = ember.m_BlendSeconds;
		m_RotationsPerBlend   = ember.m_RotationsPerBlend;
		m_BlendRotateXformsCw = ember.m_BlendRotateXformsCw;
		m_Linear              = ember.m_Linear;
		m_TemporalFilterType  = ember.m_TemporalFilterType;
		m_TemporalFilterExp	  = static_cast<T>(ember.m_TemporalFilterExp);
		m_TemporalFilterWidth = static_cast<T>(ember.m_TemporalFilterWidth);
		m_PaletteMode		  = ember.m_PaletteMode;
		m_PaletteInterp		  = ember.m_PaletteInterp;
		m_Name				  = ember.m_Name;
		m_ParentFilename	  = ember.m_ParentFilename;
		m_Index		  = ember.m_Index;
		//m_Animations  = ember.m_Animations;
		m_ScaleType	  = ember.ScaleType();
		m_Palette	  = ember.m_Palette;
		m_Curves	  = ember.m_Curves;
		m_Xforms.clear();

		for (size_t i = 0; i < ember.XformCount(); i++)
		{
			if (Xform<U>* p = ember.GetXform(i))
			{
				Xform<T> xform = *p;//Will call assignment operator to convert between types T and U.
				AddXform(xform);
			}
		}

		Xform<T> finalXform = *ember.FinalXform();//Will call assignment operator to convert between types T and U.
		SetFinalXform(finalXform);

		//Interpolated-against final xforms need animate & color speed set to 0.
		if (!ember.UseFinalXform())
		{
			m_FinalXform.m_Motion.clear();
			m_FinalXform.m_Animate = 0;
			m_FinalXform.m_AnimateOrigin = 0;
			m_FinalXform.m_ColorSpeed = 0;
		}

		SetProjFunc();
		ClearEdit();

		if (ember.m_Edits)
			m_Edits = xmlCopyDoc(ember.m_Edits, 1);

		CopyCont(m_EmberMotionElements, ember.m_EmberMotionElements);
		m_Solo = ember.m_Solo;
		m_CachedFinal = ember.m_CachedFinal;
		return *this;
	}

	/// <summary>
	/// Reserve the underlying xforms vector to contain the specified capacity.
	/// This should be called at the start of scenarios where xforms are added and their pointers
	/// are used in the process. That way no resizing takes place and the pointers remain valid.
	/// No action is taken if i is less than the existing capacity.
	/// </summary>
	/// <param name="i">The capacity to reserve</param>
	void Reserve(size_t i)
	{
		if (i > m_Xforms.capacity())
			m_Xforms.reserve(i);
	}

	/// <summary>
	/// Add a copy of a new xform to the xforms vector.
	/// </summary>
	/// <param name="xform">The xform to copy and add</param>
	void AddXform(const Xform<T>& xform)
	{
		m_Xforms.push_back(xform);
		m_Xforms[m_Xforms.size() - 1].CacheColorVals();
		m_Xforms[m_Xforms.size() - 1].ParentEmber(this);
	}

	/// <summary>
	/// Add the specified number of empty xforms.
	/// </summary>
	/// <param name="count">The number of xforms to add</param>
	void AddXforms(size_t count)
	{
		auto oldsize = m_Xforms.size();

		for (size_t i = 0; i < count; i++)
		{
			Xform<T> xform;
			xform.m_ColorX = static_cast<T>((oldsize + i) & 1);
			xform.AddVariation(m_VariationList->GetVariationCopy(eVariationId::VAR_LINEAR));
			AddXform(xform);
		}
	}

	/// <summary>
	/// Add empty padding xforms until the total xform count is xformPad.
	/// </summary>
	/// <param name="xformPad">The total number of xforms to finish with</param>
	void PadXforms(size_t xformPad)
	{
		if (xformPad > XformCount())
			AddXforms(xformPad - XformCount());
	}

	/// <summary>
	/// Replace the xforms vector with the one passed in.
	/// </summary>
	/// <param name="xforms">The xforms to replace with</param>
	/// <param name="xformPad">True to move, false to copy. Default: true.</param>
	/// <returns>True if replaced, else false.</returns>
	bool ReplaceXforms(vector<Xform<T>>& xforms, bool move = true)
	{
		if (!xforms.empty())
		{
			if (move)
				m_Xforms = std::move(xforms);
			else
				m_Xforms = xforms;

			return true;
		}
		else
			return false;
	}

	/// <summary>
	/// Copy this ember with optional padding xforms added.
	/// </summary>
	/// <param name="xformPad">The total number of xforms if additional padding xforms are desired. Default: 0.</param>
	/// <param name="doFinal">Whether to copy the final xform. Default: false.</param>
	/// <returns>The newly constructed ember</returns>
	Ember<T> Copy(size_t xformPad = 0, bool doFinal = false) const
	{
		Ember<T> ember(*this);
		ember.PadXforms(xformPad);

		if (doFinal)
		{
			if (UseFinalXform())//Caller wanted one and this ember has one.
			{
				ember.m_FinalXform = m_FinalXform;
			}
			else//Caller wanted one and this ember doesn't have one.
			{
				//Interpolated-against final xforms need animate & color speed set to 0 and motion elements cleared.
				ember.m_FinalXform.m_Affine.MakeID();
				ember.m_FinalXform.m_Post.MakeID();
				ember.m_FinalXform.m_Animate = 0;
				ember.m_FinalXform.m_AnimateOrigin = 0;
				ember.m_FinalXform.m_ColorSpeed = 0;
				ember.m_FinalXform.m_Motion.clear();
				ember.m_FinalXform.ClearAndDeleteVariations();
				ember.m_FinalXform.AddVariation(m_VariationList->GetVariationCopy(eVariationId::VAR_LINEAR));//Do this so it doesn't appear empty.
			}
		}

		return ember;
	}

	/// <summary>
	/// Delete an xform at the specified index.
	/// Shuffle xaos if present to reflect the deleted xform.
	/// </summary>
	/// <param name="i">The index to delete</param>
	/// <returns>True if success, else false.</returns>
	bool DeleteXform(size_t i)
	{
		if (i < XformCount())
		{
			m_Xforms.erase(m_Xforms.begin() + i);

			//Now shuffle xaos values from i on back by 1 for every xform.
			for (size_t x1 = 0; x1 < XformCount(); x1++)
			{
				if (auto xform = GetXform(x1))
				{
					for (size_t x2 = i + 1; x2 <= XformCount(); x2++)//Iterate from the position after the deletion index up to the old count.
						xform->SetXaos(x2 - 1, xform->Xaos(x2));

					xform->TruncateXaos();//Make sure no old values are hanging around in case more xforms are added to this ember later.
				}
			}

			return true;
		}

		return false;
	}

	/// <summary>
	/// Delete the xform at the specified index, including the final one.
	/// </summary>
	/// <param name="i">The index to delete</param>
	/// <param name="forceFinal">If true, delete the final xform when its index is passed in even if one is not present. Default: false.</param>
	/// <returns>True if success, else false.</returns>
	bool DeleteTotalXform(size_t i, bool forceFinal = false)
	{
		if (DeleteXform(i))
		{ }
		else if (i == XformCount() && (forceFinal || UseFinalXform()))
			m_FinalXform.Clear();
		else
			return false;

		return true;
	}

	/// <summary>
	/// Get a pointer to the xform at the specified index, excluding the final one.
	/// </summary>
	/// <param name="i">The index to get</param>
	/// <returns>A pointer to the xform at the index if successful, else nullptr.</returns>
	Xform<T>* GetXform(size_t i) const
	{
		if (i < XformCount())
			return const_cast<Xform<T>*>(&m_Xforms[i]);
		else
			return nullptr;
	}

	/// <summary>
	/// Get a pointer to the xform at the specified index, including the final one.
	/// </summary>
	/// <param name="i">The index to get</param>
	/// <param name="forceFinal">If true, return the final xform when its index is requested even if one is not present. Default: false.</param>
	/// <returns>A pointer to the xform at the index if successful, else nullptr.</returns>
	Xform<T>* GetTotalXform(size_t i, bool forceFinal = false) const
	{
		if (i < XformCount())
			return const_cast<Xform<T>*>(&m_Xforms[i]);
		else if (i == XformCount() && (forceFinal || UseFinalXform()))
			return const_cast<Xform<T>*>(&m_FinalXform);
		else
			return nullptr;
	}

	/// <summary>
	/// Search the xforms, excluding final, to find which one's address matches the address of the specified xform.
	/// </summary>
	/// <param name="xform">A pointer to the xform to find</param>
	/// <returns>The index of the matched xform if found, else -1.</returns>
	intmax_t GetXformIndex(const Xform<T>* xform) const
	{
		intmax_t index = -1;

		for (size_t i = 0; i < m_Xforms.size(); i++)
			if (GetXform(i) == xform)
				return static_cast<intmax_t>(i);

		return index;
	}

	/// <summary>
	/// Search the xforms, including final, to find which one's address matches the address of the specified xform.
	/// </summary>
	/// <param name="xform">A pointer to the xform to find</param>
	/// <param name="forceFinal">If true, return the index of the final xform when its pointer is passed, even if a final is not present. Default: false.</param>
	/// <returns>The index of the matched xform if found, else -1.</returns>
	intmax_t GetTotalXformIndex(const Xform<T>* xform, bool forceFinal = false) const
	{
		const auto totalXformCount = TotalXformCount(forceFinal);

		for (size_t i = 0; i < totalXformCount; i++)
			if (GetTotalXform(i, forceFinal) == xform)
				return static_cast<intmax_t>(i);

		return -1;
	}

	/// <summary>
	/// Assign the final xform.
	/// </summary>
	/// <param name="xform">The xform to copy and assign to the final xform</param>
	void SetFinalXform(const Xform<T>& xform)
	{
		m_FinalXform = xform;
		m_FinalXform.CacheColorVals();
		m_FinalXform.ParentEmber(this);
	}

	/// <summary>
	/// Delete the final xform.
	/// </summary>
	void DeleteFinalXform()
	{
		m_FinalXform.ClearAndDeleteVariations();
	}

	/// <summary>
	/// Determine whether the specified xform has the same address as the final xform.
	/// </summary>
	/// <param name="xform">A pointer to the xform to test</param>
	/// <returns>True if matched, else false.</returns>
	bool IsFinalXform(const Xform<T>* xform) const
	{
		return &m_FinalXform == xform;
	}

	/// <summary>
	/// Delete all motion elements from all xforms including final.
	/// </summary>
	void DeleteMotionElements()
	{
		size_t i = 0;

		while (auto xform = GetTotalXform(i++))
			xform->DeleteMotionElements();

		m_EmberMotionElements.clear();
	}

	/// <summary>
	/// Call CacheColorVals() and SetPrecalcFlags() on all xforms including final.
	/// </summary>
	void CacheXforms()
	{
		size_t i = 0;

		while (auto xform = GetTotalXform(i++))
		{
			xform->CacheColorVals();
			xform->SetPrecalcFlags();
		}
	}

	/// <summary>
	/// Set the projection function pointer based on the
	/// values of the 3D fields.
	/// </summary>
	void SetProjFunc()
	{
		size_t projBits = ProjBits();

		if (!projBits)//No 3D at all, then do nothing.
		{
			m_ProjFunc = &EmberNs::Ember<T>::ProjectNone;
		}
		else
		{
			m_CamMat[0][0] =  std::cos(-m_CamYaw);
			m_CamMat[1][0] = -std::sin(-m_CamYaw);
			m_CamMat[2][0] = 0;
			m_CamMat[0][1] =  std::cos(m_CamPitch) * std::sin(-m_CamYaw);
			m_CamMat[1][1] =  std::cos(m_CamPitch) * std::cos(-m_CamYaw);
			m_CamMat[2][1] = -std::sin(m_CamPitch);
			m_CamMat[0][2] =  std::sin(m_CamPitch) * std::sin(-m_CamYaw);
			m_CamMat[1][2] =  std::sin(m_CamPitch) * std::cos(-m_CamYaw);
			m_CamMat[2][2] =  std::cos(m_CamPitch);

			if (projBits & static_cast<et>(eProjBits::PROJBITS_BLUR))
			{
				if (projBits & static_cast<et>(eProjBits::PROJBITS_YAW))
					m_ProjFunc = &EmberNs::Ember<T>::ProjectPitchYawDepthBlur;
				else
					m_ProjFunc = &EmberNs::Ember<T>::ProjectPitchDepthBlur;
			}
			else if ((projBits & static_cast<et>(eProjBits::PROJBITS_PITCH)) || (projBits & static_cast<et>(eProjBits::PROJBITS_YAW)))
			{
				if (projBits & static_cast<et>(eProjBits::PROJBITS_YAW))
					m_ProjFunc = &EmberNs::Ember<T>::ProjectPitchYaw;
				else
					m_ProjFunc = &EmberNs::Ember<T>::ProjectPitch;
			}
			else
			{
				m_ProjFunc = &EmberNs::Ember<T>::ProjectZPerspective;
			}
		}

		m_BlurCoef = static_cast<T>(0.1) * m_CamDepthBlur;
	}

	/// <summary>
	/// Determine whether xaos is used in any xform, excluding final.
	/// </summary>
	/// <returns>True if any xaos found, else false.</returns>
	bool XaosPresent() const
	{
		bool b = false;

		for (auto& xform : m_Xforms) b |= xform.XaosPresent();//If at least one entry is not equal to 1, then xaos is present.

		return b;
	}

	/// <summary>
	/// Remove all xaos from this ember.
	/// </summary>
	void ClearXaos()
	{
		for (auto& xform : m_Xforms) xform.ClearXaos();
	}

	/// <summary>
	/// Change the size of the final output image and adjust the pixels per unit
	/// so that the orientation of the image remains the same in the new size.
	/// </summary>
	/// <param name="width">New width</param>
	/// <param name="height">New height</param>
	/// <param name="onlyScaleIfNewIsSmaller">True to only scale if the new dimensions are smaller than the original, else always scale.</param>
	/// <param name="scaleType">Scale type used to specify how the image should be zoomed using the scale variable with respect to the new size. Possible scaling modes are: width, height, none.</param>
	void SetSizeAndAdjustScale(size_t width, size_t height, bool onlyScaleIfNewIsSmaller, eScaleType scaleType)
	{
		if ((onlyScaleIfNewIsSmaller && (width < m_OrigFinalRasW || height < m_OrigFinalRasH)) || !onlyScaleIfNewIsSmaller)
		{
			if (scaleType == eScaleType::SCALE_WIDTH)
				m_PixelsPerUnit = m_OrigPixPerUnit * (static_cast<T>(width) / static_cast<T>(m_OrigFinalRasW));
			else if (scaleType == eScaleType::SCALE_HEIGHT)
				m_PixelsPerUnit = m_OrigPixPerUnit * (static_cast<T>(height) / static_cast<T> (m_OrigFinalRasH));
			else
				m_PixelsPerUnit = m_OrigPixPerUnit;
		}

		m_ScaleType = scaleType;
		m_FinalRasW = width;
		m_FinalRasH = height;
	}

	/// <summary>
	/// Set the original final output image dimensions to be equal to the current ones.
	/// </summary>
	void SyncSize()
	{
		m_OrigFinalRasW = m_FinalRasW;
		m_OrigFinalRasH = m_FinalRasH;
		m_OrigPixPerUnit = m_PixelsPerUnit;
	}

	/// <summary>
	/// Set the current final output image dimensions to be equal to the original ones.
	/// </summary>
	void RestoreSize()
	{
		m_FinalRasW = m_OrigFinalRasW;
		m_FinalRasH = m_OrigFinalRasH;
		m_PixelsPerUnit = m_OrigPixPerUnit;
	}

	/// <summary>
	/// Set all xform weights to 1 / xform count.
	/// </summary>
	void EqualizeWeights()
	{
		T weight = static_cast<T>(1) / m_Xforms.size();

		for (auto& xform : m_Xforms) xform.m_Weight = weight;
	}

	/// <summary>
	/// Calculates the normalized weights of the xforms and places them in the passed in vector.
	/// <param name="normalizedWeights">A vector to hold the normalized weights</param>
	/// </summary>
	void CalcNormalizedWeights(vector<T>& normalizedWeights)
	{
		T norm = 0;
		size_t i = 0;

		if (normalizedWeights.size() != m_Xforms.size())
			normalizedWeights.resize(m_Xforms.size());

		for (auto& xform : m_Xforms) norm += xform.m_Weight;

		for (auto& weight : normalizedWeights) { weight = (norm == static_cast<T>( 0 ) ? static_cast<T>( 0 ) : m_Xforms[i].m_Weight / norm); i++; }
	}

	/// <summary>
	/// Get a vector of pointers to all variations present in all Xforms, with no duplicates.
	/// Meaning, that if a variation is present in more than one Xform, only the first one encountered
	/// will be added.
	/// <param name="variations">A vector to hold pointers to the variations present. This will be cleared first.</param>
	/// </summary>
	void GetPresentVariations(vector<Variation<T>*>& variations, bool baseOnly = true) const
	{
		size_t i = 0, xformIndex = 0, totalVarCount = m_FinalXform.TotalVariationCount();
		variations.clear();

		while (auto xform = GetTotalXform(xformIndex++))
			totalVarCount += xform->TotalVariationCount();

		xformIndex = 0;
		variations.reserve(totalVarCount);

		while (auto xform = GetTotalXform(xformIndex++))
		{
			i = 0;
			totalVarCount = xform->TotalVariationCount();

			while (Variation<T>* tempVar = xform->GetVariation(i++))
			{
				if (baseOnly)
				{
					string tempVarBaseName = tempVar->BaseName();

					if (!FindIf(variations, [&] (const Variation<T>* var) -> bool { return tempVar->VariationId() == var->VariationId(); }) &&
							!FindIf(variations, [&] (const Variation<T>* var) -> bool { return tempVarBaseName == var->BaseName(); }))
						variations.push_back(tempVar);
				}
				else
				{
					if (!FindIf(variations, [&] (const Variation<T>* var) -> bool { return tempVar->VariationId() == var->VariationId(); }))
						variations.push_back(tempVar);
				}
			}
		}
	}

	/// <summary>
	/// Compute the total number of state fields within all variations of all xforms.
	/// </summary>
	/// <returns>The number of state fields</returns>
	size_t GetVariationStateParamCount() const
	{
		size_t count = 0, i = 0, j = 0;

		while (const auto xform = GetTotalXform(i++))
			for (j = 0; j < xform->TotalVariationCount(); j++)
				if (const auto var = xform->GetVariation(j))
					count += var->StateParamCount();

		return count;
	}

	/// <summary>
	/// Flatten all xforms by adding a flatten variation if none is present, and if none of the
	/// variations or parameters in the vector are not present.
	/// </summary>
	/// <param name="names">Vector of variation and parameter names that inhibit flattening</param>
	/// <returns>True if flatten was added to any of the xforms, false if it already was present or if none of the specified variations or parameters were present.</returns>
	bool Flatten(vector<string>& names)
	{
		bool flattened = false;
		size_t i = 0;

		while (auto xform = GetTotalXform(i++))
			flattened |= xform->Flatten(names);

		return flattened;
	}

	/// <summary>
	/// Unflatten all xforms by removing flatten, pre_flatten and post_flatten.
	/// </summary>
	/// <returns>True if any flatten was removed, false if it wasn't present.</returns>
	bool Unflatten()
	{
		size_t xformIndex = 0;
		bool unflattened = false;

		while (auto xform = GetTotalXform(xformIndex++))
		{
			unflattened |= xform->DeleteVariationById(eVariationId::VAR_PRE_FLATTEN);
			unflattened |= xform->DeleteVariationById(eVariationId::VAR_FLATTEN);
			unflattened |= xform->DeleteVariationById(eVariationId::VAR_POST_FLATTEN);
		}

		return unflattened;
	}

	/// <summary>
	/// Thin wrapper around Interpolate() which takes a vector of embers rather than a pointer and size.
	/// All embers are expected to be aligned, including the final xform. If not the behavior is undefined.
	/// </summary>
	/// <param name="embers">Vector of embers</param>
	/// <param name="coefs">Coefficients vector which must be the same length as the vector of embers</param>
	/// <param name="stagger">Stagger if greater than 0</param>
	void Interpolate(const vector<Ember<T>>& embers, vector<T>& coefs, T stagger)
	{
		Interpolate(embers.data(), embers.size(), coefs, stagger);
	}

	/// <summary>
	/// Interpolate the specified embers using the coefficients supplied.
	/// All embers are expected to be aligned, including the final xform. If not the behavior is undefined.
	/// </summary>
	/// <param name="embers">Pointer to buffer of embers</param>
	/// <param name="size">Number of elements in the buffer of embers</param>
	/// <param name="coefs">Coefficients vector which must be the same length as the vector of embers</param>
	/// <param name="stagger">Stagger if greater than 0</param>
	void Interpolate(const Ember<T>* embers, size_t size, vector<T>& coefs, T stagger)
	{
		if (size != coefs.size() || size < 2)
			return;

		bool allID, final;
		size_t l, maxXformCount, totalXformCount;
		T bgAlphaSave = m_Background.a;
		T coefSave[2] {0, 0};
		vector<Xform<T>*> xformVec;

		//Palette and others
		if (embers[0].m_PaletteInterp == ePaletteInterp::INTERP_HSV)
		{
			for (glm::length_t i = 0; i < 256; i++)
			{
				float t[3], s[4] = { 0, 0, 0, 0 };

				for (glm::length_t k = 0; k < size; k++)
				{
					Palette<float>::RgbToHsv(glm::value_ptr(embers[k].m_Palette[i]), t);

					for (size_t j = 0; j < 3; j++)
						s[j] += static_cast<float>(coefs[k]) * t[j];

					s[3] += static_cast<float>(coefs[k]) * embers[k].m_Palette[i][3];
				}

				Palette<float>::HsvToRgb(s, glm::value_ptr(m_Palette[i]));
				m_Palette[i][3] = s[3];

				for (glm::length_t j = 0; j < 4; j++)
					Clamp<float>(m_Palette[i][j], 0, 1);
			}
		}
		else if (embers[0].m_PaletteInterp == ePaletteInterp::INTERP_SWEEP)
		{
			//Sweep - not the best option for float indices.
			for (glm::length_t i = 0; i < 256; i++)
			{
				size_t j = (i < (256 * coefs[0])) ? 0 : 1;
				m_Palette[i] = embers[j].m_Palette[i];
			}
		}

		m_Palette.m_Index = -1;//Random.
		m_Symmetry = 0;
		m_SpatialFilterType = embers[0].m_SpatialFilterType;
		m_TemporalFilterType = embers[0].m_TemporalFilterType;
		m_PaletteMode = embers[0].m_PaletteMode;
		m_AffineInterp = embers[0].m_AffineInterp;
		//Interpolate ember parameters, these should be in the same order the members are declared.
		InterpI<&Ember<T>::m_FinalRasW>(embers, coefs, size);
		InterpI<&Ember<T>::m_FinalRasH>(embers, coefs, size);
		InterpI<&Ember<T>::m_SubBatchSize>(embers, coefs, size);
		InterpT<&Ember<T>::m_RandPointRange>(embers, coefs, size);
		InterpI<&Ember<T>::m_FuseCount>(embers, coefs, size);
		InterpI<&Ember<T>::m_Supersample>(embers, coefs, size);
		InterpI<&Ember<T>::m_TemporalSamples>(embers, coefs, size);
		InterpT<&Ember<T>::m_Quality>(embers, coefs, size);
		InterpT<&Ember<T>::m_PixelsPerUnit>(embers, coefs, size);
		InterpT<&Ember<T>::m_Zoom>(embers, coefs, size);
		InterpT<&Ember<T>::m_CamZPos>(embers, coefs, size);
		InterpT<&Ember<T>::m_CamPerspective>(embers, coefs, size);
		InterpT<&Ember<T>::m_CamYaw>(embers, coefs, size);
		InterpT<&Ember<T>::m_CamPitch>(embers, coefs, size);
		InterpT<&Ember<T>::m_CamDepthBlur>(embers, coefs, size);
		InterpT<&Ember<T>::m_BlurCurve>(embers, coefs, size);
		InterpX<m3T, &Ember<T>::m_CamMat>(embers, coefs, size);
		InterpT<&Ember<T>::m_CenterX>(embers, coefs, size);
		InterpT<&Ember<T>::m_CenterY>(embers, coefs, size);
		InterpT<&Ember<T>::m_Rotate>(embers, coefs, size);
		InterpT<&Ember<T>::m_Brightness>(embers, coefs, size);
		InterpT<&Ember<T>::m_Gamma>(embers, coefs, size);
		InterpT<&Ember<T>::m_Vibrancy>(embers, coefs, size);
		InterpT<&Ember<T>::m_GammaThresh>(embers, coefs, size);
		InterpT<&Ember<T>::m_HighlightPower>(embers, coefs, size);
		InterpT<&Ember<T>::m_K2>(embers, coefs, size);
		InterpX<Color<T>, &Ember<T>::m_Background>(embers, coefs, size); m_Background.a = bgAlphaSave;//Don't interp alpha.
		InterpT<&Ember<T>::m_TemporalFilterExp>(embers, coefs, size);
		InterpT<&Ember<T>::m_TemporalFilterWidth>(embers, coefs, size);
		InterpT<&Ember<T>::m_MaxRadDE>(embers, coefs, size);
		InterpT<&Ember<T>::m_MinRadDE>(embers, coefs, size);
		InterpT<&Ember<T>::m_CurveDE>(embers, coefs, size);
		InterpT<&Ember<T>::m_SpatialFilterRadius>(embers, coefs, size);

		//At this point, all of the curves at a given curve index (0 - 3) should have the same number of spline points across all embers.
		for (size_t i = 0; i < embers[0].m_Curves.m_Points.size(); i++)//4 point arrays.
		{
			m_Curves.m_Points[i].clear();

			while (m_Curves.m_Points[i].size() < embers[0].m_Curves.m_Points[i].size())
				m_Curves.m_Points[i].push_back(v2F(0));

			for (size_t j = 0; j < embers[0].m_Curves.m_Points[i].size(); j++)//Same number of points for this curve across all embers, so just use the first one.
			{
				v2F x(0);

				for (size_t k = 0; k < size; k++)//Iterate over all embers.
					x += float(coefs[k]) * embers[k].m_Curves.m_Points[i][j];

				m_Curves.m_Points[i][j] = x;
			}
		}

		//Normally done in assignment, must manually do here.
		SetProjFunc();
		//An extra step needed here due to the OOD that was not needed in the original.
		//A small price to pay for the conveniences it affords us elsewhere.
		//First clear the xforms, and find out the max number of xforms in all of the embers in the list.
		m_Xforms.clear();
		maxXformCount = Interpolater<T>::MaxXformCount(embers, size);//Max number of standard transforms in embers, excluding final.
		//m_Xforms.reserve(maxXformCount);
		final = Interpolater<T>::AnyFinalPresent(embers, size);//Did any embers have a final xform?
		totalXformCount = maxXformCount + (final ? 1 : 0);
		xformVec.reserve(size);

		//Populate the xform list member such that each element is a merge of all of the xforms at that position in all of the embers.
		for (size_t i = 0; i < totalXformCount; i++)//For each xform to populate.
		{
			for (size_t j = 0; j < size; j++)//For each ember in the list.
				if (i < embers[j].TotalXformCount())//Xform in this position in this ember.
					xformVec.push_back(embers[j].GetTotalXform(i));//Temporary list to pass to MergeXforms().

			if (i < maxXformCount)//Working with standard xforms?
				AddXform(Interpolater<T>::MergeXforms(xformVec, true));//Merge, set weights to zero, and add to the xform list.
			else if (final)//Or is it the final xform (i will be == maxXformCount)?
				m_FinalXform = Interpolater<T>::MergeXforms(xformVec, true);

			xformVec.clear();
		}

		//Now have a merged list, so interpolate the weight values.
		//This includes all xforms plus final.
		for (size_t i = 0; i < totalXformCount; i++)
		{
			auto thisXform = GetTotalXform(i);

			//if (i == 10)
			//	cout << i << endl;

			if (size == 2 && stagger > 0 && thisXform != &m_FinalXform)
			{
				coefSave[0] = coefs[0];
				coefSave[1] = coefs[1];
				coefs[0] = Interpolater<T>::GetStaggerCoef(coefSave[0], stagger, XformCount(), i);//Standard xform count without final.
				coefs[1] = 1 - coefs[0];
			}

			for (size_t j = 0; j < thisXform->TotalVariationCount(); j++)//For each variation in this xform.
			{
				Variation<T>* var = thisXform->GetVariation(j);
				ParametricVariation<T>* parVar = dynamic_cast<ParametricVariation<T>*>(var);//Will use below if it's parametric.
				var->m_Weight = 0;

				if (parVar)
					parVar->Clear();

				for (size_t k = 0; k < size; k++)//For each ember in the list.
				{
					auto tempXform = embers[k].GetTotalXform(i);//Xform in this position in this ember, including final.

					if (tempXform)
					{
						Variation<T>* tempVar = tempXform->GetVariationById(var->VariationId());//See if the variation at this xform index exists in that ember at this xform index.

						if (tempVar)
						{
							//Interp weight.
							var->m_Weight += tempVar->m_Weight * coefs[k];

							//If it was a parametric variation, interp params.
							if (parVar)
							{
								ParametricVariation<T>* tempParVar = dynamic_cast<ParametricVariation<T>*>(tempVar);

								if (tempParVar && (parVar->ParamCount() == tempParVar->ParamCount()))//This check will should always be true, but just check to be absolutely sure to avoid clobbering memory.
								{
									auto params = parVar->Params();
									auto tempParams = tempParVar->Params();

									for (l = 0; l < parVar->ParamCount(); l++)
									{
										if (!tempParams[l].IsPrecalc())
											*(params[l].Param()) += tempParams[l].ParamVal() * coefs[k];
									}
								}
							}
						}
					}
				}
			}

			InterpXform<&Xform<T>::m_Weight>	    (thisXform, i, embers, coefs, size);
			InterpXform<&Xform<T>::m_ColorX>	    (thisXform, i, embers, coefs, size);
			InterpXform<&Xform<T>::m_ColorSpeed>    (thisXform, i, embers, coefs, size);
			InterpXform<&Xform<T>::m_Opacity>	    (thisXform, i, embers, coefs, size);
			InterpXform<&Xform<T>::m_Animate>	    (thisXform, i, embers, coefs, size);
			InterpXform<&Xform<T>::m_AnimateOrigin>	(thisXform, i, embers, coefs, size);
			ClampGte0Ref(thisXform->m_Weight);
			ClampRef<T>(thisXform->m_ColorX, 0, 1);
			ClampRef<T>(thisXform->m_ColorSpeed, -1, 1);

			//Interp affine and post.
			if (m_AffineInterp == eAffineInterp::AFFINE_INTERP_LOG)
			{
				vector<v2T> cxMag(size);
				vector<v2T> cxAng(size);
				vector<v2T> cxTrn(size);
				thisXform->m_Affine.m_Mat = m23T(0);
				//Affine part.
				Interpolater<T>::ConvertLinearToPolar(embers, size, i, false, cxAng, cxMag, cxTrn);
				Interpolater<T>::InterpAndConvertBack(coefs, cxAng, cxMag, cxTrn, thisXform->m_Affine);
				//Post part.
				allID = true;

				for (size_t k = 0; k < size; k++)//For each ember in the list.
				{
					if (i < embers[k].TotalXformCount())//Xform in this position in this ember.
					{
						auto tempXform = embers[k].GetTotalXform(i);
						allID &= tempXform->m_Post.IsID();
					}
				}

				thisXform->m_Post.m_Mat = m23T(0);

				if (allID)
				{
					thisXform->m_Post.A(1);
					thisXform->m_Post.E(1);
				}
				else
				{
					Interpolater<T>::ConvertLinearToPolar(embers, size, i, true, cxAng, cxMag, cxTrn);
					Interpolater<T>::InterpAndConvertBack(coefs, cxAng, cxMag, cxTrn, thisXform->m_Post);
				}
			}
			else if (m_AffineInterp == eAffineInterp::AFFINE_INTERP_LINEAR)
			{
				//Interpolate pre and post affine using coefs.
				allID = true;
				thisXform->m_Affine.m_Mat = m23T(0);
				thisXform->m_Post.m_Mat	  = m23T(0);

				for (size_t k = 0; k < size; k++)
				{
					auto tempXform = embers[k].GetTotalXform(i);//Xform in this position in this ember.

					if (tempXform)
					{
						allID &= tempXform->m_Post.IsID();
						thisXform->m_Affine.m_Mat += (coefs[k] * tempXform->m_Affine.m_Mat);
						thisXform->m_Post.m_Mat	  += (coefs[k] * tempXform->m_Post.m_Mat);
					}
				}

				if (allID)
					thisXform->m_Post.m_Mat = glm::mat2x3(1);
			}

			//Final stagger cleanup goes here.
			if (size == 2 && stagger > 0 && thisXform != &m_FinalXform)
			{
				coefs[0] = coefSave[0];
				coefs[1] = coefSave[1];
			}
		}

		//Normally these functions are called automatically when
		//adding variations to a xform, or when setting a parametric
		//variation value via name lookup. However, since the values
		//were directly written to, must manually call them here.
		CacheXforms();

		//Need to merge xaos. Original does xaos all the time, but really only need to do it if at least one xform in at least one ember uses it, else skip.
		//Omit final xform from xaos processing.
		if (Interpolater<T>::AnyXaosPresent(embers, size))
		{
			for (size_t i = 0; i < XformCount(); i++)
			{
				for (size_t k = 0; k < XformCount(); k++)//First make each xform xaos array be maxXformCount elements long and set them to zero.
					m_Xforms[i].SetXaos(k, 0);

				//Now fill them with interpolated values.
				for (size_t j = 0; j < size; j++)//For each ember in the list.
				{
					auto tempXform = embers[j].GetXform(i);

					for (size_t k = 0; k < XformCount(); k++)//For each xaos entry in this xform's xaos array, sum it with the same entry in all of the embers multiplied by the coef for that ember.
					{
						m_Xforms[i].SetXaos(k, m_Xforms[i].Xaos(k) + tempXform->Xaos(k) * coefs[j]);
					}
				}

				//Make sure no xaos entries for this xform were less than zero.
				for (size_t k = 0; k < XformCount(); k++)
					if (m_Xforms[i].Xaos(k) < 0)
						m_Xforms[i].SetXaos(k, 0);
			}
		}
	}

	/// <summary>
	/// Thin wrapper around InterpolateCatmullRom().
	/// The ember vector is expected to be a length of 4 with all xforms aligned, including final.
	/// </summary>
	/// <param name="embers">Vector of embers</param>
	/// <param name="t">Used in calculating Catmull-Rom coefficients</param>
	void InterpolateCatmullRom(const vector<Ember<T>>& embers, T t)
	{
		InterpolateCatmullRom(embers.data(), embers.size(), t);
	}

	/// <summary>
	/// Use Catmull-Rom coefficients and pass to Interpolate().
	/// The ember array is expected to be a length of 4 with all xforms aligned, including final.
	/// </summary>
	/// <param name="embers">Pointer to buffer of embers</param>
	/// <param name="size">Number of elements in the buffer of embers</param>
	/// <param name="t">Used in calculating Catmull-Rom coefficients</param>
	void InterpolateCatmullRom(const Ember<T>* embers, size_t size, T t)
	{
		T t2 = t * t;
		T t3 = t2 * t;
		vector<T> cmc(4);
		cmc[0] = (2 * t2 - t - t3) / 2;
		cmc[1] = (3 * t3 - 5 * t2 + 2) / 2;
		cmc[2] = (4 * t2 - 3 * t3 + t) / 2;
		cmc[3] = (t3 - t2) / 2;
		Interpolate(embers, size, cmc, 0);
	}

	/// <summary>
	/// Rotate all pre-affine transforms in non-final xforms whose animate value is non-zero by
	/// the specified angle in a counter-clockwise direction.
	/// Omit padding xforms.
	/// Do not rotate post affine transforms.
	/// </summary>
	/// <param name="angle">The angle to rotate by</param>
	void RotateAffines(T angle)
	{
		size_t i = 0;

		while (auto xform = GetTotalXform(i++))//Flam3 only allowed animation with normal xforms. This has been changed to allow animations of final xforms.
		{
			//Assume that if there are no variations, then it's a padding xform.
			if (xform->Empty() && m_AffineInterp != eAffineInterp::AFFINE_INTERP_LOG)
				continue;

			//Don't rotate xforms with animate set to 0.
			if (xform->m_Animate != 0)
				xform->m_Affine.Rotate(angle * DEG_2_RAD_T);

			if (xform->m_AnimateOrigin != 0)
				xform->m_Affine.RotateTrans(angle * DEG_2_RAD_T);

			//Don't rotate post.
		}
	}

	/// <summary>
	/// Adds symmetry to this ember by adding additional xforms.
	/// sym = 2 or more means rotational.
	/// sym = 1 means identity, ie no symmetry.
	/// sym = 0 means pick a random symmetry (maybe none).
	/// sym = -1 means bilateral (reflection).
	/// sym = -2 or less means rotational and reflective.
	/// </summary>
	/// <param name="sym">The type of symmetry to add</param>
	/// <param name="rand">The random context to use for generating random symmetry</param>
	void AddSymmetry(intmax_t sym, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		intmax_t k = 0;
		size_t i = 0, result = 0;
		T a;

		if (sym == 0)
		{
			static intmax_t symDistrib[] =
			{
				-4, -3,
				-2, -2, -2,
				-1, -1, -1,
				2,  2,  2,
				3,  3,
				4,  4,
			};

			if (rand.RandBit())
				sym = symDistrib[rand.Rand(Vlen(symDistrib))];
			else if (rand.Rand() & 31)
				sym = intmax_t{ rand.Rand(13) } - 6;
			else
				sym = intmax_t{ rand.Rand(51) } - 25;
		}

		if (sym == 1 || sym == 0)
			return;

		m_Symmetry = sym;

		if (sym < 0)
		{
			i = XformCount();//Don't apply sym to final.
			Xform<T> xform;
			AddXform(xform);
			m_Xforms[i].m_Weight = 1;
			m_Xforms[i].m_ColorSpeed = 0;
			m_Xforms[i].m_Animate = 0;
			m_Xforms[i].m_AnimateOrigin = 0;
			m_Xforms[i].m_ColorX = 1;
			m_Xforms[i].m_ColorY = 1;//Added in case 2D palette support is ever added.
			m_Xforms[i].m_Affine.A(-1);
			m_Xforms[i].m_Affine.B(0);
			m_Xforms[i].m_Affine.C(0);
			m_Xforms[i].m_Affine.D(0);
			m_Xforms[i].m_Affine.E(1);
			m_Xforms[i].m_Affine.F(0);
			m_Xforms[i].AddVariation(m_VariationList->GetVariationCopy(eVariationId::VAR_LINEAR));
			result++;
			sym = -sym;
		}

		a = static_cast<T>(2 * M_PI / sym);

		for (k = 1; k < sym; k++)
		{
			i = XformCount();
			Xform<T> xform;
			AddXform(xform);
			m_Xforms[i].m_Weight = 1;
			m_Xforms[i].m_ColorSpeed = 0;
			m_Xforms[i].m_Animate = 0;
			m_Xforms[i].m_AnimateOrigin = 0;
			m_Xforms[i].m_ColorX = m_Xforms[i].m_ColorY = (sym < 3) ? 0 : (static_cast<T>(k - 1) / static_cast<T>(sym - 2));//Added Y.
			m_Xforms[i].m_Affine.A(Round6(std::cos(k * a)));
			m_Xforms[i].m_Affine.D(Round6(std::sin(k * a)));
			m_Xforms[i].m_Affine.B(Round6(-m_Xforms[i].m_Affine.D()));
			m_Xforms[i].m_Affine.E(m_Xforms[i].m_Affine.A());
			m_Xforms[i].m_Affine.C(0);
			m_Xforms[i].m_Affine.F(0);
			m_Xforms[i].AddVariation(m_VariationList->GetVariationCopy(eVariationId::VAR_LINEAR));
			result++;
		}

		//Sort the newly added xforms, do not touch the original ones.
		std::sort(m_Xforms.end() - result, m_Xforms.end(), &Interpolater<T>::CompareXforms);
	}

	/// <summary>
	/// Return a uint with bits set to indicate which kind of projection should be done.
	/// </summary>
	/// <param name="onlyScaleIfNewIsSmaller">A uint with bits set for each kind of projection that is needed</param>
	size_t ProjBits() const
	{
		size_t val = 0;

		if (m_CamZPos != 0) val |= static_cast<et>(eProjBits::PROJBITS_ZPOS);

		if (m_CamPerspective != 0) val |= static_cast<et>(eProjBits::PROJBITS_PERSP);

		if (m_CamPitch != 0) val |= static_cast<et>(eProjBits::PROJBITS_PITCH);

		if (m_CamYaw != 0) val |= static_cast<et>(eProjBits::PROJBITS_YAW);

		if (m_CamDepthBlur != 0) val |= static_cast<et>(eProjBits::PROJBITS_BLUR);

		return val;
	}

	/// <summary>
	/// Call the appropriate projection function via function pointer.
	/// </summary>
	/// <param name="point">The point to project</param>
	/// <param name="rand">The Isaac object to pass to the projection functions</param>
	inline void Proj(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand, const CarToRas<T>& ctr)
	{
		(this->*m_ProjFunc)(point, rand, ctr);
	}

	/// <summary>
	/// Placeholder to do nothing.
	/// </summary>
	/// <param name="point">Ignored</param>
	/// <param name="rand">Ignored</param>
	void ProjectNone(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand, const CarToRas<T>& ctr)
	{
	}

	/// <summary>
	/// Project when only z is set, and not pitch, yaw, projection or depth blur.
	/// </summary>
	/// <param name="point">The point to project</param>
	/// <param name="rand">Ignored</param>
	void ProjectZPerspective(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand, const CarToRas<T>& ctr)
	{
		T zr = Zeps(1 - m_CamPerspective * (point.m_Z - m_CamZPos));
		point.m_X /= zr;
		point.m_Y /= zr;
		point.m_Z -= m_CamZPos;
	}

	/// <summary>
	/// Project when pitch, and optionally z and perspective are set, but not depth blur or yaw.
	/// </summary>
	/// <param name="point">The point to project</param>
	/// <param name="rand">Ignored</param>
	void ProjectPitch(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand, const CarToRas<T>& ctr)
	{
		T z  = point.m_Z - m_CamZPos;
		T y  = m_CamMat[1][1] * point.m_Y + m_CamMat[2][1] * z;
		T zr = Zeps(1 - m_CamPerspective * (m_CamMat[1][2] * point.m_Y + m_CamMat[2][2] * z));
		point.m_X /= zr;
		point.m_Y  = y / zr;
		point.m_Z -= m_CamZPos;
	}

	/// <summary>
	/// Project when depth blur, and optionally pitch, perspective and z are set, but not yaw.
	/// </summary>
	/// <param name="point">The point to project</param>
	/// <param name="rand">Used for blurring</param>
	void ProjectPitchDepthBlur(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand, const CarToRas<T>& ctr)
	{
		T y, z, zr;
		T dsin, dcos;
		T t = rand.Frand01<T>() * M_2PI;
		z = point.m_Z - m_CamZPos;
		y = m_CamMat[1][1] * point.m_Y + m_CamMat[2][1] * z;
		z = m_CamMat[1][2] * point.m_Y + m_CamMat[2][2] * z;
		zr = Zeps(1 - m_CamPerspective * z);
		sincos(t, &dsin, &dcos);
		const T prcx = point.m_X / ctr.CachedCarHalfX();
		const T prcy = y / ctr.CachedCarHalfY();
		const T dist = VarFuncs<T>::Hypot(prcx, prcy) * 10;
		const T scale = m_BlurCurve ? (Sqr(dist) / (4 * m_BlurCurve)) : static_cast<T>(1);
		const T dr = rand.Frand01<T>() * (m_BlurCoef * scale) * z;
		point.m_X = (point.m_X + dr * dcos) / zr;
		point.m_Y = (y + dr * dsin) / zr;
		point.m_Z -= m_CamZPos;
	}

	/// <summary>
	/// Project when depth blur, yaw and optionally pitch are set, but not perspective and z.
	/// </summary>
	/// <param name="point">The point to project</param>
	/// <param name="rand">Used for blurring</param>
	void ProjectPitchYawDepthBlur(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand, const CarToRas<T>& ctr)
	{
		T dsin, dcos;
		const T t = rand.Frand01<T>() * M_2PI;
		T z = point.m_Z - m_CamZPos;
		const T x = m_CamMat[0][0] * point.m_X + m_CamMat[1][0] * point.m_Y;
		const T y = m_CamMat[0][1] * point.m_X + m_CamMat[1][1] * point.m_Y + m_CamMat[2][1] * z;
		z = m_CamMat[0][2] * point.m_X + m_CamMat[1][2] * point.m_Y + m_CamMat[2][2] * z;
		const T zr = Zeps(1 - m_CamPerspective * z);
		const T prcx = x / ctr.CachedCarHalfX();
		const T prcy = y / ctr.CachedCarHalfY();
		const T dist = VarFuncs<T>::Hypot(prcx, prcy) * 10;
		const T scale = m_BlurCurve ? (Sqr(dist) / (4 * m_BlurCurve)) : static_cast<T>(1);
		const T dr = rand.Frand01<T>() * (m_BlurCoef * scale) * z;
		sincos(t, &dsin, &dcos);
		point.m_X = (x + dr * dcos) / zr;
		point.m_Y = (y + dr * dsin) / zr;
		point.m_Z -= m_CamZPos;
	}

	/// <summary>
	/// Project when yaw and optionally pitch, z, and perspective are set, but not depth blur.
	/// </summary>
	/// <param name="point">The point to project</param>
	/// <param name="rand">Ignored</param>
	void ProjectPitchYaw(Point<T>& point, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand, const CarToRas<T>& ctr)
	{
		const T z = point.m_Z - m_CamZPos;
		const T x = m_CamMat[0][0] * point.m_X + m_CamMat[1][0] * point.m_Y;
		const T y = m_CamMat[0][1] * point.m_X + m_CamMat[1][1] * point.m_Y + m_CamMat[2][1] * z;
		const T zr = Zeps(1 - m_CamPerspective * (m_CamMat[0][2] * point.m_X + m_CamMat[1][2] * point.m_Y + m_CamMat[2][2] * z));
		point.m_X = x / zr;
		point.m_Y = y / zr;
		point.m_Z -= m_CamZPos;
	}

#define APP_FMP(x) x += param.second * Interpolater<T>::MotionFuncs(motion.m_MotionFunc, motion.m_MotionFreq * (blend + motion.m_MotionOffset))

	/// <summary>
	/// Update ember parameters based on stored motion elements
	/// </summary>
	/// <param name="blend">The time percentage value which dictates how much of a percentage of 360 degrees it should be rotated and the time position for the motion elements</param>
	void ApplyFlameMotion(T blend)
	{
		for (size_t i = 0; i < m_EmberMotionElements.size(); ++i)
		{
			auto& motion = m_EmberMotionElements[i];

			for (size_t j = 0; j < motion.m_MotionParams.size(); ++j)
			{
				auto& param = motion.m_MotionParams[j];

				switch (param.first)
				{
					case eEmberMotionParam::FLAME_MOTION_ZOOM:
						APP_FMP(m_Zoom);
						break;

					case eEmberMotionParam::FLAME_MOTION_ZPOS:
						APP_FMP(m_CamZPos);
						break;

					case eEmberMotionParam::FLAME_MOTION_PERSPECTIVE:
						APP_FMP(m_CamPerspective);
						break;

					case eEmberMotionParam::FLAME_MOTION_YAW:
						APP_FMP(m_CamYaw);
						break;

					case eEmberMotionParam::FLAME_MOTION_PITCH:
						APP_FMP(m_CamPitch);
						break;

					case eEmberMotionParam::FLAME_MOTION_DEPTH_BLUR:
						APP_FMP(m_CamDepthBlur);
						break;

					case eEmberMotionParam::FLAME_MOTION_CENTER_X:
						APP_FMP(m_CenterX);
						break;

					case eEmberMotionParam::FLAME_MOTION_CENTER_Y:
						APP_FMP(m_CenterY);
						break;

					case eEmberMotionParam::FLAME_MOTION_ROTATE:
						APP_FMP(m_Rotate);
						break;

					case eEmberMotionParam::FLAME_MOTION_BRIGHTNESS:
						APP_FMP(m_Brightness);
						break;

					case eEmberMotionParam::FLAME_MOTION_GAMMA:
						APP_FMP(m_Gamma);
						break;

					case eEmberMotionParam::FLAME_MOTION_GAMMA_THRESH:
						APP_FMP(m_GammaThresh);
						break;

					case eEmberMotionParam::FLAME_MOTION_HIGHLIGHT_POWER:
						APP_FMP(m_HighlightPower);
						break;

					case eEmberMotionParam::FLAME_MOTION_K2:
						APP_FMP(m_K2);
						break;

					case eEmberMotionParam::FLAME_MOTION_RAND_RANGE:
						APP_FMP(m_RandPointRange);
						break;

					case eEmberMotionParam::FLAME_MOTION_BACKGROUND_R:
						APP_FMP(m_Background.r);
						break;

					case eEmberMotionParam::FLAME_MOTION_BACKGROUND_G:
						APP_FMP(m_Background.g);
						break;

					case eEmberMotionParam::FLAME_MOTION_BACKGROUND_B:
						APP_FMP(m_Background.b);
						break;

					case eEmberMotionParam::FLAME_MOTION_VIBRANCY:
						APP_FMP(m_Vibrancy);
						break;

					case eEmberMotionParam::FLAME_MOTION_BLUR_CURVE:
						APP_FMP(m_BlurCurve);
						break;

					case eEmberMotionParam::FLAME_MOTION_SCALE:
						APP_FMP(m_Zoom);
						break;

					case eEmberMotionParam::FLAME_MOTION_NONE:
					default:
						break;
				}
			}
		}
	}

	/// <summary>
	/// Clear this ember and set to either reasonable or unreasonable default values.
	/// </summary>
	/// <param name="useDefaults">Use reasonable default if true, else use out of bounds values.</param>
	void Clear(bool useDefaults = true)
	{
		m_Palette.m_Index = -1;
		m_CenterX = 0;
		m_CenterY = 0;
		m_RotCenterY = 0;
		m_Gamma = 4;
		m_Vibrancy = 1;
		m_Brightness = 4;
		m_Symmetry = 0;
		m_Rotate = 0;
		m_PixelsPerUnit = 50;
		m_Interp = eInterp::EMBER_INTERP_SMOOTH;
		m_PaletteInterp = ePaletteInterp::INTERP_HSV;
		m_Index = 0;
		m_ParentFilename = "";
		m_ScaleType = eScaleType::SCALE_NONE;

		if (useDefaults)
		{
			//If defaults are on, set to reasonable values.
			m_HighlightPower = 1;
			m_K2 = 0;
			m_Background.Reset();
			m_FinalRasW = 1920;
			m_FinalRasH = 1080;
			m_Supersample = 1;
			m_SpatialFilterRadius = static_cast<T>(0.5);
			m_Zoom = 0;
			m_ProjFunc = &EmberNs::Ember<T>::ProjectNone;
			m_CamZPos = 0;
			m_CamPerspective = 0;
			m_CamYaw = 0;
			m_CamPitch = 0;
			m_CamDepthBlur = 0;
			m_BlurCurve = 0;
			m_BlurCoef = 0;
			m_CamMat = m3T(0);
			m_Quality = 30;
			m_SubBatchSize = 10240;
			m_RandPointRange = 1;
			m_FuseCount = 15;
			m_MaxRadDE = static_cast<T>(9.0);
			m_MinRadDE = 0;
			m_CurveDE = static_cast<T>(0.4);
			m_GammaThresh = static_cast<T>(0.01);
			m_TemporalSamples = 100;
			m_SpatialFilterType = eSpatialFilterType::GAUSSIAN_SPATIAL_FILTER;
			m_AffineInterp = eAffineInterp::AFFINE_INTERP_LOG;
			m_PaletteMode = ePaletteMode::PALETTE_LINEAR;
			m_Interp = eInterp::EMBER_INTERP_SMOOTH;
			m_Stagger = 0;
			m_Rotations = 1;
			m_SecondsPerRotation = 3;
			m_RotateXformsCw = false;
			m_BlendSeconds = 3;
			m_RotationsPerBlend = 0;
			m_BlendRotateXformsCw = false;
			m_Linear = false;
			m_TemporalFilterType = eTemporalFilterType::BOX_TEMPORAL_FILTER;
			m_TemporalFilterWidth = 1;
			m_TemporalFilterExp = 1;
		}
		else
		{
			//Defaults are off, so set to UN-reasonable values.
			m_HighlightPower = -1;
			m_K2 = -1;
			m_Background = Color<T>(-1, -1, -1, 1);
			m_FinalRasW = 0;
			m_FinalRasH = 0;
			m_Supersample = 0;
			m_SpatialFilterRadius = -1;
			m_Zoom = 999999;
			m_ProjFunc = nullptr;
			m_CamZPos = 999999;
			m_CamPerspective = 999999;
			m_CamYaw = 999999;
			m_CamPitch = 999999;
			m_CamDepthBlur = 999999;
			m_BlurCurve = 999999;
			m_BlurCoef = 999999;
			m_CamMat = m3T(999999);
			m_Quality = -1;
			m_SubBatchSize = 0;
			m_RandPointRange = 0;
			m_FuseCount = 0;
			m_MaxRadDE = -1;
			m_MinRadDE = -1;
			m_CurveDE = -1;
			m_GammaThresh = -1;
			m_TemporalSamples = 0;
			m_SpatialFilterType = eSpatialFilterType::GAUSSIAN_SPATIAL_FILTER;
			m_AffineInterp = eAffineInterp::AFFINE_INTERP_LOG;
			m_PaletteMode = ePaletteMode::PALETTE_STEP;
			m_Interp = eInterp::EMBER_INTERP_LINEAR;
			m_Stagger = 999999;
			m_Rotations = 999999;
			m_SecondsPerRotation = 999999;
			m_RotateXformsCw = false;
			m_BlendSeconds = 999999;
			m_RotationsPerBlend = 999999;
			m_BlendRotateXformsCw = false;
			m_Linear = false;
			m_TemporalFilterType = eTemporalFilterType::BOX_TEMPORAL_FILTER;
			m_TemporalFilterWidth = -1;
			m_TemporalFilterExp = -999;
		}

		m_Xforms.clear();
		m_FinalXform.Clear();
		m_Curves.Init();
		ClearEdit();
	}

	/// <summary>
	/// Thin wrapper to clear edit doc if not nullptr and set to nullptr.
	/// </summary>
	void ClearEdit()
	{
		if (m_Edits)
			xmlFreeDoc(m_Edits);

		m_Edits = nullptr;
	}

	/// <summary>
	/// Return a string representation of this ember.
	/// </summary>
	/// <returns>The string representation of this ember</returns>
	string ToString() const
	{
		size_t i;
		ostringstream ss;
		ss << "Final Raster Width: " << m_FinalRasW << "\n"
		   << "Final Raster Height: " << m_FinalRasH << "\n"
		   << "Original Raster Width: " << m_OrigFinalRasW << "\n"
		   << "Original Raster Height: " << m_OrigFinalRasH << "\n"
		   << "Supersample: " << m_Supersample << "\n"
		   << "Temporal Samples: " << m_TemporalSamples << "\n"
		   << "Symmetry: " << m_Symmetry << "\n"
		   << "Quality: " << m_Quality << "\n"
		   << "Pixels Per Unit: " << m_PixelsPerUnit << "\n"
		   << "Original Pixels Per Unit: " << m_OrigPixPerUnit << "\n"
		   << "Initial Point Range: " << m_RandPointRange << "\n"
		   << "Sub Batch Size: " << m_SubBatchSize << "\n"
		   << "Fuse Count: " << m_FuseCount << "\n"
		   << "Zoom: " << m_Zoom << "\n"
		   << "ZPos: " << m_CamZPos << "\n"
		   << "Perspective: " << m_CamPerspective << "\n"
		   << "Yaw: " << m_CamYaw << "\n"
		   << "Pitch: " << m_CamPitch << "\n"
		   << "Blur Curve: " << m_BlurCurve << "\n"
		   << "Depth Blur: " << m_CamDepthBlur << "\n"
		   << "CenterX: " << m_CenterX << "\n"
		   << "CenterY: " << m_CenterY << "\n"
		   << "RotCenterY: " << m_RotCenterY << "\n"
		   << "Rotate: " << m_Rotate << "\n"
		   << "Brightness: " << m_Brightness << "\n"
		   << "Gamma: " << m_Gamma << "\n"
		   << "Vibrancy: " << m_Vibrancy << "\n"
		   << "Gamma Threshold: " << m_GammaThresh << "\n"
		   << "Highlight Power: " << m_HighlightPower << "\n"
		   << "K2: " << m_K2 << "\n"
		   << "Time: " << m_Time << "\n"
		   << "Background: " << m_Background.r << ", " << m_Background.g << ", " << m_Background.b << ", " << m_Background.a << "\n"
		   << "Interp: " << m_Interp << "\n"
		   << "Affine Interp Type: " << m_AffineInterp << "\n"
		   << "Minimum DE Radius: " << m_MinRadDE << "\n"
		   << "Maximum DE Radius: " << m_MaxRadDE << "\n"
		   << "DE Curve: " << m_CurveDE << "\n"
		   << "Spatial Filter Type: " << m_SpatialFilterType << "\n"
		   << "Spatial Filter Radius: " << m_SpatialFilterRadius << "\n"
		   << "Stagger: " << m_Stagger << "\n"
		   << "Rotations: " << m_Rotations << "\n"
		   << "Seconds Per Rotation: " << m_SecondsPerRotation << "\n"
		   << "Rotate Xforms Clockwise: " << m_RotateXformsCw << "\n"
		   << "Blend Seconds: " << m_BlendSeconds << "\n"
		   << "Rotations Per Blend: " << m_RotationsPerBlend << "\n"
		   << "Blend Rotate Xforms Clockwise: " << m_BlendRotateXformsCw << "\n"
		   << "Temporal Filter Type: " << m_TemporalFilterType << "\n"
		   << "Temporal Filter Width: " << m_TemporalFilterWidth << "\n"
		   << "Temporal Filter Exp: " << m_TemporalFilterExp << "\n"
		   << "Palette Mode: " << m_PaletteMode << "\n"
		   << "Palette Interp: " << m_PaletteInterp << "\n"
		   << "Palette Index: " << m_Palette.m_Index << "\n"
		   //Add palette info here if needed.
		   << "Name: " << m_Name << "\n"
		   << "Index: " << m_Index << "\n"
		   << "Scale Type: " << m_ScaleType << "\n"
		   << "Parent Filename: " << m_ParentFilename << "\n"
		   << "\n";

		for (i = 0; i < XformCount(); i++)
		{
			ss << "Xform " << i << ":\n" << m_Xforms[i].ToString() << "\n";
		}

		if (UseFinalXform())
			ss << "Final Xform: " << m_FinalXform.ToString() << "\n";

		return ss.str();
	}

	/// <summary>
	/// Accessors.
	/// </summary>
	inline const Xform<T>* Xforms() const noexcept { return m_Xforms.data(); }
	inline Xform<T>* NonConstXforms() noexcept { return m_Xforms.data(); }
	inline size_t XformCount() const noexcept { return m_Xforms.size(); }
	inline const Xform<T>* FinalXform() const noexcept { return &m_FinalXform; }
	inline Xform<T>* NonConstFinalXform() noexcept { return &m_FinalXform; }
	inline bool UseFinalXform() const noexcept { return !m_FinalXform.Empty(); }
	inline size_t TotalXformCount(bool forceFinal = false) const noexcept { return XformCount() + ((forceFinal || UseFinalXform()) ? 1 : 0); }
	inline int PaletteIndex() const noexcept { return m_Palette.m_Index; }
	inline T BlurCoef() noexcept { return m_BlurCoef; }
	inline eScaleType ScaleType() const noexcept { return m_ScaleType; }

	//The width and height in pixels of the final output image. The size of the histogram and DE filtering buffers will differ from this.
	//Xml fields: "size".
	size_t m_FinalRasW = 1920;
	size_t m_FinalRasH = 1080;
	size_t m_OrigFinalRasW = 1920;//Keep track of the originals read from the Xml, because...
	size_t m_OrigFinalRasH = 1080;//the dimension may change in an editor and the originals are needed for the aspect ratio.
	T m_OrigPixPerUnit = 240;

	//The range in the x and y directions from the center of the world space from which the initial random point will be selected at the start of each sub batch.
	//Or when recovering from a bad point.
	//Xml field: "rand_range".
	T m_RandPointRange = 1;

	//The iteration depth. This was a rendering parameter in flam3 but has been made a member here
	//so that it can be adjusted more easily.
	//Xml field: "sub_batch_size".
	size_t m_SubBatchSize = DEFAULT_SBS;

	//The number of iterations to disregard for each sub batch. This was a rendering parameter in flam3 but has been made a member here
	//so that it can be adjusted more easily.
	//Xml field: "fuse".
	size_t m_FuseCount = 15;

	//The multiplier in size of the histogram and DE filtering buffers. Must be at least one, preferrably never larger than 4, only useful at 2.
	//Xml field: "supersample" or "overample (deprecated)".
	size_t m_Supersample = 1;

	//When animating, split each pass into this many pieces, each doing a fraction of the total iterations. Each temporal sample
	//will render an interpolated instance of the ember that is a fraction of the current ember and the next one.
	//When rendering a single image, this field is always set to 1.
	//Xml field: "temporal_samples".
	size_t m_TemporalSamples = 100;

	//Whether or not any symmetry was added. This field is in a bit of a state of conflict right now as flam3 has a severe bug.
	//Xml field: "symmetry".
	intmax_t m_Symmetry = 0;

	//The number of iterations per pixel of the final output image. Note this is not affected by the increase in pixels in the
	//histogram and DE filtering buffer due to supersampling. It can be affected by a non-zero zoom value though.
	//10 is a good value for interactive/real-time rendering, 100-200 is good for previewing, 1000 is good for final output. Above that is mostly a waste of energy.
	//Xml field: "quality".
	T m_Quality = 100;

	//The number of pixels in the final output image that corresponds to the distance from 0-1 in the cartesian plane used for iterating.
	//A larger value produces a more zoomed in imgage. A value of 240 is commonly used, but in practice it varies widely depending on the image.
	//Note that increasing this value does not adjust the quality by a proportional amount, so an increased value may produce a degraded image.
	//Xml field: "scale".
	T m_PixelsPerUnit = 240;

	//A value greater than 0 will zoom in the field of view, however it will also increase the quality by a proportional amount. This is used to
	//overcome the shortcoming of scale by also adjusting the quality.
	//Xml field: "zoom".
	T m_Zoom = 0;

	//3D fields.
private:
	typedef void (Ember<T>::*ProjFuncPtr)(Point<T>&, QTIsaac<ISAAC_SIZE, ISAAC_INT>&, const CarToRas<T>&);
	ProjFuncPtr m_ProjFunc;

public:
	//Xml field: "cam_zpos".
	T m_CamZPos = 0;

	//Xml field: "cam_persp".
	T m_CamPerspective = 0;

	//Xml field: "cam_yaw".
	T m_CamYaw = 0;

	//Xml field: "cam_pitch".
	T m_CamPitch = 0;

	//Xml field: "cam_dof".
	T m_CamDepthBlur = 0;

	//Xml field: "blur_curve".
	T m_BlurCurve = 0;//Used as p in the equation x^2/4p.

private:
	T m_BlurCoef = 0;

public:
	m3T m_CamMat;

	//The camera offset from the center of the cartesian plane. Since this is the camera offset, the final output image will be moved in the opposite
	//direction of the values specified. There is also a second copy of the Y coordinate needed because m_CenterY will be modified during strips rendering.
	//Xml field: "center".
	T m_CenterX = 0;
	T m_CenterY = 0;
	T m_RotCenterY = 0;

	//Rotate the camera by this many degrees. Since this is a camera rotation, the final output image will be rotated counter-clockwise.
	//Xml field: "rotate".
	T m_Rotate = 0;

	//Determine how bright to make the image during final accumulation.
	//Xml field: "brightness".
	T m_Brightness = 4;

	//Gamma level used in gamma correction during final accumulation.
	//Xml field: "gamma".
	T m_Gamma = 4;

	//Used in color correction during final accumulation.
	//Xml field: "vibrancy".
	T m_Vibrancy = 1;

	//Gamma threshold used in gamma correction during final accumulation.
	//Xml field: "gamma_threshold".
	T m_GammaThresh = static_cast<T>(0.01);

	//Value to control saturation of some pixels in gamma correction during final accumulation.
	//Xml field: "highlight_power".
	T m_HighlightPower = 1;

	//An alternative way to set brightness, ignored when zero.
	//Xml field: "logscale_k2".
	T m_K2 = 0;

	//When animating a file full of many embers, this value is used to specify the time in the animation
	//that this ember should be rendered. They must all be sequential and increase by a default value of 1.
	//Xml field: "time".
	T m_Time = 0;

	//The background color of the image used in final accumulation, ranged 0-1.
	//Xml field: "background".
	Color<T> m_Background;

	//Animation/interpolation.

	//The type of interpolation to use when interpolating between embers for animation.
	//Xml field: "interpolation".
	eInterp m_Interp = eInterp::EMBER_INTERP_SMOOTH;

	//The type of interpolation to use on affine transforms when interpolating embers for animation.
	//Xml field: "interpolation_type" or "interpolation_space (deprecated)".
	eAffineInterp m_AffineInterp = eAffineInterp::AFFINE_INTERP_LOG;

	//The type of interpolation to use for the palette when interpolating embers for animation.
	//Xml field: "palette_interpolation".
	ePaletteInterp m_PaletteInterp = ePaletteInterp::INTERP_HSV;

	//Density Estimation Filter.

	//The minimum radius of the DE filter.
	//Xml field: "estimator_minimum".
	T m_MinRadDE = 0;

	//The maximum radius of the DE filter.
	//Xml field: "estimator_radius".
	T m_MaxRadDE = 9;

	//The shape of the curve that governs how quickly or slowly the filter drops off as it moves away from the center point.
	//Xml field: "estimator_curve".
	T m_CurveDE = static_cast<T>(0.4);

	//Spatial Filter.

	//The radius of the spatial filter used in final accumulation.
	//Xml field: "filter".
	T m_SpatialFilterRadius = static_cast<T>(0.5);

	//The type of spatial filter used in final accumulation:
	//Gaussian, Hermite, Box, Triangle, Bell, Bspline, Lanczos3
	//Lanczos2, Mitchell, Blackman, Catrom, Hamming, Hanning, Quadratic.
	//Xml field: "filter_shape".
	eSpatialFilterType m_SpatialFilterType = eSpatialFilterType::GAUSSIAN_SPATIAL_FILTER;

	//Palette.

	//The method used for retrieving a color from the palette when accumulating to the histogram: step, linear.
	//Xml field: "palette_mode".
	ePaletteMode m_PaletteMode = ePaletteMode::PALETTE_LINEAR;

	//The color palette to use. Can be specified inline as Xml color fields, or as a hex buffer. Can also be specified
	//as an index into the palette file with an optional hue rotation applied. Inserting as a hex buffer is the preferred method.
	//Xml field: "color" or "colors" or "palette" .
	Palette<float> m_Palette;//Final palette that is actually used is a copy of this inside of render, which will be of type bucketT (float).

	//Curves used to adjust the color during final accumulation.
	//Xml field: "curves".
	Curves<float> m_Curves;

	//Strings.

	//The name of this ember.
	//Xml field: "name".
	string m_Name = string("No name");

	//The name of the file that this ember was contained in.
	//Xml field: "".
	string m_ParentFilename = string("No parent");

	//An Xml edit document describing information about the author as well as an edit history of the ember.
	//Xml field: "edit".
	xmlDocPtr m_Edits = nullptr;

	//The 0-based position of this ember in the file it was contained in.
	size_t m_Index = 0;

	//Animation.

	//A decimal number between 0 and 1 which governs the xform interpolation behavior like so:
	//0 (default) : xforms will interpolate all at once.
	//0.5 : the interpolation of each xform is staggered so that when the first xform is half done, the second one starts, and so on.
	//1 : each xform interpolates consecutively with no overlap.
	//Xml field: "stagger".
	double m_Stagger = 0;

	//The number of times the xforms of a keyframe will do a 360 degree rotation before moving on to the next interpolation step.
	//Note that only xforms which have the animate flag set to true will be rotated.
	//Set to 0 to omit rotation before interpolating.
	//Xml field: "rotations".
	double m_Rotations = 1;

	//The number of seconds each rotation takes to complete. Larger values give slower movement.
	//Xml field: "seconds_per_rotation".
	double m_SecondsPerRotation = 3;

	//The direction of loop rotation. Clockwise vs. counter clockwise.
	//Xml field: "rotate_xforms_cw".
	bool m_RotateXformsCw = false;

	//The number of seconds taken to interpolate from one keyframe to the next. Larger values give slower movement.
	//Xml field: "blend_seconds".
	double m_BlendSeconds = 3;

	//The number of times the xforms will do a 360 degree rotation while interpolating from one keyframe to the next.
	//Note that only xforms which have the animate flag set to true will be rotated.
	//Set to 0 to omit rotation while interpolating.
	//Xml field: "rotations_per_blend".
	size_t m_RotationsPerBlend = 0;//Could this ever be double?

	//The direction of rotation during interpolation. Clockwise vs. counter clockwise.
	//Xml field: "blend_rotate_xforms_cw".
	bool m_BlendRotateXformsCw = false;

	//The method used to compute the blending distance for each step between keyframes.
	//Linear: Each step will be a linear interpolation.
	//Log : Steps in the beginning and end of the interpolation between keyframes will be smaller and those in the middle will be larger.
	//This has the effect of slowing down the beginning and end portions, and speeding up the middle.
	//Xml field: "linear_blend".
	bool m_Linear = false;

	//Temporal Filter.

	//Only used if temporal filter type is exp, else unused.
	//Xml field: "temporal_filter_exp".
	T m_TemporalFilterExp = 1;

	//The width of the temporal filter.
	//Xml field: "temporal_filter_width".
	T m_TemporalFilterWidth = 1;

	//The type of the temporal filter: Gaussian, Box or Exp.
	//Xml field: "temporal_filter_type".
	eTemporalFilterType m_TemporalFilterType = eTemporalFilterType::BOX_TEMPORAL_FILTER;

	//The list of motion elements for the top-level flame params
	vector<EmberMotion<T>> m_EmberMotionElements;

private:
	/// <summary>
	/// The type of scaling used when resizing.
	/// </summary>
	eScaleType m_ScaleType = eScaleType::SCALE_NONE;

	//The vector containing all of the xforms.
	//Xml field: "xform".
	vector<Xform<T>> m_Xforms;

	//Optional final xform. Default is empty.
	//Discussed in section 3.2 of the paper, page 6.
	//Xml field: "finalxform".
	Xform<T> m_FinalXform;

	//Single global reference to create variations with.
	shared_ptr<VariationList<T>> m_VariationList = VariationList<T>::Instance();

	/// <summary>
	/// Interpolation function that takes the address of a member variable of type T as a template parameter.
	/// This is an alternative to using macros.
	/// </summary>
	/// <param name="embers">The list of embers to interpolate</param>
	/// <param name="coefs">The list of coefficients to interpolate</param>
	/// <param name="size">The size of the lists, both must match.</param>
	template <T Ember<T>::*m>
	void InterpT(const Ember<T>* embers, const vector<T>& coefs, size_t size)
	{
		this->*m = 0;

		for (size_t k = 0; k < size; k++)
			this->*m += coefs[k] * embers[k].*m;
	}

	/// <summary>
	/// Interpolation function that takes the address of a member variable of any type as a template parameter.
	/// </summary>
	/// <param name="embers">The list of embers to interpolate</param>
	/// <param name="coefs">The list of coefficients to interpolate</param>
	/// <param name="size">The size of the lists, both must match.</param>
	template <typename M, M Ember<T>::*m>
	void InterpX(const Ember<T>* embers, const vector<T>& coefs, size_t size)
	{
		this->*m = M();

		for (size_t k = 0; k < size; k++)
			this->*m += coefs[k] * embers[k].*m;
	}

	/// <summary>
	/// Interpolation function that takes the address of a member variable of type integer as a template parameter.
	/// </summary>
	/// <param name="embers">The list of embers to interpolate</param>
	/// <param name="coefs">The list of coefficients to interpolate</param>
	/// <param name="size">The size of the lists, both must match.</param>
	template <size_t Ember<T>::*m>
	void InterpI(const Ember<T>* embers, const vector<T>& coefs, size_t size)
	{
		T t = 0;

		for (size_t k = 0; k < size; k++)
			t += coefs[k] * embers[k].*m;

		this->*m = static_cast<size_t>(std::rint(t));
	}

	/// <summary>
	/// Interpolation function that takes the address of an xform member variable of type T as a template parameter.
	/// This is an alternative to using macros.
	/// </summary>
	/// <param name="xform">A pointer to a list of xforms to interpolate</param>
	/// <param name="i">The xform index to interpolate</param>
	/// <param name="embers">The list of embers to interpolate</param>
	/// <param name="coefs">The list of coefficients to interpolate</param>
	/// <param name="size">The size of the lists, both must match.</param>
	template <T Xform<T>::*m>
	void InterpXform(Xform<T>* xform, size_t i, const Ember<T>* embers, const vector<T>& coefs, size_t size)
	{
		xform->*m = static_cast<T>(0);

		for (size_t k = 0; k < size; k++)
			xform->*m += coefs[k] * embers[k].GetTotalXform(i)->*m;
	}

public:
	//Index of xform to have non-zero opacity, while all others have zero. This is an interactive rendering parameter and is not saved to Xml. -1 means solo is not used.
	intmax_t m_Solo = -1;

	//Cached copy of the final xform which makes it easy to add and remove the final repeatedly during editing without losing information. Used only with interactive rendering.
	Xform<T> m_CachedFinal;
};

/// <summary>
/// Comparer for sorting embers based on time.
/// </summary>
/// <param name="av">Pointer to the first ember to compare</param>
/// <param name="bv">Pointer to the second ember to compare</param>
/// <returns>True if av's time is less than bv's time, else false.</returns>
template <typename T>
static inline bool CompareEmbers(const Ember<T>& a, const Ember<T>& b)
{
	return a.m_Time < b.m_Time;
}
}
