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
    position_t initial_pos = {x, y};
    position_t *pos = entity_add_position_t(&gameState->entityManager, gameState->player, initial_pos);
    velocity_t *vel = entity_add_velocity_t(&gameState->entityManager, gameState->player, (velocity_t){0, 0});
    collision_t *col = entity_add_collision_t(&gameState->entityManager, gameState->player, (collision_t){0, CT_RECTANGLE, PLAYER_WIDTH, PLAYER_HEIGHT});
    // health_t *health = entity_add_health_t(&gameState->entityManager, gameState->player, (health_t){3});
}

void spawn_asteroid(game_state_t *gameState)
{
    if(gameState->asteroidCount >= ASTEROID_COUNT || 
       gameState->spawnDelay > 0)
    {
        return;
    }
    entity_t *asteroid = entity_manager_create_entity(&gameState->entityManager);
    asteroid->type = ET_ASTEROID;
    position_t *pos = entity_add_position_t(&gameState->entityManager, asteroid, (position_t){rand() % 800, rand() % 600});
    velocity_t *vel = entity_add_velocity_t(&gameState->entityManager, asteroid, (velocity_t){rand() % 5, rand() % 5});
    collision_t *col = entity_add_collision_t(&gameState->entityManager, asteroid, (collision_t){CM_PLAYER, CT_RECTANGLE, ASTEROID_WIDTH, ASTEROID_HEIGHT});
    health_t *health = entity_add_health_t(&gameState->entityManager, asteroid, (health_t){1});
    gameState->asteroidCount++;
    gameState->spawnDelay = ASTEROID_SPAWN_DELAY;
}

void game_state_shutdown(game_state_t *gameState)
{
    entity_manager_shutdown(&gameState->entityManager);
}

void game_init(platform_state_t *state)
{
    // TODO: This is NOT platform specific.
    state->isRunning = true;
    state->activeInput = (platform_input_t *)malloc(sizeof(platform_input_t));
    state->lastInput = (platform_input_t *)malloc(sizeof(platform_input_t));
    state->render = (render_state_t *)malloc(sizeof(render_state_t));

    state->render->width = 800;
    state->render->height = 600;
    state->render->bytesPerPixel = 4;
    state->render->pitch = state->render->width * state->render->bytesPerPixel;
    state->render->pixels = malloc(state->render->width * state->render->height *
                                   state->render->bytesPerPixel);
    memset(state->render->pixels, 0, state->render->width * state->render->height * state->render->bytesPerPixel);

    memset(state->activeInput, 0, sizeof(platform_input_t));
    memset(state->lastInput, 0, sizeof(platform_input_t));
    state->dt = 0.0f;

    state->memorySize = GB(1);
    state->memory = malloc(state->memorySize);
    game_state_t *gameState = get_game_state(state);

    entity_manager_init(&gameState->entityManager, 10);
    spawn_player(gameState, state->render->width / 2, state->render->height / 2);

    gameState->asteroidCount = 0;
}

