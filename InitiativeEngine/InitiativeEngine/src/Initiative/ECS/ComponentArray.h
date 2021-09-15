#pragma once
#include "Initiative\Core.h"
#include "Initiative\Log.h"
#include "Initiative\TypeHashing.h"

namespace itv
{
	class ComponentArrayBase
	{
	protected:

		void* mComp = nullptr;

	public:
		ComponentArrayBase() = default;
		~ComponentArrayBase() = default;

		void* GetCompArrayPtr() const { return mComp; }

#if _DEBUG
		//debug purposes only, since the archetype knows how many entities it has.
		virtual size_t GetArraySize() const = 0;
#endif

		template<class Component>
		static void RemoveComponent(void* vector, size_t index)
		{
			std::vector<Component>& ptrToVector = * static_cast<std::vector<Component>*>(vector);

			std::swap(ptrToVector->at(index), ptrToVector->at(ptrToVector->size() - 1));

			ptrToVector.pop_back();

		}

		static void AddComponent(void* vector, size_t index)
		{

		}

		template<class Component>
		static void MoveComponent( size_t sourceIndex, void* source, void* destination )
		{
			ITV_LOG("	MoveComponent: {0}", GET_FUNC_SIGNATURE);

			std::vector<Component>& sourceVector	  = *static_cast<std::vector<Component>*>( source );
			std::vector<Component>& destinationVector = *static_cast<std::vector<Component>*>( destination );

			std::swap( sourceVector[sourceIndex], sourceVector[sourceVector.size() - 1] );

			destinationVector.emplace_back( std::move(sourceVector[sourceVector.size() - 1]) );
			sourceVector.pop_back();

		}

	};

	template<class ComponentType>
	class ComponentArray : public ComponentArrayBase
	{
		friend class ArchetypeManager;

	private:
		std::vector<ComponentType> mComponents;

		ComponentArray()
		{
			mComp = &mComponents;
		}

	public:

#if _DEBUG
		size_t GetArraySize() const override { return mComponents.size(); }
#endif

	};

}
