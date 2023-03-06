#include "toy2d/renderer.hpp"
#include "toy2d/context.hpp"
#include "toy2d/shader.hpp"
#include <iostream>

namespace toy2d
{
	const size_t MVPSIZE = sizeof(float) * 4 * 4 * 2;
	const size_t COLORSIZE = sizeof(float) * 3;

	Renderer::Renderer(int maxFlightCount /*= 2*/)
		: maxFlightCount_(maxFlightCount), curFrame_(0)
	{
		createFences();
		createSemaphores();
		createCmdBuffers();
		createBuffers();
		bufferVertexData();
		createUniformBuffers(maxFlightCount);
		bufferData();
		createDescriptorPool(maxFlightCount);
		allocDescriptorSets(maxFlightCount);
		updateDescriptorSets();
		initMats();

		SetDrawColor(Color{ 0, 0, 0 });
	}

	Renderer::~Renderer()
	{
		auto& device = Context::GetInstance().device;
		device.destroyDescriptorPool(descriptorPool_);
		uniformBuffers_.clear();
		deviceUniformBuffers_.clear();
		colorBuffers_.clear();
		deviceColorBuffers_.clear();
		verticesBuffer_.reset();
		indicesBuffer_.reset();
		for (auto& sem : imageAvliableSemaphores_)
		{
			device.destroySemaphore(sem);
		}
		for (auto& sem : renderFinishSemaphores_)
		{
			device.destroySemaphore(sem);
		}
		for (auto& fence : fences_)
		{
			device.destroyFence(fence);
		}
	}

	void Renderer::SetProject(int right, int left, int bottom, int top, int far, int near)
	{
		projectMat_ = Mat4::CreateOrtho(left, right, top, bottom, near, far);
		bufferMVPData();
	}

	void Renderer::DrawRect(const Rect& rect)
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
			cmdBufs_[curFrame_].beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
			{
				cmdBufs_[curFrame_].bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipeline);
				vk::DeviceSize offset = 0;
				cmdBufs_[curFrame_].bindVertexBuffers(0, verticesBuffer_->buffer, offset);
				cmdBufs_[curFrame_].bindIndexBuffer(indicesBuffer_->buffer, 0, vk::IndexType::eUint32);

