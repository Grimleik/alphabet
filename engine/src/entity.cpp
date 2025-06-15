/* ========================================================================
   Creator: Grimleik $
   TODO: Test growing the different component arrays.
	  * Have a bunch of asteroids spawn and move around, but have
		 some asteroids be 'indestructible' and some be 'destructible'.
		 This should test the health component.
   ========================================================================*/
#include "entity.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "memorymanager.h"
#include "cu_math.h"

constexpr size_t MAX_ARCHETYPES = 64;

ECS *ECS::Instance = nullptr;

void ECS::Create()
{
	AGE_AssertCheck(Instance == nullptr, "Trying to recreate a singleton instance. Not allowed.");
	Instance = MemoryManager::Instance->PartitionSystem<ECS>();
	// TODO: Check such that our CacheBlocks are aligned for the cache block. log alignment waste.
	// Instance->chunkPool = MemoryManager::Instance->PartitionAllocator<PoolAllocator>(MB(10), true, MemoryManager::cacheLineSz);
	Instance->chunkPool.Init(MemoryManager::Instance->PartitionBlock((size_t)MB(10), true, MemoryManager::Instance->cacheLineSz), MB(10));
	Instance->archetypeMap.LazyInit(MAX_ARCHETYPES);
}

// bool check_collision(collision_t *a, transform_t *tA,
// 					 collision_t *b, transform_t *tB)
// {
// 	if (a->type == CT_RECTANGLE && b->type == CT_RECTANGLE)
// 	{
// 		return fabs(tA->pos.x - tB->pos.x) < (a->width + b->width) &&
// 			   fabs(tA->pos.y - tB->pos.y) < (a->height + b->height);
// 	}
// 	else if (a->type == CT_CIRCLE && b->type == CT_CIRCLE)
// 	{
// 		vec2f diff = {tA->pos.x - tB->pos.x, tA->pos.y - tB->pos.y};
// 		return diff.length2() < SQUARE((a->width + b->width));
// 	}
// 	else if (a->type == CT_RECTANGLE && b->type == CT_CIRCLE)
// 	{
// 		f32 expRectHalfWidth = (f32)(a->width / 2) + b->width;
// 		f32 expRectHalfHeight = (f32)(a->height / 2) + b->height;
// 		f32 circleDistanceX = (f32)fabs(tB->pos.x - tA->pos.x);
// 		f32 circleDistanceY = (f32)fabs(tB->pos.y - tA->pos.y);
// 		if (circleDistanceX >= expRectHalfWidth ||
// 			circleDistanceY >= expRectHalfHeight)
// 		{
// 			return false;
// 		}

// 		if (circleDistanceX < (a->width / 2) ||
// 			circleDistanceY < (a->height / 2))
// 		{
// 			return true;
// 		}

// 		circleDistanceX -= a->width / 2;
// 		circleDistanceY -= a->height / 2;
// 		return (circleDistanceX * circleDistanceX + circleDistanceY * circleDistanceY) < (b->width * b->width);
// 	}
// 	else if (a->type == CT_CIRCLE && b->type == CT_RECTANGLE)
// 	{
// 		return check_collision(b, tB, a, tA);
// 	}
// 	assert(false && "Invalid collision type.");
// 	return false;
// }

// void EntityManager::Init(u32 reserved)
// {
// 	reservedEntityCount = reserved;
// 	// 0 is "null".
// 	entityCount = 1;
// 	entitiesFreeList = NULL;

// 	//    entities = (entity_t *)malloc(sizeof(entity_t) * reserved);
// 	entities = (entity_t *)MemoryManager::Instance->Partition(sizeof(entities[0]) * reserved, true, alignof(entity_t));
// 	transforms = MemoryManager::Instance->Partition<ComponentContainer<transform_t>>(sizeof(ComponentContainer<transform_t>) * reserved, true, alignof(ComponentContainer<transform_t>));

