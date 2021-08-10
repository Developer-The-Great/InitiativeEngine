#include "InitiativeEngine.h"

class Sandbox : public itv::Application
{
public:
	Sandbox() = default;
	~Sandbox() = default;
};

itv::Application* itv::CreateApplication()
{
	return new Sandbox();
}