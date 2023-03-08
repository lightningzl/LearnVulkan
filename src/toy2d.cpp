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
		Context::GetInstance().initShaderModules();
		Context::GetInstance().InitRenderProcess();
		Context::GetInstance().InitGraphicsPipeline();
		Context::GetInstance().swapchain->InitFramebuffers();
		Context::GetInstance().InitCommandPool();
		Context::GetInstance().initSampler();

		int maxFlightCount = 2;
		DescriptorSetManager::Init(maxFlightCount);
		renderer_ = std::make_unique<Renderer>(maxFlightCount);
		renderer_->SetProject(windowWidth, 0, 0, windowHeight, -1, 1);
	}

	void Quit() {
		Context::GetInstance().device.waitIdle();
		renderer_.reset();
		TextureManager::Instance().Clear();
		DescriptorSetManager::Quit();
		Context::Quit();
	}

	Renderer* GetRenderer()
	{
		return renderer_.get();
	}

	Texture* LoadTexture(const std::string& filename)
	{
		return TextureManager::Instance().Load(filename);
	}

	void DestroyTexture(Texture* texture)
	{
		TextureManager::Instance().Destroy(texture);
	}

	void ResizeSwapchainImage(int w, int h)
	{
		auto& ctx = Context::GetInstance();
		ctx.device.waitIdle();
		ctx.swapchain.reset();
		ctx.getSurface();
		ctx.InitSwapchain(w, h);
		ctx.swapchain->InitFramebuffers();
	}

}