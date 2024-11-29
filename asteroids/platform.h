#if !defined(PLATFORM_SHARED_H)
/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/

/* TODO:
 * Linux:
 */
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

enum KEY_STATES
{
    KEY_UP = 0,
    KEY_DOWN = 1,
};

enum KEYS
{
    KEY_ESCAPE = 9,
    KEY_W = 25,
    KEY_A = 38,
    KEY_S = 39,
    KEY_D = 40,
    KEY_P = 33,
    KEY_SPACE = 65,
    KEY_LEFT_ARROW = 113,
    KEY_RIGHT_ARROW = 114,
    KEY_UP_ARROW = 111,
    KEY_DOWN_ARROW = 116,
    KEY_ENTER = 36,
    KEY_BACKSPACE = 22,
    KEY_COUNT = 256,
};

// typedef struct key_state_t key_state_t;
// struct key_state_t {
//     i32 state;
//     i32 transitions;
// };

typedef struct platform_input_buffer_t platform_input_buffer_t;
struct platform_input_buffer_t
{
    // key_state_t keys[256];
    int keys[256];
};

typedef struct render_state_t render_state_t;
struct render_state_t
{
    void *pixels;
    int width;
    int height;
    int pitch;
    int bytesPerPixel;
};

#define NR_OF_INPUT_BUFFERS 3
typedef struct platform_input_t platform_input_t;
struct platform_input_t {
    platform_input_buffer_t buffers[NR_OF_INPUT_BUFFERS];
    i32 activeInputBuffer;
};


typedef struct platform_state_t platform_state_t;
struct platform_state_t
{
    bool isRunning;
    void *handle;
    u64 frame;
    f64 dt;
    f64 totalTime;

    render_state_t *render;
    platform_input_t *input;
    
    // TODO: Concat into one structure.
    // platform_input_t *activeInput;
    // platform_input_t *activeInput;
    // platform_input_t *lastInput;

    void *memory;
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
bool input_is_key_down(platform_state_t *platform, int key);
bool input_is_key_pressed(platform_state_t *platform, int key);
bool input_is_key_released(platform_state_t *platform, int key);
// void platform_advance_input(platform_state_t *platform);

#endif
