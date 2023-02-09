#include "toy2d/shader.hpp"
#include "toy2d/context.hpp"

namespace toy2d
{
	Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource)
	{
		vk::ShaderModuleCreateInfo createInfo;

		createInfo.codeSize = vertexSource.size();
		createInfo.pCode = (uint32_t*)vertexSource.data();
		vertexModule = Context::GetInstance().device.createShaderModule(createInfo);

		createInfo.codeSize = fragmentSource.size();
		createInfo.pCode = (uint32_t*)fragmentSource.data();
		fragmentModule = Context::GetInstance().device.createShaderModule(createInfo);
		
		InitStage();
	}

	Shader::~Shader()
	{
		Context::GetInstance().device.destroyShaderModule(vertexModule);
		Context::GetInstance().device.destroyShaderModule(fragmentModule);
	}

	std::vector<vk::PipelineShaderStageCreateInfo>& Shader::GetStage()
	{
		return stage_;
	}

	void Shader::InitStage()
	{
		stage_.resize(2);
		stage_[0].setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(vertexModule)
			.setPName("main");
		stage_[1].setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(fragmentModule)
			.setPName("main");
	}

}
