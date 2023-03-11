#pragma once

#include <initializer_list>
#include "vulkan/vulkan.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/detail/type_quat.hpp"

namespace toy3d
{
	struct Vec2d
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

	struct Vertex2d
	{
		Vec2d position;
		Vec2d texcoord;
	};

	struct Color
	{
		float r, g, b;
	};

	struct Transform
	{
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 scale;
		//rotation�б���ʼ��Ĭ��ת��
		Transform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale) : translation(translation), rotation(glm::radians(rotation)), scale(scale) {};
		glm::mat4 GetModelMatrix()
		{
			glm::mat4 model(1.0f);
			model = glm::scale(model, scale);
			model = glm::mat4_cast(rotation) * model;
			return glm::translate(model, translation);
		}
	};

	struct Vertex3d
	{
		glm::vec3 pos;
		glm::vec2 texCoord;
		glm::vec3 normal;
		bool operator==(const Vertex3d& other) const
		{
			return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
		}

		static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescription()
		{
			std::array<vk::VertexInputAttributeDescription, 3> descriptions = {};
			descriptions[0].setBinding(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setLocation(0)
				.setOffset(0);
			descriptions[1].setBinding(0)
				.setFormat(vk::Format::eR32G32Sfloat)
				.setLocation(1)
				.setOffset(offsetof(Vertex3d, texCoord));
			descriptions[2].setBinding(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setLocation(2)
				.setOffset(offsetof(Vertex3d, normal));
			return descriptions;
		}

		static std::array<vk::VertexInputBindingDescription, 1> GetBingDescription()
		{
			std::array<vk::VertexInputBindingDescription, 1> descriptions = {};
			descriptions[0].setBinding(0)
				.setStride(sizeof(Vertex3d))
				.setInputRate(vk::VertexInputRate::eVertex);

			return descriptions;
		}
	};

	using Size = Vec2d;

	class Mat4
	{
	public:
		Mat4();
		~Mat4();

		static Mat4 CreateIdentity();
		static Mat4 CreateOnes();
		static Mat4 CreateOrtho(int left, int right, int top, int bottom, int near, int far);
		static Mat4 CreateTranslate(const Vec2d& pos);
		static Mat4 CreateScale(const Vec2d& scale);
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
		Vec2d position;
		Size size;
	};
}

namespace std {
	template<> 
	struct hash<toy3d::Vertex3d> 
	{
		size_t operator()(toy3d::Vertex3d const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}
