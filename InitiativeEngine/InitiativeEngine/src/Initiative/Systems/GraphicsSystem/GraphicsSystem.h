#pragma once
#include "Initiative\ECS\ECSSystemBase.h"

namespace itv
{
	class GraphicsSystem : public ECSSystemBase
	{
	private:

	public:

		void						    RegisterComponents() override;

		void							BeginRun() override;

		void						    Run() override;

	};

}

