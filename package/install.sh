#!/bin/sh
# zero-kit's installer for a Zero app, shipped in every App's release tarball next to app.conf, the binary and
# its icons. Puts the App in the CardputerZero's launcher. Run on the device from the unpacked folder:
#   sudo ./install.sh            install (or update)
#   sudo ./install.sh --remove   take it out of the launcher again
set -e
cd "$(dirname "$0")"
[ "$(id -u)" = 0 ] || { echo "run with sudo: sudo $0 $*"; exit 1; }
[ -f app.conf ] || { echo "app.conf is missing: run this from the unpacked release folder"; exit 1; }
# app.conf sets APP (the binary's name), NAME (shown in the launcher), COMMENT, and PACKAGES (Debian packages the
# App needs at run time; may be empty).
APP= NAME= COMMENT= PACKAGES=
. ./app.conf
[ -n "$APP" ] && [ -n "$NAME" ] || { echo "app.conf must set APP and NAME"; exit 1; }
L=/usr/share/APPLaunch

if [ "${1:-}" = "--remove" ]; then
  rm -f "$L/bin/$APP" "$L/applications/$APP.desktop" "$L/share/images/${APP}_100.png" "$L/share/images/${APP}_80.png"
  echo "removed $NAME from the launcher (its settings in ~/.config/$APP are left alone)"
  exit 0
fi

need=""
for p in $PACKAGES; do dpkg -s "$p" >/dev/null 2>&1 || need="$need $p"; done
if [ -n "$need" ]; then
  echo "installing:$need"
  apt-get update -q && apt-get install -y -q --no-install-recommends $need
fi

install -D -m 755 "$APP" "$L/bin/$APP"
install -D -m 644 "${APP}_100.png" "$L/share/images/${APP}_100.png"
install -D -m 644 "${APP}_80.png" "$L/share/images/${APP}_80.png"
mkdir -p "$L/applications"
cat > "$L/applications/$APP.desktop" <<DESKTOP
[Desktop Entry]
Name=$NAME
Comment=$COMMENT
Exec=$L/bin/$APP
Terminal=false
Icon=share/images/${APP}_100.png
Type=Application
DESKTOP

# The launcher runs Apps as its user (pi unless changed): it needs the GPU, the screen and the keyboard.
user=${SUDO_USER:-pi}
if id "$user" >/dev/null 2>&1; then
  for g in video render input; do
    if getent group "$g" >/dev/null && ! id -nG "$user" | grep -qw "$g"; then
      usermod -aG "$g" "$user" && echo "added $user to $g (takes effect after a reboot)"
    fi
  done
fi
echo "installed: $NAME is in the launcher. To try it over SSH instead: $L/bin/$APP (Q or Esc quits)"
