#pragma once

#include <string>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

// Use double-buffering
constexpr int BACK_BUFFER_COUNT = 2;

// Loads a SPIR-V shader from disk
vk::UniqueShaderModule CreateShader(const vk::Device& device, const std::string& path);

inline void ThrowIfFailed(vk::Result result)
{
	if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed with Vulkan error: " + vk::to_string(result));
	}
}

inline void ThrowIfFailed(VkResult result)
{
	ThrowIfFailed(static_cast<vk::Result>(result));
}

class PipelineBuilder
{
public:
	PipelineBuilder() : m_depthAttachmentFormat(), m_stencilAttachmentFormat() { }

	PipelineBuilder& AddShaderStage(vk::ShaderStageFlagBits stage, const vk::ShaderModule& module);
	PipelineBuilder& SetVertexInputState(const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo);
	PipelineBuilder& SetInputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo& inputAssemblyInfo);
	PipelineBuilder& SetRasterizerState(const vk::PipelineRasterizationStateCreateInfo& rasterizerInfo);
	PipelineBuilder& SetMultisampleState(const vk::PipelineMultisampleStateCreateInfo& multisampleInfo);
	PipelineBuilder& SetDepthStencilState(const vk::PipelineDepthStencilStateCreateInfo& depthStencilInfo);
	PipelineBuilder& AddColorAttachment(const vk::PipelineColorBlendAttachmentState& blendInfo, vk::Format format);
	PipelineBuilder& SetDepthAttachment(vk::Format format);
	PipelineBuilder& SetStencilAttachment(vk::Format format);
	vk::UniquePipeline CreatePipeline(const vk::Device& device, const vk::PipelineLayout& layout);

private:
	// Info structs for creating the pipeline
	std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStageInfos;
	vk::PipelineVertexInputStateCreateInfo m_vertexInputInfo;
	vk::PipelineInputAssemblyStateCreateInfo m_inputAssemblyInfo;
	vk::PipelineRasterizationStateCreateInfo m_rasterizerInfo;
	std::vector<vk::PipelineColorBlendAttachmentState> m_colorBlendAttachments;
	vk::PipelineMultisampleStateCreateInfo m_multisampleInfo;
	vk::PipelineDepthStencilStateCreateInfo m_depthStencilInfo;

	// Pipeline Attachments
	std::vector<vk::Format> m_colorAttachmentFormats;
	vk::Format m_depthAttachmentFormat;
	vk::Format m_stencilAttachmentFormat;
};

// Helper structs with constructors for use with Vulkan Memory Allocator
struct AllocationCreateInfo
{
	VmaAllocationCreateFlags flags;
	VmaMemoryUsage usage;
	vk::MemoryPropertyFlags requiredFlags;
	vk::MemoryPropertyFlags preferredFlags;
	uint32_t memoryTypeBits;
	VmaPool pool;
	void* pUserData;
	float priority;

	AllocationCreateInfo(VmaAllocationCreateFlags flags = 0,
		VmaMemoryUsage usage = VMA_MEMORY_USAGE_UNKNOWN, 
		vk::MemoryPropertyFlags requiredFlags = {},
		vk::MemoryPropertyFlags preferredFlags = {}, 
		uint32_t memoryTypeBits = 0,
		VmaPool pool = nullptr, 
		void* pUserData = nullptr, 
		float priority = 0.0f) :
		flags(flags), 
		usage(usage), 
		requiredFlags(requiredFlags), 
		preferredFlags(preferredFlags),
		memoryTypeBits(memoryTypeBits), 
		pool(pool), 
		pUserData(pUserData), 
		priority(priority)
	{
	}

	constexpr operator VmaAllocationCreateInfo() const
	{
		VmaAllocationCreateInfo info{};
		info.flags = flags;
		info.usage = usage;
		info.requiredFlags = static_cast<VkMemoryPropertyFlags>(requiredFlags);
		info.preferredFlags = static_cast<VkMemoryPropertyFlags>(preferredFlags);
		info.memoryTypeBits = memoryTypeBits;
		info.pool = pool;
		info.pUserData = pUserData;
		info.priority = priority;
		return info;
	}
};

// RAII class for VmaAllocator
class UniqueAllocator
{
public:
	UniqueAllocator() : m_allocator(nullptr)
	{
	}
	UniqueAllocator(const VmaAllocatorCreateInfo& allocatorInfo)
	{
		ThrowIfFailed(vmaCreateAllocator(&allocatorInfo, &m_allocator));
	}
	~UniqueAllocator()
	{
		vmaDestroyAllocator(m_allocator);
	}

