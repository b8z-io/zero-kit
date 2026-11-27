#!/usr/bin/env python3
"""Compare a Headless run's frames with the stored expected frames (Python standard library only).

  frames.py check  RUN_DIR EXPECTED_DIR [DIFF_DIR]   exit 1 if any frame differs, is missing or is extra
  frames.py update RUN_DIR EXPECTED_DIR              replace the expected frames with this run's

Runs write PPM; expected frames are stored as PNG so they show as pictures on GitHub. Frames are compared pixel
by pixel. For a frame that differs, DIFF_DIR gets the new frame and a picture of where it differs.
"""
import os
import struct
import sys
import zlib


def read_ppm(path):
    with open(path, "rb") as f:
        data = f.read()
    parts, pos = [], 0
    while len(parts) < 4:
        while data[pos:pos + 1].isspace():
            pos += 1
        if data[pos:pos + 1] == b"#":
            pos = data.index(b"\n", pos) + 1
            continue
        end = pos
        while not data[end:end + 1].isspace():
            end += 1
        parts.append(data[pos:end])
        pos = end
    if parts[0] != b"P6" or parts[3] != b"255":
        raise ValueError(f"{path}: not an 8-bit P6 PPM")
    w, h = int(parts[1]), int(parts[2])
    return w, h, data[pos + 1:pos + 1 + w * h * 3]


def write_png(path, w, h, rgb):
    raw = b"".join(b"\x00" + rgb[y * w * 3:(y + 1) * w * 3] for y in range(h))

    def chunk(kind, body):
        return struct.pack(">I", len(body)) + kind + body + struct.pack(">I", zlib.crc32(kind + body) & 0xFFFFFFFF)

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
                + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def read_png(path):
    with open(path, "rb") as f:
        data = f.read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"{path}: not a PNG")
    pos, idat, w = 8, b"", None
    while pos < len(data):
        n, = struct.unpack(">I", data[pos:pos + 4])
        kind, body = data[pos + 4:pos + 8], data[pos + 8:pos + 8 + n]
        if kind == b"IHDR":
            w, h, depth, ctype, _, _, interlace = struct.unpack(">IIBBBBB", body)
            if depth != 8 or ctype != 2 or interlace:
                raise ValueError(f"{path}: only 8-bit RGB non-interlaced PNGs are read")
        elif kind == b"IDAT":
            idat += body
        pos += 12 + n
    raw, stride, out, prev = zlib.decompress(idat), w * 3, bytearray(), bytearray(w * 3)
    for y in range(h):
        ftype, line = raw[y * (stride + 1)], bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for i in range(stride):
            a = line[i - 3] if i >= 3 else 0
            b = prev[i]
            c = prev[i - 3] if i >= 3 else 0
            if ftype == 1:
                line[i] = (line[i] + a) & 255
            elif ftype == 2:
                line[i] = (line[i] + b) & 255
            elif ftype == 3:
                line[i] = (line[i] + (a + b) // 2) & 255
            elif ftype == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[i] = (line[i] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 255
        out += line
        prev = line
    return w, h, bytes(out)


def frames(d, ext):
    return sorted(f[:-len(ext)] for f in os.listdir(d) if f.endswith(ext)) if os.path.isdir(d) else []


def main():
    if len(sys.argv) < 4 or sys.argv[1] not in ("check", "update"):
        sys.exit(__doc__)
    cmd, run, expected = sys.argv[1:4]
    got = frames(run, ".ppm")
    if not got:
        sys.exit(f"no frames in {run}")
    if cmd == "update":
        os.makedirs(expected, exist_ok=True)
        for name in frames(expected, ".png"):
            os.remove(os.path.join(expected, name + ".png"))
        for name in got:
            write_png(os.path.join(expected, name + ".png"), *read_ppm(os.path.join(run, name + ".ppm")))
        print(f"wrote {len(got)} expected frames to {expected}")
        return
    diff_dir = sys.argv[4] if len(sys.argv) > 4 else None
    want, bad = frames(expected, ".png"), 0
    for name in sorted(set(got) | set(want)):
        if name not in want:
            print(f"EXTRA   {name}: the run made a frame that isn't expected")
            bad += 1
            continue
        if name not in got:
            print(f"MISSING {name}: an expected frame wasn't made")
            bad += 1
            continue
        w, h, a = read_ppm(os.path.join(run, name + ".ppm"))
        ew, eh, b = read_png(os.path.join(expected, name + ".png"))
        if (w, h) != (ew, eh):
            print(f"DIFFERS {name}: {w}x{h}, expected {ew}x{eh}")
            bad += 1
            continue
        if a == b:
            print(f"same    {name}")
            continue
        changed = sum(1 for i in range(0, len(a), 3) if a[i:i + 3] != b[i:i + 3])
        print(f"DIFFERS {name}: {changed} of {w * h} pixels")
        bad += 1
        if diff_dir:
            os.makedirs(diff_dir, exist_ok=True)
            write_png(os.path.join(diff_dir, name + "-new.png"), w, h, a)
            mask = bytes(v for i in range(0, len(a), 3) for v in ((255, 0, 255) if a[i:i + 3] != b[i:i + 3] else (b[i] // 4,) * 3))
            write_png(os.path.join(diff_dir, name + "-where.png"), w, h, mask)
    if bad:
        sys.exit(f"{bad} frame(s) not as expected. If the change is intended: make update-expected, look at the "
                 f"new frames, and commit them.")


if __name__ == "__main__":
    main()
