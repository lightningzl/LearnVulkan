#pragma once
#include "render_pipeline_base.h"

namespace toy3d
{
	class RenderPipeline : public RenderPipelineBase
	{
	public:
		RenderPipeline();
		virtual ~RenderPipeline();
		virtual void initialize(RenderPipelineInitInfo initInfo) override final;

		virtual void forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> renderResource) override final;
		virtual void deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> renderResource) override final;

	};


}