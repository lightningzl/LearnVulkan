#include "toy3d/buffer.hpp"
#include "toy3d/context.hpp"

namespace toy3d
{
	Buffer::Buffer(vk::BufferUsageFlags usage, size_t size, vk::MemoryPropertyFlags property)
		: size(size)
	{
		auto& device = Context::GetInstance().device;

		vk::BufferCreateInfo createInfo;
		createInfo.setSize(size)
			.setUsage(usage)
			.setSharingMode(vk::SharingMode::eExclusive);
		buffer = device.createBuffer(createInfo);

		auto requirements = Context::GetInstance().device.getBufferMemoryRequirements(buffer);
		requireSize = requirements.size;

		auto index = QueryBufferMemTypeIndex(requirements.memoryTypeBits, property);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.setMemoryTypeIndex(index)
			.setAllocationSize(requirements.size);
		memory = device.allocateMemory(allocInfo);

		device.bindBufferMemory(buffer, memory, 0);

		if (property & vk::MemoryPropertyFlagBits::eHostVisible)
		{
			map = device.mapMemory(memory, 0, size);
		}
		else
		{
			map = nullptr;
		}
	}

	Buffer::~Buffer()
	{
		auto& device = Context::GetInstance().device;
		if (map)
		{
			device.unmapMemory(memory);
		}
		Context::GetInstance().device.freeMemory(memory);
		Context::GetInstance().device.destroy(buffer);
	}

	std::uint32_t QueryBufferMemTypeIndex(std::uint32_t type, vk::MemoryPropertyFlags flag)
	{
		auto properties = Context::GetInstance().phyDevice.getMemoryProperties();
		for (int i = 0; i < properties.memoryTypeCount; i++)
		{
			if ((1 << i) & type && properties.memoryTypes[i].propertyFlags & flag)
			{
				return i;
			}
		}

		return 0;
	}
}
