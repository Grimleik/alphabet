#if !defined(ENTITY_H)
/* ========================================================================
   Creator: Grimleik $
   TODO: Create a bucket for component types so we can compress logic about them.
   ========================================================================*/
#define ENTITY_H
#include "core.h"
#include "cu_math.h"

#define NULL_ENTITY 0

// TODO(pf): Necessary ? Use as metadata for now.
// NOTE(pf): The problem with this is that we diverge from data driven now.
enum ENTITY_TYPES
{
    ET_PLAYER,
    ET_ASTEROID,
    ET_BULLET,
};

typedef struct transform_t transform_t;
struct transform_t
{
    vec2_t pos;
    vec2_t vel;
    vec2_t drag;
    vec2_t dir;
    // f32 prevX, prevY;
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

enum COLLISION_TYPE
{
    CT_RECTANGLE,
    CT_CIRCLE,
};

typedef struct collision_t collision_t;
struct collision_t
{
    i32 mask;
    i32 type;
    i32 width, height; // if circle, both are radius.
    bool colliding;
};

bool check_collision(collision_t *a, transform_t *posA, collision_t *b, transform_t *posB);

enum COMPONENT_ID
{
    COMPONENT_TRANSFORM,
    COMPONENT_HEALTH,
    COMPONENT_COLLISION,
    COMPONENT_ID_COUNT,
};

// STUDY: Can we create this automagically ?
static const size_t COMPONENT_SIZES[COMPONENT_ID_COUNT] = {
    sizeof(transform_t),
    sizeof(health_t),
    sizeof(collision_t),
};

enum ENTITY_COMPONENTS
{
    EC_TRANSFORM = 1 << COMPONENT_TRANSFORM,
    EC_HEALTH = 1 << COMPONENT_HEALTH,
    EC_COLLISION = 1 << COMPONENT_COLLISION,
};

typedef struct entity_t entity_t;
struct entity_t
{
    i32 id;
    i32 type;
    i32 componentMask;
    // bool active; // TODO: Necessary ?
    entity_t *next; // NOTE: For freelist.
};

enum ENTITY_EVENT_TYPE
{
    ENTITY_EVENT_COLLISION,
};

typedef struct entity_event entity_event;
struct entity_event
{
    i32 type;
};

typedef struct entity_event_collision_t entity_event_collision_t;
struct entity_event_collision_t
{
    i32 entityA;
    i32 entityB;
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
    entity_t *entitiesFreeList;
    i32 reservedEntityCount;
    i32 entityCount;

    component_array_t *componentArrays;
    i32 componentArrayCount;

    entity_event *events;
    i32 eventCount;
    i32 eventMaxCount;
};

void entity_manager_init(entity_manager_t *em, int reserved);
entity_t *entity_manager_create_entity(entity_manager_t *em);
entity_t *entity_manager_get_entity(entity_manager_t *em, i32 id);
void entity_manager_destroy_entity(entity_manager_t *em, entity_t *entity);
void entity_manager_shutdown(entity_manager_t *em);

// void entity_manager_add_event(entity_manager_t *em, entity_event event);
// void entity_manager_process_events(entity_manager_t *em);

// TODO: Add bucket logic for components here, i.e arrays, sizes, freelist, etc. add them inside c function ?
// TODO: Implement a way to retrieve with only id instead of entity ptrs ?
#define DECLARE_COMPONENT_FUNCTIONS(TYPE)                              \
    TYPE *entity_add_##TYPE(entity_manager_t *em, entity_t *entity);   \
    void entity_remove_##TYPE(entity_manager_t *em, entity_t *entity); \
    TYPE *entity_get_##TYPE(entity_manager_t *em, entity_t *entity);

#define DEFINE_COMPONENT_FUNCTIONS(TYPE, COMPONENT, COMPONENT_ID)                                          \
    TYPE *entity_add_##TYPE(entity_manager_t *em, entity_t *entity)                                        \
    {                                                                                                      \
        component_array_t *ca = &em->componentArrays[COMPONENT_ID];                                        \
        int index;                                                                                         \
        assert(!ca->lookUp[entity->id] && "Function add");                                                 \
        if (ca->freeListHead)                                                                              \
        {                                                                                                  \
            index = ca->freeList[ca->freeListHead--];                                                      \
            printf("Component " #TYPE " Reusing component index: %d for entity %d.\n", index, entity->id); \
        }                                                                                                  \
        else                                                                                               \
        {                                                                                                  \
            index = ca->count++;                                                                           \
        }                                                                                                  \
        entity->componentMask |= COMPONENT;                                                                \
        ca->lookUp[entity->id] = index;                                                                    \
        TYPE *rh = (TYPE *)ca->components + index;                                                         \
        *rh = (TYPE){0};                                                                                   \
        return (TYPE *)(ca->components) + index;                                                           \
    }                                                                                                      \
    void entity_remove_##TYPE(entity_manager_t *em, entity_t *entity)                                      \
    {                                                                                                      \
        component_array_t *ca = &em->componentArrays[COMPONENT_ID];                                        \
        int index = ca->lookUp[entity->id];                                                                \
        assert(index && "Function remove");                                                                \
        ca->lookUp[entity->id] = 0;                                                                        \
        entity->componentMask &= ~COMPONENT;                                                               \
        ca->freeList[++ca->freeListHead] = index;                                                          \
    }                                                                                                      \
    TYPE *entity_get_##TYPE(entity_manager_t *em, entity_t *entity)                                        \
    {                                                                                                      \
        component_array_t *ca = &em->componentArrays[COMPONENT_ID];                                        \
        int index = ca->lookUp[entity->id];                                                                \
        if (index)                                                                                         \
        {                                                                                                  \
            return (TYPE *)(ca->components) + index;                                                       \
        }                                                                                                  \
        return NULL;                                                                                       \
    }

DECLARE_COMPONENT_FUNCTIONS(transform_t)
DECLARE_COMPONENT_FUNCTIONS(health_t)
DECLARE_COMPONENT_FUNCTIONS(collision_t)
#endif
