#pragma once

#include "EmberCommonPch.h"

#define PNG_COMMENT_MAX 8
static std::recursive_mutex fileCs;

/// <summary>
/// Write a JPEG file.
/// </summary>
/// <param name="filename">The full path and name of the file</param>
/// <param name="image">Pointer to the image data to write</param>
/// <param name="width">Width of the image in pixels</param>
/// <param name="height">Height of the image in pixels</param>
/// <param name="quality">The quality to use</param>
/// <param name="enableComments">True to embed comments, else false</param>
/// <param name="comments">The comment string to embed</param>
/// <param name="id">Id of the author</param>
/// <param name="url">Url of the author</param>
/// <param name="nick">Nickname of the author</param>
/// <returns>True if success, else false</returns>
static bool WriteJpeg(const char* filename, unsigned char* image, size_t width, size_t height, int quality, bool enableComments, const EmberImageComments& comments, const string& id, const string& url, const string& nick)
{
	bool b = false;
	FILE* file = nullptr;
	errno_t fileResult;

	//Just to be extra safe.
	try
	{
		rlg l(fileCs);
		fileResult = fopen_s(&file, filename, "wb");
	}
    catch (const std::exception& e)
	{
        cout << e.what() << endl;
		return false;
	}

	if (fileResult == 0)
	{
		size_t i = 0;
		jpeg_error_mgr jerr;
		jpeg_compress_struct info;
		string nickString, urlString, idString;
		string bvString, niString, rtString;
		string genomeString, verString;
		//Create the mandatory comment strings.
		ostringstream os;
		os << "genome: " << comments.m_Genome; genomeString = os.str(); os.str("");
		os << "error_rate: " << comments.m_Badvals; bvString = os.str(); os.str("");
		os << "samples: " << comments.m_NumIters; niString = os.str(); os.str("");
		os << "time: " << comments.m_Runtime; rtString = os.str(); os.str("");
		os << "version: " << EmberVersion(); verString = os.str(); os.str("");
		info.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&info);
		jpeg_stdio_dest(&info, file);
		info.in_color_space = JCS_RGB;
		info.input_components = 3;
		info.image_width = static_cast<JDIMENSION>(width);
		info.image_height = static_cast<JDIMENSION>(height);
		jpeg_set_defaults(&info);
#if defined(_WIN32) || defined(__APPLE__)
		jpeg_set_quality(&info, quality, static_cast<boolean>(TRUE));
		jpeg_start_compress(&info, static_cast<boolean>(TRUE));
		//Win32:TRUE is defined in MSVC2013\Windows Kits\8.1\Include\shared\minwindef.h:"#define TRUE                1"
		//cast from int to boolean in External/libjpeg/jmorecfg.h:"typedef enum  { FALSE = 0, TRUE =1  } boolean;"
#else
		jpeg_set_quality(&info, quality, TRUE);
		jpeg_start_compress(&info, TRUE);
#endif

		//Write comments to jpeg.
		if (enableComments)
		{
			string s;
			jpeg_write_marker(&info, JPEG_COM, reinterpret_cast<const unsigned char*>(verString.c_str()), static_cast<uint>(verString.size()));

			if (nick != "")
			{
				os.str("");
				os << "nickname: " << nick;
				s = os.str();
				jpeg_write_marker(&info, JPEG_COM, reinterpret_cast<const unsigned char*>(s.c_str()), static_cast<uint>(s.size()));
			}

			if (url != "")
			{
				os.str("");
				os << "url: " << url;
				s = os.str();
				jpeg_write_marker(&info, JPEG_COM, reinterpret_cast<const unsigned char*>(s.c_str()), static_cast<uint>(s.size()));
			}

			if (id != "")
			{
				os.str("");
				os << "id: " << id;
				s = os.str();
				jpeg_write_marker(&info, JPEG_COM, reinterpret_cast<const unsigned char*>(s.c_str()), static_cast<uint>(s.size()));
			}

			jpeg_write_marker(&info, JPEG_COM, reinterpret_cast<const unsigned char*>(bvString.c_str()), static_cast<uint>(bvString.size()));
			jpeg_write_marker(&info, JPEG_COM, reinterpret_cast<const unsigned char*>(niString.c_str()), static_cast<uint>(niString.size()));
			jpeg_write_marker(&info, JPEG_COM, reinterpret_cast<const unsigned char*>(rtString.c_str()), static_cast<uint>(rtString.size()));
			jpeg_write_marker(&info, JPEG_COM, reinterpret_cast<const unsigned char*>(genomeString.c_str()), static_cast<uint>(genomeString.size()));
		}

		for (i = 0; i < height; i++)
		{
			JSAMPROW row_pointer[1];
			row_pointer[0] = image + (3 * width * i);
			jpeg_write_scanlines(&info, row_pointer, 1);
		}

		jpeg_finish_compress(&info);
		jpeg_destroy_compress(&info);

		if (file != nullptr)
			if (fclose(file))//Non-zero indicates failure.
				return false;

		b = true;
	}

	return b;
}

