#include "Material.h"


namespace itv
{
	constexpr int invalid_texture = -1;

	MaterialCreationInfo::MaterialCreationInfo()
	{
		mTexturesUsed.resize(static_cast<size_t>(TextureUsageType::TEXTURE_USAGE_MAX));
	}

	bool MaterialCreationInfo::AddTexture(const char* textureDirectory, TextureUsageType textureType)
	{
		return false;
	}

	Material::Material(const MaterialCreationInfo& info)
	{
		mTextureIndices.resize( info.mTexturesUsed.size(), invalid_texture );

		for (size_t i = 0; i < info.mTexturesUsed.size(); i++)
		{
			mTextureIndices[i] = info.mTexturesUsed[i].TextureIndex;
		}

	}
}

