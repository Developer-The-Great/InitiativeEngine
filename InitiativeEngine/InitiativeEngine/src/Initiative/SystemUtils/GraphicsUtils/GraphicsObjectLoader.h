#pragma once
#include "Initiative\Core.h"
#include "Initiative\Systems\GraphicsSystem\Components\Mesh.h"
#include "Initiative\Systems\GraphicsSystem\Data\Material.h"

namespace itv
{
	class GraphicsSystem;

	class GraphicsObjectLoader
	{
	public:

		static void					  Init(GraphicsSystem* currentGraphicsSystem);

		static Mesh			  ITV_API LoadMesh(const char* fileLocation);

		static int			  ITV_API LoadTexture(const char* fileLocation);

		static unsigned char* LoadTextureFromFile(const char* fileLocation,OUT int& texWidth, OUT int& texHeight, OUT int& texChannels);

	private:

		GraphicsObjectLoader() = default;

		static GraphicsSystem* sGraphicsSystem;

	};



}
