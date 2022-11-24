#include "VulkanApp.h"

#include <string>
#include <iostream>
#include <optional>
#include <vector>

#include <DirectXMath.h>

#include "VulkanUtil.h"

using namespace DirectX;

namespace
{
	// GLFW error callback that just throws, reporting the error
	void GLFWErrorCallback(int error, const char* text)
	{
		throw std::runtime_error("Failed with GLFW error " + std::to_string(error) + ": " + text);
	}

	// Checks whether the Vulkan instance supports the required extensions
	void VerifyInstanceExtensionSupport(const std::vector<const char*>& requiredExtensions)
	{
		auto supportedExtensions = vk::enumerateInstanceExtensionProperties();

		for (const auto& requiredExtension : requiredExtensions)
		{
			bool found = false;
			for (const auto& supportedExtension : supportedExtensions)
			{
				if (strcmp(supportedExtension.extensionName, requiredExtension) == 0)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				std::cerr << "Required instance extension " << requiredExtension << " is not available\n";
			}
		}
	}

	// Checks whether the GPU supports requested extensions
	void VerifyDeviceExtensionSupport(const vk::PhysicalDevice& physicalDevice,
		const std::vector<const char*>& requiredExtensions)
	{
		auto supportedExtensions = physicalDevice.enumerateDeviceExtensionProperties();

		for (const auto& requiredExtension : requiredExtensions)
		{
			bool found = false;
			for (const auto& supportedExtension : supportedExtensions)
			{
				if (strcmp(supportedExtension.extensionName, requiredExtension) == 0)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				std::cerr << "Required device extension " << requiredExtension << " is not available\n";
			}
		}
	}

	// Iterate through Vulkan optional feature structs
	bool FeatureSupportHelper(const vk::Bool32* pRequired, const vk::Bool32* pSupported, int nFields)
	{
		for (int i = 0; i < nFields; i++)
		{
			if ((*pRequired) && !(*pSupported))
			{
				return false;
			}
			pRequired++;
			pSupported++;
		}
		return true;
	}

	bool FeatureSupportHelper(const vk::PhysicalDeviceFeatures& required, const vk::PhysicalDeviceFeatures& supported)
	{
		return FeatureSupportHelper(&required.robustBufferAccess, &supported.robustBufferAccess, 55);
	}

	bool FeatureSupportHelper(const vk::PhysicalDeviceVulkan11Features& required,
		const vk::PhysicalDeviceVulkan11Features& supported)
	{
		return FeatureSupportHelper(&required.storageBuffer16BitAccess, &supported.storageBuffer16BitAccess, 12);
	}

	bool FeatureSupportHelper(const vk::PhysicalDeviceVulkan12Features& required,
		const vk::PhysicalDeviceVulkan12Features& supported)
	{
		return FeatureSupportHelper(&required.samplerMirrorClampToEdge, &supported.samplerMirrorClampToEdge, 47);
	}

	bool FeatureSupportHelper(const vk::PhysicalDeviceVulkan13Features& required,
		const vk::PhysicalDeviceVulkan13Features& supported)
	{
		return FeatureSupportHelper(&required.robustImageAccess, &supported.robustImageAccess, 15);
	}

	// Checks whether the GPU supports requested optional Vulkan features
	void VerifyDeviceFeatureSupport(const vk::PhysicalDevice& physicalDevice,
		const vk::PhysicalDeviceFeatures& required10Features,
		const vk::PhysicalDeviceVulkan11Features& required11Features,
		const vk::PhysicalDeviceVulkan12Features& required12Features,
		const vk::PhysicalDeviceVulkan13Features& required13Features)
	{
		auto supportedFeatures = physicalDevice.getFeatures2<vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan12Features,
			vk::PhysicalDeviceVulkan13Features>();

		auto& supported10Features = supportedFeatures.get<vk::PhysicalDeviceFeatures2>().features;
		auto& supported11Features = supportedFeatures.get<vk::PhysicalDeviceVulkan11Features>();
		auto& supported12Features = supportedFeatures.get<vk::PhysicalDeviceVulkan12Features>();
		auto& supported13Features = supportedFeatures.get<vk::PhysicalDeviceVulkan13Features>();

		bool vk10Support = FeatureSupportHelper(required10Features, supported10Features);
		bool vk11Support = FeatureSupportHelper(required11Features, supported11Features);
		bool vk12Support = FeatureSupportHelper(required12Features, supported12Features);
		bool vk13Support = FeatureSupportHelper(required13Features, supported13Features);

		if (!(vk10Support && vk11Support && vk12Support && vk13Support))
		{
			throw std::runtime_error("Required Vulkan optional features are not available");
		}
	}

