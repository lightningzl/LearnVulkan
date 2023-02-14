#pragma once
#include "vulkan/vulkan.hpp"

namespace toy2d 
{
	class Swapchain final
	{
	public:
		struct Image
		{
			vk::Image image;
			vk::ImageView view;
		};

		Swapchain(vk::SurfaceKHR surface, int windowWidth, int windowHeight);
		~Swapchain();

		vk::SurfaceKHR surface = nullptr;
		vk::SwapchainKHR swapchain = nullptr;
		std::vector<Image> images;
		std::vector<vk::Framebuffer> framebuffers;

		const auto& GetExtent() const { return surfaceInfo_.extent; }
		const auto& GetFormat() const { return surfaceInfo_.format; }

		void InitFramebuffers();

	private:
		struct SurfaceInfo
		{
			vk::SurfaceFormatKHR format;
			vk::Extent2D extent;
			std::uint32_t count;
			vk::SurfaceTransformFlagBitsKHR transform;
		} surfaceInfo_;

		vk::SwapchainKHR createSwapchain();
		vk::SurfaceFormatKHR querySurfaceFormat();
		vk::Extent2D querySurfaceExtent(const vk::SurfaceCapabilitiesKHR& capability, int windowWidth, int windwoHeight);

		void querySurfaceInfo(int windowWidth, int windowHeight);
		void createImageAndViews();
		void createFramebuffers();
	};
};
