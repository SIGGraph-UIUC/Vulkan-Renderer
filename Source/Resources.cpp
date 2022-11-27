#include "Resources.h"

#include <stb_image.h>

namespace
{
	constexpr float ANISOTROPY = 8.0f;

	// Immutable Samplers
	constexpr std::array<vk::SamplerCreateInfo, 24> g_samplers = {
		// Linear wrap sampler
		vk::SamplerCreateInfo({}, 
			vk::Filter::eLinear, 
			vk::Filter::eLinear, 
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eRepeat, 
			vk::SamplerAddressMode::eRepeat, 
			vk::SamplerAddressMode::eRepeat,
			0.0f, false, 0.0f, 
			false, vk::CompareOp::eAlways , 
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Linear mirror sampler
		vk::SamplerCreateInfo({},
			vk::Filter::eLinear,
			vk::Filter::eLinear,
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eMirroredRepeat,
			vk::SamplerAddressMode::eMirroredRepeat,
			vk::SamplerAddressMode::eMirroredRepeat,
			0.0f, false, 0.0f,
			false, vk::CompareOp::eAlways ,
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Linear clamp sampler
		vk::SamplerCreateInfo({}, 
			vk::Filter::eLinear, 
			vk::Filter::eLinear, 
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge,
			0.0f, false, 0.0f, 
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Linear border sampler with white border
		vk::SamplerCreateInfo({}, 
			vk::Filter::eLinear, 
			vk::Filter::eLinear, 
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder,
			0.0f, false, 0.0f, 
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE, 
			vk::BorderColor::eFloatOpaqueWhite
		),
		// Linear border sampler with black opaque border
		vk::SamplerCreateInfo({}, 
			vk::Filter::eLinear, 
			vk::Filter::eLinear, 
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder,
			0.0f, false, 0.0f, 
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE, 
			vk::BorderColor::eFloatOpaqueBlack
		),
		// Linear border sampler with black transparent border
		vk::SamplerCreateInfo({}, 
			vk::Filter::eLinear, 
			vk::Filter::eLinear, 
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder,
			0.0f, false, 0.0f, 
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE, 
			vk::BorderColor::eFloatTransparentBlack
		),
		// Point wrap sampler
		vk::SamplerCreateInfo({}, 
			vk::Filter::eNearest, 
			vk::Filter::eNearest, 
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eRepeat, 
			vk::SamplerAddressMode::eRepeat, 
			vk::SamplerAddressMode::eRepeat,
			0.0f, false, 0.0f, 
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Point mirror sampler
		vk::SamplerCreateInfo({},
			vk::Filter::eNearest,
			vk::Filter::eNearest,
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eMirroredRepeat,
			vk::SamplerAddressMode::eMirroredRepeat,
			vk::SamplerAddressMode::eMirroredRepeat,
			0.0f, false, 0.0f,
			false, vk::CompareOp::eAlways,
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Point clamp sampler
		vk::SamplerCreateInfo({}, 
			vk::Filter::eNearest, 
			vk::Filter::eNearest, 
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge,
			0.0f, false, 0.0f,
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Point border sampler with white border
		vk::SamplerCreateInfo({}, 
			vk::Filter::eNearest, 
			vk::Filter::eNearest, 
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder,
			0.0f, false, 0.0f, 
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE, 
			vk::BorderColor::eFloatOpaqueWhite
		),
		// Point border sampler with black opaque border
		vk::SamplerCreateInfo({}, 
			vk::Filter::eNearest, 
			vk::Filter::eNearest, 
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder,
			0.0f, false, 0.0f, 
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE, 
			vk::BorderColor::eFloatOpaqueBlack
		),
		// Point border sampler with black transparent border
		vk::SamplerCreateInfo({}, 
			vk::Filter::eNearest, 
			vk::Filter::eNearest, 
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder,
			0.0f, false, 0.0f, 
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE, 
			vk::BorderColor::eFloatTransparentBlack
		),
		// Anisotropic wrap sampler
		vk::SamplerCreateInfo({}, 
			vk::Filter::eLinear, 
			vk::Filter::eLinear, 
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eRepeat, 
			vk::SamplerAddressMode::eRepeat, 
			vk::SamplerAddressMode::eRepeat,
			0.0f, true, ANISOTROPY, 
			false, vk::CompareOp::eAlways,
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Anisotropic mirror sampler
		vk::SamplerCreateInfo({}, 
			vk::Filter::eLinear, 
			vk::Filter::eLinear, 
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eMirroredRepeat, 
			vk::SamplerAddressMode::eMirroredRepeat,
			vk::SamplerAddressMode::eMirroredRepeat,
			0.0f, true, ANISOTROPY, 
			false, vk::CompareOp::eAlways,
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Anisotropic clamp sampler
		vk::SamplerCreateInfo({}, 
			vk::Filter::eLinear, 
			vk::Filter::eLinear, 
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge,
			0.0f, true, ANISOTROPY, 
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Anisotropic border sampler with white border
		vk::SamplerCreateInfo({}, 
			vk::Filter::eLinear, 
			vk::Filter::eLinear, 
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder,
			0.0f, true, ANISOTROPY, 
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE, 
			vk::BorderColor::eFloatOpaqueWhite
		),
		// Anisotropic border sampler with black opaque border
		vk::SamplerCreateInfo({}, 
			vk::Filter::eLinear, 
			vk::Filter::eLinear, 
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder,
			0.0f, true, ANISOTROPY, 
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE, 
			vk::BorderColor::eFloatOpaqueBlack
		),
		// Anisotropic border sampler with black transparent border
		vk::SamplerCreateInfo({}, 
			vk::Filter::eLinear, 
			vk::Filter::eLinear, 
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder, 
			vk::SamplerAddressMode::eClampToBorder,
			0.0f, true, ANISOTROPY, 
			false, vk::CompareOp::eAlways, 
			0.0f, VK_LOD_CLAMP_NONE, 
			vk::BorderColor::eFloatTransparentBlack
		),
		// Comparison sampler with LESS
		vk::SamplerCreateInfo({}, 
			vk::Filter::eNearest, 
			vk::Filter::eNearest, 
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge,
			0.0f, false, 0.0f, 
			true, vk::CompareOp::eLess, 
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Comparison sampler with EQUAL
		vk::SamplerCreateInfo({}, 
			vk::Filter::eNearest, 
			vk::Filter::eNearest, 
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge,
			0.0f, false, 0.0f, 
			true, vk::CompareOp::eEqual, 
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Comparison sampler with LESS_OR_EQUAL
		vk::SamplerCreateInfo({}, 
			vk::Filter::eNearest, 
			vk::Filter::eNearest, 
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge,
			0.0f, false, 0.0f, 
			true, vk::CompareOp::eLessOrEqual, 
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Comparison sampler with GREATER
		vk::SamplerCreateInfo({}, 
			vk::Filter::eNearest, 
			vk::Filter::eNearest, 
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge,
			0.0f, false, 0.0f, 
			true, vk::CompareOp::eGreater, 
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Comparison sampler with NOT_EQUAL
		vk::SamplerCreateInfo({}, 
			vk::Filter::eNearest, 
			vk::Filter::eNearest,
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge,
			0.0f, false, 0.0f, 
			true, vk::CompareOp::eNotEqual,
			0.0f, VK_LOD_CLAMP_NONE
		),
		// Comparison sampler with GREATER_OR_EQUAL
		vk::SamplerCreateInfo({}, 
			vk::Filter::eNearest, 
			vk::Filter::eNearest, 
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge, 
			vk::SamplerAddressMode::eClampToEdge,
			0.0f, false, 0.0f, 
			true, vk::CompareOp::eGreaterOrEqual, 
			0.0f, VK_LOD_CLAMP_NONE
		),
	};
}

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

	// Create samplers
	for (auto& samplerInfo : g_samplers)
	{
		m_samplers.push_back(device.createSamplerUnique(samplerInfo));
	}

	// Set up descriptors
	SetUpDescriptors(device);
}

void ResourceManager::SetUpDescriptors(const vk::Device& device)
{
	constexpr uint32_t MAX_BINDING = 128;
	constexpr uint32_t N_UNIFORM_BUFFERS = 1;
	constexpr uint32_t N_STORAGE_BUFFERS = 3;

	// Convert vk::UniqueSampler vector to vk::Sampler vector
	std::vector<vk::Sampler> samplers;
	for (const auto& uniqueSampler : m_samplers)
	{
		samplers.push_back(*uniqueSampler);
	}

	std::array<vk::DescriptorSetLayoutBinding, 6> layoutBindings = {
		// Global Constants
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		// Vertex Data
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll),
		// Material Data
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll),
		// Transform Data
		vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll),
		// Immutable Samplers
		vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eSampler, vk::ShaderStageFlagBits::eAll, samplers),
		// Bindless Textures
		vk::DescriptorSetLayoutBinding(5, vk::DescriptorType::eSampledImage, MAX_BINDING, vk::ShaderStageFlagBits::eAll),
	};
	std::array<vk::DescriptorBindingFlags, 6> layoutBindingFlags = {
		vk::DescriptorBindingFlags(),
		vk::DescriptorBindingFlags(),
		vk::DescriptorBindingFlags(),
		vk::DescriptorBindingFlags(),
		vk::DescriptorBindingFlags(),
		vk::DescriptorBindingFlagBits::eUpdateAfterBind 
			| vk::DescriptorBindingFlagBits::ePartiallyBound
			| vk::DescriptorBindingFlagBits::eVariableDescriptorCount
	};
	vk::StructureChain<vk::DescriptorSetLayoutCreateInfo, vk::DescriptorSetLayoutBindingFlagsCreateInfo> layoutInfo(
		vk::DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool, layoutBindings),
		vk::DescriptorSetLayoutBindingFlagsCreateInfo(layoutBindingFlags)
	);
	m_descriptorSetLayout = device.createDescriptorSetLayoutUnique(layoutInfo.get<vk::DescriptorSetLayoutCreateInfo>());

	std::array<vk::DescriptorPoolSize, 4> poolSizes = {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, BACK_BUFFER_COUNT * N_UNIFORM_BUFFERS),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, BACK_BUFFER_COUNT * N_STORAGE_BUFFERS),
		vk::DescriptorPoolSize(vk::DescriptorType::eSampler, BACK_BUFFER_COUNT * m_samplers.size()),
		vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, BACK_BUFFER_COUNT * MAX_BINDING)
	};
	vk::DescriptorPoolCreateInfo descriptorPoolInfo(vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind, 
		BACK_BUFFER_COUNT, poolSizes);
	m_descriptorPool = device.createDescriptorPoolUnique(descriptorPoolInfo);

	std::array<vk::DescriptorSetLayout, BACK_BUFFER_COUNT> descriptorSetLayouts{};
	descriptorSetLayouts.fill(*m_descriptorSetLayout);
	std::array<uint32_t, BACK_BUFFER_COUNT> maxBindings{};
	maxBindings.fill(MAX_BINDING);
	vk::StructureChain<vk::DescriptorSetAllocateInfo, vk::DescriptorSetVariableDescriptorCountAllocateInfo> allocateInfo(
		vk::DescriptorSetAllocateInfo(*m_descriptorPool, descriptorSetLayouts),
		vk::DescriptorSetVariableDescriptorCountAllocateInfo(maxBindings)
	);
	m_descriptorSets = device.allocateDescriptorSets(allocateInfo.get<vk::DescriptorSetAllocateInfo>());

	for (int i = 0; i < m_descriptorSets.size(); i++)
	{
		auto& set = m_descriptorSets[i];
		auto& frame = m_frameResources[i];

		vk::DescriptorBufferInfo globalBufferInfo(frame.globalConstantBuffer.GetBuffer(), 0, VK_WHOLE_SIZE);
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

ResourceManager::FrameResources::FrameResources(VmaAllocator allocator) :
	pGlobalConstantBufferData(nullptr), pMaterialBufferData(nullptr),
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
	globalConstantBuffer = UniqueAllocatedBuffer(allocator, constantBufferInfo, allocationInfo, &resultInfo);
	pGlobalConstantBufferData = resultInfo.pMappedData;
	assert(pGlobalConstantBufferData);
}

// Uses 32-bit handles because it is more efficient in a shader and we won't ever allocate
// close to 4 GB of GPU memory for vertex data anyway
uint32_t ResourceManager::CreateVertices(const vk::Device& device, 
	const void* pSrcData, uint32_t size) const
{
	auto stagingBuffer = UploadStaging(device, pSrcData, size);
	UploadBuffer(device, stagingBuffer.GetBuffer(), m_vertexBuffer.GetBuffer(), m_lastVertexOffset, size);
	uint32_t ret = m_lastVertexOffset;
	m_lastVertexOffset += size;
	return ret;
}

uint32_t ResourceManager::CreateIndices(const vk::Device& device, 
	const void* pSrcData, uint32_t size) const
{
	auto stagingBuffer = UploadStaging(device, pSrcData, size);
	UploadBuffer(device, stagingBuffer.GetBuffer(), m_indexBuffer.GetBuffer(), m_lastIndexOffset, size);
	uint32_t ret = m_lastIndexOffset;
	m_lastIndexOffset += size;
	return ret;
}

uint32_t ResourceManager::CreateTexture(const vk::Device& device, const std::string& filename, 
	bool linear, bool generateMips)
{
	uint32_t textureIdx = m_textures.size();

	// Load image from file
	int width, height, channels;
	auto* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels)
	{
		throw std::runtime_error("Error loading image: " + filename);
	}
	size_t size = 4ull * width * height;

	// Mip-map generation logic
	vk::ImageUsageFlags imageUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	uint32_t mipLevels = 1;
	if (generateMips)
	{
		imageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
		mipLevels = static_cast<uint32_t>(floor(log2(float(std::min(width, height)))) + 1);
	}

	// Set image and allocation properties, then create the image
	AllocationCreateInfo allocationInfo({}, VMA_MEMORY_USAGE_AUTO);
	vk::Format format = linear ? vk::Format::eR8G8B8A8Unorm : vk::Format::eR8G8B8A8Srgb;
	vk::ImageCreateInfo imageInfo({}, vk::ImageType::e2D, format,
		vk::Extent3D(width, height, 1), mipLevels, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
		imageUsage, vk::SharingMode::eExclusive, {}, vk::ImageLayout::eUndefined);
	m_textures.push_back(UniqueAllocatedImage(m_allocator, imageInfo, allocationInfo));

	vk::ImageViewCreateInfo viewInfo({}, m_textures.back().GetImage(), vk::ImageViewType::e2D, format, {}, 
		vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 
			0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS));
	m_textureViews.push_back(device.createImageViewUnique(viewInfo));
	
	// Upload the image
	auto stagingBuffer = UploadStaging(device, pixels, size);
	UploadImage(device, stagingBuffer.GetBuffer(), m_textures.back().GetImage(),
		vk::Offset3D(), vk::Extent3D(width, height, 1), mipLevels);

	// Bind the image
	for (int i = 0; i < m_descriptorSets.size(); i++)
	{
		auto& set = m_descriptorSets[i];
		vk::DescriptorImageInfo descriptorInfo(nullptr, *m_textureViews.back(), vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet write(set, 5, textureIdx, vk::DescriptorType::eSampledImage, descriptorInfo);
		device.updateDescriptorSets(write, {});
	}

	return textureIdx;
}

UniqueAllocatedBuffer ResourceManager::UploadStaging(const vk::Device& device, const void* pSrcData, size_t size) const
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

	return stagingBuffer;
}

void ResourceManager::UploadBuffer(const vk::Device& device, const vk::Buffer& src, 
	const vk::Buffer& dst, uint32_t dstOffset, size_t size) const
{
	// Upload to GPU
	m_commandBuffer->reset();
	vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	m_commandBuffer->begin(beginInfo);

	vk::BufferCopy copyRegion(0, dstOffset, size);
	m_commandBuffer->copyBuffer(src, dst, copyRegion);

	m_commandBuffer->end();

	vk::CommandBufferSubmitInfo commandSubmitInfo(*m_commandBuffer);
	vk::SubmitInfo2 submitInfo({}, {}, commandSubmitInfo);
	m_queue.submit2(submitInfo, *m_transferFence);

	ThrowIfFailed(device.waitForFences(*m_transferFence, true, UINT64_MAX));
	device.resetFences(*m_transferFence);
}

void ResourceManager::UploadImage(const vk::Device& device, const vk::Buffer& src, const vk::Image& dst,
	const vk::Offset3D& imageOffset, const vk::Extent3D& imageExtent, uint32_t mipLevels) const
{
	// Temporary object
	vk::ImageMemoryBarrier2 barrier;
	const uint32_t width = imageExtent.width;
	const uint32_t height = imageExtent.height;	

	m_commandBuffer->reset();
	vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	m_commandBuffer->begin(beginInfo);

	// Transition image layout from Undefined to TransferDst
	barrier = CreateImageMemoryBarrier(AccessType::eWriteHost, AccessType::eWriteTransfer,
		ImageLayout::eOptimal, ImageLayout::eOptimal, true, dst,
		vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 
			0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS));
	m_commandBuffer->pipelineBarrier2(vk::DependencyInfo({}, {}, {}, barrier));

	// Copy mip level 0 from the staging buffer
	vk::ImageSubresourceLayers subresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
	vk::BufferImageCopy copyRegion(0, 0, 0, subresource, imageOffset, imageExtent);
	m_commandBuffer->copyBufferToImage(src, dst, vk::ImageLayout::eTransferDstOptimal, copyRegion);

	// Transition layout of mip level 0 to TransferSrc, if mipmap generation is required
	if (mipLevels > 1)
	{
		barrier = CreateImageMemoryBarrier(AccessType::eWriteTransfer, AccessType::eReadTransfer,
			ImageLayout::eOptimal, ImageLayout::eOptimal, false, dst,
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, VK_REMAINING_ARRAY_LAYERS));
		m_commandBuffer->pipelineBarrier2(vk::DependencyInfo({}, {}, {}, barrier));
	}

	// Generate subsequent mip levels on the GPU -- no CPU access is required anymore
	for (uint32_t i = 1; i < mipLevels; i++)
	{
		// Copy the previous mip level into the current one using BlitImage
		vk::ImageBlit blitRegion;
		blitRegion.srcSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i - 1, 0, 1);
		blitRegion.srcOffsets[1].x = std::max(width >> (i - 1), 1u);
		blitRegion.srcOffsets[1].y = std::max(height >> (i - 1), 1u);
		blitRegion.srcOffsets[1].z = 1;
		blitRegion.dstSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1);
		blitRegion.dstOffsets[1].x = std::max(width >> i, 1u);
		blitRegion.dstOffsets[1].y = std::max(height >> i, 1u);
		blitRegion.dstOffsets[1].z = 1;
		m_commandBuffer->blitImage(dst, vk::ImageLayout::eTransferSrcOptimal, dst,
			vk::ImageLayout::eTransferDstOptimal, blitRegion, vk::Filter::eLinear);

