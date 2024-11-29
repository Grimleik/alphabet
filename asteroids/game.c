/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#include "game.h"
#include "cu_math.h"
#include "entity.h"

#define ASTEROID_COUNT 4
#define ASTEROID_SPAWN_DELAY 3.0f

// TODO: Data driven instead of hardcoded.
#define PLAYER_WIDTH 40
#define PLAYER_HEIGHT 40
#define ASTEROID_WIDTH 10
#define ASTEROID_HEIGHT 10
#define BULLET_WIDTH 5
#define BULLET_HEIGHT 5
#define BULLET_SPEED 10

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0x00000000
#define COLOR_RED 0xFF0000
#define COLOR_GREEN 0x00FF00
#define COLOR_BLUE 0x0000FF
#define COLOR_YELLOW 0xFFFF00
#define COLOR_BROWN 0xA52A2A
#define COLOR_ORANGE 0xFFA500

#define PLAYER_COLOR COLOR_WHITE
#define ASTEROID_COLOR COLOR_WHITE
#define COLLISION_OUTLINE_COLOR COLOR_GREEN
#define COLLIDED_COLOR COLOR_RED

enum DRAW_COMMAND_TYPE
{
    DCT_CLEAR_SCREEN,
    DCT_RECTANGLE,
    DCT_CIRCLE,
    DCT_LINE,
    // DCT_BMP,
};

// TODO: Write commands for each type and cast them when used.
typedef struct draw_command_t draw_command_t;
struct draw_command_t
{
    i32 type;
    i32 x, y;
    i32 width, height; // repurposed as x2, y2 for lines.
    u32 color;
    bool filled;
    entity_t *dbg_entity;
};

#define MAX_DRAW_COMMANDS 100

typedef struct game_state_t game_state_t;
struct game_state_t
{
    entity_manager_t entityManager;
    entity_t *player;
    i32 asteroidCount;
    f32 spawnDelay;

    draw_command_t drawCommands[MAX_DRAW_COMMANDS];
    i32 drawCommandCount;

    bool paused;
};

game_state_t *get_game_state(platform_state_t *state)
{
    return (game_state_t *)state->memory;
}

void push_draw_command(draw_command_t *drawCommands, i32 *drawCommandCount, draw_command_t drawCommand)
{
    drawCommands[*drawCommandCount] = drawCommand;
    (*drawCommandCount)++;
}

void set_pixel(render_state_t *render, int x, int y, int color)
{
    if (x < 0 || x >= render->width || y < 0 || y >= render->height)
    {
        return;
    }

    int *pixel = (int *)((u8 *)render->pixels + x * render->bytesPerPixel +
                         y * render->pitch);
    *pixel = color;
}

void spawn_player(game_state_t *gameState, i32 x, i32 y)
{
    gameState->player = entity_manager_create_entity(&gameState->entityManager);
    gameState->player->type = ET_PLAYER;
    transform_t *transform = entity_add_transform_t(&gameState->entityManager, gameState->player);
    transform->pos = (vec2_t){x, y};
    transform->dir = (vec2_t){0, 0};
    collision_t *col = entity_add_collision_t(&gameState->entityManager, gameState->player);
    col->type = CT_RECTANGLE;
    col->width = PLAYER_WIDTH;
    col->height = PLAYER_HEIGHT;
    // health_t *health = entity_add_health_t(&gameState->entityManager, gameState->player, (health_t){3});
}

void spawn_asteroid(game_state_t *gameState)
{
    if (gameState->asteroidCount >= ASTEROID_COUNT ||
        gameState->spawnDelay > 0)
    {
        return;
    }
    entity_t *asteroid = entity_manager_create_entity(&gameState->entityManager);
    asteroid->type = ET_ASTEROID;
    transform_t *transform = entity_add_transform_t(&gameState->entityManager, asteroid);
    transform->pos = (vec2_t){rand() % 800, rand() % 600};
    transform->dir = (vec2_t){rand() % 5, rand() % 5};
    transform->vel = transform->dir;
    collision_t *col = entity_add_collision_t(&gameState->entityManager, asteroid);
    col->type = CT_RECTANGLE;
    col->width = ASTEROID_WIDTH;
    col->height = ASTEROID_HEIGHT;
    col->mask = CM_PLAYER | CM_BULLET;
    health_t *health = entity_add_health_t(&gameState->entityManager, asteroid);
    health->health = 1;
    gameState->asteroidCount++;
    gameState->spawnDelay = ASTEROID_SPAWN_DELAY;
}

