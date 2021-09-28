#pragma once

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

namespace itv
{
	int ExecuteUnitTests(int argc, char** argv)
	{
		return Catch::Session().run(argc, argv);
	}
}