/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#include "memorymanager.h"

MemoryManager *MemoryManager::Instance = nullptr;

MemoryManager::MemoryManager(void *mem, size_t size)
	: block(mem), activeSz(sizeof(MemoryManager)), maxSz(size)
{
}

MemoryManager::~MemoryManager()
{
	Instance = nullptr;
	free(block);
	block = nullptr;
}

void MemoryManager::Create(void *memory, size_t size)
{
	Instance = new (memory) MemoryManager(memory, size);
}

void *MemoryManager::Partition(size_t szInBytes, bool zeroInit, size_t alignment)
{
	size_t align = Align(activeSz, alignment);
	szInBytes += align;
	if ((szInBytes + activeSz >= maxSz))
	{
		AGE_Assert("MemoryStack out of memory.");
		return nullptr;
	}
	void *result = static_cast<u8 *>(block) + activeSz + align;
	if (zeroInit)
		memset(result, 0, szInBytes);
	activeSz += szInBytes;
	return result;
}