#if !defined(MEMORYPOOL_H)
/* ========================================================================
   Creator: Grimleik $
   Can also be called a 'Linear Allocator'
   ========================================================================*/
#define MEMORYPOOL_H
#include "core.h"
#include "memorymanager.h"

class LinearAllocator : public IAllocator
{
public:
	LinearAllocator(void *block, size_t sz) : block(block), maxSz(sz), activeSz(0)
	{
	}
	~LinearAllocator() = default;

	void *Allocate(size_t szInBytes, size_t alignment = 0)
	{
		szInBytes += Align(activeSz, alignment);
		if ((szInBytes + activeSz >= maxSz))
		{
			AGE_Assert("MemoryStack out of memory.");
			return nullptr;
		}
		void *result = static_cast<u8 *>(block) + activeSz;
		activeSz += szInBytes;
		return result;
	}

	template <typename T>
	T *Allocate(size_t elemCount, size_t alignment = 0)
	{
		return (T *)Allocate(sizeof(T) * elemCount, alignment);
	}

	void Reset()
	{
		activeSz = 0;
	}

	size_t GetUsedSize() { return activeSz; }
	size_t GetRemainingSize() { return maxSz - activeSz; }

private:
	void *block;
	size_t maxSz;
	size_t activeSz;
};
#endif
