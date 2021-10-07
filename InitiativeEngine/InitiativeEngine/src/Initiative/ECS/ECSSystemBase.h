#pragma once
#include "ECSUtils.h"

namespace itv
{
	class ArchetypeManager;

	class ECSSystemBase
	{
	private:

		ArchetypeManager* mArchetypeManager = nullptr;

		ECSSystemBase(ArchetypeManager* archetypeManager) : mArchetypeManager(archetypeManager)
		{

		}


	public:

		virtual void						    RegisterComponents() = 0;

		virtual void							BeginRun() = 0;

		virtual void						    Run() = 0;

		template<typename ComponentType,
			typename... ComponentTypes>
		ArchetypeQuery							FindArchetypesWith();

		template<class Component> inline void	RegisterComponent();

	};

}


