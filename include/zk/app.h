// zero-kit app: the edges of an App's loop (ADR 0002).
//
// The App owns main() and its loop; zero-kit is called at three points:
//
//   zk_app *app = zk_app_init("hello", &argc, argv, NULL);
//   static uint8_t picture[ZK_PICTURE_BYTES];
//   while (zk_frame_begin(app)) {
//     ... draw this frame's Picture using zk_app_dt(app) ...
//     zk_frame_end(app, picture);
//   }
//   zk_app_close(app);
//
// zk_app_init takes the shared flags out of argv and leaves the App's own:
//   --fb PATH         the LCD framebuffer (default /dev/fb_lcd)
//   --rotate DEG      0, 90, 180 or 270 (default: guess from the panel's shape)
//   --swap            swap the bytes of each pixel (for panels that show wrong colours)
//   --fps N           target frame rate (default the App's, else 30)
//   --out DIR         a Headless run: write Pictures to DIR as PPM files instead of using the LCD
//   --frames N        stop after N frames
//   --every K         with --out, write every K-th frame (default 1)
//
// In a Headless run (ADR 0007) the time step is exactly 1/fps and nothing sleeps, so the same run always gives the
// same Pictures. Call everything from the App's loop thread. Only bad flags exit (with usage); nothing else does.
#pragma once
#include <stdint.h>

#include "zk/display.h"

#define ZK_PICTURE_BYTES (ZK_W * ZK_H * 4)   // a Picture: ZK_W x ZK_H RGBA, top row first (ADR 0004)

typedef struct zk_app zk_app;

// name: the App's name, used in messages (and later for Settings). app_usage: a line describing the App's own
// flags for --help, or NULL. Removes the flags it knows from argv and updates *argc.
// Prints usage and exits on --help (status 0) or on a bad shared flag (status 2).
zk_app *zk_app_init(const char *name, int *argc, char **argv, const char *app_usage);

// Sets the target frame rate unless --fps was given. Call before the first zk_frame_begin.
void zk_app_set_fps(zk_app *app, int fps);

// Starts a frame. Returns 0 when the App should stop (after --frames, zk_app_quit, or SIGINT/SIGTERM).
int zk_frame_begin(zk_app *app);

// Ends the frame: hands the Picture (ZK_PICTURE_BYTES, top row first) to every Output, then paces the loop.
void zk_frame_end(zk_app *app, const uint8_t *picture);

float zk_app_dt(const zk_app *app);        // seconds since the last frame began (exactly 1/fps in a Headless run)
double zk_app_time(const zk_app *app);     // seconds since the first frame (the sum of every dt)
long zk_app_frame(const zk_app *app);      // this frame's number, from 0
int zk_app_headless(const zk_app *app);    // 1 in a Headless run
const char *zk_app_name(const zk_app *app);
const char *zk_app_screen_status(const zk_app *app);   // what the LCD is, or why there isn't one

void zk_app_quit(zk_app *app);             // zk_frame_begin returns 0 next time
void zk_app_close(zk_app *app);            // releases everything; NULL is fine
