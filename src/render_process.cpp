#include "toy2d/render_process.hpp"
#include "toy2d/shader.hpp"
#include "toy2d/context.hpp"
#include "toy2d/math.hpp"

namespace toy2d
{

	RenderProcess::RenderProcess()
	{
		layout = createLayout();
		renderPass = createRenderPass();
		graphicsPipeline = nullptr;
	}

	RenderProcess::~RenderProcess()
	{
		auto& device = Context::GetInstance().device;
		device.destroyRenderPass(renderPass);
		device.destroyPipelineLayout(layout);
		device.destroyPipeline(graphicsPipeline);
	}

	void RenderProcess::RecreateGraphicsPipeline(const Shader& shader)
	{
		if (graphicsPipeline)
		{
			Context::GetInstance().device.destroyPipeline(graphicsPipeline);
		}
		graphicsPipeline = createGraphicsPipeline(shader);
	}

	void RenderProcess::RecreateRenderPass()
	{
		if (renderPass)
		{
			Context::GetInstance().device.destroyRenderPass(renderPass);
		}
		renderPass = createRenderPass();
	}

	vk::PipelineLayout RenderProcess::createLayout()
	{
		vk::PipelineLayoutCreateInfo createInfo;
		createInfo.setSetLayouts(Context::GetInstance().shader->GetDescriptorSetLayouts());
		return Context::GetInstance().device.createPipelineLayout(createInfo);
	}

	vk::Pipeline RenderProcess::createGraphicsPipeline(const Shader& shader)
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
		auto binding = Vec::GetBingDescription();
		auto attribute = Vec::GetAttributeDescription();
		inputState.setVertexBindingDescriptions(binding)
			.setVertexAttributeDescriptions(attribute);

		// 2.Vertex Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAsm;
		inputAsm.setPrimitiveRestartEnable(false)
			.setTopology(vk::PrimitiveTopology::eTriangleList);

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
		vk::PipelineColorBlendAttachmentState attachState;
		attachState.setBlendEnable(false)
			.setColorWriteMask(vk::ColorComponentFlagBits::eR
				| vk::ColorComponentFlagBits::eG
				| vk::ColorComponentFlagBits::eB
				| vk::ColorComponentFlagBits::eA);

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

		auto result = Context::GetInstance().device.createGraphicsPipeline(nullptr, createInfo);
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

}
