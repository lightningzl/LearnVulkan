#include "toy2d/shader.hpp"
#include "toy2d/context.hpp"

namespace toy2d
{
	Shader::Shader(const std::vector<char>& vertexSource, const std::vector<char>& fragmentSource)
	{
		vk::ShaderModuleCreateInfo vertexModuleCreateInfo, fragModuleCreateInfo;

		vertexModuleCreateInfo.codeSize = vertexSource.size();
		vertexModuleCreateInfo.pCode = (uint32_t*)vertexSource.data();
		fragModuleCreateInfo.codeSize = fragmentSource.size();
		fragModuleCreateInfo.pCode = (uint32_t*)fragmentSource.data();

		vertexModule_ = Context::GetInstance().device.createShaderModule(vertexModuleCreateInfo);
		fragmentModule_ = Context::GetInstance().device.createShaderModule(fragModuleCreateInfo);

		initDescriptorSetLayouts();
	}

	Shader::~Shader()
	{
		auto& device = Context::GetInstance().device;
		for (auto& layout : layouts_)
		{
			device.destroyDescriptorSetLayout(layout);
		}

		device.destroyShaderModule(vertexModule_);
		device.destroyShaderModule(fragmentModule_);
	}

	void Shader::initDescriptorSetLayouts()
	{
		vk::DescriptorSetLayoutCreateInfo createInfo;
		vk::DescriptorSetLayoutBinding binding;
		binding.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex)
			.setDescriptorCount(1);
		createInfo.setBindings(binding);
		layouts_.push_back(Context::GetInstance().device.createDescriptorSetLayout(createInfo));

		binding.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)
			.setDescriptorCount(1);
		createInfo.setBindings(binding);
		layouts_.push_back(Context::GetInstance().device.createDescriptorSetLayout(createInfo));
	}
}
