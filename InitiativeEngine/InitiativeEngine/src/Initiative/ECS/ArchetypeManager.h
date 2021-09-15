#pragma once
#include "Initiative\Core.h"
#include "Initiative\ECS\ComponentArray.h"
#include "Initiative\TypeHashing.h"
#include "Initiative\Containers\sparse_set.h"
#include "Initiative\Log.h"

namespace itv
{
	typedef size_t TypeID;

	#define INVALID_ENTITY 0

	class Archetype;
	class ArchetypeManager;

	class Entity
	{
		friend class ArchetypeManager;
		friend class Archetype;

	private:

		TypeID mID;

		static ArchetypeManager* sArchetypeManager;

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

	class ArchetypeType
	{
	private:

		std::vector<TypeID> mTypes;

	public:

		ArchetypeType(std::vector<TypeID>&& types) : mTypes(std::move(types)) {}

		ArchetypeType(const std::vector<TypeID>& types) : mTypes(types) {}

		ArchetypeType() = default;

		inline TypeID Size() const { return mTypes.size(); }

		inline TypeID At(size_t index) const { return mTypes[index]; }

		void AddType(TypeID id)
		{
			mTypes.push_back(id);
			std::sort(mTypes.begin(), mTypes.end());
		}

		inline bool operator==(const ArchetypeType& types) const
		{
			//early exit if ArchetypeTypes are empty or are not equal in length

			if (mTypes.empty() && types.mTypes.empty()) return true;
			
			if (mTypes.size() != types.mTypes.size()) return false;

			for (size_t i = 0; i < mTypes.size(); i++)
			{
				if (mTypes[i] != types.mTypes[i])
				{
					return false;
				}
			}

			return true;
		}
	};



	class Archetype
	{
		friend class ArchetypeManager;
		friend class Entity;
	private:

		ArchetypeType							 mArchetypeType;

		std::vector<
			std::unique_ptr<ComponentArrayBase>> mComponentArrays;

		sparse_set<Entity>						 mEntities;

		static ArchetypeManager*				 sArchetypeManager;

		void moveExistingEntity(TypeID id, Archetype& newArchetype);

		void instatiateNewComponentArray(const TypeID id);

		Archetype(const ArchetypeType& archetypeType);

	public:

		Archetype(const Archetype& other) = delete;

		Archetype(Archetype&& other)
		{
			mArchetypeType = std::move(other.mArchetypeType);
			mComponentArrays = std::move(other.mComponentArrays);
			mEntities = std::move(other.mEntities);
		}

		Archetype& operator=(const Archetype& other) = delete;

		Archetype& operator=(Archetype&& other)
		{
			mArchetypeType = std::move(other.mArchetypeType);
			mComponentArrays = std::move(other.mComponentArrays);
			mEntities = std::move(other.mEntities);
		}

		inline size_t GetComponentArrayCount() const			  { return mComponentArrays.size(); }

		ArchetypeType GetArchetypeTypeCopy() const				  { return mArchetypeType; }

		const ArchetypeType& GetArchetypeType() const			  { return mArchetypeType; }

		inline size_t GetEntityIndex(Entity entity) const         { return mEntities.find_dense_index_of(entity); }

		void MoveEntityAtTo(TypeID id, Archetype& newArchetype);

		void* FindComponentArrayOfType(TypeID typeHash);

	};

	class ArchetypeQuery
	{
	public:

	private:

		std::vector<Archetype*> mArchetypes;

	};

	typedef std::function<void(size_t sourceIndex,void* source, void* destination)>  componentArrayFunc;
	typedef std::function<std::unique_ptr<ComponentArrayBase>()> componentArrayInstantiationFunc;

	class ArchetypeManager 
	{
		friend class Entity;

	private:

		struct componentActionFuncs
		{
			componentArrayInstantiationFunc mTypeToComponentInstantiationFunc;
			componentArrayFunc			    mTypeToComponentArrayMove;
		};

		std::unordered_map<TypeID, 
			componentActionFuncs>				 mComponentHashToComponentActions;

		std::unordered_map<TypeID, size_t>		 mEntityRecords;
		std::vector<Archetype>					 mArchetypes;

		Archetype& InstantiateArchetype(const ArchetypeType& types);

