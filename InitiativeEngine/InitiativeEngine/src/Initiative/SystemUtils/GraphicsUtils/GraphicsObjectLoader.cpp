#include "GraphicsObjectLoader.h"
#include "Initiative/Systems/GraphicsSystem/GraphicsSystem.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace itv
{
	GraphicsSystem* GraphicsObjectLoader::sGraphicsSystem = nullptr;

	void GraphicsObjectLoader::Init(GraphicsSystem* currentGraphicsSystem)
	{
		sGraphicsSystem = currentGraphicsSystem;
	}

	Mesh GraphicsObjectLoader::LoadMesh(const char* fileLocation)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fileLocation))
		{
			throw std::runtime_error(warn + err);
		}
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}

		int index = sGraphicsSystem->LoadMeshIntoGraphicsSystem(vertices,indices);

		return Mesh(std::move(vertices), std::move(indices), index);
	}

	int ITV_API GraphicsObjectLoader::LoadTexture(const char* fileLocation)
	{

		return -1;
	}

}

