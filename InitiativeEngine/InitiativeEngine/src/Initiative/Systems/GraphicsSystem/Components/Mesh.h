#pragma once
#include "Initiative\Core.h"
#include "Initiative\math.h"

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
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;


	};



}