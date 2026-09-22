#pragma once

#include "Utils.h"
#include "PaletteList.h"
#include "VariationList.h"
#include "Ember.h"
#include "Spline.h"

#ifdef __APPLE__
	#include <libgen.h>
#endif

/// <summary>
/// XmlToEmber and Locale classes.
/// </summary>

namespace EmberNs
{
/// <summary>
/// Convenience class for setting and resetting the locale.
/// It's set up in the constructor and restored in the destructor.
/// This relieves the caller of having to manually do it everywhere.
/// </summary>
class EMBER_API Locale
{
public:
	Locale(int category = LC_NUMERIC, const char* loc = "C");
	~Locale();

private:
	int m_Category;
	string m_NewLocale;
	string m_OriginalLocale;
};

/// <summary>
/// Class for reading standard Xml flame files as well as Chaotica .chaos files into ember objects.
/// This class derives from EmberReport, so the caller is able
/// to retrieve a text dump of error information if any errors occur.
/// Since this class contains a VariationList object, it's important to declare one
/// instance and reuse it for the duration of the program instead of creating and deleting
/// them as local variables.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API XmlToEmber : public EmberReport
{
public:
	XmlToEmber();
	template <typename Alloc, template <typename, typename> class C>
	bool Parse(byte* buf, const char* filename, C<Ember<T>, Alloc>& embers, bool useDefaults);
	template <typename Alloc, template <typename, typename> class C>
	bool Parse(const char* filename, C<Ember<T>, Alloc>& embers, bool useDefaults);
	template <typename valT>
	bool Aton(const char* str, valT& val);
	static vector<string> m_FlattenNames;

private:
	template <typename Alloc, template <typename, typename> class C>
	void ScanForEmberNodes(xmlNode* curNode, const char* parentFile, C<Ember<T>, Alloc>& embers, bool useDefaults);
	template <typename Alloc, template <typename, typename> class C>
	void ScanForChaosNodes(xmlNode* curNode, const char* parentFile, C<Ember<T>, Alloc>& embers, bool useDefaults);
	bool ParseEmberElement(xmlNode* emberNode, Ember<T>& currentEmber);
	bool ParseEmberElementFromChaos(xmlNode* emberNode, Ember<T>& currentEmber);
	bool AttToEmberMotionFloat(xmlAttrPtr att, const char* attStr, eEmberMotionParam param, EmberMotion<T>& motion);
	bool ParseXform(xmlNode* childNode, Xform<T>& xform, bool motion, bool fromEmber);
	static string GetCorrectedParamName(const unordered_map<string, string>& names, const char* name);
	static string GetCorrectedVariationName(vector<pair<pair<string, string>, vector<string>>>& vec, xmlAttrPtr att);
	static string GetCorrectedVariationName(vector<pair<pair<string, string>, vector<string>>>& vec, const string& varname);
	static bool XmlContainsTag(xmlAttrPtr att, const char* name);
	bool ParseHexColors(const char* colstr, Ember<T>& ember, size_t numColors, intmax_t chan);
	template <typename valT>
    bool ParseAndAssign(const xmlChar* name, const char* attStr, const char* str, valT& val);
	template <typename valT>
	bool ParseAndAssignContent(xmlNode* node, const char* fieldname, const char* fieldnameval, valT& val);
	bool ParseAndAssignContent(xmlNode* node, const char* fieldname, const char* fieldnameval, std::string& val);

	static bool m_Init;
	static unordered_map<string, string> m_BadParamNames;
	static vector<pair<pair<string, string>, vector<string>>> m_BadVariationNames;
	shared_ptr<VariationList<T>> m_VariationList;//The variation list used to make copies of variations to populate the embers with.
	shared_ptr<PaletteList<float>> m_PaletteList;
};
}
