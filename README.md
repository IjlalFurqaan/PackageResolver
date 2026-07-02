# PackageResolver

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-green.svg)](https://cmake.org/)

PackageResolver is a robust, dependency-free C++ package dependency resolver. Inspired by modern package managers such as `npm`, `pip`, and `cargo`, it provides a comprehensive suite of tools for parsing package manifests, building dependency graphs, performing topological sorting for installation order, detecting circular dependencies, and resolving complex version constraints.

## Features

- **Dependency Resolution:** Computes the optimal installation order using topological sorting.
- **Cycle Detection:** Automatically detects and reports circular dependencies in the package graph.
- **Version Constraint Matching:** Supports a wide range of semantic versioning operators (`>=`, `<=`, `>`, `<`, `=`, `!=`, `^`, `~`).
- **Zero Dependencies:** Built entirely with standard C++17—no external libraries required, including a custom-built JSON parser.
- **Simulated Installation:** Dry-run capabilities to visualize the installation process.

## Getting Started

### Prerequisites

Ensure you have the following installed:
- A C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16 or higher

### Building from Source

Clone the repository and build the project using CMake:

```bash
git clone https://github.com/IjlalFurqaan/PackageResolver.git
cd PackageResolver
mkdir build && cd build
cmake ..
cmake --build .
```

## Usage

The `pkgr` command-line tool provides several operations to interact with manifests and the registry.

```bash
# Run from the project root directory:
./build/bin/pkgr <command> [arguments]
```

### Available Commands

| Command | Description | Example |
|---------|-------------|---------|
| `resolve` | Resolves dependencies and displays the computed installation order. | `./build/bin/pkgr resolve sample_project/manifest.json` |
| `install` | Resolves dependencies and simulates the package installation process. | `./build/bin/pkgr install sample_project/manifest.json` |
| `graph` | Displays a visual tree of the dependency graph. | `./build/bin/pkgr graph sample_project/manifest.json` |
| `check` | Validates the dependency graph for circular dependencies. | `./build/bin/pkgr check sample_project/manifest.json` |
| `list` | Lists all available packages currently present in the registry. | `./build/bin/pkgr list` |
| `info` | Displays detailed metadata for a specific package. | `./build/bin/pkgr info web-framework` |
| `version` | Displays the current version of the PackageResolver. | `./build/bin/pkgr version` |
| `help` | Displays the help menu. | `./build/bin/pkgr help` |

## Manifest Format

Package manifests are defined using JSON. The resolver reads these files to determine the package's metadata and dependencies.

**Example `manifest.json`:**
```json
{
    "name": "my-project",
    "version": "1.0.0",
    "description": "An example package manifest.",
    "dependencies": {
        "http-client": ">=1.5.0",
        "json-parser": "^2.0.0"
    }
}
```

### Supported Version Constraints

| Operator | Meaning | Example | Resolution |
|----------|---------|---------|------------|
| `>=` | Greater than or equal | `>=1.0.0` | Any version `1.0.0` or higher |
| `<=` | Less than or equal | `<=2.0.0` | Any version `2.0.0` or lower |
| `>` | Greater than | `>1.0.0` | Strictly greater than `1.0.0` |
| `<` | Less than | `<2.0.0` | Strictly less than `2.0.0` |
| `=` | Exact match | `=1.2.3` | Exactly `1.2.3` |
| `!=` | Not equal | `!=1.0.0` | Any version except `1.0.0` |
| `^` | Compatible | `^1.2.0` | `>=1.2.0, <2.0.0` (Same major) |
| `~` | Close to | `~1.2.0` | `>=1.2.0, <1.3.0` (Same minor) |

## Architecture Overview

The system is designed with modularity in mind, separating concerns into discrete components:

```text
PackageResolver/
├── include/
│   ├── version.h         # Semantic versioning with operator overloading
│   ├── constraint.h      # Version constraint matching logic
│   ├── package.h         # Package metadata definitions
│   ├── json_parser.h     # Native JSON parsing utility
│   ├── manifest.h        # Manifest reading and validation
│   ├── graph.h           # Directed graph and topological sort algorithms
│   ├── registry.h        # Local filesystem package registry management
│   ├── resolver.h        # Core dependency resolution engine
│   ├── installer.h       # Installation simulation logic
│   ├── cli.h             # CLI argument parsing and execution
│   └── exceptions.h      # Custom error handling
├── src/                  # Implementation files
├── registry/             # Default package registry
└── sample_project/       # Example project for testing
```

## License

This project is licensed under the MIT License.
