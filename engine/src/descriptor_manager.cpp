#include "toy3d/descriptor_manager.hpp"
#include "toy3d/context.hpp"
#include "toy3d/shader.hpp"
#include <memory>

namespace toy3d
{
	std::unique_ptr<DescriptorSetManager> DescriptorSetManager::instance_ = nullptr;

	DescriptorSetManager::DescriptorSetManager(uint32_t maxFlight): maxFlight_(maxFlight)
	{
		vk::DescriptorPoolCreateInfo createInfo;
		vk::DescriptorPoolSize size;
		size.setType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(maxFlight * 2);
		createInfo.setMaxSets(maxFlight)
			.setPoolSizes(size);
		bufferSetPool_.pool_ = Context::GetInstance().device.createDescriptorPool(createInfo);
		bufferSetPool_.remainNum_ = maxFlight;
	}

	DescriptorSetManager::~DescriptorSetManager()
	{
		auto& device = Context::GetInstance().device;
		
		device.destroyDescriptorPool(bufferSetPool_.pool_);
		for (auto& pool : fulledImageSetPool_)
		{
			device.destroyDescriptorPool(pool.pool_);
		}
		for (auto& pool : avalibleImageSetPool_)
		{
			device.destroyDescriptorPool(pool.pool_);
		}
	}

	std::vector<DescriptorSetManager::SetInfo> DescriptorSetManager::AllocBufferSets(uint32_t num)
	{
		std::vector<vk::DescriptorSetLayout> layouts(maxFlight_, Context::GetInstance().shader->GetDescriptorSetLayouts()[0]);
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.setDescriptorPool(bufferSetPool_.pool_)
			.setDescriptorSetCount(num)
			.setSetLayouts(layouts);
		auto sets = Context::GetInstance().device.allocateDescriptorSets(allocInfo);

		std::vector<SetInfo> result(num);

		for (int i = 0; i < num; i++)
		{
			result[i].set = sets[i];
			result[i].pool = bufferSetPool_.pool_;
		}

		return result;
	}

	DescriptorSetManager::SetInfo DescriptorSetManager::AllocImageSet()
	{
		std::vector<vk::DescriptorSetLayout> layouts{ Context::GetInstance().shader->GetDescriptorSetLayouts()[1] };
		auto& poolInfo = getAvalibleImagePoolInfo();
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.setDescriptorPool(poolInfo.pool_)
			.setDescriptorSetCount(1)
			.setSetLayouts(layouts);
		auto sets = Context::GetInstance().device.allocateDescriptorSets(allocInfo);

		SetInfo result;
		result.pool = poolInfo.pool_;
		result.set = sets[0];

		poolInfo.remainNum_ = std::max<int>(static_cast<int>(poolInfo.remainNum_) - sets.size(), 0);
		if (poolInfo.remainNum_ == 0)
		{
			fulledImageSetPool_.push_back(poolInfo);
			avalibleImageSetPool_.pop_back();
		}

		return result;
	}

	void DescriptorSetManager::FreeImageSet(const SetInfo& setInfo)
	{
		auto it = std::find_if(fulledImageSetPool_.begin(), fulledImageSetPool_.end(), [&](const PoolInfo& poolInfo)
			{
				return poolInfo.pool_ == setInfo.pool;
			});

		if (it != fulledImageSetPool_.end())
		{
			it->remainNum_++;
			avalibleImageSetPool_.push_back(*it);
			fulledImageSetPool_.erase(it);
			return;
		}

		it = std::find_if(avalibleImageSetPool_.begin(), avalibleImageSetPool_.end(), [&](const PoolInfo& poolInfo)
			{
				return poolInfo.pool_ == setInfo.pool;
			});

		if (it != fulledImageSetPool_.end())
		{
			it->remainNum_++;
			return;
		}
		Context::GetInstance().device.freeDescriptorSets(setInfo.pool, setInfo.set);
	}

	void DescriptorSetManager::addImageSetPool()
	{
		constexpr uint32_t MaxSetNum = 10;
		
		vk::DescriptorPoolCreateInfo createInfo;
		vk::DescriptorPoolSize size;
		size.setType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(MaxSetNum);
		createInfo.setMaxSets(MaxSetNum)
			.setPoolSizes(size)
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
		avalibleImageSetPool_.push_back({ Context::GetInstance().device.createDescriptorPool(createInfo), MaxSetNum });
	}

	DescriptorSetManager::PoolInfo& DescriptorSetManager::getAvalibleImagePoolInfo()
	{
		if (avalibleImageSetPool_.empty())
		{
			addImageSetPool();
		}
		return avalibleImageSetPool_.back();
	}

}
