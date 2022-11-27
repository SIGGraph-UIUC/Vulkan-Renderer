#include "VulkanUtil.h"

#include <fstream>

namespace
{
	constexpr std::array<AccessInfo, 26> g_accessInfoTable = {
		// eNone
		AccessInfo{
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::ImageLayout::eUndefined
		},
		// eReadIndirectBuffer
		AccessInfo{
			vk::PipelineStageFlagBits2::eDrawIndirect,
			vk::AccessFlagBits2::eIndirectCommandRead,
			vk::ImageLayout::eUndefined
		},
		// eReadIndexBuffer
		AccessInfo{
			vk::PipelineStageFlagBits2::eIndexInput,
			vk::AccessFlagBits2::eIndexRead,
			vk::ImageLayout::eUndefined
		},
		// eReadVertexBuffer
		AccessInfo{
			vk::PipelineStageFlagBits2::eVertexAttributeInput,
			vk::AccessFlagBits2::eVertexAttributeRead,
			vk::ImageLayout::eUndefined
		},
		// eReadVertexShader
		AccessInfo{
			vk::PipelineStageFlagBits2::eVertexShader,
			vk::AccessFlagBits2::eShaderRead,
			vk::ImageLayout::eShaderReadOnlyOptimal
		},
		// eReadFragmentShader
		AccessInfo{
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eShaderRead,
			vk::ImageLayout::eShaderReadOnlyOptimal
		},
		// eReadFragmentShaderColorInputAttachment
		AccessInfo{
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eInputAttachmentRead,
			vk::ImageLayout::eShaderReadOnlyOptimal
		},
		// eReadFragmentShaderDepthStencilInputAttachment
		AccessInfo{
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::AccessFlagBits2::eDepthStencilAttachmentRead,
			vk::ImageLayout::eDepthStencilReadOnlyOptimal
		},
		// eReadColorAttachment
		AccessInfo{
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentRead,
			vk::ImageLayout::eColorAttachmentOptimal
		},
		// eReadDepthStencilAttachment
		AccessInfo{
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::AccessFlagBits2::eDepthStencilAttachmentRead,
			vk::ImageLayout::eDepthStencilReadOnlyOptimal
		},
		// eReadComputeShader
		AccessInfo{
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderRead,
			vk::ImageLayout::eShaderReadOnlyOptimal
		},
		// eReadAnyShader
		AccessInfo{
			vk::PipelineStageFlagBits2::eAllGraphics | vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderRead,
			vk::ImageLayout::eShaderReadOnlyOptimal
		},
		// eReadTransfer
		AccessInfo{
			vk::PipelineStageFlagBits2::eTransfer,
			vk::AccessFlagBits2::eTransferRead,
			vk::ImageLayout::eTransferSrcOptimal
		},
		// eReadHost
		AccessInfo{
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eHostRead,
			vk::ImageLayout::eGeneral
		},
		// eReadPresent
		AccessInfo{
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::ImageLayout::ePresentSrcKHR
		},
		// eWriteVertexShader
		AccessInfo{
			vk::PipelineStageFlagBits2::eVertexShader,
			vk::AccessFlagBits2::eShaderWrite,
			vk::ImageLayout::eGeneral
		},
		// eWriteFragmentShader
		AccessInfo{
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eShaderWrite,
			vk::ImageLayout::eGeneral
		},
		// eWriteColorAttachment
		AccessInfo{
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::ImageLayout::eColorAttachmentOptimal
		},
		// eWriteDepthStencilAttachment
		AccessInfo{
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		},
		// eWriteComputeShader
		AccessInfo{
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderWrite,
			vk::ImageLayout::eGeneral
		},
		// eWriteAnyShader
		AccessInfo{
			vk::PipelineStageFlagBits2::eAllGraphics | vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderWrite,
			vk::ImageLayout::eGeneral
		},
		// eWriteTransfer
		AccessInfo{
			vk::PipelineStageFlagBits2::eTransfer,
			vk::AccessFlagBits2::eTransferWrite,
			vk::ImageLayout::eTransferDstOptimal
		},
		// eWriteHostPreinitialized
		AccessInfo{
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eHostWrite,
			vk::ImageLayout::ePreinitialized
		},
		// eWriteHost
		AccessInfo{
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eHostWrite,
			vk::ImageLayout::eGeneral
		},
		// eReadWriteColorAttachment
		AccessInfo{
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::ImageLayout::eGeneral
		},
		// eGeneral
		AccessInfo{
			vk::PipelineStageFlagBits2::eAllCommands,
			vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
			vk::ImageLayout::eGeneral
		}
	};

	constexpr bool RequiresWriteAccess(AccessType type)
	{
		return type >= AccessType::eWriteVertexShader;
	}
}

