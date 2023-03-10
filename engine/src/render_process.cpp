#include "toy3d/render_process.hpp"
#include "toy3d/shader.hpp"
#include "toy3d/context.hpp"
#include "toy3d/math.hpp"

namespace toy3d
{

	RenderProcess::RenderProcess()
	{
		layout = createLayout();
		CreateRenderPass();
		pipelineCache_ = createPipelineCache();
	}

	RenderProcess::~RenderProcess()
	{
		auto& device = Context::GetInstance().device;
		device.destroyRenderPass(renderPass);
		device.destroyPipelineLayout(layout);
		device.destroyPipeline(graphicsPipelineWithTriangleTopology);
		device.destroyPipeline(graphicsPipelineWithLineTopology);
		device.destroyPipelineCache(pipelineCache_);
	}

	void RenderProcess::CreateGraphicsPipeline(const Shader& shader)
	{
		graphicsPipelineWithTriangleTopology = createGraphicsPipeline(shader, vk::PrimitiveTopology::eTriangleList);
		graphicsPipelineWithLineTopology = createGraphicsPipeline(shader, vk::PrimitiveTopology::eLineList);
	}

	void RenderProcess::CreateRenderPass()
	{
		renderPass = createRenderPass();
	}

	vk::PipelineLayout RenderProcess::createLayout()
	{
		vk::PipelineLayoutCreateInfo createInfo;
		auto range = Context::GetInstance().shader->GetPushConstantRange();
		createInfo.setSetLayouts(Context::GetInstance().shader->GetDescriptorSetLayouts())
			.setPushConstantRanges(range);
		return Context::GetInstance().device.createPipelineLayout(createInfo);
	}

	vk::Pipeline RenderProcess::createGraphicsPipeline(const Shader& shader, vk::PrimitiveTopology topology)
	{
		auto& context = Context::GetInstance();

		vk::GraphicsPipelineCreateInfo createInfo;

		// 0.shader prepare
		std::array<vk::PipelineShaderStageCreateInfo, 2> stageCreateInfos;
		stageCreateInfos[0].setModule(shader.GetVertexModule())
			.setPName("main")
			.setStage(vk::ShaderStageFlagBits::eVertex); 
		stageCreateInfos[1].setModule(shader.GetFragModule())
			.setPName("main")
			.setStage(vk::ShaderStageFlagBits::eFragment);

		// 1.Vertex Input
		vk::PipelineVertexInputStateCreateInfo inputState;
		auto binding = Vec2d::GetBingDescription();
		auto attribute = Vec2d::GetAttributeDescription();
		inputState.setVertexBindingDescriptions(binding)
			.setVertexAttributeDescriptions(attribute);

		// 2.Vertex Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAsm;
		inputAsm.setPrimitiveRestartEnable(false)
			.setTopology(topology);

		// 3.Viewport and scissor
		vk::PipelineViewportStateCreateInfo viewportState;
		vk::Viewport viewport(0, 0, context.swapchain->GetExtent().width, context.swapchain->GetExtent().height, 0, 1);
		vk::Rect2D scissor({ 0, 0 }, context.swapchain->GetExtent());
		viewportState.setViewports(viewport)
			.setScissors(scissor);

		// 4.Rasterization
		vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
		rasterizationInfo.setRasterizerDiscardEnable(false)
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eClockwise)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1)
			.setDepthClampEnable(false);

		// 5.Mutisample
		vk::PipelineMultisampleStateCreateInfo mutissampleInfo;
		mutissampleInfo.setSampleShadingEnable(false)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1);

		// 6.depth and stencil test

		// 7.color blending
		/*
		 * newRGB = (srcFactor * srcRGB) <op> (dstFactor * dstRGB)
		 * newA = (srcFactor * srcA) <op> (dstFactor * dstA)
		 *
		 * newRGB = 1 * srcRGB + (1 - srcA) * dstRGB
		 * newA = srcA === 1 * srcA + 0 * dstA
		 */
		vk::PipelineColorBlendAttachmentState attachState;
		attachState.setBlendEnable(true)
			.setColorWriteMask(vk::ColorComponentFlagBits::eR
				| vk::ColorComponentFlagBits::eG
				| vk::ColorComponentFlagBits::eB
				| vk::ColorComponentFlagBits::eA)
			.setSrcColorBlendFactor(vk::BlendFactor::eOne)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusConstantAlpha)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setAlphaBlendOp(vk::BlendOp::eAdd)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero);

		vk::PipelineColorBlendStateCreateInfo blendInfo;
		blendInfo.setLogicOpEnable(false)
			.setAttachments(attachState);

		// 8.create graphics pipeline
		createInfo.setPVertexInputState(&inputState)
			.setPInputAssemblyState(&inputAsm)
			.setStages(stageCreateInfos)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizationInfo)
			.setPMultisampleState(&mutissampleInfo)
			.setPColorBlendState(&blendInfo)
			.setRenderPass(renderPass)
			.setLayout(layout);

		auto result = Context::GetInstance().device.createGraphicsPipeline(pipelineCache_, createInfo);
		if (result.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("create graphics pipline failed");
		}

		return result.value;
	}

	vk::RenderPass RenderProcess::createRenderPass()
	{
		vk::RenderPassCreateInfo createInfo;
		vk::AttachmentDescription attachDescription;
		attachDescription.setFormat(Context::GetInstance().swapchain->GetFormat().format)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setSamples(vk::SampleCountFlagBits::e1);

		vk::AttachmentReference reference;
		reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setAttachment(0);

		vk::SubpassDescription subpass;
		subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(reference);

		vk::SubpassDependency dependency;
		dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

		createInfo.setAttachments(attachDescription)
			.setSubpasses(subpass)
			.setDependencies(dependency);

		return Context::GetInstance().device.createRenderPass(createInfo);
	}

	vk::PipelineCache RenderProcess::createPipelineCache()
	{
		vk::PipelineCacheCreateInfo createInfo;
		createInfo.setInitialDataSize(0);

		return Context::GetInstance().device.createPipelineCache(createInfo);
	}

}
