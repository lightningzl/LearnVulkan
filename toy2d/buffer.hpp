#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d
{
	class Buffer
	{
	public:
		Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property);
		~Buffer();

		vk::Buffer buffer;
		vk::DeviceMemory memory;
		size_t size;

	private:
		struct MemoryInfo 
		{
			size_t size;
			uint32_t index;
		};

		void createBuffer(size_t size, vk::BufferUsageFlags usage);
		void allocateMemory(MemoryInfo info);
		void bindMemory2Buffer();
		MemoryInfo queryMemoryInfo(vk::MemoryPropertyFlags property);

	};


}