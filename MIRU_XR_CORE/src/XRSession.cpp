#include "miru_xr_core_common.h"
#include "XRSession.h"
#include "XRInstance.h"
#include "XRSpace.h"
#include "XRSwapchain.h"
#include "XRSystem.h"

#include "d3d12/D3D12Context.h"
#include "vulkan/VKContext.h"

using namespace miru;
using namespace xr;

d3d12::Context::OpenXRD3D12Data openXRD3D12Data = {};
vulkan::Context::OpenXRVulkanData openXRVulkanData = {};

Session::Session(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_SessionCI.type = XR_TYPE_SESSION_CREATE_INFO;
	m_SessionCI.next = nullptr;

#if defined(XR_USE_GRAPHICS_API_D3D12)
	XrGraphicsBindingD3D12KHR xrD3D12GraphicsBinding;
	if (m_CI.instance->m_CI.api == base::GraphicsAPI::API::D3D12)
	{
		d3d12::ContextRef context = ref_cast<d3d12::Context>(m_CI.context);
		xrD3D12GraphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_D3D12_KHR;
		xrD3D12GraphicsBinding.next = nullptr;
		xrD3D12GraphicsBinding.device = context->m_Device;
		xrD3D12GraphicsBinding.queue = context->m_Queues[0];
		m_SessionCI.next = &xrD3D12GraphicsBinding;
	}
#endif
#if defined(XR_USE_GRAPHICS_API_VULKAN)
	XrGraphicsBindingVulkanKHR xrVulkanGraphicsBinding;
	if (m_CI.instance->m_CI.api == base::GraphicsAPI::API::VULKAN)
	{
		vulkan::ContextRef context = ref_cast<vulkan::Context>(m_CI.context);
		xrVulkanGraphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
		xrVulkanGraphicsBinding.next = nullptr;
		xrVulkanGraphicsBinding.instance = context->m_Instance;
		xrVulkanGraphicsBinding.physicalDevice = context->m_PhysicalDevices.m_PDIs[context->m_PhysicalDeviceIndex].m_PhysicalDevice;
		xrVulkanGraphicsBinding.device = context->m_Device;
		xrVulkanGraphicsBinding.queueFamilyIndex = 0;
		xrVulkanGraphicsBinding.queueIndex = 0;
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

bool Session::IsActive()
{
	return (m_State == State::SYNCHRONIZED || m_State == State::VISIBLE || m_State == State::FOCUSED);
}

void Session::WaitFrame()
{
	m_FrameState.type = XR_TYPE_FRAME_STATE;
	m_FrameState.next =  nullptr;
	m_FrameState.predictedDisplayTime = 0;
	m_FrameState.predictedDisplayPeriod = 0;
	m_FrameState.shouldRender = false;

	XrFrameWaitInfo frameWaitInfo;
	frameWaitInfo.type = XR_TYPE_FRAME_WAIT_INFO;
	frameWaitInfo.next = nullptr;
	MIRU_XR_ASSERT(xrWaitFrame(m_Session, &frameWaitInfo, &m_FrameState), "ERROR: OPENXR: Failed to wait for XR Frame.");
}

void Session::BeginFrame()
{
	XrFrameBeginInfo frameBeginInfo;
	frameBeginInfo.type = XR_TYPE_FRAME_BEGIN_INFO;
	frameBeginInfo.next = nullptr;
	MIRU_XR_ASSERT(xrBeginFrame(m_Session, &frameBeginInfo), "ERROR: OPENXR: Failed to begin the XR Frame.");
}

void Session::EndFrame(const std::vector<CompositionLayer::BaseHeader*>& layers)
{
	std::vector<XrCompositionLayerProjection> layerProjections;
	std::vector<std::vector<XrCompositionLayerProjectionView>> layerProjectionViews;
	std::vector<XrCompositionLayerQuad> layerQuads;

	layerProjections.reserve(layers.size());
	layerProjectionViews.reserve(layers.size());
	layerQuads.reserve(layers.size());

	for (auto& layer : layers)
	{
		switch (layer->type)
		{
		default:
		case CompositionLayer::Type::BASE_HEADER:
		{
			continue;
		}
		case CompositionLayer::Type::PROJECTION:
		{
			CompositionLayer::Projection* _layerProjection = reinterpret_cast<CompositionLayer::Projection*>(layer);
			
			layerProjectionViews.push_back({});
			for (auto& _layerProjectionView : _layerProjection->views)
			{
				XrCompositionLayerProjectionView layerProjectionView;
				layerProjectionView.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
				layerProjectionView.next = nullptr;
				layerProjectionView.pose = _layerProjectionView.view.pose;
				layerProjectionView.fov = _layerProjectionView.view.fov;
				layerProjectionView.subImage.swapchain = _layerProjectionView.swapchainSubImage.swapchain->m_Swapchain;
				layerProjectionView.subImage.imageRect.offset.x = _layerProjectionView.swapchainSubImage.imageRect.offset.x;
				layerProjectionView.subImage.imageRect.offset.y = _layerProjectionView.swapchainSubImage.imageRect.offset.y;
				layerProjectionView.subImage.imageRect.extent.width = _layerProjectionView.swapchainSubImage.imageRect.extent.width;
				layerProjectionView.subImage.imageRect.extent.height = _layerProjectionView.swapchainSubImage.imageRect.extent.height;
				layerProjectionView.subImage.imageArrayIndex = _layerProjectionView.swapchainSubImage.imageArrayIndex;
				layerProjectionViews.back().push_back(layerProjectionView);
			}

			XrCompositionLayerProjection layerProjection;
			layerProjection.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
			layerProjection.next = nullptr;
			layerProjection.layerFlags = static_cast<XrCompositionLayerFlags>(layer->layerFlags);
			layerProjection.space = layer->space->m_Space;
			layerProjection.viewCount = static_cast<uint32_t>(layerProjectionViews.back().size());
			layerProjection.views = layerProjectionViews.back().data();
			layerProjections.push_back(layerProjection);

			continue;
		}
		case CompositionLayer::Type::QUAD:
		{
			CompositionLayer::Quad* _layerQuad = reinterpret_cast<CompositionLayer::Quad*>(layer);
			XrCompositionLayerQuad layerQuad;
			layerQuad.type = XR_TYPE_COMPOSITION_LAYER_QUAD;
			layerQuad.next = nullptr;
			layerQuad.layerFlags = static_cast<XrCompositionLayerFlags>(layer->layerFlags);
			layerQuad.space = layer->space->m_Space;
			layerQuad.eyeVisibility = static_cast<XrEyeVisibility>(_layerQuad->eyeVisibility);
			layerQuad.subImage;
			layerQuad.pose = _layerQuad->pose;
			layerQuad.size.width = _layerQuad->width;
			layerQuad.size.height = _layerQuad->height;
			layerQuads.push_back(layerQuad);
			continue;
		}
		}
	}

	std::vector<XrCompositionLayerBaseHeader*> _layers;
	for (auto& layerProjection : layerProjections)
		_layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&layerProjection));
	for (auto& layerQuad : layerQuads)
		_layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&layerQuad));

	XrFrameEndInfo frameEndInfo;
	frameEndInfo.type = XR_TYPE_FRAME_END_INFO;
	frameEndInfo.next = nullptr;
	frameEndInfo.displayTime = m_FrameState.predictedDisplayTime;
	frameEndInfo.environmentBlendMode = static_cast<XrEnvironmentBlendMode>(m_EnvironmentBlendMode);
	frameEndInfo.layerCount = static_cast<uint32_t>(_layers.size());
	frameEndInfo.layers = _layers.data();
	MIRU_XR_ASSERT(xrEndFrame(m_Session, &frameEndInfo), "ERROR: OPENXR: Failed to end the XR Frame.");

	for (auto& layerProjectionView : layerProjectionViews)
		layerProjectionView.clear();
}

