#pragma once
#include <memory>

namespace toy3d
{


	class RenderResourceBase
	{
	public:
		RenderResourceBase();
		virtual ~RenderResourceBase();
		virtual void initialize();
		virtual void clear();

	private:

	};


}

