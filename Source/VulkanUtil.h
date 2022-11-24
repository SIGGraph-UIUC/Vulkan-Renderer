#pragma once

#include <string>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <vulkan/vk_enum_string_helper.h>
#include <vma/vk_mem_alloc.h>

// Use double-buffering
constexpr int BACK_BUFFER_COUNT = 2;

// Loads a SPIR-V shader from disk
vk::UniqueShaderModule CreateShader(const vk::Device& device, const std::string& path);

// Helper function to insert a single image memory barrier into a command buffer
void InsertImageMemoryBarrier(const vk::CommandBuffer& commandBuffer, const vk::Image& image,
	vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::ImageLayout oldLayout,
	vk::ImageLayout newLayout, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask,
	const vk::ImageSubresourceRange& subresourceRange);

inline void ThrowIfFailed(VkResult result)
{
	using namespace std::string_literals;

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Vulkan error"s + string_VkResult(result));
	}
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

	VmaAllocator operator*() const { return m_allocator; }

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
	static_assert(std::is_enum_v<T>, "EnumToInt only works for enums");
	return static_cast<std::underlying_type_t<T>>(e);
}

template<typename T>
constexpr T IntToEnum(std::underlying_type_t<T> i)
{
	static_assert(std::is_enum_v<T>, "IntToEnum only works for enums");
	return static_cast<T>(i);
}
