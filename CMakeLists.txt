cmake_minimum_required(VERSION 3.16)
project(PackageResolver VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Collect source files
set(SOURCES
    src/main.cpp
    src/version.cpp
    src/constraint.cpp
    src/package.cpp
    src/json_parser.cpp
    src/manifest.cpp
    src/graph.cpp
    src/resolver.cpp
    src/registry.cpp
    src/installer.cpp
    src/cli.cpp
)

# Executable
add_executable(pkgr ${SOURCES})

# Set output directory
set_target_properties(pkgr PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(pkgr PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Enable ANSI escape codes on Windows
    target_compile_definitions(pkgr PRIVATE ENABLE_VIRTUAL_TERMINAL_PROCESSING)
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(pkgr PRIVATE /W4)
else()
    target_compile_options(pkgr PRIVATE -Wall -Wextra -Wpedantic)
endif()
