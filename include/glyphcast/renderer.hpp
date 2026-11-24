#ifndef GLYPHCAST_RENDERER_HPP
#define GLYPHCAST_RENDERER_HPP

#include "glyphcast/image.hpp"

#include <string>

namespace glyphcast {

// Character ramps ordered from darkest (first) to brightest (last).
inline constexpr const char* kRampClassic = " .:-=+*#%@";
inline constexpr const char* kRampMinimal = " .:#";
inline constexpr const char* kRampDense =
    "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

enum class ColorMode { None, Ansi };

struct RenderOptions {
    int width = 80;
    std::string ramp = kRampClassic;
    bool invert = false;
    ColorMode color = ColorMode::None;
};

// Maps a preset name ("classic", "minimal", "dense") to its ramp.
// Any other text is returned unchanged and used as a custom ramp.
std::string resolveRamp(const std::string& nameOrRamp);

// Relative luminance in the range [0, 1], using Rec. 709 weights.
double luminance(const Pixel& pixel);

// Converts an image to text art, one line per row of output.
// Throws std::invalid_argument for a non-positive width or an empty ramp.
std::string render(const Image& image, const RenderOptions& options);

}  // namespace glyphcast

#endif  // GLYPHCAST_RENDERER_HPP
