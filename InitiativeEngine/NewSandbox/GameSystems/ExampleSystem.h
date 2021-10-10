#pragma once
#include "Initiative\ECS\ECSSystemBase.h"

namespace itv
{
	class ArchetypeManager;

	class ExampleSystem : public ECSSystemBase
	{
	private:



	public:

		ExampleSystem() = default;
		~ExampleSystem() = default;

		void						    RegisterComponents() override;

		void							BeginRun() override;

		void						    Run() override;


	};



}