/// <summary>
/// Write a PNG file.
/// </summary>
/// <param name="filename">The full path and name of the file</param>
/// <param name="image">Pointer to the image data to write</param>
/// <param name="width">Width of the image in pixels</param>
/// <param name="height">Height of the image in pixels</param>
/// <param name="bytesPerChannel">Bytes per channel, 1 or 2.</param>
/// <param name="enableComments">True to embed comments, else false</param>
/// <param name="comments">The comment string to embed</param>
/// <param name="id">Id of the author</param>
/// <param name="url">Url of the author</param>
/// <param name="nick">Nickname of the author</param>
/// <returns>True if success, else false</returns>
static bool WritePng(const char* filename, unsigned char* image, size_t width, size_t height, size_t bytesPerChannel, bool enableComments, const EmberImageComments& comments, const string& id, const string& url, const string& nick)
{
	bool b = false;
	FILE* file = nullptr;
	errno_t fileResult;

	//Just to be extra safe.
	try
	{
		rlg l(fileCs);
		fileResult = fopen_s(&file, filename, "wb");
	}
    catch (const std::exception& e)
	{
        cout << e.what() << endl;
		return false;
	}

	if (fileResult == 0)
	{
		png_structp  png_ptr;
		png_infop    info_ptr;
		size_t i = 0;
		constexpr glm::uint16 testbe = 1;
		vector<unsigned char*> rows(height);

		for (i = 0; i < height; i++)
			rows[i] = image + i * width * 4 * bytesPerChannel;

		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		info_ptr = png_create_info_struct(png_ptr);

		if (setjmp(png_jmpbuf(png_ptr)))
		{
			fclose(file);
			png_destroy_write_struct(&png_ptr, &info_ptr);
			perror("writing file");
			return false;
		}

		png_init_io(png_ptr, file);
		png_set_IHDR(png_ptr, info_ptr, static_cast<png_uint_32>(width), static_cast<png_uint_32>(height), 8 * static_cast<png_uint_32>(bytesPerChannel),
					 PNG_COLOR_TYPE_RGBA,
					 PNG_INTERLACE_NONE,
					 PNG_COMPRESSION_TYPE_BASE,
					 PNG_FILTER_TYPE_BASE);
		//png_set_filter(png_ptr, 0, PNG_NO_FILTERS);

		if (enableComments == 1)
		{
			png_text text[PNG_COMMENT_MAX];
			text[0].compression = PNG_TEXT_COMPRESSION_NONE;
			text[0].key = const_cast<png_charp>("ember_version");
			text[0].text = const_cast<png_charp>(EmberVersion());
			text[1].compression = PNG_TEXT_COMPRESSION_NONE;
			text[1].key = const_cast<png_charp>("ember_nickname");
			text[1].text = const_cast<png_charp>(nick.c_str());
			text[2].compression = PNG_TEXT_COMPRESSION_NONE;
			text[2].key = const_cast<png_charp>("ember_url");
			text[2].text = const_cast<png_charp>(url.c_str());
			text[3].compression = PNG_TEXT_COMPRESSION_NONE;
			text[3].key = const_cast<png_charp>("ember_id");
			text[3].text = const_cast<png_charp>(id.c_str());
			text[4].compression = PNG_TEXT_COMPRESSION_NONE;
			text[4].key = const_cast<png_charp>("ember_error_rate");
			text[4].text = const_cast<png_charp>(comments.m_Badvals.c_str());
			text[5].compression = PNG_TEXT_COMPRESSION_NONE;
			text[5].key = const_cast<png_charp>("ember_samples");
			text[5].text = const_cast<png_charp>(comments.m_NumIters.c_str());
			text[6].compression = PNG_TEXT_COMPRESSION_NONE;
			text[6].key = const_cast<png_charp>("ember_time");
			text[6].text = const_cast<png_charp>(comments.m_Runtime.c_str());
			text[7].compression = PNG_TEXT_COMPRESSION_zTXt;
			text[7].key = const_cast<png_charp>("ember_genome");
			text[7].text = const_cast<png_charp>(comments.m_Genome.c_str());
			png_set_text(png_ptr, info_ptr, text, PNG_COMMENT_MAX);
		}

		png_write_info(png_ptr, info_ptr);

		//Must set this after png_write_info().
		if (bytesPerChannel == 2 && testbe != htons(testbe))
		{
			png_set_swap(png_ptr);
		}

		png_write_image(png_ptr, rows.data());
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);

		if (file != nullptr)
			if (fclose(file))
				return false;

		b = true;
	}

	return b;
}

