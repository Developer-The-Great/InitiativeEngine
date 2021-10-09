#pragma once
#include "Initiative\Core.h"

namespace itv
{
	class ArchetypeManager;

	class Entity
	{
		friend class ArchetypeManager;
		friend class Archetype;

	private:

		TypeID mID;

		ITV_API static ArchetypeManager* sArchetypeManager;

		Entity(TypeID ID) : mID(ID) {};

	public:

		~Entity() = default;
		Entity(Entity&& other) = default;
		Entity(Entity& other) = default;

		inline Entity& operator=(const Entity& other)
		{
			mID = other.mID;
			return *this;
		}

		template<class Component>
		void AddComponent(Component& component);

		template<class Component>
		bool HasComponent() const;

		template<class Component>
		void RemoveComponent(Component& component);

		operator size_t() const { return mID; }

		inline size_t GetID() const { return mID; }

	}; 
}

