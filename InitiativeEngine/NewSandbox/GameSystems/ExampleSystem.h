#pragma once
#include "Initiative\ECS\ECSSystemBase.h"
#include "Initiative\Events\InputEvents.h"

namespace itv
{
	class ArchetypeManager;

	class ExampleSystem : public ECSSystemBase
	{
	private:

		Observer<MouseMoveEvent> mMouseMoveObserver;
		Observer<InputEvent> mInputObserver;

		

	public:

		ExampleSystem();
		~ExampleSystem() = default;

		void						    RegisterComponents() override;

		void							BeginRun() override;

		void						    Run() override;


	};



}


