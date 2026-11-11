#include "EmberCLPch.h"
#include "RendererClDevice.h"

namespace EmberCLns
{
/// <summary>
/// Constructor that assigns members.
/// The object is not fully initialized at this point, the caller
/// must manually call Init().
/// </summary>
/// <param name="platform">The index of the platform to use</param>
/// <param name="device">The index device of the device to use</param>
/// <param name="shared">True if shared with OpenGL, else false.</param>
/// <returns>True if success, else false.</returns>
RendererClDevice::RendererClDevice(size_t platform, size_t device, bool shared)
{
	m_Init = false;
	m_Shared = shared;
	m_NVidia = false;
	m_WarpSize = 0;
	m_Calls = 0;
	m_PlatformIndex = platform;
	m_DeviceIndex = device;
	m_Info = OpenCLInfo::Instance();
}

/// <summary>
/// Initialization of the OpenCLWrapper member.
/// </summary>
/// <returns>True if success, else false.</returns>
bool RendererClDevice::Init()
{
	bool b = true;

	if (!m_Wrapper.Ok())
	{
		m_Init = false;
		b = m_Wrapper.Init(m_PlatformIndex, m_DeviceIndex, m_Shared);
	}

	if (b && m_Wrapper.Ok() && !m_Init)
	{
		m_NVidia = Find(ToLower(m_Info->PlatformName(m_PlatformIndex)), "nvidia") && m_Wrapper.LocalMemSize() > (32 * 1024);
		m_WarpSize = m_Wrapper.WarpSize();

		if (m_WarpSize == 0)
			m_WarpSize = m_NVidia ? 32 : 64;//If the warp size is not available, then assume 32 for nVidia and 64 for AMD.

		m_Init = true;
	}

	return b;
}

/// <summary>
/// OpenCL property accessors, getters only.
/// </summary>
bool RendererClDevice::Ok() const noexcept { return m_Init; }
bool RendererClDevice::Shared() const noexcept { return m_Shared; }
bool RendererClDevice::Nvidia() const noexcept { return m_NVidia; }
size_t RendererClDevice::WarpSize() const noexcept { return m_WarpSize; }
size_t RendererClDevice::PlatformIndex() const noexcept { return m_PlatformIndex; }
size_t RendererClDevice::DeviceIndex() const noexcept { return m_DeviceIndex; }

/// <summary>
/// Clear the error report for this class as well as the wrapper.
/// </summary>
void RendererClDevice::ClearErrorReport() noexcept
{
	EmberReport::ClearErrorReport();
	m_Wrapper.ClearErrorReport();
}

/// <summary>
/// Concatenate and return the error report for this class and the
/// wrapper as a single string.
/// </summary>
/// <returns>The concatenated error report string</returns>
string RendererClDevice::ErrorReportString()
{
	const auto s = EmberReport::ErrorReportString();
	return s + m_Wrapper.ErrorReportString();
}

/// <summary>
/// Concatenate and return the error report for this class and the
/// wrapper as a vector of strings.
/// </summary>
/// <returns>The concatenated error report vector of strings</returns>
vector<string> RendererClDevice::ErrorReport()
{
	auto ours = EmberReport::ErrorReport();
	const auto s = m_Wrapper.ErrorReport();
	ours.insert(ours.end(), s.begin(), s.end());
	return ours;
}
}
