#pragma once
#include "ECSUtils.h"
#include "Entity.h"
#include "ArchetypeManager.h"

namespace itv
{
	class ArchetypeManager;

	class ECSSystemBase
	{
		friend class ECSSystemManager;

	private:

		ArchetypeManager* mArchetypeManager = nullptr;

		inline void SetArchetypeManager(ArchetypeManager* archetypeManager) 
		{ 
			mArchetypeManager = archetypeManager;
		}

	protected:

		template<typename ComponentType,
			typename... ComponentTypes>
				ArchetypeQuery					FindArchetypesWith();

		template<class Component>  inline void	RegisterComponent();

		ITV_API Entity CreateEntity();

		ITV_API Entity GetWorldEntity();


	public:

		ECSSystemBase()
		{

		}

		virtual void						    RegisterComponents() = 0;

		virtual void							BeginRun() = 0;

		virtual void						    Run() = 0;

	};


}

#include "ECSSystemBase.inl"


