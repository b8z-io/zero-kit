// zero-kit display: the CardputerZero's LCD, a Screen.
//
// Takes a Picture (ZK_W x ZK_H RGBA, top row first) and writes it to a Linux framebuffer in whatever pixel format
// and orientation the panel reports. The panel may say it is portrait (170 x 320), so the Picture is turned; some
// panels want the two bytes of each RGB565 pixel swapped.
//
// Call from the App's loop thread only. Nothing here exits: a failure returns NULL and says why in err.
#pragma once
#include <stddef.h>
#include <stdint.h>

#define ZK_W 320
#define ZK_H 170

typedef struct zk_display zk_display;

// rotate: 0, 90, 180 or 270, or -1 to guess from the panel's shape. swap: 1 to swap each pixel's two bytes.
// Returns NULL if path isn't a usable framebuffer, with the reason in err (err may be NULL).
zk_display *zk_display_open(const char *path, int rotate, int swap, char *err, size_t n);

// A one-line description of the panel, e.g. "/dev/fb_lcd 170x320 16 bpp, rotate 90".
const char *zk_display_describe(const zk_display *d);

// Shows a Picture: ZK_W x ZK_H RGBA, top row first.
void zk_display_show(zk_display *d, const uint8_t *picture);

// Blanks the panel, releases it and frees d. NULL is fine.
void zk_display_close(zk_display *d);
