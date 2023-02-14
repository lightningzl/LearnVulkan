#include "toy2d/renderer.hpp"
#include "toy2d/context.hpp"
#include "toy2d/vertex.hpp"
#include <iostream>

namespace toy2d
{
	
	const std::array<Vertex, 3> vertexes =
	{
		Vertex{0.0, -0.5},
		Vertex{0.5, 0.5},
		Vertex{-0.5, 0.5},
	};

	Renderer::Renderer(int maxFlightCount /*= 2*/)
		: maxFlightCount_(maxFlightCount), curFrame_(0)
	{
		createFence();
		createSemaphores();
		createCmdBuffers();
		createVertexBuffer();
		bufferVertexData();
	}

	Renderer::~Renderer()
	{
		hostVertexBuffer_.reset();
		deviceVertexBuffer_.reset();
		auto& device = Context::GetInstance().device;
		for (auto& sem : imageAvliableSemaphores_)
		{
			device.destroySemaphore(sem);
		}
		for (auto& sem : renderFinishSemaphores_)
		{
			device.destroySemaphore(sem);
		}
		for (auto fence : fences_)
		{
			device.destroyFence(fence);
		}
	}

	void Renderer::DrawTriangle()
	{
		auto& ctx = Context::GetInstance();
		auto& device = ctx.device;
		if (device.waitForFences(fences_[curFrame_], true, std::numeric_limits<std::uint64_t>::max()) != vk::Result::eSuccess)
		{
			throw std::runtime_error("wait for fence failed");
		}
		device.resetFences(fences_[curFrame_]);
		
		auto& swapchain = ctx.swapchain;
		auto resultValue = device.acquireNextImageKHR(swapchain->swapchain, std::numeric_limits<uint64_t>::max(), imageAvliableSemaphores_[curFrame_], nullptr);
		if (resultValue.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("wait for image in swapchain failed");
		}
		auto imageIndex = resultValue.value;

		auto& cmdMgr = ctx.commandManager;
		cmdBufs_[curFrame_].reset();

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmdBufs_[curFrame_].begin(beginInfo);
		{
			vk::ClearValue clearValue;
			clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
			vk::RenderPassBeginInfo renderPassBegin;
			renderPassBegin.setRenderPass(ctx.renderProcess->renderPass)
				.setRenderArea(vk::Rect2D({}, swapchain->GetExtent()))
				.setFramebuffer(swapchain->framebuffers[imageIndex])
				.setClearValues(clearValue);
			cmdBufs_[curFrame_].beginRenderPass(renderPassBegin, {});
			{
				cmdBufs_[curFrame_].bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipeline);
				vk::DeviceSize offset = 0;
				cmdBufs_[curFrame_].bindVertexBuffers(0, deviceVertexBuffer_->buffer, offset);
				cmdBufs_[curFrame_].draw(3, 1, 0, 0);
			}
			cmdBufs_[curFrame_].endRenderPass();
		}
		cmdBufs_[curFrame_].end();

		vk::SubmitInfo submit;
		vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		submit.setCommandBuffers(cmdBufs_[curFrame_])
			.setWaitSemaphores(imageAvliableSemaphores_[curFrame_])
			.setSignalSemaphores(renderFinishSemaphores_[curFrame_])
			.setWaitDstStageMask(flags);
		ctx.graphcisQueue.submit(submit, fences_[curFrame_]);

		vk::PresentInfoKHR presentInfo;
		presentInfo.setImageIndices(imageIndex)
			.setSwapchains(swapchain->swapchain)
			.setWaitSemaphores(renderFinishSemaphores_[curFrame_]);
		if (ctx.presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
		{
			throw std::runtime_error("present queue execute failed");
		}

		curFrame_ = (curFrame_ + 1) % maxFlightCount_;
	}

	void Renderer::createFence()
	{
		fences_.resize(maxFlightCount_, nullptr);

		for (auto& fence : fences_)
		{
			vk::FenceCreateInfo createInfo;
			createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
			fence = Context::GetInstance().device.createFence(createInfo);
		}	
	}

	void Renderer::createSemaphores()
	{
		auto& device = Context::GetInstance().device;

		vk::SemaphoreCreateInfo createInfo;

		imageAvliableSemaphores_.resize(maxFlightCount_);
		renderFinishSemaphores_.resize(maxFlightCount_);

		for (auto& sem : imageAvliableSemaphores_)
		{
			sem = device.createSemaphore(createInfo);
		}

		for (auto& sem : renderFinishSemaphores_)
		{
			sem = device.createSemaphore(createInfo);
		}
	}

	void Renderer::createCmdBuffers()
	{
		cmdBufs_.resize(maxFlightCount_);

		for (auto& cmd : cmdBufs_)
		{
			cmd = Context::GetInstance().commandManager->CreateOneCommandBuffer();
		}
	}

	void Renderer::createVertexBuffer()
	{
		hostVertexBuffer_.reset(new Buffer(sizeof(vertexes), vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		deviceVertexBuffer_.reset(new Buffer(sizeof(vertexes), vk::BufferUsageFlagBits::eVertexBuffer
			| vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal));
	}

	void Renderer::bufferVertexData()
	{
		void* ptr = Context::GetInstance().device.mapMemory(hostVertexBuffer_->memory, 0, hostVertexBuffer_->size);
		memcpy(ptr, vertexes.data(), sizeof(vertexes));
		Context::GetInstance().device.unmapMemory(hostVertexBuffer_->memory);

		auto cmdBuf = Context::GetInstance().commandManager->CreateOneCommandBuffer();

		vk::CommandBufferBeginInfo begin;
		begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmdBuf.begin(begin);
		{
			vk::BufferCopy region;
			region.setSize(hostVertexBuffer_->size)
				.setSrcOffset(0)
				.setDstOffset(0);
			cmdBuf.copyBuffer(hostVertexBuffer_->buffer, deviceVertexBuffer_->buffer, region);
		}
		cmdBuf.end();

		vk::SubmitInfo submit;
		submit.setCommandBuffers(cmdBuf);
		Context::GetInstance().graphcisQueue.submit(submit);
		Context::GetInstance().device.waitIdle();
		Context::GetInstance().commandManager->FreeCmd(cmdBuf);
	}

}
