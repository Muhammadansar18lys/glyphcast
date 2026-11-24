#include "glyphcast/image.hpp"
#include "glyphcast/renderer.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr const char* kVersion = "1.0.0";

void printUsage() {
    std::cout << "Glyphcast " << kVersion << " - turn images into text art\n\n"
              << "Usage: glyphcast <image> [options]\n\n"
              << "Arguments:\n"
              << "  <image>            binary PPM (P6) or 24-bit uncompressed BMP\n\n"
              << "Options:\n"
              << "  -w, --width N      output width in characters (default: 80)\n"
              << "  -r, --ramp NAME    preset (classic, minimal, dense) or a custom ramp\n"
              << "      --invert       invert brightness (for light backgrounds)\n"
              << "      --color        emit 24-bit ANSI colour\n"
              << "  -h, --help         show this message\n";
}

struct Arguments {
    std::string imagePath;
    glyphcast::RenderOptions options;
    bool help = false;
};

std::string nextValue(int argc, char** argv, int& index) {
    if (index + 1 >= argc) {
        throw std::invalid_argument(std::string("missing value for ") + argv[index]);
    }
    return argv[++index];
}

Arguments parseArguments(int argc, char** argv) {
    Arguments args;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            args.help = true;
        } else if (arg == "-w" || arg == "--width") {
            args.options.width = std::stoi(nextValue(argc, argv, i));
        } else if (arg == "-r" || arg == "--ramp") {
            args.options.ramp = glyphcast::resolveRamp(nextValue(argc, argv, i));
        } else if (arg == "--invert") {
            args.options.invert = true;
        } else if (arg == "--color") {
            args.options.color = glyphcast::ColorMode::Ansi;
        } else if (!arg.empty() && arg[0] == '-') {
            throw std::invalid_argument("unknown option: " + arg);
        } else {
            args.imagePath = arg;
        }
    }
    if (!args.help && args.imagePath.empty()) {
        throw std::invalid_argument("no image file given (see --help)");
    }
    return args;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const Arguments args = parseArguments(argc, argv);
        if (args.help) {
            printUsage();
            return EXIT_SUCCESS;
        }
        const glyphcast::Image image = glyphcast::loadImage(args.imagePath);
        std::cout << glyphcast::render(image, args.options);
        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::cerr << "glyphcast: error: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
