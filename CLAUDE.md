# Zero Kit: notes for Claude

The shared C library behind the Zero apps for the M5Stack CardputerZero.

**Where to start:** Run `/grill-with-docs` on the zero-kit design (the open questions above), then `/to-spec` and `/to-tickets`.

## This repo: zero-kit

The shared base for the CardputerZero apps, so each app is only its own idea. Pulled out of wake-zero and
dome-zero (which currently carry copies), then extended for ndi-zero, timecam-zero and vj-zero.

What belongs here (each a small module with a narrow interface):

- **display**: the LCD (`/dev/fb_lcd`, rotation, RGB565), and **HDMI** output through DRM/KMS (new, unproven).
- **keys**: the keyboard (evdev, held-key state, the Fn arrows).
- **gfx**: EGL/GBM OpenGL ES 2 context, render-to-texture, readback; the 8-bit-target helpers (dither, 16-bit packing).
- **font**: the 5x7 HUD font with a halo.
- **camera**: rpicam-vid on a pipe, plus a fake camera for headless runs.
- **imu**: BMI270/BMM150 over IIO with orientation fusion.
- **audio**: ALSA out (with a playback clock) and **in** (the mic; new).
- **capture**: PNG snapshots, MP4 recording, UDP stream.
- **ndi**: NDI send (from wake-zero) and **receive** (new, for ndi-zero), loaded at run time.
- **app**: the main loop shape every app shares (frame pacing, headless `--out` mode, flash messages, settings in
  `~/.config`), and **packaging**: install.sh / install-ndi.sh / deploy, .desktop and icon conventions, a CI
  workflow the apps copy.

Open questions for `/grill-with-docs`: how apps consume it (git submodule vs. vendored copy vs. a .deb); one
static library or source files compiled in; what the app loop owns vs. the app; how HDMI and the LCD share a frame.

## The Zero family

- **zero-kit**: the shared C library every new Zero app uses (built first).
- **ndi-zero**: a pocket NDI® monitor. **timecam-zero**: a camera that photographs time. **vj-zero**: a pocket VJ
  instrument. All three build on zero-kit.
- **wake-zero** and **dome-zero** are already public and keep their own copies of the device code for now; they
  move onto zero-kit once the three new apps are done. Don't change them from here.

## The CardputerZero (what every Zero app runs on)

- **M5Stack CardputerZero**: Raspberry Pi CM0 (BCM2710A1, the Pi Zero 2 W's chip: 4x Cortex-A53 @ 1 GHz,
  512 MB). Debian 13 trixie, arm64 (Raspberry Pi OS). Apps run as user `pi`.
- **Screen**: 320x170 ST7789 at `/dev/fb_lcd` (RGB565; the panel may report portrait, so rotate; `--swap` for
  BGR). Write frames with `glReadPixels` -> convert -> `pwrite`.
- **GPU**: VideoCore IV, OpenGL ES 2.0 through Mesa vc4 on `/dev/dri/renderD128` (EGL + GBM, no window).
  **Only 8-bit render targets**: feedback needs dithered rounding; anything needing more precision packs 16-bit
  fixed point into two channels and filters by hand, or runs on the CPU with OpenMP (4 cores). Keep shaders
  simple; render at screen size and scale up with hard pixel edges.
- **Camera**: IMX219 via `rpicam-vid` (YUV420 on a pipe; give children `PR_SET_PDEATHSIG` or a killed app leaves
  the camera busy). Camera faces away from the screen.
- **Motion**: BMI270 accel/gyro + BMM150 magnetometer through Linux IIO sysfs. Dome Zero has a working Mahony
  fusion (`src/imu.c`).
- **Keyboard**: evdev `/dev/input/by-path/platform-3f804000.i2c-event`, shared with the launcher. Arrows are
  **Fn + F (up) / Z (left) / X (down) / C (right)**; plain F/Z/X/C should work too. `=` and `-` are hard to reach.
  Holding **Esc** makes the launcher kill the app.
