# Ray Tracing

Simple ray traced rendering using C++ and OpenGL.

Inspired by [Sebastian Lague](https://youtu.be/Qz0KTGYJtUk) and 
[Ray Tracing In One Weekend](https://raytracing.github.io/).

## Features

* Ray traced lighting
* Real-time preview
* Render to image (png)
* Object & material customisation
* .obj models with BVH acceleration (Work in progress)
* Save and load scenes
* Highly customisable gui ([Dear ImGui](https://github.com/ocornut/imgui))

## Controls
 
* W - Forwards  
* A - Left  
* S - Back  
* D - Right
* Space - Up
* Control - Down
* Shift - Sprint
* Mouse & Arrows - Pan Camera
* Tab - Toggle Movement
* Escape - Exit Program / Cancel Render

## Getting Started

Either download the latest release or follow below to build yourself.  
**Note gui windows may be poorly sized / positioned on first start.**

### Dependencies

**All platforms**

* A GPU and driver supporting **OpenGL 4.6**
* [Git](https://git-scm.com/) (Optional, can download source as zip instead)
* [CMake](https://cmake.org/download/) 3.20 or newer
* A C++17 compiler
* [Ninja](https://ninja-build.org/)

**Linux**

* GTK 3 (file dialogs)
* Wayland and wayland-protocols
* libxkbcommon
* X11: Xrandr, Xinerama, Xcursor, Xi
* OpenGL (libglvnd)

**Windows (native)**

* [Visual Studio 2022](https://visualstudio.microsoft.com/) with the *Desktop development with C++* workload (includes the compiler, Windows SDK, and optionally CMake and Ninja)

**Cross compile Windows exe from Linux (MinGW)**

* MinGW-w64 C++ cross compiler
* Static winpthreads

***

### Clone the repository

```bash
git clone https://github.com/git-cameron/Ray-Tracing.git
cd Ray-Tracing
```

### Linux

**Build and run**

```bash
cmake --preset linux-release
cmake --build build/linux-release -j
./build/linux-release/Ray_Tracing
```

Use `linux-debug` for a debug build

### Windows (native)

Run the build commands below from the **x64 Native Tools Command Prompt for VS 2022**

**Release**

```bat
cmake -S . -B build\msvc-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build\msvc-release
build\msvc-release\Ray_Tracing.exe
```

**Debug**

```bat
cmake -S . -B build\msvc-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build\msvc-debug
build\msvc-debug\Ray_Tracing.exe
```

### Cross compile Windows exe from Linux (MinGW)

```bash
cmake --preset mingw-release
cmake --build build/mingw-release -j
```

For a debug build use the `mingw-debug` preset (output in `build/mingw-debug/`)

## Third-party libraries

[GLFW](https://www.glfw.org/), [glad](https://github.com/Dav1dde/glad),
[Dear ImGui](https://github.com/ocornut/imgui), [GLM](https://github.com/g-truc/glm),
[stb](https://github.com/nothings/stb),
[tinyobjloader](https://github.com/tinyobjloader/tinyobjloader),
[nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended)

## AI Disclosure

AI was used to help migrate the build from a Visual Studio project to CMake, and to help update this README.

## License

See [LICENSE](LICENSE)