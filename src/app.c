// zero-kit app: shared flags, frame begin/end, pacing and Headless runs.
#include "zk/app.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

struct zk_app {
  const char *name, *usage, *out_dir, *fb_path;
  int fps, fps_given, frames_max, every, rotate, swap, running, out_failed;
  long frame;
  float dt;
  double t, last;
  double next;   // when the next frame is due (seconds, monotonic)
  zk_display *lcd;
  char screen_status[200];
};

static volatile sig_atomic_t stop_requested;
static void on_signal(int sig) { (void)sig; stop_requested = 1; }

static double now(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void print_usage(FILE *f, const zk_app *app) {
  fprintf(f,
          "usage: %s [--fb PATH] [--rotate 0|90|180|270] [--swap] [--fps N]\n"
          "       %*s [--out DIR [--every K]] [--frames N]%s%s\n",
          app->name, (int)strlen(app->name), "", app->usage ? " " : "", app->usage ? app->usage : "");
}

static void usage_error(const zk_app *app, const char *what, const char *flag) {
  fprintf(stderr, "%s: %s %s\n", app->name, what, flag);
  print_usage(stderr, app);
  exit(2);   // the one place zero-kit exits: the App was started wrongly (ADR 0006)
}

static int int_arg(const zk_app *app, const char *flag, const char *s, int lo, int hi) {
  char *end;
  errno = 0;
  long v = s ? strtol(s, &end, 10) : 0;
  if (!s || errno || end == s || *end || v < lo || v > hi) usage_error(app, "bad or missing value for", flag);
  return (int)v;
}

zk_app *zk_app_init(const char *name, int *argc, char **argv, const char *app_usage) {
  zk_app *app = calloc(1, sizeof *app);
  if (!app) { fprintf(stderr, "%s: out of memory\n", name); exit(1); }
  app->name = name;
  app->usage = app_usage;
  app->fb_path = "/dev/fb_lcd";
  app->fps = 30;
  app->every = 1;
  app->rotate = -1;
  app->running = 1;

  int kept = 1;
  for (int i = 1; i < *argc; i++) {
    const char *a = argv[i], *v = i + 1 < *argc ? argv[i + 1] : NULL;
    if (!strcmp(a, "--help") || !strcmp(a, "-h")) { print_usage(stdout, app); exit(0); }
    else if (!strcmp(a, "--fb")) { if (!v) usage_error(app, "missing value for", a); app->fb_path = v; i++; }
    else if (!strcmp(a, "--rotate")) {
      app->rotate = int_arg(app, a, v, 0, 270); i++;
      if (app->rotate % 90) usage_error(app, "rotate must be 0, 90, 180 or 270, not", v);
    }
    else if (!strcmp(a, "--swap")) app->swap = 1;
    else if (!strcmp(a, "--fps")) { app->fps = int_arg(app, a, v, 1, 240); app->fps_given = 1; i++; }
    else if (!strcmp(a, "--out")) { if (!v) usage_error(app, "missing value for", a); app->out_dir = v; i++; }
    else if (!strcmp(a, "--frames")) { app->frames_max = int_arg(app, a, v, 1, 100000000); i++; }
    else if (!strcmp(a, "--every")) { app->every = int_arg(app, a, v, 1, 100000000); i++; }
    else argv[kept++] = argv[i];   // the App's own
  }
  *argc = kept;
  argv[kept] = NULL;

  struct sigaction sa = { 0 };
  sa.sa_handler = on_signal;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  if (app->out_dir) {
    if (mkdir(app->out_dir, 0755) < 0 && errno != EEXIST)
      fprintf(stderr, "%s: can't make %s: %s\n", name, app->out_dir, strerror(errno));
    snprintf(app->screen_status, sizeof app->screen_status, "headless: Pictures to %s", app->out_dir);
  } else {
    char err[160];
    app->lcd = zk_display_open(app->fb_path, app->rotate, app->swap, err, sizeof err);
    if (app->lcd) snprintf(app->screen_status, sizeof app->screen_status, "%s", zk_display_describe(app->lcd));
    else snprintf(app->screen_status, sizeof app->screen_status, "no screen: %s", err);
    fprintf(stderr, "%s: %s\n", name, app->screen_status);
  }
  return app;
}

void zk_app_set_fps(zk_app *app, int fps) {
  if (!app->fps_given && fps >= 1 && fps <= 240) app->fps = fps;
}

int zk_frame_begin(zk_app *app) {
  if (stop_requested) app->running = 0;
  if (!app->running) return 0;
  const double step = 1.0 / app->fps;
  if (app->frame == 0) {
    app->dt = (float)step;
    app->last = now();
    app->next = app->last;
  } else if (app->out_dir) {
    app->dt = (float)step;   // Headless: the time step is fixed, so runs repeat exactly
    app->t += step;
  } else {
    const double n = now();
    double dt = n - app->last;
    if (dt > 0.25) dt = 0.25;   // after a stall, don't jump the App forward by seconds
    app->dt = (float)dt;
    app->t += dt;
    app->last = n;
  }
  return 1;
}

static void write_ppm(zk_app *app, long n, const uint8_t *picture) {
  char path[4096];
  snprintf(path, sizeof path, "%s/frame_%05ld.ppm", app->out_dir, n);
  FILE *f = fopen(path, "wb");
  if (f) {
    fprintf(f, "P6\n%d %d\n255\n", ZK_W, ZK_H);
    uint8_t row[ZK_W * 3];
    for (int y = 0; y < ZK_H; y++) {
      const uint8_t *s = picture + 4 * y * ZK_W;
      for (int x = 0; x < ZK_W; x++) { row[3 * x] = s[4 * x]; row[3 * x + 1] = s[4 * x + 1]; row[3 * x + 2] = s[4 * x + 2]; }
      fwrite(row, 1, sizeof row, f);
    }
    if (fclose(f) == 0) return;
  }
  if (!app->out_failed) fprintf(stderr, "%s: can't write %s: %s\n", app->name, path, strerror(errno));
  app->out_failed = 1;
}

void zk_frame_end(zk_app *app, const uint8_t *picture) {
  if (picture) {
    if (app->out_dir) {
      if (app->frame % app->every == 0) write_ppm(app, app->frame / app->every, picture);
    } else if (app->lcd) {
      zk_display_show(app->lcd, picture);
    }
  }
  app->frame++;
  if (app->frames_max && app->frame >= app->frames_max) app->running = 0;
  if (app->out_dir || !app->running) return;

  // Pace to the target rate: sleep until the next frame is due; if we're more than a frame late, stop catching up.
  app->next += 1.0 / app->fps;
  const double n = now();
  if (app->next > n) {
    const double left = app->next - n;
    struct timespec ts = { (time_t)left, (long)((left - (time_t)left) * 1e9) };
    while (nanosleep(&ts, &ts) < 0 && errno == EINTR && !stop_requested) {}
  } else if (n - app->next > 1.0 / app->fps) {
    app->next = n;
  }
}

float zk_app_dt(const zk_app *app) { return app->dt; }
double zk_app_time(const zk_app *app) { return app->t; }
long zk_app_frame(const zk_app *app) { return app->frame; }
int zk_app_headless(const zk_app *app) { return app->out_dir != NULL; }
const char *zk_app_name(const zk_app *app) { return app->name; }
const char *zk_app_screen_status(const zk_app *app) { return app->screen_status; }
void zk_app_quit(zk_app *app) { app->running = 0; }

void zk_app_close(zk_app *app) {
  if (!app) return;
  zk_display_close(app->lcd);
  free(app);
}
