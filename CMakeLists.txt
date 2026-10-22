cmake_minimum_required(VERSION 3.20)
project(Ray_Tracing LANGUAGES C CXX)

# embedFiles needs std::filesystem
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------- Dependencies ----------------
# GLFW: use the system one if available, otherwise build from source
# (on Linux this needs the X11/Wayland dev packages installed)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# ---------------- Shader embedding ----------------
if(CMAKE_CROSSCOMPILING)
    # Build the tool for the host machine, ignoring the toolchain file
    include(ExternalProject)
    ExternalProject_Add(embedFiles_host
            SOURCE_DIR        ${CMAKE_SOURCE_DIR}/tools
            BINARY_DIR        ${CMAKE_BINARY_DIR}/embedFiles_host
            CMAKE_ARGS        -DCMAKE_TOOLCHAIN_FILE= -DCMAKE_BUILD_TYPE=Release
            BUILD_COMMAND     ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release
            INSTALL_COMMAND   ""
            BUILD_BYPRODUCTS  ${CMAKE_BINARY_DIR}/embedFiles_host/embedFiles)
    set(EMBED_TOOL     ${CMAKE_BINARY_DIR}/embedFiles_host/embedFiles)
    set(EMBED_TOOL_DEP embedFiles_host)
else()
    add_executable(embedFiles tools/embedFiles.cpp)
    set(EMBED_TOOL     embedFiles)
    set(EMBED_TOOL_DEP embedFiles)
endif()

set(SHADER_DIR    ${CMAKE_SOURCE_DIR}/src/shaders)
set(GEN_DIR       ${CMAKE_BINARY_DIR}/generated)
set(SHADER_HEADER ${GEN_DIR}/ShaderStrings.h)

file(GLOB SHADER_FILES CONFIGURE_DEPENDS ${SHADER_DIR}/*)

add_custom_command(
        OUTPUT  ${SHADER_HEADER}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}
        COMMAND ${EMBED_TOOL} ${SHADER_DIR} ${SHADER_HEADER}
        DEPENDS ${EMBED_TOOL_DEP} ${SHADER_FILES}
        COMMENT "Embedding shaders into ShaderStrings.h"
        VERBATIM)

# ---------------- Third-party static libraries ----------------
# glad (C)
add_library(glad STATIC external/glad/glad.c)
target_include_directories(glad SYSTEM PUBLIC
        external/glad
        external)
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})

# imgui
add_library(imgui STATIC
        external/imgui/imgui.cpp
        #external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_impl_glfw.cpp
        external/imgui/imgui_impl_opengl3.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp)
target_include_directories(imgui SYSTEM PUBLIC external/imgui)
target_link_libraries(imgui PUBLIC glfw glad)
target_compile_definitions(imgui PUBLIC GLFW_INCLUDE_NONE)

# Native file dialogs (vendored nativefiledialog-extended)
add_subdirectory(external/nfd)

# Header-only libraries (resolved through the `external` include root:
add_library(header_only_deps INTERFACE)
target_include_directories(header_only_deps SYSTEM INTERFACE external)

# ---------------- Main app ----------------
file(GLOB APP_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(Ray_Tracing ${APP_SOURCES} ${SHADER_HEADER})

target_include_directories(Ray_Tracing PRIVATE
        ${GEN_DIR} # generated ShaderStrings.h
        src
        src/headers)

target_link_libraries(Ray_Tracing PRIVATE
        glad imgui header_only_deps glfw nfd ${CMAKE_DL_LIBS})

if(MSVC)
    target_compile_options(Ray_Tracing PRIVATE /W3)
    target_compile_definitions(Ray_Tracing PRIVATE _CRT_SECURE_NO_WARNINGS)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Ray_Tracing)
else()
    target_compile_options(Ray_Tracing PRIVATE -Wall -Wno-sign-compare)
endif()

# Ship a standalone Windows exe from the MinGW cross build
if(MINGW)
    target_link_options(Ray_Tracing PRIVATE -static -static-libgcc -static-libstdc++)
endif()

# Run from the repo root so imgui.ini resolves.
set_target_properties(Ray_Tracing PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")