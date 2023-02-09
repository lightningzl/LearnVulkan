#pragma once

#include "vulkan/vulkan.hpp"
#include <memory>
#include <cassert>
#include <iostream>
#include <optional>
#include "tool.hpp"
#include "swapchain.hpp"
#include "render_process.hpp"
#include "renderer.hpp"

namespace toy2d {

	class Shader;
	class Context final {
	public:
		static void Init(std::vector<const char*>& extensions, CreateSurfaceFunc func);
		static void Quit();

		static Context& GetInstance()
		{
			assert(instance_);
			return *instance_;
		}

		~Context();

		struct QueueFamilyIndices final
		{
			std::optional<uint32_t> graphicsQueue;
			std::optional<uint32_t> presentQueue;
			operator bool()
			{
				return graphicsQueue.has_value() && presentQueue.has_value();
			}
		};

		vk::Instance instance;
		vk::PhysicalDevice phyDevice;
		vk::Device device;
		vk::Queue graphcisQueue;
		vk::Queue presentQueue;
		vk::SurfaceKHR surface;
		std::unique_ptr<Swapchain> swapchain;
		std::unique_ptr<RenderProcess> renderProcess;
		std::unique_ptr<Renderer> renderer;
		QueueFamilyIndices queueFamilyIndices;

		void InitSwapchain(int w, int h);
		void DestroySwapchain();

		void InitRenderProcess(int w, int h, Shader* shader);
		void DestroyRenderProcess();

		void InitRenderer();
		void DestroyRenderer();

	private:
		static std::unique_ptr<Context> instance_;

		Context(std::vector<const char*>& extensions, CreateSurfaceFunc func);

		void createInstance(std::vector<const char*>& extensions);
		void pickPhyiscalDevice();
		void createDevice();
		void getQueues();

		void queryQueueFamilyIndices();
	};

}