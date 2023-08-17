#include "miru_xr_core.h"

#define STB_IMAGE_IMPLEMENTATION
#include "STBI/stb_image.h"

using namespace miru;
using namespace xr;
using namespace base;

int main()
{
	GraphicsAPI::SetAPI(GraphicsAPI::API::D3D12);

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
	contextCI.extensions = Context::ExtensionsBit::MULTIVIEW | Context::ExtensionsBit::SHADER_VIEWPORT_INDEX_LAYER | Context::ExtensionsBit::DYNAMIC_RENDERING;
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
	const ViewConfigurations::EnvironmentBlendMode& environmentBlendMode = viewConfigurations->m_EnvironmentBlendModes[viewConfigurationType][0];
	session->SetViewConfigurationsType(viewConfigurationType);
	session->SetViewConfigurationsEnvironmentBlendMode(environmentBlendMode);

	ReferenceSpace::CreateInfo referenceSpaceCI;
	referenceSpaceCI.session = session;
	referenceSpaceCI.type = ReferenceSpace::Type::VIEW;
	referenceSpaceCI.pose.orientation = { 1.0, 0.0, 0.0, 0.0 };
	referenceSpaceCI.pose.position = { 0.0f, 0.0f, 0.0f };
	ReferenceSpaceRef referenceSpace = CreateRef<ReferenceSpace>(&referenceSpaceCI);

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
		swapchainImageViewCI.device = context->GetDevice();
		swapchainImageViewCI.image = swapchainImages[i];
		swapchainImageViewCI.viewType = Image::Type::TYPE_2D_ARRAY;
		swapchainImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 2 };
		swapchainImageViews[i] = ImageView::Create(&swapchainImageViewCI);
	}

	Allocator::CreateInfo allocCI;
	allocCI.debugName = "CPU_ALLOC_0";
	allocCI.context = context;
	allocCI.blockSize = Allocator::BlockSize::BLOCK_SIZE_64MB;
	allocCI.properties = Allocator::PropertiesBit::HOST_VISIBLE_BIT | Allocator::PropertiesBit::HOST_COHERENT_BIT;
	AllocatorRef cpu_alloc_0 = Allocator::Create(&allocCI);
	allocCI.debugName = "GPU_ALLOC_0";
	allocCI.properties = Allocator::PropertiesBit::DEVICE_LOCAL_BIT;
	AllocatorRef gpu_alloc_0 = Allocator::Create(&allocCI);

	Image::CreateInfo depthImageCI;
	depthImageCI.debugName = "DepthImage";
	depthImageCI.device = context->GetDevice();
	depthImageCI.type = Image::Type::TYPE_2D_ARRAY;
	depthImageCI.format = Image::Format::D32_SFLOAT;
	depthImageCI.width = swapchainCI.width;
	depthImageCI.height = swapchainCI.height;
	depthImageCI.depth = 1;
	depthImageCI.mipLevels = swapchainCI.mipCount;
	depthImageCI.arrayLayers = swapchainCI.arraySize;
	depthImageCI.sampleCount = Image::SampleCountBit(swapchainCI.sampleCount);
	depthImageCI.usage = Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageCI.layout = GraphicsAPI::IsD3D12() ? Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL : Image::Layout::UNKNOWN;
	depthImageCI.size = 0;
	depthImageCI.data = nullptr;
	depthImageCI.allocator = gpu_alloc_0;
	depthImageCI.externalImage = nullptr;
	ImageRef depthImage = Image::Create(&depthImageCI);

	ImageView::CreateInfo depthImageViewCI;
	depthImageViewCI.debugName = "SwapchainImageView";
	depthImageViewCI.device = context->GetDevice();
	depthImageViewCI.image = depthImage;
	depthImageViewCI.viewType = Image::Type::TYPE_2D_ARRAY;
	depthImageViewCI.subresourceRange = { Image::AspectBit::DEPTH_BIT, 0, 1, 0, 2 };
	ImageViewRef depthImageView = ImageView::Create(&depthImageViewCI);

	float vertices[32] =
	{
		-0.5f, -0.5f, -0.5f, 1.0f,
		+0.5f, -0.5f, -0.5f, 1.0f,
		+0.5f, +0.5f, -0.5f, 1.0f,
		-0.5f, +0.5f, -0.5f, 1.0f,
		-0.5f, -0.5f, +0.5f, 1.0f,
		+0.5f, -0.5f, +0.5f, 1.0f,
		+0.5f, +0.5f, +0.5f, 1.0f,
		-0.5f, +0.5f, +0.5f, 1.0f,
	};
	uint32_t indices[36] = {
		0, 1, 2, 2, 3, 0,
		1, 5, 6, 6, 2, 1,
		5, 4, 7, 7, 6, 5,
		4, 0, 3, 3, 7, 4,
		3, 2, 6, 6, 7, 3,
		4, 5, 1, 1, 0, 4
	};

	Buffer::CreateInfo verticesBufferCI;
	verticesBufferCI.debugName = "Vertices Buffer";
	verticesBufferCI.device = context->GetDevice();
	verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	verticesBufferCI.size = sizeof(vertices);
	verticesBufferCI.data = vertices;
	verticesBufferCI.allocator = cpu_alloc_0;
	BufferRef c_vb = Buffer::Create(&verticesBufferCI);
	verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::VERTEX_BIT;
	verticesBufferCI.data = nullptr;
	verticesBufferCI.allocator = gpu_alloc_0;
	BufferRef g_vb = Buffer::Create(&verticesBufferCI);
	BufferView::CreateInfo vbViewCI;
	vbViewCI.debugName = "VerticesBufferView";
	vbViewCI.device = context->GetDevice();
	vbViewCI.type = BufferView::Type::VERTEX;
	vbViewCI.buffer = g_vb;
	vbViewCI.offset = 0;
	vbViewCI.size = sizeof(vertices);
	vbViewCI.stride = 4 * sizeof(float);
	BufferViewRef vbv = BufferView::Create(&vbViewCI);

	Buffer::CreateInfo indicesBufferCI;
	indicesBufferCI.debugName = "Indices Buffer";
	indicesBufferCI.device = context->GetDevice();
	indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	indicesBufferCI.size = sizeof(indices);
	indicesBufferCI.data = indices;
	indicesBufferCI.allocator = cpu_alloc_0;
	BufferRef c_ib = Buffer::Create(&indicesBufferCI);
	indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::INDEX_BIT;
	indicesBufferCI.data = nullptr;
	indicesBufferCI.allocator = gpu_alloc_0;
	BufferRef g_ib = Buffer::Create(&indicesBufferCI);
	BufferView::CreateInfo ibViewCI;
	ibViewCI.debugName = "IndicesBufferView";
	ibViewCI.device = context->GetDevice();
	ibViewCI.type = BufferView::Type::INDEX;
	ibViewCI.buffer = g_ib;
	ibViewCI.offset = 0;
	ibViewCI.size = sizeof(indices);
	ibViewCI.stride = sizeof(uint32_t);
	BufferViewRef ibv = BufferView::Create(&ibViewCI);

	int img_width;
	int img_height;
	int bpp;
	uint8_t* imageData = stbi_load("../../MIRU/Branding/logo.png", &img_width, &img_height, &bpp, 4);
	Buffer::CreateInfo imageBufferCI;
	imageBufferCI.debugName = "MIRU logo upload buffer";
	imageBufferCI.device = context->GetDevice();
	imageBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	imageBufferCI.size = img_width * img_height * 4;
	imageBufferCI.data = imageData;
	imageBufferCI.allocator = cpu_alloc_0;
	BufferRef c_imageBuffer = Buffer::Create(&imageBufferCI);
	Image::CreateInfo imageCI;
	imageCI.debugName = "MIRU logo Image";
	imageCI.device = context->GetDevice();
	imageCI.type = Image::Type::TYPE_CUBE;
	imageCI.format = Image::Format::R8G8B8A8_UNORM;
	imageCI.width = img_width;
	imageCI.height = img_height;
	imageCI.depth = 1;
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 6;
	imageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	imageCI.usage = Image::UsageBit::TRANSFER_DST_BIT | Image::UsageBit::SAMPLED_BIT;
	imageCI.layout = Image::Layout::UNKNOWN;
	imageCI.size = img_width * img_height * 4;
	imageCI.data = nullptr;
	imageCI.allocator = gpu_alloc_0;
	imageCI.externalImage = nullptr;
	ImageRef image = Image::Create(&imageCI);
	ImageView::CreateInfo imageViewCI;
	imageViewCI.debugName = "MIRU logo ImageView";
	imageViewCI.device = context->GetDevice();
	imageViewCI.image = image;
	imageViewCI.viewType = Image::Type::TYPE_CUBE;
	imageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	ImageViewRef imageView = ImageView::Create(&imageViewCI);
	Sampler::CreateInfo samplerCI;
	samplerCI.debugName = "Default Sampler";
	samplerCI.device = context->GetDevice();
	samplerCI.magFilter = Sampler::Filter::NEAREST;
	samplerCI.minFilter = Sampler::Filter::NEAREST;
	samplerCI.mipmapMode = Sampler::MipmapMode::NEAREST;
	samplerCI.addressModeU = Sampler::AddressMode::REPEAT;
	samplerCI.addressModeV = Sampler::AddressMode::REPEAT;
	samplerCI.addressModeW = Sampler::AddressMode::REPEAT;
	samplerCI.mipLodBias = 1;
	samplerCI.anisotropyEnable = false;
	samplerCI.maxAnisotropy = 1.0f;
	samplerCI.compareEnable = false;
	samplerCI.compareOp = CompareOp::NEVER;
	samplerCI.minLod = 0;
	samplerCI.maxLod = 1;
	samplerCI.borderColour = Sampler::BorderColour::FLOAT_OPAQUE_BLACK;
	samplerCI.unnormalisedCoordinates = false;
	SamplerRef sampler = Sampler::Create(&samplerCI);

	mars::float4x4 proj = mars::float4x4::Perspective(mars::DegToRad(90.0), float(swapchainCI.width) / float(swapchainCI.height), 0.1f, 100.0f);
	mars::float4x4 view[2] = { mars::float4x4::Identity(), mars::float4x4::Identity() };
	mars::float4x4 modl = mars::float4x4::Translation({ 0.0f, 0.0f, 0.0f });
	float ubData[3 * sizeof(mars::float4x4)];
	memcpy(ubData + 0 * 16, proj.GetData(), sizeof(proj));
	memcpy(ubData + 1 * 16, view[0].GetData(), sizeof(view));

	Buffer::CreateInfo ubCI;
	ubCI.debugName = "Camera UB";
	ubCI.device = context->GetDevice();
	ubCI.usage = Buffer::UsageBit::UNIFORM_BIT;
	ubCI.size = 3 * sizeof(mars::float4x4);
	ubCI.data = ubData;
	ubCI.allocator = cpu_alloc_0;
	BufferRef ub1 = Buffer::Create(&ubCI);
	BufferView::CreateInfo ubViewCamCI;
	ubViewCamCI.debugName = "Camera UBView";
	ubViewCamCI.device = context->GetDevice();
	ubViewCamCI.type = BufferView::Type::UNIFORM;
	ubViewCamCI.buffer = ub1;
	ubViewCamCI.offset = 0;
	ubViewCamCI.size = 3 * sizeof(mars::float4x4);
	ubViewCamCI.stride = 0;
	BufferViewRef ubViewCam = BufferView::Create(&ubViewCamCI);
	ubCI.debugName = "Model UB";
	ubCI.device = context->GetDevice();
	ubCI.usage = Buffer::UsageBit::UNIFORM_BIT;
	ubCI.size = sizeof(mars::float4x4);
	ubCI.data = (void*)modl.GetData();
	ubCI.allocator = cpu_alloc_0;
	BufferRef ub2 = Buffer::Create(&ubCI);
	BufferView::CreateInfo ubViewMdlCI;
	ubViewMdlCI.debugName = "Model UBView";
	ubViewMdlCI.device = context->GetDevice();
	ubViewMdlCI.type = BufferView::Type::UNIFORM;
	ubViewMdlCI.buffer = ub2;
	ubViewMdlCI.offset = 0;
	ubViewMdlCI.size = sizeof(mars::float4x4);
	ubViewMdlCI.stride = 0;
	BufferViewRef ubViewMdl = BufferView::Create(&ubViewMdlCI);

	CommandPool::CreateInfo cmdPoolCI;
	cmdPoolCI.debugName = "CmdPool";
	cmdPoolCI.context = context;
	cmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	cmdPoolCI.queueType = CommandPool::QueueType::GRAPHICS;
	CommandPoolRef cmdPool = CommandPool::Create(&cmdPoolCI);
	cmdPoolCI.queueType = CommandPool::QueueType::TRANSFER;
	CommandPoolRef cmdCopyPool = CommandPool::Create(&cmdPoolCI);

	CommandBuffer::CreateInfo cmdBufferCI, cmdCopyBufferCI;
	cmdBufferCI.debugName = "CmdBuffer";
	cmdBufferCI.commandPool = cmdPool;
	cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	cmdBufferCI.commandBufferCount = swapchain->GetImageCount() + 1;
	CommandBufferRef cmdBuffer = CommandBuffer::Create(&cmdBufferCI);
	cmdCopyBufferCI.debugName = "CmdCopyBuffer";
	cmdCopyBufferCI.commandPool = cmdCopyPool;
	cmdCopyBufferCI.level = CommandBuffer::Level::PRIMARY;
	cmdCopyBufferCI.commandBufferCount = 1;
	CommandBufferRef cmdCopyBuffer = CommandBuffer::Create(&cmdCopyBufferCI);

	//Transfer CmdBuffer
	Fence::CreateInfo transferFenceCI = { "TransferFence", context->GetDevice(), false, UINT64_MAX };
	FenceRef transferFence = Fence::Create(&transferFenceCI);
	Semaphore::CreateInfo transferSemaphoreCI = { "TransferSemaphore", context->GetDevice() };
	SemaphoreRef transferSemaphore = Semaphore::Create(&transferSemaphoreCI);
	{
		cmdCopyBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		cmdCopyBuffer->CopyBuffer(0, c_vb, g_vb, { { 0, 0, sizeof(vertices) } });
		cmdCopyBuffer->CopyBuffer(0, c_ib, g_ib, { { 0, 0, sizeof(indices) } });

		Barrier::CreateInfo bCI;
		bCI.type = Barrier::Type::IMAGE;
		bCI.srcAccess = Barrier::AccessBit::NONE_BIT;
		bCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		bCI.image = image;
		bCI.oldLayout = Image::Layout::UNKNOWN;
		bCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
		BarrierRef b = Barrier::Create(&bCI);
		cmdCopyBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, { b });
		cmdCopyBuffer->CopyBufferToImage(0, c_imageBuffer, image, Image::Layout::TRANSFER_DST_OPTIMAL, {
			{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 0, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}},
			{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 1, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}},
			{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 2, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}},
			{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 3, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}},
			{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 4, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}},
			{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 5, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}}
			});

		cmdCopyBuffer->End(0);
	}
	CommandBuffer::SubmitInfo copySI = { { 0 }, {}, {}, {}, { transferSemaphore }, {} };
	cmdCopyBuffer->Submit({ copySI }, nullptr);
	{
		cmdBuffer->Begin(3, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		Barrier::CreateInfo bCI;
		bCI.type = Barrier::Type::IMAGE;
		bCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		bCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		bCI.image = image;
		bCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		bCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
		BarrierRef b = Barrier::Create(&bCI);
		cmdBuffer->PipelineBarrier(3, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::FRAGMENT_SHADER_BIT, DependencyBit::NONE_BIT, { b });

		cmdBuffer->End(3);
	}
	CommandBuffer::SubmitInfo copy2SI = { { 3 }, { transferSemaphore }, {}, { PipelineStageBit::TRANSFER_BIT }, {}, {} };
	cmdBuffer->Submit({ copy2SI }, transferFence);
	transferFence->Wait();

	//Basic shader
	Shader::CreateInfo shaderCI;
	shaderCI.debugName = "Basic: Vertex Shader Module";
	shaderCI.device = context->GetDevice();
	shaderCI.stageAndEntryPoints = { {Shader::StageBit::VERTEX_BIT, "vs_main"} };
	shaderCI.binaryFilepath = "res/bin/basic_vs_6_1_vs_main.spv";
	shaderCI.binaryCode = {};
	shaderCI.recompileArguments = {
		"res/shaders/basic.hlsl",
		"res/bin",
		{"../../MIRU/MIRU_SHADER_COMPILER/shaders/includes"},
		"vs_main",
		"vs_6_1",
		{},
		true,
		true,
		{"-Zi", "-Od", "-Fd"},
		""
	};
	ShaderRef vertexShader = Shader::Create(&shaderCI);
	shaderCI.debugName = "Basic: Fragment Shader Module";
	shaderCI.stageAndEntryPoints = { { Shader::StageBit::PIXEL_BIT, "ps_main"} };
	shaderCI.binaryFilepath = "res/bin/basic_ps_6_1_ps_main.spv";
	shaderCI.recompileArguments.entryPoint = "ps_main";
	shaderCI.recompileArguments.shaderModel = "ps_6_1";
	ShaderRef fragmentShader = Shader::Create(&shaderCI);

	//Basic and Pipeline Descriptor sets
	DescriptorPool::CreateInfo descriptorPoolCI;
	descriptorPoolCI.debugName = "Basic: Descriptor Pool";
	descriptorPoolCI.device = context->GetDevice();
	descriptorPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, 1}, {DescriptorType::UNIFORM_BUFFER, 2} };
	descriptorPoolCI.maxSets = 3;
	DescriptorPoolRef descriptorPool = DescriptorPool::Create(&descriptorPoolCI);
	DescriptorSetLayout::CreateInfo setLayoutCI;
	setLayoutCI.debugName = "Basic: DescSetLayout1";
	setLayoutCI.device = context->GetDevice();
	setLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT } };
	DescriptorSetLayoutRef setLayout1 = DescriptorSetLayout::Create(&setLayoutCI);
	setLayoutCI.debugName = "Basic: DescSetLayout2";
	setLayoutCI.descriptorSetLayoutBinding = {
		{0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT },
		{1, DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shader::StageBit::FRAGMENT_BIT }
	};
	DescriptorSetLayoutRef setLayout2 = DescriptorSetLayout::Create(&setLayoutCI);
	DescriptorSet::CreateInfo descriptorSetCI;
	descriptorSetCI.debugName = "Basic: Descriptor Set 0";
	descriptorSetCI.descriptorPool = descriptorPool;
	descriptorSetCI.descriptorSetLayouts = { setLayout1 };
	DescriptorSetRef descriptorSet_p0 = DescriptorSet::Create(&descriptorSetCI);
	descriptorSetCI.debugName = "Basic: Descriptor Set 1";
	descriptorSetCI.descriptorSetLayouts = { setLayout2 };
	DescriptorSetRef descriptorSet_p1 = DescriptorSet::Create(&descriptorSetCI);
	descriptorSet_p0->AddBuffer(0, 0, { { ubViewCam } });
	descriptorSet_p1->AddBuffer(0, 0, { { ubViewMdl } });
	descriptorSet_p1->AddImage(0, 1, { { sampler, imageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
	descriptorSet_p0->Update();
	descriptorSet_p1->Update();

	//Main RenderPass
	RenderPass::CreateInfo renderPassCI;
	renderPassCI.debugName = "Basic: RenderPass";
	renderPassCI.device = context->GetDevice();
	renderPassCI.attachments = {
		{swapchainCI.format,						//Colour
		Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
		RenderPass::AttachmentLoadOp::CLEAR,
		RenderPass::AttachmentStoreOp::STORE,
		RenderPass::AttachmentLoadOp::DONT_CARE,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		Image::Layout::UNKNOWN,
		Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
		},
		{depthImage->GetCreateInfo().format,		//Depth
		Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
		RenderPass::AttachmentLoadOp::CLEAR,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		RenderPass::AttachmentLoadOp::DONT_CARE,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		Image::Layout::UNKNOWN,
		Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}
	};
	renderPassCI.subpassDescriptions = {
		{PipelineType::GRAPHICS, {}, {{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {}, {{1, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}}, {} },
	};
	renderPassCI.subpassDependencies = {
		{RenderPass::SubpassExternal, 0,
		PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT,
		(Barrier::AccessBit)0, Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT | Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT, DependencyBit::NONE_BIT}
	};
	renderPassCI.multiview = { { 0b11 }, {}, { 0b11 } };
	RenderPassRef renderPass = RenderPass::Create(&renderPassCI);

	//Basic pipeline
	Pipeline::CreateInfo pCI;
	pCI.debugName = "Basic";
	pCI.device = context->GetDevice();
	pCI.type = PipelineType::GRAPHICS;
	pCI.shaders = { vertexShader, fragmentShader };
	pCI.vertexInputState.vertexInputBindingDescriptions = { {0, sizeof(vertices) / 8, VertexInputRate::VERTEX} };
	pCI.vertexInputState.vertexInputAttributeDescriptions = { {0, 0, VertexType::VEC4, 0, "POSITION"} };
	pCI.inputAssemblyState = { PrimitiveTopology::TRIANGLE_LIST, false };
	pCI.tessellationState = {};
	pCI.viewportState.viewports = { {} }; //Specify number of viewports to be used.
	pCI.viewportState.scissors = { {} }; //Specify number of scissors to be used.
	pCI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::BACK_BIT, FrontFace::CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f };
	pCI.multisampleState = { Image::SampleCountBit::SAMPLE_COUNT_1_BIT, false, 1.0f, UINT32_MAX, false, false };
	pCI.depthStencilState = { true, true, CompareOp::GREATER, false, false, {}, {}, 0.0f, 1.0f };
	pCI.colourBlendState.logicOpEnable = false;
	pCI.colourBlendState.logicOp = LogicOp::COPY;
	pCI.colourBlendState.attachments = { {true, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendOp::ADD,
											BlendFactor::ONE, BlendFactor::ZERO, BlendOp::ADD, (ColourComponentBit)15 } };
	pCI.colourBlendState.blendConstants[0] = 0.0f;
	pCI.colourBlendState.blendConstants[1] = 0.0f;
	pCI.colourBlendState.blendConstants[2] = 0.0f;
	pCI.colourBlendState.blendConstants[3] = 0.0f;
	pCI.dynamicStates = { { DynamicState::VIEWPORT, DynamicState::SCISSOR } }; //Specify dynamics state.
	pCI.layout = { {setLayout1, setLayout2 }, {} };
	pCI.renderPass = nullptr;
	pCI.dynamicRendering = { 0b11, { swapchainCI.format}, depthImageCI.format, Image::Format::UNKNOWN };
	PipelineRef pipeline = Pipeline::Create(&pCI);

	Fence::CreateInfo fenceCI;
	fenceCI.debugName = "DrawFence";
	fenceCI.device = context->GetDevice();
	fenceCI.signaled = true;
	fenceCI.timeout = UINT64_MAX;
	std::vector<FenceRef> draws;
	for (uint32_t i = 0; i < swapchain->GetImageCount(); i++)
		draws.push_back(Fence::Create(&fenceCI));

	uint32_t frameNumber = 0;
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

		if (session->IsRunning())
		{
			session->WaitFrame();
			session->BeginFrame();

			std::vector<CompositionLayer::BaseHeader*> layers;
			CompositionLayer::Projection layerProjection;

			if (session->IsActive() && session->m_FrameState.shouldRender)
			{
				std::vector<View> views;
				if (session->LocateViews(referenceSpace, views))
				{
					const uint32_t& width = viewConfigurations->m_Views[viewConfigurationType][0].recommendedImageRectWidth;
					const uint32_t& height = viewConfigurations->m_Views[viewConfigurationType][0].recommendedImageRectHeight;

					layerProjection.type = CompositionLayer::Type::PROJECTION;
					layerProjection.layerFlags = CompositionLayer::Flags::CORRECT_CHROMATIC_ABERRATION_BIT | CompositionLayer::Flags::BLEND_TEXTURE_SOURCE_ALPHA_BIT;
					layerProjection.space = referenceSpace;
					layerProjection.views.resize(views.size());
					size_t i = 0;
					for (const auto& view : views)
					{
						layerProjection.views[i].view = views[i];
						layerProjection.views[i].swapchainSubImage.swapchain = swapchain;
						layerProjection.views[i].swapchainSubImage.imageRect.offset.x = 0;
						layerProjection.views[i].swapchainSubImage.imageRect.offset.y = 0;
						layerProjection.views[i].swapchainSubImage.imageRect.extent.width = static_cast<int32_t>(width);
						layerProjection.views[i].swapchainSubImage.imageRect.extent.height = static_cast<int32_t>(height);
						layerProjection.views[i].swapchainSubImage.imageArrayIndex = static_cast<uint32_t>(i);
						i++;
					}

					swapchain->Acquire();
					swapchain->Wait(XR_INFINITE_DURATION);
					uint32_t imageIndex = swapchain->GetImageIndex();

					//Render Stuff
					{
						draws[imageIndex]->Wait();
						draws[imageIndex]->Reset();

						cmdBuffer->Reset(imageIndex, false);
						cmdBuffer->Begin(imageIndex, CommandBuffer::UsageBit::SIMULTANEOUS);

						RenderingAttachmentInfo colourRAI = { 
							swapchainImageViews[imageIndex], Image::Layout::COLOUR_ATTACHMENT_OPTIMAL,
							ResolveModeBits::NONE_BIT, nullptr, Image::Layout::UNKNOWN,
							RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE,
							{1.0f, 0.0f, 0.0f, 1.0f} };
						RenderingAttachmentInfo depthRAI = { 
							depthImageView, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
							ResolveModeBits::NONE_BIT, nullptr, Image::Layout::UNKNOWN,
							RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::DONT_CARE,
							{0.0f, 0} };

						RenderingInfo renderInfo;
						renderInfo.flags = RenderingFlagBits::NONE_BIT;
						renderInfo.renderArea = { {0, 0}, {width, height} };
						renderInfo.layerCount = 1;
						renderInfo.viewMask = 0b11;
						renderInfo.colourAttachments = { colourRAI };
						renderInfo.pDepthAttachment = &depthRAI;
						renderInfo.pStencilAttachment = nullptr;
						cmdBuffer->BeginRendering(imageIndex, renderInfo);

						cmdBuffer->BindPipeline(imageIndex, pipeline);
						cmdBuffer->SetViewport(imageIndex, { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} });
						cmdBuffer->SetScissor(imageIndex, { {{(int32_t)0, (int32_t)0}, {width, height}} });
						cmdBuffer->BindDescriptorSets(imageIndex, { descriptorSet_p0 }, 0, pipeline);
						cmdBuffer->BindDescriptorSets(imageIndex, { descriptorSet_p1 }, 1, pipeline);
						cmdBuffer->BindVertexBuffers(imageIndex, { vbv });
						cmdBuffer->BindIndexBuffer(imageIndex, ibv);
						cmdBuffer->DrawIndexed(imageIndex, 36);

						cmdBuffer->EndRendering(imageIndex);
						cmdBuffer->End(imageIndex);

						CommandBuffer::SubmitInfo mainSI = { { imageIndex }, {}, {}, {}, {}, {} };
						cmdBuffer->Submit({ mainSI }, draws[imageIndex]);

						float fov = abs(views[0].fov.angleLeft) + abs(views[0].fov.angleRight);
						proj = mars::float4x4::Perspective(fov, float(width) / float(height), 0.1f, 100.0f);
						if (GraphicsAPI::IsD3D12())
							proj.f *= -1;
						views[0].pose.orientation.k *= -1;
						views[1].pose.orientation.k *= -1;
						view[0] = mars::float4x4::Inverse(/*views[0].pose.orientation.ToRotationMatrix4<float>() **/ mars::float4x4::Translation(views[0].pose.position));
						view[1] = mars::float4x4::Inverse(/*views[1].pose.orientation.ToRotationMatrix4<float>() **/ mars::float4x4::Translation(views[0].pose.position));
						modl = mars::float4x4::Translation({ 0.0f, 0.0f, +3.0f })
							//* translate(mat4(1.0f), { float(var_x)/10.0f, float(var_y)/10.0f, float(var_z)/10.0f})
							* mars::float4x4::Rotation((frameNumber * 1.0f) * 3.14159 / 180.0, { 0, 1, 0 })
							* mars::float4x4::Rotation((frameNumber * 1.0f) * 3.14159 / 180.0, { 1, 0, 0 })
							* mars::float4x4::Rotation((frameNumber * 1.0f) * 3.14159 / 180.0, { 0, 0, 1 });

						memcpy(ubData + 0 * 16, proj.GetData(), sizeof(proj));
						memcpy(ubData + 1 * 16, view[0].GetData(), sizeof(view));

						cpu_alloc_0->SubmitData(ub1->GetAllocation(), 3 * sizeof(mars::float4x4), ubData);
						cpu_alloc_0->SubmitData(ub2->GetAllocation(), sizeof(mars::float4x4), (void*)modl.GetData());

					}

					swapchain->Release();

					layers.push_back(reinterpret_cast<CompositionLayer::BaseHeader*>(&layerProjection));
				}
			}

			session->EndFrame(layers);
			frameNumber++;
		}
	}
}