void game_logic(platform_state_t *state)
{
    int i;

    if (state->activeInput->keys[KEY_ESCAPE] == KEY_PRESSED)
    {
        state->isRunning = false;
    }

    game_state_t *gameState = get_game_state(state);

    if (state->activeInput->keys[KEY_P] == KEY_PRESSED)
    {
        gameState->paused = false;
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
    if (state->activeInput->keys[KEY_W] == KEY_PRESSED)
    {
        player_movement.y -= 1;
    }
    if (state->activeInput->keys[KEY_S] == KEY_PRESSED)
    {
        player_movement.y += 1;
    }
    if (state->activeInput->keys[KEY_A] == KEY_PRESSED)
    {
        player_movement.x -= 1;
    }
    if (state->activeInput->keys[KEY_D] == KEY_PRESSED)
    {
        player_movement.x += 1;
    }

    velocity_t *vel = entity_get_velocity_t(&gameState->entityManager, player);

    if (vec2_length2(player_movement) > 0)
    {
        player_movement = vec2_normalize(player_movement);
    }
    vel->x = player_movement.x * 5;
    vel->y = player_movement.y * 5;
    // printf("Player movement: %f, %f\n", player_movement.x, player_movement.y);
    position_t *pos = entity_get_position_t(&gameState->entityManager, player);

    // ASTEROIDS:
    if (gameState->asteroidCount < ASTEROID_COUNT)
    {
        spawn_asteroid(gameState);
    }
    gameState->spawnDelay -= state->dt;

    // BULLETS:
    // SPAWN:
    if (state->activeInput->keys[KEY_SPACE] == KEY_PRESSED)
    {
        // TODO:
    }

    // DRAW COMMAND PUSH:

    /* SIMULATION:
     * For now: The only thing that changes the state is if something has a movement component.
     * TODO: Only velocity components are rendered.. should we have a separate component for rendering ?
     * TODO(entityA/entityB): Should we do something else to handle entity destruction during the simulation ?
        The good thing about removing them is that they don't linger and cause more events but it
        adds this complexity.
     */
    for (i = 1; i < gameState->entityManager.componentArrays[COMPONENT_VELOCITY].count; ++i)
    {
        component_array_t *ca_vel = &gameState->entityManager.componentArrays[COMPONENT_VELOCITY];
        int velLU = ca_vel->lookUp[i];
        entity_t *entity = &gameState->entityManager.entities[velLU];
        if (!entity->id)
            continue;
        velocity_t *vel = entity_get_velocity_t(&gameState->entityManager, entity);
        position_t *pos = entity_get_position_t(&gameState->entityManager, entity);

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
        default:
            break;
        }
        push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                          (draw_command_t){DCT_RECTANGLE, pos->x, pos->y, width, height, color, true, entity});

        i32 lineLength = width;
        push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                          (draw_command_t){DCT_LINE, pos->x, pos->y,
                                           pos->x + vel->x * lineLength,
                                           pos->y + vel->y * lineLength, COLOR_WHITE});

        // pos->prevX = pos->x;
        // pos->prevY = pos->y;
        pos->x = pos->x + vel->x;
        pos->y = pos->y + vel->y;
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
        vel->x *= 1.0f - vel->dragX;
        vel->y *= 1.0f - vel->dragY;
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

            position_t *posA = entity_get_position_t(&gameState->entityManager, entityA);
            collision_t *colB = entity_get_collision_t(&gameState->entityManager, entityB);
            position_t *posB = entity_get_position_t(&gameState->entityManager, entityB);

            i8 mask = ((colA->mask & (1 << entityB->type)) << 1) | (colB->mask & (1 << entityA->type));
            bool collision = mask && check_collision(colA, posA, colB, posB);
            // Check A
            if ((mask & 2) && collision)
            {
                check_collision(colA, posA, colB, posB);

                push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                                  (draw_command_t){DCT_RECTANGLE, posA->x, posA->y, colA->width + colB->width, colA->height + colB->height, COLLISION_OUTLINE_COLOR, false});

                colA->colliding = true;
                collision = true;
                // gameState->paused = true;
                printf("Entity %d colliding with %d\n", entityA->id, entityB->id);
                health_t *health = entity_get_health_t(&gameState->entityManager, entityA);
                if (health)
                {
                    health->health -= 1;
                    if (health->health <= 0)
                    {
                        entity_manager_destroy_entity(&gameState->entityManager, entityA);
                        if(entityA->type == ET_ASTEROID)
                        {
                            gameState->asteroidCount--;
                        }
                    }
                }
            }

            // Check B
            if ((mask & 1) && collision)
            {
                printf("Entity %d colliding with %d\n", entityB->id, entityA->id);
                colB->colliding = true;
                // gameState->paused = true;

                push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                                  (draw_command_t){DCT_RECTANGLE, posB->x, posB->y, colB->width + colA->width, colB->height + colA->height, COLLISION_OUTLINE_COLOR, false});
                health_t *health = entity_get_health_t(&gameState->entityManager, entityB);
                if (health)
                {
                    health->health -= 1;
                    if (health->health <= 0)
                    {
                        entity_manager_destroy_entity(&gameState->entityManager, entityB);
                        if(entityB->type == ET_ASTEROID)
                        {
                            gameState->asteroidCount--;
                        }
                    }
                }
            }

            if (collision)
            {

                push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                                  (draw_command_t){DCT_RECTANGLE, posA->x, posA->y, colA->width, colA->height, COLOR_BLUE, false});

                push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                                  (draw_command_t){DCT_RECTANGLE, posB->x, posB->y, colB->width, colB->height, COLOR_YELLOW, false});
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
    free(state->activeInput);
    free(state->lastInput);
    free(state->render->pixels);
    free(state->render);
}