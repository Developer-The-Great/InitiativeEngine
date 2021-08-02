#include <InitiativeEngine.h>

class Sandbox : public Itv::Application
{
public:
		Sandbox() = default;
		~Sandbox() = default;
};

Itv::Application* Itv::CreateApplication()
{
	return new Sandbox();
}
