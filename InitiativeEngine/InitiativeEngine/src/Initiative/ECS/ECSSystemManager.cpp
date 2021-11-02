#include "ECSSystemManager.h"

namespace itv
{
	ECSSystemManager::ECSSystemManager(ArchetypeManager* archetypeManager) : mArchetypeManager(archetypeManager)
	{
	}

	void ECSSystemManager::RegisterSystemComponents()
	{
		for (std::unique_ptr<ECSSystemBase>& system : mSystems)
		{
			system->RegisterComponents();
		}
	}

	void ECSSystemManager::InitializeSystem()
	{
		for (std::unique_ptr<ECSSystemBase>& system : mSystems)
		{
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