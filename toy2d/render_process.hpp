#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d
{
	class Shader;
	class RenderProcess
	{
	public:
		RenderProcess();
		~RenderProcess();

		vk::Pipeline graphicsPipelineWithTriangleTopology = nullptr;
		vk::Pipeline graphicsPipelineWithLineTopology = nullptr;
		vk::PipelineLayout layout = nullptr;
		vk::RenderPass renderPass = nullptr;

		void CreateGraphicsPipeline(const Shader& shader);
		void CreateRenderPass();

	private:
		vk::PipelineCache pipelineCache_ = nullptr;

		vk::PipelineLayout createLayout();
		vk::Pipeline createGraphicsPipeline(const Shader& shader, vk::PrimitiveTopology topology);
		vk::RenderPass createRenderPass();
		vk::PipelineCache createPipelineCache();
	};


}
