#pragma once
#include "Core.h"
namespace itv
{
	class ITV_API Application
	{
	public:
		Application() = default;
		virtual ~Application() = default;

		void Run();
	};

	Application* CreateApplication();
}
