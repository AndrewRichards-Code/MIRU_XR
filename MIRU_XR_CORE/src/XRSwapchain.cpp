#include "miru_xr_core_common.h"

#if defined(MIRU_D3D12)
#include "d3d12/D3D12_Include.h"
#define XR_USE_GRAPHICS_API_D3D12
#endif
#if defined(MIRU_VULKAN)
#include "vulkan/VK_Include.h"
#define XR_USE_GRAPHICS_API_VULKAN
#endif
#include "openxr/openxr_platform.h"

#include "XRInstance.h"
#include "XRSwapchain.h"
#include "XRSession.h"

using namespace miru;
using namespace xr;

static_assert(sizeof(XrSwapchainImageD3D12KHR) == sizeof(Swapchain::SwapchainImage));
static_assert(sizeof(XrSwapchainImageVulkanKHR) == sizeof(Swapchain::SwapchainImage));

static DXGI_FORMAT ToD3D12ImageFormat(base::Image::Format format);

Swapchain::Swapchain(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	uint32_t formatCount = 0;
	MIRU_XR_FATAL(xrEnumerateSwapchainFormats(m_CI.session->m_Session, 0, &formatCount, nullptr), "ERROR: OPENXR: Failed to enumerate SwapchainFormats.");
	m_Format.resize(formatCount);
	MIRU_XR_FATAL(xrEnumerateSwapchainFormats(m_CI.session->m_Session, formatCount, &formatCount, m_Format.data()), "ERROR: OPENXR: Failed to enumerate SwapchainFormats.");

	bool IsInstanceD3D12 = m_CI.session->m_CI.instance->m_CI.api == base::GraphicsAPI::API::D3D12;
	bool IsInstanceVulkan = m_CI.session->m_CI.instance->m_CI.api == base::GraphicsAPI::API::VULKAN;

	int64_t apiFormat = IsInstanceD3D12 ? ToD3D12ImageFormat(m_CI.format) : static_cast<int64_t>(m_CI.format);

	if (!arc::FindInVector(m_Format, apiFormat))
	{
		MIRU_XR_FATAL(true, "ERROR: OPENXR: Failed to find supplied Format in the enumerated SwapchainFormats for this Session.");
	}

	m_SwapchainCI.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
	m_SwapchainCI.next = nullptr;
	m_SwapchainCI.createFlags = 0;
	m_SwapchainCI.usageFlags = static_cast<XrSwapchainUsageFlags>(m_CI.usage);
	m_SwapchainCI.format = apiFormat;
	m_SwapchainCI.sampleCount = m_CI.sampleCount;
	m_SwapchainCI.width = m_CI.width;
	m_SwapchainCI.height = m_CI.height;
	m_SwapchainCI.faceCount = m_CI.faceCount;
	m_SwapchainCI.arraySize = m_CI.arraySize;
	m_SwapchainCI.mipCount = m_CI.mipCount;
	MIRU_XR_FATAL(xrCreateSwapchain(m_CI.session->m_Session, &m_SwapchainCI, &m_Swapchain), "ERROR: OPENXR: Failed to create Swapchain.");

	MIRU_XR_FATAL(xrEnumerateSwapchainImages(m_Swapchain, 0, &m_ImageCount, nullptr), "ERROR: OPENXR: Failed to enumerate SwapchainImages.");
	XrSwapchainImageBaseHeader* swapchainImages = nullptr;
#if defined(MIRU_D3D12)
	if (IsInstanceD3D12)
	{
		m_SwapchainImages.resize(m_ImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR });
		swapchainImages = (XrSwapchainImageBaseHeader*)m_SwapchainImages.data();
	}
#endif
#if defined(MIRU_VULKAN)
	if (IsInstanceVulkan)
	{
		m_SwapchainImages.resize(m_ImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });
		swapchainImages = (XrSwapchainImageBaseHeader*)m_SwapchainImages.data();
	}
#endif
	MIRU_XR_FATAL(xrEnumerateSwapchainImages(m_Swapchain, m_ImageCount, &m_ImageCount, swapchainImages), "ERROR: OPENXR: Failed to enumerate SwapchainImages.");
}

