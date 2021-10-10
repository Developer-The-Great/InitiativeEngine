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

		inline TypeID operator[](size_t index) const
		{
			return mTypes[index];
		}

		bool ContainsTypesInside( const ArchetypeType& otherType ) const
		{
			if (mTypes.empty() && otherType.mTypes.empty()) { return true; }

			TypeID typeToSearchFor = otherType[0];
			size_t foundTypesCount = 0;

			for (TypeID componentType : mTypes)
			{
				if (componentType == typeToSearchFor)
				{
					foundTypesCount++;

					if (foundTypesCount == otherType.Size())
					{
						return true;
					}

					typeToSearchFor = otherType[foundTypesCount];

				}
			}

			return false;
		}

	};

	class Archetype;

	class ArchetypeQueryIterator
	{
	private:

		Archetype* mArchetypePtr = nullptr;


	public:

		ArchetypeQueryIterator(Archetype* pInitialPtr) : mArchetypePtr(pInitialPtr)
		{

		}

		ITV_API Archetype& operator++();
		
		ITV_API Archetype& operator*();

		ITV_API bool	   operator==(const ArchetypeQueryIterator& other);

		ITV_API bool	   operator!=(const ArchetypeQueryIterator& other);
		
	};

	class ArchetypeQuery
	{
	public:

		using iterator = ArchetypeQueryIterator;

		ArchetypeQuery(std::vector<Archetype*>&& archetypesChosen) 
			: mArchetypes( std::move( archetypesChosen ) )
		{

		}

		inline Archetype& operator[](size_t index) { return *mArchetypes[index]; }

		inline size_t Size() const				   { return mArchetypes.size(); }

		inline ITV_API ArchetypeQueryIterator begin();

		inline ITV_API ArchetypeQueryIterator end();

	private:

		std::vector<Archetype*> mArchetypes;

	};

}