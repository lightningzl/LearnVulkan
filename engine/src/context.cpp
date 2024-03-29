#include "toy3d/context.hpp"
#include "toy3d/shader.hpp"
#include "toy3d/render_system.h"

namespace toy3d {

	Context* Context::instance_ = nullptr;

	void Context::Init(std::vector<const char*>& extensions, GetSurfaceCallback callback)
	{
		instance_ = new Context(extensions, callback);
	}

	void Context::Quit() 
	{
		delete instance_;
	}

	Context& Context::GetInstance()
	{
		assert(instance_);
		return *instance_;
	}

	Context::Context(std::vector<const char*>& extensions, GetSurfaceCallback callback) 
	{
		getSurfaceCallback_ = callback;

		instance = createInstance(extensions);
		if (!instance)
		{
			std::cout << "create instance faild" << std::endl;
			exit(1);
		}

		phyDevice = pickPhyiscalDevice();
		if (!phyDevice)
		{
			std::cout << "pickup phyiscal device faild" << std::endl;
			exit(1);
		}

		getSurface();

		device = createDevice(surface_);
		if (!device)
		{
			std::cout << "create device faild" << std::endl;
			exit(1);
		}

		graphcisQueue = device.getQueue(queueInfo.graphicsIndex.value(), 0);
		presentQueue = device.getQueue(queueInfo.presentIndex.value(), 0);

		renderSystem = std::make_shared<RenderSystem>();
		renderSystem->initialize();
	}

	Context::~Context() 
	{
		device.destroySampler(sampler);
		shader.reset();
		commandManager.reset();
		renderProcess.reset();
		swapchain.reset();
		device.destroy();
		instance.destroy();
	}

	void Context::InitSwapchain(int windowWidth, int windowHeight)
	{
		swapchain = std::make_unique<Swapchain>(surface_, windowWidth, windowHeight);
	}

	void Context::InitGraphicsPipeline()
	{
		renderProcess->CreateGraphicsPipeline(*shader);
	}

	void Context::InitCommandPool()
	{
		commandManager = std::make_unique<CommandManager>();
	}

	void Context::initShaderModules()
	{
		auto vertexSource = ReadWholeFile("./vert.spv");
		auto fragSource = ReadWholeFile("./frag.spv");
		shader = std::make_unique<Shader>(vertexSource, fragSource);
	}

	void Context::initSampler()
	{
		vk::SamplerCreateInfo createInfo;
		createInfo.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setAnisotropyEnable(false)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(false)
			.setCompareEnable(false)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear);
		sampler = Context::GetInstance().device.createSampler(createInfo);
	}

	void Context::getSurface()
	{
		surface_ = getSurfaceCallback_(instance);
		if (!surface_)
		{
			std::cout << "create surface faild" << std::endl;
			exit(1);
		}
	}

	void Context::InitRenderProcess()
	{
		renderProcess = std::make_unique<RenderProcess>();
	}

	vk::Instance Context::createInstance(std::vector<const char*>& extensions)
	{
		vk::InstanceCreateInfo createInfo;

		vk::ApplicationInfo appInfo;
		appInfo.setApiVersion(VK_API_VERSION_1_3);

		createInfo.setPApplicationInfo(&appInfo)
			.setPEnabledExtensionNames(extensions);

		std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
		createInfo.setPEnabledLayerNames(layers);

		return vk::createInstance(createInfo);
	}

	vk::PhysicalDevice Context::pickPhyiscalDevice()
	{
		auto devices = instance.enumeratePhysicalDevices();
		if (devices.size() == 0)
		{
			std::cout << "you don't have suitable device to support vulkan" << std::endl;
		}
		return devices[0];
	}

	vk::Device Context::createDevice(vk::SurfaceKHR surface)
	{
		vk::DeviceCreateInfo deviceCreateInfo;
		queryQueueInfo(surface);
		
		std::array extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		deviceCreateInfo.setPEnabledExtensionNames(extensions);

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		float priorities = 1.0;
		if (queueInfo.graphicsIndex.value() == queueInfo.presentIndex.value())
		{
			vk::DeviceQueueCreateInfo queueCreateInfo;
			queueCreateInfo.setPQueuePriorities(&priorities)
				.setQueueCount(1)
				.setQueueFamilyIndex(queueInfo.graphicsIndex.value());
			queueCreateInfos.push_back(std::move(queueCreateInfo));
		}
		else
		{
			vk::DeviceQueueCreateInfo queueCreateInfo;
			queueCreateInfo.setPQueuePriorities(&priorities)
				.setQueueCount(1)
				.setQueueFamilyIndex(queueInfo.graphicsIndex.value());
			queueCreateInfos.push_back(queueCreateInfo);

			queueCreateInfo.setQueueFamilyIndex(queueInfo.presentIndex.value());
			queueCreateInfos.push_back(queueCreateInfo);
		}
		deviceCreateInfo.setQueueCreateInfos(queueCreateInfos);

		return phyDevice.createDevice(deviceCreateInfo);
	}

	void Context::queryQueueInfo(vk::SurfaceKHR surface)
	{
		auto queueProperties = phyDevice.getQueueFamilyProperties();
		for (int i = 0; i < queueProperties.size(); i++)
		{
			if (queueProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				queueInfo.graphicsIndex = i;
			}
			if (phyDevice.getSurfaceSupportKHR(i, surface))
			{
				queueInfo.presentIndex = i;
			}
			if (queueInfo.graphicsIndex.has_value()
				&& queueInfo.presentIndex.has_value())
			{
				break;
			}
		}
	}

}