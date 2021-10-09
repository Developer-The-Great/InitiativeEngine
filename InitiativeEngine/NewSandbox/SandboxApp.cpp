#include "InitiativeEngine.h"
#include "GameSystems\ExampleSystem.h"

namespace itv
{
	class Sandbox : public itv::Application
	{
	public:
		Sandbox() = default;
		~Sandbox() = default;

		void RegisterGameSystems(itv::ECSSystemRegistrationAdmin& systemRegAdmin) override
		{
			ITV_LOG("-------------- Initializing Game Systems --------------");

			systemRegAdmin.RegisterSystem<ExampleSystem>();

		};

	};
}


itv::Application* itv::CreateApplication()
{
	return new Sandbox();
}