#pragma once

#include "vulkan/vulkan.hpp"
#include "buffer.hpp"
#include "math.hpp"

namespace toy2d
{
	class Renderer
	{
	public:
		Renderer(int maxFlightCount = 2);
		~Renderer();

		void SetProject(int right, int left, int bottom, int top, int far, int near);
		void DrawRect(const Rect& rect);
		void SetDrawColor(const Color& color);

	private:

		struct MVP
		{
			Mat4 project;
			Mat4 view;
			Mat4 model;
		};

		int maxFlightCount_;
		int curFrame_;
		std::vector<vk::Fence> fences_;
		std::vector<vk::Semaphore> imageAvliableSemaphores_;
		std::vector<vk::Semaphore> renderFinishSemaphores_;
		std::vector<vk::CommandBuffer> cmdBufs_;
		std::unique_ptr<Buffer> verticesBuffer_;
		std::unique_ptr<Buffer> indicesBuffer_;
		Mat4 projectMat_;
		Mat4 viewMat_;
		std::vector<std::unique_ptr<Buffer>> uniformBuffers_;
		std::vector<std::unique_ptr<Buffer>> deviceUniformBuffers_;
		std::vector<std::unique_ptr<Buffer>> colorBuffers_;
		std::vector<std::unique_ptr<Buffer>> deviceColorBuffers_;

		vk::DescriptorPool descriptorPool1_;
		vk::DescriptorPool descriptorPool2_;
		std::pair<std::vector<vk::DescriptorSet>, std::vector<vk::DescriptorSet>> descriptorSets_;

		void createFences();
		void createSemaphores();
		void createCmdBuffers();
		void createBuffers();
		void createUniformBuffers(int flightCount);
		void bufferData();
		void bufferVertexData();
		void bufferIndicesData();
		void bufferMVPData(const Mat4& model);
		void initMats();
		void createDescriptorPool(int flightCount);
		void allocDescriptorSets(int flightCount);
		void updateDescriptorSets();
		void transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size);
	};
}
