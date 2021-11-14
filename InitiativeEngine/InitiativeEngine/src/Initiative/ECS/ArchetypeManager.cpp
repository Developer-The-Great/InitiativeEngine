#include "ArchetypeManager.h"



namespace itv
{
	constexpr size_t default_archetype_index = 0;
	constexpr size_t archetype_reserve = 100;

	TypeID ArchetypeManager::nextEntityID = 0;

	ArchetypeManager* Entity::sArchetypeManager = nullptr;

	ArchetypeManager::ArchetypeManager()
	{
		//mArchetypes should never get resized because it will invalidate references to archetypes
		mArchetypes.reserve(archetype_reserve);

		Entity::sArchetypeManager = this;
		Archetype::sArchetypeManager = this;

		InstantiateArchetype(ArchetypeType());

		nextEntityID = 0;

		Entity worldEntity = CreateEntity();
		assert(worldEntity.mID == world_entity_id);
	}

	Archetype& ArchetypeManager::InstantiateArchetype(const ArchetypeType& types)
	{
		mArchetypes.emplace_back( Archetype(types) );

		return mArchetypes[mArchetypes.size()-1];
	}

	void ArchetypeManager::UpdateEntityRecord(TypeID id, size_t newArchetypeIndex)
	{
		auto it = mEntityRecords.find(id);
		it->second = newArchetypeIndex;
	}

	ArchetypeQuery ArchetypeManager::GetArchetypesWith(const ArchetypeType& type)
	{
		std::vector<Archetype*> archetypePtrs;
		archetypePtrs.reserve( mArchetypes.size() );

		for (Archetype& archetype : mArchetypes)
		{
			if ( archetype.GetArchetypeType().ContainsTypesInside( type ) )
			{
				archetypePtrs.push_back( &archetype );
			}
		}

		return ArchetypeQuery( std::move( archetypePtrs ) );
	}

	std::optional<std::reference_wrapper<Archetype>> ArchetypeManager::GetArchetype(const ArchetypeType& type)
	{
		for ( Archetype& archetype : mArchetypes )
		{
			if ( archetype.GetArchetypeType() == type )
			{
				return std::optional<std::reference_wrapper<Archetype>>{ archetype };
			}
		}

		return std::nullopt;
	}

	ITV_API std::optional<std::reference_wrapper<Archetype>> ArchetypeManager::GetArchetype(const ArchetypeType& type, size_t& outIndex)
	{
		for (size_t i = 0; i < mArchetypes.size(); i++)
		{
			if (mArchetypes[i].GetArchetypeType() == type)
			{
				outIndex = i;
				return std::optional<std::reference_wrapper<Archetype>>{mArchetypes[i]};
			}
		}

		return std::nullopt;
	}

	Archetype& ArchetypeManager::GetRecord(TypeID id)
	{
		return mArchetypes[mEntityRecords.at(id)];
	}

	Entity ArchetypeManager::CreateEntity()
	{
		nextEntityID++;
		
		mArchetypes[default_archetype_index].mEntities.push_back_unique(Entity(nextEntityID));

		mEntityRecords.insert( std::make_pair(nextEntityID, default_archetype_index) );

		return Entity(nextEntityID);
	}

	Entity ArchetypeManager::GetWorldEntity() const
	{
		return Entity(world_entity_id);
	}

	componentArrayFunc& ArchetypeManager::GetComponentArrayMoveFunc(TypeID componentType)
	{
		return mComponentHashToComponentActions.at(componentType).mTypeToComponentArrayMove; 
	}
	componentArrayInstantiationFunc& ArchetypeManager::GetComponentArrayInstantiationFunc(TypeID componentType)
	{
		return mComponentHashToComponentActions.at(componentType).mTypeToComponentInstantiationFunc;
	}

}

