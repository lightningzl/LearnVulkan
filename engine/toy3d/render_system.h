#pragma once
#include <memory>

namespace toy3d
{
	class RHI;
	class Camera;
	class RenderResourceBase;
	class RenderPipelineBase;

	class RenderSystem
	{
	public:
		RenderSystem() = default;
		~RenderSystem();

		void initialize();
		void tick(float delta_time);
		void clear();


		std::shared_ptr<RHI> rhi;
		std::shared_ptr<Camera> camera;
		std::shared_ptr<RenderResourceBase> renderResource;
		std::shared_ptr<RenderPipelineBase> renderPipeline;
	private:
	};

}