/// <summary>
/// Convert an RGB buffer to BGR for usage with BMP.
/// </summary>
/// <param name="buffer">The buffer to convert</param>
/// <param name="width">The width.</param>
/// <param name="height">The height.</param>
/// <param name="newSize">The size of the new buffer created</param>
/// <returns>The converted buffer if successful, else NULL.</returns>
static vector<unsigned char> ConvertRGBToBMPBuffer(unsigned char* buffer, size_t width, size_t height, size_t& newSize)
{
	if (buffer == nullptr || width == 0 || height == 0)
		return vector<unsigned char>();

	size_t padding = 0;
	const auto scanlinebytes = width * 3;

	while ((scanlinebytes + padding ) % 4 != 0)
		padding++;

	const auto psw = scanlinebytes + padding;
	newSize = height * psw;
	vector<unsigned char> newBuf(newSize);
	size_t bufpos = 0;
	size_t newpos = 0;

	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < 3 * width; x += 3)
		{
			bufpos = y * 3 * width + x;     // position in original buffer
			newpos = (height - y - 1) * psw + x; // position in padded buffer
			newBuf[newpos] = buffer[bufpos + 2];     // swap r and b
			newBuf[newpos + 1] = buffer[bufpos + 1]; // g stays
			newBuf[newpos + 2] = buffer[bufpos];     // swap b and r
			//No swap.
			//newBuf[newpos] = buffer[bufpos];
			//newBuf[newpos + 1] = buffer[bufpos + 1];
			//newBuf[newpos + 2] = buffer[bufpos + 2];
		}
	}

	return newBuf;
}

