#pragma once
#include "Core.h"
#include "Window\Window.h"
#include "Events\WindowEvents.h"

namespace itv
{
	class Application
	{
	public:
		ITV_API Application();
		virtual ~Application() = default;

		ITV_API void Run();

	private:

		std::unique_ptr<Window> mWindow;
		bool mApplicationRunning = true;

		Observer<WindowCloseEvent> closeEventObserver;

	};

	Application* CreateApplication();
}
