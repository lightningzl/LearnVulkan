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

		vk::Pipeline graphicsPipeline;
		vk::PipelineLayout layout;
		vk::RenderPass renderPass;

		void RecreateGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource);
		void RecreateRenderPass();

	private:
		vk::PipelineLayout createLayout();
		vk::Pipeline createGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource);
		vk::RenderPass createRenderPass();
	};


}
