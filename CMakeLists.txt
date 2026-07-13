cmake_minimum_required(VERSION 3.16)
project(PackageResolver VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(PKGR_BUILD_TESTS "Build the unit test suite" ON)

# Core library shared by the CLI and the test suite
add_library(pkgr_lib STATIC
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
target_include_directories(pkgr_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

# CLI executable
add_executable(pkgr src/main.cpp)
target_link_libraries(pkgr PRIVATE pkgr_lib)
set_target_properties(pkgr PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set(PKGR_TARGETS pkgr_lib pkgr)

# Unit tests
if(PKGR_BUILD_TESTS)
    enable_testing()

    add_executable(pkgr_tests
        tests/test_main.cpp
        tests/test_version.cpp
        tests/test_constraint.cpp
        tests/test_graph.cpp
        tests/test_resolver.cpp
    )
    target_link_libraries(pkgr_tests PRIVATE pkgr_lib)
    set_target_properties(pkgr_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    add_test(NAME unit_tests COMMAND pkgr_tests)
    list(APPEND PKGR_TARGETS pkgr_tests)
endif()

# Platform-specific settings and warnings
foreach(target IN LISTS PKGR_TARGETS)
    if(WIN32)
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
        # Enable ANSI escape codes on Windows
        target_compile_definitions(${target} PRIVATE ENABLE_VIRTUAL_TERMINAL_PROCESSING)
    endif()

    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
