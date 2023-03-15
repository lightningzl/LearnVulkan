#pragma once

#include "vulkan/vulkan.hpp"
#include "buffer.hpp"
#include "math.hpp"

namespace toy3d
{
	class Mesh
	{
	public:
		Mesh(std::string_view modelPath);
		~Mesh();

		std::unique_ptr<Buffer> verticesBuffer;
		uint32_t vertexCount;

		std::unique_ptr<Buffer> indicesBuffer;
		uint32_t indexCount;
	private:
		void loadModel(std::string_view modelPath);
	};
}
