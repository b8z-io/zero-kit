# Zero Kit

The shared base for the Zero apps on the M5Stack CardputerZero: everything an app needs that isn't its own idea.

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

**Screen**:
An Output a person looks at live: the LCD, and (once proven) HDMI. Only Screens may show the HUD.
_Avoid_: display (that's the Module's name), monitor

**Headless run**:
Running an App with no device attached, writing Pictures to files instead of a Screen, with fake inputs where it needs them.
_Avoid_: test mode, offline mode

## Relationships

- An **App** uses a chosen set of **Modules**.
- Each frame an **App** makes one **Picture**; every enabled **Output** receives it.
- The **HUD** is composed onto **Screens** only. The LCD shows it by default; HDMI shows the clean **Picture** unless the App says otherwise.
- A **Headless run** replaces the **Screens** with frame files.

## Flagged ambiguities

- "frame" meant both the image and the loop tick. Resolved: the image is the **Picture**; "frame" means one pass of the loop.
