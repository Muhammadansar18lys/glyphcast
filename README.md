<div align="center">

# 🎨 Glyphcast

### Turn any picture into text art, in C++, Python and the browser.

Built around one small, fast algorithm: average each cell of the image, measure its brightness,
then pick a character that matches. Optional 24-bit colour keeps the original tones.

[![CI](https://github.com/Muhammadansar18lys/glyphcast/actions/workflows/ci.yml/badge.svg)](https://github.com/Muhammadansar18lys/glyphcast/actions/workflows/ci.yml)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
![Python](https://img.shields.io/badge/Python-3.10%2B-3776AB?logo=python&logoColor=white)
![JavaScript](https://img.shields.io/badge/JavaScript-ES2020-F7DF1E?logo=javascript&logoColor=black)
![CMake](https://img.shields.io/badge/CMake-3.16%2B-064F8C?logo=cmake&logoColor=white)
[![License: MIT](https://img.shields.io/badge/License-MIT-7c5cff.svg)](LICENSE)

</div>

---

## ✦ Overview

Glyphcast converts an image into a grid of characters. Each character is chosen from a
*ramp*, ordered from darkest to brightest:

```text
 .:-=+*#%@      ← classic ramp: sparse on the left, dense on the right
```

Bright areas get heavy glyphs such as `@` and `#`, and shadows get light ones such as `.` and
space. Colour output paints each glyph in the average colour of its cell.

The project is one algorithm in three implementations, each with the same output:

| Version | Location | Stack | Best for |
|---|---|---|---|
| ⚙️ **C++ CLI and library** | `src/`, `include/` | C++17, CMake, no dependencies | Speed and a reusable core |
| 🐍 **Python CLI** | `python/` | Python 3.10+ standard library | Scripting and quick use |
| 🌐 **Browser app** | `web/` | HTML, CSS, vanilla JS | Any image format, no install |

## ✦ Features

- 🖼️ Reads binary **PPM (P6)** and 24-bit **BMP** natively, and **any format** in the browser
- 📐 Corrects for terminal cell shape, so pictures don't look stretched
- 🎚️ Three built-in ramps (`classic`, `minimal`, `dense`) plus custom ramp strings
- 🌈 24-bit ANSI colour output for modern terminals
- 🔄 Invert mode for light backgrounds
- 📋 Browser app: drag and drop, live preview, copy and download
- 🧪 Unit tests in C++ and Python, with CI on every push

## ✦ Quick start

### C++

Requirements: a C++17 compiler and CMake 3.16+.

```bash
git clone https://github.com/Muhammadansar18lys/glyphcast.git
cd glyphcast

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

./build/glyphcast photo.bmp --width 100 --ramp dense --color
ctest --test-dir build --output-on-failure
```

### Python

Requirements: Python 3.10 or newer. No packages to install.

```bash
python python/glyphcast.py photo.ppm --width 80 --ramp minimal
python python/glyphcast.py photo.ppm -o art.txt --color
```

### Browser

No build step. Open `web/index.html` in any modern browser, then drop an image onto the page.
Your image is processed on your device and is never uploaded.

## ✦ Options

| Option | Meaning | Default |
|---|---|---|
| `-w`, `--width N` | Output width in characters | `80` |
| `-r`, `--ramp NAME` | `classic`, `minimal`, `dense`, or a custom ramp string | `classic` |
| `--invert` | Invert brightness (for light backgrounds) | off |
| `--color` | Emit 24-bit ANSI colour | off |
| `-o`, `--output FILE` | Python CLI only: write to a file | stdout |

Example custom ramp, from darkest to brightest:

```bash
./build/glyphcast photo.bmp --ramp " .,:;i1tfLCG08@"
```

## ✦ How it works

1. **Load:** the image is decoded into an RGB raster.
2. **Sample:** the raster is split into a grid. Each cell is averaged in the pixel space it covers.
3. **Measure:** each average colour becomes a luminance value, using Rec. 709 weights:
   `L = (0.2126 R + 0.7152 G + 0.0722 B) / 255`.
4. **Map:** luminance selects a glyph from the ramp. `--invert` flips the scale first.
5. **Correct aspect:** rows are halved in count, because terminal characters are about twice as tall as they are wide.

## ✦ Project structure

```
glyphcast/
├── include/glyphcast/
│   ├── image.hpp            # Image type and file loading API
│   └── renderer.hpp         # Render options, ramps and render()
├── src/
│   ├── image.cpp            # PPM and BMP decoders
│   ├── renderer.cpp         # Sampling, luminance and output
│   └── main.cpp             # Command-line interface
├── tests/
│   └── test_renderer.cpp    # C++ unit tests
├── python/
│   ├── glyphcast.py         # Python implementation and CLI
│   └── test_glyphcast.py    # Python unit tests
├── web/
│   ├── index.html           # Browser app layout
│   ├── style.css            # Minimal, colourful theme
│   └── glyphcast.js         # Browser renderer
├── .github/workflows/ci.yml # Build, test and syntax checks
├── CMakeLists.txt
├── CHANGELOG.md
├── CONTRIBUTING.md
├── LICENSE
└── README.md
```

## ✦ Roadmap

- [ ] PNG and JPEG support in the C++ and Python versions
- [ ] Edge-detection ramp for outline-style art
- [ ] Export to HTML with colour preserved
- [ ] Animated output for GIF input

## ✦ Contributing

Contributions are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md) for the workflow, the checks to run,
and the rule that all three implementations must agree.

## ✦ Author

**Muhammad Ansar**
Student, software engineering, CUSIT Peshawar
🔗 [github.com/Muhammadansar18lys](https://github.com/Muhammadansar18lys)

## ✦ License

Released under the MIT License. See [LICENSE](LICENSE).

---

<div align="center">

🟣 🔵 🟢 🟡 🟠 🔴 &nbsp; **Every picture is a pattern of characters.** &nbsp; 🔴 🟠 🟡 🟢 🔵 🟣

</div>
