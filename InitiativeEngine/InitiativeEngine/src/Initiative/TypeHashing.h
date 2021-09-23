#pragma once
#include "Core.h"

namespace itv
{
#ifdef _MSC_VER
	#define GET_FUNC_SIGNATURE __FUNCSIG__
#else
	#define GET_FUNC_SIGNATURE ""
	#error "Initiative currently only supports msvc"
#endif

	constexpr size_t StringHash(const char* charArray)
	{
		size_t hash = 0xcbf29ce484222325;
		std::uint64_t prime = 0x00000100000001b3;

		while (*charArray != '\0')
		{
			hash = hash ^ (std::uint8_t)*charArray;
			hash *= prime;
			charArray++;
		}
		return hash;
	}

	template<class Type>
	constexpr size_t GenerateTypeHash()
	{
		constexpr size_t value = StringHash(GET_FUNC_SIGNATURE);
		return value;
	}
}