#include "toy2d/renderer.hpp"
#include "toy2d/context.hpp"
#include "toy2d/shader.hpp"
#include <iostream>

namespace toy2d
{
	const size_t MVPSIZE = sizeof(Mat4) * 2;
	const size_t COLORSIZE = sizeof(Color);

	Renderer::Renderer(int maxFlightCount)
		: maxFlightCount_(maxFlightCount), curFrame_(0)
	{
		createFences();
		createSemaphores();
		createCmdBuffers();
		createBuffers();
		createUniformBuffers(maxFlightCount);
		descriptorSets_ = DescriptorSetManager::Instance().AllocBufferSets(maxFlightCount);
		updateDescriptorSets();
		initMats();
		createWhiteTexture();

		SetDrawColor(Color{ 0, 0, 0 });
	}

	Renderer::~Renderer()
	{
		auto& device = Context::GetInstance().device;
		uniformBuffers_.clear();
		deviceUniformBuffers_.clear();
		colorBuffers_.clear();
		deviceColorBuffers_.clear();
		rectVerticesBuffer_.reset();
		rectIndicesBuffer_.reset();
		lineVerticesBuffer_.reset();
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

	void Renderer::DrawRect(const Rect& rect, Texture& texture)
	{
		auto& ctx = Context::GetInstance();
		bufferRectData();

		cmdBufs_[curFrame_].bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipelineWithTriangleTopology);
		vk::DeviceSize offset = 0;
		cmdBufs_[curFrame_].bindVertexBuffers(0, rectVerticesBuffer_->buffer, offset);
		cmdBufs_[curFrame_].bindIndexBuffer(rectIndicesBuffer_->buffer, 0, vk::IndexType::eUint32);
		auto layout = ctx.renderProcess->layout;
		cmdBufs_[curFrame_].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, { descriptorSets_[curFrame_].set, texture.set.set }, {});
		auto model = Mat4::CreateTranslate(rect.position).Mul(Mat4::CreateScale(rect.size));
		cmdBufs_[curFrame_].pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(Mat4), model.GetData());
		cmdBufs_[curFrame_].drawIndexed(6, 1, 0, 0, 0);
	}

	void Renderer::DrawLine(const Vec& p1, const Vec& p2)
	{
		auto& ctx = Context::GetInstance();
		bufferLineData(p1, p2);

		cmdBufs_[curFrame_].bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipelineWithLineTopology);
		vk::DeviceSize offset = 0;
		cmdBufs_[curFrame_].bindVertexBuffers(0, lineVerticesBuffer_->buffer, offset);
		auto layout = ctx.renderProcess->layout;
		cmdBufs_[curFrame_].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, { descriptorSets_[curFrame_].set, whiteTexture->set.set }, {});
		auto model = Mat4::CreateIdentity();
		cmdBufs_[curFrame_].pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(Mat4), model.GetData());
		cmdBufs_[curFrame_].drawIndexed(2, 1, 0, 0, 0);
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

	void Renderer::StartRender()
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
		imageIndex_ = resultValue.value;

		auto& cmdMgr = ctx.commandManager;
		cmdBufs_[curFrame_].reset();

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmdBufs_[curFrame_].begin(beginInfo);

		vk::ClearValue clearValue;
		clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
		vk::RenderPassBeginInfo renderPassBegin;
		renderPassBegin.setRenderPass(ctx.renderProcess->renderPass)
			.setRenderArea(vk::Rect2D({}, swapchain->GetExtent()))
			.setFramebuffer(swapchain->framebuffers[imageIndex_])
			.setClearValues(clearValue);
		cmdBufs_[curFrame_].beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
	}

	void Renderer::EndRender()
	{
		auto& ctx = Context::GetInstance();
		auto& swapchain = ctx.swapchain;

		cmdBufs_[curFrame_].endRenderPass();
		cmdBufs_[curFrame_].end();
		vk::SubmitInfo submit;
		vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		submit.setCommandBuffers(cmdBufs_[curFrame_])
			.setWaitSemaphores(imageAvliableSemaphores_[curFrame_])
			.setSignalSemaphores(renderFinishSemaphores_[curFrame_])
			.setWaitDstStageMask(flags);
		ctx.graphcisQueue.submit(submit, fences_[curFrame_]);

		vk::PresentInfoKHR presentInfo;
		presentInfo.setImageIndices(imageIndex_)
			.setSwapchains(swapchain->swapchain)
			.setWaitSemaphores(renderFinishSemaphores_[curFrame_]);
		if (ctx.presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
		{
			throw std::runtime_error("present queue execute failed");
		}

		curFrame_ = (curFrame_ + 1) % maxFlightCount_;
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
		rectVerticesBuffer_.reset(new Buffer(vk::BufferUsageFlagBits::eVertexBuffer,
			sizeof(Vertex) * 4,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		rectIndicesBuffer_.reset(new Buffer(vk::BufferUsageFlagBits::eIndexBuffer,
			sizeof(uint32_t) * 6,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		lineVerticesBuffer_.reset(new Buffer(vk::BufferUsageFlagBits::eVertexBuffer,
			sizeof(Vertex) * 2,
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

	void Renderer::bufferRectData()
	{
		bufferRectVertexData();
		bufferRectIndicesData();
	}

	void Renderer::bufferRectVertexData()
	{
		Vertex vertices[] =
		{
			{Vec{-0.5, -0.5},	Vec{0, 0}},
			{Vec{0.5, -0.5},	Vec{1, 0}},
			{Vec{0.5, 0.5},		Vec{1, 1}},
			{Vec{-0.5, 0.5},	Vec{0, 1}},
		};
		memcpy(rectVerticesBuffer_->map, vertices, sizeof(vertices));
	}

	void Renderer::bufferRectIndicesData()
	{
		uint32_t indices[] = 
		{
			0, 1, 3,
			1, 2, 3,
		};
		memcpy(rectIndicesBuffer_->map, indices, sizeof(indices));
	}

	void Renderer::bufferLineData(const Vec& p1, const Vec& p2)
	{
		Vertex vertices[] =
		{
			{p1,	Vec{0, 0}},
			{p2,	Vec{0, 0}},
		};
		memcpy(lineVerticesBuffer_->map, vertices, sizeof(vertices));
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

	void Renderer::updateDescriptorSets()
	{
		for (int i = 0; i < descriptorSets_.size(); i++)
		{
			std::vector<vk::WriteDescriptorSet> writerInfos(2);

			vk::DescriptorBufferInfo bufferInfo1;
			bufferInfo1.setBuffer(deviceUniformBuffers_[i]->buffer)
				.setOffset(0)
				.setRange(MVPSIZE);

			writerInfos[0].setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setBufferInfo(bufferInfo1)
				.setDstBinding(0)
				.setDstSet(descriptorSets_[i].set)
				.setDstArrayElement(0)
				.setDescriptorCount(1);

			vk::DescriptorBufferInfo bufferInfo2;
			bufferInfo2.setBuffer(deviceColorBuffers_[i]->buffer)
				.setOffset(0)
				.setRange(COLORSIZE);

			writerInfos[1].setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setBufferInfo(bufferInfo2)
				.setDstBinding(1)
				.setDstSet(descriptorSets_[i].set)
				.setDstArrayElement(0)
				.setDescriptorCount(1);

			Context::GetInstance().device.updateDescriptorSets(writerInfos, {});
		}
	}

	void Renderer::transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size)
	{
		Context::GetInstance().commandManager->ExecuteCmd(Context::GetInstance().graphcisQueue,
			[&](vk::CommandBuffer cmdBuf){
				vk::BufferCopy region;
				region.setSize(size)
					.setSrcOffset(srcOffset)
					.setDstOffset(dstOffset);
				cmdBuf.copyBuffer(src.buffer, dst.buffer, region);
			});
	}

	void Renderer::createWhiteTexture()
	{
		unsigned char data[] = { 0xFF, 0xFF, 0xFF, 0xFF };
		whiteTexture = TextureManager::Instance().Create(data, 1, 1);
	}

}
