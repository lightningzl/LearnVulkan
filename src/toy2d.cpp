#include "toy2d/toy2d.hpp"
#include "toy2d/shader.hpp"

namespace toy2d 
{
	Shader* shader;
	void Init(std::vector<const char*>& extensions, CreateSurfaceFunc func, int w, int h) {
		Context::Init(extensions, func);
		Context::GetInstance().InitSwapchain(w, h);
		shader = new Shader(ReadWholeFile("./vert.spv"), ReadWholeFile("./frag.spv"));
		Context::GetInstance().InitRenderProcess(w, h, shader);
		Context::GetInstance().InitRenderer();
	}

	void Quit() {
		delete shader;
		Context::GetInstance().device.waitIdle();
		Context::GetInstance().DestroyRenderer();
		Context::GetInstance().DestroyRenderProcess();
		Context::GetInstance().DestroySwapchain();
		Context::Quit();
	}
}