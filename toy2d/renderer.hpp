#pragma once

#include "vulkan/vulkan.hpp"
#include "buffer.hpp"

namespace toy2d
{
	class Renderer
	{
	public:
		Renderer(int maxFlightCount = 2);
		~Renderer();

		void DrawTriangle();

	private:
		int maxFlightCount_;
		int curFrame_;
		std::vector<vk::Fence> fences_;
		std::vector<vk::Semaphore> imageAvliableSemaphores_;
		std::vector<vk::Semaphore> renderFinishSemaphores_;
		std::vector<vk::CommandBuffer> cmdBufs_;

		std::unique_ptr<Buffer> hostVertexBuffer_;
		std::unique_ptr<Buffer> deviceVertexBuffer_;

		vk::CommandPool cmdPool_;
		vk::CommandBuffer  cmdBuf_;

		vk::Semaphore imageAvaliable_;
		vk::Semaphore imageDrawFinish_;
		vk::Fence cmdAvaliableFence_;

		void createFence();
		void createSemaphores();
		void createCmdBuffers();
		void createVertexBuffer();
		void bufferVertexData();
	};
}
