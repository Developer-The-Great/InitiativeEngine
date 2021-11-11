#pragma once
#include "Initiative\Core.h"
#include "Initiative\math.h"
#include "Initiative\Log.h"
#include "Initiative\Systems\GraphicsSystem\Data\Material.h"

namespace itv
{
	


	struct Vertex {
		math::vec3 pos;
		math::vec3 color;
		math::vec2 texCoord;

		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}

	};

	struct Mesh
	{
		std::vector<Vertex> Vertices;
		std::vector<uint32_t> Indices;

		Material MeshMaterial;

		int BufferIndex = -1;

		Mesh() { ITV_LOG("CTR");  }
		~Mesh() = default;

		Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices, int bufferIndex) 
			: Vertices( std::move(vertices) ), Indices( std::move(indices) ), BufferIndex(bufferIndex)
		{
			ITV_LOG("MOVE");
		}

		Mesh(const Mesh& meshToCopy) 
			: Vertices(meshToCopy.Vertices), Indices(meshToCopy.Indices), 
			BufferIndex(meshToCopy.BufferIndex), MeshMaterial(meshToCopy.MeshMaterial)
		{
			ITV_LOG("CPY CTR");
		}

		Mesh(Mesh&& tempMesh) noexcept 
			: Vertices(std::move(tempMesh.Vertices)), Indices(std::move(tempMesh.Indices)),
			BufferIndex(tempMesh.BufferIndex), MeshMaterial( std::move(tempMesh.MeshMaterial) )
		{
			ITV_LOG("MOVE CTR");
		}

		Mesh& operator=(Mesh&& other) {
			Vertices = std::move(other.Vertices);
			Indices = std::move(other.Indices);
			BufferIndex = other.BufferIndex;
			MeshMaterial = std::move(other.MeshMaterial);
			ITV_LOG("MOVE ASSIGN CTR");
			return *this;
		}
	};

}

namespace std
{
	template<> struct hash<itv::Vertex> {
		size_t operator()(itv::Vertex const& vertex) const {
			return ((hash<itv::math::vec3>()(vertex.pos) ^
				(hash<itv::math::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<itv::math::vec2>()(vertex.texCoord) << 1);
		}
	};
}