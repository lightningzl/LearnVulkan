#pragma once
#include "vulkan/vulkan.hpp"

namespace toy2d 
{
	class Swapchain final
	{
	public:
		Swapchain(int w, int h);
		~Swapchain();

		vk::SwapchainKHR swapchain;
		
		struct SwapchainInfo 
		{
			vk::Extent2D imageExtent;
			uint32_t imageCount;
			vk::SurfaceFormatKHR format;
			vk::SurfaceTransformFlagsKHR transform;
			vk::PresentModeKHR present;
		};

		SwapchainInfo info;

		void queryInfo(int w, int h);
	};
};
