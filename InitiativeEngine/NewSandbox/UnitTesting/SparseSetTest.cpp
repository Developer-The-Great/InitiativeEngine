#include "catch.hpp"
#include "Initiative\Containers\sparse_set.h"

SCENARIO("values are correctly added to the sparse set", "[sparse_set]")
{
	GIVEN("An Empty sparse_set")
	{
		itv::sparse_set<size_t> sparseSet;
		REQUIRE( sparseSet.size() == 0 );

		WHEN("A unique value is added")
		{
			sparseSet.push_back_unique(1);

			THEN("the value is then added to the dense container")
			{
				REQUIRE(sparseSet[0] == 1);
			}

			THEN("the sparse container contains the index of the value in the dense container")
			{
				REQUIRE(sparseSet.find_dense_index_of(1) == 0);
			}

			THEN("the size of the sparse_set is updated")
			{
				REQUIRE(sparseSet.size() == 1);
			}
		}

		WHEN("A NON unique value is added")
		{
			sparseSet.push_back_unique(1);
			sparseSet.push_back_unique(1);

			THEN("the value is *NOT* added to the dense container")
			{
				REQUIRE(sparseSet.size() == 1);
			}

		}

		WHEN("A number of unique values are added")
		{
			REQUIRE(sparseSet.size() == 0);

			sparseSet.push_back_unique(3);
			sparseSet.push_back_unique(5);
			sparseSet.push_back_unique(4);

			THEN("Calling the contain function on values that are present in the sparse_set yields TRUE")
			{
				REQUIRE(sparseSet.contains(3));
				REQUIRE(sparseSet.contains(5));
				REQUIRE(sparseSet.contains(4));

			}

			THEN("Calling the contain function on values that are *NOT* in the sparse set yields FALSE")
			{
				REQUIRE(!sparseSet.contains(2));
				REQUIRE(!sparseSet.contains(6));
				REQUIRE(!sparseSet.contains(7));
				REQUIRE(!sparseSet.contains(8));

			}

			THEN("the sparse container contains the index of the value in the dense container")
			{
				REQUIRE(sparseSet.find_dense_index_of(3) == 0);
				REQUIRE(sparseSet.find_dense_index_of(5) == 1);
				REQUIRE(sparseSet.find_dense_index_of(4) == 2);
			}
		}
	}
}

SCENARIO("values are correctly removed from the sparse set", "[sparse_set]")
{
	GIVEN("A sparse_set with a number of elements")
	{
		itv::sparse_set<size_t> sparseSet;
		sparseSet.push_back_unique(7);
		sparseSet.push_back_unique(3);
		sparseSet.push_back_unique(11);
		sparseSet.push_back_unique(17);

		REQUIRE(sparseSet.size() == 4);

		WHEN("A value that exist in the sparse_set is removed ")
		{
			sparseSet.remove(3);

			THEN("the sparse set does not contain the value anymore")
			{
				REQUIRE(!sparseSet.contains(3));
			}

			THEN("the sparse set does size is updated")
			{
				REQUIRE(sparseSet.size() == 3);
			}

		}

		WHEN("A value that does *NOT* exist in the sparse_set is removed ")
		{
			bool isRemoved = sparseSet.remove(18);

			THEN("the remove function returns false")
			{
				REQUIRE(!isRemoved);
			}

			THEN("the sparse set size is not updated")
			{
				REQUIRE(sparseSet.size() == 4);
			}
		}
	}
}

SCENARIO("we can iterate through the dense container of the sparse_set", "[sparse_set]")
{
	GIVEN("A sparse_set with a number of elements")
	{
		itv::sparse_set<size_t> sparseSet;
		sparseSet.push_back_unique(20);
		sparseSet.push_back_unique(3);
		sparseSet.push_back_unique(7);
		sparseSet.push_back_unique(4);

		REQUIRE(sparseSet.size() == 4);

		WHEN("we iterate through the sparse set with a range based for loop and store them in a std::vector")
		{
			std::vector<size_t> collectedElements;

			for (size_t value : sparseSet)
			{
				collectedElements.push_back(value);
			}

			THEN("the std::vector contains the elements in order")
			{
				REQUIRE(collectedElements[0] == 20);
				REQUIRE(collectedElements[1] == 3);
				REQUIRE(collectedElements[2] == 7);
				REQUIRE(collectedElements[3] == 4);
			}

		}
	}
}