void spawn_bullet(game_state_t *gameState)
{
    entity_t *bullet = entity_manager_create_entity(&gameState->entityManager);
    bullet->type = ET_BULLET;
    transform_t *transform = entity_add_transform_t(&gameState->entityManager, bullet);
    transform_t *playerTransform = entity_get_transform_t(&gameState->entityManager, gameState->player);
    transform->pos = (vec2_t){playerTransform->pos.x, playerTransform->pos.y};
    transform->dir = (vec2_t){playerTransform->dir.x, playerTransform->dir.y};
    transform->vel = vec2_mul_s(transform->dir, BULLET_SPEED);
    collision_t *col = entity_add_collision_t(&gameState->entityManager, bullet);
    col->type = CT_RECTANGLE;
    col->mask = CM_ASTEROID;
    col->width = BULLET_WIDTH * 10;
    col->height = BULLET_HEIGHT * 10;
    health_t *health = entity_add_health_t(&gameState->entityManager, bullet);
    health->health = 1;
}

void game_state_shutdown(game_state_t *gameState)
{
    entity_manager_shutdown(&gameState->entityManager);
}

void game_init(platform_state_t *state)
{
    // TODO: This is NOT platform specific.
    state->isRunning = true;
    state->input = (platform_input_t *)malloc(sizeof(platform_input_t));
    state->render = (render_state_t *)malloc(sizeof(render_state_t));

    state->render->width = 800;
    state->render->height = 600;
    state->render->bytesPerPixel = 4;
    state->render->pitch = state->render->width * state->render->bytesPerPixel;
    state->render->pixels = malloc(state->render->width * state->render->height *
                                   state->render->bytesPerPixel);
    memset(state->render->pixels, 0, state->render->width * state->render->height * state->render->bytesPerPixel);

    memset(state->input, 0, sizeof(platform_input_t));
    state->dt = 0.0f;

    state->memorySize = GB(1);
    state->memory = malloc(state->memorySize);
    game_state_t *gameState = get_game_state(state);

    entity_manager_init(&gameState->entityManager, 10);
    spawn_player(gameState, state->render->width / 2, state->render->height / 2);

    gameState->asteroidCount = 0;
}

static void entity_modify_health(game_state_t *gameState, entity_t* entity, health_t *health, i32 amount)
{
    health->health += amount;
    if (health->health <= 0)
    {
        entity_manager_destroy_entity(&gameState->entityManager, entity);
        if (entity->type == ET_ASTEROID)
        {
            gameState->asteroidCount--;
        }
    }
}

