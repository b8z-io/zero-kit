# Zero Kit

The shared C library behind the Zero apps for the M5Stack CardputerZero.

**Status: early.** The App loop and the LCD work headless; the example App, hello, installs on the device.

zero-kit is the shared base every new Zero app builds on: small Modules (display, keys, gfx, camera, …) that an App
takes as a git submodule and compiles in, choosing only the ones it needs. The App keeps its own loop; zero-kit
handles the edges: shared flags, the LCD, pacing and Headless runs. The words used here are defined in
[CONTEXT.md](CONTEXT.md); the decisions behind them are in [docs/adr](docs/adr); the plan is
[issue #1](https://github.com/b8z-io/zero-kit/issues/1).

## What works

| Module | What it does | Checked headless (CI) | Checked on the device |
|---|---|---|---|
| app | Shared flags, frame begin/end, pacing, Headless runs | yes | not yet |
| display | Picture to the LCD framebuffer, rotation, byte swap | only "no screen" | not yet (the code is Dome Zero's, which works) |

## Using it in an App

```make
ZK_MODULES = app display
include zero-kit/zero-kit.mk

myapp: myapp.c $(ZK_SRC) $(ZK_HDR)
	$(CC) $(CFLAGS) $(ZK_CFLAGS) -o $@ myapp.c $(ZK_SRC) $(ZK_LDLIBS)
```

See [examples/hello](examples/hello) for the loop.

An App's CI is a few lines calling zero-kit's reusable workflow, pinned to the same tag as its zero-kit submodule;
see the top of [.github/workflows/app.yml](.github/workflows/app.yml). An App's `package/` folder holds only
`app.conf` (its name, launcher text and run-time Debian packages) and its 100x100 and 80x80 icons; the release adds
zero-kit's [install.sh](package/install.sh), which puts it in the launcher. `make test` builds hello and runs the checks; after an
intended change to what hello draws, `make update-expected` rewrites the stored frames in
[tests/expected/hello](tests/expected/hello) (look at them before committing).

Part of a family of apps for the [M5Stack CardputerZero](https://docs.m5stack.com/en/CardputerZero), alongside
[Wake Zero](https://github.com/b8z-io/wake-zero) and [Dome Zero](https://github.com/b8z-io/dome-zero).

Built with [Claude](https://claude.ai) (Anthropic's AI): I come up with it, direct it and test it on the hardware;
Claude writes most of the code.

## Try hello on the CardputerZero

From your computer (change `cardputerzero.local` if your device has another name or address):

```bash
ssh -t pi@cardputerzero.local 'rm -rf /tmp/hello && mkdir /tmp/hello && curl -fsSL https://github.com/b8z-io/zero-kit/releases/download/latest/hello.tar.gz | tar -xz -C /tmp/hello && sudo /tmp/hello/install.sh'
```

Then open **Hello Zero** in the launcher: a yellow square bounces over a blue gradient, with red, green, blue and
white marks in the top-left, top-right, bottom-left and bottom-right corners. Hold Esc to leave it (keys come in a
later version). To take it out again: `sudo /tmp/hello/install.sh --remove` on the device.

## Support

If you enjoy it: [Buy Me a Coffee](https://buymeacoffee.com/batesyboy) · [GitHub Sponsors](https://github.com/sponsors/b8z-io)

## Licence

MIT. See [LICENSE](LICENSE).
