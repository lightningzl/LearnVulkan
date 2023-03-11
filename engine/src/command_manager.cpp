#include "toy3d/command_manager.hpp"
#include "toy3d/context.hpp"

namespace toy3d
{
	CommandManager::CommandManager()
	{
		pool_ = createCommandPool();
	}

	CommandManager::~CommandManager()
	{
		Context::GetInstance().device.destroyCommandPool(pool_);
	}

	vk::CommandBuffer CommandManager::CreateOneCommandBuffer()
	{
		return CreateCommandBuffers(1)[0];
	}

	std::vector<vk::CommandBuffer> CommandManager::CreateCommandBuffers(std::uint32_t count)
	{
		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.setCommandPool(pool_)
			.setCommandBufferCount(1)
			.setLevel(vk::CommandBufferLevel::ePrimary);

		return Context::GetInstance().device.allocateCommandBuffers(allocInfo);
	}

	void CommandManager::ResetCmds()
	{
		Context::GetInstance().device.resetCommandPool(pool_);
	}

	void CommandManager::FreeCmd(vk::CommandBuffer buffer)
	{
		Context::GetInstance().device.freeCommandBuffers(pool_, buffer);
	}

	void CommandManager::ExecuteCmd(vk::Queue queue, RecordCmdFunc func)
	{
		auto cmdBuf = CreateOneCommandBuffer();

		vk::CommandBufferBeginInfo begin;
		begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmdBuf.begin(begin);
		if (func)
		{
			func(cmdBuf);
		}
		cmdBuf.end();

		vk::SubmitInfo submit;
		submit.setCommandBuffers(cmdBuf);
		queue.submit(submit);
		queue.waitIdle();
		Context::GetInstance().device.waitIdle();
		FreeCmd(cmdBuf);
	}

	vk::CommandPool CommandManager::createCommandPool()
	{
		auto& ctxInstance = Context::GetInstance();
		
		vk::CommandPoolCreateInfo createInfo;

		createInfo.setQueueFamilyIndex(ctxInstance.queueInfo.graphicsIndex.value())
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

		return ctxInstance.device.createCommandPool(createInfo);
	}

}
