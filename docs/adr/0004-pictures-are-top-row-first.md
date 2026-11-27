# Pictures are RGBA with the top row first

A Picture handed to Zero Kit is 320x170 RGBA, top row first. The gfx Module flips once after `glReadPixels` (which gives the bottom row first).

wake-zero and dome-zero pass bottom-first buffers, following GL, and even the CPU-rendered dome-zero copied that order. Everything downstream (PNG, PPM, ffmpeg, NDI, the framebuffer) wants top-first, and a CPU App drawing bottom-first is an upside-down bug waiting to happen. One flip of 320x170 a frame is negligible. Code ported from the reference apps has to drop its own flips.

## Exception: full-size Test patterns

ndi-zero's sender sends a Test pattern at a real Format (up to 1080p60, if the device can keep up), not the Picture scaled up. The testpattern Module draws a chosen pattern at any size; the sender Output carries that, and the Screens show a 320x170 version as the Picture. Still patterns are drawn once per Format; moving ones redraw only what moves. This is the one case where an Output carries something other than the Picture, and it doesn't break ADR 0003: a name or counter burned into a Test pattern is content, not HUD.
