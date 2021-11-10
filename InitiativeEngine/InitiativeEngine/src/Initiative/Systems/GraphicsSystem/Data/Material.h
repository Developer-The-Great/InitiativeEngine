#pragma once
#include "Initiative\Core.h"

namespace itv
{

	enum class MaterialType
	{
		NO_MATERIAL,
		TEXTURED_UNLIT,
	};

	enum class TextureUsageType
	{
		ALBEDO_TEXTURE,
		TEXTURE_USAGE_MAX
	};

	struct TextureInfo
	{
		TextureUsageType TextureUsage;
		int TextureIndex;

	};

	class MaterialCreationInfo
	{
		friend class Material;
	private:

		std::vector< TextureInfo > mTexturesUsed;

	public:

		MaterialCreationInfo();

		bool AddTexture(const char* textureDirectory, TextureUsageType textureType);

	};

	class Material
	{
	private:

		std::vector<int> mTextureIndices;

	public:

		Material(const MaterialCreationInfo& info);

		inline int GetTexture(TextureUsageType textureUsageType) { return mTextureIndices[ static_cast<size_t>( textureUsageType) ]; }


	};





}
