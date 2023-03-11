#pragma once

#include "vulkan/vulkan.hpp"

namespace toy3d
{
	class Shader
	{
	public:
		Shader(const std::vector<char>& vertexSource, const std::vector<char>& fragmentSource);
		virtual ~Shader();

		vk::ShaderModule GetVertexModule() const { return vertexModule_; }
		vk::ShaderModule GetFragModule() const { return fragmentModule_; }

		const std::vector<vk::DescriptorSetLayout>& GetDescriptorSetLayouts() const { return layouts_; }
		std::vector<vk::PushConstantRange> GetPushConstantRange() const;

	private:
		vk::ShaderModule vertexModule_;
		vk::ShaderModule fragmentModule_;
		std::vector<vk::DescriptorSetLayout> layouts_;

		void initDescriptorSetLayouts();
	};
}