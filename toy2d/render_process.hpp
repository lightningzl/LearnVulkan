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

		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		vk::RenderPass renderPass;

		void InitPipeline(int width, int height, Shader* shader);
		void InitPipelineLayout();
		void InitRenderPass();

	private:

	};


}
