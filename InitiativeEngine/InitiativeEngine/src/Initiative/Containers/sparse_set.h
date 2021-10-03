#pragma once
#include "Initiative\Core.h"

namespace itv
{
	template<class Value>
	class sparse_set
	{
	private:

		std::vector<size_t> mSparseContainer;
		std::vector<size_t> mDenseContainer;

		size_t mSize = 0;
		size_t mCapacity = 0;

	public:

		using iterator = typename std::vector<size_t>::iterator;
		
		sparse_set() = default;
		~sparse_set() = default;

		iterator begin() { return mDenseContainer.begin(); }
		iterator end() { return mDenseContainer.begin() + mSize; }

		size_t size() const { return mSize; }

		bool push_back_unique(const Value& value)
		{
			size_t valueID = static_cast<size_t>(value);

			if (!contains(value))
			{
				if (valueID >= mSize)
				{
					reserve(valueID + 1);
				}

				auto iter = mDenseContainer.begin() + mSize;
				*iter = value;

				mSparseContainer[valueID] = mSize;
				++mSize;

				return true;
			}

			return false;
		}

		bool contains(const Value& valueType) const
		{
			return valueType < mCapacity &&
				mSparseContainer[static_cast<size_t>(valueType)] < mSize &&
				mDenseContainer[mSparseContainer[static_cast<size_t>(valueType)]] == valueType;
		}

		size_t find_dense_index_of(const Value& valueType) const
		{
			return mSparseContainer[static_cast<size_t>(valueType)];
		}

		void reserve(size_t size)
		{
			if (size > mCapacity)
			{
				mSparseContainer.resize( size,std::numeric_limits<size_t>::max() );
				mDenseContainer.resize( size, std::numeric_limits<size_t>::max() );
				mCapacity = size;
			}
		}

		bool remove(const Value& value)
		{
			//size_t valueID = static_cast<size_t>(value);

			if (contains(value))
			{
				if (mSparseContainer[ static_cast<size_t>(value) ] != mSize - 1)
				{
					mDenseContainer[mSparseContainer[ static_cast<size_t>(value) ]] = mDenseContainer[mSize - 1];
					mSparseContainer[mDenseContainer[ mSize - 1					 ]] = mSparseContainer[ static_cast<size_t>(value) ];
				}
				--mSize;

				return true;
			}
			return false;
		}

		Value& operator[](size_t index)
		{
			#if _DEBUG
				if (index >= mSize) throw std::out_of_range("sparse_set subsrcipt out of range");
			#endif

			return mDenseContainer[index];
		}





	};
}