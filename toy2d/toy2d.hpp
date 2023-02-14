#pragma once

#include "vulkan/vulkan.hpp"
#include "context.hpp"
#include "renderer.hpp"

namespace toy2d 
{
	void Init(std::vector<const char*>& extensions, GetSurfaceCallback callback, int windowWidth, int windowHeight);
	void Quit();
	Renderer* GetRenderer();

}
