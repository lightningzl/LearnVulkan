#include "toy2d/toy2d.hpp"
#include "toy2d/shader.hpp"
#include "toy2d/renderer.hpp"

namespace toy2d 
{
	std::unique_ptr<Renderer> renderer_;

	void Init(std::vector<const char*>& extensions, GetSurfaceCallback callback, int windowWidth, int windowHeight)
	{
		Context::Init(extensions, callback);
		Context::GetInstance().InitSwapchain(windowWidth, windowHeight);
		Context::GetInstance().InitRenderProcess();
		Context::GetInstance().InitGraphicsPipeline();
		Context::GetInstance().swapchain->InitFramebuffers();
		Context::GetInstance().InitCommandPool();

		renderer_ = std::make_unique<Renderer>();
	}

	void Quit() {
		Context::GetInstance().device.waitIdle();
		renderer_.reset();
		Context::Quit();
	}

	Renderer* GetRenderer()
	{
		return renderer_.get();
	}
}