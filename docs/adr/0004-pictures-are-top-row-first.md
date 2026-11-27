# Pictures are RGBA with the top row first

A Picture handed to Zero Kit is 320x170 RGBA, top row first. The gfx Module flips once after `glReadPixels` (which gives the bottom row first).

wake-zero and dome-zero pass bottom-first buffers, following GL, and even the CPU-rendered dome-zero copied that order. Everything downstream (PNG, PPM, ffmpeg, NDI, the framebuffer) wants top-first, and a CPU App drawing bottom-first is an upside-down bug waiting to happen. One flip of 320x170 a frame is negligible. Code ported from the reference apps has to drop its own flips.
