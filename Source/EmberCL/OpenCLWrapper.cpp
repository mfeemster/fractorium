#include "EmberCLPch.h"
#include "OpenCLWrapper.h"

namespace EmberCLns
{
/// <summary>
/// Constructor that sets everything to an uninitialized state.
/// No OpenCL setup is done here other than what's done in the
/// global OpenCLInfo object. The caller must explicitly do it.
/// </summary>
OpenCLWrapper::OpenCLWrapper()
{
	//Pre-allocate some space to avoid temporary copying.
	m_Programs.reserve(4);
	m_Buffers.reserve(4);
	m_Images.reserve(4);
	m_GLImages.reserve(4);
}

/// <summary>
/// Initialize the specified platform and device.
/// This can be shared with OpenGL.
/// </summary>
/// <param name="platform">The index platform of the platform to use</param>
/// <param name="device">The index device of the device to use</param>
/// <param name="shared">True if shared with OpenGL, else false.</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::Init(size_t platformIndex, size_t deviceIndex, bool shared)
{
	cl_int err;
	auto& platforms = m_Info->Platforms();
	auto& devices = m_Info->Devices();
	m_Init = false;
	ClearErrorReport();

	if (m_Info->Ok())
	{
		if (platformIndex < platforms.size() && platformIndex < devices.size())
		{
			cl::Context context;

			if (m_Info->CreateContext(platforms[platformIndex], context, shared))//Platform index is within range, now do context.
			{
				if (deviceIndex < devices[platformIndex].size())//Context is ok, now do device.
				{
					auto q = cl::CommandQueue(context, devices[platformIndex][deviceIndex], 0, &err);//At least one GPU device is present, so create a command queue.

					if (m_Info->CheckCL(err, "cl::CommandQueue()"))//Everything was successful so assign temporaries to members.
					{
						m_Platform = platforms[platformIndex];
						m_Device = devices[platformIndex][deviceIndex];
						m_Context = context;
						m_Queue = q;
						m_PlatformIndex = platformIndex;
						m_DeviceIndex = deviceIndex;
						m_DeviceVec.clear();
						m_DeviceVec.push_back(m_Device);
						m_LocalMemSize = size_t(m_Info->GetInfo<cl_ulong>(m_PlatformIndex, m_DeviceIndex, CL_DEVICE_LOCAL_MEM_SIZE));
						m_GlobalMemSize = size_t(m_Info->GetInfo<cl_ulong>(m_PlatformIndex, m_DeviceIndex, CL_DEVICE_GLOBAL_MEM_SIZE));
						m_MaxAllocSize = size_t(m_Info->GetInfo<cl_ulong>(m_PlatformIndex, m_DeviceIndex, CL_DEVICE_MAX_MEM_ALLOC_SIZE));
						m_Shared = shared;
						m_Init = true;//Command queue is ok, it's now ok to begin building and running programs.
					}
				}
			}
		}
	}

	return m_Init;
}

/// <summary>
/// Compile and add the program, using the specified entry point.
/// If a program with the same name already exists then it will be replaced.
/// </summary>
/// <param name="name">The name of the program</param>
/// <param name="program">The program source</param>
/// <param name="entryPoint">The name of the entry point kernel function in the program</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::AddProgram(const string& name, const string& program, const string& entryPoint, bool doublePrecision)
{
	Spk spk;

	if (CreateSPK(name, program, entryPoint, spk, doublePrecision))
	{
		for (auto& p : m_Programs)
		{
			if (name == p.m_Name)
			{
				p = spk;
				return true;
			}
		}

		//Nothing was found, so add.
		m_Programs.push_back(spk);
		return true;
	}

	return false;
}

/// <summary>
/// Clear the programs.
/// </summary>
void OpenCLWrapper::ClearPrograms()
{
	m_Programs.clear();
}

/// <summary>
/// Add a buffer with the specified size and name.
/// Three possible actions to take:
///		Buffer didn't exist, so create and add.
///		Buffer existed, but was a different size, replace.
///		Buffer existed with the same size, do nothing.
/// </summary>
/// <param name="name">The name of the buffer</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <param name="flags">The buffer flags. Default: CL_MEM_READ_WRITE.</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::AddBuffer(const string& name, size_t size, cl_mem_flags flags)
{
	cl_int err;

	if (m_Init)
	{
		int bufferIndex = FindBufferIndex(name);

		if (bufferIndex == -1)//If the buffer didn't exist, create and add.
		{
			cl::Buffer buff(m_Context, flags, size, nullptr, &err);

			if (!m_Info->CheckCL(err, "cl::Buffer()"))
				return false;

			NamedBuffer nb(buff, name);
			m_Buffers.push_back(nb);
		}
		else if (GetBufferSize(bufferIndex) != size)//If it did exist, only create and add if the sizes were different.
		{
			m_Buffers[bufferIndex] = NamedBuffer(cl::Buffer(m_Context, flags, size_t(1), nullptr, &err), "emptybuffer");//First clear out the original so the two don't exist in memory at once.
			char ch = 0;

			if (!WriteBuffer("emptybuffer", &ch, 1))//Write the dummy buffer at least once because OpenCL seems to do a lazy instantiation of buffers.
				return false;

			cl::Buffer buff(m_Context, flags, size, nullptr, &err);//Create the new buffer.

			if (!m_Info->CheckCL(err, "cl::Buffer()"))
				return false;

			NamedBuffer nb(buff, name);//Make a named buffer out of the new buffer.
			m_Buffers[bufferIndex] = nb;//Finally, assign.
		}

		//If the buffer existed and the sizes were the same, take no action.
		return true;//Either operation succeeded.
	}

	return false;
}

/// <summary>
/// Add a host side buffer with the specified name, size and host data pointer.
/// Three possible actions to take:
///		Buffer didn't exist, so create and add.
///		Buffer existed, but was a different size or pointer, replace.
///		Buffer existed with the same size and pointer, do nothing.
/// </summary>
/// <param name="name">The name of the buffer</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <param name="data">The pointer to the beginning of the host side data.</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::AddHostBuffer(const string& name, size_t size, void* data)
{
	cl_int err;

	if (m_Init)
	{
		int bufferIndex = FindBufferIndex(name);

		if (bufferIndex == -1)//If the buffer didn't exist, create and add.
		{
			cl::Buffer buff(m_Context, CL_MEM_USE_HOST_PTR, size, data, &err);

			if (!m_Info->CheckCL(err, "cl::Buffer()"))
				return false;

			NamedBuffer nb(buff, name);
			m_Buffers.push_back(nb);
		}
		else
		{
			if (GetBufferSize(bufferIndex) != size ||//If it did exist, only create and add if the sizes...
					data != m_Buffers[bufferIndex].m_Buffer.getInfo<CL_MEM_HOST_PTR>(nullptr))//...or addresses were different.
			{
				m_Buffers[bufferIndex] = NamedBuffer(cl::Buffer(m_Context, CL_MEM_USE_HOST_PTR, size_t(1), data, &err), "emptybuffer");//First clear out the original so the two don't exist in memory at once.
				char ch = 0;

				if (!WriteBuffer("emptybuffer", &ch, 1))//Write the dummy buffer at least once because OpenCL seems to do a lazy instantiation of buffers.
					return false;

				cl::Buffer buff(m_Context, CL_MEM_USE_HOST_PTR, size, data, &err);//Create the new buffer.

				if (!m_Info->CheckCL(err, "cl::Buffer()"))
					return false;

				NamedBuffer nb(buff, name);//Make a named buffer out of the new buffer.
				m_Buffers[bufferIndex] = nb;//Finally, assign.
			}
		}

		//If the buffer existed and the sizes and pointers were the same, take no action.
		return true;//Either operation succeeded.
	}

	return false;
}

/// <summary>
/// Add and/or write a buffer of data with the specified name to the list of buffers.
/// Three possible actions to take:
///		Buffer didn't exist, so create and add.
///		Buffer existed, but was a different size. Replace.
///		Buffer existed with the same size, copy data.
/// </summary>
/// <param name="name">The name of the buffer</param>
/// <param name="data">A pointer to the buffer</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <param name="flags">The buffer flags. Default: CL_MEM_READ_WRITE.</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::AddAndWriteBuffer(const string& name, void* data, size_t size, cl_mem_flags flags)
{
	bool b = false;

	if (AddBuffer(name, size, flags))
		b = WriteBuffer(name, data, size);

	return b;
}

/// <summary>
/// Write data to an existing buffer with the specified name.
/// </summary>
/// <param name="name">The name of the buffer</param>
/// <param name="data">A pointer to the buffer</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::WriteBuffer(const string& name, void* data, size_t size)
{
	int bufferIndex = FindBufferIndex(name);
	return bufferIndex != -1 ? WriteBuffer(bufferIndex, data, size) : false;
}

/// <summary>
/// Write data to an existing buffer at the specified index.
/// </summary>
/// <param name="bufferIndex">The index of the buffer</param>
/// <param name="data">A pointer to the buffer</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::WriteBuffer(size_t bufferIndex, void* data, size_t size)
{
	if (m_Init && (bufferIndex < m_Buffers.size()) && (GetBufferSize(bufferIndex) == size))
	{
		cl::Event e;
		const auto err = m_Queue.enqueueWriteBuffer(m_Buffers[bufferIndex].m_Buffer, CL_TRUE, 0, size, data, nullptr, &e);
		e.wait();
		m_Queue.finish();

		if (m_Info->CheckCL(err, "cl::CommandQueue::enqueueWriteBuffer()"))
			return true;
	}

	return false;
}

/// <summary>
/// Read data from an existing buffer with the specified name.
/// </summary>
/// <param name="name">The name of the buffer</param>
/// <param name="data">A pointer to a buffer to copy the data to</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::ReadBuffer(const string& name, void* data, size_t size)
{
	int bufferIndex = FindBufferIndex(name);
	return bufferIndex != -1 ? ReadBuffer(bufferIndex, data, size) : false;
}

/// <summary>
/// Read data from an existing buffer at the specified index.
/// </summary>
/// <param name="bufferIndex">The index of the buffer</param>
/// <param name="data">A pointer to a buffer to copy the data to</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::ReadBuffer(size_t bufferIndex, void* data, size_t size)
{
	if (m_Init && (bufferIndex < m_Buffers.size()) && (GetBufferSize(bufferIndex) == size))
	{
		cl::Event e;
		const auto err = m_Queue.enqueueReadBuffer(m_Buffers[bufferIndex].m_Buffer, CL_TRUE, 0, size, data, nullptr, &e);
		e.wait();
		m_Queue.finish();

		if (m_Info->CheckCL(err, "cl::CommandQueue::enqueueReadBuffer()"))
			return true;
	}

	return false;
}

/// <summary>
/// Find the index of the buffer with the specified name.
/// </summary>
/// <param name="name">The name of the buffer to search for</param>
/// <returns>The index if found, else -1.</returns>
int OpenCLWrapper::FindBufferIndex(const string& name)
{
	for (size_t i = 0; i < m_Buffers.size(); i++)
		if (m_Buffers[i].m_Name == name)
			return int(i);

	return -1;
}

/// <summary>
/// Get the size of the buffer with the specified name.
/// </summary>
/// <param name="name">The name of the buffer to search for</param>
/// <returns>The size of the buffer if found, else 0.</returns>
size_t OpenCLWrapper::GetBufferSize(const string& name)
{
	int bufferIndex = FindBufferIndex(name);
	return bufferIndex != -1 ? GetBufferSize(bufferIndex) : 0;
}

/// <summary>
/// Get the size of the buffer at the specified index.
/// </summary>
/// <param name="name">The index of the buffer to get the size of</param>
/// <returns>The size of the buffer if found, else 0.</returns>
size_t OpenCLWrapper::GetBufferSize(size_t bufferIndex)
{
	if (m_Init && (bufferIndex < m_Buffers.size()))
		return m_Buffers[bufferIndex].m_Buffer.getInfo<CL_MEM_SIZE>(nullptr);

	return 0;
}

/// <summary>
/// Clear all buffers.
/// </summary>
void OpenCLWrapper::ClearBuffers()
{
	m_Buffers.clear();
}

/// <summary>
/// Add and/or write a new 2D image.
/// Three possible actions to take:
///		Image didn't exist, so create and add.
///		Image existed, but was a different size. Replace.
///		Image existed with the same size, copy data.
/// </summary>
/// <param name="name">The name of the image to add/replace</param>
/// <param name="flags">The memory flags</param>
/// <param name="format">The image format</param>
/// <param name="width">The width in pixels of the image</param>
/// <param name="height">The height in pixels of the image</param>
/// <param name="row_pitch">The row pitch (usually zero)</param>
/// <param name="data">The image data. Default: NULL.</param>
/// <param name="shared">True if shared with an OpenGL texture, else false. Default: false.</param>
/// <param name="texName">The texture ID of the shared OpenGL texture if shared. Default: 0.</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::AddAndWriteImage(const string& name, cl_mem_flags flags, const cl::ImageFormat& format, ::size_t width, ::size_t height, ::size_t row_pitch, void* data, bool shared, GLuint texName)
{
	cl_int err;

	if (m_Init)
	{
		int imageIndex = FindImageIndex(name, shared);

		if (imageIndex == -1)//If the image didn't exist, create and add.
		{
			if (shared)
			{
				cl::ImageGL imageGL(m_Context, flags, GL_TEXTURE_2D, 0, texName, &err);
				NamedImage2DGL namedImageGL(imageGL, name);

				if (m_Info->CheckCL(err, "cl::ImageGL()"))
				{
					m_GLImages.push_back(namedImageGL);
					imageIndex = int(m_GLImages.size()) - 1;
				}
				else
					return false;
			}
			else
			{
				NamedImage2D namedImage(cl::Image2D(m_Context, flags, format, width, height, row_pitch, data, &err), name);

				if (m_Info->CheckCL(err, "cl::Image2D()"))
				{
					m_Images.push_back(namedImage);
					imageIndex = int(m_Images.size()) - 1;
				}
				else
					return false;
			}
		}
		else//It did exist, so create new if sizes are different. Write if data is not NULL.
		{
			if (shared)
			{
				cl::ImageGL imageGL = m_GLImages[imageIndex].m_Image;

				if (!CompareImageParams(imageGL, flags, format, width, height, row_pitch))
				{
					NamedImage2DGL namedImageGL(cl::ImageGL(m_Context, flags, GL_TEXTURE_2D, 0, texName, &err), name);//Sizes are different, so create new.

					if (m_Info->CheckCL(err, "cl::ImageGL()"))
						m_GLImages[imageIndex] = namedImageGL;
					else
						return false;
				}
			}
			else
			{
				if (!CompareImageParams(m_Images[imageIndex].m_Image, flags, format, width, height, row_pitch))
				{
					m_Images[imageIndex] = NamedImage2D();//First clear out the original so the two don't exist in memory at once.
					NamedImage2D namedImage(cl::Image2D(m_Context, flags, format, width, height, row_pitch, data, &err), name);

					if (m_Info->CheckCL(err, "cl::Image2D()"))
						m_Images[imageIndex] = namedImage;
					else
						return false;
				}
			}
		}

		if (data)
			return WriteImage2D(imageIndex, shared, width, height, row_pitch, data);
		else//Strange case: images were same dimensions but no data was passed in, so do nothing.
			return true;
	}

	return false;
}

/// <summary>
/// Write data to an existing 2D image at the specified index.
/// </summary>
/// <param name="index">The index of the image</param>
/// <param name="shared">True if shared with an OpenGL texture, else false.</param>
/// <param name="width">The width in pixels of the image</param>
/// <param name="height">The height in pixels of the image</param>
/// <param name="row_pitch">The row pitch (usually zero)</param>
/// <param name="data">The image data</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::WriteImage2D(size_t index, bool shared, ::size_t width, ::size_t height, ::size_t row_pitch, void* data)
{
	if (m_Init)
	{
		cl_int err;
		cl::Event e;
#ifdef  OCL_USE_1_2_V
		cl::size_t<3> origin, region;
#else
		cl::array<cl::size_type, 3> origin, region;
#endif
		origin[0] = 0;
		origin[1] = 0;
		origin[2] = 0;
		region[0] = width;
		region[1] = height;
		region[2] = 1;

		if (shared && index < m_GLImages.size())
		{
			cl::ImageGL imageGL = m_GLImages[index].m_Image;

			if (EnqueueAcquireGLObjects(imageGL))
			{
				err = m_Queue.enqueueWriteImage(imageGL, CL_TRUE, origin, region, row_pitch, 0, data, nullptr, &e);
				e.wait();
				m_Queue.finish();
				bool b = EnqueueReleaseGLObjects(imageGL);
				return m_Info->CheckCL(err, "cl::enqueueWriteImage()") && b;
			}
		}
		else if (!shared && index < m_Images.size())
		{
			err = m_Queue.enqueueWriteImage(m_Images[index].m_Image, CL_TRUE, origin, region, row_pitch, 0, data, nullptr, &e);
			e.wait();
			m_Queue.finish();
			return m_Info->CheckCL(err, "cl::enqueueWriteImage()");
		}
	}

	return false;
}

/// <summary>
/// Read data from an existing 2D image with the specified name.
/// </summary>
/// <param name="name">The name of the image</param>
/// <param name="width">The width in pixels of the image</param>
/// <param name="height">The height in pixels of the image</param>
/// <param name="row_pitch">The row pitch (usually zero)</param>
/// <param name="shared">True if shared with an OpenGL texture, else false.</param>
/// <param name="data">A pointer to a buffer to copy the data to</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::ReadImage(const string& name, ::size_t width, ::size_t height, ::size_t row_pitch, bool shared, void* data)
{
	if (m_Init)
	{
		int imageIndex = FindImageIndex(name, shared);

		if (imageIndex != -1)
			return ReadImage(imageIndex, width, height, row_pitch, shared, data);
	}

	return false;
}

/// <summary>
/// Read data from an existing 2D image at the specified index.
/// </summary>
/// <param name="name">The name of the image</param>
/// <param name="width">The width in pixels of the image</param>
/// <param name="height">The height in pixels of the image</param>
/// <param name="row_pitch">The row pitch (usually zero)</param>
/// <param name="shared">True if shared with an OpenGL texture, else false.</param>
/// <param name="data">A pointer to a buffer to copy the data to</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::ReadImage(size_t imageIndex, ::size_t width, ::size_t height, ::size_t row_pitch, bool shared, void* data)
{
	if (m_Init)
	{
		cl_int err;
		cl::Event e;
#ifdef  OCL_USE_1_2_V
		cl::size_t<3> origin, region;
#else
		cl::array<cl::size_type, 3> origin, region;
#endif
		origin[0] = 0;
		origin[1] = 0;
		origin[2] = 0;
		region[0] = width;
		region[1] = height;
		region[2] = 1;

		if (shared && imageIndex < m_GLImages.size())
		{
			cl::ImageGL imageGL = m_GLImages[imageIndex].m_Image;

			if (EnqueueAcquireGLObjects(imageGL))
			{
				err = m_Queue.enqueueReadImage(m_GLImages[imageIndex].m_Image, true, origin, region, row_pitch, 0, data);
				bool b = EnqueueReleaseGLObjects(m_GLImages[imageIndex].m_Image);
				return m_Info->CheckCL(err, "cl::enqueueReadImage()") && b;
			}
		}
		else if (!shared && imageIndex < m_Images.size())
		{
			err = m_Queue.enqueueReadImage(m_Images[imageIndex].m_Image, true, origin, region, row_pitch, 0, data);
			return m_Info->CheckCL(err, "cl::enqueueReadImage()");
		}
	}

	return false;
}

/// <summary>
/// Find the index of the 2D image with the specified name.
/// </summary>
/// <param name="name">The name of the image to search for</param>
/// <param name="shared">True if shared with an OpenGL texture, else false.</param>
/// <returns>The index if found, else -1.</returns>
int OpenCLWrapper::FindImageIndex(const string& name, bool shared)
{
	if (shared)
	{
		for (size_t i = 0; i < m_GLImages.size(); i++)
			if (m_GLImages[i].m_Name == name)
				return int(i);
	}
	else
	{
		for (size_t i = 0; i < m_Images.size(); i++)
			if (m_Images[i].m_Name == name)
				return int(i);
	}

	return -1;
}

/// <summary>
/// Get the size of the 2D image with the specified name.
/// </summary>
/// <param name="name">The name of the image to search for</param>
/// <param name="shared">True if shared with an OpenGL texture, else false.</param>
/// <returns>The size of the 2D image if found, else 0.</returns>
size_t OpenCLWrapper::GetImageSize(const string& name, bool shared)
{
	int imageIndex = FindImageIndex(name, shared);
	return GetImageSize(imageIndex, shared);
}

/// <summary>
/// Get the size of the 2D image at the specified index.
/// </summary>
/// <param name="imageIndex">Index of the image to search for</param>
/// <param name="shared">True if shared with an OpenGL texture, else false.</param>
/// <returns>The size of the 2D image if found, else 0.</returns>
size_t OpenCLWrapper::GetImageSize(size_t imageIndex, bool shared)
{
	size_t size = 0;

	if (m_Init)
	{
		if (shared && imageIndex < m_GLImages.size())
		{
			vector<cl::Memory> images;
			images.push_back(m_GLImages[imageIndex].m_Image);
			cl::ImageGL image = m_GLImages[imageIndex].m_Image;

			if (EnqueueAcquireGLObjects(&images))
				size = image.getImageInfo<CL_IMAGE_WIDTH>(nullptr) * image.getImageInfo<CL_IMAGE_HEIGHT>(nullptr) * image.getImageInfo<CL_IMAGE_ELEMENT_SIZE>(nullptr);//Should pitch be checked here?

			EnqueueReleaseGLObjects(&images);
		}
		else if (!shared && imageIndex < m_Images.size())
		{
			cl::Image2D image = m_Images[imageIndex].m_Image;
			size = image.getImageInfo<CL_IMAGE_WIDTH>(nullptr) * image.getImageInfo<CL_IMAGE_HEIGHT>(nullptr) * image.getImageInfo<CL_IMAGE_ELEMENT_SIZE>(nullptr);//Should pitch be checked here?
		}
	}

	return size;
}

/// <summary>
/// Compare the passed in image with the specified parameters.
/// </summary>
/// <param name="image">The image to compare</param>
/// <param name="flags">The memory flags to compare (ommitted)</param>
/// <param name="format">The format to compare</param>
/// <param name="width">The width to compare</param>
/// <param name="height">The height to compare</param>
/// <param name="row_pitch">The row_pitch to compare (omitted)</param>
/// <returns>True if all parameters matched, else false.</returns>
bool OpenCLWrapper::CompareImageParams(cl::Image& image, cl_mem_flags flags, const cl::ImageFormat& format, ::size_t width, ::size_t height, ::size_t row_pitch)
{
	cl_image_format tempFormat = image.getImageInfo<CL_IMAGE_FORMAT>(nullptr);
	return (/*image.getImageInfo<CL_MEM_FLAGS>()       == flags  &&*/
			   tempFormat.image_channel_data_type       		== format.image_channel_data_type &&
			   tempFormat.image_channel_order           		== format.image_channel_order &&
			   image.getImageInfo<CL_IMAGE_WIDTH>(nullptr)     == width  &&
			   image.getImageInfo<CL_IMAGE_HEIGHT>(nullptr)    == height/* &&
			image.getImageInfo<CL_IMAGE_ROW_PITCH>() == row_pitch*/);//Pitch will be (width * bytes per pixel) + padding.
}

/// <summary>
/// Clear all images.
/// </summary>
/// <param name="shared">True to clear shared images, else clear regular images.</param>
void OpenCLWrapper::ClearImages(bool shared)
{
	if (shared)
		m_GLImages.clear();
	else
		m_Images.clear();
}

/// <summary>
/// Create a 2D image and store in the image passed in.
/// </summary>
/// <param name="image2D">The 2D image to store the newly created image in</param>
/// <param name="flags">The memory flags to use</param>
/// <param name="format">The format to use</param>
/// <param name="width">The width in pixels of the image</param>
/// <param name="height">The height in pixels of the image</param>
/// <param name="row_pitch">The row pitch (usually zero)</param>
/// <param name="data">The image data. Default: NULL.</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::CreateImage2D(cl::Image2D& image2D, cl_mem_flags flags, cl::ImageFormat format, ::size_t width, ::size_t height, ::size_t row_pitch, void* data)
{
	if (m_Init)
	{
		cl_int err;
		image2D = cl::Image2D(m_Context,
							  flags,
							  format,
							  width,
							  height,
							  row_pitch,
							  data,
							  &err);
		return m_Info->CheckCL(err, "cl::Image2D()");
	}

	return false;
}

/// <summary>
/// Create a 2D image shared with an OpenGL texture and store in the image passed in.
/// </summary>
/// <param name="image2DGL">The 2D image to store the newly created image in</param>
/// <param name="flags">The memory flags to use</param>
/// <param name="target">The target</param>
/// <param name="miplevel">The mip map level</param>
/// <param name="texobj">The texture ID of the shared OpenGL texture</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::CreateImage2DGL(cl::ImageGL& image2DGL, cl_mem_flags flags, GLenum target, GLint miplevel, GLuint texobj)
{
	if (m_Init)
	{
		cl_int err;
		image2DGL = cl::ImageGL(m_Context,
								flags,
								target,
								miplevel,
								texobj,
								&err);
		return m_Info->CheckCL(err, "cl::ImageGL()");
	}

	return false;
}

/// <summary>
/// Acquire the shared 2D image with the specified name.
/// </summary>
/// <param name="name">The name of the image to acquire</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::EnqueueAcquireGLObjects(const string& name)
{
	int index = FindImageIndex(name, true);

	if (index != -1)
		return EnqueueAcquireGLObjects(m_GLImages[index].m_Image);

	return false;
}

/// <summary>
/// Acquire the shared 2D image.
/// </summary>
/// <param name="image">The image to acquire</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::EnqueueAcquireGLObjects(cl::ImageGL& image)
{
	if (m_Init && m_Shared)
	{
		vector<cl::Memory> images;
		images.push_back(image);
		const auto err = m_Queue.enqueueAcquireGLObjects(&images);
		m_Queue.finish();
		return m_Info->CheckCL(err, "cl::CommandQueue::enqueueAcquireGLObjects()");
	}

	return false;
}

/// <summary>
/// Reelease the shared 2D image with the specified name.
/// </summary>
/// <param name="name">The name of the image to release</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::EnqueueReleaseGLObjects(const string& name)
{
	int index = FindImageIndex(name, true);

	if (index != -1)
		return EnqueueReleaseGLObjects(m_GLImages[index].m_Image);

	return false;
}

/// <summary>
/// Release the shared 2D image.
/// </summary>
/// <param name="image">The image to release</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::EnqueueReleaseGLObjects(cl::ImageGL& image)
{
	if (m_Init && m_Shared)
	{
		vector<cl::Memory> images;
		images.push_back(image);
		const auto err = m_Queue.enqueueReleaseGLObjects(&images);
		m_Queue.finish();
		return m_Info->CheckCL(err, "cl::CommandQueue::enqueueReleaseGLObjects()");
	}

	return false;
}

/// <summary>
/// Acquire a vector of shared OpenGL memory objects.
/// </summary>
/// <param name="memObjects">The memory objects to acquire</param>
/// <returns>True if success, else false.</returns>
#ifdef  OCL_USE_1_2_V
	bool OpenCLWrapper::EnqueueAcquireGLObjects(const VECTOR_CLASS<cl::Memory>* memObjects)
#else
	bool OpenCLWrapper::EnqueueAcquireGLObjects(const cl::vector<cl::Memory>* memObjects)
#endif
{
	if (m_Init && m_Shared)
	{
		const auto err = m_Queue.enqueueAcquireGLObjects(memObjects);
		m_Queue.finish();
		return m_Info->CheckCL(err, "cl::CommandQueue::enqueueAcquireGLObjects()");
	}

	return false;
}

/// <summary>
/// Release a vector of shared OpenGL memory objects.
/// </summary>
/// <param name="memObjects">The memory objects to release</param>
/// <returns>True if success, else false.</returns>
#ifdef  OCL_USE_1_2_V
	bool OpenCLWrapper::EnqueueReleaseGLObjects(const VECTOR_CLASS<cl::Memory>* memObjects)
#else
	bool OpenCLWrapper::EnqueueReleaseGLObjects(const cl::vector<cl::Memory>* memObjects)
#endif
{
	if (m_Init && m_Shared)
	{
		const auto err = m_Queue.enqueueReleaseGLObjects(memObjects);
		m_Queue.finish();
		return m_Info->CheckCL(err, "cl::CommandQueue::enqueueReleaseGLObjects()");
	}

	return false;
}

/// <summary>
/// Create a texture sampler.
/// </summary>
/// <param name="sampler">The sampler to store the newly created sampler in</param>
/// <param name="normalizedCoords">True to use normalized coordinates, else don't.</param>
/// <param name="addressingMode">The addressing mode to use</param>
/// <param name="filterMode">The filter mode to use</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::CreateSampler(cl::Sampler& sampler, cl_bool normalizedCoords, cl_addressing_mode addressingMode, cl_filter_mode filterMode)
{
	cl_int err;
	sampler = cl::Sampler(m_Context,
						  normalizedCoords,
						  addressingMode,
						  filterMode,
						  &err);
	return m_Info->CheckCL(err, "cl::Sampler()");
}

/// <summary>
/// Set the argument at the specified index for the kernel at the specified index to be
/// the buffer with the specified name.
/// </summary>
/// <param name="kernelIndex">Index of the kernel</param>
/// <param name="argIndex">Index of the argument</param>
/// <param name="name">The name of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::SetBufferArg(size_t kernelIndex, cl_uint argIndex, const string& name)
{
	int bufferIndex = OpenCLWrapper::FindBufferIndex(name);
	return bufferIndex != -1 ? SetBufferArg(kernelIndex, argIndex, bufferIndex) : false;
}

/// <summary>
/// Set the argument at the specified index for the kernel at the specified index to be
/// the buffer at the specified index.
/// </summary>
/// <param name="kernelIndex">Index of the kernel</param>
/// <param name="argIndex">Index of the argument</param>
/// <param name="bufferIndex">Index of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::SetBufferArg(size_t kernelIndex, cl_uint argIndex, size_t bufferIndex)
{
	if (m_Init && bufferIndex < m_Buffers.size())
		return SetArg<cl::Buffer>(kernelIndex, argIndex, m_Buffers[bufferIndex].m_Buffer);

	return false;
}

/// <summary>
/// Set the argument at the specified index for the kernel at the specified index to be
/// the 2D image with the specified name.
/// </summary>
/// <param name="kernelIndex">Index of the kernel</param>
/// <param name="argIndex">Index of the argument</param>
/// <param name="shared">True if shared with an OpenGL texture, else false</param>
/// <param name="name">The name of the 2D image</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::SetImageArg(size_t kernelIndex, cl_uint argIndex, bool shared, const string& name)
{
	if (m_Init)
	{
		int imageIndex = FindImageIndex(name, shared);
		return SetImageArg(kernelIndex, argIndex, shared, imageIndex);
	}

	return false;
}

/// <summary>
/// Set the argument at the specified index for the kernel at the specified index to be
/// the 2D image at the specified index.
/// </summary>
/// <param name="kernelIndex">Index of the kernel</param>
/// <param name="argIndex">Index of the argument</param>
/// <param name="shared">True if shared with an OpenGL texture, else false</param>
/// <param name="imageIndex">Index of the 2D image</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::SetImageArg(size_t kernelIndex, cl_uint argIndex, bool shared, size_t imageIndex)
{
	cl_int err;

	if (m_Init)
	{
		if (shared && imageIndex < m_GLImages.size())
		{
			err = m_Programs[kernelIndex].m_Kernel.setArg(argIndex, m_GLImages[imageIndex].m_Image);
			return m_Info->CheckCL(err, "cl::Kernel::setArg()");
		}
		else if (!shared && imageIndex < m_Images.size())
		{
			err = m_Programs[kernelIndex].m_Kernel.setArg(argIndex, m_Images[imageIndex].m_Image);
			return m_Info->CheckCL(err, "cl::Kernel::setArg()");
		}
	}

	return false;
}

/// <summary>
/// Find the index of the kernel with the specified name.
/// </summary>
/// <param name="name">The name of the kernel to search for</param>
/// <returns>The index if found, else -1.</returns>
int OpenCLWrapper::FindKernelIndex(const string& name)
{
	for (size_t i = 0; i < m_Programs.size(); i++)
		if (m_Programs[i].m_Name == name)
			return int(i);

	return -1;
}

/// <summary>
/// Run the kernel at the specified index, using the specified grid and block dimensions.
/// </summary>
/// <param name="kernelIndex">Index of the kernel to run</param>
/// <param name="totalGridWidth">Total width of the grid</param>
/// <param name="totalGridHeight">Total height of the grid</param>
/// <param name="totalGridDepth">The total depth grid</param>
/// <param name="blockWidth">Width of each block</param>
/// <param name="blockHeight">Height of each block</param>
/// <param name="blockDepth">Depth of each block</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::RunKernel(size_t kernelIndex, size_t totalGridWidth, size_t totalGridHeight, size_t totalGridDepth,
							  size_t blockWidth, size_t blockHeight, size_t blockDepth)
{
	if (m_Init && kernelIndex < m_Programs.size())
	{
		cl::Event e;
		const auto err = m_Queue.enqueueNDRangeKernel(m_Programs[kernelIndex].m_Kernel,
						 cl::NullRange,
						 cl::NDRange(totalGridWidth, totalGridHeight, totalGridDepth),
						 cl::NDRange(blockWidth, blockHeight, blockDepth),
						 nullptr,
						 &e);
		e.wait();
		m_Queue.finish();
		return m_Info->CheckCL(err, "cl::CommandQueue::enqueueNDRangeKernel()");
	}

	return false;
}

