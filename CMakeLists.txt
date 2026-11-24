cmake_minimum_required(VERSION 3.16)
project(Glyphcast VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(glyphcast_core src/image.cpp src/renderer.cpp)
target_include_directories(glyphcast_core PUBLIC include)
if(NOT MSVC)
    target_compile_options(glyphcast_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(glyphcast src/main.cpp)
target_link_libraries(glyphcast PRIVATE glyphcast_core)

option(GLYPHCAST_BUILD_TESTS "Build the unit tests" ON)
if(GLYPHCAST_BUILD_TESTS)
    enable_testing()
    add_executable(glyphcast_tests tests/test_renderer.cpp)
    target_link_libraries(glyphcast_tests PRIVATE glyphcast_core)
    add_test(NAME glyphcast_tests COMMAND glyphcast_tests)
endif()
