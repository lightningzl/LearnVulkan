#pragma once

#include "vulkan/vulkan.hpp"
#include "context.hpp"
#include "renderer.hpp"
#include "texture.hpp"

namespace toy2d 
{
	void Init(std::vector<const char*>& extensions, GetSurfaceCallback callback, int windowWidth, int windowHeight);
	void Quit();
	Renderer* GetRenderer();
	Texture* LoadTexture(const std::string& filename);
	void DestroyTexture(Texture* texture);
	void ResizeSwapchainImage(int w, int h);
}
