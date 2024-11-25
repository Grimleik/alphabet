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

// TODO: Having to extend this explicitly when we add a component type is not very nice
size_t COMPONENT_SIZES[] = {
    sizeof(position_t),
    sizeof(velocity_t),
    sizeof(health_t),
};

void entity_manager_init(entity_manager_t *em, int reserved)
{
   memset(em, 0, sizeof(entity_manager_t));
   em->reservedEntityCount = reserved;
   // 0 is "null".
   em->entityCount = 1;

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
}

entity_t *entity_manager_create_entity(entity_manager_t *em)
{
   entity_t *result = &em->entities[em->entityCount];
   result->id = em->entityCount;
   em->entityCount++;
   return result;
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

DEFINE_COMPONENT_FUNCTIONS(position_t, EC_POSITION, COMPONENT_POSITION)
DEFINE_COMPONENT_FUNCTIONS(velocity_t, EC_VELOCITY, COMPONENT_VELOCITY)
DEFINE_COMPONENT_FUNCTIONS(health_t, EC_HEALTH, COMPONENT_HEALTH)
DEFINE_COMPONENT_FUNCTIONS(collision_t, EC_COLLISION, COMPONENT_COLLISION)