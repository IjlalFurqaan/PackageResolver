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

## 🎯 C++ Concepts Covered (Interview Prep)

This project naturally uses every major beginner-to-intermediate C++ concept. Use this as a study map:

### Beginner
| Concept | Where to Find It |
|---------|-------------------|
| Classes & Structs | `Version`, `Package`, `Dependency`, every component |
| Enums / `enum class` | `ConstraintOp`, `TokenType`, `JsonType`, `Command` |
| Constructors & Destructors | All classes — parameterized, default, explicit |
| References & Pointers | Graph traversal, resolver, registry lookups |
| String manipulation | Version parsing, constraint parsing, JSON tokenizer |
| File I/O (`fstream`) | `ManifestParser`, `JsonParser::parse_file` |
| STL containers | `vector`, `map`, `unordered_map`, `unordered_set`, `queue`, `stack` |
| Control flow & loops | Everywhere — switch, if/else, range-for, while |
| Namespaces | `pkgr::` namespace throughout |

### Intermediate
| Concept | Where to Find It |
|---------|-------------------|
| Inheritance & Polymorphism | `JsonValue` hierarchy (5 subclasses), Exception hierarchy |
| Templates | `Graph<T>` — full template class with algorithms |
| Smart pointers | `unique_ptr` (JSON AST), `shared_ptr` (Registry packages) |
| RAII | File handles in parser, graph cleanup |
| Move semantics | `Package` constructor, `std::move` throughout |
| Operator overloading | `Version` (all 6 comparisons + `<<`), `VersionConstraint` |
| Exception handling | Custom hierarchy, try/catch in CLI |
| Const correctness | Every getter, const methods, const references |
| Lambdas | Registry sorting, `std::find_if`, `std::sort` |
| Rule of Five | `Package` class (copy/move constructors & assignments) |
| `std::optional` | `Registry::find_package` return type |
| Design patterns | Strategy (constraint ops), Factory-like (parser), Composition |
| Graph algorithms | DFS, BFS, topological sort (Kahn's), cycle detection |
| STL algorithms | `std::sort`, `std::find`, `std::find_if`, `std::reverse` |

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

MIT — built for learning and interview preparation.
