#include "ExampleSystem.h"
#include "ExampleComponents\SimpleFPSComponents.h"
#include "Initiative\ECS\ArchetypeManager.h"

namespace itv
{
	void ExampleSystem::RegisterComponents()
	{
		RegisterComponent<EnemyHealthComponent>();
		RegisterComponent<PlayerGun>();
		RegisterComponent<ScoreTracker>();
		RegisterComponent<EnemySpawnArea>();
	}

	void ExampleSystem::BeginRun()
	{
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

	}

	void ExampleSystem::Run()
	{
		//find player archetype
		ArchetypeQuery playerQuery = FindArchetypesWith<PlayerGun>();
		ITV_LOG("Number of archetypes in playerQuery {0} ", playerQuery.Size());

		for (Archetype& archetype : playerQuery)
		{
			auto compArrayHandle = archetype.GetComponentArray<PlayerGun>();

			ITV_LOG("	Number of players in archetype {0} ", archetype.GetEntityCount());

			for (size_t i = 0; i < archetype.GetEntityCount(); i++)
			{
				ITV_LOG("		Current gun mode is {0} ", static_cast<int>(compArrayHandle[i].gunIdentifier) );
			}
		}



		//if player scrolls up
			//switch gun up

		//if player scrolls down 
			//switch gun down

		//if player mouse click left
			//shoot gun

		//if player rotate
			//rotate player

		//find Spawner areas
		ArchetypeQuery spawnAreaQuery = FindArchetypesWith<EnemySpawnArea>();
		ITV_LOG("Number of archetypes in spawnAreaQuery {0} ", spawnAreaQuery.Size());

		for (Archetype& archetype : spawnAreaQuery)
		{
			auto compArrayHandle = archetype.GetComponentArray<EnemySpawnArea>();

			ITV_LOG("Number of spawn areas in archetype {0} ", archetype.GetEntityCount());

			for (size_t i = 0; i < archetype.GetEntityCount(); i++)
			{
				ITV_LOG("		Current spawn area gun mode is {0} ", static_cast<int>(compArrayHandle[i].gunIdentifier));
			}

		}

		//for each spawner area 
			//check if its time to spawn
				//spawn if it is 


		//for each EnemyHealthComponent
			//move component in direction
			
		



	}
}

