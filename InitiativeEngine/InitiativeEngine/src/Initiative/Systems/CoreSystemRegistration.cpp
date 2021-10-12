#include "Initiative\ECS\ECSSystemManager.h"
#include "GraphicsSystem\GraphicsSystem.h"

namespace itv
{
	void ECSSystemManager::RegisterCoreSystems()
	{
		RegisterSystem<GraphicsSystem>();
	}

}