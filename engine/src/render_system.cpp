#include "render_system.h"
#include "rhi.h"
#include "render_pipeline.h"
#include "render_resource.h"
#include "camera.h"

namespace toy3d
{
	RenderSystem::~RenderSystem()
	{
		clear();
	}

	void RenderSystem::initialize()
	{
		rhi = std::make_shared<RHI>();
		rhi->initialize();

		renderResource = std::make_shared<RenderResource>();


		camera = std::make_shared<Camera>();

		renderPipeline = std::make_shared<RenderPipeline>();
		
		

	}

	void RenderSystem::clear()
	{
		if (rhi)
		{
			rhi->clear();
		}
		rhi.reset();

		if (renderResource)
		{
			renderResource->clear();
		}
		renderResource.reset();

		if (renderPipeline)
		{
			renderPipeline->clear();
		}
		renderPipeline.reset();
	}

}
