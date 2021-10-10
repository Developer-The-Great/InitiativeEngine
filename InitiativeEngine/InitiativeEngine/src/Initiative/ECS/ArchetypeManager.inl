#pragma once

namespace itv
{
	template<class Component>
	void ArchetypeManager::RegisterComponent()
	{
		constexpr TypeID componentHash = GenerateTypeHash<Component>();
		ITV_LOG("generated component hash {0}", componentHash);

		componentActionFuncs componentActions;

		componentActions.mTypeToComponentInstantiationFunc =
			[]()
		{
			//std::make_unique cannnot used here because ComponentArray constructor is private
			return std::move(std::unique_ptr<ComponentArray<Component>>
				(new ComponentArray<Component>()));
		};

		componentActions.mTypeToComponentArrayMove =
			[](size_t sourceIndex, void* source, void* destination)
		{
			ComponentArrayBase::MoveComponent<Component>(sourceIndex, source, destination);
		};

		mComponentHashToComponentActions.insert(std::make_pair(componentHash, componentActions));
	}

	template<class Component>
	inline bool ArchetypeManager::IsComponentRegistered()
	{
		constexpr TypeID componentHash = GenerateTypeHash<Component>();
		return mComponentHashToComponentActions.find(componentHash) != mComponentHashToComponentActions.end();
	}

	template<typename ComponentType, typename ...ComponentTypes>
	inline ArchetypeQuery ArchetypeManager::FindArchetypesWith()
	{
		std::vector<TypeID> componentHashes;
		componentHashes.reserve(sizeof...(ComponentTypes) + 1);

		collectComponentHashes<ComponentType, ComponentTypes...>(componentHashes);

		ArchetypeType types(std::move(componentHashes));

		return GetArchetypesWith(types);
	}

	template<typename ComponentType, typename ...ComponentTypes>
	void ArchetypeManager::collectComponentHashes(std::vector<TypeID>& generatedHashes)
	{
		generatedHashes.push_back(GenerateTypeHash<ComponentType>());

		collectComponentHashes< ComponentTypes... >(generatedHashes);
	}
}