#pragma once
#include "Event.h"

namespace itv
{
	struct InputEvent final : public EventBase
	{
		InputEvent(int key, int action, int mods) : Key(key), Action(action), Mods(mods)
		{

		}

		int Key;
		int Action;
		int Mods;
	};

	struct MouseMoveEvent final : public EventBase
	{
		MouseMoveEvent(int currentX, int currentY, int deltaX, int deltaY) :
			CurrentX(currentX), CurrentY(currentY), DeltaX(deltaX), DeltaY(deltaY)
		{
		}

		int CurrentX, CurrentY;
		int DeltaX, DeltaY;

	};



}