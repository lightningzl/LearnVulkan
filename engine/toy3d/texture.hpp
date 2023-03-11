#pragma once

#include "vulkan/vulkan.hpp"
#include "descriptor_manager.hpp"
#include <string_view>
#include <string>

namespace toy3d
{
	class Buffer;
	class TextureManager;
	class DescriptorSetManager;

	class Texture
	{
	public:
		friend class TextureManager;
		~Texture();

		vk::Image image;
		vk::DeviceMemory memory;
		vk::ImageView view;
		DescriptorSetManager::SetInfo set;

	private:
		Texture(std::string_view filename);
		Texture(void* data, uint32_t w, uint32_t h);

		void createImage(uint32_t w, uint32_t h);
		void createImageView();
		void allocMemory();
		void transitionImageLayoutFromUndefine2Dst();
		void transitionImageLayoutFromDst2Optimal();
		void transformData2Image(Buffer& buffer, uint32_t w, uint32_t h);
		void updataDescriptorSet();
		void init(void* data, uint32_t w, uint32_t h);
	};

	class TextureManager
	{
	public:
		static TextureManager& Instance()
		{
			if (!instance_)
			{
				instance_ = std::make_unique<TextureManager>();
			}
			return *instance_;
		}

		Texture* Load(const std::string& filename);
		//data must be a RGBA888 format data
		Texture* Create(void* data, uint32_t w, uint32_t h);
		void Destroy(Texture* texture);
		void Clear();
	private:
		static std::unique_ptr<TextureManager> instance_;
		std::vector<std::unique_ptr<Texture>> datas_;
	};

}