	constexpr std::array<VertexP3N3, 36> cubeVertices = {
		VertexP3N3{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(-0.0f, 0.0f, 1.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-0.0f, 0.0f, 1.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-0.0f, 0.0f, 1.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -0.0f, -0.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, -0.0f, -0.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, -0.0f, -0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(-0.0f, 0.0f, 1.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(-0.0f, 0.0f, 1.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-0.0f, 0.0f, 1.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -0.0f, -0.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -0.0f, -0.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, -0.0f, -0.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		VertexP3N3{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		VertexP3N3{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
	};

	constexpr std::array<uint32_t, 36> cubeIndices = {
		1, 0, 2,
		4, 3, 5,
		7, 6, 8,
		10, 9, 11,
		13, 12, 14,
		16, 15, 17,
		19, 18, 20,
		22, 21, 23,
		25, 24, 26,
		28, 27, 29,
		31, 30, 32,
		34, 33, 35,
	};
}

VulkanApp::VulkanApp() : 
	m_gfxQueueIdx(0),
	m_frameCount(0),
	m_backBufferFormat(vk::Format::eB8G8R8A8Srgb),
	m_depthBufferFormat(vk::Format::eD32SfloatS8Uint),
	m_aspectRatio(0.0f),
	m_window(640, 480),
	m_sizeChanged(false),
	m_vertexBuffer(0)
{
	m_window.OnTick.Register(this, &VulkanApp::Tick);
	m_window.OnResize.Register(this, &VulkanApp::OnResize);

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

VulkanApp::~VulkanApp()
{
	// Allow all rendering operations to finish
	m_device->waitIdle();
}

void VulkanApp::Run()
{
	m_window.MainLoop();
}

void VulkanApp::OnResize(int width, int height)
{
	m_sizeChanged = true;
}

void VulkanApp::Tick()
{
	Update();
	Render();
	m_frameCount++;
	m_resourceManager.IncrementFrameCount();
}

void VulkanApp::Update()
{
	double t = GetTime();

	// Recommended usage pattern for mapped memory is to create a local variable, then memcpy
	GlobalConstants globalConstants{};
	
	auto eye = XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
	auto focus = XMVectorZero();
	auto up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	auto view = XMMatrixLookAtRH(eye, focus, up);
	auto proj = XMMatrixPerspectiveFovRH(XMConvertToRadians(80.0f), m_aspectRatio, 0.1f, 100.0f);
	auto model = XMMatrixTranslation(0.0f, -0.1f, 0.0f) * XMMatrixScaling(25.0f, 25.0f, 25.0f)
		* XMMatrixRotationRollPitchYaw(0.0f, fmod(t, 2.0f * XM_PI), 0.0f);
	auto viewProj = model * view * proj;
	XMStoreFloat4x4(&globalConstants.viewProj, XMMatrixTranspose(viewProj));
	
	void* ptr = m_resourceManager.GetConstants();
	memcpy(ptr, &globalConstants, sizeof(globalConstants));
}

void VulkanApp::Render()
{
	// Check if window needs resizing
	if (m_sizeChanged)
	{
		m_device->waitIdle();

		CreateWindowSizeDependentResources();
		m_sizeChanged = false;
	}

	// Select the frame based on the current frame count
	auto frameIdx = m_frameCount % BACK_BUFFER_COUNT;
	auto& frame = m_frames[frameIdx];

	m_device->waitForFences(*frame.renderFence, true, UINT64_MAX);
	auto [result, swapchainImageIdx] = m_device->acquireNextImageKHR(*m_swapchain, UINT64_MAX, *frame.imageReadySemaphore);
	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
	{
		m_sizeChanged = true;
	}
	m_device->resetFences(*frame.renderFence);

	auto& image = m_swapchainImages[swapchainImageIdx];
	auto& imageView = m_swapchainImageViews[swapchainImageIdx];

	// Record command buffer
	{
		frame.commandBuffer->reset();
		vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		frame.commandBuffer->begin(vk::CommandBufferBeginInfo());

		// Transition the swapchain image
		InsertImageMemoryBarrier(*frame.commandBuffer, image,
			vk::AccessFlagBits::eNone, 
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::ImageLayout::eUndefined, 
			vk::ImageLayout::eAttachmentOptimal,
			vk::PipelineStageFlagBits::eNone, 
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
		// Ensure all previous writes to the depth buffer have completed
		InsertImageMemoryBarrier(*frame.commandBuffer, m_depthBuffer.GetImage(),
			vk::AccessFlagBits::eNone,
			vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::ImageLayout::eAttachmentOptimal,
			vk::ImageLayout::eAttachmentOptimal,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1));

		// Clear the backbuffer
		constexpr std::array<float, 4> clearColor = { 0.95f, 0.77f, 0.33f, 0.0f };
		constexpr vk::ClearValue colorClearValue{ vk::ClearColorValue(clearColor) };
		constexpr vk::ClearValue depthClearValue{ vk::ClearDepthStencilValue(1.0f, 0) };
		vk::RenderingAttachmentInfo colorAttachmentInfo(*m_colorBufferView,
			vk::ImageLayout::eAttachmentOptimal,
			vk::ResolveModeFlagBits::eAverage,
			*imageView,
			vk::ImageLayout::eAttachmentOptimal,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			colorClearValue);
		vk::RenderingAttachmentInfo depthAttachmentInfo(*m_depthBufferView,
			vk::ImageLayout::eAttachmentOptimal,
			{}, {}, {},
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			depthClearValue);
		vk::RenderingInfo renderInfo({}, m_screenScissor, 1, 0, colorAttachmentInfo,
			&depthAttachmentInfo, &depthAttachmentInfo);

		frame.commandBuffer->beginRendering(renderInfo);

		// Bind the pipeline
		frame.commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
		// Set dynamic state that we didn't specify in our pipeline
		frame.commandBuffer->setViewport(0, m_screenViewport);
		frame.commandBuffer->setScissor(0, m_screenScissor);

		// Bind descriptor sets from our Resource Manager
		frame.commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 
			0, m_resourceManager.GetDescriptorSet(), {});
		// Bind the index buffer from the Resource Manager
		frame.commandBuffer->bindIndexBuffer(m_resourceManager.GetIndexBuffer(), 0, vk::IndexType::eUint32);

		frame.commandBuffer->drawIndexed(m_mesh.indices.size(), 1, 0, 0, 0);

		frame.commandBuffer->endRendering();

		InsertImageMemoryBarrier(*frame.commandBuffer, image,
			vk::AccessFlagBits::eColorAttachmentWrite, 
			vk::AccessFlagBits::eNone,
			vk::ImageLayout::eColorAttachmentOptimal, 
			vk::ImageLayout::ePresentSrcKHR,
			vk::PipelineStageFlagBits::eColorAttachmentOutput, 
			vk::PipelineStageFlagBits::eNone,
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

		frame.commandBuffer->end();
	}

	// Submit the frame
	{
		vk::SemaphoreSubmitInfo waitSemaphoreInfo(*frame.imageReadySemaphore,
			0, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
		vk::CommandBufferSubmitInfo commandBufferInfo(*frame.commandBuffer);
		vk::SemaphoreSubmitInfo signalSemaphoreInfo(*frame.renderSemaphore,
			0, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
		vk::SubmitInfo2 submitInfo({}, waitSemaphoreInfo, commandBufferInfo, signalSemaphoreInfo);
		m_gfxQueue.submit2(submitInfo, *frame.renderFence);
	}

	// Present
	{
		vk::PresentInfoKHR presentInfo(*frame.renderSemaphore, *m_swapchain, swapchainImageIdx);
		auto result = m_gfxQueue.presentKHR(presentInfo);
		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
		{
			m_sizeChanged = true;
		}
	}
}

void VulkanApp::CreateDeviceDependentResources()
{
	using namespace std::string_literals;

	std::vector<const char*> enabledLayers;
#ifdef _DEBUG
	enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

	// Create Vulkan resources
	CreateInstance(enabledLayers);
	CreateSurface();
	CreatePhysicalDevice();
	CreateDevice(enabledLayers);

	// Create the allocator
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = m_physicalDevice;
	allocatorInfo.device = *m_device;
	allocatorInfo.instance = *m_instance;
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
	m_allocator = UniqueAllocator(allocatorInfo);
	m_resourceManager = ResourceManager(*m_device, *m_allocator, m_gfxQueue, m_gfxQueueIdx);

	// Create per-frame resources so we can use double buffering
	for (auto& frame : m_frames)
	{
		frame = FrameResources(*m_device, m_gfxQueueIdx);
	}

	m_mesh = LoadModel(ASSET_PATH + "/bunny.obj"s)[0];
	m_vertexBuffer = m_resourceManager.CreateVertices(*m_device, m_mesh.vertices.data(),
		m_mesh.vertices.size() * sizeof(m_mesh.vertices[0]));
	m_indexBuffer = m_resourceManager.CreateIndices(*m_device, m_mesh.indices.data(),
		m_mesh.indices.size() * sizeof(m_mesh.indices[0]));

	// Create the pipeline layout and pipeline
	// This will be removed later when the engine becomes dynamic
	vk::DescriptorSetLayout descriptorSetLayout = m_resourceManager.GetDescriptorSetLayout();
	vk::PipelineLayoutCreateInfo layoutInfo({}, descriptorSetLayout);
	m_pipelineLayout = m_device->createPipelineLayoutUnique(layoutInfo);

	// Input assembly state
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo({}, vk::PrimitiveTopology::eTriangleList);

	// Rasterization state
	vk::PipelineRasterizationStateCreateInfo rasterizerInfo;
	rasterizerInfo.lineWidth = 1.0f;
	rasterizerInfo.cullMode = vk::CullModeFlagBits::eBack;
	rasterizerInfo.frontFace = vk::FrontFace::eClockwise;

	vk::PipelineMultisampleStateCreateInfo multisampleInfo({}, vk::SampleCountFlagBits::e8);

	// Color blend state
	vk::PipelineColorBlendAttachmentState blendInfo;
	blendInfo.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
		| vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

	// Depth stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencilInfo({}, true, true, vk::CompareOp::eLess, true);
	depthStencilInfo.minDepthBounds = 0.0f;
	depthStencilInfo.maxDepthBounds = 1.0f;

	PipelineBuilder builder;
	builder.SetInputAssemblyState(inputAssemblyInfo);
	builder.SetVertexInputState(vk::PipelineVertexInputStateCreateInfo());
	builder.SetRasterizerState(rasterizerInfo);
	builder.SetMultisampleState(multisampleInfo);
	builder.SetDepthStencilState(depthStencilInfo);
	builder.AddColorAttachment(blendInfo, m_backBufferFormat);
	builder.SetDepthAttachment(m_depthBufferFormat);
	builder.SetStencilAttachment(m_depthBufferFormat);

	// Create shaders for our triangle. According to the spec, you don't need to keep the vkShaderModules
	// around after creating the pipeline, so they are not stored in the main class.
	auto vertexShader = CreateShader(*m_device, SHADER_PATH + "/TriangleVS.spv"s);
	builder.AddShaderStage(vk::ShaderStageFlagBits::eVertex, *vertexShader);
	auto pixelShader = CreateShader(*m_device, SHADER_PATH + "/TrianglePS.spv"s);
	builder.AddShaderStage(vk::ShaderStageFlagBits::eFragment, *pixelShader);
	m_pipeline = builder.CreatePipeline(*m_device, *m_pipelineLayout);
}

void VulkanApp::CreateWindowSizeDependentResources()
{
	auto [width, height] = m_window.GetSize();

	m_aspectRatio = 1.0f * width / height;
	m_backBufferExtent = vk::Extent2D(width, height);
	m_screenViewport = vk::Viewport(0.0f, height, width, -height, 0.0f, 1.0f);
	m_screenScissor = vk::Rect2D(vk::Offset2D(), vk::Extent2D(width, height));

	CreateSwapchain();

	// Create color buffer
	AllocationCreateInfo allocationInfo({}, VMA_MEMORY_USAGE_AUTO);
	vk::ImageCreateInfo colorInfo({}, vk::ImageType::e2D, m_backBufferFormat, vk::Extent3D(m_backBufferExtent, 1),
		1, 1, vk::SampleCountFlagBits::e8, vk::ImageTiling::eOptimal, 
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
		vk::SharingMode::eExclusive, {}, vk::ImageLayout::eUndefined);
	m_colorBuffer = UniqueAllocatedImage(*m_allocator, colorInfo, allocationInfo);
	vk::ImageViewCreateInfo colorViewInfo({}, m_colorBuffer.GetImage(), vk::ImageViewType::e2D, m_backBufferFormat,
		{}, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
	m_colorBufferView = m_device->createImageViewUnique(colorViewInfo);

	// Create depth buffer
	vk::ImageCreateInfo depthInfo({}, vk::ImageType::e2D, m_depthBufferFormat, vk::Extent3D(m_backBufferExtent, 1),
		1, 1, vk::SampleCountFlagBits::e8, vk::ImageTiling::eOptimal, 
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::SharingMode::eExclusive, {}, vk::ImageLayout::eUndefined);
	m_depthBuffer = UniqueAllocatedImage(*m_allocator, depthInfo, allocationInfo);
	vk::ImageViewCreateInfo depthViewInfo({}, m_depthBuffer.GetImage(), vk::ImageViewType::e2D, m_depthBufferFormat,
		{}, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1));
	m_depthBufferView = m_device->createImageViewUnique(depthViewInfo);
}

void VulkanApp::CreateInstance(std::vector<const char*>& enabledLayers)
{
	vk::ApplicationInfo appInfo("Vulkan App", 0, nullptr, 0, VK_API_VERSION_1_3);

	std::vector<const char*> requiredInstanceExtensions = {
			"VK_KHR_surface",
			"VK_KHR_win32_surface"
	};
	VerifyInstanceExtensionSupport(requiredInstanceExtensions);

	// Request validation layers to be enabled in debug builds
	vk::InstanceCreateInfo instanceInfo({}, &appInfo, enabledLayers, requiredInstanceExtensions);
	m_instance = vk::createInstanceUnique(instanceInfo);
}

void VulkanApp::CreateSurface()
{
	HINSTANCE hInstance = GetModuleHandleW(nullptr);
	HWND hWnd = m_window.GetHWnd();
	vk::Win32SurfaceCreateInfoKHR surfaceInfo({}, hInstance, hWnd);
	m_surface = m_instance->createWin32SurfaceKHRUnique(surfaceInfo);
}

void VulkanApp::CreatePhysicalDevice()
{
	// Select the first physical device for now
	m_physicalDevice = m_instance->enumeratePhysicalDevices()[0];
	auto properties = m_physicalDevice.getProperties();
	std::string deviceType;
	switch (properties.deviceType)
	{
	case vk::PhysicalDeviceType::eDiscreteGpu:
		deviceType = "Discrete GPU";
		break;
	case vk::PhysicalDeviceType::eIntegratedGpu:
		deviceType = "Integrated GPU";
		break;
	default:
		deviceType = "Other";
		break;
	}
	// Report what GPU we chose and its type
	std::cout << "Choosing GPU: " << properties.deviceName << " (" << deviceType << ")\n";

	// Retrieve the graphics queue index
	std::optional<uint32_t> gfxQueueIdx;
	auto queueFamilies = m_physicalDevice.getQueueFamilyProperties();
	for (uint32_t i = 0; i < queueFamilies.size(); i++)
	{
		// Query for graphics and presentation support
		if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics
			&& m_physicalDevice.getSurfaceSupportKHR(i, *m_surface))
		{
			gfxQueueIdx = i;
			break;
		}
	}
	if (!gfxQueueIdx)
	{
		throw std::runtime_error("No graphics queue available");
	}
	m_gfxQueueIdx = *gfxQueueIdx;
}

void VulkanApp::CreateDevice(std::vector<const char*>& enabledLayers)
{
	// Ensure that the GPU supports required features. Insert additional features here as needed.
	vk::PhysicalDeviceFeatures required10Features;
	vk::PhysicalDeviceVulkan11Features required11Features;
	vk::PhysicalDeviceVulkan12Features required12Features;
	vk::PhysicalDeviceVulkan13Features required13Features;
	// Enable the features we require for the app
	required10Features.samplerAnisotropy = true;
	required12Features.scalarBlockLayout = true;
	required12Features.descriptorBindingPartiallyBound = true;
	required12Features.descriptorIndexing = true;
	required13Features.dynamicRendering = true;
	required13Features.synchronization2 = true;
	// Check the physical device supports required features
	VerifyDeviceFeatureSupport(m_physicalDevice, required10Features, required11Features,
		required12Features, required13Features);
	// Create a structure chain with the required features
	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features> requiredFeatures(
			vk::PhysicalDeviceFeatures2(required10Features),
			required11Features,
			required12Features,
			required13Features
		);
	// Extensions we will require (separate from core Vulkan optional features)
	std::vector<const char*> requiredExtensions = {
			"VK_KHR_swapchain"
	};
	VerifyDeviceExtensionSupport(m_physicalDevice, requiredExtensions);

	// Now create the logical device
	float queuePriority = 1.0f;
	vk::DeviceQueueCreateInfo queueInfo({}, m_gfxQueueIdx, 1, &queuePriority);
	vk::StructureChain<vk::DeviceCreateInfo, vk::PhysicalDeviceFeatures2> deviceInfo(
		vk::DeviceCreateInfo({}, queueInfo, enabledLayers, requiredExtensions),
		requiredFeatures.get<vk::PhysicalDeviceFeatures2>()
	);
	m_device = m_physicalDevice.createDeviceUnique(deviceInfo.get<vk::DeviceCreateInfo>());

	// Retrieve the device's graphics queue
	m_gfxQueue = m_device->getQueue(m_gfxQueueIdx, 0);
}

void VulkanApp::CreateSwapchain()
{
	// Create the swapchain itself
	vk::SwapchainCreateInfoKHR swapchainInfo({}, *m_surface, 2, m_backBufferFormat,
		vk::ColorSpaceKHR::eSrgbNonlinear, m_backBufferExtent, 1,
		vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, {},
		vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque,
		vk::PresentModeKHR::eFifo);
	// Passing in the old swapchain can improve speed by reusing the resources
	swapchainInfo.oldSwapchain = *m_swapchain;
	m_swapchain = m_device->createSwapchainKHRUnique(swapchainInfo);
	m_swapchainImages = m_device->getSwapchainImagesKHR(*m_swapchain);

	// Create image views for the swapchain, clearing the previous images first
	m_swapchainImageViews.clear();
	for (const auto& image : m_swapchainImages)
	{
		vk::ImageViewCreateInfo imageViewInfo({}, image, vk::ImageViewType::e2D, m_backBufferFormat,
			{}, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
		m_swapchainImageViews.push_back(m_device->createImageViewUnique(imageViewInfo));
	}
}

VulkanApp::FrameResources::FrameResources(const vk::Device& device, uint32_t gfxQueueIdx)
{
	// Create the command pool
	vk::CommandPoolCreateInfo poolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, gfxQueueIdx);
	commandPool = device.createCommandPoolUnique(poolInfo);

	// Create the command buffer
	vk::CommandBufferAllocateInfo bufferInfo(*commandPool, vk::CommandBufferLevel::ePrimary, 1);
	commandBuffer = std::move(device.allocateCommandBuffersUnique(bufferInfo)[0]);

	// Create semaphores
	vk::SemaphoreCreateInfo semaphoreInfo;
	imageReadySemaphore = device.createSemaphoreUnique(semaphoreInfo);
	renderSemaphore = device.createSemaphoreUnique(semaphoreInfo);

	// Create fence
	vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);
	renderFence = device.createFenceUnique(fenceInfo);
}
