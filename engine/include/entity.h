#if !defined(ENTITY_H)
/*========================================================================
	Creator: Grimleik $
	TODO: rename to ecs.
========================================================================*/
#define ENTITY_H
#include "core.h"
#include "cu_math.h"
#include "containers.h"
#include "poolallocator.h"
#include "logger.h"

using EntityID = u32;
constexpr size_t MAX_ENTITIES = 100'000;

struct DataBlockPartition
{
	DataBlockPartition *next;
	DataBlockPartition *prev;
	void *data;
};

// Basic Components. TODO: Let user define their own.

struct Transform
{
	vec2f pos;
};

// TODO: Move from transform_t, rename transform_t to Transform.
struct Physics
{
	vec2f vel;
	vec2f drag;
	vec2f dir;
	f32 speed;
	f32 acc;
};

enum COLLISION_TYPE
{
	CT_RECTANGLE,
	CT_CIRCLE,
};

struct Collision
{
	i32 mask;
	i32 type;
	i32 width, height; // if circle, both are radius.
	bool colliding;
};

// TODO: This is game specific, MOVE.
struct Health
{
	i32 health;
};

bool check_collision(Collision *a, Transform *posA, Collision *b, Transform *posB);

using ComponentMask = u64;

struct IArchetype
{
};

template <typename... Components>
struct ArchetypeChunk
{
	// static constexpr size_t COMPONENT_SIZE = (sizeof(Components) + ...);
	// static constexpr size_t MAX_ENTITIES_PER_CHUNK = CHUNK_SIZE_BYTES / COMPONENT_SIZE;
	// static constexpr size_t WASTE = CHUNK_SIZE_BYTES % COMPONENT_SIZE;
	// static_assert(MAX_ENTITIES_PER_CHUNK > 0, "Chunk size is too small for the components.");

	ArchetypeChunk(void *memory, size_t memorySz)
	{
		static_assert(sizeof(State) == sizeof(state), "State size is not equal to u64.");
		size_t componentSize = (sizeof(Components) + ...);
		size_t nrOfComponents = memorySz / componentSize;
		size_t waste = memorySz % componentSize;
		SetCount(0);
		SetComponentCount(nrOfComponents);
		SetArchetypSize(componentSize);
		SetWaste(waste);
		AGE_AssertCheck(nrOfComponents > 0, "Chunk size is too small for the components.");
		AGE_LOG(LOG_LEVEL::DEBUG, "Chunk size: {}. Number of components: {}. Component size: {}. Waster: {}",
				memorySz, nrOfComponents, componentSize, waste);
		([&]()
		 {
			std::get<Components*>(componentArrays) = (Components*)memory;
			memory = (u8*)memory + (sizeof(Components) * nrOfComponents); }(), ...);
	}

	std::tuple<Components *...> componentArrays;
	inline bool IsFull() const { return Count() >= ComponentCount(); }

	inline void SetCount(u8 count_) 	{ state = (state & ~(0xFF << State::COUNT)) | (count_ << State::COUNT); }
	inline u16 Count() const 			{ return (state >> State::COUNT) & ((1 << (State::NR_OF_COMPONENTS - State::COUNT)) - 1); }
	inline u8 ComponentCount() const	{ return (state >> State::NR_OF_COMPONENTS) & ((1 << (State::WASTE - State::NR_OF_COMPONENTS)) - 1); }
	inline u8 WasteInfo() const 		{ return (state >> State::WASTE) & ((1 << (State::ARCHETYPE_SIZE - State::WASTE)) - 1); }
	inline u8 ArchetypeSize() const 	{ return (state >> State::ARCHETYPE_SIZE) & ((1 << (State::END - State::ARCHETYPE_SIZE)) - 1); }

