#pragma once

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#include "Resources.h"
#include "Window.h"
#include "ModelLoading.h"

class VulkanApp
{
public:
	VulkanApp();
	~VulkanApp();

	void Run();

private:
	// Structures that are duplicated for each frame for double buffering purposes
	struct FrameResources
	{
		// Default constructor because we can't initialize the FrameData without the vkDevice
		FrameResources() = default;

		// Constructor just for ease of use
		FrameResources(const vk::Device& device, uint32_t gfxQueueIdx);

		// Signaled when the next swapchain image is ready to start rendering
		vk::UniqueSemaphore imageReadySemaphore;
		// Signaled when rendering is finished on the GPU and the frame can be presented
		vk::UniqueSemaphore renderSemaphore;
		// Same thing but for GPU-CPU synchronization
		vk::UniqueFence renderFence;
		vk::UniqueCommandPool commandPool;
		vk::UniqueCommandBuffer commandBuffer;
	};

	// CPU code, updating uniforms, organizing scene data etc.
	void Update();
	// Rendering logic
	void Render();
	void Tick();
	void OnResize(int width, int height);

	// Functions to separate resource creation
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();

	// Helpers to split up the above methods for readability
	void CreateInstance(std::vector<const char*>& enabledLayers);
	void CreateSurface();
	void CreatePhysicalDevice();
	void CreateDevice(std::vector<const char*>& enabledLayers);
	void CreateSwapchain();

	// Vulkan resources
	vk::PhysicalDevice m_physicalDevice;
	vk::UniqueInstance m_instance;
	vk::UniqueDevice m_device;
	vk::Queue m_gfxQueue;
	uint32_t m_gfxQueueIdx;
	std::array<FrameResources, BACK_BUFFER_COUNT> m_frames;

	// GPU Memory allocator - it must be destroyed only AFTER all resources created from it!
	UniqueAllocator m_allocator;

	// Swapchain and depth buffer resources
	vk::UniqueSurfaceKHR m_surface;
	vk::UniqueSwapchainKHR m_swapchain;
	std::vector<vk::Image> m_swapchainImages;
	std::vector<vk::UniqueImageView> m_swapchainImageViews;
	UniqueAllocatedImage m_depthBuffer;
	vk::UniqueImageView m_depthBufferView;

	// Rendering objects
	vk::Viewport m_screenViewport;
	vk::Rect2D m_screenScissor;

	// Swapchain properties
	vk::Extent2D m_backBufferExtent;
	vk::Format m_backBufferFormat;
	vk::Format m_depthBufferFormat;
	float m_aspectRatio;

	// Window Handle
	Window m_window;

	// Current frame
	uint64_t m_frameCount;

	// Flag to check each frame if the window size changed
	bool m_sizeChanged;

	// To be removed from this class later once the pipelines aren't hard-coded
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	// Graphics resource management	
	ResourceManager m_resourceManager;

	Mesh m_mesh;
	uint32_t m_vertexBuffer;
	uint32_t m_indexBuffer;
};