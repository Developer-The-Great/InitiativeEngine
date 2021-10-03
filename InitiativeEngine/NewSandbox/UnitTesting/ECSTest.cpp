#include "catch.hpp"
#include "Initiative/ECS/ArchetypeManager.h"

using namespace itv;

SCENARIO("ArchetypeManager puts all empty entities in the default transform", "[ECS]")
{
	GIVEN("An archetype manager")
	{
		ArchetypeManager archetypeManager;

		WHEN("An entity is initialized")
		{
			itv::Entity firstEnt = archetypeManager.CreateEntity();

			THEN("It will be placed in the first archetype that has no types")
			{
				Archetype& archetypeInRecord = archetypeManager.GetRecord(firstEnt.GetID());

				ArchetypeType noType;
				auto optionalArchetype = archetypeManager.GetArchetype(noType);

				REQUIRE( &optionalArchetype.value().get() == &archetypeInRecord );

			}
		}
	}
}

SCENARIO("when a component is added to an entity it is moved from its original archetype to another", "[ECS]")
{
	GIVEN("An archetype manager with an initialized entity")
	{
		ArchetypeManager archetypeManager;
		itv::Entity firstEnt = archetypeManager.CreateEntity();

		struct TestComponent1
		{
			float a, b, c;
		};

		struct TestComponent2
		{
			int b;
		};

		struct TestComponent3
		{
			char b;
		};

		constexpr size_t comp1Hash = GenerateTypeHash<TestComponent1>();
		constexpr size_t comp2Hash = GenerateTypeHash<TestComponent2>();
		constexpr size_t comp3Hash = GenerateTypeHash<TestComponent3>();

		archetypeManager.RegisterComponent<TestComponent1>();
		archetypeManager.RegisterComponent<TestComponent2>();
		archetypeManager.RegisterComponent<TestComponent3>();

		REQUIRE( comp1Hash != comp2Hash );
		REQUIRE( comp1Hash != comp3Hash );
		REQUIRE( comp2Hash != comp3Hash );


		WHEN("A the first component is added")
		{
			TestComponent1 testComponent;

			firstEnt.AddComponent(testComponent);

			auto comp1Arch = archetypeManager.GetArchetype
			(ArchetypeType(std::vector<size_t>{comp1Hash}));

			THEN("The entity will be moved to the correct archetype")
			{
				Archetype& archetypeInRecord = archetypeManager.GetRecord(firstEnt.GetID());

				REQUIRE(&comp1Arch.value().get() == &archetypeInRecord);
				REQUIRE( comp1Arch.value().get().GetEntityIndex(firstEnt) == 0 );
			}

			THEN("The entity cannot be found in the old archetype")
			{
				ArchetypeType noType;
				auto optionalArchetype = archetypeManager.GetArchetype(noType);

				REQUIRE( optionalArchetype.value().get().GetEntityIndex(firstEnt) == std::numeric_limits<size_t>::max() );
			}

			THEN("Calling has component on the added component yields TRUE")
			{
				REQUIRE( firstEnt.HasComponent< TestComponent1 >() );
			}

			THEN("Calling has component on a component that has *NOT* been added yields FALSE")
			{
				REQUIRE( !firstEnt.HasComponent< TestComponent2 >() );
			}

			WHEN("A second component that has not been added is added")
			{
				TestComponent2 testComponent2;
				firstEnt.AddComponent(testComponent2);

				auto comp12Arch = archetypeManager.GetArchetype
				(ArchetypeType(std::vector<size_t>{comp1Hash,comp2Hash}));
				
				THEN("The entity is not in the old archetype")
				{
					Archetype& archetypeInRecord = archetypeManager.GetRecord(firstEnt.GetID());

					REQUIRE(&comp1Arch.value().get() != &archetypeInRecord);
					REQUIRE(&comp12Arch.value().get() == &archetypeInRecord);
				}

				THEN("Calling has component on the added components yields TRUE")
				{
					REQUIRE(firstEnt.HasComponent< TestComponent1 >());
					REQUIRE(firstEnt.HasComponent< TestComponent2 >());
				}
			}
		}
	}
}

SCENARIO("When the archetypemanager is queried for a certain archetype it gets the correct archetype ", "[ECS]")
{
	GIVEN("An archetype manager with a number of initialized entities located in different archetypes")
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
		scale Scale;

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
		posRotScaleEnt.AddComponent(Scale);

		WHEN("We query for entities with the position entity ")
		{
			itv::ArchetypeQuery positionQuery = archetypeManager.FindArchetypesWith<position>();

			THEN("we get all entities that have this query")
			{
				size_t totalComponentsFound = 0;

				for (Archetype& archetype : positionQuery)
				{
					auto componentArray = archetype.GetComponentArray<position>();

					totalComponentsFound += componentArray.Size();

				}
			
				REQUIRE( totalComponentsFound == 4 );

			}
		}

		WHEN("We query for entities with the rotation entity ")
		{
			itv::ArchetypeQuery rotationQuery = archetypeManager.FindArchetypesWith<rotation>();

			THEN("we get all entities that have this query")
			{
				size_t totalComponentsFound = 0;

				for (Archetype& archetype : rotationQuery)
				{
					auto componentArray = archetype.GetComponentArray<rotation>();

					totalComponentsFound += componentArray.Size();

				}

				REQUIRE(totalComponentsFound == 2);
			}
		}

		WHEN("We query for entities with the scale entity ")
		{
			itv::ArchetypeQuery scaleQuery = archetypeManager.FindArchetypesWith<scale>();

			THEN("we get all entities that have this query")
			{
				size_t totalComponentsFound = 0;

				for (Archetype& archetype : scaleQuery)
				{
					auto componentArray = archetype.GetComponentArray<scale>();

					totalComponentsFound += componentArray.Size();

				}

				REQUIRE(totalComponentsFound == 1);
			}
		}


	}
}