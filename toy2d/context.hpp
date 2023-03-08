#pragma once

#include "vulkan/vulkan.hpp"
#include <memory>
#include <iostream>
#include <optional>
#include <functional>

#include "tool.hpp"
#include "swapchain.hpp"
#include "render_process.hpp"
#include "command_manager.hpp"

namespace toy2d {

	using GetSurfaceCallback = std::function<vk::SurfaceKHR(vk::Instance)>;
	class Shader;
	class Context
	{
	public:
		friend void Init(std::vector<const char*>& extensions, GetSurfaceCallback callback, int windowWidth, int windowHeight);
		friend void ResizeSwapchainImage(int w, int h);

		static void Init(std::vector<const char*>& extensions, GetSurfaceCallback callback);
		static void Quit();
		static Context& GetInstance();

		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsIndex;
			std::optional<uint32_t> presentIndex;
			operator bool()
			{
				return graphicsIndex.has_value() && presentIndex.has_value();
			}
		} queueInfo;

		vk::Instance instance;
		vk::PhysicalDevice phyDevice;
		vk::Device device;
		vk::Queue graphcisQueue;
		vk::Queue presentQueue;
		vk::Sampler sampler;

		std::unique_ptr<Swapchain> swapchain;
		std::unique_ptr<RenderProcess> renderProcess;
		std::unique_ptr<CommandManager> commandManager;
		std::unique_ptr<Shader> shader;

	private:
		static Context* instance_;
		vk::SurfaceKHR surface_ = nullptr;
		GetSurfaceCallback getSurfaceCallback_ = nullptr;

		Context(std::vector<const char*>& extensions, GetSurfaceCallback callback);
		~Context();

		void InitRenderProcess();
		void InitSwapchain(int windowWidth, int windowHeight);
		void InitGraphicsPipeline();
		void InitCommandPool();
		void initShaderModules();
		void initSampler();
		void getSurface();

		vk::Instance createInstance(std::vector<const char*>& extensions);
		vk::PhysicalDevice pickPhyiscalDevice();
		vk::Device createDevice(vk::SurfaceKHR surface);

		void queryQueueInfo(vk::SurfaceKHR surface);
	};

}