- **Launcher (APPLaunch)**: `.desktop` files in `/usr/share/APPLaunch/applications`, `Exec=/usr/share/APPLaunch/bin/<app>`,
  icons 100x100 and 80x80 PNG in `/usr/share/APPLaunch/share/images`. The user needs groups video, render, input.
- **Also on board**: HDMI out, mic, speaker + 3.5 mm jack, IR TX/RX, Ethernet, Wi-Fi, RTC, battery gauge (BQ27220).
  HDMI output from our apps is not yet proven.
- **`/tmp` is a small RAM disk.** Never unpack big things there (use `/var/tmp`, or stream).

## How the Zero apps are built

- C, Makefile, only Debian packages (`libegl-dev libgles-dev libgbm-dev`, `libasound2-dev`, ...). No frameworks.
- Every app has a **headless mode** (`--out DIR --frames N --every K` writing PPM frames, a fake camera where it
  uses one) so it can be tested and screenshotted without the device: Mesa llvmpipe with `LIBGL_ALWAYS_SOFTWARE=1`.
- **CI (GitHub Actions)**: run the headless checks on ubuntu x86, build arm64 in `debian:trixie` under qemu,
  publish a `latest` release tarball (binary + `package/*`). Use Node 24 action versions (checkout/setup-node/
  upload-artifact/download-artifact v7, setup-qemu-action v4) and a concurrency group.
- **Install** is one line on the device, or over SSH from the user's laptop:
  `ssh -t pi@<device> 'rm -rf /tmp/<app> && mkdir /tmp/<app> && curl -fsSL https://github.com/b8z-io/<app>/releases/download/latest/<app>.tar.gz | tar -xz -C /tmp/<app> && sudo /tmp/<app>/install.sh'`
- **Capture**: snapshots are PNG scaled 4x nearest-neighbour; recordings are fragmented MP4 via ffmpeg (survive
  being cut off). Labels/HUD never go into captures or network output.
- **NDI®**: the NDI library is under NDI's licence and is never committed or shipped. Builds fetch the headers
  with `tools/fetch_ndi_sdk.sh` (git-ignored `ndi-sdk/`); on the device `install-ndi.sh` (with the user's consent)
  streams the SDK and keeps only the aarch64 `libndi.so.6` in `/usr/share/APPLaunch/lib/ndi/`. Load it with
  `dlopen` + `NDIlib_v6_load()`. Wherever NDI is mentioned: "NDI® is a registered trademark of Vizrt NDI AB" and a
  link to https://ndi.video/. Work networks often block NDI discovery (mDNS): Access Manager / extra IPs fix it.
- **Reference code** (copy from these, they work on the device): `b8z-io/wake-zero` (GLES, camera, analysis,
  capture, NDI send, installers, CI) and `b8z-io/dome-zero` (video decode, ALSA audio clock, IMU fusion).

## Working with the user (Batesy)

- He tests on the real device; Claude can't reach it. Give him **one command to paste on his laptop** (SSH to
  `pi@<device>`), never long typing on the Cardputer keyboard. Paste-safe: no `<placeholders>` that bash would
  treat as redirects.
- Verify before claiming: build, run headless, look at the frames. Say plainly what was and wasn't tested on the device.
- Git identity in this repo is local: `Andy Bates <35863400+b8z-io@users.noreply.github.com>`. Never change it.
  Commits end with the attribution lines the session gives you.
- READMEs say it was built with Claude (he directs and tests, Claude writes most of the code) and link his
  Buy Me a Coffee (`batesyboy`) and GitHub Sponsors (`b8z-io`).

## Agent skills

### Issue tracker

Issues and specs live in this repo's GitHub Issues (`gh` CLI). See `docs/agents/issue-tracker.md`.

### Triage labels

The default five: needs-triage, needs-info, ready-for-agent, ready-for-human, wontfix. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context: `CONTEXT.md` and `docs/adr/` at the repo root, created as decisions are made. See `docs/agents/domain.md`.