Swapchain::~Swapchain()
{
	MIRU_XR_FATAL(xrDestroySwapchain(m_Swapchain), "ERROR: OPENXR: Failed to destroy Swapchain.");
}

void Swapchain::Acquire(uint32_t& imageIndex)
{
	m_SwapchianImageAI.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
	m_SwapchianImageAI.next = nullptr;
	MIRU_XR_FATAL(xrAcquireSwapchainImage(m_Swapchain, &m_SwapchianImageAI, &imageIndex), "ERROR: OPENXR: Failed to acquire SwapchainImage.");
}

void Swapchain::Wait(int64_t timeout_ns)
{
	m_SwapchianImageWI.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
	m_SwapchianImageWI.next = nullptr;
	m_SwapchianImageWI.timeout = static_cast<XrDuration>(timeout_ns);
	MIRU_XR_FATAL(xrWaitSwapchainImage(m_Swapchain, &m_SwapchianImageWI), "ERROR: OPENXR: Failed to wait SwapchainImage.");
}

void Swapchain::Release()
{
	m_SwapchianImageRI.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
	m_SwapchianImageRI.next = nullptr;
	MIRU_XR_FATAL(xrReleaseSwapchainImage(m_Swapchain, &m_SwapchianImageRI), "ERROR: OPENXR: Failed to release SwapchainImage.");
}

Swapchain::Image Swapchain::GetImage(uint32_t imageIndex)
{
	return m_SwapchainImages[imageIndex].image;
}

