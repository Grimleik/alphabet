/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/

#include "platform.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <time.h>
#include <unistd.h>

int x11_error_handler(Display *display, XErrorEvent *error)
{
	char error_text[1024];
	XGetErrorText(display, error->error_code, error_text, 1024);
	fprintf(stderr, "X11 Error: %d\n", error->error_code);
	fprintf(stderr, "%s \n", error_text);
	return 0;
}

double get_time_in_seconds()
{
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	return (double)time.tv_sec + (double)time.tv_nsec / 1e9;
}

typedef struct linux_state_t linux_state_t;
struct linux_state_t
{
	Display *display;
	Window window;
	int screen;
};

platform_state_t *platform_init(platform_callback_t callback)
{
	platform_state_t *result = (platform_state_t *)malloc(sizeof(platform_state_t));

	result->handle = (linux_state_t *)malloc(sizeof(linux_state_t));

	callback(result);
	linux_state_t *state = (linux_state_t *)result->handle;
	int status;

	XSetErrorHandler(x11_error_handler);

	state->display = XOpenDisplay(NULL);
	if (state->display == NULL)
	{
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

void platform_start(platform_state_t *platform, platform_callback_t callback)
{
	XEvent event;
	int status;
	render_state_t *render = platform->render;
	linux_state_t *state = (linux_state_t *)platform->handle;
	platform->frame = 0;

	XImage *image = XCreateImage(
		state->display, DefaultVisual(state->display, state->screen),
		DefaultDepth(state->display, state->screen), ZPixmap, 0,
		(char *)render->pixels, render->width, render->height, 32, 0);

	f64 lastTime = get_time_in_seconds();
	f64 currentTime = 0.0f;
	while (platform->isRunning)
	{
		// NOTE(pf): Msg pump.
		while (XPending(state->display))
		{
			XNextEvent(state->display, &event);
			if (event.type == KeyPress || event.type == KeyRelease)
			{
				XKeyEvent *keyEvent = (XKeyEvent *)&event;
				platform_input_buffer_t *buffer = platform->input->buffers + platform->input->activeInputBuffer;
				platform_input_buffer_t *prevBuffer = platform->input->buffers + wrap(platform->input->activeInputBuffer - 1, NR_OF_INPUT_BUFFERS);
				// if (buffer->keys[keyEvent->keycode].transitions == 0)
				{
					if (event.type == KeyPress)
					{
						buffer->keys[keyEvent->keycode] = KEY_DOWN;
						// buffer->keys[keyEvent->keycode].state = KEY_DOWN;
						// buffer->keys[keyEvent->keycode].transitions++;
					}
					else if (event.type == KeyRelease)
					{

						buffer->keys[keyEvent->keycode] = KEY_UP;
						// buffer->keys[keyEvent->keycode].state = KEY_UP;
						// buffer->keys[keyEvent->keycode].transitions++;
					}

					// printf("Frame %ld: Keycode: %d changed from %d to %d in event %d.\n",
					//        platform->frame,
					//        keyEvent->keycode,
					//        prevBuffer->keys[keyEvent->keycode],
					//        buffer->keys[keyEvent->keycode],
					//        event.type);
				}
			}
		}

		// Callback to game logic.
		callback(platform);

		// Draw.
		status = XPutImage(state->display, state->window,
						   DefaultGC(state->display, state->screen), image, 0,
						   0, 0, 0, render->width, render->height);

		status = XFlush(state->display);

		// Input swap.

		platform->input->activeInputBuffer = wrap(platform->input->activeInputBuffer + 1, NR_OF_INPUT_BUFFERS);
		memcpy(&platform->input->buffers[platform->input->activeInputBuffer],
			   &platform->input->buffers[wrap(platform->input->activeInputBuffer - 1, NR_OF_INPUT_BUFFERS)],
			   sizeof(int) * 256);

		// Timing.
		currentTime = get_time_in_seconds();

		long sleep_ns = (TARGET_FRAME_DURATION - platform->dt) * 1e9;
		if (platform->dt < TARGET_FRAME_DURATION)
		{
			struct timespec sleep_time;
			sleep_time.tv_sec = sleep_ns / 1e9;
			sleep_time.tv_nsec = sleep_ns % 1000000000L;
			nanosleep(&sleep_time, NULL);
			// STUDY(pf): We must update our dt if we need to sleep!
			currentTime = get_time_in_seconds();
		}

		platform->dt = currentTime - lastTime;
		platform->totalTime += platform->dt;
		lastTime = currentTime;
		platform->frame++;
		// printf("FPS: %f %ld\n", 1.0f / platform->dt, sleep_ns);
	}
}

void platform_shutdown(platform_state_t **platform_state,
					   platform_callback_t callback)
{

	callback(*platform_state);

	linux_state_t *state = (linux_state_t *)(*platform_state)->handle;
	XCloseDisplay(state->display);
	free((*platform_state)->handle);
	free(*platform_state);
	*platform_state = NULL;
}