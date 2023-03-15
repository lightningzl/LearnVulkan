#include "render_pipeline.h"

namespace toy3d
{

	RenderPipeline::RenderPipeline()
	{
	}

	RenderPipeline::~RenderPipeline()
	{
	}

	void RenderPipeline::initialize(RenderPipelineInitInfo initInfo)
	{
		rhi_ = initInfo.rhi;
		mainPass_ = std::make_shared<MainPass>();
		
		RenderPassCommonInfo passCommonInfo;
		passCommonInfo.rhi = rhi_;
		passCommonInfo.renderResource = initInfo.renderResource;

		mainPass_->setCommonInfo(passCommonInfo);

	}

	void RenderPipeline::forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> renderResource)
	{

	}

	void RenderPipeline::deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> renderResource)
	{

	}

}