static DXGI_FORMAT ToD3D12ImageFormat(base::Image::Format format)
{
	switch (format)
	{
	case base::Image::Format::UNKNOWN:
	case base::Image::Format::R4G4_UNORM_PACK8:
	case base::Image::Format::R4G4B4A4_UNORM_PACK16:
	case base::Image::Format::B4G4R4A4_UNORM_PACK16:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::R5G6B5_UNORM_PACK16:
	case base::Image::Format::B5G6R5_UNORM_PACK16:
		return DXGI_FORMAT_B5G6R5_UNORM;
	case base::Image::Format::R5G5B5A1_UNORM_PACK16:
	case base::Image::Format::B5G5R5A1_UNORM_PACK16:
	case base::Image::Format::A1R5G5B5_UNORM_PACK16:
		return DXGI_FORMAT_B5G5R5A1_UNORM;
		//R8
	case base::Image::Format::R8_UNORM:
		return DXGI_FORMAT_R8_UNORM;
	case base::Image::Format::R8_SNORM:
		return DXGI_FORMAT_R8_SNORM;
	case base::Image::Format::R8_USCALED:
	case base::Image::Format::R8_SSCALED:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::R8_UINT:
		return DXGI_FORMAT_R8_UINT;
	case base::Image::Format::R8_SINT:
		return DXGI_FORMAT_R8_SINT;
	case base::Image::Format::R8_SRGB:
		return DXGI_FORMAT_UNKNOWN;
		//RG8
	case base::Image::Format::R8G8_UNORM:
		return DXGI_FORMAT_R8G8_UNORM;
	case base::Image::Format::R8G8_SNORM:
		return DXGI_FORMAT_R8G8_SNORM;
	case base::Image::Format::R8G8_USCALED:
	case base::Image::Format::R8G8_SSCALED:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::R8G8_UINT:
		return DXGI_FORMAT_R8G8_UINT;
	case base::Image::Format::R8G8_SINT:
		return DXGI_FORMAT_R8G8_SINT;
	case base::Image::Format::R8G8_SRGB:
		return DXGI_FORMAT_UNKNOWN;
		//RGB8
	case base::Image::Format::R8G8B8_UNORM:
	case base::Image::Format::B8G8R8_UNORM:
	case base::Image::Format::R8G8B8_SNORM:
	case base::Image::Format::B8G8R8_SNORM:
	case base::Image::Format::R8G8B8_USCALED:
	case base::Image::Format::B8G8R8_USCALED:
	case base::Image::Format::R8G8B8_SSCALED:
	case base::Image::Format::B8G8R8_SSCALED:
	case base::Image::Format::R8G8B8_UINT:
	case base::Image::Format::B8G8R8_UINT:
	case base::Image::Format::R8G8B8_SINT:
	case base::Image::Format::B8G8R8_SINT:
	case base::Image::Format::R8G8B8_SRGB:
	case base::Image::Format::B8G8R8_SRGB:
		return DXGI_FORMAT_UNKNOWN;
		//RGBA8
	case base::Image::Format::R8G8B8A8_UNORM:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case base::Image::Format::B8G8R8A8_UNORM:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case base::Image::Format::A8B8G8R8_UNORM_PACK32:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case base::Image::Format::R8G8B8A8_SNORM:
	case base::Image::Format::B8G8R8A8_SNORM:
	case base::Image::Format::A8B8G8R8_SNORM_PACK32:
		return DXGI_FORMAT_R8G8B8A8_SNORM;
	case base::Image::Format::R8G8B8A8_USCALED:
	case base::Image::Format::B8G8R8A8_USCALED:
	case base::Image::Format::A8B8G8R8_USCALED_PACK32:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::R8G8B8A8_SSCALED:
	case base::Image::Format::B8G8R8A8_SSCALED:
	case base::Image::Format::A8B8G8R8_SSCALED_PACK32:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::R8G8B8A8_UINT:
	case base::Image::Format::B8G8R8A8_UINT:
	case base::Image::Format::A8B8G8R8_UINT_PACK32:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case base::Image::Format::R8G8B8A8_SINT:
	case base::Image::Format::B8G8R8A8_SINT:
	case base::Image::Format::A8B8G8R8_SINT_PACK32:
		return DXGI_FORMAT_R8G8B8A8_SINT;
	case base::Image::Format::R8G8B8A8_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case base::Image::Format::B8G8R8A8_SRGB:
		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	case base::Image::Format::A8B8G8R8_SRGB_PACK32:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		//RGB10_A2
	case base::Image::Format::A2R10G10B10_UNORM_PACK32:
	case base::Image::Format::A2B10G10R10_UNORM_PACK32:
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case base::Image::Format::A2R10G10B10_SNORM_PACK32:
	case base::Image::Format::A2B10G10R10_SNORM_PACK32:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::A2R10G10B10_USCALED_PACK32:
	case base::Image::Format::A2B10G10R10_USCALED_PACK32:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::A2R10G10B10_SSCALED_PACK32:
	case base::Image::Format::A2B10G10R10_SSCALED_PACK32:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::A2R10G10B10_UINT_PACK32:
	case base::Image::Format::A2B10G10R10_UINT_PACK32:
		return DXGI_FORMAT_R10G10B10A2_UINT;
	case base::Image::Format::A2R10G10B10_SINT_PACK32:
	case base::Image::Format::A2B10G10R10_SINT_PACK32:
		return DXGI_FORMAT_UNKNOWN;
		//R16
	case base::Image::Format::R16_UNORM:
		return DXGI_FORMAT_R16_UNORM;
	case base::Image::Format::R16_SNORM:
		return DXGI_FORMAT_D16_UNORM;
	case base::Image::Format::R16_USCALED:
	case base::Image::Format::R16_SSCALED:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::R16_UINT:
		return DXGI_FORMAT_R16_UINT;
	case base::Image::Format::R16_SINT:
		return DXGI_FORMAT_R16_SINT;
	case base::Image::Format::R16_SFLOAT:
		return DXGI_FORMAT_R16_FLOAT;
		//RG16
	case base::Image::Format::R16G16_UNORM:
		return DXGI_FORMAT_R16G16_UNORM;
	case base::Image::Format::R16G16_SNORM:
		return DXGI_FORMAT_R16G16_SNORM;
	case base::Image::Format::R16G16_USCALED:
	case base::Image::Format::R16G16_SSCALED:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::R16G16_UINT:
		return DXGI_FORMAT_R16G16_UINT;
	case base::Image::Format::R16G16_SINT:
		return DXGI_FORMAT_R16G16_SINT;
	case base::Image::Format::R16G16_SFLOAT:
		return DXGI_FORMAT_R16G16_FLOAT;
		//RGB16
	case base::Image::Format::R16G16B16_UNORM:
	case base::Image::Format::R16G16B16_SNORM:
	case base::Image::Format::R16G16B16_USCALED:
	case base::Image::Format::R16G16B16_SSCALED:
	case base::Image::Format::R16G16B16_UINT:
	case base::Image::Format::R16G16B16_SINT:
	case base::Image::Format::R16G16B16_SFLOAT:
		return DXGI_FORMAT_UNKNOWN;
		//RGBA16
	case base::Image::Format::R16G16B16A16_UNORM:
		return DXGI_FORMAT_R16G16B16A16_UNORM;
	case base::Image::Format::R16G16B16A16_SNORM:
		return DXGI_FORMAT_R16G16B16A16_SNORM;
	case base::Image::Format::R16G16B16A16_USCALED:
	case base::Image::Format::R16G16B16A16_SSCALED:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::R16G16B16A16_UINT:
		return DXGI_FORMAT_R16G16B16A16_UINT;
	case base::Image::Format::R16G16B16A16_SINT:
		return DXGI_FORMAT_R16G16B16A16_SINT;
	case base::Image::Format::R16G16B16A16_SFLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
		//R32
	case base::Image::Format::R32_UINT:
		return DXGI_FORMAT_R32_UINT;
	case base::Image::Format::R32_SINT:
		return DXGI_FORMAT_R32_SINT;
	case base::Image::Format::R32_SFLOAT:
		return DXGI_FORMAT_R32_FLOAT;
		//RG32
	case base::Image::Format::R32G32_UINT:
		return DXGI_FORMAT_R32G32_UINT;
	case base::Image::Format::R32G32_SINT:
		return DXGI_FORMAT_R32G32_SINT;
	case base::Image::Format::R32G32_SFLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
		//RGB32
	case base::Image::Format::R32G32B32_UINT:
		return DXGI_FORMAT_R32G32B32_UINT;
	case base::Image::Format::R32G32B32_SINT:
		return DXGI_FORMAT_R32G32B32_SINT;
	case base::Image::Format::R32G32B32_SFLOAT:
		return DXGI_FORMAT_R32G32B32_FLOAT;
		//RGBA32
	case base::Image::Format::R32G32B32A32_UINT:
		return DXGI_FORMAT_R32G32B32A32_UINT;
	case base::Image::Format::R32G32B32A32_SINT:
		return DXGI_FORMAT_R32G32B32A32_SINT;
	case base::Image::Format::R32G32B32A32_SFLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		//R64, RG64, RGB64, RGBA64
	case base::Image::Format::R64_UINT:
	case base::Image::Format::R64_SINT:
	case base::Image::Format::R64_SFLOAT:
	case base::Image::Format::R64G64_UINT:
	case base::Image::Format::R64G64_SINT:
	case base::Image::Format::R64G64_SFLOAT:
	case base::Image::Format::R64G64B64_SINT:
	case base::Image::Format::R64G64B64_SFLOAT:
	case base::Image::Format::R64G64B64A64_UINT:
	case base::Image::Format::R64G64B64A64_SINT:
	case base::Image::Format::R64G64B64A64_SFLOAT:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::B10G11R11_UFLOAT_PACK32:
		return DXGI_FORMAT_R11G11B10_FLOAT;
	case base::Image::Format::E5B9G9R9_UFLOAT_PACK32:
		return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
	case base::Image::Format::D16_UNORM:
		return DXGI_FORMAT_D16_UNORM;
	case base::Image::Format::X8_D24_UNORM_PACK32:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	case base::Image::Format::D32_SFLOAT:
		return DXGI_FORMAT_D32_FLOAT;
	case base::Image::Format::S8_UINT:
		return DXGI_FORMAT_R8_UINT;
	case base::Image::Format::D16_UNORM_S8_UINT:
		return DXGI_FORMAT_UNKNOWN;
	case base::Image::Format::D24_UNORM_S8_UINT:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case base::Image::Format::D32_SFLOAT_S8_UINT:
		return DXGI_FORMAT_UNKNOWN;
	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}