// 	// em->eventMaxCount = 1024;
// 	// em->events = (entity_event *)malloc(sizeof(entity_event) * em->eventMaxCount);
// 	// em->eventCount = 0;
// }

// transform_t *EntityManager::AddTransform(entity_t *entity)
// {
// 	transform_t *result = transforms->AddComponent(entity);
// 	return result;
// }

// entity_t *entity_manager_create_entity(entity_manager_t *em)
// {
// 	entity_t *result = entities;
// 	if (!result)
// 	{
// 		result = &em->entities[em->entityCount];
// 		result->id = em->entityCount;
// 		em->entityCount++;
// 		printf("Creating entity: %d\n", result->id);
// 	}
// 	else
// 	{
// 		printf("Reusing entity: %d\n", result->id);
// 		em->entitiesFreeList = em->entitiesFreeList->next;
// 	}
// 	return result;
// }

// entity_t *entity_manager_get_entity(entity_manager_t *em, i32 id)
// {
// 	return &em->entities[id];
// }

// void entity_manager_destroy_entity(entity_manager_t *em, entity_t *entity)
// {
// 	for (int i = 0; i < em->componentArrayCount; ++i)
// 	{
// 		component_array_t *ca = &em->componentArrays[i];
// 		if (entity->componentMask & (1 << i))
// 		{
// 			int index = ca->lookUp[entity->id];
// 			assert(index);
// 			ca->lookUp[entity->id] = 0;
// 			entity->componentMask &= ~(1 << i);
// 			ca->freeList[++ca->freeListHead] = index;
// 		}
// 	}
// 	printf("Destroying entity: %d\n", entity->id);
// 	entity->next = em->entitiesFreeList;
// 	em->entitiesFreeList = entity;
// }

// void entity_manager_shutdown(entity_manager_t *em)
// {
// 	for (int i = 0; i < em->componentArrayCount; ++i)
// 	{
// 		component_array_t *ca = &em->componentArrays[i];
// 		free(ca->lookUp);
// 		free(ca->freeList);
// 		free(ca->components);
// 	}
// 	free(em->componentArrays);
// 	free(em->entities);
// }

// void entity_manager_add_event(entity_manager_t *em, entity_event event)
// {
//    assert(em->eventCount < em->eventMaxCount);
//    em->events[em->eventCount++] = event;
// }

// void entity_manager_process_events(entity_manager_t *em)
// {
//    for (int i = 0; i < em->eventCount; ++i)
//    {
//       entity_event *event = &em->events[i];
//       switch (event->type)
//       {
//       case ENTITY_EVENT_COLLISION:
//       {
//          entity_event_collision_t *collisionEvent = (entity_event_collision_t *)event;
//          entity_t *entityA = &em->entities[collisionEvent->entityA];
//          entity_t *entityB = &em->entities[collisionEvent->entityB];
//          if (entityA->componentMask & EC_HEALTH)
//          {
//             health_t *health = entity_get_health_t(em, entityA);
//             health->health -= 1;
//             if (health->health <= 0)
//             {
//                // TODO: Handle removal of health component.
//                entityA->componentMask = 0;
//             }
//          }
//          if (entityB->componentMask & EC_HEALTH)
//          {

//             health_t *health = entity_get_health_t(em, entityB);
//             health->health -= 1;
//             if (health->health <= 0)
//             {
//                // TODO: Handle removal of health component.
//                entityB->componentMask = 0;
//             }
//          }
//       }
//       break;
//       }
//    }
//    em->eventCount = 0;
// }

// DEFINE_COMPONENT_FUNCTIONS(transform_t, EC_TRANSFORM, COMPONENT_TRANSFORM)
// DEFINE_COMPONENT_FUNCTIONS(health_t, EC_HEALTH, COMPONENT_HEALTH)
// DEFINE_COMPONENT_FUNCTIONS(collision_t, EC_COLLISION, COMPONENT_COLLISION)

// entity_t *EntityManager::CreateEntity(entity_manager_t *em)
// {
// 	return nullptr;
// }
