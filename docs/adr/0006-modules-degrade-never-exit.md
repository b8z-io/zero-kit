# Modules report failure and let the App carry on; they never exit

No Zero Kit Module calls `exit` or `abort`. Opening something that isn't there (no camera, no IMU, no NDI library, no HDMI) returns NULL or -1 with a status string the App can show on the HUD, and the App keeps running with less. The only exception is bad command-line flags at init, which print usage and exit.

On a pocket device an App that dies drops silently back to the launcher with no explanation; one that says "CAMERA OFF" tells the person what's wrong. wake-zero already treats its camera this way.
