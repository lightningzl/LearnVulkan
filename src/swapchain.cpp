#include "toy2d/swapchain.hpp"
#include "toy2d/context.hpp"

namespace toy2d
{
	Swapchain::Swapchain(vk::SurfaceKHR surface, int windowWidth, int windowHeight)
		: surface(surface)
	{
		querySurfaceInfo(windowWidth, windowHeight);
		swapchain = createSwapchain();
		createImageAndViews();
	}

	Swapchain::~Swapchain()
	{
		for (auto& framebuffer : framebuffers)
		{
			Context::GetInstance().device.destroyFramebuffer(framebuffer);
		}
		for (auto& img : images)
		{
			Context::GetInstance().device.destroyImageView(img.view);
		}
		Context::GetInstance().device.destroySwapchainKHR(swapchain);
		Context::GetInstance().instance.destroySurfaceKHR(surface);
	}

	void Swapchain::InitFramebuffers()
	{
		createFramebuffers();
	}

	vk::SwapchainKHR Swapchain::createSwapchain()
	{
		vk::SwapchainCreateInfoKHR createInfo;
		createInfo.setClipped(true)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setImageExtent(surfaceInfo_.extent)
			.setImageColorSpace(surfaceInfo_.format.colorSpace)
			.setImageFormat(surfaceInfo_.format.format)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setMinImageCount(surfaceInfo_.count)
			.setImageArrayLayers(1)
			.setPresentMode(vk::PresentModeKHR::eFifo)
			.setPreTransform(surfaceInfo_.transform)
			.setSurface(surface);

		auto& queueInfo = Context::GetInstance().queueInfo;
		if (queueInfo.graphicsIndex.value() == queueInfo.presentIndex.value())
		{
			createInfo.setQueueFamilyIndices(queueInfo.graphicsIndex.value())
				.setImageSharingMode(vk::SharingMode::eExclusive);
		}
		else
		{
			std::array indices = { queueInfo.graphicsIndex.value(), queueInfo.presentIndex.value() };
			createInfo.setQueueFamilyIndices(indices)
				.setImageSharingMode(vk::SharingMode::eConcurrent);
		}

		return Context::GetInstance().device.createSwapchainKHR(createInfo);
	}

	vk::SurfaceFormatKHR Swapchain::querySurfaceFormat()
	{
		auto formats = Context::GetInstance().phyDevice.getSurfaceFormatsKHR(surface);
		for (auto& format : formats)
		{
			if (format.format == vk::Format::eR8G8B8Srgb
				&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return format;
			}
		}
		return formats[0];
	}

	vk::Extent2D Swapchain::querySurfaceExtent(const vk::SurfaceCapabilitiesKHR& capability, int windowWidth, int windwoHeight)
	{
		if (capability.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capability.currentExtent;
		}
		else
		{
			auto extent = vk::Extent2D{
				static_cast<uint32_t>(windowWidth),
				static_cast<uint32_t>(windwoHeight)
			};

			extent.width = std::clamp(extent.width, capability.minImageExtent.width, capability.maxImageExtent.width);
			extent.height = std::clamp(extent.height, capability.minImageExtent.height, capability.maxImageExtent.height);
			return extent;
		}
	}

	void Swapchain::querySurfaceInfo(int windowWidth, int windowHeight)
	{
		surfaceInfo_.format = querySurfaceFormat();
		
		auto capability = Context::GetInstance().phyDevice.getSurfaceCapabilitiesKHR(surface);
		surfaceInfo_.count = std::clamp(capability.minImageCount + 1, capability.minImageCount, capability.maxImageCount);
		surfaceInfo_.transform = capability.currentTransform;
		surfaceInfo_.extent = querySurfaceExtent(capability, windowWidth, windowHeight);
	}

	void Swapchain::createImageAndViews()
	{
		auto images = Context::GetInstance().device.getSwapchainImagesKHR(swapchain);

		for (auto& image : images)
		{
			Image img;
			img.image = image;
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
				.setFormat(surfaceInfo_.format.format)
				.setSubresourceRange(range);
			img.view = Context::GetInstance().device.createImageView(createInfo);
			this->images.push_back(img);
		}
	}

	void Swapchain::createFramebuffers()
	{
		for (auto& img : images)
		{
			auto& view = img.view;

			vk::FramebufferCreateInfo createInfo;
			createInfo.setAttachments(view)
				.setWidth(GetExtent().width)
				.setHeight(GetExtent().height)
				.setRenderPass(Context::GetInstance().renderProcess->renderPass)
				.setLayers(1);
			framebuffers.push_back(Context::GetInstance().device.createFramebuffer(createInfo));
		}
	}

}

