#if !defined(MEMORYMANAGER_H)
/* ========================================================================
   Creator: Grimleik $

   NOTE: Currently the MemoryManager can only Partition out memory, there is no
   reclaiming.
   ========================================================================*/
#define MEMORYMANAGER_H

#include "core.h"
#include "singleton.h"
#include <logger.h>

inline size_t Align(size_t szInBytes, size_t alignment)
{
	return (alignment - (szInBytes % alignment)) % alignment;
}

// TODO(pf):
// * Pass a reference to the MemoryManager and return the allocated memory to
//   the memory manager in a virtual destructor. This requires a pass on the
//   memory manager to handle allocations of any size and defragmentation.
class IAllocator
{
public:
	IAllocator(void *block, size_t sizeInBytes) : block(block), maxSz(sizeInBytes), activeSz(0) {}
	virtual ~IAllocator() = default;
	virtual void Reset() = 0;
	virtual size_t GetUsedSize() = 0;
	virtual size_t GetRemainingSize() = 0;
	// TODO:
	// virtual void *Allocate(size_t sizeInBytes) = 0;
	// virtual void Free(void *mem, size_t sizeInBytes) = 0;
	void *Peek() { return (u8 *)block + activeSz; }

protected:
	void *block;
	size_t activeSz;
	size_t maxSz;
	friend class MemoryManager;
};

class MemoryManager : public ISingleton
{
public:
	static void Create(void *mem, size_t size, size_t cacheLineSize);
	static MemoryManager *Instance;

	template <typename T>
	T *PartitionSystem()
	{
		return (T *)PartitionBlock(sizeof(T), true, alignof(T));
	}

	void *PartitionBlock(size_t szInBytes = 0, bool zeroInit = true, size_t alignment = 1);

	template <typename T, typename... Args>
	T *PartitionWithArgs(Args &&...args)
	{
		void *result = PartitionBlock(sizeof(T), false, alignof(T));
		if (!result)
		{
			return nullptr;
		}
		return new (result) T(std::forward<Args>(args)...);
	}

	template <typename T, typename C>
	T *PartitionAllocator(u32 elements)
	{
		static_assert(std::is_base_of<IAllocator, T>::value, "Allocator must derive from IAllocator");
		size_t sz = sizeof(T) + elements * sizeof(C);
		void *result = PartitionBlock(sz, true, alignof(C));
		if (!result)
		{
			return nullptr;
		}
		return new (result) T(result, sz);
	}

	size_t MarkAllocation()
	{
		return activeSz;
	}

	void ResetToMark(size_t mark)
	{
		AGE_AssertCheck(mark <= activeSz, "MemoryManager: Cannot reset to a mark that is greater than the current allocation.");
		activeSz = mark;
	}

	size_t GetRemainingSize() { return maxSz - activeSz; }
	size_t GetUsedSize() { return activeSz; }
	size_t GetMaxSize() { return maxSz; }

	void Report();

	const size_t cacheLineSz;

private:
	MemoryManager(void *mem, size_t size, size_t cacheLineSz);
	~MemoryManager();

	void *Peek() { return (u8 *)block + activeSz; }
	void *block;
	size_t activeSz;
	size_t maxSz;
};

#endif
