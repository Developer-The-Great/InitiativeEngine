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

		ITV_API MaterialCreationInfo();

		ITV_API bool AddTexture(const char* textureDirectory, TextureUsageType textureType);

	};

	class Material
	{
	private:

		std::vector<int> mTextureIndices;

	public:
		Material() = default;
		ITV_API Material(const MaterialCreationInfo& info);

		Material(const Material& materialToCopy) = default;

		Material(Material&& materialToMove) noexcept
			: mTextureIndices(std::move(materialToMove.mTextureIndices))
		{

		}

		Material& operator=(Material&& other)
		{
			mTextureIndices = std::move(other.mTextureIndices);

			return *this;
		}

		inline int GetTexture(TextureUsageType textureUsageType) { return mTextureIndices[ static_cast<size_t>( textureUsageType) ]; }


	};





}