void game_logic(platform_state_t *state)
{
    int i;

    if (input_is_key_pressed(state, KEY_ESCAPE))
    {
        state->isRunning = false;
    }

    game_state_t *gameState = get_game_state(state);

    if (input_is_key_pressed(state, KEY_P))
    {
        gameState->paused = !gameState->paused;
    }

    if (gameState->paused)
    {
        return;
    }

    // CLEAR SCREEN:
    push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                      (draw_command_t){DCT_CLEAR_SCREEN, 0, 0, 0, 0, 0, true});
    // LOGIC:
    // PLAYER:
    entity_t *player = gameState->player;
    vec2_t player_movement = {0, 0};
    if (input_is_key_down(state, KEY_W))
    {
        player_movement.y -= 1;
    }
    if (input_is_key_down(state, KEY_S))
    {
        player_movement.y += 1;
    }
    if (input_is_key_down(state, KEY_A))
    {
        player_movement.x -= 1;
    }
    if (input_is_key_down(state, KEY_D))
    {
        player_movement.x += 1;
    }

    transform_t *playerTransform = entity_get_transform_t(&gameState->entityManager, player);

    if (vec2_length2(player_movement) > 0)
    {
        player_movement = vec2_normalize(player_movement);
        // TODO(pf): Implement correct steering logic.
        playerTransform->dir = player_movement;
    }
    playerTransform->vel = vec2_mul_s(player_movement, 5);

    // ASTEROIDS:
    if (gameState->asteroidCount < ASTEROID_COUNT)
    {
        spawn_asteroid(gameState);
    }
    gameState->spawnDelay -= state->dt;

    // BULLETS:
    // SPAWN:
    if (input_is_key_pressed(state, KEY_SPACE))
    {
        // TODO:
        spawn_bullet(gameState);
    }

    // DRAW COMMAND PUSH:

    /* SIMULATION:
     * For now: The only thing that changes the state is if something has a movement component.
     * TODO: Only velocity components are rendered.. should we have a separate component for rendering ?
     * TODO(entityA/entityB): Should we do something else to handle entity destruction during the simulation ?
        The good thing about removing them is that they don't linger and cause more events but it
        adds this complexity.
     */
    for (i = 1; i < gameState->entityManager.componentArrays[COMPONENT_TRANSFORM].count; ++i)
    {
        component_array_t *ca_transforms = &gameState->entityManager.componentArrays[COMPONENT_TRANSFORM];
        int velLU = ca_transforms->lookUp[i];
        entity_t *entity = &gameState->entityManager.entities[velLU];
        if (!entity->id)
            continue;

        transform_t *transform = entity_get_transform_t(&gameState->entityManager, entity);

        i32 color = PLAYER_COLOR;
        i32 width = PLAYER_WIDTH;
        i32 height = PLAYER_HEIGHT;
        switch (entity->type)
        {
        case ET_PLAYER:
            color = PLAYER_COLOR;
            width = PLAYER_WIDTH;
            height = PLAYER_HEIGHT;
            break;
        case ET_ASTEROID:
            color = ASTEROID_COLOR;
            width = ASTEROID_WIDTH;
            height = ASTEROID_HEIGHT;
            break;
        case ET_BULLET:
            color = COLOR_YELLOW;
            width = BULLET_WIDTH;
            height = BULLET_HEIGHT;
        default:
            break;
        }
        push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                          (draw_command_t){DCT_RECTANGLE, transform->pos.x, transform->pos.y, width, height, color, true, entity});

        i32 lineLength = width;
        push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                          (draw_command_t){DCT_LINE, transform->pos.x, transform->pos.y,
                                           transform->pos.x + transform->dir.x * lineLength,
                                           transform->pos.y + transform->dir.y * lineLength, COLOR_WHITE});

        // pos->prevX = pos->x;
        // pos->prevY = pos->y;
        transform->pos = vec2_add_v(transform->pos, transform->vel);
        vec2_t *pos = &transform->pos;
        // printf("Entity: %d, pos: %f, %f\n", entity->id, pos->x, pos->y);
        if (pos->x > state->render->width)
        {
            pos->x -= state->render->width;
        }

        if (pos->x < 0)
        {
            pos->x += state->render->width;
        }

        if (pos->y > state->render->height)
        {
            pos->y -= state->render->height;
        }

        if (pos->y < 0)
        {
            pos->y += state->render->height;
        }
        vec2_t *vel = &transform->vel;
        // vel->x *= 1.0f - transform->drag.x;
        // vel->y *= 1.0f - transform->drag.y;
    }

    // COLLISION:

    component_array_t *ca_collision = &gameState->entityManager.componentArrays[COMPONENT_COLLISION];
    for (i = 1; i < ca_collision->count; ++i)
    {
        entity_t *entityA = entity_manager_get_entity(&gameState->entityManager, ca_collision->lookUp[i]);
        if (!entityA->id)
            continue;

        collision_t *colA = entity_get_collision_t(&gameState->entityManager, entityA);
        bool collision = false;
        for (int j = i + 1; j < ca_collision->count; ++j)
        {
            entity_t *entityB = entity_manager_get_entity(&gameState->entityManager, ca_collision->lookUp[j]);
            if (!entityB->id)
                continue;

            transform_t *tA = entity_get_transform_t(&gameState->entityManager, entityA);
            collision_t *colB = entity_get_collision_t(&gameState->entityManager, entityB);

            transform_t *tB = entity_get_transform_t(&gameState->entityManager, entityB);

            i8 mask = ((colA->mask & (1 << entityB->type)) << 1) | (colB->mask & (1 << entityA->type));
            bool collision = mask && check_collision(colA, tA, colB, tB);
            // Check A
            if ((mask & 2) && collision)
            {
                check_collision(colA, tA, colB, tB);

                push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                                  (draw_command_t){DCT_RECTANGLE, tA->pos.x, tA->pos.y, colA->width + colB->width, colA->height + colB->height, COLLISION_OUTLINE_COLOR, false});

                colA->colliding = true;
                collision = true;
                // gameState->paused = true;
                printf("Entity %d colliding with %d\n", entityA->id, entityB->id);
                health_t *healthA = entity_get_health_t(&gameState->entityManager, entityA);
                health_t *healthB = entity_get_health_t(&gameState->entityManager, entityB);
                if (healthA)
                    entity_modify_health(gameState, entityA, healthA, -1);
                if (healthB)
                    entity_modify_health(gameState, entityB, healthB, -1);
            }

            // Check B
            if ((mask & 1) && collision)
            {
                printf("Entity %d colliding with %d\n", entityB->id, entityA->id);
                colB->colliding = true;
                // gameState->paused = true;

                push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                                  (draw_command_t){DCT_RECTANGLE, tB->pos.x, tB->pos.y, colB->width + colA->width, colB->height + colA->height, COLLISION_OUTLINE_COLOR, false});
                health_t *healthA = entity_get_health_t(&gameState->entityManager, entityA);
                health_t *healthB = entity_get_health_t(&gameState->entityManager, entityB);
                if (healthA)
                    entity_modify_health(gameState, entityA, healthA, -1);
                if (healthB)
                    entity_modify_health(gameState, entityB, healthB, -1);
            }

            if (collision)
            {
                push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                                  (draw_command_t){DCT_RECTANGLE, tA->pos.x, tA->pos.y, colA->width, colA->height, COLOR_BLUE, false});

                push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                                  (draw_command_t){DCT_RECTANGLE, tB->pos.x, tB->pos.y, colB->width, colB->height, COLOR_YELLOW, false});
            }
        }
        if (!collision)
        {
            colA->colliding = false;
        }
    }

    // !SIMULATION

    // DRAWING:
    render_state_t *render = state->render;

    // PROCESS DRAW COMMANDS:
    for (i = 0; i < gameState->drawCommandCount; ++i)
    {
        draw_command_t *dc = &gameState->drawCommands[i];
        collision_t *col = dc->dbg_entity ? entity_get_collision_t(&gameState->entityManager, dc->dbg_entity) : NULL;
        dc->color = (col && col->colliding) ? COLLIDED_COLOR : dc->color;

        switch (dc->type)
        {
        case DCT_CLEAR_SCREEN:
            if (dc->color == 0)
            {
                memset(render->pixels, 0, render->width * render->height * render->bytesPerPixel);
            }
            else
            {
                assert(false && "Not implemented.");
                for (int y = 0; y < render->height; ++y)
                {
                    for (int x = 0; x < render->width; ++x)
                    {
                        set_pixel(render, x, y, dc->color);
                    }
                }
            }
            break;
        case DCT_RECTANGLE:

            int hw = dc->width / 2;
            int hh = dc->height / 2;
            for (int y = -hh; y <= hh; ++y)
            {
                for (int x = -hw; x <= hw; ++x)
                {
                    if (dc->filled || x == -hw || x == hw || y == -hh || y == hh)
                        set_pixel(render, dc->x + x, dc->y + y, dc->color);
                }
            }
            break;

        case DCT_CIRCLE:

            break;

        case DCT_LINE:
            // https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)
            f32 dx = dc->width - dc->x;
            f32 dy = dc->height - dc->y;
            i32 steps;
            if (abs(dx) > abs(dy))
                steps = abs(dx);
            else
                steps = abs(dy);
            if (steps > 0)
            {
                dx /= steps;
                dy /= steps;
                f32 x = dc->x;
                f32 y = dc->y;
                int step = 0;
                while (step <= steps)
                {
                    set_pixel(render, (i32)x, (i32)y, dc->color);
                    x += dx;
                    y += dy;
                    step++;
                }
            }
            break;

        default:
            break;
        }
    }
    gameState->drawCommandCount = 0;
}

void game_shutdown(platform_state_t *state)
{
    game_state_shutdown(get_game_state(state));
    free(state->input);
    free(state->render->pixels);
    free(state->render);
}