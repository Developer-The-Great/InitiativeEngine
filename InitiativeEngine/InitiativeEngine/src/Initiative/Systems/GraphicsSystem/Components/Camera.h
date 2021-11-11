#pragma once
#include "Initiative\Core.h"

namespace itv
{
	struct Camera
	{
		float Fovy;
		float NearPlane;
		float FarPlane;

		Camera() = default;
		Camera(float fovy, float nearPlane, float farPlane) 
			: Fovy(fovy),NearPlane(nearPlane),FarPlane(farPlane)
		{

		}



	};



}
