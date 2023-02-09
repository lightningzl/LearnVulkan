#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d
{
	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void Render();

	private:
		vk::CommandPool cmdPool_;
		vk::CommandBuffer  cmdBuf_;

		vk::Semaphore imageAvaliable_;
		vk::Semaphore imageDrawFinish_;
		vk::Fence cmdAvaliableFence_;

		void initCmdPool();
		void allocCmdBuf();
		void createSems();
		void createFence();
	};
}