/// <summary>
/// OpenCL properties, getters only.
/// </summary>
bool OpenCLWrapper::Ok() const { return m_Init; }
bool OpenCLWrapper::Shared() const { return m_Shared; }
const cl::Context& OpenCLWrapper::Context() const { return m_Context; }
size_t OpenCLWrapper::PlatformIndex() const { return m_PlatformIndex; }
size_t OpenCLWrapper::DeviceIndex() const { return m_DeviceIndex; }
size_t OpenCLWrapper::TotalDeviceIndex() const { return m_Info->TotalDeviceIndex(m_PlatformIndex, m_DeviceIndex); }
const string& OpenCLWrapper::DeviceName() const { return m_Info->DeviceName(m_PlatformIndex, m_DeviceIndex); }
size_t OpenCLWrapper::LocalMemSize() const { return m_LocalMemSize; }
size_t OpenCLWrapper::GlobalMemSize() const { return m_GlobalMemSize; }
size_t OpenCLWrapper::MaxAllocSize() const { return m_MaxAllocSize; }
size_t OpenCLWrapper::WarpSize()
{
	if (m_WarpSize == 0)
	{
		try
		{
			auto width = m_Info->GetInfo<cl_uint>(m_PlatformIndex, m_DeviceIndex, CL_DEVICE_WAVEFRONT_WIDTH_AMD);
		
			if (width != 0)
			{
				m_WarpSize = static_cast<size_t>(width);
				return m_WarpSize;
			}

			width = m_Info->GetInfo<cl_uint>(m_PlatformIndex, m_DeviceIndex, CL_DEVICE_WARP_SIZE_NV);

			if (width != 0)
			{
				m_WarpSize = static_cast<size_t>(width);
				return m_WarpSize;
			}
		}
		catch (const std::exception& e)
		{
			AddToReport("Exception in OpenCLWrapper::WarpSize(): " + string(e.what()));
		}
	}

	return m_WarpSize;
}

