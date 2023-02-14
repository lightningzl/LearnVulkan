#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d
{

	class CommandManager
	{
	public:
		CommandManager();
		~CommandManager();

		vk::CommandBuffer CreateOneCommandBuffer();
		std::vector<vk::CommandBuffer> CreateCommandBuffers(std::uint32_t count);
		void ResetCmds();
		void FreeCmd(vk::CommandBuffer buffer);

	private:
		vk::CommandPool pool_;

		vk::CommandPool createCommandPool();

	};



}
