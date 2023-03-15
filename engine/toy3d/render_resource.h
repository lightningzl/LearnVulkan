#pragma once
#include "render_resource_base.h"

namespace toy3d
{
	class RenderResource : public RenderResourceBase
	{
	public:
		RenderResource();
		virtual ~RenderResource();
		virtual void initialize();
		virtual void clear();

	private:

	};



}
