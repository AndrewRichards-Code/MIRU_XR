#include "miru_xr_core_common.h"
#include "XRInstance.h"
#include "XRSystem.h"

using namespace miru;
using namespace xr;

System::System(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	if (!m_CI.instance)
	{
		MIRU_XR_ASSERT(true, "ERROR: OPENXR: Instance is nullptr.");
	}

	m_SystemGI.type = XR_TYPE_SYSTEM_GET_INFO;
	m_SystemGI.next = nullptr;
	m_SystemGI.formFactor = static_cast<XrFormFactor>(m_CI.formFactor);
	MIRU_XR_ASSERT(xrGetSystem(m_CI.instance->m_Instance, &m_SystemGI, &m_SystemID), "ERROR: OPENXR: Failed to get System.");

	m_SystemProperties.type = XR_TYPE_SYSTEM_PROPERTIES;
	m_SystemProperties.next = nullptr;
	MIRU_XR_ASSERT(xrGetSystemProperties(m_CI.instance->m_Instance, m_SystemID, &m_SystemProperties), "ERROR: OPENXR: Failed to get SystemProperties.");
}

System::~System()
{
}

#if defined(XR_USE_GRAPHICS_API_D3D12)
XrGraphicsRequirementsD3D12KHR System::GetD3D12GraphicsRequirements()
{
	XrGraphicsRequirementsD3D12KHR graphicsRequirements;
	PFN_xrGetD3D12GraphicsRequirementsKHR xrGetD3D12GraphicsRequirementsKHR;
	MIRU_XR_ASSERT(xrGetInstanceProcAddr(m_CI.instance->m_Instance, "xrGetD3D12GraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetD3D12GraphicsRequirementsKHR), "ERROR: OPENXR: Failed to get InstanceProcAddr.");
	MIRU_XR_ASSERT(xrGetD3D12GraphicsRequirementsKHR(m_CI.instance->m_Instance, m_SystemID, &graphicsRequirements), "ERROR: OPENXR: Failed to get D3D12GraphicsRequirements.");
	return graphicsRequirements;
}
#endif

#if defined(XR_USE_GRAPHICS_API_VULKAN)
XrGraphicsRequirementsVulkanKHR System::GetVulkanGraphicsRequirements()
{
	XrGraphicsRequirementsVulkanKHR graphicsRequirements;
	PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR;
	MIRU_XR_ASSERT(xrGetInstanceProcAddr(m_CI.instance->m_Instance, "xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsRequirementsKHR), "ERROR: OPENXR: Failed to get InstanceProcAddr.");
	MIRU_XR_ASSERT(xrGetVulkanGraphicsRequirementsKHR(m_CI.instance->m_Instance, m_SystemID, &graphicsRequirements), "ERROR: OPENXR: Failed to get D3D12GraphicsRequirements.");
	return graphicsRequirements;
}
#endif