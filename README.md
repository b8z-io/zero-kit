# Zero Kit

The shared C library behind the Zero apps for the M5Stack CardputerZero.

**Status: early.** The App loop and the LCD work headless; nothing to install on the device yet.

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

See [examples/hello](examples/hello) for the loop. `make test` builds hello and runs the checks; after an
intended change to what hello draws, `make update-expected` rewrites the stored frames in
[tests/expected/hello](tests/expected/hello) (look at them before committing).

Part of a family of apps for the [M5Stack CardputerZero](https://docs.m5stack.com/en/CardputerZero), alongside
[Wake Zero](https://github.com/b8z-io/wake-zero) and [Dome Zero](https://github.com/b8z-io/dome-zero).

Built with [Claude](https://claude.ai) (Anthropic's AI): I come up with it, direct it and test it on the hardware;
Claude writes most of the code.

## Support

If you enjoy it: [Buy Me a Coffee](https://buymeacoffee.com/batesyboy) · [GitHub Sponsors](https://github.com/sponsors/b8z-io)

## Licence

MIT. See [LICENSE](LICENSE).
