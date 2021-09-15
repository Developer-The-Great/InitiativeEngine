#pragma once
#include "Initiative\Core.h"

namespace itv
{
	class ArchetypeType
	{
	private:

		std::vector<TypeID> mTypes;

	public:

		ArchetypeType(std::vector<TypeID>&& types) : mTypes(std::move(types)) {}

		ArchetypeType(const std::vector<TypeID>& types) : mTypes(types) {}

		ArchetypeType() = default;

		inline TypeID Size() const { return mTypes.size(); }

		inline TypeID At(size_t index) const { return mTypes[index]; }

		void AddType(TypeID id)
		{
			mTypes.push_back(id);
			std::sort(mTypes.begin(), mTypes.end());
		}

		inline bool operator==(const ArchetypeType& types) const
		{
			//early exit if ArchetypeTypes are empty or are not equal in length

			if (mTypes.empty() && types.mTypes.empty()) return true;

			if (mTypes.size() != types.mTypes.size()) return false;

			for (size_t i = 0; i < mTypes.size(); i++)
			{
				if (mTypes[i] != types.mTypes[i])
				{
					return false;
				}
			}

			return true;
		}
	};

	class Archetype;

	class ArchetypeQuery
	{
	public:

	private:

		std::vector<Archetype*> mArchetypes;

	};




}