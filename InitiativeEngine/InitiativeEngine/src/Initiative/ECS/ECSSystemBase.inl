#pragma once

namespace itv
{
	template<typename ComponentType, typename ...ComponentTypes>
	inline ArchetypeQuery ECSSystemBase::FindArchetypesWith()
	{
		return mArchetypeManager->FindArchetypesWith<ComponentType, ComponentTypes...>();
	}

	template<class Component>
	inline void ECSSystemBase::RegisterComponent()
	{
		mArchetypeManager->RegisterComponent<Component>();
	}

}
