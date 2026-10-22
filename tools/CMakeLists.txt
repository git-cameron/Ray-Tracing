cmake_minimum_required(VERSION 3.20)
project(embedFiles LANGUAGES CXX)

# Host-side helper that turns the shaders into ShaderStrings.h.
# Only used by the main build when cross-compiling (see the top-level
# CMakeLists.txt); native builds compile embedFiles.cpp directly.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(embedFiles embedFiles.cpp)