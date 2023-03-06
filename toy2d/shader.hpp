#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d
{
	class Shader
	{
	public:
		Shader(const std::vector<char>& vertexSource, const std::vector<char>& fragmentSource);
		virtual ~Shader();

		vk::ShaderModule GetVertexModule() const { return vertexModule_; }
		vk::ShaderModule GetFragModule() const { return fragmentModule_; }

		const std::vector<vk::DescriptorSetLayout>& GetDescriptorSetLayouts() const { return layouts_; }
		vk::PushConstantRange GetPushConstantRange() const;

	private:
		vk::ShaderModule vertexModule_;
		vk::ShaderModule fragmentModule_;
		std::vector<vk::DescriptorSetLayout> layouts_;

		void initDescriptorSetLayouts();
	};
}