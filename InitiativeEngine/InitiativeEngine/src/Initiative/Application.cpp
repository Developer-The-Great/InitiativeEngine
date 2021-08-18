#include "Application.h"

namespace itv
{
	Application::Application()
	{
		WindowInfo info;
		mWindow = Window::Create(info);

		bool& applicationRunning = mApplicationRunning;

		closeEventObserver.SetEventFunc([&applicationRunning](WindowCloseEvent& event)
		{
			applicationRunning = false;
		});

		mWindow->GetWindowSubject().RegisterObserver(closeEventObserver);

	}

	void Application::Run()
	{
		while (mApplicationRunning)
		{
			mWindow->Update();
		}
	}
}

