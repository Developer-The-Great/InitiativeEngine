#pragma once

namespace itv
{
	template<class Component>
	void Entity::AddComponent(Component& component)
	{
		ITV_LOG("Add Component: {0}", GET_FUNC_SIGNATURE);

		constexpr TypeID componentHash = GenerateTypeHash<Component>();

		assert(sArchetypeManager->IsComponentRegistered<Component>());

		//[1] Build ArchetypeType of Archetype we are looking for 

		Archetype& oldArchetype = sArchetypeManager->GetRecord(mID);

		ArchetypeType newType = oldArchetype.GetArchetypeTypeCopy();
		newType.AddType(componentHash);

		size_t newArchetypeIndex;
		auto newArchetypeOptionalRef = sArchetypeManager->GetArchetype(newType, newArchetypeIndex);

		//[2] Construct a new one destination archetype if it doesnt exist yet
		if (!newArchetypeOptionalRef.has_value())
		{
			newArchetypeOptionalRef = sArchetypeManager->InstantiateArchetype(newType);
			newArchetypeIndex = sArchetypeManager->GetArchetypeCount() - 1;
		}

		Archetype& newArchetype = newArchetypeOptionalRef.value().get();

		assert(&newArchetype != &oldArchetype);

		//[3] Get each component of current archetype and move them to the new archetype

		const ArchetypeType& types = oldArchetype.GetArchetypeType();

		size_t oldArchetypeIndex = oldArchetype.GetEntityIndex(mID);

		assert(oldArchetypeIndex != invalid_entity_id);

		for (size_t i = 0; i < oldArchetype.GetComponentArrayCount(); i++)
		{

			auto& moveFunc = sArchetypeManager->GetComponentArrayMoveFunc(types.At(i));

			void* currentLoc = oldArchetype.mComponentArrays[i]->GetCompArrayPtr();
			void* destinationLoc = newArchetype.FindComponentArrayOfType(types.At(i));

			moveFunc(oldArchetypeIndex, currentLoc, destinationLoc);

		}

		oldArchetype.moveExistingEntity(mID, newArchetype);

		void* destinationLoc = newArchetype.FindComponentArrayOfType(componentHash);

		//[4] add new component to new Archetype

		std::vector<Component>* newComponentDestination =
			static_cast<std::vector<Component>*>(destinationLoc);

		newComponentDestination->push_back(component);

		//[5] Update entity record

		sArchetypeManager->UpdateEntityRecord(mID, newArchetypeIndex); //TODO this shoudlnt be last archetype

		ITV_LOG("");
	};

	template<class Component>
	bool Entity::HasComponent() const
	{
		constexpr TypeID componentHash = GenerateTypeHash<Component>();

		Archetype& archetype = sArchetypeManager->GetRecord(mID);

		return archetype.FindComponentArrayOfType(componentHash) != nullptr;
	}

	template<class Component>
	void Entity::RemoveComponent(Component& component)
	{
		constexpr TypeID componentHash = GenerateTypeHash<Component>();

	}
}