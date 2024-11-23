/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/

#include "platform.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <time.h>

int x11_error_handler(Display *display, XErrorEvent *error) {
    char error_text[1024];
    XGetErrorText(display, error->error_code, error_text, 1024);
    fprintf(stderr, "X11 Error: %d\n", error->error_code);
    fprintf(stderr, "%s \n", error_text);
    return 0;
}

double get_time_in_seconds() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (double)time.tv_sec + (double)time.tv_nsec / 1e6;
}

typedef struct linux_state_t linux_state_t;
struct linux_state_t {
    Display *display;
    Window window;
    int screen;
};

platform_state_t *platform_init(platform_callback_t callback) {
    platform_state_t *result =
        (platform_state_t *)malloc(sizeof(platform_state_t));

    result->handle = (linux_state_t *)malloc(sizeof(linux_state_t));

    callback(result);
    linux_state_t *state = (linux_state_t *)result->handle;
    int status;

    XSetErrorHandler(x11_error_handler);

    state->display = XOpenDisplay(NULL);
    if (state->display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    state->screen = DefaultScreen(state->display);

    state->window = XCreateSimpleWindow(
        state->display, RootWindow(state->display, state->screen), 10, 10, 800,
        600, 1, BlackPixel(state->display, state->screen),
        WhitePixel(state->display, state->screen));

    status = XMapWindow(state->display, state->window);
    XSelectInput(state->display, state->window, ExposureMask | KeyPressMask | KeyReleaseMask);
    return result;
}

void platform_start(platform_state_t *platform, platform_callback_t callback) {

    XEvent event;
    int status;
    render_state_t *render = platform->render;
    linux_state_t *state = (linux_state_t *)platform->handle;

    XImage *image = XCreateImage(
        state->display, DefaultVisual(state->display, state->screen),
        DefaultDepth(state->display, state->screen), ZPixmap, 0,
        (char *)render->pixels, render->width, render->height, 32, 0);

    f64 lastTime = get_time_in_seconds();
    f64 currentTime = 0.0f;
    while (platform->isRunning) {

        while (XPending(state->display)) {
            XNextEvent(state->display, &event);
            if (event.type == KeyPress || event.type == KeyRelease) {
                XKeyEvent *keyEvent = (XKeyEvent *)&event;
                platform->activeInput->keys[keyEvent->keycode] =
                    event.type == KeyPress ? KEY_PRESSED : KEY_RELEASED;
                    printf("Keycode: %d changed to %d.\n", keyEvent->keycode, event.type == KeyPress ? KEY_PRESSED : KEY_RELEASED);
            }
        }

        // Callback to game logic.
        callback(platform);
        // TODO: Draw.

        status = XPutImage(state->display, state->window,
                           DefaultGC(state->display, state->screen), image, 0,
                           0, 0, 0, render->width, render->height);

        status = XFlush(state->display);
        // TODO: Input.
        memcpy(platform->lastInput->keys, platform->activeInput->keys,
               sizeof(int) * 256);
        // memset(platform->activeInput->keys, 0, sizeof(int) * 256);
        // TODO: Timing.
        currentTime = get_time_in_seconds();
        platform->dt = currentTime - lastTime;
        platform->totalTime += platform->dt;
        lastTime = currentTime;
        // printf("FPS: %f\n", 1000.0f / platform->dt);
        // TODO: Better frame limiting.
        usleep(1000000 / 60);
    }

    image->data = NULL;
    status = XDestroyImage(image);
}

void platform_shutdown(platform_state_t **platform_state,
                       platform_callback_t callback) {

    callback(*platform_state);

    linux_state_t *state = (linux_state_t *)(*platform_state)->handle;
    XCloseDisplay(state->display);
    free((*platform_state)->handle);
    free(*platform_state);
    *platform_state = NULL;
}