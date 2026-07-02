# 📦 PackageResolver

A C++ package dependency resolver built from scratch — no external libraries. Inspired by npm, pip, and cargo, this tool parses package manifests, builds dependency graphs, performs topological sorting for install order, detects circular dependencies, and resolves version constraints.

## 🚀 Quick Start

### Build
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Run
```bash
# From the project root directory:
./build/bin/pkgr resolve sample_project/manifest.json
./build/bin/pkgr install sample_project/manifest.json
./build/bin/pkgr graph sample_project/manifest.json
./build/bin/pkgr list
./build/bin/pkgr info web-framework
```

## 📋 Commands

| Command | Description |
|---------|-------------|
| `resolve <manifest>` | Resolve dependencies & show install order |
| `install <manifest>` | Resolve + simulate package installation |
| `graph <manifest>` | Show dependency tree visualization |
| `check <manifest>` | Check for circular dependencies |
| `list` | List all available packages in registry |
| `info <package>` | Show package details |
| `help` | Show help message |
| `version` | Show version |

## 🏗️ Architecture

```
PackageResolver/
├── include/              # Header files
│   ├── version.h         # Semantic versioning with operator overloading
│   ├── constraint.h      # Version constraint matching (>=, ^, ~, etc.)
│   ├── package.h         # Package metadata with Rule of Five
│   ├── json_parser.h     # Hand-written JSON parser (no external deps!)
│   ├── manifest.h        # Manifest file reader
│   ├── graph.h           # Template directed graph with algorithms
│   ├── registry.h        # Filesystem-based package registry
│   ├── resolver.h        # Dependency resolution engine
│   ├── installer.h       # Simulated package installer
│   ├── cli.h             # Command-line interface
│   ├── exceptions.h      # Custom exception hierarchy
│   └── colors.h          # Terminal color utilities
├── src/                  # Implementation files
├── registry/             # Sample package registry
└── sample_project/       # Sample project to test with
```

## 📦 Sample Registry

The `registry/` directory contains 8 interconnected packages:

```
logger (1.0.0)           ← leaf, no dependencies
json-parser (2.0.1)      ← leaf
crypto (3.1.0)           ← leaf
http-client (1.5.0)      ← depends on: logger
database (2.3.0)         ← depends on: logger
web-framework (1.2.0)    ← depends on: http-client, json-parser
orm (1.1.0)              ← depends on: database, logger
cache (1.0.0)            ← depends on: logger, crypto
```

## 📝 Manifest Format

```json
{
    "name": "my-project",
    "version": "1.0.0",
    "description": "Optional description",
    "dependencies": {
        "package-name": ">=1.0.0",
        "another-pkg": "^2.0.0"
    }
}
```

### Version Constraints
| Operator | Meaning | Example |
|----------|---------|---------|
| `>=` | Greater than or equal | `>=1.0.0` |
| `<=` | Less than or equal | `<=2.0.0` |
| `>` | Greater than | `>1.0.0` |
| `<` | Less than | `<2.0.0` |
| `=` | Exact match | `=1.2.3` |
| `!=` | Not equal | `!=1.0.0` |
| `^` | Compatible (same major) | `^1.2.0` → `>=1.2.0, <2.0.0` |
| `~` | Close to (same minor) | `~1.2.0` → `>=1.2.0, <1.3.0` |

## 🔧 Requirements

- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16+
- No external dependencies!

## 📄 License

MIT License.
