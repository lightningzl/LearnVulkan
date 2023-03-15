#pragma once
#include <memory>

namespace toy3d
{
	class RHI;
	class RenderResourceBase;

	struct RenderPassInitInfo
	{};

	struct RenderPassCommonInfo
	{
		std::shared_ptr<RHI>                rhi;
		std::shared_ptr<RenderResourceBase> renderResource;
	};

	class RenderPassBase
	{
	public:
		virtual void initialize(RenderPassInitInfo initInfo) = 0;
		virtual void postInitialize() {};
		virtual void setCommonInfo(RenderPassCommonInfo commonInfo);
		virtual void preparePassData(std::shared_ptr<RenderResourceBase> render_resource) {};

	protected:
		std::shared_ptr<RHI>                rhi_;
		std::shared_ptr<RenderResourceBase> renderResource_;
	};

}