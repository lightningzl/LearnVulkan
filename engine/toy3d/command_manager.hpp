#pragma once

#include "vulkan/vulkan.hpp"
#include <functional>

namespace toy3d
{

	class CommandManager
	{
	public:
		using RecordCmdFunc = std::function<void(vk::CommandBuffer&)>;

		CommandManager();
		~CommandManager();

		vk::CommandBuffer CreateOneCommandBuffer();
		std::vector<vk::CommandBuffer> CreateCommandBuffers(std::uint32_t count);
		void ResetCmds();
		void FreeCmd(vk::CommandBuffer buffer);

		void ExecuteCmd(vk::Queue queue, RecordCmdFunc func);
	private:
		vk::CommandPool pool_;

		vk::CommandPool createCommandPool();

	};



}
