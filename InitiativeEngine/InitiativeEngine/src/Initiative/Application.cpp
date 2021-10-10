#include "Application.h"
#include "Containers\sparse_set.h"
#include "ECS\ArchetypeManager.h"

#include "ECS\ECSSystemManager.h"

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
		ECSSystemManager systemManager(&archetypeManager);
		
		systemManager.RegisterCoreSystems();

		ECSSystemRegistrationAdmin admin(&systemManager);
		RegisterGameSystems(admin);

		systemManager.InitializeSystem();
		
		while (mApplicationRunning)
		{
			mWindow->Update();

			systemManager.RunSystems();
		}
	}
}

