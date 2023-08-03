#include "miru_xr_core_common.h"
#include "XRInstance.h"
#include "XRSwapchain.h"
#include "XRSession.h"

using namespace miru;
using namespace xr;

Swapchain::Swapchain(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	uint32_t formatCount = 0;
	MIRU_XR_ASSERT(xrEnumerateSwapchainFormats(m_CI.session->m_Session, 0, &formatCount, nullptr), "ERROR: OPENXR: Failed to enumerate SwapchainFormats.");
	m_Format.resize(formatCount);
	MIRU_XR_ASSERT(xrEnumerateSwapchainFormats(m_CI.session->m_Session, formatCount, &formatCount, m_Format.data()), "ERROR: OPENXR: Failed to enumerate SwapchainFormats.");

	if (!arc::FindInVector(m_Format, m_CI.format))
	{
		MIRU_XR_ASSERT(true, "ERROR: OPENXR: Failed to find supplied Format in the enumerated SwapchainFormats for this Session.");
	}

	m_SwapchainCI.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
	m_SwapchainCI.next = nullptr;
	m_SwapchainCI.createFlags = 0;
	m_SwapchainCI.usageFlags = static_cast<XrSwapchainUsageFlags>(m_CI.usage);
	m_SwapchainCI.format = m_CI.format;
	m_SwapchainCI.sampleCount = m_CI.sampleCount;
	m_SwapchainCI.width = m_CI.width;
	m_SwapchainCI.height = m_CI.height;
	m_SwapchainCI.faceCount = m_CI.faceCount;
	m_SwapchainCI.arraySize = m_CI.arraySize;
	m_SwapchainCI.mipCount = m_CI.mipCount;
	MIRU_XR_ASSERT(xrCreateSwapchain(m_CI.session->m_Session, &m_SwapchainCI, &m_Swapchain), "ERROR: OPENXR: Failed to create Swapchain.");

	MIRU_XR_ASSERT(xrEnumerateSwapchainImages(m_Swapchain, 0, &m_ImageCount, nullptr), "ERROR: OPENXR: Failed to enumerate SwapchainImages.");
	XrSwapchainImageBaseHeader* swapchainImages = nullptr;
#if defined(MIRU_XR_D3D12)
	if (m_CI.session->m_CI.instance->m_CI.api == Instance::API::D3D12)
	{
		m_SwapchainImagesD3D12.resize(m_ImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR });
		swapchainImages = (XrSwapchainImageBaseHeader*)m_SwapchainImagesD3D12.data();
	}
#endif
#if defined(MIRU_XR_VULKAN)
	if (m_CI.session->m_CI.instance->m_CI.api == Instance::API::VULKAN)
	{
		m_SwapchainImagesVulkan.resize(m_ImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });
		swapchainImages = (XrSwapchainImageBaseHeader*)m_SwapchainImagesVulkan.data();
	}
#endif
	MIRU_XR_ASSERT(xrEnumerateSwapchainImages(m_Swapchain, m_ImageCount, &m_ImageCount, swapchainImages), "ERROR: OPENXR: Failed to enumerate SwapchainImages.");
}

Swapchain::~Swapchain()
{
	MIRU_XR_ASSERT(xrDestroySwapchain(m_Swapchain), "ERROR: OPENXR: Failed to destroy Swapchain.");
}

void Swapchain::Acquire()
{
	m_SwapchianImageAI.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
	m_SwapchianImageAI.next = nullptr;
	MIRU_XR_ASSERT(xrAcquireSwapchainImage(m_Swapchain, &m_SwapchianImageAI, &m_ImageIndex), "ERROR: OPENXR: Failed to acquire SwapchainImage.");
}

void Swapchain::Wait(int64_t timeout_ns)
{
	m_SwapchianImageWI.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
	m_SwapchianImageWI.next = nullptr;
	m_SwapchianImageWI.timeout = static_cast<XrDuration>(timeout_ns);
	MIRU_XR_ASSERT(xrWaitSwapchainImage(m_Swapchain, &m_SwapchianImageWI), "ERROR: OPENXR: Failed to wait SwapchainImage.");
}

void Swapchain::Release()
{
	m_SwapchianImageRI.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
	m_SwapchianImageRI.next = nullptr;
	MIRU_XR_ASSERT(xrReleaseSwapchainImage(m_Swapchain, &m_SwapchianImageRI), "ERROR: OPENXR: Failed to release SwapchainImage.");
}

Swapchain::Image miru::xr::Swapchain::GetImage(uint32_t imageIndex)
{
#if defined(MIRU_XR_D3D12)
	if (m_CI.session->m_CI.instance->m_CI.api == Instance::API::D3D12)
	{
		return m_SwapchainImagesD3D12[imageIndex].texture;
	}
#endif
#if defined(MIRU_XR_VULKAN)
	else if (m_CI.session->m_CI.instance->m_CI.api == Instance::API::VULKAN)
	{
		return m_SwapchainImagesVulkan[imageIndex].image;
	}
#endif
	return nullptr;
}
