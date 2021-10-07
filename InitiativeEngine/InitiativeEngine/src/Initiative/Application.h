#pragma once
#include "Core.h"
#include "Window\Window.h"
#include "Events\WindowEvents.h"

namespace itv
{
	class ECSSystemRegistrationAdmin;

	class Application
	{
	private:

		std::unique_ptr<Window> mWindow;
		bool mApplicationRunning = true;

		Observer<WindowCloseEvent> closeEventObserver;

	public:

		ITV_API Application();
		virtual ~Application() = default;

		ITV_API void Run();

		virtual void RegisterGameSystems(ECSSystemRegistrationAdmin& systemRegAdmin) {};

	};

	Application* CreateApplication();
}
