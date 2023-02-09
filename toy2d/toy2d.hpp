#pragma once

#include "vulkan/vulkan.hpp"
#include "context.hpp"
#include "renderer.hpp"

namespace toy2d 
{
	void Init(std::vector<const char*>& extensions, CreateSurfaceFunc func, int w, int h);
	void Quit();

	inline Renderer& GetRenderer()
	{
		return *Context::GetInstance().renderer;
	}
}
