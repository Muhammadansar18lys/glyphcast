#!/usr/bin/env python3
"""Glyphcast: turn images into text art, with optional 24-bit colour.

Uses only the standard library. Supported inputs are binary PPM (P6) and
24-bit uncompressed BMP. The algorithm matches the C++ core.
"""

from __future__ import annotations

import argparse
import math
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

RAMP_CLASSIC = " .:-=+*#%@"
RAMP_MINIMAL = " .:#"
RAMP_DENSE = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "
PRESET_RAMPS = {"classic": RAMP_CLASSIC, "minimal": RAMP_MINIMAL, "dense": RAMP_DENSE}

CELL_ASPECT = 0.5
RESET = "\x1b[0m"
_WHITESPACE = b" \t\r\n\x0b\x0c"


class ImageFormatError(ValueError):
    """Raised when an image file cannot be decoded."""


@dataclass(frozen=True)
class Image:
    """An immutable RGB raster, stored row by row from the top."""

    width: int
    height: int
    pixels: list[tuple[int, int, int]]

    def __post_init__(self) -> None:
        if self.width <= 0 or self.height <= 0:
            raise ValueError("image dimensions must be positive")
        if len(self.pixels) != self.width * self.height:
            raise ValueError("pixel count does not match image dimensions")

    def at(self, x: int, y: int) -> tuple[int, int, int]:
        return self.pixels[y * self.width + x]


def resolve_ramp(name_or_ramp: str) -> str:
    """Return the preset ramp for a name, or the input unchanged as a custom ramp."""
    return PRESET_RAMPS.get(name_or_ramp.lower(), name_or_ramp)


def luminance(r: float, g: float, b: float) -> float:
    """Relative luminance in [0, 1], using Rec. 709 weights."""
    return (0.2126 * r + 0.7152 * g + 0.0722 * b) / 255.0


def load_image(path: str | Path) -> Image:
    data = Path(path).read_bytes()
    if data[:2] == b"P6":
        return _load_ppm(data)
    if data[:2] == b"BM":
        return _load_bmp(data)
    raise ImageFormatError(f"unsupported image format (use binary PPM or 24-bit BMP): {path}")


def _skip_space_and_comments(data: bytes, pos: int) -> int:
    while pos < len(data):
        byte = data[pos:pos + 1]
        if byte in _WHITESPACE:
            pos += 1
        elif byte == b"#":
            while pos < len(data) and data[pos:pos + 1] != b"\n":
                pos += 1
        else:
            break
    return pos


def _read_ppm_int(data: bytes, pos: int) -> tuple[int, int]:
    pos = _skip_space_and_comments(data, pos)
    start = pos
    while pos < len(data) and data[pos:pos + 1].isdigit():
        pos += 1
    if pos == start:
        raise ImageFormatError("invalid PPM header")
    return int(data[start:pos]), pos


def _load_ppm(data: bytes) -> Image:
    width, pos = _read_ppm_int(data, 2)  # skip the "P6" magic number
    height, pos = _read_ppm_int(data, pos)
    maxval, pos = _read_ppm_int(data, pos)
    if maxval != 255:
        raise ImageFormatError("only 8-bit PPM (maxval 255) is supported")
    pos += 1  # exactly one whitespace byte separates the header from pixel data

    needed = width * height * 3
    if len(data) < pos + needed:
        raise ImageFormatError("PPM pixel data is truncated")
    pixels = [tuple(data[i:i + 3]) for i in range(pos, pos + needed, 3)]
    return Image(width, height, pixels)


def _load_bmp(data: bytes) -> Image:
    if len(data) < 54:
        raise ImageFormatError("BMP header is truncated")
    (offset,) = struct.unpack_from("<I", data, 10)
    width, height_raw = struct.unpack_from("<ii", data, 18)
    (bits_per_pixel,) = struct.unpack_from("<H", data, 28)
    (compression,) = struct.unpack_from("<I", data, 30)

    if bits_per_pixel != 24 or compression != 0:
        raise ImageFormatError("only uncompressed 24-bit BMP is supported")
    if width <= 0 or height_raw == 0:
        raise ImageFormatError("BMP has invalid dimensions")

    top_down = height_raw < 0
    height = abs(height_raw)
    stride = (width * 3 + 3) // 4 * 4  # rows are padded to 4-byte boundaries
    if len(data) < offset + stride * height:
        raise ImageFormatError("BMP pixel data is truncated")

    pixels: list[tuple[int, int, int]] = []
    for row in range(height):
        src_row = row if top_down else height - 1 - row  # BMP is usually bottom-up
        base = offset + stride * src_row
        line = data[base:base + width * 3]
        for x in range(width):
            blue, green, red = line[3 * x], line[3 * x + 1], line[3 * x + 2]
            pixels.append((red, green, blue))
    return Image(width, height, pixels)


def _average(image: Image, x0: int, y0: int, x1: int, y1: int) -> tuple[float, float, float]:
    r = g = b = 0
    count = 0
    for y in range(y0, y1):
        for x in range(x0, x1):
            pr, pg, pb = image.at(x, y)
            r += pr
            g += pg
            b += pb
            count += 1
    return r / count, g / count, b / count


def render(
    image: Image,
    width: int = 80,
    ramp: str = RAMP_CLASSIC,
    invert: bool = False,
    color: bool = False,
) -> str:
    """Convert an image to text art. Raises ValueError for invalid options."""
    if width <= 0:
        raise ValueError("width must be positive")
    if not ramp:
        raise ValueError("character ramp must not be empty")

    rows = max(1, math.floor(image.height / image.width * width * CELL_ASPECT + 0.5))
    levels = len(ramp)
    lines = []
    for row in range(rows):
        y0 = row * image.height // rows
        y1 = max(y0 + 1, (row + 1) * image.height // rows)
        parts = []
        for col in range(width):
            x0 = col * image.width // width
            x1 = max(x0 + 1, (col + 1) * image.width // width)
            r, g, b = _average(image, x0, y0, x1, y1)
            lum = luminance(r, g, b)
            if invert:
                lum = 1.0 - lum
            glyph = ramp[min(levels - 1, int(lum * levels))]
            if color:
                parts.append(f"\x1b[38;2;{int(r)};{int(g)};{int(b)}m{glyph}")
            else:
                parts.append(glyph)
        lines.append("".join(parts) + (RESET if color else ""))
    return "\n".join(lines) + "\n"


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="glyphcast", description="Turn an image into text art.")
    parser.add_argument("image", help="path to a binary PPM (P6) or 24-bit BMP file")
    parser.add_argument("-w", "--width", type=int, default=80,
                        help="output width in characters (default: 80)")
    parser.add_argument("-r", "--ramp", default="classic",
                        help="preset (classic, minimal, dense) or a custom ramp string")
    parser.add_argument("--invert", action="store_true",
                        help="invert brightness (for light backgrounds)")
    parser.add_argument("--color", action="store_true", help="emit 24-bit ANSI colour")
    parser.add_argument("-o", "--output", help="write to a file instead of stdout")
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        image = load_image(args.image)
        art = render(
            image,
            width=args.width,
            ramp=resolve_ramp(args.ramp),
            invert=args.invert,
            color=args.color,
        )
    except (OSError, ValueError) as exc:
        print(f"glyphcast: error: {exc}", file=sys.stderr)
        return 1

    if args.output:
        Path(args.output).write_text(art, encoding="utf-8")
    else:
        sys.stdout.write(art)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
