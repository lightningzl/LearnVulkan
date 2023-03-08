#pragma once

#include "vulkan/vulkan.hpp"
#include "buffer.hpp"
#include "math.hpp"
#include "texture.hpp"

namespace toy2d
{
	class Renderer
	{
	public:
		Renderer(int maxFlightCount);
		~Renderer();

		void SetProject(int right, int left, int bottom, int top, int far, int near);
		void DrawRect(const Rect& rect, Texture& texture);
		void DrawLine(const Vec& p1, const Vec& p2);
		void SetDrawColor(const Color& color);

		void StartRender();
		void EndRender();

	private:

		struct MVP
		{
			Mat4 project;
			Mat4 view;
		};

		int maxFlightCount_;
		int curFrame_;
		uint32_t imageIndex_;
		std::vector<vk::Fence> fences_;
		std::vector<vk::Semaphore> imageAvliableSemaphores_;
		std::vector<vk::Semaphore> renderFinishSemaphores_;
		std::vector<vk::CommandBuffer> cmdBufs_;
		std::unique_ptr<Buffer> rectVerticesBuffer_;
		std::unique_ptr<Buffer> rectIndicesBuffer_;
		std::unique_ptr<Buffer> lineVerticesBuffer_;
 		Mat4 projectMat_;
		Mat4 viewMat_;
		std::vector<std::unique_ptr<Buffer>> uniformBuffers_;
		std::vector<std::unique_ptr<Buffer>> deviceUniformBuffers_;
		std::vector<std::unique_ptr<Buffer>> colorBuffers_;
		std::vector<std::unique_ptr<Buffer>> deviceColorBuffers_;
		std::vector<DescriptorSetManager::SetInfo> descriptorSets_;

		Texture* whiteTexture;

		void createFences();
		void createSemaphores();
		void createCmdBuffers();
		void createBuffers();
		void createUniformBuffers(int flightCount);

		void bufferRectData();
		void bufferRectVertexData();
		void bufferRectIndicesData();
		void bufferLineData(const Vec& p1, const Vec& p2);

		void bufferMVPData();
		void initMats();
		void updateDescriptorSets();
		void transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size);
		void createWhiteTexture();
	};
}
