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
	PoolAllocator(void *block_, size_t sz) : block(block_), activeSz(0), maxSz(0)
	{
		// NOTE(pf): Internal freelist uses the free'd memory to track elements
		// that can be reused, but this means that pool allocated objects have a
		// size requirement to hold these pointers.
		static_assert(sizeof(T) >= sizeof(T *) + sizeof(FreeList *));
		freeList = nullptr;
	}
	~PoolAllocator() = default;

	T *Allocate()
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
			result = ((u8*)block) + activeSz;
			activeSz += sizeof(T);
		}
		return result;
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

private:
	struct FreeList
	{
		T *el;
		FreeList *next;
	};
	void *block;
	size_t activeSz;
	size_t maxSz;
	FreeList *freeList;
};
#endif