	// inline u8 ComponentCount() const 	{ return (state >> State::NR_OF_COMPONENTS & ((1 << STATE_SEGMENT_SIZE) - 1)); }
	// inline u8 Waste() const 			{ return (state >> State::WASTE & ((1 << STATE_SEGMENT_SIZE) - 1)); }
	// inline u8 ArchetypeSize() const 	{ return (state >> State::ARCHETYPE_SIZE & ((1 << STATE_SEGMENT_SIZE) - 1)); }

private:
	enum State : u64
	{
		COUNT = 8 * 0,
		// COUNT = 8 * 1,
		NR_OF_COMPONENTS = 8 * 2,
		WASTE = 8 * 3,
		ARCHETYPE_SIZE = 8 * 4,
		END = 8 * 5,
		// MAX = 8 * 7,
	};

	inline void SetComponentCount(u8 count_) { state = (state & ~(0xFF << State::NR_OF_COMPONENTS)) | (count_ << State::NR_OF_COMPONENTS); }
	inline void SetWaste(u8 waste_) { state = (state & ~(0xFF << State::WASTE)) | (waste_ << State::WASTE); }
	inline void SetArchetypSize(u8 size_) { state = (state & ~(0xFF << State::ARCHETYPE_SIZE)) | (size_ << State::ARCHETYPE_SIZE); }
	u64 state;
};

template <typename... Components>
class Archetype : public IArchetype
{
public:
	using Chunk = ArchetypeChunk<Components...>;
	DataBlockPartition *chunks;

	Archetype();

	ComponentMask GetMask() { return mask; }

	EntityID AddEntity(Components &&...comps);
	void DestroyEntity(EntityID id);

	template <typename Func>
	void ForEach(Func &&func);

private:
	struct ChunkEntry
	{
		Chunk *chunk;
		size_t index;
	};

	// Template helper function to assign components to the chunk.
	// void AssignComponentsToChunkHelper(Chunk &chunk, size_t index, Components &&...comps);
	template <size_t... Is>
	void AssignComponentsToChunkHelper(Chunk &chunk, size_t index, std::tuple<Components &&...> &&values, std::index_sequence<Is...>);

	void AssignComponentsToChunk(Chunk &chunk, size_t index, Components &&...comps);

	// std::unordered_map<EntityID, ChunkEntry> entityToChunk;
	HashMap<EntityID, ChunkEntry> entityToChunk;
	EntityID nextEntityID = 1;
	EntityID *nextFreeID = nullptr;
	ComponentMask mask;
};

class ECS : public ISingleton
{
public:
	static void Create();
	static ECS *Instance;

	template <typename T>
	inline ComponentMask GetComponentID()
	{
		static ComponentMask id = nextComponentID++;
		return id;
	}

	template <typename... Components>
	inline ComponentMask CreateMask() { return ((ComponentMask(1) << GetComponentID<Components>()) | ...); }

	template <typename... Components>
	EntityID CreateEntity(Components &&...comps);

	template <typename... Components>
	void DestroyEntity(EntityID id);

	template <typename Func, typename... Components>
	void ForEachArchetype(Func &&func);

private:
	HashMap<ComponentMask, IArchetype *> archetypeMap;
	PoolAllocator<DataBlockPartition> chunkPool;
	static inline ComponentMask nextComponentID = 0;
	HashMap<EntityID, bool> entityIDTaken;

	EntityID nextEntityID = 1;
	EntityID *nextFreeID = nullptr;

	template <typename... Components>
	friend class Archetype;
};

// ============================================================
// ========== Implementation of Archetype methods =============
// ============================================================
template <typename... Components>
Archetype<Components...>::Archetype()
{
	mask = ECS::Instance->CreateMask<Components...>();
	chunks = nullptr;
}

template <typename... Components>
EntityID Archetype<Components...>::AddEntity(Components &&...comps)
{
	if (!chunks)
	{
		chunks = ECS::Instance->chunkPool.AllocateAsZero();
	}

	Chunk *chunk = (Chunk *)chunks->data;
	if (chunk->IsFull())
	{
		chunks->next = ECS::Instance->chunkPool.AllocateAsZero();
		chunks->next->prev = chunks;
		chunks = chunks->next;
	}
	chunk = (Chunk *)chunks->data;

	size_t index = chunk->Count();
	EntityID newID = nextEntityID++;
	AssignComponentsToChunk(*chunk, index, std::forward<Components>(comps)...);
	// entityToChunk[newID] = {chunk, index};
	AGE_AssertCheck(index < (u16)(index + 1), "Chunk count wrap. This should not happen.");
	chunk->SetCount((u8)(index + 1));

	AGE_LOG(LOG_LEVEL::DEBUG, "Allocating {} bytes. {} / {} left.", chunk->ArchetypeSize(),
			chunk->Count(), chunk->ComponentCount());
	AGE_LOG(LOG_LEVEL::DEBUG, "Chunk: {}. Index: {}. EntityID: {}", (void *)chunk, index, nextEntityID);

	return newID;
}

