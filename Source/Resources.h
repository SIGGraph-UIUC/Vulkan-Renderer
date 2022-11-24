#pragma once

#include <DirectXMath.h>

#include "VulkanUtil.h"

class UniqueAllocatedBuffer
{
public:
	UniqueAllocatedBuffer() : m_allocator(nullptr), m_buffer(nullptr), m_allocation(nullptr)
	{
	}
	UniqueAllocatedBuffer(VmaAllocator allocator, const vk::BufferCreateInfo& bufferInfo,
		const AllocationCreateInfo& allocationInfo, VmaAllocationInfo* pResultInfo = nullptr);
	~UniqueAllocatedBuffer();

	UniqueAllocatedBuffer(const UniqueAllocatedBuffer& other) = delete;
	UniqueAllocatedBuffer& operator=(const UniqueAllocatedBuffer& other) = delete;

	UniqueAllocatedBuffer(UniqueAllocatedBuffer&& other) : UniqueAllocatedBuffer()
	{
		swap(*this, other);
	}
	UniqueAllocatedBuffer& operator=(UniqueAllocatedBuffer&& other)
	{
		swap(*this, other);
		return *this;
	}

	explicit operator bool()
	{
		return m_allocator && m_buffer && m_allocation;
	}

	VkBuffer GetBuffer() const { return m_buffer; }
	VmaAllocation GetAllocation() const { return m_allocation; }

	friend void swap(UniqueAllocatedBuffer& first, UniqueAllocatedBuffer& second)
	{
		using std::swap;
		swap(first.m_allocator, second.m_allocator);
		swap(first.m_buffer, second.m_buffer);
		swap(first.m_allocation, second.m_allocation);
	}

private:
	VmaAllocator m_allocator;
	VkBuffer m_buffer;
	VmaAllocation m_allocation;
};

class UniqueAllocatedImage
{
public:
	UniqueAllocatedImage() : m_allocator(nullptr), m_image(nullptr), m_allocation(nullptr)
	{
	}
	UniqueAllocatedImage(VmaAllocator allocator, const vk::ImageCreateInfo& imageInfo,
		const AllocationCreateInfo& allocationInfo, VmaAllocationInfo* pResultInfo = nullptr);
	~UniqueAllocatedImage();

	UniqueAllocatedImage(const UniqueAllocatedImage& other) = delete;
	UniqueAllocatedImage& operator=(const UniqueAllocatedImage& other) = delete;

	UniqueAllocatedImage(UniqueAllocatedImage&& other) : UniqueAllocatedImage()
	{
		swap(*this, other);
	}
	UniqueAllocatedImage& operator=(UniqueAllocatedImage&& other)
	{
		swap(*this, other);
		return *this;
	}

	explicit operator bool()
	{
		return m_allocator && m_image && m_allocation;
	}

	VkImage GetImage() const { return m_image; }
	VmaAllocation GetAllocation() const { return m_allocation; }

	friend void swap(UniqueAllocatedImage& first, UniqueAllocatedImage& second)
	{
		using std::swap;
		swap(first.m_allocator, second.m_allocator);
		swap(first.m_image, second.m_image);
		swap(first.m_allocation, second.m_allocation);
	}

private:
	VmaAllocator m_allocator;
	VkImage m_image;
	VmaAllocation m_allocation;
};

// Global constants for all shaders
struct GlobalConstants
{
	DirectX::XMFLOAT3 eyePosition;
	DirectX::XMFLOAT4X4 viewProj;
};

// Manager for bindless resources
class ResourceManager
{
public:
	ResourceManager() : m_lastVertexOffset(0), m_lastIndexOffset(0), m_frameCount(0)
	{
	}

	ResourceManager(const vk::Device& device, VmaAllocator allocator, 
		const vk::Queue queue, uint32_t queueIdx);

	// Uses 32-bit handles because it is more efficient in a shader and we won't ever allocate
	// close to 4 GB of GPU memory for vertex data anyway
	// TODO: Make these type safe
	uint32_t CreateVertices(const vk::Device& device, const void* pSrcData, uint32_t size) const;
	uint32_t CreateIndices(const vk::Device& device, const void* pSrcData, uint32_t size) const;
	uint32_t CreateMaterial(const void* pSrcData, uint32_t size) const;
	uint32_t CreateTransform(const void* pSrcData, uint32_t size) const;
	
	// Note: for performance reasons data should only be written to these addresses through a 
	// straight memcpy rather than assignment operators etc, to avoid accidental reads.
	// TODO: Make these type safe
	void* GetMaterial(uint32_t idx) const
	{
		return m_frameResources[m_frameCount % BACK_BUFFER_COUNT].pMaterialBufferData;
	}
	void* GetTransform(uint32_t idx) const
	{
		return m_frameResources[m_frameCount % BACK_BUFFER_COUNT].pTransformBufferData;
	}
	void* GetConstants() const
	{
		return m_frameResources[m_frameCount % BACK_BUFFER_COUNT].pGlobalBufferData;
	}

	void IncrementFrameCount()
	{
		m_frameCount++;
	}

	// Retrieve the descriptor sets for binding in the render loop
	vk::DescriptorSet GetDescriptorSet() const
	{
		return m_descriptorSets[m_frameCount % BACK_BUFFER_COUNT];
	}
	vk::DescriptorSetLayout GetDescriptorSetLayout() const
	{
		return *m_descriptorSetLayout;
	}
	// The index buffer is bound separately rather than via a descriptor
	vk::Buffer GetIndexBuffer() const
	{
		return m_indexBuffer.GetBuffer();
	}

private:
	void UploadBuffer(const vk::Device& device, VkBuffer dst, 
		uint32_t offset, const void* pSrcData, uint32_t size) const;

	void SetUpDescriptors(const vk::Device& device);

	// For shared GPU-CPU resources, double buffering must be used
	struct FrameResources
	{
		FrameResources() :
			pGlobalBufferData(nullptr), pMaterialBufferData(nullptr),
			lastMaterialOffset(0), pTransformBufferData(nullptr), lastTransformOffset(0)
		{
		}
		FrameResources(VmaAllocator allocator);

		UniqueAllocatedBuffer globalBuffer;
		void* pGlobalBufferData;

		UniqueAllocatedBuffer materialBuffer;
		void* pMaterialBufferData;
		mutable uint32_t lastMaterialOffset;

		UniqueAllocatedBuffer transformBuffer;
		void* pTransformBufferData;
		mutable uint32_t lastTransformOffset;
	};

	std::array<FrameResources, BACK_BUFFER_COUNT> m_frameResources;
	
	// Immutable GPU-side resources don't require double buffering
	UniqueAllocatedBuffer m_vertexBuffer;
	mutable uint32_t m_lastVertexOffset;

	UniqueAllocatedBuffer m_indexBuffer;
	mutable uint32_t m_lastIndexOffset;

	// Vulkan resources for uploading
	vk::Queue m_queue;
	vk::UniqueFence m_transferFence;
	vk::UniqueCommandPool m_commandPool;
	vk::UniqueCommandBuffer m_commandBuffer;

	// Descriptors for the bindless tables
	vk::UniqueDescriptorSetLayout m_descriptorSetLayout;
	vk::UniqueDescriptorPool m_descriptorPool;
	std::vector<vk::DescriptorSet> m_descriptorSets;

	// Cache the allocator
	VmaAllocator m_allocator;

	uint64_t m_frameCount;
};
