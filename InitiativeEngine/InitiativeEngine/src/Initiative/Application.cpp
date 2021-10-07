#include "Application.h"
#include "Containers\sparse_set.h"
#include "ECS\ArchetypeManager.h"

#include "Log.h"
#include "math.h"

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
		ArchetypeManager archetypeManager;
		archetypeManager.RegisterCoreSystems();

		ECSSystemRegistrationAdmin admin(&archetypeManager);
		RegisterGameSystems(admin);

		while (mApplicationRunning)
		{
			mWindow->Update();
		}
	}
}

