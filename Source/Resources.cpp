#include "Resources.h"

UniqueAllocatedBuffer::UniqueAllocatedBuffer(VmaAllocator allocator, const vk::BufferCreateInfo& bufferInfo, 
	const AllocationCreateInfo& allocationInfo, VmaAllocationInfo* pResultInfo) :
	m_allocator(allocator)
{
	// VMA library uses a C API, so we need to convert the classes in order to pass pointers to them
	VkBufferCreateInfo bufferInfoTmp = bufferInfo;
	VmaAllocationCreateInfo allocationInfoTmp = allocationInfo;
	ThrowIfFailed(vmaCreateBuffer(allocator, &bufferInfoTmp, &allocationInfoTmp, &m_buffer, &m_allocation, pResultInfo));
}

UniqueAllocatedBuffer::~UniqueAllocatedBuffer()
{
	if (m_allocator && m_buffer && m_allocation)
	{
		vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
	}
}

UniqueAllocatedImage::UniqueAllocatedImage(VmaAllocator allocator, const vk::ImageCreateInfo& imageInfo, 
	const AllocationCreateInfo& allocationInfo, VmaAllocationInfo* pResultInfo) :
	m_allocator(allocator)
{
	VkImageCreateInfo imageInfoTmp = imageInfo;
	VmaAllocationCreateInfo allocationInfoTmp = allocationInfo;
	ThrowIfFailed(vmaCreateImage(allocator, &imageInfoTmp, &allocationInfoTmp, &m_image, &m_allocation, pResultInfo));
}

UniqueAllocatedImage::~UniqueAllocatedImage()
{
	if (m_allocator && m_image && m_allocation)
	{
		vmaDestroyImage(m_allocator, m_image, m_allocation);
	}
}

ResourceManager::ResourceManager(const vk::Device& device, VmaAllocator allocator, 
	const vk::Queue queue, uint32_t queueIdx) :
	m_queue(queue), m_lastVertexOffset(0), m_lastIndexOffset(0), m_allocator(allocator)
{
	// Create the fence
	vk::FenceCreateInfo fenceInfo;
	m_transferFence = device.createFenceUnique(fenceInfo);

	// Create the command pool
	vk::CommandPoolCreateInfo poolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueIdx);
	m_commandPool = device.createCommandPoolUnique(poolInfo);

	// Create the command buffer
	vk::CommandBufferAllocateInfo commandInfo(*m_commandPool, vk::CommandBufferLevel::ePrimary, 1);
	m_commandBuffer = std::move(device.allocateCommandBuffersUnique(commandInfo)[0]);

	// 32 MB buffer
	constexpr size_t bufferSize = 33554432;

	// Create a big vertex buffer
	vk::BufferCreateInfo bufferInfo({}, bufferSize,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::SharingMode::eExclusive);
	AllocationCreateInfo allocationInfo({}, VMA_MEMORY_USAGE_AUTO);
	m_vertexBuffer = UniqueAllocatedBuffer(m_allocator, bufferInfo, allocationInfo);

	// Create a big index buffer
	bufferInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	m_indexBuffer = UniqueAllocatedBuffer(m_allocator, bufferInfo, allocationInfo);

	// Create per-frame double-buffered resources
	for (auto& frame : m_frameResources)
	{
		frame = FrameResources(m_allocator);
	}

	// Set up descriptors
	SetUpDescriptors(device);
}

void ResourceManager::SetUpDescriptors(const vk::Device& device)
{
	std::array<vk::DescriptorSetLayoutBinding, 4> layoutBindings = {
		// Global Constants
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics),
		// Vertex Data
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics),
		// Material Data
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics),
		// Transform Data
		vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics)
	};
	vk::DescriptorSetLayoutCreateInfo layoutInfo({}, layoutBindings);
	m_descriptorSetLayout = device.createDescriptorSetLayoutUnique(layoutInfo);

	std::array<vk::DescriptorPoolSize, 2> poolSizes = {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 16),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 16)
	};
	vk::DescriptorPoolCreateInfo descriptorPoolInfo({}, 16, poolSizes);
	m_descriptorPool = device.createDescriptorPoolUnique(descriptorPoolInfo);

	std::array<vk::DescriptorSetLayout, BACK_BUFFER_COUNT> descriptorSetLayouts;
	descriptorSetLayouts.fill(*m_descriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo(*m_descriptorPool, descriptorSetLayouts);
	m_descriptorSets = device.allocateDescriptorSets(allocInfo);

	for (int i = 0; i < m_descriptorSets.size(); i++)
	{
		auto& set = m_descriptorSets[i];
		auto& frame = m_frameResources[i];

		vk::DescriptorBufferInfo globalBufferInfo(frame.globalBuffer.GetBuffer(), 0, VK_WHOLE_SIZE);
		vk::WriteDescriptorSet constantBufferWrite(set, 0, 0, vk::DescriptorType::eUniformBuffer, {}, globalBufferInfo);
		device.updateDescriptorSets(constantBufferWrite, {});

		vk::DescriptorBufferInfo vertexBufferInfo(m_vertexBuffer.GetBuffer(), 0, VK_WHOLE_SIZE);
		vk::DescriptorBufferInfo materialBufferInfo(frame.materialBuffer.GetBuffer(), 0, VK_WHOLE_SIZE);
		vk::DescriptorBufferInfo transformBufferInfo(frame.transformBuffer.GetBuffer(), 0, VK_WHOLE_SIZE);
		std::array<vk::WriteDescriptorSet, 3> storageBufferWrites = {
			vk::WriteDescriptorSet(set, 1, 0, vk::DescriptorType::eStorageBuffer, {}, vertexBufferInfo),
			vk::WriteDescriptorSet(set, 2, 0, vk::DescriptorType::eStorageBuffer, {}, materialBufferInfo),
			vk::WriteDescriptorSet(set, 3, 0, vk::DescriptorType::eStorageBuffer, {}, transformBufferInfo)
		};
		device.updateDescriptorSets(storageBufferWrites, {});
	}
}

