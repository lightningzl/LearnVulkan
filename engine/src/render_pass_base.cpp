#include "render_pass_base.h"

namespace toy3d
{
	void RenderPassBase::setCommonInfo(RenderPassCommonInfo commonInfo)
	{
		rhi_ = commonInfo.rhi;
		renderResource_ = commonInfo.renderResource;
	}
}