// Loads a SPIR-V shader from disk
vk::UniqueShaderModule CreateShader(const vk::Device& device, const std::string& path)
{
	// Read the file
	std::ifstream ifs(path, std::ios::ate | std::ios::binary);
	if (!ifs)
	{
		throw std::runtime_error("Failed to open shader file " + path);
	}
	size_t size = ifs.tellg();
	std::vector<char> buf(size);
	ifs.seekg(0);
	ifs.read(buf.data(), size);
	ifs.close();

	vk::ShaderModuleCreateInfo smci({}, size, reinterpret_cast<const uint32_t*>(buf.data()));
	return device.createShaderModuleUnique(smci);
}

double GetTime()
{
	static LARGE_INTEGER s_frequency{};

	if (s_frequency.QuadPart == 0)
	{
		QueryPerformanceFrequency(&s_frequency);
	}
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return 1.0 * count.QuadPart / s_frequency.QuadPart;
}

vk::MemoryBarrier2 CreateMemoryBarrier(const vk::ArrayProxy<AccessType>& prevAccesses, const vk::ArrayProxy<AccessType>& nextAccesses)
{
	vk::MemoryBarrier2 ret;

	for (auto access : prevAccesses)
	{
		// Write accesses should appear on their own
		assert(!(RequiresWriteAccess(access) && prevAccesses.size() > 1));

		const auto& info = g_accessInfoTable[EnumToInt(access)];

		ret.srcStageMask |= info.stageMask;

		// If the previous access was a write, make memory available
		if (RequiresWriteAccess(access))
		{
			ret.srcAccessMask |= info.accessMask;
		}
	}
	for (auto access : nextAccesses)
	{
		// Write accesses should appear on their own
		assert(!(RequiresWriteAccess(access) && nextAccesses.size() > 1));

		const auto& info = g_accessInfoTable[EnumToInt(access)];

		ret.dstStageMask |= info.stageMask;

		// If previous accesses included a write, make memory visible
		if (ret.srcAccessMask)
		{
			ret.dstAccessMask |= info.accessMask;
		}
	}
	return ret;
}

vk::BufferMemoryBarrier2 CreateBufferMemoryBarrier(const vk::ArrayProxy<AccessType>& prevAccesses, const vk::ArrayProxy<AccessType>& nextAccesses, const vk::Buffer& buffer, vk::DeviceSize offset, vk::DeviceSize size, uint32_t srcQueueFamilyIdx, uint32_t dstQueueFamilyIdx)
{
	vk::BufferMemoryBarrier2 ret({}, {}, {}, {}, srcQueueFamilyIdx, dstQueueFamilyIdx, buffer, offset, size);

	// Other steps are exactly the same as for a normal MemoryBarrier.
	for (auto access : prevAccesses)
	{
		// Write accesses should appear on their own
		assert(!(RequiresWriteAccess(access) && prevAccesses.size() > 1));

		const auto& info = g_accessInfoTable[EnumToInt(access)];

		ret.srcStageMask |= info.stageMask;

		// If the previous access was a write, make memory available
		if (RequiresWriteAccess(access))
		{
			ret.srcAccessMask |= info.accessMask;
		}
	}
	for (auto access : nextAccesses)
	{
		// Write accesses should appear on their own
		assert(!(RequiresWriteAccess(access) && nextAccesses.size() > 1));

		const auto& info = g_accessInfoTable[EnumToInt(access)];

		ret.dstStageMask |= info.stageMask;

		// If previous accesses included a write, make memory visible
		if (ret.srcAccessMask)
		{
			ret.dstAccessMask |= info.accessMask;
		}
	}
	return ret;
}

vk::ImageMemoryBarrier2 CreateImageMemoryBarrier(const vk::ArrayProxy<AccessType>& prevAccesses, 
	const vk::ArrayProxy<AccessType>& nextAccesses, ImageLayout oldLayout, ImageLayout newLayout, 
	bool discard, const vk::Image& image, const vk::ImageSubresourceRange& subresourceRange, 
	uint32_t srcQueueFamilyIdx, uint32_t dstQueueFamilyIdx)
{
	vk::ImageMemoryBarrier2 ret({}, {}, {}, {}, {}, {}, srcQueueFamilyIdx, dstQueueFamilyIdx, image, subresourceRange);

	for (auto access : prevAccesses)
	{
		// Write accesses should appear on their own
		assert(!(RequiresWriteAccess(access) && prevAccesses.size() > 1));

		const auto& info = g_accessInfoTable[EnumToInt(access)];

		ret.srcStageMask |= info.stageMask;

		// If the previous access was a write, make memory available
		if (RequiresWriteAccess(access))
		{
			ret.srcAccessMask |= info.accessMask;
		}

		// Deal with layout transitions
		if (discard)
		{
			ret.oldLayout = vk::ImageLayout::eUndefined;
		}
		else
		{
			vk::ImageLayout layout = vk::ImageLayout::eUndefined;
			if (oldLayout == ImageLayout::eGeneral)
			{
				layout = (access == AccessType::eReadPresent)
					? vk::ImageLayout::ePresentSrcKHR
					: vk::ImageLayout::eGeneral;
			}
			else if (oldLayout == ImageLayout::eOptimal)
			{
				layout = info.imageLayout;
			}
			// Check that layouts aren't mixed
			assert((ret.oldLayout == vk::ImageLayout::eUndefined) || (ret.oldLayout == layout));
			ret.oldLayout = layout;
		}
	}
	for (auto access : nextAccesses)
	{
		// Write accesses should appear on their own
		assert(!(RequiresWriteAccess(access) && nextAccesses.size() > 1));

		const auto& info = g_accessInfoTable[EnumToInt(access)];

		ret.dstStageMask |= info.stageMask;

		// If previous accesses included a write, make memory visible
		if (ret.srcAccessMask)
		{
			ret.dstAccessMask |= info.accessMask;
		}

		// Deal with layout transitions
		vk::ImageLayout layout = vk::ImageLayout::eUndefined;
		if (newLayout == ImageLayout::eGeneral)
		{
			layout = (access == AccessType::eReadPresent)
				? vk::ImageLayout::ePresentSrcKHR
				: vk::ImageLayout::eGeneral;
		}
		else if (oldLayout == ImageLayout::eOptimal)
		{
			layout = info.imageLayout;
		}
		// Check that layouts aren't mixed
		assert((ret.newLayout == vk::ImageLayout::eUndefined) || (ret.newLayout == layout));
		ret.newLayout = layout;
	}
	return ret;
}

