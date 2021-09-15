#include "Application.h"
#include "Containers\sparse_set.h"
#include "ECS\ArchetypeManager.h"
#include "Log.h"

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
		sparse_set<size_t> test;
		test.push_back_unique(0);


		struct position
		{
			float x, y, z;
		};

		struct rotation
		{
			float x, y, z, w;
		};

		struct scale
		{
			float x, y, z;
		};



		ArchetypeManager manager;

		Entity a = manager.CreateEntity();

		manager.RegisterComponent<position>();
		manager.RegisterComponent<rotation>();
		manager.RegisterComponent<scale>();

		position aPosition;
		rotation aRotation;
		scale aScale;

		a.AddComponent(aPosition);
		//a.AddComponent(aRotation);
		a.AddComponent(aScale);

		ITV_LOG(" has position? {0} ",a.HasComponent<position>() );
		ITV_LOG(" has rotation? {0} ",a.HasComponent<rotation>());
		ITV_LOG(" has scale? {0} ",   a.HasComponent<scale>());

		while (mApplicationRunning)
		{
			mWindow->Update();
		}
	}
}

