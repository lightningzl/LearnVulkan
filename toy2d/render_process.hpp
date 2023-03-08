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

		vk::Pipeline graphicsPipeline = nullptr;
		vk::PipelineLayout layout = nullptr;
		vk::RenderPass renderPass = nullptr;

		void CreateGraphicsPipeline(const Shader& shader);
		void CreateRenderPass();

	private:
		vk::PipelineLayout createLayout();
		vk::Pipeline createGraphicsPipeline(const Shader& shader);
		vk::RenderPass createRenderPass();
	};


}
