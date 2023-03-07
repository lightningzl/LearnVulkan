#pragma once

#include <initializer_list>
#include "vulkan/vulkan.hpp"

namespace toy2d
{
	struct Vec
	{
		union
		{
			struct 
			{
				float x, y;
			};
			
			struct 
			{
				float w, h;
			};
		};

		static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescription();
		static std::vector<vk::VertexInputBindingDescription> GetBingDescription();
	};

	struct Vertex
	{
		Vec position;
		Vec texcoord;
	};

	struct Color
	{
		float r, g, b;
	};

	using Size = Vec;

	class Mat4
	{
	public:
		Mat4();
		~Mat4();

		static Mat4 CreateIdentity();
		static Mat4 CreateOnes();
		static Mat4 CreateOrtho(int left, int right, int top, int bottom, int near, int far);
		static Mat4 CreateTranslate(const Vec& pos);
		static Mat4 CreateScale(const Vec& scale);
		static Mat4 Create(const std::initializer_list<float>& initList);

		const float* GetData() const { return data_; }
		void Set(int x, int y, float value)
		{
			data_[x * 4 + y] = value;
		}
		float Get(int x, int y) const 
		{
			return data_[x * 4 + y];
		}

		Mat4 Mul(const Mat4& m) const;

	private:
		float data_[4 * 4];
	};

	struct Rect
	{
		Vec position;
		Size size;
	};
}

