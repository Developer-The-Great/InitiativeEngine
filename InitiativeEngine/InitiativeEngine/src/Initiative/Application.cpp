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
		Entity b = manager.CreateEntity();

		manager.RegisterComponent<position>();
		manager.RegisterComponent<rotation>();
		manager.RegisterComponent<scale>();

		position aPosition;
		rotation aRotation;
		scale aScale;

		aPosition = { 0,0,0 };

		a.AddComponent(aPosition);
		a.AddComponent(aRotation);
		a.AddComponent(aScale);

		aPosition = { 1,2,3 };

		b.AddComponent(aPosition);

		ITV_LOG(" has position? {0} ",a.HasComponent<position>() );
		ITV_LOG(" has rotation? {0} ",a.HasComponent<rotation>());
		ITV_LOG(" has scale?	{0} ",   a.HasComponent<scale>());

		//ArchetypeType type(std::vector<size_t>{ GenerateTypeHash<position>() });

		ArchetypeQuery query = manager.FindArchetypesWith<position,rotation>();  //manager.GetArchetypesWith(type);

		for (size_t i = 0; i < query.Size(); i++)
		{
			auto& archetype = query[i];

			auto componentArrayHandle = archetype.GetComponentArray<position>();

			for (size_t i = 0; i < archetype.GetEntityCount(); i++)
			{
				position& ptr = componentArrayHandle[0];
				ITV_LOG("position {0} , {1} , {2}", ptr.x, ptr.y, ptr.z);
			}

		}

		while (mApplicationRunning)
		{
			mWindow->Update();
		}
	}
}

