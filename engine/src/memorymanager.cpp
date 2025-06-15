/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#include "memorymanager.h"

MemoryManager *MemoryManager::Instance = nullptr;


MemoryManager::MemoryManager(void *mem, size_t size, size_t cacheLineSz_)
	: block(mem), activeSz(sizeof(MemoryManager)), maxSz(size), cacheLineSz(cacheLineSz_)
{
}

MemoryManager::~MemoryManager()
{
	Instance = nullptr;
	free(block);
	block = nullptr;
}

void MemoryManager::Create(void *memory, size_t size, size_t cacheLineSz)
{
	Instance = new (memory) MemoryManager(memory, size, cacheLineSz);
}

void *MemoryManager::PartitionBlock(size_t szInBytes, bool zeroInit, size_t alignment)
{
	size_t align = Align(activeSz, alignment);
	szInBytes += align;
	if ((szInBytes + activeSz >= maxSz))
	{
		szInBytes = 0;
		AGE_Assert("MemoryStack out of memory.");
		return nullptr;
	}
	void *result = static_cast<u8 *>(block) + activeSz + align;
	if (zeroInit)
		memset(result, 0, szInBytes);
	activeSz += szInBytes;
	return result;
}

void MemoryManager::Report()
{
	AGE_LOG(LOG_LEVEL::DEBUG, "Memory allocation report: \n\tUsed size: {} \n\tRemaining Size: {} \n\tMax size: {} \n\tMemory usage: {} ",
			MemoryManager::Instance->GetUsedSize(),
			MemoryManager::Instance->GetRemainingSize(),
			MemoryManager::Instance->GetMaxSize(),
			(float)MemoryManager::Instance->GetUsedSize() / (float)MemoryManager::Instance->GetMaxSize() * 100.0f);
}
