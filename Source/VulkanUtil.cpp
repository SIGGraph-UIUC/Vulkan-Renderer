#include "VulkanUtil.h"

#include <fstream>

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

// Helper function to insert an image memory barrier into the command buffer
void InsertImageMemoryBarrier(const vk::CommandBuffer& commandBuffer, const vk::Image& image,
	vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::ImageLayout oldLayout, 
	vk::ImageLayout newLayout, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask, 
	const vk::ImageSubresourceRange& subresourceRange)
{
	vk::ImageMemoryBarrier imageMemoryBarrier(srcAccessMask, dstAccessMask, oldLayout, newLayout,
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, subresourceRange);
	commandBuffer.pipelineBarrier(srcStageMask, dstStageMask, {}, {}, {}, imageMemoryBarrier);
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
	return 1.0f * count.QuadPart / s_frequency.QuadPart;
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
