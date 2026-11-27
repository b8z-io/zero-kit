# The HUD reaches Screens only, never captures or network Outputs

Zero Kit keeps the Picture and the HUD separate all the way to the Outputs. Snapshots, recordings, streams, NDI and Headless frame files get the clean Picture; only Screens compose the HUD on top. On HDMI the HUD is off by default, since HDMI is usually for an audience (a projector, a monitor).

This is a rule in the kit, not a convention each App follows, so no App can leak labels into what it sends out.

Two choices make the rule structural. The App asks for HUD items (text, dots, the Flash) instead of drawing into a buffer, so it never holds anything that could reach a capture. And Zero Kit owns the list of active Outputs (recording, stream, NDI send, Headless frames) and feeds them all from frame-end; the App only starts and stops them, so nothing goes out except through that one path.
