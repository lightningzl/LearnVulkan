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
		std::vector<std::unique_ptr<Buffer>> hostUniformBuffer_;
		std::vector<std::unique_ptr<Buffer>> deviceUniformBuffer_;

		vk::DescriptorPool descriptorPool_;
		std::vector<vk::DescriptorSet> sets_;

		void createFence();
		void createSemaphores();
		void createCmdBuffers();
		void createVertexBuffer();
		void createUniformBuffers();
		void bufferVertexData();
		void bufferUniformData();
		void createDescriptorPool();
		void allocateSets();
		void updateSets();

		void copyBuffer(vk::Buffer& src, vk::Buffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
	};
}
