// zero-kit display: the LCD framebuffer (ported from Dome Zero, which had it from Wake Zero).
#include "zk/display.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

struct zk_display {
  int fd, xres, yres, bpp, line, rotate, swap;
  struct fb_bitfield r, g, b;
  unsigned char *map, *back;
  size_t size;
  char what[160];
};

zk_display *zk_display_open(const char *path, int rotate, int swap, char *err, size_t n) {
  char dummy[8];
  if (!err) { err = dummy; n = sizeof dummy; }
  int fd = open(path, O_RDWR);
  if (fd < 0) { snprintf(err, n, "can't open %s: %s", path, strerror(errno)); return NULL; }
  struct fb_var_screeninfo v;
  struct fb_fix_screeninfo fx;
  if (ioctl(fd, FBIOGET_VSCREENINFO, &v) < 0 || ioctl(fd, FBIOGET_FSCREENINFO, &fx) < 0) {
    snprintf(err, n, "%s isn't a framebuffer", path);
    close(fd);
    return NULL;
  }
  if (v.bits_per_pixel != 16 && v.bits_per_pixel != 24 && v.bits_per_pixel != 32) {
    snprintf(err, n, "%s has %u bits per pixel, which isn't handled", path, v.bits_per_pixel);
    close(fd);
    return NULL;
  }
  zk_display *d = calloc(1, sizeof *d);
  if (!d) { snprintf(err, n, "out of memory"); close(fd); return NULL; }
  d->fd = fd;
  d->xres = v.xres; d->yres = v.yres; d->bpp = v.bits_per_pixel; d->line = fx.line_length;
  d->r = v.red; d->g = v.green; d->b = v.blue;
  d->size = (size_t)fx.line_length * v.yres;
  // A portrait panel (170 x 320) needs the Picture turned; an explicit rotate overrides the guess.
  d->rotate = rotate >= 0 ? rotate : (d->xres < d->yres ? 90 : 0);
  d->swap = swap;
  d->map = mmap(NULL, d->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (d->map == MAP_FAILED) d->map = NULL;   // fall back to pwrite()
  d->back = calloc(1, d->size);
  if (!d->back) { snprintf(err, n, "out of memory"); zk_display_close(d); return NULL; }
  snprintf(d->what, sizeof d->what, "%s %dx%d %d bpp, rotate %d%s", path, d->xres, d->yres, d->bpp, d->rotate,
           d->swap ? ", bytes swapped" : "");
  return d;
}

const char *zk_display_describe(const zk_display *d) { return d ? d->what : "no display"; }

static inline unsigned pack_px(const zk_display *d, const uint8_t *p) {
  return ((unsigned)(p[0] >> (8 - d->r.length)) << d->r.offset) | ((unsigned)(p[1] >> (8 - d->g.length)) << d->g.offset) |
         ((unsigned)(p[2] >> (8 - d->b.length)) << d->b.offset);
}

static void flush(zk_display *d) {
  if (d->map) memcpy(d->map, d->back, d->size);
  else if (pwrite(d->fd, d->back, d->size, 0) < 0) { /* nothing useful to do; the next frame tries again */ }
}

void zk_display_show(zk_display *d, const uint8_t *picture) {
  if (!d || !picture) return;
  const int bytes = d->bpp / 8;
  for (int y = 0; y < ZK_H; y++) {
    const uint8_t *src = picture + 4 * y * ZK_W;
    for (int x = 0; x < ZK_W; x++, src += 4) {
      int fx, fy;
      switch (d->rotate) {
        case 90: fx = ZK_H - 1 - y; fy = x; break;
        case 180: fx = ZK_W - 1 - x; fy = ZK_H - 1 - y; break;
        case 270: fx = y; fy = ZK_W - 1 - x; break;
        default: fx = x; fy = y;
      }
      if (fx >= d->xres || fy >= d->yres) continue;
      unsigned char *o = d->back + fy * d->line + fx * bytes;
      const unsigned v = pack_px(d, src);
      if (bytes == 2) {
        if (d->swap) { o[0] = v >> 8; o[1] = v; } else { o[0] = v; o[1] = v >> 8; }
      } else if (bytes == 4) {
        o[0] = v; o[1] = v >> 8; o[2] = v >> 16; o[3] = 0xff;
      } else {
        o[0] = v; o[1] = v >> 8; o[2] = v >> 16;
      }
    }
  }
  flush(d);
}

void zk_display_close(zk_display *d) {
  if (!d) return;
  if (d->back) { memset(d->back, 0, d->size); flush(d); free(d->back); }
  if (d->map) munmap(d->map, d->size);
  close(d->fd);
  free(d);
}
