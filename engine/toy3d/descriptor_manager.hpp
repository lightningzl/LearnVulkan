#pragma once

#include "vulkan/vulkan.hpp"
#include <vector>

namespace toy3d
{
	class DescriptorSetManager
	{
	public:
		struct SetInfo
		{
			vk::DescriptorSet set;
			vk::DescriptorPool pool;
		};

		static void Init(uint32_t maxFlight)
		{
			instance_ = std::make_unique<DescriptorSetManager>(maxFlight);
		}

		static void Quit()
		{
			instance_.reset();
		}

		static DescriptorSetManager& Instance()
		{
			return *instance_;
		}
		
		DescriptorSetManager(uint32_t maxFlight);
		~DescriptorSetManager();

		std::vector<SetInfo> AllocBufferSets(uint32_t num);

		SetInfo AllocImageSet();
		void FreeImageSet(const SetInfo& setInfo);

	private:
		static std::unique_ptr<DescriptorSetManager> instance_;
		struct PoolInfo
		{
			vk::DescriptorPool pool_;
			uint32_t remainNum_;
		};

		PoolInfo bufferSetPool_;

		std::vector<PoolInfo> fulledImageSetPool_;
		std::vector<PoolInfo> avalibleImageSetPool_;

		void addImageSetPool();
		PoolInfo& getAvalibleImagePoolInfo();

		uint32_t maxFlight_;
	};
}
