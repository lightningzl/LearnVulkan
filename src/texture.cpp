#include "toy2d/texture.hpp"
#include "toy2d/buffer.hpp"
#include "toy2d/context.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "toy2d//stb_image.h"

namespace toy2d
{
	Texture::Texture(std::string_view filename)
	{
		int w, h, channel;
		stbi_uc* pixels = stbi_load(filename.data(), &w, &h, &channel, STBI_rgb_alpha);
		if (!pixels)
		{
			throw std::runtime_error("image load failed");
		}

		init(pixels, w, h);

		stbi_image_free(pixels);
	}

	Texture::Texture(void* data, uint32_t w, uint32_t h)
	{
		init(data, w, h);
	}

	Texture::~Texture()
	{
		auto& device = Context::GetInstance().device;
		device.destroyImageView(view);
		device.freeMemory(memory);
		device.destroyImage(image);
	}

	void Texture::createImage(uint32_t w, uint32_t h)
	{
		vk::ImageCreateInfo createInfo;
		createInfo.setImageType(vk::ImageType::e2D)
			.setArrayLayers(1)
			.setMipLevels(1)
			.setExtent({ w, h, 1 })
			.setFormat(vk::Format::eR8G8B8A8Srgb)
			.setTiling(vk::ImageTiling::eOptimal)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
			.setSamples(vk::SampleCountFlagBits::e1);
		image = Context::GetInstance().device.createImage(createInfo);
	}

	void Texture::createImageView()
	{
		vk::ImageViewCreateInfo createInfo;
		vk::ImageSubresourceRange range;
		range.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1)
			.setAspectMask(vk::ImageAspectFlagBits::eColor);
		createInfo.setImage(image)
			.setViewType(vk::ImageViewType::e2D)
			.setComponents(vk::ComponentMapping{})
			.setFormat(vk::Format::eR8G8B8A8Srgb)
			.setSubresourceRange(range);
		view = Context::GetInstance().device.createImageView(createInfo);
	}

	void Texture::allocMemory()
	{
		auto& device = Context::GetInstance().device;
		vk::MemoryAllocateInfo allocInfo;

		auto requirements = device.getImageMemoryRequirements(image);
		allocInfo.setAllocationSize(requirements.size);

		auto index = QueryBufferMemTypeIndex(requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
		allocInfo.setMemoryTypeIndex(index);

		memory = device.allocateMemory(allocInfo);
	}

	void Texture::transitionImageLayoutFromUndefine2Dst()
	{
		Context::GetInstance().commandManager->ExecuteCmd(Context::GetInstance().graphcisQueue,
			[&](vk::CommandBuffer cmdBuf) {
				vk::ImageMemoryBarrier barrier;
				vk::ImageSubresourceRange range;
				range.setLayerCount(1)
					.setBaseArrayLayer(0)
					.setLevelCount(1)
					.setBaseMipLevel(0)
					.setAspectMask(vk::ImageAspectFlagBits::eColor);
				barrier.setImage(image)
					.setOldLayout(vk::ImageLayout::eUndefined)
					.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
					.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
					.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
					.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
					.setSubresourceRange(range);
				cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, nullptr, barrier);
			});
	}

	void Texture::transitionImageLayoutFromDst2Optimal()
	{
		Context::GetInstance().commandManager->ExecuteCmd(Context::GetInstance().graphcisQueue,
			[&](vk::CommandBuffer cmdBuf) {
				vk::ImageMemoryBarrier barrier;
				vk::ImageSubresourceRange range;
				range.setLayerCount(1)
					.setBaseArrayLayer(0)
					.setLevelCount(1)
					.setBaseMipLevel(0)
					.setAspectMask(vk::ImageAspectFlagBits::eColor);
				barrier.setImage(image)
					.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
					.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
					.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
					.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
					.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
					.setSubresourceRange(range);
				cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, nullptr, barrier);
			});
	}

	void Texture::transformData2Image(Buffer& buffer, uint32_t w, uint32_t h)
	{
		Context::GetInstance().commandManager->ExecuteCmd(Context::GetInstance().graphcisQueue,
			[&](vk::CommandBuffer cmdBuf) {
				vk::BufferImageCopy region;
				vk::ImageSubresourceLayers subsource;
				subsource.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseArrayLayer(0)
					.setMipLevel(0)
					.setLayerCount(1);
				region.setBufferImageHeight(0)
					.setBufferOffset(0)
					.setImageOffset(0)
					.setImageExtent({ w, h, 1 })
					.setBufferRowLength(0)
					.setImageSubresource(subsource);
				cmdBuf.copyBufferToImage(buffer.buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
			});
	}

	void Texture::updataDescriptorSet()
	{
		vk::WriteDescriptorSet writer;
		vk::DescriptorImageInfo imageInfo;
		imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(view)
			.setSampler(Context::GetInstance().sampler);

		writer.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setImageInfo(imageInfo)
			.setDstBinding(0)
			.setDstSet(set.set)
			.setDstArrayElement(0)
			.setDescriptorCount(1);

		Context::GetInstance().device.updateDescriptorSets(writer, {});
	}

	void Texture::init(void* data, uint32_t w, uint32_t h)
	{

		const uint32_t size = w * h * 4;
		std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>(vk::BufferUsageFlagBits::eTransferSrc, size, vk::MemoryPropertyFlagBits::eHostCached | vk::MemoryPropertyFlagBits::eHostVisible);
		memcpy(buffer->map, data, size);

		createImage(w, h);
		allocMemory();
		Context::GetInstance().device.bindImageMemory(image, memory, 0);

		transitionImageLayoutFromUndefine2Dst();
		transformData2Image(*buffer, w, h);
		transitionImageLayoutFromDst2Optimal();

		createImageView();

		set = DescriptorSetManager::Instance().AllocImageSet();

		updataDescriptorSet();
	}

	std::unique_ptr<TextureManager> TextureManager::instance_ = nullptr;

	Texture* TextureManager::Load(const std::string& filename)
	{
		datas_.push_back(std::unique_ptr<Texture>(new Texture(filename)));
		return datas_.back().get();
	}

	Texture* TextureManager::Create(void* data, uint32_t w, uint32_t h)
	{
		datas_.push_back(std::unique_ptr<Texture>(new Texture(data, w, h)));
		return datas_.back().get();
	}

	void TextureManager::Destroy(Texture* texture)
	{
		auto it = std::find_if(datas_.begin(), datas_.end(), [&](const std::unique_ptr<Texture>& t)
			{
				return t.get() == texture;
			});
		if (it != datas_.end())
		{
			Context::GetInstance().device.waitIdle();
			datas_.erase(it);
			return;
		}
	}

	void TextureManager::Clear()
	{
		datas_.clear();
	}

}
