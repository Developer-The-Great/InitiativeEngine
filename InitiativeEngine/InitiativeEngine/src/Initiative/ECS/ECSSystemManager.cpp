#include "ECSSystemManager.h"

namespace itv
{
	ECSSystemManager::ECSSystemManager(ArchetypeManager* archetypeManager) : mArchetypeManager(archetypeManager)
	{
	}

	void ECSSystemManager::InitializeSystem()
	{
		for (std::unique_ptr<ECSSystemBase>& system : mSystems)
		{
			system->RegisterComponents();
			system->BeginRun();
		}
	}

	void ECSSystemManager::RunSystems()
	{
		for (std::unique_ptr<ECSSystemBase>& system : mSystems)
		{
			system->Run();
		}
	}

}