#include "toy2d/swapchain.hpp"
#include "toy2d/context.hpp"

namespace toy2d
{
	Swapchain::Swapchain(int w, int h)
	{
		queryInfo(w, h);
		
		vk::SwapchainCreateInfoKHR createInfo;
		createInfo.setClipped(true)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setSurface(Context::GetInstance().surface)
			.setImageColorSpace(info.format.colorSpace)
			.setImageFormat(info.format.format)
			.setImageExtent(info.imageExtent)
			.setMinImageCount(info.imageCount)
			.setPresentMode(info.present);
		
		auto& queueIndices = Context::GetInstance().queueFamilyIndices;
		if (queueIndices.graphicsQueue.value() == queueIndices.presentQueue.value())
		{
			createInfo.setQueueFamilyIndices(queueIndices.graphicsQueue.value())
				.setImageSharingMode(vk::SharingMode::eExclusive);
		}
		else
		{
			std::array indices = { queueIndices.graphicsQueue.value(), queueIndices.presentQueue.value() };
			createInfo.setQueueFamilyIndices(indices)
				.setImageSharingMode(vk::SharingMode::eConcurrent);
		}

		swapchain = Context::GetInstance().device.createSwapchainKHR(createInfo);
	}

	Swapchain::~Swapchain()
	{
		Context::GetInstance().device.destroySwapchainKHR(swapchain);
	}

	void Swapchain::queryInfo(int w, int h)
	{
		auto& phyDevice = Context::GetInstance().phyDevice;
		auto& surface = Context::GetInstance().surface;
		auto formats = phyDevice.getSurfaceFormatsKHR(surface);
		info.format = formats[0];
		for (const auto& format : formats)
		{
			if (format.format == vk::Format::eR8G8B8Srgb
				&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				info.format = format;
				break;
			}
		}

		auto capabilities = phyDevice.getSurfaceCapabilitiesKHR(surface);
		info.imageCount = std::clamp<uint32_t>(2, capabilities.minImageCount, capabilities.maxImageCount);
		info.imageExtent.width = std::clamp<uint32_t>(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		info.imageExtent.height = std::clamp<uint32_t>(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		info.transform = capabilities.currentTransform;

		auto presents = phyDevice.getSurfacePresentModesKHR(surface);
		info.present = vk::PresentModeKHR::eFifo;
		for (const auto& present : presents)
		{
			if (present == vk::PresentModeKHR::eMailbox)
			{
				info.present = present;
				break;
			}
		}
	}

}
