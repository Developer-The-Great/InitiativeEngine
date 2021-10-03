#include "ECSUtils.h"
#include "ArchetypeManager.h"

namespace itv
{
	Archetype& ArchetypeQueryIterator::operator++()
	{
		mArchetypePtr++;
		return *mArchetypePtr;
	}

	Archetype& ArchetypeQueryIterator::operator*()
	{
		return *mArchetypePtr;
	}
	ITV_API bool ArchetypeQueryIterator::operator==(const ArchetypeQueryIterator& other)
	{
		return mArchetypePtr == other.mArchetypePtr;
	}
	ITV_API bool ArchetypeQueryIterator::operator!=(const ArchetypeQueryIterator& other)
	{
		return mArchetypePtr != other.mArchetypePtr;
	}

	inline ArchetypeQueryIterator ArchetypeQuery::begin()
	{
		return ArchetypeQueryIterator(*mArchetypes.data());
	}

	inline ArchetypeQueryIterator ArchetypeQuery::end()
	{
		return ArchetypeQueryIterator(*mArchetypes.data() + mArchetypes.size());
	}

}


