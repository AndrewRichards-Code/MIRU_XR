#include "miru_core.h"
#define MIRU_D3D12
#include "d3d12/D3D12_Include.h" //Must include D3D12_Include.h ahead of miru_xr_core.h's d3d12.h
#include "d3d12/D3D12Context.h"
#include "d3d12/D3D12Image.h"
#define MIRU_VULKAN
#include "vulkan/VK_Include.h"
#include "vulkan/VKContext.h"
#include "vulkan/VKImage.h"

#include "miru_xr_core.h"

using namespace miru;
using namespace xr;
using namespace base;

int main()
{
	Instance::CreateInfo instanceCI;
	instanceCI.applicationName = "MIRU_XR_TEST";
	instanceCI.api = Instance::API::D3D12;
	InstanceRef instance = CreateRef<Instance>(&instanceCI);

	System::CreateInfo systemCI;
	systemCI.instance = instance;
	systemCI.formFactor = System::FormFactor::HEAD_MOUNTED_DISPLAY;
	SystemRef system = CreateRef<System>(&systemCI);

	ContextRef context;
	Context::CreateInfo contextCI;
	contextCI.applicationName = instanceCI.applicationName;
	contextCI.debugValidationLayers = true;
	contextCI.extensions = Context::ExtensionsBit::MULTIVIEW | Context::ExtensionsBit::SHADER_VIEWPORT_INDEX_LAYER;
	contextCI.deviceDebugName = "GPU Device";

	d3d12::Context::OpenXRD3D12Data openXRD3D12Data;
	vulkan::Context::OpenXRVulkanData openXRVulkanData;
	if (instanceCI.api == Instance::API::D3D12)
	{
		GraphicsAPI::SetAPI(GraphicsAPI::API::D3D12);
		XrGraphicsRequirementsD3D12KHR d3d12GraphicsRequirements = Session::GetGraphicsRequirementsD3D12(instance, system);
		openXRD3D12Data.type = Context::CreateInfoExtensionStructureTypes::OPENXR_D3D12_DATA;
		openXRD3D12Data.pNext = nullptr;
		openXRD3D12Data.adapterLuid = d3d12GraphicsRequirements.adapterLuid;
		openXRD3D12Data.minFeatureLevel = d3d12GraphicsRequirements.minFeatureLevel;
		contextCI.pNext = &openXRD3D12Data;
	}
	else if (instanceCI.api == Instance::API::VULKAN)
	{
		GraphicsAPI::SetAPI(GraphicsAPI::API::VULKAN);
		XrGraphicsRequirementsVulkanKHR vulkanGraphicsRequirements = Session::GetGraphicsRequirementsVulkan(instance, system);
		openXRVulkanData.type = Context::CreateInfoExtensionStructureTypes::OPENXR_VULKAN_DATA;
		openXRVulkanData.pNext = nullptr;
		openXRVulkanData.minApiVersionMajorSupported = XR_VERSION_MAJOR(vulkanGraphicsRequirements.minApiVersionSupported);
		openXRVulkanData.minApiVersionMinorSupported = XR_VERSION_MINOR(vulkanGraphicsRequirements.minApiVersionSupported);
		openXRVulkanData.maxApiVersionMajorSupported = XR_VERSION_MAJOR(vulkanGraphicsRequirements.maxApiVersionSupported);
		openXRVulkanData.maxApiVersionMinorSupported = XR_VERSION_MINOR(vulkanGraphicsRequirements.maxApiVersionSupported);
		openXRVulkanData.instanceExtensions = Session::GetInstanceExtensionsVulkan(instance, system);
		openXRVulkanData.deviceExtensions = Session::GetDeviceExtensionsVulkan(instance, system);
		openXRVulkanData.getPhysicalDeviceVulkan = [](VkInstance vkInstance, Ref<void> miruXrInstance, Ref<void> miruXrSystem) -> VkPhysicalDevice {
			return Session::GetPhysicalDeviceVulkan(vkInstance, static_ref_cast<Instance>(miruXrInstance), static_ref_cast<System>(miruXrSystem)); };
		openXRVulkanData.miruXrInstance = instance;
		openXRVulkanData.miruXrSystem = system;
		contextCI.pNext = &openXRVulkanData;
	}
	context = Context::Create(&contextCI);

	Session::CreateInfo sessionCI;
	sessionCI.instance = instance;
	sessionCI.system = system;
	if (instanceCI.api == Instance::API::D3D12)
	{
		d3d12::ContextRef d3d12Context = ref_cast<d3d12::Context>(context);
		sessionCI.graphicsBindingD3D12.device = d3d12Context->m_Device;
		sessionCI.graphicsBindingD3D12.queue = d3d12Context->m_Queues[0];
	}
	else if (instanceCI.api == Instance::API::VULKAN)
	{
		vulkan::ContextRef vulkanContext = ref_cast<vulkan::Context>(context);
		sessionCI.graphicsBindingVulkan.instance = vulkanContext->m_Instance;
		sessionCI.graphicsBindingVulkan.physicalDevice = vulkanContext->m_PhysicalDevices.m_PDIs[vulkanContext->m_PhysicalDeviceIndex].m_PhysicalDevice;
		sessionCI.graphicsBindingVulkan.device = vulkanContext->m_Device;
		sessionCI.graphicsBindingVulkan.queueFamilyIndex = 0;
		sessionCI.graphicsBindingVulkan.queueIndex = 0;
	}
	SessionRef session = CreateRef<Session>(&sessionCI);

	ViewConfigurations::CreateInfo viewConfigurationsCI;
	viewConfigurationsCI.instance = instance;
	viewConfigurationsCI.system = system;
	ViewConfigurationsRef viewConfigurations = CreateRef<ViewConfigurations>(&viewConfigurationsCI);

	const ViewConfigurations::Type& viewConfigurationType = viewConfigurations->m_Types[0];
	session->SetViewConfigurationsType(viewConfigurationType);

	xr::Swapchain::CreateInfo swapchainCI;
	swapchainCI.session = session;
	swapchainCI.usage = xr::Swapchain::Usage::COLOUR_ATTACHMENT_BIT | xr::Swapchain::Usage::SAMPLED_BIT;
	swapchainCI.format = GraphicsAPI::IsD3D12() ? DXGI_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;
	swapchainCI.sampleCount = viewConfigurations->m_Views[viewConfigurationType][0].recommendedSwapchainSampleCount;
	swapchainCI.width = viewConfigurations->m_Views[viewConfigurationType][0].recommendedImageRectWidth;
	swapchainCI.height = viewConfigurations->m_Views[viewConfigurationType][0].recommendedImageRectHeight;
	swapchainCI.faceCount = 1;
	swapchainCI.arraySize = static_cast<uint32_t>(viewConfigurations->m_Views[viewConfigurationType].size());
	swapchainCI.mipCount = 1;
	xr::SwapchainRef swapchain = CreateRef<xr::Swapchain>(&swapchainCI);
	std::vector<ImageRef> swapchainImages(swapchain->GetImageCount());
	std::vector<ImageViewRef> swapchainImageViews(swapchain->GetImageCount());

	for (uint32_t i = 0; i < swapchain->GetImageCount(); i++)
	{
		Image::CreateInfo swapchainImageCI;
		swapchainImageCI.debugName = "SwapchainImage";
		swapchainImageCI.device = context->GetDevice();
		swapchainImageCI.type = Image::Type::TYPE_2D_ARRAY;
		swapchainImageCI.format = Image::Format::UNKNOWN;
		swapchainImageCI.width = swapchainCI.width;
		swapchainImageCI.height = swapchainCI.height;
		swapchainImageCI.depth = 1;
		swapchainImageCI.mipLevels = swapchainCI.mipCount;
		swapchainImageCI.arrayLayers = swapchainCI.arraySize;
		swapchainImageCI.sampleCount = Image::SampleCountBit(swapchainCI.sampleCount);
		swapchainImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT | Image::UsageBit::SAMPLED_BIT;
		swapchainImageCI.layout = Image::Layout::UNKNOWN;
		swapchainImageCI.size = 0;
		swapchainImageCI.data = nullptr;
		swapchainImageCI.allocator = nullptr;
		swapchainImageCI.externalImage = swapchain->GetImage(i);
		if (instanceCI.api == Instance::API::D3D12)
			swapchainImageCI.format = d3d12::Image::ToMIRUImageFormat(static_cast<DXGI_FORMAT>(swapchainCI.format));
		else if (instanceCI.api == Instance::API::VULKAN)
			swapchainImageCI.format = static_cast<Image::Format>(swapchainCI.format);
		
		swapchainImages[i] = Image::Create(&swapchainImageCI);

		ImageView::CreateInfo swapchainImageViewCI;
		swapchainImageViewCI.debugName = "SwapchainImageView";
		swapchainImageViewCI.device = context->GetDevice();;
		swapchainImageViewCI.image = swapchainImages[i];
		swapchainImageViewCI.viewType = Image::Type::TYPE_2D_ARRAY;
		swapchainImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 2 };

		swapchainImageViews[i] = ImageView::Create(&swapchainImageViewCI);
	}

	bool applicationRunning = true;
	while (applicationRunning)
	{
		instance->PollEvents(
			nullptr,
			[&](XrEventDataInstanceLossPending* instanceLossPending)
			{
				session->RequestExit();
				applicationRunning = false;
			},
			nullptr,
			nullptr,
			[&](XrEventDataSessionStateChanged* sessionStateChanged)
			{
				session->StateChanged(sessionStateChanged);
				
				if (sessionStateChanged->state == (XrSessionState)Session::State::EXITING)
					applicationRunning = false;
			});
	}
}