#include "miru_xr_core_common.h"
#include "XRSession.h"
#include "XRInstance.h"
#include "XRSystem.h"

using namespace miru;
using namespace xr;

Session::Session(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_SessionCI.type = XR_TYPE_SESSION_CREATE_INFO;
	m_SessionCI.next = nullptr;

#if defined(XR_USE_GRAPHICS_API_D3D12)
	XrGraphicsBindingD3D12KHR xrD3D12GraphicsBinding;
	if (m_CI.instance->m_CI.api == Instance::API::D3D12)
	{
		const GraphicsBindingD3D12& graphicsBindingD3D12 = m_CI.graphicsBindingD3D12;
		xrD3D12GraphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_D3D12_KHR;
		xrD3D12GraphicsBinding.next = nullptr;
		xrD3D12GraphicsBinding.device = graphicsBindingD3D12.device;
		xrD3D12GraphicsBinding.queue = graphicsBindingD3D12.queue;
		m_SessionCI.next = &xrD3D12GraphicsBinding;
	}
#endif
#if defined(XR_USE_GRAPHICS_API_VULKAN)
	XrGraphicsBindingVulkanKHR xrVulkanGraphicsBinding;
	if (m_CI.instance->m_CI.api == Instance::API::VULKAN)
	{
		const GraphicsBindingVulkan& graphicsBindingVulkan = m_CI.graphicsBindingVulkan;
		xrVulkanGraphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
		xrVulkanGraphicsBinding.next = nullptr;
		xrVulkanGraphicsBinding.instance = graphicsBindingVulkan.instance;
		xrVulkanGraphicsBinding.physicalDevice = graphicsBindingVulkan.physicalDevice;
		xrVulkanGraphicsBinding.device = graphicsBindingVulkan.device;
		xrVulkanGraphicsBinding.queueFamilyIndex = graphicsBindingVulkan.queueFamilyIndex;
		xrVulkanGraphicsBinding.queueIndex = graphicsBindingVulkan.queueIndex;
		m_SessionCI.next = &xrVulkanGraphicsBinding;
	}
#endif

	m_SessionCI.createFlags = 0;
	m_SessionCI.systemId = m_CI.system->m_SystemID;
	
	MIRU_XR_ASSERT(xrCreateSession(m_CI.instance->m_Instance, &m_SessionCI, &m_Session), "ERROR: OPENXR: Failed to create Session.");
}

Session::~Session()
{
	MIRU_XR_ASSERT(xrDestroySession(m_Session), "ERROR: OPENXR: Failed to destroy Session.");
}

void Session::StateChanged(XrEventDataSessionStateChanged* sessionStateChanged)
{
	if (sessionStateChanged->state == XR_SESSION_STATE_READY) {
		Begin();
		m_Running = true;
	}
	if (sessionStateChanged->state == XR_SESSION_STATE_STOPPING) {
		End();
		m_Running = false;
	}
	if (sessionStateChanged->state == XR_SESSION_STATE_EXITING) {
		m_Running = false;
	}
	m_State = static_cast<State>(sessionStateChanged->state);
}

void Session::Begin()
{
	m_SessionBI.type = XR_TYPE_SESSION_BEGIN_INFO;
	m_SessionBI.next = nullptr;
	m_SessionBI.primaryViewConfigurationType = static_cast<XrViewConfigurationType>(m_Type);

	MIRU_XR_ASSERT(xrBeginSession(m_Session, &m_SessionBI), "ERROR: OPENXR: Failed to begin Session.");
}

void Session::End()
{
	MIRU_XR_ASSERT(xrEndSession(m_Session), "ERROR: OPENXR: Failed to end Session.");
}

void Session::RequestExit()
{
	MIRU_XR_ASSERT(xrRequestExitSession(m_Session), "ERROR: OPENXR: Failed to request exit Session.");
}