// Uses 32-bit handles because it is more efficient in a shader and we won't ever allocate
// close to 4 GB of GPU memory for vertex data anyway
uint32_t ResourceManager::CreateVertices(const vk::Device& device, 
	const void* pSrcData, uint32_t size) const
{
	UploadBuffer(device, m_vertexBuffer.GetBuffer(), m_lastVertexOffset, pSrcData, size);
	uint32_t ret = m_lastVertexOffset;
	m_lastVertexOffset += size;
	return ret;
}

uint32_t ResourceManager::CreateIndices(const vk::Device& device, 
	const void* pSrcData, uint32_t size) const
{
	UploadBuffer(device, m_indexBuffer.GetBuffer(), m_lastIndexOffset, pSrcData, size);
	uint32_t ret = m_lastIndexOffset;
	m_lastIndexOffset += size;
	return ret;
}

void ResourceManager::UploadBuffer(const vk::Device& device, VkBuffer dst, 
	uint32_t offset, const void* pSrcData, uint32_t size) const
{
	// Create staging buffer
	vk::BufferCreateInfo bufferInfo({}, size, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive);
	AllocationCreateInfo allocationInfo(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO);
	UniqueAllocatedBuffer stagingBuffer(m_allocator, bufferInfo, allocationInfo);

	// Transfer
	void* ptr;
	ThrowIfFailed(vmaMapMemory(m_allocator, stagingBuffer.GetAllocation(), &ptr));
	memcpy(ptr, pSrcData, size);
	vmaUnmapMemory(m_allocator, stagingBuffer.GetAllocation());

	// Upload to GPU. For simplicity, we are doing this synchronously so we will stall the CPU
	// until the upload is completed. It could be optimized by waiting for the vertex stage only.
	m_commandBuffer->reset();
	vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	m_commandBuffer->begin(beginInfo);

	vk::BufferCopy copyRegion(0, offset, size);
	m_commandBuffer->copyBuffer(stagingBuffer.GetBuffer(), dst, copyRegion);

	m_commandBuffer->end();

	vk::CommandBufferSubmitInfo commandSubmitInfo(*m_commandBuffer);
	vk::SubmitInfo2 submitInfo({}, {}, commandSubmitInfo);
	m_queue.submit2(submitInfo, *m_transferFence);

	device.waitForFences(*m_transferFence, true, UINT64_MAX);
	device.resetFences(*m_transferFence);
}

ResourceManager::FrameResources::FrameResources(VmaAllocator allocator) :
	pGlobalBufferData(nullptr), pMaterialBufferData(nullptr),
	lastMaterialOffset(0), pTransformBufferData(nullptr), lastTransformOffset(0)
{
	// 32 MB buffer
	constexpr size_t bufferSize = 33554432;

	// Resources frequently accessed by CPU should be in memory that is both HOST_VISIBLE
	// and DEVICE_LOCAL if possible. Modern GPUs have at least 250 MB of such memory.
	AllocationCreateInfo allocationInfo(VMA_ALLOCATION_CREATE_MAPPED_BIT
		| VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO);
	VmaAllocationInfo resultInfo{};

	vk::BufferCreateInfo storageBufferInfo({}, bufferSize,
		vk::BufferUsageFlagBits::eStorageBuffer, vk::SharingMode::eExclusive);
	materialBuffer = UniqueAllocatedBuffer(allocator, storageBufferInfo, allocationInfo, &resultInfo);
	pMaterialBufferData = resultInfo.pMappedData;
	assert(pMaterialBufferData);

	transformBuffer = UniqueAllocatedBuffer(allocator, storageBufferInfo, allocationInfo, &resultInfo);
	pTransformBufferData = resultInfo.pMappedData;
	assert(pTransformBufferData);

	vk::BufferCreateInfo constantBufferInfo({}, sizeof(GlobalConstants),
		vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive);
	globalBuffer = UniqueAllocatedBuffer(allocator, constantBufferInfo, allocationInfo, &resultInfo);
	pGlobalBufferData = resultInfo.pMappedData;
	assert(pGlobalBufferData);
}
