# Cross-compile a Windows x64 executable from Linux using MinGW-w64.
# Debian/Ubuntu: sudo apt install mingw-w64
# Fedora:        sudo dnf install mingw64-gcc mingw64-gcc-c++ mingw64-winpthreads-static
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER  x86_64-w64-mingw32-windres)

# Where MinGW keeps its Windows headers and libs (opengl32 etc.).
# Debian/Ubuntu use the first path, Fedora the second; missing ones are ignored.
set(CMAKE_FIND_ROOT_PATH
        /usr/x86_64-w64-mingw32
        /usr/x86_64-w64-mingw32/sys-root/mingw)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Link the MinGW runtime statically so the .exe runs without extra DLLs
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++")