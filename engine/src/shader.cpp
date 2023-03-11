#include "toy3d/shader.hpp"
#include "toy3d/context.hpp"
#include "toy3d/math.hpp"

namespace toy3d
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

	std::vector<vk::PushConstantRange> Shader::GetPushConstantRange() const
	{
		std::vector<vk::PushConstantRange> ranges(2);
		ranges[0].setOffset(0)
			.setSize(sizeof(Mat4))
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);
		ranges[1].setOffset(sizeof(Mat4))
			.setSize(sizeof(Color))
			.setStageFlags(vk::ShaderStageFlagBits::eFragment);
		return ranges;
	}

	void Shader::initDescriptorSetLayouts()
	{
		vk::DescriptorSetLayoutCreateInfo createInfo;
		std::vector<vk::DescriptorSetLayoutBinding> bindings(1);
		bindings[0].setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex)
			.setDescriptorCount(1);

		createInfo.setBindings(bindings);
		layouts_.push_back(Context::GetInstance().device.createDescriptorSetLayout(createInfo));

		bindings.resize(1);
		bindings[0].setBinding(0)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)
			.setDescriptorCount(1);

		createInfo.setBindings(bindings);
		layouts_.push_back(Context::GetInstance().device.createDescriptorSetLayout(createInfo));
	}
}
