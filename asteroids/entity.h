#if !defined(ENTITY_H)
/* ========================================================================
   Creator: Grimleik $
   TODO: Create a bucket for component types so we can compress logic about them.
   ========================================================================*/
#define ENTITY_H
#include "core.h"

#define NULL_ENTITY 0

// TODO(pf): Necessary ? Use as metadata for now.
// NOTE(pf): The problem with this is that we diverge from data driven now.
enum ENTITY_TYPES
{
    ET_PLAYER,
    ET_ASTEROID,
    ET_BULLET,
};

typedef struct position_t position_t;
struct position_t
{
    f32 x, y;
};

typedef struct velocity_t velocity_t;
struct velocity_t
{
    f32 x, y;
    f32 dragX, dragY;
};

typedef struct health_t health_t;
struct health_t
{
    i32 health;
};

enum COLLISION_MASK
{
    CM_PLAYER = 1 << ET_PLAYER,
    CM_ASTEROID = 1 << ET_ASTEROID,
    CM_BULLET = 1 << ET_BULLET,
};

typedef struct collision_t collision_t;
struct collision_t {
    i32 mask;
};

// TODO(pf): This is a bit of a mess, but it works for now.
enum COMPONENT_ID
{
    COMPONENT_POSITION,
    COMPONENT_VELOCITY,
    COMPONENT_HEALTH,
    COMPONENT_COLLISION,
    COMPONENT_ID_COUNT,
};

enum ENTITY_COMPONENTS
{
    EC_POSITION = 1 << COMPONENT_POSITION,
    EC_VELOCITY = 1 << COMPONENT_VELOCITY,
    EC_HEALTH = 1 << COMPONENT_HEALTH,
    EC_COLLISION = 1 << COMPONENT_COLLISION,
};

typedef struct entity_t entity_t;
struct entity_t
{
    i32 id;
    i32 type;
    i32 componentMask;
};

typedef struct component_array_t component_array_t;
struct component_array_t
{
    void *components;
    i32 count;
    i32 *lookUp;
    i32 *freeList;
    i32 freeListHead;
};

typedef struct entity_manager_t entity_manager_t;
struct entity_manager_t
{
    entity_t *entities;
    i32 reservedEntityCount;
    i32 entityCount;

    component_array_t *componentArrays;
    i32 componentArrayCount;
};

void entity_manager_init(entity_manager_t *em, int reserved);
entity_t *entity_manager_create_entity(entity_manager_t *em);
void entity_manager_shutdown(entity_manager_t *em);

// TODO: Add bucket logic for components here, i.e arrays, sizes, freelist, etc. add them inside c function ?
#define DECLARE_COMPONENT_FUNCTIONS(TYPE)                                 \
    TYPE *entity_add_##TYPE(entity_manager_t *em, entity_t *entity, TYPE component); \
    void entity_remove_##TYPE(entity_manager_t *em, entity_t *entity);               \
    TYPE *entity_get_##TYPE(entity_manager_t *em, entity_t *entity);

#define DEFINE_COMPONENT_FUNCTIONS(TYPE, COMPONENT, COMPONENT_ID)                   \
    TYPE *entity_add_##TYPE(entity_manager_t *em, entity_t *entity, TYPE component) \
    {                                                                               \
        component_array_t *ca = &em->componentArrays[COMPONENT_ID];                 \
        int index;                                                                  \
        assert(!ca->lookUp[entity->id]);                                            \
        if (ca->freeListHead)                                                       \
        {                                                                           \
            index = ca->freeList[ca->freeListHead--];                               \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            index = ca->count++;                                                    \
        }                                                                           \
        entity->componentMask |= COMPONENT;                                         \
        ca->lookUp[entity->id] = index;                                             \
        TYPE *rh = (TYPE *)ca->components + index;                                 \
        *rh = component;                                                            \
        return (TYPE *)(ca->components) + index;                                    \
    }                                                                               \
    void entity_remove_##TYPE(entity_manager_t *em, entity_t *entity)               \
    {                                                                               \
        component_array_t *ca = &em->componentArrays[COMPONENT_ID];                 \
        int index = ca->lookUp[entity->id];                                         \
        assert(index);                                                              \
        ca->lookUp[entity->id] = 0;                                                 \
        entity->componentMask &= ~COMPONENT;                                        \
        ca->freeList[++ca->freeListHead] = index;                                   \
    }                                                                               \
    TYPE *entity_get_##TYPE(entity_manager_t *em, entity_t *entity)                 \
    {                                                                               \
        component_array_t *ca = &em->componentArrays[COMPONENT_ID];                 \
        int index = ca->lookUp[entity->id];                                         \
        if (index)                                                                  \
        {                                                                           \
            return (TYPE *)(ca->components) + index;                                \
        }                                                                           \
        return NULL;                                                                \
    }

DECLARE_COMPONENT_FUNCTIONS(position_t)
DECLARE_COMPONENT_FUNCTIONS(velocity_t)
DECLARE_COMPONENT_FUNCTIONS(health_t)
DECLARE_COMPONENT_FUNCTIONS(collision_t)
#endif
