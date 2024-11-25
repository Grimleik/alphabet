/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#include "game.h"
#include "cu_math.h"
#include "entity.h"

#define ASTEROID_COUNT 5

enum DRAW_COMMAND_TYPE
{
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
    i32 color;
};

#define MAX_DRAW_COMMANDS 100
typedef struct asteroid_t asteroid_t;
struct asteroid_t
{
    entity_t *entity;
    asteroid_t *next;
};

typedef struct game_state_t game_state_t;
struct game_state_t
{
    entity_manager_t entityManager;
    entity_t *player;
    asteroid_t *asteroids;
    i32 asteroidCount;
    draw_command_t drawCommands[MAX_DRAW_COMMANDS];
    i32 drawCommandCount;
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

void spawn_asteroid(game_state_t *gameState)
{
    entity_t *asteroid = entity_manager_create_entity(&gameState->entityManager);
    asteroid->type = ET_ASTEROID;
    position_t *pos = entity_add_position_t(&gameState->entityManager, asteroid, (position_t){rand() % 800, rand() % 600});
    velocity_t *vel = entity_add_velocity_t(&gameState->entityManager, asteroid, (velocity_t){rand() % 5, rand() % 5});
    asteroid_t *newAsteroid = (asteroid_t *)malloc(sizeof(asteroid_t));
    newAsteroid->entity = asteroid;
    newAsteroid->next = gameState->asteroids;
    gameState->asteroids = newAsteroid;
    gameState->asteroidCount++;
}

void game_state_shutdown(game_state_t *gameState)
{
    entity_manager_shutdown(&gameState->entityManager);
    asteroid_t *p = gameState->asteroids;
    while (p)
    {
        asteroid_t *next = p->next;
        free(p);
        p = next;
    }
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
    gameState->player = entity_manager_create_entity(&gameState->entityManager);
    gameState->player->type = ET_PLAYER;
    position_t *pos = entity_add_position_t(&gameState->entityManager, gameState->player, (position_t){40, 40});
    pos->x = state->render->width / 2;
    pos->y = state->render->height / 2;
    velocity_t *vel = entity_add_velocity_t(&gameState->entityManager, gameState->player, (velocity_t){0, 0});

    gameState->asteroidCount = 0;
    for (int i = 0; i < ASTEROID_COUNT; ++i)
    {
        spawn_asteroid(gameState);
    }
}

void game_logic(platform_state_t *state)
{
    if (state->activeInput->keys[KEY_ESCAPE] == KEY_PRESSED)
    {
        state->isRunning = false;
    }

    game_state_t *gameState = get_game_state(state);

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
    push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                      (draw_command_t){DCT_RECTANGLE, pos->x, pos->y, 40, 40, 0xFF00FF00});
    i32 lineLength = 20;
    push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                      (draw_command_t){DCT_LINE, pos->x, pos->y,
                                       pos->x + vel->x * lineLength,
                                       pos->y + vel->y * lineLength, 0xFFFFFFFF});
    // BULLETS:
    // SPAWN:
    if (state->activeInput->keys[KEY_SPACE] == KEY_PRESSED)
    {
    }
    // ASTEROIDS:
    if (gameState->asteroidCount < ASTEROID_COUNT)
    {
        spawn_asteroid(gameState);
    }

    for (asteroid_t *asteroid = gameState->asteroids; asteroid; asteroid = asteroid->next)
    {
        position_t *pos = entity_get_position_t(&gameState->entityManager, asteroid->entity);
        push_draw_command(gameState->drawCommands, &gameState->drawCommandCount,
                          (draw_command_t){DCT_RECTANGLE, pos->x, pos->y, 10, 10, 0xFFFF0000});
    }

    // UPDATE POSITIONS:
    for (int i = 0; i < gameState->entityManager.componentArrays[COMPONENT_VELOCITY].count; ++i)
    {
        component_array_t *ca_vel = &gameState->entityManager.componentArrays[COMPONENT_VELOCITY];
        int lookup = ca_vel->lookUp[i];
        if (lookup)
        {
            // TODO: This is a bit messy to go from lookup into entity to pass entity to get int that we already had ??
            entity_t *entity = &gameState->entityManager.entities[lookup];
            velocity_t *vel = entity_get_velocity_t(&gameState->entityManager, entity);
            position_t *pos = entity_get_position_t(&gameState->entityManager, entity);
            pos->x += vel->x;
            pos->y += vel->y;
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
    }
    // DRAWING: 
    render_state_t *render = state->render;
    // CLEAR SCREEN:
    memset(render->pixels, 0, render->width * render->height * render->bytesPerPixel);

    // DRAW ENTITIES:
    for (int i = 0; i < gameState->drawCommandCount; ++i)
    {
        draw_command_t *dc = &gameState->drawCommands[i];
        switch (dc->type)
        {
        case DCT_RECTANGLE:

            int hw = dc->width / 2;
            int hh = dc->height / 2;
            for (int y = -hh; y <= hh; ++y)
            {
                for (int x = -hw; x <= hw; ++x)
                {
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
                int i = 0;
                while (i <= steps)
                {
                    set_pixel(render, (i32)x, (i32)y, dc->color);
                    x += dx;
                    y += dy;
                    i++;
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