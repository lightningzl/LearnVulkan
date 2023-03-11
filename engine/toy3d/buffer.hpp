#pragma once

#include "vulkan/vulkan.hpp"

namespace toy3d
{
	class Buffer
	{
	public:
		Buffer(vk::BufferUsageFlags usage, size_t size, vk::MemoryPropertyFlags property);
		~Buffer();

		vk::Buffer buffer;
		vk::DeviceMemory memory;
		void* map;
		size_t size;
		size_t requireSize;

		Buffer(const Buffer& buffer) = delete;
		Buffer& operator=(const Buffer& buffer) = delete;
	};

	std::uint32_t QueryBufferMemTypeIndex(std::uint32_t type, vk::MemoryPropertyFlags flag);

}