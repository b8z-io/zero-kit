#!/bin/sh
# zero-kit's checks, run by CI (and `make test`). Each prints what it checked; any failure stops with status 1.
#   UPDATE=1 tests/run.sh   rewrite the stored expected frames instead of comparing (then look at them!)
set -eu
cd "$(dirname "$0")/.."
HELLO=examples/hello/hello
T=test
rm -rf "$T" && mkdir -p "$T"
fail() { echo "FAIL: $*" >&2; exit 1; }
pass() { echo "ok:   $*"; }

# 1. A Headless run writes exactly N/K frames, and matches the stored expected frames.
"$HELLO" --out "$T/run1" --frames 60 --every 15
[ "$(ls "$T/run1" | wc -l)" -eq 4 ] || fail "expected 4 frames from --frames 60 --every 15, got $(ls "$T/run1" | wc -l)"
pass "Headless run wrote 4 frames"
if [ "${UPDATE:-}" = 1 ]; then
  python3 tools/frames.py update "$T/run1" tests/expected/hello
else
  python3 tools/frames.py check "$T/run1" tests/expected/hello "$T/diff" || fail "frames differ from tests/expected/hello (see $T/diff)"
  pass "frames match the stored expected frames"
fi

# 2. Two runs give byte-identical frames (ADR 0007).
"$HELLO" --out "$T/run2" --frames 60 --every 15
for f in "$T"/run1/*.ppm; do cmp -s "$f" "$T/run2/$(basename "$f")" || fail "$(basename "$f") differs between two runs"; done
pass "two runs are identical"

# 3. hello links nothing it doesn't use: no EGL, GLES, GBM or ALSA.
if command -v readelf >/dev/null; then
  if readelf -d "$HELLO" | grep -Ei 'NEEDED.*(EGL|GLES|gbm|asound)'; then fail "hello links GPU or sound libraries"; fi
  pass "hello links no EGL/GLES/GBM/ALSA"
fi

# 4. Bad flags print usage and exit 2; --help exits 0.
set +e
"$HELLO" --frames nope >"$T/bad.txt" 2>&1; st=$?
set -e
[ $st -eq 2 ] && grep -q '^usage:' "$T/bad.txt" || fail "a bad flag should print usage and exit 2 (got $st)"
"$HELLO" --help | grep -q '^usage:' || fail "--help should print usage"
pass "bad flags print usage and exit 2; --help works"

# 5. No screen: the App says why and carries on (ADR 0006).
"$HELLO" --fb /dev/null --frames 3 2>"$T/noscreen.txt" || fail "hello should run on without a screen"
grep -q "no screen" "$T/noscreen.txt" || fail "hello should say there's no screen"
pass "no screen: says so and carries on"

# 6. No Module exits or aborts; only app.c may exit, for bad flags or --help at init (ADR 0006).
if grep -n -E '\b(abort|_exit)\(' src/*.c; then fail "a Module calls abort/_exit"; fi
if grep -l -E '\bexit\(' src/*.c | grep -v '^src/app.c$'; then fail "a Module other than app calls exit"; fi
pass "no Module exits, apart from app's usage at init"

echo "all checks passed"