bool Session::LocateViews(SpaceRef space, std::vector<View>& views)
{
	XrViewState viewState;
	viewState.type = XR_TYPE_VIEW_STATE;
	viewState.next = nullptr;
	viewState.viewStateFlags = 0;

	XrViewLocateInfo viewLocateInfo;
	viewLocateInfo.type = XR_TYPE_VIEW_LOCATE_INFO;
	viewLocateInfo.next = nullptr;
	viewLocateInfo.viewConfigurationType = static_cast<XrViewConfigurationType>(m_Type);
	viewLocateInfo.displayTime = m_FrameState.predictedDisplayTime;
	viewLocateInfo.space = space->m_Space;
	uint32_t viewCount = 0;

	XrResult result = xrLocateViews(m_Session, &viewLocateInfo, &viewState, 0, &viewCount, nullptr);
	if (result != XR_SUCCESS) 
	{
		std::cout << "ERROR: OPENXR: Failed to locate Views." << std::endl;
		return false;
	}

	std::vector<XrView> xrViews(viewCount, { XR_TYPE_VIEW });

	result = xrLocateViews(m_Session, &viewLocateInfo, &viewState, static_cast<uint32_t>(xrViews.size()), &viewCount, xrViews.data());
	if (result != XR_SUCCESS)
	{
		std::cout << "ERROR: OPENXR: Failed to locate Views." << std::endl;
		return false;
	}

	for (const auto& xrView : xrViews)
	{
		View view;
		view.pose = xrView.pose;
		view.fov = xrView.fov;
		views.push_back(view);
	}

	return true;
}

