# Zero Kit

The shared base for the Zero apps on the M5Stack CardputerZero: everything an app needs that isn't its own idea.

This glossary is the shared kernel of the Zero family. Every App's own `CONTEXT.md` uses these terms as defined here and adds only the words for its own idea; a term every App would share is changed here, not in an App.

## Language

**App**:
A program for the CardputerZero built on Zero Kit (ndi-zero, timecam-zero, vj-zero).
_Avoid_: program, project

**Module**:
One self-contained part of Zero Kit (display, keys, gfx, font, camera, imu, audio, capture, ndi, app) that an App chooses to use or leave out.
_Avoid_: component, library, package

**Picture**:
The clean image an App makes each frame: what the App is *for*, with nothing drawn over it.
_Avoid_: frame (ambiguous with a tick of the loop), image, output

**HUD**:
Everything drawn over the Picture to tell the person holding the device something: labels, readouts, the Flash, indicator dots.
_Avoid_: overlay, UI, OSD

**Flash**:
A short message shown on the HUD for a moment after something happens ("SAVED", "PAUSED").
_Avoid_: toast, notification

**Output**:
Anywhere a Picture goes: a Screen, a headless frame file, a snapshot, a recording, a stream, NDI send.
_Avoid_: sink, target (that word is taken by render targets in gfx)

**Test pattern**:
A known image made to check a video chain, drawn at any size: still (bars, grid, ramp) or moving (a marker, a counter). It is the content itself, so a name or clock burned into it is not HUD.
_Avoid_: test card, test signal, slate

**Format**:
The size and frame rate a full-size Output is sent at (e.g. 1080p30, 720p60).
_Avoid_: mode, resolution (on its own)

**Screen**:
An Output a person looks at live: the LCD, and (once proven) HDMI. Only Screens may show the HUD.
_Avoid_: display (that's the Module's name), monitor

**Fake**:
A stand-in for an input Module (camera, IMU, mic, NDI receive, keys) that behaves the same way every run, so a Headless run is repeatable.
_Avoid_: mock, stub, simulator

**Standard keys**:
The keys every App answers the same way: S snapshot, V record on/off, Q or Esc quit, D the Debug readout. An App may switch any of them off.
_Avoid_: hotkeys, global keys

**Debug readout**:
The HUD line D toggles: frame rate, active Outputs, renderer, battery, plus whatever the App adds.
_Avoid_: info line, stats

**Settings**:
The few values an App remembers between runs, kept per App in the person's home directory.
_Avoid_: config, preferences

**Headless run**:
Running an App with no device attached, writing Pictures to files instead of a Screen, with fake inputs where it needs them.
_Avoid_: test mode, offline mode

## Relationships

- An **App** uses a chosen set of **Modules**.
- Each frame an **App** makes one **Picture**; every enabled **Output** receives it.
- A full-size **Output** (NDI send of a **Test pattern**) may carry something other than the **Picture**, at its own **Format**; the **Screens** then show a 320x170 version as the **Picture**.
- The **HUD** is composed onto **Screens** only. The LCD shows it by default; HDMI shows the clean **Picture** unless the App says otherwise.
- **Zero Kit** decides which **Outputs** are on and feeds them all; an **App** only asks for them to start or stop.
- The **App** never draws on a **Screen** directly: it asks for **HUD** items and **Zero Kit** draws them.
- A **Headless run** replaces the **Screens** with frame files and every input with its **Fake**, and always gives the same Pictures.

## Flagged ambiguities

- "frame" meant both the image and the loop tick. Resolved: the image is the **Picture**; "frame" means one pass of the loop.
