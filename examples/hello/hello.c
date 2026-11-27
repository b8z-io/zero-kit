// hello: the smallest Zero app. A square bounces over a slowly shifting gradient, with coloured corner marks so
// you can tell on the LCD which way up the Picture is (red top left, green top right, blue bottom left, white
// bottom right).
//
//   hello [shared zero-kit flags]      e.g. hello --out frames --frames 60 --every 15
#include <stdint.h>
#include <string.h>

#include "zk/app.h"

static void fill(uint8_t *pic, int x0, int y0, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
  for (int y = y0; y < y0 + h; y++)
    for (int x = x0; x < x0 + w; x++) {
      if (x < 0 || y < 0 || x >= ZK_W || y >= ZK_H) continue;
      uint8_t *p = pic + 4 * (y * ZK_W + x);
      p[0] = r; p[1] = g; p[2] = b; p[3] = 255;
    }
}

int main(int argc, char **argv) {
  zk_app *app = zk_app_init("hello", &argc, argv, NULL);
  static uint8_t pic[ZK_PICTURE_BYTES];
  float x = 20, y = 30, vx = 90, vy = 60;   // the square, in pixels and pixels per second
  const int size = 36;

  while (zk_frame_begin(app)) {
    const float dt = zk_app_dt(app);
    x += vx * dt; y += vy * dt;
    if (x < 0) { x = -x; vx = -vx; }
    if (y < 0) { y = -y; vy = -vy; }
    if (x > ZK_W - size) { x = 2 * (ZK_W - size) - x; vx = -vx; }
    if (y > ZK_H - size) { y = 2 * (ZK_H - size) - y; vy = -vy; }

    const int shift = (int)(zk_app_time(app) * 40) & 255;
    for (int py = 0; py < ZK_H; py++)
      for (int px = 0; px < ZK_W; px++) {
        uint8_t *p = pic + 4 * (py * ZK_W + px);
        p[0] = (uint8_t)(px * 255 / (ZK_W - 1) / 3);
        p[1] = (uint8_t)((py * 255 / (ZK_H - 1) + shift) & 255) / 4;
        p[2] = 80 + (uint8_t)(py * 120 / (ZK_H - 1));
        p[3] = 255;
      }
    fill(pic, (int)x, (int)y, size, size, 255, 200, 40);
    fill(pic, 0, 0, 12, 12, 255, 0, 0);
    fill(pic, ZK_W - 12, 0, 12, 12, 0, 255, 0);
    fill(pic, 0, ZK_H - 12, 12, 12, 0, 0, 255);
    fill(pic, ZK_W - 12, ZK_H - 12, 12, 12, 255, 255, 255);
    zk_frame_end(app, pic);
  }
  zk_app_close(app);
  return 0;
}
