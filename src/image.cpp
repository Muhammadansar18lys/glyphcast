#include "glyphcast/image.hpp"

#include <cctype>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace glyphcast {

namespace {

using Bytes = std::vector<unsigned char>;

Bytes readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("cannot open file: " + path);
    }
    return Bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

std::size_t skipWhitespaceAndComments(const Bytes& d, std::size_t pos) {
    while (pos < d.size()) {
        if (std::isspace(d[pos])) {
            ++pos;
        } else if (d[pos] == '#') {
            while (pos < d.size() && d[pos] != '\n') {
                ++pos;
            }
        } else {
            break;
        }
    }
    return pos;
}

int readHeaderInt(const Bytes& d, std::size_t& pos) {
    pos = skipWhitespaceAndComments(d, pos);
    if (pos >= d.size() || !std::isdigit(d[pos])) {
        throw std::runtime_error("invalid PPM header");
    }
    int value = 0;
    while (pos < d.size() && std::isdigit(d[pos])) {
        value = value * 10 + (d[pos] - '0');
        ++pos;
    }
    return value;
}

Image loadPpm(const Bytes& d) {
    std::size_t pos = 2;  // skip the "P6" magic number
    const int width = readHeaderInt(d, pos);
    const int height = readHeaderInt(d, pos);
    const int maxValue = readHeaderInt(d, pos);
    if (maxValue != 255) {
        throw std::runtime_error("only 8-bit PPM (maxval 255) is supported");
    }
    ++pos;  // exactly one whitespace byte separates the header from pixel data

    const std::size_t needed = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 3;
    if (d.size() < pos + needed) {
        throw std::runtime_error("PPM pixel data is truncated");
    }

    std::vector<Pixel> pixels;
    pixels.reserve(needed / 3);
    for (std::size_t i = 0; i < needed; i += 3) {
        pixels.push_back({d[pos + i], d[pos + i + 1], d[pos + i + 2]});
    }
    return Image(width, height, std::move(pixels));
}

std::uint32_t readLe32(const Bytes& d, std::size_t off) {
    return static_cast<std::uint32_t>(d[off]) |
           (static_cast<std::uint32_t>(d[off + 1]) << 8) |
           (static_cast<std::uint32_t>(d[off + 2]) << 16) |
           (static_cast<std::uint32_t>(d[off + 3]) << 24);
}

std::uint16_t readLe16(const Bytes& d, std::size_t off) {
    return static_cast<std::uint16_t>(d[off] | (d[off + 1] << 8));
}

Image loadBmp(const Bytes& d) {
    if (d.size() < 54) {
        throw std::runtime_error("BMP header is truncated");
    }
    const std::size_t dataOffset = readLe32(d, 10);
    const int width = static_cast<std::int32_t>(readLe32(d, 18));
    const int heightRaw = static_cast<std::int32_t>(readLe32(d, 22));
    const std::uint16_t bitsPerPixel = readLe16(d, 28);
    const std::uint32_t compression = readLe32(d, 30);

    if (bitsPerPixel != 24 || compression != 0) {
        throw std::runtime_error("only uncompressed 24-bit BMP is supported");
    }
    if (width <= 0 || heightRaw == 0) {
        throw std::runtime_error("BMP has invalid dimensions");
    }

    const bool topDown = heightRaw < 0;
    const int height = topDown ? -heightRaw : heightRaw;
    const std::size_t w = static_cast<std::size_t>(width);
    const std::size_t h = static_cast<std::size_t>(height);
    const std::size_t stride = (w * 3 + 3) & ~std::size_t{3};  // rows are padded to 4 bytes

    if (d.size() < dataOffset + stride * h) {
        throw std::runtime_error("BMP pixel data is truncated");
    }

    std::vector<Pixel> pixels;
    pixels.reserve(w * h);
    for (std::size_t row = 0; row < h; ++row) {
        const std::size_t srcRow = topDown ? row : h - 1 - row;  // BMP is usually bottom-up
        const std::size_t base = dataOffset + stride * srcRow;
        for (std::size_t x = 0; x < w; ++x) {
            const std::size_t p = base + x * 3;
            pixels.push_back({d[p + 2], d[p + 1], d[p]});  // stored as BGR
        }
    }
    return Image(width, height, std::move(pixels));
}

}  // namespace

Image::Image(int width, int height, std::vector<Pixel> pixels)
    : width_(width), height_(height), pixels_(std::move(pixels)) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("image dimensions must be positive");
    }
    if (pixels_.size() != static_cast<std::size_t>(width) * static_cast<std::size_t>(height)) {
        throw std::invalid_argument("pixel count does not match image dimensions");
    }
}

const Pixel& Image::at(int x, int y) const {
    return pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) +
                   static_cast<std::size_t>(x)];
}

Image loadImage(const std::string& path) {
    const Bytes data = readFile(path);
    if (data.size() >= 2 && data[0] == 'P' && data[1] == '6') {
        return loadPpm(data);
    }
    if (data.size() >= 2 && data[0] == 'B' && data[1] == 'M') {
        return loadBmp(data);
    }
    throw std::runtime_error("unsupported image format (use binary PPM or 24-bit BMP): " + path);
}

}  // namespace glyphcast
