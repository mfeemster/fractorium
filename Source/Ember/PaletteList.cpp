#include "EmberPch.h"
#include "PaletteList.h"
#include "XmlToEmber.h"

namespace EmberNs
{
/// <summary>
/// Empty constructor which initializes the palette map with the default palette file.
/// </summary>
template <typename T>
PaletteList<T>::PaletteList()
{
}

/// <summary>
/// Destructor which saves any modifiable palettes to file, just in case they were modified.
/// </summary>
template <typename T>
PaletteList<T>::~PaletteList()
{
	for (auto& palFile : s_Palettes)
	{
		if (IsModifiable(palFile.first))
			Save(palFile.first);
	}
}

/// <summary>
/// Create a new palette file with the given name and vector of palettes, and save it.
/// </summary>
/// <param name="filename">The full path to the file to add</param>
/// <param name="palettes">The list of palettes which comprise the file</param>
/// <returns>True if the file did not exist, was successfully added and saved, else false.</returns>
template <typename T>
bool PaletteList<T>::AddPaletteFile(const string& filename, const vector<Palette<T>>& palettes)
{
	if (!GetPaletteListByFullPath(filename))
	{
		s_Palettes.insert(make_pair(filename, palettes));
		Save(filename);
		return true;
	}

	return false;
}

/// <summary>
/// Create an new empty palette file with the given name with a single modifiable palette in it.
/// </summary>
/// <param name="filename">The full path to the file to add</param>
/// <param name="palettes">The list of palettes which comprise the file</param>
/// <returns>True if the file did not exist, was successfully added and saved, else false.</returns>
template <typename T>
bool PaletteList<T>::AddEmptyPaletteFile(const string& filename)
{
	if (!GetPaletteListByFullPath(filename))
	{
		const auto item = s_Palettes.insert(make_pair(filename, vector<Palette<T>>()));
		Palette<T> p;
		p.m_Index = 0;
		p.m_Name = "empty-default";
		p.m_Filename = make_shared<string>(filename);
		p.m_SourceColors = map<T, v4T>
		{
			{ T(0), v4T(T(0), T(0), T(0), T(1)) },
			{ T(1), v4T(T(0), T(0), T(0), T(1)) }
		};
		item.first->second.push_back(p);
		Save(filename);
		return true;
	}

	return false;
}

/// <summary>
/// Add a new palette to an existing palette file and save the file.
/// </summary>
/// <param name="filename">The full path to the existing palette file to add</param>
/// <param name="palette">The new palette to add to the file</param>
/// <returns>True if the palette file existed, the palette was added, and the file was successfully saved, else false.</returns>
template <typename T>
bool PaletteList<T>::AddPaletteToFile(const string& filename, const Palette<T>& palette)
{
	if (const auto p = GetPaletteListByFullPathOrFilename(filename))
	{
		p->push_back(palette);
		p->back().m_Filename = make_shared<string>(filename);//Ensure the filename matches because this could have been duplicated from another palette file.
		p->back().m_Index = static_cast<int>(p->size()) - 1;
		Save(filename);
		return true;
	}

	return false;
}

/// <summary>
/// Replace an existing palette in a palette file with a new one and save the file.
/// The match is done based on palette name, so if there are duplicate names in
/// the file, only the first one will be replaced.
/// </summary>
/// <param name="filename">The full path to the existing palette file to replace a palette in</param>
/// <param name="palette">The new palette to use to replace an existing one in the file</param>
/// <returns>True if the palette file existed, the palette was replaced, and the file was successfully saved, else false.</returns>
template <typename T>
bool PaletteList<T>::Replace(const string& filename, const Palette<T>& palette)
{
	if (const auto p = GetPaletteListByFullPathOrFilename(filename))
	{
		for (auto& pal : *p)
		{
			if (pal.m_Name == palette.m_Name)
			{
				const auto index = pal.m_Index;
				pal = palette;
				pal.m_Index = index;
				Save(filename);
				return true;
			}
		}
	}

	return false;
}

/// <summary>
/// Replace an existing palette in a palette file with a new one and save the file.
/// The match is done based on the passed in index.
/// </summary>
/// <param name="filename">The full path to the existing palette file to replace a palette in</param>
/// <param name="palette">The new palette to use to replace an existing one in the file</param>
/// <param name="index">The 0-based index of the palette to replace</param>
/// <returns>True if the palette file existed, the palette was replaced, and the file was successfully saved, else false.</returns>
template <typename T>
bool PaletteList<T>::Replace(const string& filename, const Palette<T>& palette, int index)
{
	if (const auto p = GetPaletteListByFullPathOrFilename(filename))
	{
        if (static_cast<size_t>(index) < p->size())
		{
			(*p)[index] = palette;
			(*p)[index].m_Index = index;
			Save(filename);
			return true;
		}
	}

	return false;
}

/// <summary>
/// Delete an existing palette from a palette file.
/// The match is done based on the passed in index.
/// </summary>
/// <param name="filename">The full path to the existing palette file to delete a palette from</param>
/// <param name="index">The 0-based index of the palette to delete</param>
/// <returns>True if the palette file existed, the palette was deleted, and the file was successfully saved, else false.</returns>
template <typename T>
bool PaletteList<T>::Delete(const string& filename, int index)
{
	int i = 0;

	if (const auto p = GetPaletteListByFullPathOrFilename(filename))
	{
        if (static_cast<size_t>(index) < p->size())
		{
			p->erase(p->begin() + index);

			for (auto& pal : *p)
				pal.m_Index = i++;

			Save(filename);
			return true;
		}
	}

	return false;
}

/// <summary>
/// Read an Xml palette file into memory.
/// This must be called before any palette file usage.
/// </summary>
/// <param name="filename">The full path to the file to read</param>
/// <param name="force">If true, override the initialization state and force a read, else observe the initialization state.</param>
/// <returns>Whether anything was read</returns>
template <typename T>
bool PaletteList<T>::Add(const string& filename, bool force)
{
	bool added = true;
	const auto contains = GetPaletteListByFullPathOrFilename(filename) != nullptr;
	const auto filenameonly = GetFilename(filename);

	if (contains && !force)//Don't allow any palettes with the same name, even if they reside in different paths.
		return false;

	const auto palettes = s_Palettes.insert(make_pair(filename, vector<Palette<T>>()));

	if (force || palettes.second)
	{
		string buf;
		const char* loc = __FUNCTION__;

		if (ReadFile(filename.c_str(), buf))
		{
			const auto lower = ToLower(filename);
			const auto pfilename = shared_ptr<string>(new string(filename));

			if (EndsWith(lower, ".xml"))
			{
				//Subtract 1 to make reading with nullterminate set to true work on linux.
				const auto doc = xmlReadMemory(static_cast<const char*>(buf.data()), static_cast<int>(buf.size()), filename.c_str(), nullptr, XML_PARSE_NONET);

				if (doc)
				{
					auto rootNode = xmlDocGetRootElement(doc);

					if (!Compare(rootNode->name, "palettes"))
					{
						palettes.first->second.clear();
						palettes.first->second.reserve(buf.size() / 2048);//Roughly the size in bytes it takes to store the xml text of palette.
						ParsePalettes(rootNode, pfilename, palettes.first->second);
						xmlFreeDoc(doc);
					}

					if (palettes.first->second.empty())
					{
						added = false;//Reading failed, likely not a valid palette file.
						s_Palettes.erase(filename);
						AddToReport(string(loc) + " : Couldn't parse xml doc");
					}
				}
				else
				{
					added = false;
					s_Palettes.erase(filename);
					AddToReport(string(loc) + " : Couldn't load xml doc");
				}
			}
			else if (EndsWith(lower, ".ugr") || EndsWith(lower, ".gradient") || EndsWith(lower, ".gradients"))
			{
				if (!ParsePalettes(buf, pfilename, palettes.first->second))
				{
					added = false;
					s_Palettes.erase(filename);
					AddToReport(string(loc) + " : Couldn't read palette file " + filename);
				}
			}
		}
		else
		{
			added = false;
			s_Palettes.erase(filename);
			AddToReport(string(loc) + " : Couldn't read palette file " + filename);
		}
	}

	return added;
}

/// <summary>
/// Get the palette at a random index in a random file in the map.
/// Attempt to avoid selecting a palette which is all black.
/// </summary>
/// <returns>A pointer to a random palette in a random file if successful, else nullptr.</returns>
template <typename T>
Palette<T>* PaletteList<T>::GetRandomPalette()
{
	size_t attempts = 0;

	while (attempts < Size() * 10)
	{
		auto p = s_Palettes.begin();
		const auto paletteFileIndex = QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(Size());
		size_t i = 0;

		//Move p forward i elements.
		while (i < paletteFileIndex && p != s_Palettes.end())
		{
			++i;
			++p;
		}

		if (i < Size())
		{
			const size_t paletteIndex = QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedRand(p->second.size());

			if (paletteIndex < p->second.size() && !p->second[paletteIndex].IsEmpty())
				return &p->second[paletteIndex];
		}

		attempts++;
	}

	return Size() ? (&s_Palettes.begin()->second[0]) : nullptr;
}

/// <summary>
/// Get the palette at a specified index in the specified file in the map.
/// </summary>
/// <param name="filename">The filename of the palette to retrieve</param>
/// <param name="i">The index of the palette to read. A value of -1 indicates a random palette.</param>
/// <returns>A pointer to the requested palette if the index was in range, else nullptr.</returns>
template <typename T>
Palette<T>* PaletteList<T>::GetPaletteByFilename(const string& filename, size_t i)
{
	if (const auto palettes = GetPaletteListByFilename(filename))
		if (i < palettes->size())
			return &(*palettes)[i];

	return nullptr;
}

/// <summary>
/// Get the palette at a specified index in the specified file in the map.
/// </summary>
/// <param name="filename">The full path and filename of the palette to retrieve</param>
/// <param name="i">The index of the palette to read. A value of -1 indicates a random palette.</param>
/// <returns>A pointer to the requested palette if the index was in range, else nullptr.</returns>
template <typename T>
Palette<T>* PaletteList<T>::GetPaletteByFullPath(const string& filename, size_t i)
{
	if (const auto palettes = GetPaletteListByFullPath(filename))
		if (i < palettes->size())
			return &(*palettes)[i];

	return nullptr;
}

/// <summary>
/// Get a pointer to a palette with a specified name in the specified full path and filename in the map.
/// </summary>
/// <param name="filename">The filename of the palette to retrieve</param>
/// <param name="name">The name of the palette to retrieve</param>
/// <returns>A pointer to the requested palette if found, else nullptr.</returns>
template <typename T>
Palette<T>* PaletteList<T>::GetPaletteByName(const string& filename, const string& name)
{
	if (const auto palettes = GetPaletteListByFullPathOrFilename(filename))
		for (auto& palette : *palettes)
			if (palette.m_Name == name)
				return &palette;

	return nullptr;
}

/// <summary>
/// Get the palette file with the specified filename in the map.
/// </summary>
/// <param name="filename">The filename of the palette to retrieve</param>
/// <returns>A pointer to the requested palette if found, else nullptr.</returns>
template <typename T>
vector<Palette<T>>* PaletteList<T>::GetPaletteListByFilename(const string& filename)
{
	const auto filenameonly = GetFilename(filename);

	for (auto& palettes : s_Palettes)
		if (GetFilename(palettes.first) == filenameonly)
			return &palettes.second;

	return nullptr;
}

/// <summary>
/// Get the palette file with the specified full path and filename in the map.
/// </summary>
/// <param name="filename">The full path and filename of the palette to retrieve</param>
/// <returns>A pointer to the requested palette if found, else nullptr.</returns>
template <typename T>
vector<Palette<T>>* PaletteList<T>::GetPaletteListByFullPath(const string& filename)
{
	const auto palettes = s_Palettes.find(filename);

	if (palettes != s_Palettes.end() && !palettes->second.empty())
		return &palettes->second;

	return nullptr;
}

/// <summary>
/// Get the palette file with the specified full path and filename in the map.
/// If that does not work, try getting it with the filename alone.
/// </summary>
/// <param name="filename">The full path and filename or just the filename of the palette to retrieve</param>
/// <returns>A pointer to the requested palette if found, else nullptr.</returns>
template <typename T>
vector<Palette<T>>* PaletteList<T>::GetPaletteListByFullPathOrFilename(const string& filename)
{
	auto p = GetPaletteListByFullPath(filename);

	if (!p)
		p = GetPaletteListByFilename(filename);

	return p;
}

/// <summary>
/// Get full path and filename of the pallete with the specified filename
/// </summary>
/// <param name="filename">The filename only of the palette to retrieve</param>
/// <returns>A pointer to the requested palette if found, else nullptr.</returns>
template <typename T>
string PaletteList<T>::GetFullPathFromFilename(const string& filename)
{
	const auto filenameonly = GetFilename(filename);

	for (auto& palettes : s_Palettes)
		if (GetFilename(palettes.first) == filenameonly)
			return palettes.first;

	return "";
}

/// <summary>
/// Get a copy of the palette at a specified index in the specified file in the map
/// with its hue adjusted by the specified amount.
/// </summary>
/// <param name="filename">The filename of the palette to retrieve</param>
/// <param name="i">The index of the palette to read.</param>
/// <param name="hue">The hue adjustment to apply</param>
/// <param name="palette">The palette to store the output</param>
/// <returns>True if successful, else false.</returns>
template <typename T>
bool PaletteList<T>::GetHueAdjustedPalette(const string& filename, size_t i, T hue, Palette<T>& palette)
{
	if (const auto unadjustedPal = GetPaletteByFullPath(filename, i))
	{
		unadjustedPal->MakeHueAdjustedPalette(palette, hue);
		return true;
	}

	return false;
}

/// <summary>
/// Clear the palette list and reset the initialization state.
/// </summary>
template <typename T>
void PaletteList<T>::Clear()
{
	s_Palettes.clear();
}

/// <summary>
/// Get the size of the palettes map.
/// This will be the number of files read.
/// </summary>
/// <returns>The size of the palettes map</returns>
template <typename T>
size_t PaletteList<T>::Size() { return s_Palettes.size(); }

/// <summary>
/// Get the size of specified palette vector in the palettes map.
/// </summary>
/// <param name="index">The index of the palette in the map to retrieve</param>
/// <returns>The size of the palette vector at the specified index in the palettes map</returns>
template <typename T>
size_t PaletteList<T>::Size(size_t index)
{
	size_t i = 0;
	auto p = s_Palettes.begin();

	while (i < index && p != s_Palettes.end())
	{
		++i;
		++p;
	}

	return p->second.size();
}

/// <summary>
/// Get the size of specified palette vector in the palettes map.
/// </summary>
/// <param name="s">The filename of the palette in the map to retrieve</param>
/// <returns>The size of the palette vector at the specified index in the palettes map</returns>
template <typename T>
size_t PaletteList<T>::Size(const string& s)
{
	return s_Palettes[s].size();
}

/// <summary>
/// Get the name of specified palette in the palettes map.
/// </summary>
/// <param name="index">The index of the palette in the map to retrieve</param>
/// <returns>The name of the palette vector at the specified index in the palettes map</returns>
template <typename T>
const string& PaletteList<T>::Name(size_t index)
{
	size_t i = 0;
	auto p = s_Palettes.begin();

	while (i < index && p != s_Palettes.end())
	{
		++i;
		++p;
	}

	return p->first;
}

/// <summary>
/// Determines whether all palettes in the passed in palette file are modifiable,
/// meaning whether the source colors have at least one element in them.
/// </summary>
/// <param name="filename">The full path to the existing palette file to search for a modifiable palette in</param>
/// <returns>True if at all palettes in the file were modifiable, else false.</returns>
template <typename T>
bool PaletteList<T>::IsModifiable(const string& filename)
{
	if (const auto palFile = GetPaletteListByFullPathOrFilename(filename))
		for (auto& pal : *palFile)
			if (pal.m_SourceColors.empty())
				return false;

	return true;
}

/// <summary>
/// Get a const ref to the underlying static palette structure.
/// </summary>
/// <returns>s_Palettes</returns>
template <typename T>
const map<string, vector<Palette<T>>>& PaletteList<T>::Palettes() const
{
	return s_Palettes;
}

/// <summary>
/// Saves an existing file to disk.
/// </summary>
/// <param name="filename">The full path to the existing palette file to save</param>
/// <returns>True if successful, else false.</returns>
template <typename T>
bool PaletteList<T>::Save(const string& filename)
{
	const auto fullpath = GetFullPathFromFilename(filename);

	try
	{
		size_t index = 0;
		ostringstream os;

		if (const auto palFile = GetPaletteListByFullPathOrFilename(filename))
		{
			ofstream f(fullpath);
			os << "<palettes>\n";

			if (f.is_open())
			{
				for (auto& pal : *palFile)
				{
					os << "<palette number=\"" << index++ << "\" name=\"" << pal.m_Name << "\"";

					if (!pal.m_SourceColors.empty())
					{
						os << " source_colors=\"";

						for (auto& sc : pal.m_SourceColors)//Need to clamp these each from 0 to 1. Use our custom clamp funcs.//TODO
							os << Clamp<T>(sc.first, 0, 1) << "," << Clamp<T>(sc.second.r, 0, 1) << "," << Clamp<T>(sc.second.g, 0, 1) << "," << Clamp<T>(sc.second.b, 0, 1) << " ";

						os << "\"";
					}

					os << " data=\"";

					for (int i = 0; i < 32; i++)
					{
						for (int j = 0; j < 8; j++)
						{
							const size_t idx = 8 * i + j;
							os << "00";
							os << hex << setw(2) << setfill('0') << static_cast<int>(std::rint(pal[idx][0] * 255));
							os << hex << setw(2) << setfill('0') << static_cast<int>(std::rint(pal[idx][1] * 255));
							os << hex << setw(2) << setfill('0') << static_cast<int>(std::rint(pal[idx][2] * 255));
						}

						os << "\n";
					}

					os << "\"/>\n";
				}
			}

			os << "</palettes>";
			string s = os.str();
			f.write(s.c_str(), s.size());
			return true;
		}
	}
	catch (const std::exception& e)
	{
		cout << "Error: Writing palette file " << fullpath << " failed: " << e.what() << "\n";
		return false;
	}
	catch (...)
	{
		cout << "Error: Writing palette file " << fullpath << " failed.\n";
		return false;
	}

	return false;
}

/// <summary>
/// Parses an Xml node for all palettes present and stores them in the passed in palette vector.
/// Note that although the Xml color values are expected to be 0-255, they are converted and
/// stored as normalized colors, with values from 0-1.
/// </summary>
/// <param name="node">The parent note of all palettes in the Xml file.</param>
/// <param name="filename">The name of the Xml file.</param>
/// <param name="palettes">The vector to store the parsed palettes associated with this file in.</param>
template <typename T>
void PaletteList<T>::ParsePalettes(xmlNode* node, const shared_ptr<string>& filename, vector<Palette<T>>& palettes)
{
	char* val;
	xmlAttrPtr attr;
	int index = 0;
	Locale lcl;//This is required to properly read commas in the custom palette file. Because foreign locales treat a comma as the decimal point, which causes errors.

	while (node)
	{
		if (node->type == XML_ELEMENT_NODE && !Compare(node->name, "palette"))
		{
			attr = node->properties;
			Palette<T> palette;

			while (attr)
			{
				val = reinterpret_cast<char*>(xmlGetProp(node, attr->name));

				if (!Compare(attr->name, "data"))
				{
					string s1, s;
					size_t tmp, colorCount = 0;
					stringstream ss, temp(val); ss >> std::hex;
					s.reserve(2048);

					while (temp >> s1)
						s += s1;

					auto length = s.size();

					for (size_t strIndex = 0; strIndex < length;)
					{
						strIndex += 2;//Skip past the 00 at the beginning of each RGB.

						for (glm::length_t i = 0; i < 3 && colorCount < palette.Size(); i++)
						{
							const char tmpStr[3] = { s[strIndex++], s[strIndex++], 0 };//Read out and convert the string two characters at a time.
							ss.clear();//Reset and fill the string stream.
							ss.str(tmpStr);
							ss >> tmp;//Do the conversion.
							palette.m_Entries[colorCount][i] = static_cast<T>(tmp) / static_cast<T>(255);//Hex palette is [0..255], convert to [0..1].
						}

						colorCount++;
					}
				}
				else if (!Compare(attr->name, "source_colors"))
				{
					const string s(val);
					const auto vec1 = Split(s, ' ');

					for (auto& v : vec1)
					{
						const auto vec2 = Split(v, ',');

						if (vec2.size() == 4)
						{
							float d1 = Clamp(std::stof(vec2[0]), 0.0f, 1.0f);
							palette.m_SourceColors[d1] = v4F(Clamp(std::stof(vec2[1]), 0.0f, 1.0f),
															 Clamp(std::stof(vec2[2]), 0.0f, 1.0f),
															 Clamp(std::stof(vec2[3]), 0.0f, 1.0f), 1.0f);
						}
					}
				}
				else if (!Compare(attr->name, "name"))
				{
					palette.m_Name = string(val);
				}

				xmlFree(val);
				attr = attr->next;
			}

			palette.m_Index = index++;
			palette.m_Filename = filename;
			palettes.push_back(palette);
		}
		else
		{
			ParsePalettes(node->children, filename, palettes);
		}

		node = node->next;
	}
}

/// <summary>
/// Parses a gradient file for all palettes present and stores them in the passed in palette vector.
/// Note that although the Xml color values are expected to be 0-255, they are converted and
/// stored as normalized colors, with values from 0-1.
/// This format is from Ultra Fractal and Apophysis.
/// </summary>
/// <param name="buf">The data to parse.</param>
/// <param name="filename">The name of the gradient file.</param>
/// <param name="palettes">The vector to store the parsed palettes associated with this file in.</param>
/// <returns>True if at least one palette is read, else false.</returns>
template <typename T>
bool PaletteList<T>::ParsePalettes(const string& buf, const shared_ptr<string>& filename, vector<Palette<T>>& palettes)
{
	int paletteIndex = 0;
	size_t index = 0;
	string line;
	string name;
	bool reading = false;
	bool found = false;
	istringstream iss(buf);
	vector<string> splitVec;
	const char leftBrace = '{';
	const char rightBrace = '}';
	const string titleStr = "title=";
	const string indexStr = "index=";
	const string colorStr = "color=";
	const string titleDelStr = " =\"";
	const string colorDelStr = " =";
	Palette<T> palette;
	Color<T> col;
	palettes.clear();

	while (std::getline(iss, line))
	{
		if (!reading && Contains(line, leftBrace))
		{
			reading = true;
		}
		else if (Contains(line, rightBrace))
		{
			if (found)
				palettes.push_back(palette);

			reading = false;
			found = false;
		}

		if (reading)
		{
			if (Find(line, titleStr))
			{
				splitVec = Split(line, titleDelStr, true);

				if (splitVec.size() > 2)
					name = splitVec[1];
			}
			else if (Find(line, indexStr) && Find(line, colorStr))
			{
				if (!found)
				{
					index = 0;
					found = true;
					palette.Clear();
					palette.m_Index = paletteIndex++;
					palette.m_Name = name;
					palette.m_Filename = filename;
				}

				splitVec = Split(line, colorDelStr, true);

				if (splitVec.size() > 3 && index < 256)
				{
					int val = std::stoi(splitVec[3]);
					col.r = (val & 0xFF) / T(255);//Hex palette is [0..255], convert to [0..1].
					col.g = ((val >> 8) & 0xFF) / T(255);
					col.b = ((val >> 16) & 0xFF) / T(255);
					palette[index] = col;
				}

				index++;
			}
		}
	}

	return !palettes.empty();
}

template EMBER_API class PaletteList<float>;

#ifdef DO_DOUBLE
	template EMBER_API class PaletteList<double>;
#endif
}
