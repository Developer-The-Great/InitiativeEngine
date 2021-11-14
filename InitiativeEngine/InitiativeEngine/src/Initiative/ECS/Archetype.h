#pragma once
#include "Initiative\Core.h"
#include "Initiative\Containers\sparse_set.h"
#include "Initiative\ECS\ECSUtils.h"
#include "Initiative\ECS\ComponentArray.h"
#include "Entity.h"

namespace itv
{
	constexpr TypeID invalid_entity_id = std::numeric_limits<size_t>::max();
	constexpr TypeID world_entity_id = 1;

	class Archetype
	{
		friend class ArchetypeManager;
		friend class Entity;

	private:

		ArchetypeType mArchetypeType;

		std::vector<
			std::unique_ptr<ComponentArrayBase>> mComponentArrays;

		sparse_set<Entity> mEntities;

		static ArchetypeManager* sArchetypeManager;

		ITV_API void moveExistingEntity(TypeID id, Archetype& newArchetype);

		void instatiateNewComponentArray(const TypeID id);

		Archetype(const ArchetypeType& archetypeType);

	public:

		Archetype(const Archetype& other) = delete;

		Archetype& operator=(const Archetype& other) = delete;

		Archetype(Archetype&& other) noexcept :
			mArchetypeType(std::move(other.mArchetypeType)),
			mComponentArrays(std::move(other.mComponentArrays)),
			mEntities(std::move(other.mEntities))
		{

		}

		Archetype& operator=(Archetype&& other)
		{
			mArchetypeType = std::move(other.mArchetypeType);
			mComponentArrays = std::move(other.mComponentArrays);
			mEntities = std::move(other.mEntities);
		}

		inline size_t GetComponentArrayCount() const { return mComponentArrays.size(); }

		ArchetypeType GetArchetypeTypeCopy() const { return mArchetypeType; }

		const  ArchetypeType& GetArchetypeType() const { return mArchetypeType; }

		inline size_t GetEntityIndex(Entity entity) const
		{
#if _DEBUG

			if (!mEntities.contains(entity))
			{
				return invalid_entity_id;
			}

#endif

			return mEntities.find_dense_index_of(entity);
		}

		inline size_t GetEntityCount() const { return mEntities.size(); }

		template<class Component>
		ComponentArrayHandle<Component> GetComponentArray();

		ITV_API void* FindComponentArrayOfType(TypeID typeHash);

	};
}