				cmdBufs_[curFrame_].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
					Context::GetInstance().renderProcess->layout,
					0, descriptorSets_[curFrame_], {});
				auto model = Mat4::CreateTranslate(rect.position).Mul(Mat4::CreateScale(rect.size));
				cmdBufs_[curFrame_].pushConstants(Context::GetInstance().renderProcess->layout,
					vk::ShaderStageFlagBits::eVertex, 0, sizeof(Mat4), model.GetData());
				cmdBufs_[curFrame_].drawIndexed(6, 1, 0, 0, 0);
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

	void Renderer::SetDrawColor(const Color& color)
	{
		for (int i = 0; i < colorBuffers_.size(); i++)
		{
			auto& buffer = colorBuffers_[i];
			memcpy(buffer->map, (void*)&color, COLORSIZE);
			transformBuffer2Device(*buffer, *deviceColorBuffers_[i], 0, 0, buffer->size);
		}
	}

	void Renderer::createFences()
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

	void Renderer::createBuffers()
	{
		verticesBuffer_.reset(new Buffer(vk::BufferUsageFlagBits::eVertexBuffer,
			sizeof(float) * 8,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		indicesBuffer_.reset(new Buffer(vk::BufferUsageFlagBits::eIndexBuffer,
			sizeof(float) * 6,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
	}

	void Renderer::createUniformBuffers(int flightCount)
	{
		uniformBuffers_.resize(flightCount);
		for (auto& buffer : uniformBuffers_)
		{
			buffer.reset(new Buffer(vk::BufferUsageFlagBits::eTransferSrc,
				MVPSIZE,
				vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible));
		}

		deviceUniformBuffers_.resize(flightCount);
		for (auto& buffer : deviceUniformBuffers_)
		{
			buffer.reset(new Buffer(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
				MVPSIZE,
				vk::MemoryPropertyFlagBits::eDeviceLocal));
		}

		colorBuffers_.resize(flightCount);
		for (auto& buffer : colorBuffers_)
		{
			buffer.reset(new Buffer(vk::BufferUsageFlagBits::eTransferSrc,
				COLORSIZE,
				vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible));
		}

		deviceColorBuffers_.resize(flightCount);
		for (auto& buffer : deviceColorBuffers_)
		{
			buffer.reset(new Buffer(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
				COLORSIZE,
				vk::MemoryPropertyFlagBits::eDeviceLocal));
		}
	}

	void Renderer::bufferData()
	{
		bufferVertexData();
		bufferIndicesData();
	}

	void Renderer::bufferVertexData()
	{
		Vec vertices[] = 
		{
			Vec{{-0.5, -0.5}},
			Vec{{0.5, -0.5}},
			Vec{{0.5, 0.5}},
			Vec{{-0.5, 0.5}},
		};
		memcpy(verticesBuffer_->map, vertices, sizeof(vertices));
	}

	void Renderer::bufferIndicesData()
	{
		uint32_t indices[] = 
		{
			0, 1, 3,
			1, 2, 3,
		};
		memcpy(indicesBuffer_->map, indices, sizeof(indices));
	}

	void Renderer::bufferMVPData()
	{
		MVP mvp;
		mvp.project = projectMat_;
		mvp.view = viewMat_;
		for (int i = 0; i < uniformBuffers_.size(); i++)
		{
			auto& buffer = uniformBuffers_[i];
			memcpy(buffer->map, (void*)&mvp, sizeof(mvp));
			transformBuffer2Device(*buffer, *deviceUniformBuffers_[i], 0, 0, buffer->size);
		}
	}

	void Renderer::initMats()
	{
		viewMat_ = Mat4::CreateIdentity();
		projectMat_ = Mat4::CreateIdentity();
	}

	void Renderer::createDescriptorPool(int flightCount)
	{
		vk::DescriptorPoolCreateInfo createInfo;
		vk::DescriptorPoolSize poolSize;
		poolSize.setType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(flightCount);
		std::vector<vk::DescriptorPoolSize> sizes(2, poolSize);
		createInfo.setMaxSets(flightCount)
			.setPoolSizes(sizes);
		descriptorPool_ = Context::GetInstance().device.createDescriptorPool(createInfo);
	}

	void Renderer::allocDescriptorSets(int flightCount)
	{
		std::vector layouts(flightCount, Context::GetInstance().shader->GetDescriptorSetLayouts()[0]);
		vk::DescriptorSetAllocateInfo allocInfo;

		allocInfo.setDescriptorPool(descriptorPool_)
			.setSetLayouts(layouts);
		descriptorSets_ = Context::GetInstance().device.allocateDescriptorSets(allocInfo);
	}

	void Renderer::updateDescriptorSets()
	{
		for (int i = 0; i < descriptorSets_.size(); i++)
		{
			vk::DescriptorBufferInfo bufferInfo1;
			bufferInfo1.setBuffer(deviceUniformBuffers_[i]->buffer)
				.setOffset(0)
				.setRange(MVPSIZE);

			std::vector<vk::WriteDescriptorSet> writerInfos(2);
			writerInfos[0].setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setBufferInfo(bufferInfo1)
				.setDstBinding(0)
				.setDstSet(descriptorSets_[i])
				.setDstArrayElement(0)
				.setDescriptorCount(1);

			vk::DescriptorBufferInfo bufferInfo2;
			bufferInfo2.setBuffer(deviceColorBuffers_[i]->buffer)
				.setOffset(0)
				.setRange(COLORSIZE);

			writerInfos[1].setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setBufferInfo(bufferInfo2)
				.setDstBinding(1)
				.setDstSet(descriptorSets_[i])
				.setDstArrayElement(0)
				.setDescriptorCount(1);

			Context::GetInstance().device.updateDescriptorSets(writerInfos, {});
		}
	}

	void Renderer::transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size)
	{
		auto cmdBuf = Context::GetInstance().commandManager->CreateOneCommandBuffer();

		vk::CommandBufferBeginInfo begin;
		begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmdBuf.begin(begin);
		{
			vk::BufferCopy region;
			region.setSize(size)
				.setSrcOffset(srcOffset)
				.setDstOffset(dstOffset);
			cmdBuf.copyBuffer(src.buffer, dst.buffer, region);
		}
		cmdBuf.end();

		vk::SubmitInfo submit;
		submit.setCommandBuffers(cmdBuf);
		Context::GetInstance().graphcisQueue.submit(submit);
		Context::GetInstance().graphcisQueue.waitIdle();
		Context::GetInstance().device.waitIdle();
		Context::GetInstance().commandManager->FreeCmd(cmdBuf);
	}

}