		// Transition new mip level to TransferSrc memory layout
		barrier = CreateImageMemoryBarrier(AccessType::eWriteTransfer, AccessType::eReadTransfer,
			ImageLayout::eOptimal, ImageLayout::eOptimal, false, dst,
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, i, 1, 0, VK_REMAINING_ARRAY_LAYERS));
		m_commandBuffer->pipelineBarrier2(vk::DependencyInfo({}, {}, {}, barrier));
	}

	// Transition image layout from TransferDst to ShaderReadOnly for all mip levels at once
	barrier = CreateImageMemoryBarrier(AccessType::eWriteTransfer, AccessType::eReadAnyShader,
		ImageLayout::eOptimal, ImageLayout::eOptimal,
		true, dst,
		vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 
			0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS));
	m_commandBuffer->pipelineBarrier2(vk::DependencyInfo({}, {}, {}, barrier));

	m_commandBuffer->end();

	vk::CommandBufferSubmitInfo commandSubmitInfo(*m_commandBuffer);
	vk::SubmitInfo2 submitInfo({}, {}, commandSubmitInfo);
	m_queue.submit2(submitInfo, *m_transferFence);

	ThrowIfFailed(device.waitForFences(*m_transferFence, true, UINT64_MAX));
	device.resetFences(*m_transferFence);
}