	UniqueAllocator(const UniqueAllocator& other) = delete;
	UniqueAllocator& operator=(const UniqueAllocator& other) = delete;

	UniqueAllocator(UniqueAllocator&& other) : UniqueAllocator()
	{
		swap(*this, other);
	}
	UniqueAllocator& operator=(UniqueAllocator&& other)
	{
		swap(*this, other);
		return *this;
	}

	const VmaAllocator& operator*() const { return m_allocator; }
	VmaAllocator& operator*() { return m_allocator; }

	friend void swap(UniqueAllocator& first, UniqueAllocator& second)
	{
		using std::swap;
		swap(first.m_allocator, second.m_allocator);
	}

private:
	VmaAllocator m_allocator;
};

template<typename... Args>
class Signal
{
public:
	using Index = size_t;

	Signal() = default;

	void Send(Args... args) const
	{
		for (auto& slot : m_connected)
		{
			slot(args...);
		}
	}

	Index Register(const std::function<void(Args...)>& slot)
	{
		m_connected.push_back(slot);
		return m_connected.size() - 1;
	}

	template<typename T>
	Index Register(T* instance, void (T::* func)(Args...))
	{
		return Register([=](Args... args) { (instance->*func)(args...); });
	}

	template<typename T>
	Index Register(const T* instance, void (T::* func)(Args...) const)
	{
		return Register([=](Args... args) { (instance->*func)(args...); });
	}

	void Deregister(Index id)
	{
		Index index = 0;
		for (const auto& it = m_connected.cbegin(); it != m_connected.cend(); ++it)
		{
			if (index == id)
			{
				m_connected.erase(it);
				return;
			}
			index++;
		}
	}

private:
	std::vector<std::function<void(Args...)>> m_connected;
};

double GetTime();

template<typename T>
constexpr auto EnumToInt(T e)
{
	static_assert(std::is_enum_v<T>, "EnumToInt only operates on enums");
	return static_cast<std::underlying_type_t<T>>(e);
}

template<typename T>
constexpr T IntToEnum(std::underlying_type_t<T> i)
{
	static_assert(std::is_enum_v<T>, "IntToEnum only operates on enums");
	return static_cast<T>(i);
}

// Defines common access patterns
enum class AccessType
{
	eNone,
	eReadIndirectBuffer,
	eReadIndexBuffer,
	eReadVertexBuffer,
	eReadVertexShader,
	eReadFragmentShader,
	eReadFragmentShaderColorInputAttachment,
	eReadFragmentShaderDepthStencilInputAttachment,
	eReadColorAttachment,
	eReadDepthStencilAttachment,
	eReadComputeShader,
	eReadAnyShader,
	eReadTransfer,
	eReadHost,
	eReadPresent,
	eWriteVertexShader,
	eWriteFragmentShader,
	eWriteColorAttachment,
	eWriteDepthStencilAttachment,
	eWriteComputeShader,
	eWriteAnyShader,
	eWriteTransfer,
	eWriteHostPreinitialized,
	eWriteHost,
	eReadWriteColorAttachment,
	eGeneral
};

enum class ImageLayout
{
	eOptimal,
	eGeneral
};

struct AccessInfo
{
	vk::PipelineStageFlags2 stageMask;
	vk::AccessFlags2 accessMask;
	vk::ImageLayout imageLayout;
};

vk::MemoryBarrier2 CreateMemoryBarrier(const vk::ArrayProxy<AccessType>& prevAccesses,
	const vk::ArrayProxy<AccessType>& nextAccesses);

vk::BufferMemoryBarrier2 CreateBufferMemoryBarrier(const vk::ArrayProxy<AccessType>& prevAccesses,
	const vk::ArrayProxy<AccessType>& nextAccesses, const vk::Buffer& buffer, vk::DeviceSize offset, vk::DeviceSize size,
	uint32_t srcQueueFamilyIdx = VK_QUEUE_FAMILY_IGNORED, uint32_t dstQueueFamilyIdx = VK_QUEUE_FAMILY_IGNORED);

vk::ImageMemoryBarrier2 CreateImageMemoryBarrier(const vk::ArrayProxy<AccessType>& prevAccesses,
	const vk::ArrayProxy<AccessType>& nextAccesses, ImageLayout oldLayout, ImageLayout newLayout,
	bool discard, const vk::Image& image, const vk::ImageSubresourceRange& subresourceRange,
	uint32_t srcQueueFamilyIdx = VK_QUEUE_FAMILY_IGNORED, uint32_t dstQueueFamilyIdx = VK_QUEUE_FAMILY_IGNORED);
