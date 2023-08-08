#include "miru_xr_core.h"

using namespace miru;
using namespace xr;
using namespace base;

int main()
{
	GraphicsAPI::SetAPI(GraphicsAPI::API::VULKAN);

	Instance::CreateInfo instanceCI;
	instanceCI.applicationName = "MIRU_XR_TEST";
	instanceCI.api = GraphicsAPI::GetAPI();
	InstanceRef instance = CreateRef<Instance>(&instanceCI);

	System::CreateInfo systemCI;
	systemCI.instance = instance;
	systemCI.formFactor = System::FormFactor::HEAD_MOUNTED_DISPLAY;
	SystemRef system = CreateRef<System>(&systemCI);

	Context::CreateInfo contextCI;
	contextCI.applicationName = instanceCI.applicationName;
	contextCI.debugValidationLayers = true;
	contextCI.extensions = Context::ExtensionsBit::MULTIVIEW | Context::ExtensionsBit::SHADER_VIEWPORT_INDEX_LAYER;
	contextCI.deviceDebugName = "GPU Device";
	contextCI.pNext = Session::GetMIRUOpenXRData(instance, system);
	ContextRef context = Context::Create(&contextCI);

	Session::CreateInfo sessionCI;
	sessionCI.instance = instance;
	sessionCI.system = system;
	sessionCI.context = context;
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
	swapchainCI.format = Image::Format::B8G8R8A8_UNORM;
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
		swapchainImageCI.format = swapchainCI.format;
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