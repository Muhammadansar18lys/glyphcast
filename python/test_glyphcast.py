"""Unit tests for the Python implementation. Run with: python -m unittest -v"""

import tempfile
import unittest
from pathlib import Path

import glyphcast as gc


def solid(value: int, size: int = 10) -> gc.Image:
    return gc.Image(size, size, [(value, value, value)] * (size * size))


class LuminanceTests(unittest.TestCase):
    def test_black_is_zero(self) -> None:
        self.assertEqual(gc.luminance(0, 0, 0), 0)

    def test_white_is_one(self) -> None:
        self.assertAlmostEqual(gc.luminance(255, 255, 255), 1.0)


class RenderTests(unittest.TestCase):
    def test_shape_follows_aspect_ratio(self) -> None:
        lines = gc.render(solid(255), width=4).splitlines()
        self.assertEqual(lines, ["@@@@", "@@@@"])

    def test_bright_uses_densest_glyph(self) -> None:
        self.assertEqual(gc.render(solid(255), width=2)[0], "@")

    def test_dark_uses_sparsest_glyph(self) -> None:
        self.assertEqual(gc.render(solid(0), width=2)[0], " ")

    def test_invert_swaps_ends(self) -> None:
        self.assertEqual(gc.render(solid(255), width=2, invert=True)[0], " ")

    def test_colour_output_contains_escape(self) -> None:
        self.assertIn("\x1b[38;2;128;128;128m", gc.render(solid(128), width=2, color=True))

    def test_invalid_width_raises(self) -> None:
        with self.assertRaises(ValueError):
            gc.render(solid(0), width=0)

    def test_empty_ramp_raises(self) -> None:
        with self.assertRaises(ValueError):
            gc.render(solid(0), ramp="")


class RampTests(unittest.TestCase):
    def test_preset_names_resolve(self) -> None:
        self.assertEqual(gc.resolve_ramp("minimal"), gc.RAMP_MINIMAL)

    def test_custom_ramp_passes_through(self) -> None:
        self.assertEqual(gc.resolve_ramp("@#"), "@#")


class LoaderTests(unittest.TestCase):
    def test_ppm_round_trip(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "tiny.ppm"
            path.write_bytes(b"P6\n2 1\n255\n" + bytes([255, 0, 0, 0, 255, 0]))
            image = gc.load_image(path)
        self.assertEqual((image.width, image.height), (2, 1))
        self.assertEqual(image.pixels, [(255, 0, 0), (0, 255, 0)])

    def test_unknown_format_raises(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "notes.txt"
            path.write_text("not an image")
            with self.assertRaises(gc.ImageFormatError):
                gc.load_image(path)


if __name__ == "__main__":
    unittest.main()
