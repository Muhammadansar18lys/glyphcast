#include "glyphcast/image.hpp"
#include "glyphcast/renderer.hpp"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int failures = 0;

#define EXPECT(cond)                                                              \
    do {                                                                          \
        if (!(cond)) {                                                            \
            std::cerr << __FILE__ << ":" << __LINE__ << ": FAILED: " #cond "\n";  \
            ++failures;                                                           \
        }                                                                         \
    } while (0)

glyphcast::Image solidImage(std::uint8_t value, int size) {
    std::vector<glyphcast::Pixel> pixels(static_cast<std::size_t>(size * size),
                                         glyphcast::Pixel{value, value, value});
    return glyphcast::Image(size, size, std::move(pixels));
}

void testLuminance() {
    EXPECT(glyphcast::luminance(glyphcast::Pixel{0, 0, 0}) == 0.0);
    EXPECT(std::fabs(glyphcast::luminance(glyphcast::Pixel{255, 255, 255}) - 1.0) < 1e-9);
}

void testResolveRamp() {
    EXPECT(glyphcast::resolveRamp("minimal") == glyphcast::kRampMinimal);
    EXPECT(glyphcast::resolveRamp("@#") == "@#");
}

void testOutputShapeFollowsAspect() {
    glyphcast::RenderOptions options;
    options.width = 4;
    const std::string art = glyphcast::render(solidImage(255, 10), options);

    // A square image at width 4 gives 4 * 1.0 * 0.5 = 2 rows.
    EXPECT(art == "@@@@\n@@@@\n");
}

void testBrightAndDarkMapToRampEnds() {
    glyphcast::RenderOptions options;
    options.width = 2;
    EXPECT(glyphcast::render(solidImage(255, 4), options)[0] == '@');
    EXPECT(glyphcast::render(solidImage(0, 4), options)[0] == ' ');
}

void testInvertSwapsEnds() {
    glyphcast::RenderOptions options;
    options.width = 2;
    options.invert = true;
    EXPECT(glyphcast::render(solidImage(255, 4), options)[0] == ' ');
}

void testColorOutputContainsAnsi() {
    glyphcast::RenderOptions options;
    options.width = 2;
    options.color = glyphcast::ColorMode::Ansi;
    const std::string art = glyphcast::render(solidImage(128, 4), options);
    EXPECT(art.find("\x1b[38;2;128;128;128m") != std::string::npos);
}

void testInvalidOptionsThrow() {
    glyphcast::RenderOptions options;
    options.width = 0;
    bool threw = false;
    try {
        glyphcast::render(solidImage(0, 2), options);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    EXPECT(threw);
}

void testLoadPpmRoundTrip() {
    const std::string path = "glyphcast_test.ppm";
    {
        std::ofstream out(path, std::ios::binary);
        out << "P6\n2 1\n255\n";
        const char pixels[] = {static_cast<char>(255), 0, 0, 0, static_cast<char>(255), 0};
        out.write(pixels, sizeof(pixels));
    }
    const glyphcast::Image image = glyphcast::loadImage(path);
    std::remove(path.c_str());

    EXPECT(image.width() == 2);
    EXPECT(image.height() == 1);
    EXPECT(image.at(0, 0).r == 255);
    EXPECT(image.at(1, 0).g == 255);
}

void testUnsupportedFormatThrows() {
    const std::string path = "glyphcast_test.bin";
    {
        std::ofstream out(path, std::ios::binary);
        out << "XX not an image";
    }
    bool threw = false;
    try {
        glyphcast::loadImage(path);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    std::remove(path.c_str());
    EXPECT(threw);
}

}  // namespace

int main() {
    testLuminance();
    testResolveRamp();
    testOutputShapeFollowsAspect();
    testBrightAndDarkMapToRampEnds();
    testInvertSwapsEnds();
    testColorOutputContainsAnsi();
    testInvalidOptionsThrow();
    testLoadPpmRoundTrip();
    testUnsupportedFormatThrows();

    if (failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }
    std::cerr << failures << " test(s) failed.\n";
    return 1;
}
