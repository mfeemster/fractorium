#pragma once

#include "EmberCLPch.h"
#include "OpenCLInfo.h"

/// <summary>
/// OpenCLWrapper, Spk, NamedBuffer, NamedImage2D, NamedImage2DGL classes.
/// </summary>

namespace EmberCLns
{
/// <summary>
/// Class to contain all of the things needed to store an OpenCL program.
/// The name of it, the source, the compiled program object and the kernel.
/// </summary>
class EMBERCL_API Spk
{
public:
	string m_Name;
	cl::Program::Sources m_Source;
	cl::Program m_Program;
	cl::Kernel m_Kernel;
};

/// <summary>
/// Class to hold an OpenCL buffer with a name to identify it by.
/// </summary>
class EMBERCL_API NamedBuffer
{
public:
	NamedBuffer() noexcept
	{
	}

	NamedBuffer(const cl::Buffer& buff, const string& name)
	{
		m_Buffer = buff;
		m_Name = name;
	}

	cl::Buffer m_Buffer;
	string m_Name;
};

/// <summary>
/// Class to hold a 2D image with a name to identify it by.
/// </summary>
class EMBERCL_API NamedImage2D
{
public:
	NamedImage2D() noexcept
	{
	}

	NamedImage2D(const cl::Image2D& image, const string& name)
	{
		m_Image = image;
		m_Name = name;
	}

	cl::Image2D m_Image;
	string m_Name;
};

/// <summary>
/// Class to hold a 2D image that is mapped to an OpenGL texture
/// and a name to identify it by.
/// </summary>
class EMBERCL_API NamedImage2DGL
{
public:
	NamedImage2DGL() noexcept
	{
	}

	NamedImage2DGL(const cl::ImageGL& image, const string& name)
	{
		m_Image = image;
		m_Name = name;
	}

	cl::ImageGL m_Image;
	string m_Name;
};

/// <summary>
/// Running kernels in OpenCL can require quite a bit of setup, tear down and
/// general housekeeping. This class helps shield the user from such hassles.
/// Its main utility is in holding collections of programs, buffers and images
/// all identified by names. That way, a user can access them as needed without
/// having to pollute their code.
/// In addition, writing to an existing object by name determines if the object
/// can be overwritten, or if it needs to be deleted and replaced by the new one.
/// This class derives from EmberReport, so the caller is able
/// to retrieve a text dump of error information if any errors occur.
/// </summary>
class EMBERCL_API OpenCLWrapper : public EmberReport
{
public:
	OpenCLWrapper();
	bool Init(size_t platformIndex, size_t deviceIndex, bool shared = false);

	//Programs.
	bool AddProgram(const string& name, const string& program, const string& entryPoint, bool doublePrecision);
	void ClearPrograms();

	//Buffers.
	bool AddBuffer(const string& name, size_t size, cl_mem_flags flags = CL_MEM_READ_WRITE);
	bool AddHostBuffer(const string& name, size_t size, void* data);
	bool AddAndWriteBuffer(const string& name, void* data, size_t size, cl_mem_flags flags = CL_MEM_READ_WRITE);
	bool WriteBuffer(const string& name, void* data, size_t size);
	bool WriteBuffer(size_t bufferIndex, void* data, size_t size);
	bool ReadBuffer(const string& name, void* data, size_t size);
	bool ReadBuffer(size_t bufferIndex, void* data, size_t size);
	int FindBufferIndex(const string& name);
	size_t GetBufferSize(const string& name);
	size_t GetBufferSize(size_t bufferIndex);
	void ClearBuffers();

