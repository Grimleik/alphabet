#if !defined(MEMORYMANAGER_H)
/* ========================================================================
   Creator: Grimleik $

   NOTE: Currently the MemoryManager can only Partition out memory, there is no
   reclaiming.
   ========================================================================*/
#define MEMORYMANAGER_H

#include "core.h"
#include "singleton.h"

// TODO(pf):
// * Pass a reference to the MemoryManager and return the allocated memory to
//   the memory manager in a virtual destructor. This requires a pass on the
//   memory manager to handle allocations of any size and defragmentation.
class IAllocator
{
public:
	IAllocator(void *block, size_t sizeInBytes);
	virtual ~IAllocator() = default;
	virtual void Reset() = 0;
	virtual size_t GetUsedSize() = 0;
	virtual size_t GetRemainingSize() = 0;
	// TODO:
	// virtual void *Allocate() = 0;
	// virtual void Free(void *mem);
};

class MemoryManager : public ISingleton
{
public:
	
	static void Create(void* mem, size_t size);
	static MemoryManager *Instance;

	void *Partition(size_t szInBytes, bool zeroInit = true, size_t alignment = 1);

	template <typename T, typename... Args>
	T *Partition(Args &&...args)
	{
		void *result = Partition(sizeof(T));
		if (!result)
		{
			return nullptr;
		}
		return new (result) T(std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	std::unique_ptr<T> PartitionAllocator(size_t szInBytes, size_t alignment = 1, Args &&...args)
	{
		static_assert(std::is_base_of<IAllocator, T>::value, "Allocator must derive from IAllocator");
		void *result = Partition(szInBytes, alignment);
		if (!result)
		{
			return nullptr;
		}
		return std::make_unique<T>(result, szInBytes, std::forward<Args>(args)...);
	}

	size_t GetRemainingSize() { return maxSz - activeSz; }
	size_t GetUsedSize() { return activeSz; }

private:
	MemoryManager(void* mem, size_t size);
	~MemoryManager();

	void *block;
	size_t activeSz;
	size_t maxSz;
};

#endif
