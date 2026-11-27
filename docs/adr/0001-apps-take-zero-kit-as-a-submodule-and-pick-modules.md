# Apps take Zero Kit as a git submodule and compile in the Modules they name

Each App adds Zero Kit as a git submodule pinned to a tag, includes its `zero-kit.mk`, and names the Modules it wants (e.g. `ZK_MODULES = display keys font capture`). The .mk compiles those sources into the App's own build and adds each Module's link flags; there is no prebuilt library and no .deb.

wake-zero and dome-zero carry vendored copies that have already drifted, so a vendored copy was rejected. A .deb was rejected because nothing on the device needs Zero Kit at run time (Apps ship one binary) and it would need an apt repo and version discipline for no gain. A static archive was rejected because the App would still have to know every Module's link flags, and a CPU-only App like dome-zero must be able to leave out EGL/GBM entirely.

Consequence: bumping Zero Kit is a deliberate one-line commit in each App, and CI checks out with submodules.
