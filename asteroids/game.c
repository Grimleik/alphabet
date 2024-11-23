/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#include "game.h"

enum ENTITY_TYPE { PLAYER, BULLET, ASTEROID };

typedef struct entity_t entity_t;
struct entity_t {
    int x, y;
    int w, h;
};

typedef struct game_state_t game_state_t;
struct game_state_t {
    entity_t *player;
    entity_t *bullets;
    entity_t *freeList;
    entity_t *asteroids;
    u32 entityCount;
};
game_state_t *get_game_state(platform_state_t *state) {
    return (game_state_t *)state->memory;
}

void game_init(platform_state_t *state) {
    // TODO: This is NOT platform specific.
    state->isRunning = true;
    state->activeInput = (platform_input_t *)malloc(sizeof(platform_input_t));
    state->lastInput = (platform_input_t *)malloc(sizeof(platform_input_t));
    state->render = (render_state_t *)malloc(sizeof(render_state_t));

    state->render->width = 800;
    state->render->height = 600;
    state->render->bytesPerPixel = 4;
    state->render->pitch = state->render->width * state->render->bytesPerPixel;
    state->render->pixels =
        malloc(state->render->width * state->render->height *
               state->render->bytesPerPixel);
    memset(state->render->pixels, 0,
           state->render->width * state->render->height *
               state->render->bytesPerPixel);

    memset(state->activeInput, 0, sizeof(platform_input_t));
    memset(state->lastInput, 0, sizeof(platform_input_t));
    state->dt = 0.0f;

    state->memorySize = GB(1);
    state->memory = malloc(state->memorySize);
    game_state_t *gameState = get_game_state(state);
    gameState->player.x = 40;
    gameState->player.y = 40;

    gameState->player.w = 40;
    gameState->player.h = 40;
}

void set_pixel(render_state_t *render, int x, int y, int color) {
    if (x < 0 || x >= render->width || y < 0 || y >= render->height) {
        return;
    }

    int *pixel = (int *)((u8 *)render->pixels + x * render->bytesPerPixel +
                         y * render->pitch);
    *pixel = color;
}

void game_logic(platform_state_t *state) {
    if (state->activeInput->keys[KEY_ESCAPE] == KEY_PRESSED) {
        state->isRunning = false;
    }

    game_state_t *gameState = get_game_state(state);

    // LOGIC:
    if (state->activeInput->keys[KEY_W] == KEY_PRESSED) {
        gameState->player.y -= 1;
    }
    if (state->activeInput->keys[KEY_S] == KEY_PRESSED) {
        gameState->player.y += 1;
    }
    if (state->activeInput->keys[KEY_A] == KEY_PRESSED) {
        gameState->player.x -= 1;
    }
    if (state->activeInput->keys[KEY_D] == KEY_PRESSED) {
        gameState->player.x += 1;
    }

    // DRAWING:
    render_state_t *render = state->render;
    // CLEAR SCREEN:
    memset(render->pixels, 0,
           render->width * render->height * render->bytesPerPixel);

    for (int y = gameState->player.y;
         y < gameState->player.y + gameState->player.h; y++) {
        for (int x = gameState->player.x;
             x < gameState->player.x + gameState->player.w; x++) {
            set_pixel(render, x, y, 0xFF0000);
        }
    }
}

void game_shutdown(platform_state_t *state) {
    free(state->activeInput);
    free(state->lastInput);
    free(state->render->pixels);
    free(state->render);
}