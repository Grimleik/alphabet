#if !defined(CONTAINER_H)
/*========================================================================
	Creator: Grimleik $
========================================================================*/
#define CONTAINER_H
#include "memorymanager.h"
#include "logger.h"

template <typename T>
class StaticArray
{
public:
	StaticArray()
	{
	}

	StaticArray(size_t size_)
		: size(size_),
		  data((T *)MemoryManager::Instance->PartitionBlock(size_ * sizeof(T), true, alignof(T)))
	{
	}

	~StaticArray() {}

	void LazyInit(size_t size_)
	{
		AGE_AssertCheck(size_ == 0, "StaticArray already initialized.");
		size = size_;
		data = (T *)MemoryManager::Instance->PartitionBlock(size_ * sizeof(T), true, alignof(T));
	}

	T &operator[](size_t index)
	{
		return data[index];
	}

	const T &operator[](size_t index) const
	{
		return data[index];
	}

private:
	T *data;
	size_t size;
};

template <typename T>
struct HashMapKeyPolicy
{
	static size_t index(const T &key, size_t size)
	{
		return std::hash<T>{}(key) % size;
	}
};

template <>
struct HashMapKeyPolicy<int>
{
	static size_t index(const int &key, size_t size)
	{
		return (key % size + size) % size; // Ensure positive index
	}
};

template <>
struct HashMapKeyPolicy<u32>
{
	static size_t index(const int &key, size_t size)
	{
		return (key % size + size) % size; // Ensure positive index
	}
};

template <>
struct HashMapKeyPolicy<u64>
{
	static size_t index(const int &key, size_t size)
	{
		return (key % size + size) % size; // Ensure positive index
	}
};

// NOTE: This hashmap eploys policies, see above, to determine the index of a key.
// It uses linear probing for collision resolution.
// And it allocates an extra slot for a guard element at index 0.
template <typename K, typename V>
class HashMap
{
public:
	HashMap()
	{
	}

	HashMap(size_t size_)
		: size(size_),
		  data((std::pair<K, V> *)MemoryManager::Instance->PartitionBlock((1 + size_) * sizeof(std::pair<K, V>), true, alignof(std::pair<K, V>))),
		  occupied((bool *)MemoryManager::Instance->PartitionBlock(size_ * sizeof(bool), true, alignof(bool)))
	{
	}

	~HashMap() {}

	void LazyInit(size_t size_)
	{
		AGE_AssertCheck(size == 0, "HashMap already initialized.");
		size = size_;
		data = (std::pair<K, V> *)MemoryManager::Instance->PartitionBlock((1 + size_) * sizeof(std::pair<K, V>), true, alignof(std::pair<K, V>));
		occupied = (bool *)MemoryManager::Instance->PartitionBlock(size_ * sizeof(bool), true, alignof(bool));
	}

	V &operator[](const K &key)
	{
		return at_index(HashMapKeyPolicy<K>::index(key, size), key);
	}

	bool Contains(const K &key)
	{
		AGE_AssertCheck(size > 0, "HashMap not initialized.");
		size_t index = HashMapKeyPolicy<K>::index(key, size);

		for (size_t i = 0; i < size; ++i)
		{
			size_t probeIndex = ((index + i) % (size - 1)) + 1;
			if (data[probeIndex].first == key)
			{
				return true;
			}
			if (!occupied[probeIndex])
			{
				return false;
			}
		}
		return false;
	}

	void Remove(const K &key)
	{
		AGE_AssertCheck(size > 0, "HashMap not initialized.");
		size_t index = std::hash<K>{}(key) % size;

		for (size_t i = 0; i < size; ++i)
		{
			size_t probeIndex = ((index + i) % (size - 1)) + 1;
			if (data[probeIndex].first == key)
			{
				data[probeIndex].first = K{};
				data[probeIndex].second = V{};
				occupied[probeIndex] = false;
				return;
			}
			if (!occupied[probeIndex])
			{
				AGE_LOG(LOG_LEVEL::ERR, "Key not found in HashMap.");
				return;
			}
		}
	}

private:
	V &at_index(size_t index, const K &key)
	{
		AGE_AssertCheck(size > 0, "HashMap not initialized.");
		size_t probeIndex = ((index % (size - 1)) + 1);
		if (data[probeIndex].first == key)
		{
			occupied[probeIndex] = true;
			return data[probeIndex].second;
		}
		if (!occupied[probeIndex])
		{
			occupied[probeIndex] = true;
			data[probeIndex].first = key;
			data[probeIndex].second = V{};
			return data[probeIndex].second;
		}
		return data[0].second; // Return guard if full.
	}

	std::pair<K, V> *data;
	bool *occupied;
	size_t size;
};

constexpr size_t AGE_STR_MAX_LEN = 32;
// NOTE: Strings are generally inefficient. This is a simple static
// non-allocating string implementation.
class AGEString
{
public:
	AGEString()
	{
		data[0] = '\0';
		size = 0;
	}
	AGEString(const char *str)
	{
		AGE_AssertCheck(strlen(str) < AGE_STR_MAX_LEN, "String is too long.");
		strcpy_s(data, str);
		data[strlen(str)] = '\0';
	}

	~AGEString()
	{
	}

	const char *c_str() const
	{
		return data;
	}

	const char *operator=(const char *str)
	{
		AGE_AssertCheck(strlen(str) < AGE_STR_MAX_LEN, "String is too long.");
		strcpy_s(data, str);
		data[strlen(str)] = '\0';
		return data;
	}

private:
	char data[AGE_STR_MAX_LEN];
	size_t size;
};

template <typename T>
class SLink
{
public:
	SLink()
	{
		next = nullptr;
	}

	SLink(std::unique_ptr<T> next_)
	{
		next = std::move(next_);
	}

	std::unique_ptr<T> value;
	SLink<T> *next;
};

#endif