/// <summary>
/// Clear the error report for this class as well as the global OpenCLInfo instance.
/// </summary>
void OpenCLWrapper::ClearErrorReport() noexcept
{
	EmberReport::ClearErrorReport();
	m_Info->ClearErrorReport();
}

/// <summary>
/// Concatenate and return the error report for this class and the
/// global OpenCLInfo instance as a single string.
/// </summary>
/// <returns>The concatenated error report string</returns>
string OpenCLWrapper::ErrorReportString()
{
	auto s = EmberReport::ErrorReportString();
	return s + m_Info->ErrorReportString();
}

/// <summary>
/// Concatenate and return the error report for this class and the
/// global OpenCLInfo instance as a vector of strings.
/// </summary>
/// <returns>The concatenated error report vector of strings</returns>
vector<string> OpenCLWrapper::ErrorReport()
{
	auto ours = EmberReport::ErrorReport();
	auto s = m_Info->ErrorReport();
	ours.insert(ours.end(), s.begin(), s.end());
	return ours;
}

/// <summary>
/// Make even grid dimensions.
/// The size of the blocks in terms of threads must divide evenly into the total number of threads in the grid.
/// In the case of a remainder, expand the width and height of the grid to the next highest evenly divisible value.
/// Ex:
///		blockW = 5, blockH = 5
///		gridW = 18, gridH = 27
///
/// To make these even:
///		gridW = 20, gridH = 30
/// </summary>
/// <param name="blockW">The width of each block in terms of threads.</param>
/// <param name="blockH">The height of each block in terms of threads.</param>
/// <param name="gridW">The width of the entire grid in terms of threads.</param>
/// <param name="gridH">The width of the entire grid in terms of threads.</param>
void OpenCLWrapper::MakeEvenGridDims(size_t blockW, size_t blockH, size_t& gridW, size_t& gridH)
{
	if (gridW % blockW != 0)
		gridW += (blockW - (gridW % blockW));

	if (gridH % blockH != 0)
		gridH += (blockH - (gridH % blockH));
}

