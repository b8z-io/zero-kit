# Apps call Zero Kit's reusable CI workflow instead of copying one

Zero Kit publishes a GitHub reusable workflow (`workflow_call`) that does the headless checks, the arm64 build in `debian:trixie` under qemu and the `latest` release tarball, plus `install.sh` / `install-ndi.sh` templates filled in from the App's name. Each App's workflow is a few lines that call it, pinned to the same tag as its submodule.

The original plan was for Apps to copy a workflow. Copies drift, exactly as the C sources in wake-zero and dome-zero did, so one maintained copy wins.