/// <summary>
/// Save a Bmp file.
/// </summary>
/// <param name="filename">The full path and name of the file</param>
/// <param name="image">Pointer to the image data to write</param>
/// <param name="width">Width of the image in pixels</param>
/// <param name="height">Height of the image in pixels</param>
/// <param name="paddedSize">Padded size, greater than or equal to total image size.</param>
/// <returns>True if success, else false</returns>
static bool SaveBmp(const char* filename, const unsigned char* image, size_t width, size_t height, size_t paddedSize)
{
#ifdef _WIN32
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER info;
	DWORD bwritten;
	HANDLE file = nullptr;
	memset (&bmfh, 0, sizeof (BITMAPFILEHEADER));
	memset (&info, 0, sizeof (BITMAPINFOHEADER));
	bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + static_cast<DWORD>(paddedSize);
	bmfh.bfOffBits = 0x36;
	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = static_cast<LONG>(width);
	info.biHeight = static_cast<LONG>(height);
	info.biPlanes = 1;
	info.biBitCount = 24;
	info.biCompression = BI_RGB;
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 0x0ec4;
	info.biYPelsPerMeter = 0x0ec4;
	info.biClrUsed = 0;
	info.biClrImportant = 0;

	//Just to be extra safe.
	try
	{
		rlg l(fileCs);

		if ((file = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == NULL)
		{
			if (file != 0)
				CloseHandle(file);

			return false;
		}
	}
    catch (const std::exception& e)
	{
        cout << e.what() << endl;
		CloseHandle(file);
		return false;
	}

	if (WriteFile(file, &bmfh, sizeof (BITMAPFILEHEADER), &bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}

	if (WriteFile(file, &info, sizeof(BITMAPINFOHEADER), &bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}

	if (WriteFile(file, image, static_cast<DWORD>(paddedSize), &bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}

	CloseHandle(file);
#endif
	return true;
}

/// <summary>
/// Convert a buffer from RGB to BGR and write a Bmp file.
/// </summary>
/// <param name="filename">The full path and name of the file</param>
/// <param name="image">Pointer to the image data to write</param>
/// <param name="width">Width of the image in pixels</param>
/// <param name="height">Height of the image in pixels</param>
/// <returns>True if success, else false</returns>
static bool WriteBmp(const char* filename, unsigned char* image, size_t width, size_t height)
{
	bool b = false;
	size_t newSize;
	auto bgrBuf = ConvertRGBToBMPBuffer(image, width, height, newSize);
	b = SaveBmp(filename, bgrBuf.data(), width, height, newSize);
	return b;
}

/// <summary>
/// Write an EXR file which will use the 16 bit half float format for each pixel channel.
/// This is used for high color precision because it uses
/// HALFS for each color channel.
/// </summary>
/// <param name="filename">The full path and name of the file</param>
/// <param name="image">Pointer to the image data to write</param>
/// <param name="width">Width of the image in pixels</param>
/// <param name="height">Height of the image in pixels</param>
/// <param name="enableComments">True to embed comments, else false</param>
/// <param name="comments">The comment string to embed</param>
/// <param name="id">Id of the author</param>
/// <param name="url">Url of the author</param>
/// <param name="nick">Nickname of the author</param>
/// <returns>True if success, else false</returns>
static bool WriteExr16(const char* filename, Rgba* image, size_t width, size_t height, bool enableComments, const EmberImageComments& comments, const string& id, const string& url, const string& nick)
{
	try
	{
		const auto iw = static_cast<int>(width);
		const auto ih = static_cast<int>(height);
		std::unique_ptr<RgbaOutputFile> file;

		try
		{
			rlg l(fileCs);
			file = std::make_unique<RgbaOutputFile>(filename, iw, ih, RgbaChannels::WRITE_RGBA);
		}
        catch (const std::exception& e)
		{
            cout << e.what() << endl;
			return false;
		}

		if (enableComments)
		{
			auto& header = const_cast<Imf::Header&>(file->header());
			header.insert("ember_version", StringAttribute(EmberVersion()));
			header.insert("ember_nickname", StringAttribute(nick));
			header.insert("ember_url", StringAttribute(url));
			header.insert("ember_id", StringAttribute(id));
			header.insert("ember_error_rate", StringAttribute(comments.m_Badvals));
			header.insert("ember_samples", StringAttribute(comments.m_NumIters));
			header.insert("ember_time", StringAttribute(comments.m_Runtime));
			header.insert("ember_genome", StringAttribute(comments.m_Genome));
		}

		file->setFrameBuffer(image, 1, iw);
		file->writePixels(ih);
		return true;
	}
    catch (const std::exception& e)
	{
		cout << e.what() << endl;
		return false;
	}
}

/// <summary>
/// Write an EXR file which will use the 32 bit half float format for each pixel channel.
/// This is used for extreme color precision because it uses
/// floats for each color channel.
/// </summary>
/// <param name="filename">The full path and name of the file</param>
/// <param name="r">Pointer to the red image data to write</param>
/// <param name="g">Pointer to the green image data to write</param>
/// <param name="b">Pointer to the blue image data to write</param>
/// <param name="a">Pointer to the alpha image data to write</param>
/// <param name="width">Width of the image in pixels</param>
/// <param name="height">Height of the image in pixels</param>
/// <param name="enableComments">True to embed comments, else false</param>
/// <param name="comments">The comment string to embed</param>
/// <param name="id">Id of the author</param>
/// <param name="url">Url of the author</param>
/// <param name="nick">Nickname of the author</param>
/// <returns>True if success, else false</returns>
static bool WriteExr32(const char* filename, float* r, float* g, float* b, float* a, size_t width, size_t height, bool enableComments, const EmberImageComments& comments, const string& id, const string& url, const string& nick)
{
	try
	{
		const auto iw = static_cast<int>(width);
		const auto ih = static_cast<int>(height);
		std::unique_ptr<OutputFile> file;

		try
		{
			rlg l(fileCs);
			Header header(iw, ih);
			header.channels().insert("R", Channel(PixelType::FLOAT));
			header.channels().insert("G", Channel(PixelType::FLOAT));
			header.channels().insert("B", Channel(PixelType::FLOAT));
			header.channels().insert("A", Channel(PixelType::FLOAT));
			file = std::make_unique<OutputFile>(filename, header);
		}
        catch (const std::exception& e)
		{
			cout << e.what() << endl;
			return false;
		}

		if (enableComments)
		{
			auto& header = const_cast<Imf::Header&>(file->header());
			header.insert("ember_version", StringAttribute(EmberVersion()));
			header.insert("ember_nickname", StringAttribute(nick));
			header.insert("ember_url", StringAttribute(url));
			header.insert("ember_id", StringAttribute(id));
			header.insert("ember_error_rate", StringAttribute(comments.m_Badvals));
			header.insert("ember_samples", StringAttribute(comments.m_NumIters));
			header.insert("ember_time", StringAttribute(comments.m_Runtime));
			header.insert("ember_genome", StringAttribute(comments.m_Genome));
		}

		FrameBuffer frameBuffer;
		frameBuffer.insert("R",
						   Slice(PixelType::FLOAT,
                                 reinterpret_cast<char*>(r),
								 sizeof(*r) * 1,
								 sizeof(*r) * width));
		frameBuffer.insert("G",
						   Slice(PixelType::FLOAT,
                                 reinterpret_cast<char*>(g),
								 sizeof(*g) * 1,
								 sizeof(*g) * width));
		frameBuffer.insert("B",
						   Slice(PixelType::FLOAT,
                                 reinterpret_cast<char*>(b),
								 sizeof(*b) * 1,
								 sizeof(*b) * width));
		frameBuffer.insert("A",
						   Slice(PixelType::FLOAT,
                                 reinterpret_cast<char*>(a),
								 sizeof(*a) * 1,
								 sizeof(*a) * width));
		file->setFrameBuffer(frameBuffer);
		file->writePixels(ih);
		return true;
	}
    catch (const std::exception& e)
	{
		cout << e.what() << endl;
		return false;
	}
}
