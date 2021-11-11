#pragma once
#include "Initiative\Core.h"
#include "Initiative\ECS\ComponentArray.h"
#include "Initiative\ECS\ECSUtils.h"

#include "Initiative\ECS\Entity.h"

#include "Initiative\TypeHashing.h"
#include "Initiative\Containers\sparse_set.h"
#include "Initiative\Log.h"

namespace itv
{
	constexpr TypeID invalid_entity_id = std::numeric_limits<size_t>::max();
	constexpr TypeID world_entity_id = 1;

	class Archetype;
	class ArchetypeManager;
	class ECSSystemBase;

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

		ITV_API void moveExistingEntity(TypeID id, Archetype& newArchetype);

		void instatiateNewComponentArray(const TypeID id);

		Archetype(const ArchetypeType& archetypeType);

	public:

		Archetype(const Archetype& other) = delete;

		Archetype& operator=(const Archetype& other) = delete;

		Archetype(Archetype&& other)
		{
			mArchetypeType = std::move(other.mArchetypeType);
			mComponentArrays = std::move(other.mComponentArrays);
			mEntities = std::move(other.mEntities);
		}

		Archetype& operator=(Archetype&& other)
		{
			mArchetypeType = std::move(other.mArchetypeType);
			mComponentArrays = std::move(other.mComponentArrays);
			mEntities = std::move(other.mEntities);
		}

		inline size_t GetComponentArrayCount() const			  { return mComponentArrays.size(); }

		ArchetypeType GetArchetypeTypeCopy() const				  { return mArchetypeType; }

		const  ArchetypeType& GetArchetypeType() const			  { return mArchetypeType; }

		inline size_t GetEntityIndex(Entity entity) const         
		{ 
			#if _DEBUG

			if (!mEntities.contains(entity))
			{
				return invalid_entity_id;
			}

			#endif

			return mEntities.find_dense_index_of(entity); 
		}

		inline size_t GetEntityCount() const				      { return mEntities.size(); }

		template<class Component> 
			ComponentArrayHandle<Component> GetComponentArray(); //TODO Should probably be const

		ITV_API void* FindComponentArrayOfType(TypeID typeHash);

	};

	typedef std::function<void(size_t sourceIndex,void* source, void* destination)>  componentArrayFunc;
	typedef std::function<std::unique_ptr<ComponentArrayBase>()> componentArrayInstantiationFunc;

	class ArchetypeManager
	{
		friend class Entity;
	private:

		static TypeID nextEntityID;

		struct componentActionFuncs
		{
			componentArrayInstantiationFunc		 mTypeToComponentInstantiationFunc;
			componentArrayFunc					 mTypeToComponentArrayMove;
		};

		std::unordered_map<TypeID, 
			componentActionFuncs>				 mComponentHashToComponentActions;

		std::unordered_map<TypeID, size_t>		 mEntityRecords;
		std::vector<Archetype>					 mArchetypes;



		ITV_API Archetype&						InstantiateArchetype(const ArchetypeType& types);

		ITV_API void						    UpdateEntityRecord(TypeID id,size_t newArchetypeIndex);

		template <typename... ComponentTypes>
			typename std::enable_if
			<sizeof...(ComponentTypes) == 0>
				::type							collectComponentHashes(std::vector<size_t>& generatedHashes) {};

		template<typename ComponentType,
			typename... ComponentTypes>
				void							collectComponentHashes(std::vector<size_t>& generatedHashes);

	public:

		

		ITV_API									ArchetypeManager();
												~ArchetypeManager() = default;

		inline size_t						    GetArchetypeCount() const { return mArchetypes.size(); }

		ITV_API ArchetypeQuery					GetArchetypesWith(const ArchetypeType& type);

		template<typename ComponentType,
			typename... ComponentTypes>
				ArchetypeQuery				    FindArchetypesWith();

		ITV_API std::optional
			<std::reference_wrapper<Archetype>> GetArchetype(const ArchetypeType& type);

		ITV_API std::optional
			<std::reference_wrapper<Archetype>> GetArchetype(const ArchetypeType& type,size_t& outIndex);

		ITV_API Archetype& 						GetRecord(TypeID id);

		ITV_API Entity 							CreateEntity();

		ITV_API Entity							GetWorldEntity() const;

		ITV_API componentArrayFunc&				GetComponentArrayMoveFunc(TypeID componentType);

		componentArrayInstantiationFunc&		GetComponentArrayInstantiationFunc(TypeID componentType);

		template<class Component> void			RegisterComponent();

		ITV_API void							RegisterGenericComponents();

		template<class Component> bool			IsComponentRegistered();

	};

	template<class Component>
	inline ComponentArrayHandle<Component> Archetype::GetComponentArray()
	{
		constexpr TypeID componentHash = GenerateTypeHash<Component>();

		void*  foundComponentArray = FindComponentArrayOfType(componentHash);

		assert(foundComponentArray);

		std::vector<Component>* newComponentDestination =
			static_cast<std::vector<Component>*>(foundComponentArray);

		return ComponentArrayHandle<Component>( newComponentDestination->data(),mEntities.size() );
	}

}

#include "ArchetypeManager.inl"
#include "Entity.inl"

