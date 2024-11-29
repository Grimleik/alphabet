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
#include "cu_math.h"

bool check_collision(collision_t *a, transform_t *tA,
                     collision_t *b, transform_t *tB)
{
   if (a->type == CT_RECTANGLE && b->type == CT_RECTANGLE)
   {
      return abs(tA->pos.x - tB->pos.x) < (a->width + b->width) &&
             abs(tA->pos.y - tB->pos.y) < (a->height + b->height);
   }
   else if (a->type == CT_CIRCLE && b->type == CT_CIRCLE)
   {
      vec2_t diff = {tA->pos.x - tB->pos.x, tA->pos.y - tB->pos.y};
      return vec2_length(diff) < (a->width + b->width);
   }
   else if (a->type == CT_RECTANGLE && b->type == CT_CIRCLE)
   {
      f32 expRectHalfWidth = (a->width / 2) + b->width;
      f32 expRectHalfHeight = (a->height / 2) + b->height;
      f32 circleDistanceX = abs(tB->pos.x - tA->pos.x);
      f32 circleDistanceY = abs(tB->pos.y - tA->pos.y);
      if (circleDistanceX >= expRectHalfWidth ||
          circleDistanceY >= expRectHalfHeight)
      {
         return false;
      }

      if (circleDistanceX < (a->width / 2) ||
          circleDistanceY < (a->height / 2))
      {
         return true;
      }

      circleDistanceX -= a->width / 2;
      circleDistanceY -= a->height / 2;
      return (circleDistanceX * circleDistanceX + circleDistanceY * circleDistanceY) < (b->width * b->width);
   }
   else if (a->type == CT_CIRCLE && b->type == CT_RECTANGLE)
   {
      return check_collision(b, tB, a, tA);
   }
   assert(false && "Invalid collision type.");
   return false;
}

void entity_manager_init(entity_manager_t *em, int reserved)
{
   memset(em, 0, sizeof(entity_manager_t));
   em->reservedEntityCount = reserved;
   // 0 is "null".
   em->entityCount = 1;
   em->entitiesFreeList = NULL;

   em->entities = (entity_t *)malloc(sizeof(entity_t) * reserved);
   memset(em->entities, 0, sizeof(entity_t) * reserved);
   em->componentArrayCount = COMPONENT_ID_COUNT;
   em->componentArrays = (component_array_t *)malloc(sizeof(component_array_t) * em->componentArrayCount);

   for (int i = 0; i < em->componentArrayCount; ++i)
   {
      component_array_t *ca = &em->componentArrays[i];
      // NOTE: 0 is 'null'.
      ca->count = 1;
      // TODO: Decide on a size.
      ca->components = (void *)malloc(COMPONENT_SIZES[i] * reserved);

      // TODO: These needs to grow when entities grow.
      ca->lookUp = (int *)malloc(sizeof(int) * reserved);
      memset(ca->lookUp, 0, sizeof(int) * reserved);
      ca->freeList = (int *)malloc(sizeof(int) * reserved);
      memset(ca->freeList, 0, sizeof(int) * reserved);
      ca->freeListHead = 0;
   }

   em->eventMaxCount = 1024;
   em->events = (entity_event *)malloc(sizeof(entity_event) * em->eventMaxCount);
   em->eventCount = 0;
}

entity_t *entity_manager_create_entity(entity_manager_t *em)
{
   entity_t *result = em->entitiesFreeList;
   if (!result)
   {
      result = &em->entities[em->entityCount];
      result->id = em->entityCount;
      em->entityCount++;
      printf("Creating entity: %d\n", result->id);
   }
   else
   {
      printf("Reusing entity: %d\n", result->id);
      em->entitiesFreeList = em->entitiesFreeList->next;
   }
   return result;
}

entity_t *entity_manager_get_entity(entity_manager_t *em, i32 id)
{
   return &em->entities[id];
}

void entity_manager_destroy_entity(entity_manager_t *em, entity_t *entity)
{
   for (int i = 0; i < em->componentArrayCount; ++i)
   {
      component_array_t *ca = &em->componentArrays[i];
      if (entity->componentMask & (1 << i))
      {
         int index = ca->lookUp[entity->id];
         assert(index);
         ca->lookUp[entity->id] = 0;
         entity->componentMask &= ~(1 << i);
         ca->freeList[++ca->freeListHead] = index;
      }
   }
   printf("Destroying entity: %d\n", entity->id);
   entity->next = em->entitiesFreeList;
   em->entitiesFreeList = entity;
}

void entity_manager_shutdown(entity_manager_t *em)
{
   for (int i = 0; i < em->componentArrayCount; ++i)
   {
      component_array_t *ca = &em->componentArrays[i];
      free(ca->lookUp);
      free(ca->freeList);
      free(ca->components);
   }
   free(em->componentArrays);
   free(em->entities);
}

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

DEFINE_COMPONENT_FUNCTIONS(transform_t, EC_TRANSFORM, COMPONENT_TRANSFORM)
DEFINE_COMPONENT_FUNCTIONS(health_t, EC_HEALTH, COMPONENT_HEALTH)
DEFINE_COMPONENT_FUNCTIONS(collision_t, EC_COLLISION, COMPONENT_COLLISION)