	//Images.
	bool AddAndWriteImage(const string& name, cl_mem_flags flags, const cl::ImageFormat& format, ::size_t width, ::size_t height, ::size_t row_pitch, void* data = nullptr, bool shared = false, GLuint texName = 0);
	bool WriteImage2D(size_t index, bool shared, size_t width, size_t height, size_t row_pitch, void* data);
	bool ReadImage(const string& name, ::size_t width, ::size_t height, ::size_t row_pitch, bool shared, void* data);
	bool ReadImage(size_t imageIndex, ::size_t width, ::size_t height, ::size_t row_pitch, bool shared, void* data);
	int FindImageIndex(const string& name, bool shared);
	size_t GetImageSize(const string& name, bool shared);
	size_t GetImageSize(size_t imageIndex, bool shared);
	bool CompareImageParams(cl::Image& image, cl_mem_flags flags, const cl::ImageFormat& format, ::size_t width, ::size_t height, ::size_t row_pitch);
	void ClearImages(bool shared);
	bool CreateImage2D(cl::Image2D& image2D, cl_mem_flags flags, cl::ImageFormat format, ::size_t width, ::size_t height, ::size_t row_pitch = 0, void* data = nullptr);
	bool CreateImage2DGL(cl::ImageGL& image2DGL, cl_mem_flags flags, GLenum target, GLint miplevel, GLuint texobj);
	bool EnqueueAcquireGLObjects(const string& name);
	bool EnqueueAcquireGLObjects(cl::ImageGL& image);
	bool EnqueueReleaseGLObjects(const string& name);
	bool EnqueueReleaseGLObjects(cl::ImageGL& image);
#ifdef  OCL_USE_1_2_V
	bool EnqueueAcquireGLObjects(const VECTOR_CLASS<cl::Memory>* memObjects = nullptr);
	bool EnqueueReleaseGLObjects(const VECTOR_CLASS<cl::Memory>* memObjects = nullptr);
#else
	bool EnqueueAcquireGLObjects(const cl::vector<cl::Memory>* memObjects = nullptr);
	bool EnqueueReleaseGLObjects(const cl::vector<cl::Memory>* memObjects = nullptr);
#endif
	bool CreateSampler(cl::Sampler& sampler, cl_bool normalizedCoords, cl_addressing_mode addressingMode, cl_filter_mode filterMode);

	//Arguments.
	bool SetBufferArg(size_t kernelIndex, cl_uint argIndex, const string& name);
	bool SetBufferArg(size_t kernelIndex, cl_uint argIndex, size_t bufferIndex);
	bool SetImageArg(size_t kernelIndex, cl_uint argIndex, bool shared, const string& name);
	bool SetImageArg(size_t kernelIndex, cl_uint argIndex, bool shared, size_t imageIndex);

	/// <summary>
	/// Set an argument in the specified kernel, at the specified argument index.
	/// Must keep this here in the .h because it's templated.
	/// </summary>
	/// <param name="kernelIndex">Index of the kernel whose argument will be set</param>
	/// <param name="argIndex">Index of the argument to set</param>
	/// <param name="arg">The argument value to set</param>
	/// <returns>True if success, else false</returns>
	template <typename T>
	bool SetArg(size_t kernelIndex, cl_uint argIndex, T arg)
	{
		if (m_Init && kernelIndex < m_Programs.size())
		{
			cl_int err = m_Programs[kernelIndex].m_Kernel.setArg(argIndex, arg);
			return m_Info->CheckCL(err, "cl::Kernel::setArg()");
		}

		return false;
	}

	//Kernels.
	int FindKernelIndex(const string& name);
	bool RunKernel(size_t kernelIndex, size_t totalGridWidth, size_t totalGridHeight, size_t totalGridDepth, size_t blockWidth, size_t blockHeight, size_t blockDepth);

	//Accessors.
	bool Ok() const;
	bool Shared() const;
	const cl::Context& Context() const;
	size_t PlatformIndex() const;
	size_t DeviceIndex() const;
	size_t TotalDeviceIndex() const;
	const string& DeviceName() const;
	size_t LocalMemSize() const;
	size_t GlobalMemSize() const;
	size_t MaxAllocSize() const;
	size_t WarpSize();

	//Public virtual functions overridden from base classes.
	void ClearErrorReport() noexcept override;
	string ErrorReportString() override;
	vector<string> ErrorReport() override;

	static void MakeEvenGridDims(size_t blockW, size_t blockH, size_t& gridW, size_t& gridH);

private:
	bool CreateSPK(const string& name, const string& program, const string& entryPoint, Spk& spk, bool doublePrecision);

	bool m_Init = false;
	bool m_Shared = false;
	size_t m_PlatformIndex = 0;
	size_t m_DeviceIndex = 0;
	size_t m_LocalMemSize = 0;
	size_t m_GlobalMemSize = 0;
	size_t m_MaxAllocSize = 0;
	size_t m_WarpSize = 0;
	cl::Platform m_Platform;
	cl::Context m_Context;
	cl::Device m_Device;
	cl::CommandQueue m_Queue;
	shared_ptr<OpenCLInfo> m_Info = OpenCLInfo::Instance();
	std::vector<cl::Device> m_DeviceVec;
	std::vector<Spk> m_Programs;
	std::vector<NamedBuffer> m_Buffers;
	std::vector<NamedImage2D> m_Images;
	std::vector<NamedImage2DGL> m_GLImages;
};
}
