#if !defined(PLATFORM_SHARED_H)
/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#define PLATFORM_SHARED_H
#include <stdbool.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "core.h"

#define KB(x) (x * 1024)
#define MB(x) (KB(x) * 1024)
#define GB(x) (MB(x) * 1024)

enum KEY_STATES { KEY_UP = 0, KEY_DOWN = 1, KEY_PRESSED = 2, KEY_RELEASED = 3 };

enum KEYS {
    KEY_ESCAPE = 9,
    KEY_W = 25,
    KEY_A = 38,
    KEY_S = 39,
    KEY_D = 40,
    KEY_SPACE = 65,
    KEY_LEFT_ARROW = 113,
    KEY_RIGHT_ARROW = 114,
    KEY_UP_ARROW = 111,
    KEY_DOWN_ARROW = 116,
    KEY_ENTER = 36,
    KEY_BACKSPACE = 22,
    _COUNT = 256,
};

typedef struct platform_input_t platform_input_t;
struct platform_input_t {
    int keys[256];
};

typedef struct render_state_t render_state_t;
struct render_state_t {
    void *pixels;
    int width;
    int height;
    int pitch;
    int bytesPerPixel;
};

typedef struct platform_state_t platform_state_t;
struct platform_state_t {
    bool isRunning;
    void *handle;
    f64 dt;
    f64 totalTime;

    render_state_t *render;
    // TODO: Concat into one structure.
    platform_input_t *activeInput;
    platform_input_t *lastInput;

    void* memory;
    u32 memorySize;
};

typedef void (*platform_callback_t)(platform_state_t *state);

// PLATFORM API
/* REQUIRED FUNCTIONALITY
* Fetch input
* Fetch time
* Present graphics through backbuffer.
*/
platform_state_t *platform_init(platform_callback_t callback);
void platform_start(platform_state_t *platform, platform_callback_t callback);
void platform_shutdown(platform_state_t **platform_state, platform_callback_t callback);

#endif
