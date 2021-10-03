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
		ArchetypeManager archetypeManager;

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

		archetypeManager.RegisterComponent<position>();
		archetypeManager.RegisterComponent<rotation>();
		archetypeManager.RegisterComponent<scale>();

		position pos;
		rotation rot;
		scale scale;

		auto posEnt = archetypeManager.CreateEntity();
		posEnt.AddComponent(pos);

		auto posEnt2 = archetypeManager.CreateEntity();
		posEnt2.AddComponent(pos);

		auto posRotEnt = archetypeManager.CreateEntity();
		posRotEnt.AddComponent(pos);
		posRotEnt.AddComponent(rot);

		auto posRotScaleEnt = archetypeManager.CreateEntity();
		posRotScaleEnt.AddComponent(pos);
		posRotScaleEnt.AddComponent(rot);
		posRotScaleEnt.AddComponent(scale);

		itv::ArchetypeQuery positionQuery = archetypeManager.FindArchetypesWith<position>();

		for (size_t i = 0; i < positionQuery.Size(); i++)
		{
			auto& archetype = positionQuery[i];

			auto componentArray = archetype.GetComponentArray<position>();



		}

		while (mApplicationRunning)
		{
			mWindow->Update();
		}
	}
}

