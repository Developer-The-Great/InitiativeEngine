#include "ECSSystemBase.h"
#include "ArchetypeManager.h"

namespace itv
{
	Entity ECSSystemBase::CreateEntity()
	{
		return mArchetypeManager->CreateEntity();
	}

	Entity ECSSystemBase::GetWorldEntity()
	{
		return mArchetypeManager->GetWorldEntity();
	}

}

