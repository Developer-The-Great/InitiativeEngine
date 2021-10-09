#pragma once
#include "Initiative\Core.h"
#include "Initiative\math.h"

namespace itv
{
	enum class GUN_IDENTIFIER
	{
		LEFT,
		MIDDLE,
		RIGHT,
		MAX
	};

	struct EnemyHealthComponent
	{
		int numberOfHitsLeft = 1;
		GUN_IDENTIFIER gunIdentifier = GUN_IDENTIFIER::MAX;

	};

	struct PlayerGun
	{
		GUN_IDENTIFIER gunIdentifier = GUN_IDENTIFIER::MAX;

	};

	struct ScoreTracker
	{
		float currentScore;


	};

	struct EnemySpawnArea
	{
		GUN_IDENTIFIER gunIdentifier = GUN_IDENTIFIER::MAX;
	};


}