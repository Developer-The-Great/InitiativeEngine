#include "ExampleSystem.h"
#include "ExampleComponents\SimpleFPSComponents.h"
#include "Initiative\ECS\ArchetypeManager.h"
#include "Initiative\Systems\GraphicsSystem\Components\Camera.h"
#include "Initiative\GenericComponents\GenericComponents.h"
#include "Initiative\Systems\GraphicsSystem\Components\WindowHandle.h"
#include "Initiative\InputCodes.h"
#include "Initiative\Systems\GraphicsSystem\Components\Mesh.h"
#include "Initiative\math.h"
#include "Initiative\SystemUtils\GraphicsUtils\GraphicsObjectLoader.h"


namespace itv
{
	ExampleSystem::ExampleSystem()
	{
		mMouseMoveObserver.SetEventFunc( [this](MouseMoveEvent& input)
		{
			ArchetypeQuery cameraQuery = FindArchetypesWith<Camera, Transform>();
			for (Archetype& cameraArchetype : cameraQuery)
			{
				auto transformArrayHandle = cameraArchetype.GetComponentArray<Transform>();

				Transform& cameraTransform = transformArrayHandle[0];
				math::mat4 transform = cameraTransform.GetLocalTransform();

				cameraTransform.Rotate(math::vec3(0, 1, 0), -input.DeltaX * 0.25f);
				cameraTransform.Rotate(cameraTransform.GetRight(), input.DeltaY * 0.25f);
			}


		});

		mInputObserver.SetEventFunc( [this](InputEvent& input)
		{
			auto windowHandleOpt = GetWorldEntity().GetComponent<WindowHandle>();
			auto windowHandle = windowHandleOpt.value().get();

			if (input.Key == KeyCode::L && input.Action == KeyAction::Press)
			{
				windowHandle.window->SetMouseLockState(MouseLockState::Locked);
			}

			if (input.Key == KeyCode::O && input.Action == KeyAction::Press)
			{
				windowHandle.window->SetMouseLockState(MouseLockState::Normal);
			}
			
			

			ArchetypeQuery cameraQuery = FindArchetypesWith<Camera, Transform>();
			for (Archetype& cameraArchetype : cameraQuery)
			{
				auto transformArrayHandle = cameraArchetype.GetComponentArray<Transform>();

				float movement = 0.1f;

				Transform& cameraTransform = transformArrayHandle[0];

				if (input.Key == KeyCode::W && input.Action == KeyAction::Hold)
				{
					cameraTransform.Translate(cameraTransform.GetForward() * movement);
				}

				if (input.Key == KeyCode::A && input.Action == KeyAction::Hold)
				{
					cameraTransform.Translate(cameraTransform.GetRight() * movement);
				}

				if (input.Key == KeyCode::S && input.Action == KeyAction::Hold)
				{
					cameraTransform.Translate(cameraTransform.GetForward() * -movement);
				}

				if (input.Key == KeyCode::D && input.Action == KeyAction::Hold)
				{
					cameraTransform.Translate(cameraTransform.GetRight() * -movement);
				}
			}


			

		});
	}

	void ExampleSystem::RegisterComponents()
	{
		RegisterComponent<EnemyHealthComponent>();
		RegisterComponent<PlayerGun>();
		RegisterComponent<ScoreTracker>();
		RegisterComponent<EnemySpawnArea>();
	}