template <typename... Components>
void Archetype<Components...>::DestroyEntity(EntityID id)
{
	if (entityToChunk.Contains(id))
	{
		auto &entry = entityToChunk[id];
		auto *chunk = entry.chunk;
		size_t index = entry.index;

		// Move the last element in the chunk to the index of the destroyed entity.
		if (index < chunk->count - 1)
		{
			AssignComponentsToChunk(*chunk, index, std::move(chunk->componentArrays[chunk->count - 1]));
		}
		entityToChunk[chunk->count - 1] = index;
		entityToChunk.Remove(index);
		chunk->count--;

		if (chunk->isFull())
		{
			chunks = chunks->next;
			ECS::Instance->chunkPool->Free(chunks);
		}
	}
	else
	{
		AGE_LOG(LOG_LEVEL::ERR, "Trying to destroy an entity that does not exist.");
	}
}

template <typename... Components>
template <typename Func>
void Archetype<Components...>::ForEach(Func &&func)
{
	for (auto &chunk : chunks)
	{
		for (size_t i = 0; i < chunk->count; ++i)
		{
			func((std::get<std::array<Components, Chunk::MAX_ENTITIES_PER_CHUNK>>(chunk->componentsArray)[i])...);
		}
	}
}

template <typename... Components>
template <size_t... Is>
void Archetype<Components...>::AssignComponentsToChunkHelper(Chunk &chunk, size_t index, std::tuple<Components &&...> &&values, std::index_sequence<Is...>)
{
	// Unpack the tuple and assign each component to the chunk's component array.
	// This is a helper function to avoid code duplication.
	((std::get<Is>(chunk.componentArrays)[index] = std::get<Is>(std::move(values))), ...);
}

template <typename... Components>
void Archetype<Components...>::AssignComponentsToChunk(Chunk &chunk, size_t index, Components &&...comps)
{
	auto values = std::forward_as_tuple(std::forward<Components>(comps)...);
	// Use std::index_sequence to unpack the tuple and assign components to the chunk.
	AssignComponentsToChunkHelper(chunk, index, std::move(values), std::index_sequence_for<Components...>{});
}

// ============================================================
// ========== Implementation of ECS methods ===================
// ============================================================

template <typename... Components>
EntityID ECS::CreateEntity(Components &&...comps)
{
	ComponentMask mask = CreateMask<Components...>();
	if (!archetypeMap.Contains(mask))
	{
		archetypeMap[mask] = MemoryManager::Instance->PartitionWithArgs<Archetype<Components...>>();
	}

	auto *archetype = static_cast<Archetype<Components...> *>(archetypeMap[mask]);
	return archetype->AddEntity(std::forward<Components>(comps)...);
}

template <typename... Components>
void ECS::DestroyEntity(EntityID id)
{
	ComponentMask mask = CreateMask<Components...>();
	if (!archetypeMap.Contains(mask))
	{
		AGE_LOG(LOG_LEVEL::ERR, "Entity {} not found in archetype map.", id);
		return;
	}
	auto *archetype = static_cast<Archetype<Components...> *>(archetypeMap[mask]);
	archetype->DestroyEntity(id);
}

template <typename Func, typename... Components>
void ECS::ForEachArchetype(Func &&func)
{
	ComponentMask mask = CreateMask<Components>();
	if (!archetypeMap.Contains(mask))
	{
		return;
	}
	archetypeMap[mask]->ForEach(std::forward<Func>(func));
}

#endif