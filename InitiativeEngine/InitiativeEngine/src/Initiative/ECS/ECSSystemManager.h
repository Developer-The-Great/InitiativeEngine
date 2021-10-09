#pragma once
#include "Initiative\Core.h"
#include "ECSSystemBase.h"

namespace itv
{
	class ECSSystemBase;
	class ECSSystemManager;
	class ArchetypeManager;

	class ECSSystemRegistrationAdmin
	{
	private:

		ECSSystemManager* mSystemManager = nullptr;

	public:

		ECSSystemRegistrationAdmin(ECSSystemManager* systemManager)
			: mSystemManager(systemManager)
		{

		}
		~ECSSystemRegistrationAdmin() = default;

		template<typename System>	void RegisterSystem();

	};

	class ECSSystemManager
	{
	private:

		std::vector
			< std::unique_ptr<ECSSystemBase> >	 mSystems;

		ArchetypeManager*						 mArchetypeManager = nullptr;

	public:

		ITV_API									ECSSystemManager(ArchetypeManager* archetypeManager);

		ITV_API void							RegisterCoreSystems();

		template <typename System,
			typename = std::enable_if
			< std::is_base_of
			<ECSSystemBase, System>::value >>
			void							    RegisterSystem()
		{
			auto systemPtr = std::make_unique< System >();
			systemPtr->SetArchetypeManager(mArchetypeManager);

			mSystems.emplace_back(std::move(systemPtr));
		}

		ITV_API void							InitializeSystem();

		ITV_API void							RunSystems();

	};

	template<typename System>
	inline void ECSSystemRegistrationAdmin::RegisterSystem()
	{
		mSystemManager->RegisterSystem<System>();
	}




}