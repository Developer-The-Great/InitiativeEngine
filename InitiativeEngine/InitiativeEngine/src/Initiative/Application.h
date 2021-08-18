#pragma once
#include "Core.h"
#include "Window\Window.h"
#include "Events\WindowEvents.h"

namespace itv
{
	class ITV_API Application
	{
	public:
		Application();
		virtual ~Application() = default;

		void Run();

	private:

		std::unique_ptr<Window> mWindow;
		bool mApplicationRunning = true;

		Observer<WindowCloseEvent> closeEventObserver;

	};

	Application* CreateApplication();
}