PipelineBuilder& PipelineBuilder::AddShaderStage(vk::ShaderStageFlagBits stage, const vk::ShaderModule& module)
{
	vk::PipelineShaderStageCreateInfo shaderStageInfo({}, stage, module, "main");
	m_shaderStageInfos.push_back(shaderStageInfo);
	return *this;
}

PipelineBuilder& PipelineBuilder::SetVertexInputState(const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo)
{
	m_vertexInputInfo = vertexInputInfo;
	return *this;
}

PipelineBuilder& PipelineBuilder::SetInputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo& inputAssemblyInfo)
{
	m_inputAssemblyInfo = inputAssemblyInfo;
	return *this;
}

PipelineBuilder& PipelineBuilder::SetRasterizerState(const vk::PipelineRasterizationStateCreateInfo& rasterizerInfo)
{
	m_rasterizerInfo = rasterizerInfo;
	return *this;
}

PipelineBuilder& PipelineBuilder::SetMultisampleState(const vk::PipelineMultisampleStateCreateInfo& multisampleInfo)
{
	m_multisampleInfo = multisampleInfo;
	return *this;
}

PipelineBuilder& PipelineBuilder::SetDepthStencilState(const vk::PipelineDepthStencilStateCreateInfo& depthStencilInfo)
{
	m_depthStencilInfo = depthStencilInfo;
	return *this;
}

PipelineBuilder& PipelineBuilder::AddColorAttachment(const vk::PipelineColorBlendAttachmentState& blendInfo,
	vk::Format format)
{
	m_colorBlendAttachments.push_back(blendInfo);
	m_colorAttachmentFormats.push_back(format);
	return *this;
}

PipelineBuilder& PipelineBuilder::SetDepthAttachment(vk::Format format)
{
	m_depthAttachmentFormat = format;
	return *this;
}

PipelineBuilder& PipelineBuilder::SetStencilAttachment(vk::Format format)
{
	m_stencilAttachmentFormat = format;
	return *this;
}

vk::UniquePipeline PipelineBuilder::CreatePipeline(const vk::Device& device, const vk::PipelineLayout& layout)
{
	// Setting the viewport and scissor dynamically is standard, because the window size could change whenever
	std::array<vk::DynamicState, 2> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};
	vk::PipelineDynamicStateCreateInfo dynamicInfo({}, dynamicStates);

	// Color blend state
	vk::PipelineColorBlendStateCreateInfo colorBlendInfo({}, false,
		vk::LogicOp::eCopy, m_colorBlendAttachments);

	// Viewport state (to be set dynamically)
	vk::PipelineViewportStateCreateInfo viewportInfo({}, 1, nullptr, 1, nullptr);

	// Create the pipeline with an attachment for dynamic rendering
	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineInfo(
		vk::GraphicsPipelineCreateInfo(
			{}, m_shaderStageInfos, &m_vertexInputInfo, &m_inputAssemblyInfo, nullptr, &viewportInfo, 
			&m_rasterizerInfo, &m_multisampleInfo, &m_depthStencilInfo, &colorBlendInfo, &dynamicInfo, layout
		),
		vk::PipelineRenderingCreateInfo(
			{}, m_colorAttachmentFormats, m_depthAttachmentFormat, m_stencilAttachmentFormat
		)
	);
	vk::UniquePipeline ret = device.createGraphicsPipelineUnique(vk::PipelineCache(),
		pipelineInfo.get<vk::GraphicsPipelineCreateInfo>()).value;

	// Clear the Builder class and return
	*this = PipelineBuilder();
	return ret;
}
