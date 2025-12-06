#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include <pulse/simple.h>
#include <pulse/error.h>
#include <X11/Xlib.h>

#include "game.h"

#define NANO_PER_SEC (1000LL * 1000LL * 1000LL)

long long now_nano_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * NANO_PER_SEC + ts.tv_nsec;
}


int main(void) {
    Game game = game_init();

    printf("game.fps               = %ld\n", game.fps);
    printf("game.display_width     = %ld\n", game.display_width);
    printf("game.display_height    = %ld\n", game.display_height);
    printf("game.audio_channels    = %ld\n", game.audio_channels);
    printf("game.audio_sample_rate = %ld\n", game.audio_sample_rate);

    pa_sample_spec ss = {
        .format=PA_SAMPLE_S16LE,
        .rate=game.audio_sample_rate,
        .channels=game.audio_channels,
    };

    int pa_error = 0;
    pa_simple *pa = pa_simple_new(
        NULL,               // default server
        "The Game",         // application name
        PA_STREAM_PLAYBACK, // playback only
        NULL,               // default device
        "playback",         // stream description
        &ss,
        NULL,
        NULL,
        &pa_error
    );

    if (pa == NULL) {
        fprintf(stderr, "ERROR: pa_simple_new() failed: %s\n", pa_strerror(pa_error));
        return 1;
    }

   Display *display = XOpenDisplay(NULL);
   if (display == NULL) {
       fprintf(stderr, "ERROR: could not open default X display\n");
       return 1;
   }
   printf("Display: %p\n", display);

   Window window = XCreateSimpleWindow(
                display,
                XDefaultRootWindow(display),
                0, 0,
                game.display_width, game.display_height,
                0,
                0,
                0
           );

   printf("Window = %ld\n", window);

   XWindowAttributes wa = {0};
   XGetWindowAttributes(display, window, &wa);

   XImage *image = XCreateImage(display,
           wa.visual,
           wa.depth,
           ZPixmap,
           0,
           (char*) game.display,
           game.display_width,
           game.display_height,
           32,
           game.display_width * sizeof(*game.display));

   GC gc = XCreateGC(display, window, 0, NULL);
   printf("gc = %p\n", gc);


   Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
   XSetWMProtocols(display, window, &wm_delete_window, 1);

   XSelectInput(display, window, KeyPressMask | PointerMotionMask);
   XStoreName(display, window, "Game Console");

   XMapWindow(display, window);
   XFlush(display);
   uint64_t frame_ns = NANO_PER_SEC / game.fps;

   bool quit = false;
   while (!quit) {
       uint64_t begin = now_nano_sec();
       while (XPending(display) > 0) {
           XEvent event = {0};
           XNextEvent(display, &event);
           switch (event.type) {
               case KeyPress: {
                    switch (XLookupKeysym(&event.xkey, 0)) {
                        case 'q':
                            quit = true;
                            break;
                    }
                } break;

               case MotionNotify: {
                    // Example: handle mouse position
                    // int mx = event.xmotion.x;
                    // int my = event.xmotion.y;
                } break;

               case ClientMessage: {
                    if ((Atom) event.xclient.data.l[0] == wm_delete_window) {
                        quit = true;
                    }
                } break;
               default: 
                    break;
           }
       }
       game_update();
       uint64_t end = now_nano_sec();


       XPutImage(display, window, gc, image, 0, 0, 0, 0, game.display_width, game.display_height);

       size_t audio_size_in_bytes = game.audio_sample_rate / game.fps * game.audio_channels*sizeof(*game.audio);
       pa_simple_write(pa, game.audio, audio_size_in_bytes, &pa_error);
       
       // Frame limiter
       uint64_t elapsed = end - begin;
       if (elapsed < frame_ns) usleep((frame_ns - elapsed) / 1000);
   }

    return 0;
}
