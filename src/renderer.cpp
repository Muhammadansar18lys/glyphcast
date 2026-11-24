#include "glyphcast/renderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace glyphcast {

namespace {

// Terminal cells are roughly twice as tall as they are wide.
constexpr double kCellAspect = 0.5;

struct Color {
    double r;
    double g;
    double b;
};

double luminanceOf(double r, double g, double b) {
    return (0.2126 * r + 0.7152 * g + 0.0722 * b) / 255.0;
}

// Averages every pixel in the half-open rectangle [x0, x1) x [y0, y1).
Color averageBlock(const Image& image, int x0, int y0, int x1, int y1) {
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
    int count = 0;
    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            const Pixel& p = image.at(x, y);
            r += p.r;
            g += p.g;
            b += p.b;
            ++count;
        }
    }
    return {r / count, g / count, b / count};
}

}  // namespace

std::string resolveRamp(const std::string& nameOrRamp) {
    if (nameOrRamp == "classic") return kRampClassic;
    if (nameOrRamp == "minimal") return kRampMinimal;
    if (nameOrRamp == "dense") return kRampDense;
    return nameOrRamp;
}

double luminance(const Pixel& pixel) {
    return luminanceOf(pixel.r, pixel.g, pixel.b);
}

std::string render(const Image& image, const RenderOptions& options) {
    if (options.width <= 0) {
        throw std::invalid_argument("width must be positive");
    }
    if (options.ramp.empty()) {
        throw std::invalid_argument("character ramp must not be empty");
    }

    const int cols = options.width;
    const int rows = std::max(
        1, static_cast<int>(std::lround(static_cast<double>(image.height()) /
                                        image.width() * cols * kCellAspect)));
    const std::size_t levels = options.ramp.size();

    std::string out;
    out.reserve(static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols + 1) * 24);

    for (int row = 0; row < rows; ++row) {
        const int y0 = row * image.height() / rows;
        const int y1 = std::max(y0 + 1, (row + 1) * image.height() / rows);

        for (int col = 0; col < cols; ++col) {
            const int x0 = col * image.width() / cols;
            const int x1 = std::max(x0 + 1, (col + 1) * image.width() / cols);

            const Color c = averageBlock(image, x0, y0, x1, y1);
            double lum = luminanceOf(c.r, c.g, c.b);
            if (options.invert) {
                lum = 1.0 - lum;
            }
            const std::size_t index =
                std::min(levels - 1, static_cast<std::size_t>(lum * static_cast<double>(levels)));
            const char glyph = options.ramp[index];

            if (options.color == ColorMode::Ansi) {
                out += "\x1b[38;2;" + std::to_string(static_cast<int>(c.r)) + ";" +
                       std::to_string(static_cast<int>(c.g)) + ";" +
                       std::to_string(static_cast<int>(c.b)) + "m";
            }
            out += glyph;
        }

        if (options.color == ColorMode::Ansi) {
            out += "\x1b[0m";
        }
        out += '\n';
    }
    return out;
}

}  // namespace glyphcast