		void UpdateEntityRecord(TypeID id,size_t newArchetypeIndex);

	public:

		ArchetypeManager();

		inline size_t						    GetArchetypeCount() const { return mArchetypes.size(); }

		ArchetypeQuery						    GetArchetypesWith(const ArchetypeType& types);

		std::optional
			<std::reference_wrapper<Archetype>> GetArchetype(const ArchetypeType& types);

		Archetype&							    GetRecord(TypeID id);

		Entity 									CreateEntity();

		componentArrayFunc&						GetComponentArrayMoveFunc(TypeID componentType);

		componentArrayInstantiationFunc&		GetComponentArrayInstantiationFunc(TypeID componentType);

		template<class Component> void			RegisterComponent();

		template<class Component> bool			IsComponentRegistered() 
		{ 
			constexpr TypeID componentHash = GenerateTypeHash<Component>();
			return mComponentHashToComponentActions.find(componentHash) != mComponentHashToComponentActions.end();
		}
		


	};

	template<class Component>
	void Entity::AddComponent(Component& component)
	{
		ITV_LOG("Add Component: {0}", GET_FUNC_SIGNATURE);

		constexpr TypeID componentHash = GenerateTypeHash<Component>();

#ifdef _DEBUG
		assert(sArchetypeManager->IsComponentRegistered<Component>());
#endif 

		//[1] Build ArchetypeType of Archetype we are looking for 

		Archetype& oldArchetype = sArchetypeManager->GetRecord(mID);

		ArchetypeType newType = oldArchetype.GetArchetypeTypeCopy();
		newType.AddType(componentHash);

		auto newArchetypeOptionalRef = sArchetypeManager->GetArchetype(newType);

		//[2] Construct a new one destination archetype if it doesnt exist yet
		if ( !newArchetypeOptionalRef.has_value() )
		{
			newArchetypeOptionalRef = sArchetypeManager->InstantiateArchetype(newType);
		}

		Archetype& newArchetype = newArchetypeOptionalRef.value().get();

#ifdef _DEBUG
		assert(&newArchetype != &oldArchetype);
#endif 

		//[3] Get each component of current archetype and move them to the new archetype

		const ArchetypeType& types = oldArchetype.GetArchetypeType();

		size_t oldArchetypeIndex = oldArchetype.GetEntityIndex(mID);

		for (size_t i = 0; i < oldArchetype.GetComponentArrayCount(); i++)
		{

			auto& moveFunc = sArchetypeManager->GetComponentArrayMoveFunc(types.At(i));

			void* currentLoc	 = oldArchetype.mComponentArrays[i]->GetCompArrayPtr();
			void* destinationLoc = newArchetype.FindComponentArrayOfType(types.At(i));

			moveFunc( oldArchetypeIndex,currentLoc,destinationLoc );

		}

		oldArchetype.moveExistingEntity( mID, newArchetype );

		void* destinationLoc = newArchetype.FindComponentArrayOfType(componentHash);

		//[4] add new component to new Archetype

		std::vector<Component> * newComponentDestination = 
			static_cast<std::vector<Component> *>(destinationLoc);

		newComponentDestination->push_back(component);

		//[5] Update entity record

		sArchetypeManager->UpdateEntityRecord(mID, sArchetypeManager->GetArchetypeCount() - 1);

		ITV_LOG("");
	};

	template<class Component>
	bool Entity::HasComponent() const
	{
		return false;
	}


	template<class Component>
	void ArchetypeManager::RegisterComponent()
	{
		constexpr TypeID componentHash = GenerateTypeHash<Component>();
		ITV_LOG("Registered Component with {0} ", componentHash);

		componentActionFuncs componentActions;

		componentActions.mTypeToComponentInstantiationFunc = 
			[]()
		{
			//std::make_unique cannnot used here because ComponentArray constructor is private
			return std::move( std::unique_ptr<ComponentArray<Component>>
				( new ComponentArray<Component>() ) );
		};

		componentActions.mTypeToComponentArrayMove =
			[](size_t sourceIndex, void* source, void* destination)
		{
			ComponentArrayBase::MoveComponent<Component>(sourceIndex, source, destination);
		};

			
		mComponentHashToComponentActions.insert( std::make_pair(componentHash, componentActions ) );

	}

}