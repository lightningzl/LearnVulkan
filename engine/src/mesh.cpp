#include "toy3d/mesh.h"
#include "tiny_obj_loader.h"
#include "toy3d/texture.hpp"
#include <unordered_map>

namespace toy3d
{
	Mesh::Mesh(std::string_view modelPath)
	{
		loadModel(modelPath);
	}

	Mesh::~Mesh()
	{
		verticesBuffer.reset();
		indicesBuffer.reset();
	}

	void Mesh::loadModel(std::string_view modelPath)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &warn, modelPath.data())) 
		{
			throw std::runtime_error(err);
		}

		std::vector<Vertex3d> vertices;
		std::vector<uint32_t> indices;
		std::unordered_map<Vertex3d, uint32_t> uniqueVertices = {};

		for (const auto& shape : shapes) 
		{
			for (const auto& index : shape.mesh.indices) 
			{
				Vertex3d vertex;
				vertex.pos =
				{
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord =
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					1 - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.normal =
				{
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}
		
		verticesBuffer.reset(new Buffer(vk::BufferUsageFlagBits::eVertexBuffer,
			sizeof(Vertex3d) * vertices.size(),
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		indicesBuffer.reset(new Buffer(vk::BufferUsageFlagBits::eIndexBuffer,
			sizeof(uint32_t) * indices.size(),
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));

		memcpy(verticesBuffer->map, vertices.data(), sizeof(Vertex3d) * vertices.size());
		memcpy(indicesBuffer->map, indices.data(), sizeof(uint32_t) * indices.size());
	}
}