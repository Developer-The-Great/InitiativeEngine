#include "InitiativeEngine.h"


class Sandbox : public itv::Application
{
public:
	Sandbox() = default;
	~Sandbox() = default;

	void RegisterGameSystems(itv::ECSSystemRegistrationAdmin& systemRegAdmin) override
	{
		ITV_LOG("-------------- Initializing Game Systems --------------");
		
	
	};

};

itv::Application* itv::CreateApplication()
{
	return new Sandbox();
}