#if defined(XR_USE_GRAPHICS_API_D3D12)
XrGraphicsRequirementsD3D12KHR Session::GetGraphicsRequirementsD3D12(InstanceRef instance, SystemRef system) 
{
	PFN_xrGetD3D12GraphicsRequirementsKHR xrGetD3D12GraphicsRequirementsKHR = (PFN_xrGetD3D12GraphicsRequirementsKHR)GetInstanceProcAddr(instance->m_Instance, "xrGetD3D12GraphicsRequirementsKHR");
	XrGraphicsRequirementsD3D12KHR graphicsRequirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR };
	MIRU_XR_ASSERT(xrGetD3D12GraphicsRequirementsKHR(instance->m_Instance, system->m_SystemID, &graphicsRequirements), "ERROR: OPENXR: Failed to get Graphics Requirements for D3D12.");
	return graphicsRequirements;
}
#endif

#if defined(XR_USE_GRAPHICS_API_VULKAN)
XrGraphicsRequirementsVulkanKHR Session::GetGraphicsRequirementsVulkan(InstanceRef instance, SystemRef system)
{
	PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR = (PFN_xrGetVulkanGraphicsRequirementsKHR)GetInstanceProcAddr(instance->m_Instance, "xrGetVulkanGraphicsRequirementsKHR");
	XrGraphicsRequirementsVulkanKHR graphicsRequirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
	MIRU_XR_ASSERT(xrGetVulkanGraphicsRequirementsKHR(instance->m_Instance, system->m_SystemID, &graphicsRequirements), "ERROR: OPENXR: Failed to get Graphics Requirements for Vulkan.");
	return graphicsRequirements;
}
std::vector<std::string> Session::GetInstanceExtensionsVulkan(InstanceRef instance, SystemRef system)
{
	PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR = (PFN_xrGetVulkanInstanceExtensionsKHR)GetInstanceProcAddr(instance->m_Instance, "xrGetVulkanInstanceExtensionsKHR");

	uint32_t extensionNamesSize = 0;
	MIRU_XR_ASSERT(xrGetVulkanInstanceExtensionsKHR(instance->m_Instance, system->m_SystemID, 0, &extensionNamesSize, nullptr), "ERROR: OPENXR: Failed to get Vulkan Instance Extensions.");

	std::vector<char> extensionNames(extensionNamesSize);
	MIRU_XR_ASSERT(xrGetVulkanInstanceExtensionsKHR(instance->m_Instance, system->m_SystemID, extensionNamesSize, &extensionNamesSize, extensionNames.data()), "ERROR: OPENXR: Failed to get Vulkan Instance Extensions.");

	std::stringstream streamData(extensionNames.data());
	std::vector<std::string> extensions;
	std::string extension;
	while (std::getline(streamData, extension, ' ')) {
		extensions.push_back(extension);
	}
	return extensions;
}
std::vector<std::string> Session::GetDeviceExtensionsVulkan(InstanceRef instance, SystemRef system)
{
	PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR = (PFN_xrGetVulkanDeviceExtensionsKHR)GetInstanceProcAddr(instance->m_Instance, "xrGetVulkanDeviceExtensionsKHR");

	uint32_t extensionNamesSize = 0;
	MIRU_XR_ASSERT(xrGetVulkanDeviceExtensionsKHR(instance->m_Instance, system->m_SystemID, 0, &extensionNamesSize, nullptr), "ERROR: OPENXR: Failed to get Vulkan Device Extensions.");

	std::vector<char> extensionNames(extensionNamesSize);
	MIRU_XR_ASSERT(xrGetVulkanDeviceExtensionsKHR(instance->m_Instance, system->m_SystemID, extensionNamesSize, &extensionNamesSize, extensionNames.data()), "ERROR: OPENXR: Failed to get Vulkan Device Extensions.");

	std::stringstream streamData(extensionNames.data());
	std::vector<std::string> extensions;
	std::string extension;
	while (std::getline(streamData, extension, ' ')) {
		extensions.push_back(extension);
	}
	return extensions;
}
VkPhysicalDevice Session::GetPhysicalDeviceVulkan(VkInstance vkInstance, InstanceRef instance, SystemRef system)
{
	PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR = (PFN_xrGetVulkanGraphicsDeviceKHR)GetInstanceProcAddr(instance->m_Instance, "xrGetVulkanGraphicsDeviceKHR");
	
	VkPhysicalDevice physicalDevice;
	MIRU_XR_ASSERT(xrGetVulkanGraphicsDeviceKHR(instance->m_Instance, system->m_SystemID, vkInstance, &physicalDevice), "ERROR: OPENXR: Failed to get Graphics Device for Vulkan.");
	return physicalDevice;
}
#endif