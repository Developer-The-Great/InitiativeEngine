#include "Archetype.h"
#include "ArchetypeManager.h"

namespace itv
{
	ArchetypeManager* Archetype::sArchetypeManager = nullptr;

	Archetype::Archetype(const ArchetypeType& archetypeType) : mArchetypeType(archetypeType)
	{
		for (int i = 0; i < archetypeType.Size(); ++i)
		{
			instatiateNewComponentArray(mArchetypeType.At(i));
		}
	}

	void Archetype::moveExistingEntity(TypeID id, Archetype& newArchetype)
	{
		mEntities.remove(static_cast<Entity>(id));
		newArchetype.mEntities.push_back_unique(static_cast<Entity>(id));
	}

	void Archetype::instatiateNewComponentArray(const TypeID id)
	{
		auto instantiationFunc = sArchetypeManager->GetComponentArrayInstantiationFunc(id);
		mComponentArrays.emplace_back(instantiationFunc());
	}

	void* Archetype::FindComponentArrayOfType(TypeID typeHash)
	{
		for (size_t i = 0; i < mComponentArrays.size(); i++)
		{
			if (mArchetypeType.At(i) == typeHash)
			{
				return mComponentArrays[i]->GetCompArrayPtr();
			}
		}

		return nullptr;
	}
}