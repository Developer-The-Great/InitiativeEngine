#pragma once
#include "Initiative\Core.h"


namespace itv
{
	//Events in the Initiative Engine are currently blocking. This means that the moment 
	//that ObserverSubject::RaiseEvent<Event> is called, the event will be dispatched and handled
	//immediately.

	//TODO implement an event bus

	class EventBase
	{
	public:
		EventBase() = default;
		~EventBase() = default;
	};

	class ObserverSubject;

	class ITV_API ObserverBase
	{
		friend class ObserverSubject;
	protected:

		ObserverSubject* mSubject = nullptr;

	};

	template<class Event>
	class Observer final : public ObserverBase
	{
		friend class ObserverSubject;
		typedef std::function<void(Event&)> EventFunc; //TODO replace implementation of EventFunc with "impossibly fast delegates"

	public:
		Observer() 
		{ 
			static_assert(std::is_base_of<EventBase, Event>::value); 

			static size_t observerIdentifierCounter = 0;

			mObserverIdentifier = observerIdentifierCounter;
			observerIdentifierCounter++;
		};

		~Observer()
		{
			if (mSubject)
			{
				unRegisterFromCurrentSubject();
			}
		}

		size_t GetObserverIdentifier() const { return mObserverIdentifier; }

		constexpr size_t GetObservingEventType() const { return typeid(Event).hash_code(); }

		void SetEventFunc(EventFunc newEventFunc) { mEventFunc = newEventFunc; }

	private:

		void handleEvent(Event& event) { mEventFunc(event); }

		void unRegisterFromCurrentSubject();
		
		EventFunc mEventFunc;

		size_t mObserverIdentifier;

	};

	class ITV_API ObserverSubject final
	{
	public:
		ObserverSubject(size_t vectorReserve = 4)
		{
			mObservers.reserve(vectorReserve);
			mObserverIdentifiers.reserve(vectorReserve);
			mObserverType.reserve(vectorReserve);
		}

		~ObserverSubject()
		{
			for (ObserverBase * observer : mObservers)
			{
				observer->mSubject = nullptr;
			}
		}

		template<class Event>
		void RegisterObserver(Observer<Event>& observer)
		{
			static_assert(std::is_base_of<EventBase, Event>::value);

			observer.unRegisterFromCurrentSubject();
			observer.mSubject = this;

			mObservers.push_back(&observer);
			mObserverIdentifiers.push_back(observer.GetObserverIdentifier());
			mObserverType.push_back(observer.GetObservingEventType());
		}

		template<class Event>
		void UnRegisterObserver(Observer<Event>& observer)
		{
			size_t id = observer.GetObserverIdentifier();
			size_t foundIdentifier = mObservers.size();

			for (size_t observerID : mObserverIdentifiers)
			{
				if (observerID == id)
				{
					foundIdentifier = observerID;
					break;
				}
			}

			if (foundIdentifier != mObservers.size())
			{
				size_t lastElement = mObservers.size() - 1;

				std::swap( mObservers.at(foundIdentifier), mObservers.at(lastElement) );
				std::swap( mObserverIdentifiers.at(foundIdentifier), mObserverIdentifiers.at(lastElement) );
				std::swap( mObserverType.at(foundIdentifier), mObserverType.at(lastElement) );

				mObservers.pop_back();
				mObserverIdentifiers.pop_back();
				mObserverType.pop_back();
			}
		}

		template<class Event>
		void RaiseEvent(Event& event)
		{
			static_assert(std::is_base_of<EventBase, Event>::value);

			static size_t eventHashCode = typeid(Event).hash_code();

			for (size_t i = 0; i < mObservers.size(); i++)
			{
				if (eventHashCode == mObserverType.at(i))
				{
					auto castedObserver = static_cast<Observer<Event>*>(mObservers.at(i));
					castedObserver->handleEvent(event);
				}
			}

		}

	private:

		std::vector<size_t> mObserverType;
		std::vector<size_t> mObserverIdentifiers;
		std::vector< ObserverBase*> mObservers;

	};

	template<class T> void Observer<T>::unRegisterFromCurrentSubject()
	{
		if (mSubject)
		{
			auto& observer = *this;
			mSubject->UnRegisterObserver(observer);
		}
	}



}