void* Session::GetMIRUOpenXRData(InstanceRef instance, SystemRef system) 
{
#if defined(XR_USE_GRAPHICS_API_D3D12)
	if (instance->m_CI.api == base::GraphicsAPI::API::D3D12)
	{
		PFN_xrGetD3D12GraphicsRequirementsKHR xrGetD3D12GraphicsRequirementsKHR = (PFN_xrGetD3D12GraphicsRequirementsKHR)GetInstanceProcAddr(instance->m_Instance, "xrGetD3D12GraphicsRequirementsKHR");
		XrGraphicsRequirementsD3D12KHR graphicsRequirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR };
		MIRU_XR_ASSERT(xrGetD3D12GraphicsRequirementsKHR(instance->m_Instance, system->m_SystemID, &graphicsRequirements), "ERROR: OPENXR: Failed to get Graphics Requirements for D3D12.");

		openXRD3D12Data.type = base::Context::CreateInfoExtensionStructureTypes::OPENXR_D3D12_DATA;
		openXRD3D12Data.pNext = nullptr;
		openXRD3D12Data.adapterLuid = graphicsRequirements.adapterLuid;
		openXRD3D12Data.minFeatureLevel = graphicsRequirements.minFeatureLevel;
		return &openXRD3D12Data;
	}
#endif
#if defined(XR_USE_GRAPHICS_API_VULKAN)
	if (instance->m_CI.api == base::GraphicsAPI::API::VULKAN)
	{
		PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR = (PFN_xrGetVulkanGraphicsRequirementsKHR)GetInstanceProcAddr(instance->m_Instance, "xrGetVulkanGraphicsRequirementsKHR");
		XrGraphicsRequirementsVulkanKHR graphicsRequirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
		MIRU_XR_ASSERT(xrGetVulkanGraphicsRequirementsKHR(instance->m_Instance, system->m_SystemID, &graphicsRequirements), "ERROR: OPENXR: Failed to get Graphics Requirements for Vulkan.");

		auto GetInstanceExtensionsVulkan = [&](InstanceRef instance, SystemRef system) -> std::vector<std::string>
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
		};

		auto GetDeviceExtensionsVulkan = [&](InstanceRef instance, SystemRef system) -> std::vector<std::string>
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
		};

		openXRVulkanData.type = base::Context::CreateInfoExtensionStructureTypes::OPENXR_VULKAN_DATA;
		openXRVulkanData.pNext = nullptr;
		openXRVulkanData.minApiVersionMajorSupported = XR_VERSION_MAJOR(graphicsRequirements.minApiVersionSupported);
		openXRVulkanData.minApiVersionMinorSupported = XR_VERSION_MINOR(graphicsRequirements.minApiVersionSupported);
		openXRVulkanData.maxApiVersionMajorSupported = XR_VERSION_MAJOR(graphicsRequirements.maxApiVersionSupported);
		openXRVulkanData.maxApiVersionMinorSupported = XR_VERSION_MINOR(graphicsRequirements.maxApiVersionSupported);
		openXRVulkanData.instanceExtensions = GetInstanceExtensionsVulkan(instance, system);
		openXRVulkanData.deviceExtensions = GetDeviceExtensionsVulkan(instance, system);
		openXRVulkanData.getPhysicalDeviceVulkan = [&](VkInstance vkInstance) -> VkPhysicalDevice 
		{
			PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR = (PFN_xrGetVulkanGraphicsDeviceKHR)GetInstanceProcAddr(instance->m_Instance, "xrGetVulkanGraphicsDeviceKHR");

			VkPhysicalDevice physicalDevice;
			MIRU_XR_ASSERT(xrGetVulkanGraphicsDeviceKHR(instance->m_Instance, system->m_SystemID, vkInstance, &physicalDevice), "ERROR: OPENXR: Failed to get Graphics Device for Vulkan.");
			return physicalDevice; 
		};
		return &openXRVulkanData;
	}
#endif
	return nullptr;
}