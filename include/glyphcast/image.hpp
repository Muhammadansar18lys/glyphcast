#ifndef GLYPHCAST_IMAGE_HPP
#define GLYPHCAST_IMAGE_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace glyphcast {

struct Pixel {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
};

// An immutable RGB raster stored row by row, top to bottom.
class Image {
public:
    Image(int width, int height, std::vector<Pixel> pixels);

    int width() const { return width_; }
    int height() const { return height_; }
    const Pixel& at(int x, int y) const;

private:
    int width_;
    int height_;
    std::vector<Pixel> pixels_;
};

// Loads a binary PPM (P6) or a 24-bit uncompressed BMP file.
// Throws std::runtime_error when the file is missing, truncated or unsupported.
Image loadImage(const std::string& path);

}  // namespace glyphcast

#endif  // GLYPHCAST_IMAGE_HPP