	void ExampleSystem::BeginRun()
	{
		auto windowHandleOpt = GetWorldEntity().GetComponent<WindowHandle>();
		
		auto windowHandle = windowHandleOpt.value().get();
		windowHandle.window->GetWindowSubject().RegisterObserver(mMouseMoveObserver);
		windowHandle.window->GetWindowSubject().RegisterObserver(mInputObserver);

		// Create Camera
		Camera camera(math::radians(45.0f), 0.1f, 10000.0f);

		math::vec3 position = math::vec3(0.0f, 1.0f, 3.0f);

		math::quat rot = math::ConstructQuatFromVec(math::vec3(0,0,-1), math::vec3(0, 1, 0));

		Transform cameraTransform( math::vec3(position), math::quat(rot), math::vec3(1.0f));

		Entity cameraEntity = CreateEntity();
		cameraEntity.AddComponent(cameraTransform);
		cameraEntity.AddComponent(camera);


		//[1] Instatiate Player
		PlayerGun gun;
		Entity player = CreateEntity();
		gun.gunIdentifier = GUN_IDENTIFIER::LEFT;

		player.AddComponent(gun);
		
		//[2] Instatiate ObstaclePositions
		EnemySpawnArea area;

		Entity leftSpawner = CreateEntity();
		area.gunIdentifier = GUN_IDENTIFIER::LEFT;
		leftSpawner.AddComponent(area);

		Entity middleSpawner = CreateEntity();
		area.gunIdentifier = GUN_IDENTIFIER::MIDDLE;
		middleSpawner.AddComponent(area);

		Entity rightSpawner = CreateEntity();
		area.gunIdentifier = GUN_IDENTIFIER::RIGHT;
		rightSpawner.AddComponent(area);

		//[3] Level Area
		Entity wallInFrontOfPlayer = CreateEntity();
		Entity playerFloor = CreateEntity();
		Entity enemyFloor = CreateEntity();
		Entity enemySeperator = CreateEntity();

		//[4] Score Tracker
		Entity scoreTracker = CreateEntity();

		ScoreTracker tracker;
		scoreTracker.AddComponent(tracker);

		//temporary
		Mesh mesh = GraphicsObjectLoader::LoadMesh("Models/viking_room.obj");

		MaterialCreationInfo materialInfo;
		materialInfo.AddTexture("textures/texture.jpg", TextureUsageType::ALBEDO_TEXTURE);

		mesh.MeshMaterial = Material{ materialInfo };

		Transform objectTransform;
		
		objectTransform.SetLocalPosition( math::vec3(0, 0, 0) );
		objectTransform.SetLocalRotation( math::ConstructQuatFromVec( math::vec3(0, 1, 0), math::vec3(1, 0, 0) ) );
		objectTransform.SetLocalScale( math::vec3(1) );

		{
			Entity renderTestEntity = CreateEntity();
			renderTestEntity.AddComponent(mesh);
			renderTestEntity.AddComponent(objectTransform);
		}
		
		{
			objectTransform.SetLocalPosition(math::vec3(2, 0, 0));

			Entity renderTestEntity = CreateEntity();
			renderTestEntity.AddComponent(mesh);
			renderTestEntity.AddComponent(objectTransform);
		}

		{
			objectTransform.SetLocalPosition(math::vec3(-2, 0, 0));

			Entity renderTestEntity = CreateEntity();
			renderTestEntity.AddComponent(mesh);
			renderTestEntity.AddComponent(objectTransform);
		}

		{
			objectTransform.SetLocalPosition(math::vec3(0, 2, 0));

			Entity renderTestEntity = CreateEntity();
			renderTestEntity.AddComponent(mesh);
			renderTestEntity.AddComponent(objectTransform);
		}

		{
			objectTransform.SetLocalPosition(math::vec3(0, -2, 0));

			Entity renderTestEntity = CreateEntity();
			renderTestEntity.AddComponent(mesh);
			renderTestEntity.AddComponent(objectTransform);
		}

		//Entity testHouse2 = CreateEntity();
	}

	void ExampleSystem::Run()
	{
		////find player archetype
		//ArchetypeQuery playerQuery = FindArchetypesWith<PlayerGun>();
		//ITV_LOG("Number of archetypes in playerQuery {0} ", playerQuery.Size());

		//for (Archetype& archetype : playerQuery)
		//{
		//	auto compArrayHandle = archetype.GetComponentArray<PlayerGun>();

		//	ITV_LOG("	Number of players in archetype {0} ", archetype.GetEntityCount());

		//	for (size_t i = 0; i < archetype.GetEntityCount(); i++)
		//	{
		//		ITV_LOG("		Current gun mode is {0} ", static_cast<int>(compArrayHandle[i].gunIdentifier) );
		//	}
		//}



		////if player scrolls up
		//	//switch gun up

		////if player scrolls down 
		//	//switch gun down

		////if player mouse click left
		//	//shoot gun

		////if player rotate
		//	//rotate player

		////find Spawner areas
		//ArchetypeQuery spawnAreaQuery = FindArchetypesWith<EnemySpawnArea>();
		//ITV_LOG("Number of archetypes in spawnAreaQuery {0} ", spawnAreaQuery.Size());

		//for (Archetype& archetype : spawnAreaQuery)
		//{
		//	auto compArrayHandle = archetype.GetComponentArray<EnemySpawnArea>();

		//	ITV_LOG("Number of spawn areas in archetype {0} ", archetype.GetEntityCount());

		//	for (size_t i = 0; i < archetype.GetEntityCount(); i++)
		//	{
		//		ITV_LOG("		Current spawn area gun mode is {0} ", static_cast<int>(compArrayHandle[i].gunIdentifier));
		//	}

		//}

		//for each spawner area 
			//check if its time to spawn
				//spawn if it is 


		//for each EnemyHealthComponent
			//move component in direction
			
		



	}
}

