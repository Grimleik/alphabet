#if !defined(POOLALLOCATOR_H)
/* ========================================================================
	Creator: Grimleik $
========================================================================*/
#define POOLALLOCATOR_H
#include "memorymanager.h"

template <typename T>
class PoolAllocator : public IAllocator
{
public:
	PoolAllocator(void *block_, size_t sz) : IAllocator(block_, sz), freeList(nullptr)
	{
	}

	~PoolAllocator() = default;

	void Init(void *block_, size_t sz)
	{
		block = block_;
		maxSz = sz;
		activeSz = 0;
		freeList = nullptr;
	}

	void *Allocate(size_t size)
	{
		T *result;
		if (freeList)
		{
			result = reinterpret_cast<T *>(freeList);
			freeList = freeList->next;
		}
		else
		{
			AGE_AssertCheck(sizeof(T) + activeSz < maxSz, "Pool Allocator out of memory.");
			result = (T *)(((u8 *)block) + activeSz);
			// size_t allocSize = 40;//sizeof(T) + Align(sizeof(T), alignof(T));
			size_t allocSize = size;
			AGE_LOG(LOG_LEVEL::DEBUG, "Allocating {} bytes at {}", allocSize, (void *)result);
			activeSz += allocSize;
		}
		return result;
	}

	T *Allocate()
	{
		return (T *)Allocate(sizeof(T));
	}

	T *AllocateAsZero()
	{
		T *result = Allocate();
		*result = {};
		return result;
	}

	void Free(T *elem)
	{
		FreeList *free = reinterpret_cast<FreeList *>(elem);
		if (freeList)
		{
			free->next = freeList;
			freeList = free;
		}
		else
			freeList = free;
	}

	size_t GetRemainingSize() { return maxSz - activeSz; }
	size_t GetUsedSize() { return activeSz; }

	void Reset() override
	{
		freeList = nullptr;
		activeSz = 0;
	}

private:
	struct FreeList
	{
		T *el;
		FreeList *next;
	};
	FreeList *freeList;
};
#endif
