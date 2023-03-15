#pragma once
#include <memory>
#include "main_pass.h"

namespace toy3d
{
	class RHI;
	class RenderResourceBase;

	struct RenderPipelineInitInfo
	{
		std::shared_ptr<RHI>				rhi;
		std::shared_ptr<RenderResourceBase> renderResource;
	};

	class RenderPipelineBase
	{
		friend class RenderSystem;
	public:
		virtual ~RenderPipelineBase() {};
		virtual void initialize(RenderPipelineInitInfo initInfo) = 0;
		virtual void clear() {};
		virtual void preparePassData(std::shared_ptr<RenderResourceBase> renderResource);
		virtual void forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> renderResource) {};
		virtual void deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> renderResource) {};

	protected:
		std::shared_ptr<RHI> rhi_;

		std::shared_ptr<RenderPassBase> mainPass_;
	};


}