/// <summary>
/// Create an Spk object created by compiling the program arguments passed in.
/// </summary>
/// <param name="name">The name of the program</param>
/// <param name="program">The source of the program</param>
/// <param name="entryPoint">The name of the entry point kernel function in the program</param>
/// <param name="spk">The Spk object to store the resulting compiled program in</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::CreateSPK(const string& name, const string& program, const string& entryPoint, Spk& spk, bool doublePrecision)
{
	if (m_Init)
	{
		cl_int err;
#ifndef OCL_USE_1_2_V
		vector<std::string> programvec { program };
#endif
		spk.m_Name = name;
#ifdef OCL_USE_1_2_V
		spk.m_Source = cl::Program::Sources(1, std::make_pair(program.c_str(), program.length() + 1));
#else
		spk.m_Source = cl::Program::Sources(programvec);
#endif
		spk.m_Program = cl::Program(m_Context, spk.m_Source);

		if (doublePrecision)
			err = spk.m_Program.build(m_DeviceVec, "-cl-mad-enable -cl-no-signed-zeros -cl-denorms-are-zero");//Tinker with other options later.
		else
			err = spk.m_Program.build(m_DeviceVec, "-cl-mad-enable -cl-no-signed-zeros -cl-denorms-are-zero -cl-single-precision-constant");

		//err = spk.m_Program.build(m_DeviceVec, "-cl-single-precision-constant");
		//err = spk.m_Program.build(m_DeviceVec, "-cl-mad-enable -cl-single-precision-constant");
		//err = spk.m_Program.build(m_DeviceVec, "-cl-mad-enable -cl-no-signed-zeros -cl-fast-relaxed-math -cl-single-precision-constant");//This can cause some rounding.
		//err = spk.m_Program.build(m_DeviceVec, "-cl-mad-enable -cl-no-signed-zeros -cl-single-precision-constant -cl-denorms-are-zero");
		//err = spk.m_Program.build(m_DeviceVec, "-cl-mad-enable -cl-single-precision-constant");

		if (m_Info->CheckCL(err, "cl::Program::build()"))
		{
			//Building of program is ok, now create kernel with the specified entry point.
			spk.m_Kernel = cl::Kernel(spk.m_Program, entryPoint.c_str(), &err);

			if (m_Info->CheckCL(err, "cl::Kernel()"))
				return true;//Everything is ok.
		}
		else
		{
			for (auto& i : m_DeviceVec)
				AddToReport(spk.m_Program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(i, nullptr));
		}
	}

	return false;
}
}
