#include "toy2d/buffer.hpp"
#include "toy2d/context.hpp"

namespace toy2d
{
	Buffer::Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property)
		: size(size)
	{
		createBuffer(size, usage);
		allocateMemory(queryMemoryInfo(property));
		bindMemory2Buffer();
	}

	Buffer::~Buffer()
	{
		Context::GetInstance().device.freeMemory(memory);
		Context::GetInstance().device.destroy(buffer);
	}

	void Buffer::createBuffer(size_t size, vk::BufferUsageFlags usage)
	{
		vk::BufferCreateInfo createInfo;
		createInfo.setSize(size)
			.setUsage(usage)
			.setSharingMode(vk::SharingMode::eExclusive);
		
		buffer = Context::GetInstance().device.createBuffer(createInfo);
	}

	void Buffer::allocateMemory(MemoryInfo info)
	{
		vk::MemoryAllocateInfo allocInfo;
		allocInfo.setMemoryTypeIndex(info.index)
			.setAllocationSize(info.size);
		memory = Context::GetInstance().device.allocateMemory(allocInfo);
	}

	void Buffer::bindMemory2Buffer()
	{
		Context::GetInstance().device.bindBufferMemory(buffer, memory, 0);
	}

	Buffer::MemoryInfo Buffer::queryMemoryInfo(vk::MemoryPropertyFlags property)
	{
		MemoryInfo info;
		auto requirements = Context::GetInstance().device.getBufferMemoryRequirements(buffer);
		info.size = requirements.size;

		auto properties = Context::GetInstance().phyDevice.getMemoryProperties();
		for (int i = 0; i < properties.memoryTypeCount; i++)
		{
			if ((1 << i) & requirements.memoryTypeBits
				&& properties.memoryTypes[i].propertyFlags & property)
			{
				info.index = i;
				break;
			}
		}

		return info;
	}

}
