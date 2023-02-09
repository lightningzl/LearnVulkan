#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d
{
	class Shader
	{
	public:
		Shader(const std::string& vertexSource, const std::string& fragmentSource);
		virtual ~Shader();

		vk::ShaderModule vertexModule;
		vk::ShaderModule fragmentModule;

		std::vector<vk::PipelineShaderStageCreateInfo>& GetStage();

	private:
		std::vector<vk::PipelineShaderStageCreateInfo> stage_;

		void InitStage();

	};
}