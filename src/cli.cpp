
#include "cli.h"
#include "manifest.h"
#include "exceptions.h"
#include "colors.h"
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace pkgr {

CLI::CLI(int argc, char* argv[]) {
    executable_path_ = argv[0];
    for (int i = 1; i < argc; ++i) {
        args_.emplace_back(argv[i]);
    }
}

int CLI::run() {
    using namespace color;

    enable_colors();

    if (args_.empty()) {
        print_banner();
        cmd_help();
        return 0;
    }

    Command cmd = parse_command();

    try {
        switch (cmd) {
            case Command::RESOLVE:
                if (args_.size() < 2) {
                    std::cerr << RED << "  Error: " << RESET 
                              << "Missing manifest path\n";
                    std::cerr << DIM << "  Usage: pkgr resolve <manifest.json>\n" << RESET;
                    return 1;
                }
                return cmd_resolve(args_[1]);

            case Command::INSTALL:
                if (args_.size() < 2) {
                    std::cerr << RED << "  Error: " << RESET 
                              << "Missing manifest path\n";
                    return 1;
                }
                return cmd_install(args_[1]);

            case Command::GRAPH:
                if (args_.size() < 2) {
                    std::cerr << RED << "  Error: " << RESET 
                              << "Missing manifest path\n";
                    return 1;
                }
                return cmd_graph(args_[1]);

            case Command::CHECK:
                if (args_.size() < 2) {
                    std::cerr << RED << "  Error: " << RESET 
                              << "Missing manifest path\n";
                    return 1;
                }
                return cmd_check(args_[1]);

            case Command::LIST:
                return cmd_list();

            case Command::INFO:
                if (args_.size() < 2) {
                    std::cerr << RED << "  Error: " << RESET 
                              << "Missing package name\n";
                    return 1;
                }
                return cmd_info(args_[1]);

            case Command::HELP:
                print_banner();
                cmd_help();
                return 0;

            case Command::VERSION:
                cmd_version();
                return 0;

            case Command::UNKNOWN:
                std::cerr << RED << "  Unknown command: " << RESET 
                          << args_[0] << "\n";
                cmd_help();
                return 1;
        }
    } catch (const CircularDependencyError& e) {
        std::cerr << "\n" << BOLD_RED << "  ⛔ " << e.what() << RESET << "\n\n";
        return 1;
    } catch (const PackageNotFoundError& e) {
        std::cerr << "\n" << BOLD_RED << "  ❌ " << e.what() << RESET << "\n";
        std::cerr << DIM << "  Check if the package exists in the registry.\n" << RESET << "\n";
        return 1;
    } catch (const VersionConflictError& e) {
        std::cerr << "\n" << BOLD_RED << "  ❌ " << e.what() << RESET << "\n";
        std::cerr << DIM << "  Try relaxing the version constraint.\n" << RESET << "\n";
        return 1;
    } catch (const JsonParseError& e) {
        std::cerr << "\n" << BOLD_RED << "  ❌ " << e.what() << RESET << "\n\n";
        return 1;
    } catch (const PackageResolverError& e) {
        std::cerr << "\n" << BOLD_RED << "  Error: " << e.what() << RESET << "\n\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "\n" << BOLD_RED << "  Unexpected error: " << e.what() << RESET << "\n\n";
        return 1;
    }

    return 0;
}

Command CLI::parse_command() const {
    if (args_.empty()) return Command::HELP;

    const auto& cmd = args_[0];

    if (cmd == "resolve"  || cmd == "r")       return Command::RESOLVE;
    if (cmd == "install"  || cmd == "i")       return Command::INSTALL;
    if (cmd == "graph"    || cmd == "g"  || cmd == "tree") return Command::GRAPH;
    if (cmd == "check"    || cmd == "c")       return Command::CHECK;
    if (cmd == "list"     || cmd == "ls" || cmd == "l")    return Command::LIST;
    if (cmd == "info")                         return Command::INFO;
    if (cmd == "help"     || cmd == "-h" || cmd == "--help") return Command::HELP;
    if (cmd == "version"  || cmd == "-v" || cmd == "--version") return Command::VERSION;

    return Command::UNKNOWN;
}

int CLI::cmd_resolve(const std::string& manifest_path) {
    using namespace color;

    auto registry = load_registry();
    auto root_pkg = ManifestParser::parse(manifest_path);

    std::cout << "\n" << BOLD_CYAN << "  📦 Resolving dependencies for: " 
              << RESET << BOLD_WHITE << root_pkg.id() << RESET << "\n\n";

    Resolver resolver(registry);
    auto resolved = resolver.resolve(root_pkg);

    if (resolved.empty()) {
        std::cout << GREEN << "  ✅ No dependencies to install\n" << RESET << "\n";
        return 0;
    }

    std::cout << BOLD << "  📋 Install Order (topological sort):\n" << RESET;

    int index = 1;
    for (const auto& pkg : resolved) {
        std::string depth_indicator;
        for (int d = 0; d < pkg.depth; ++d) {
            depth_indicator += "  ";
        }

        std::cout << GRAY << "  " << std::setw(3) << index << ". " << RESET
                  << BOLD_WHITE << pkg.name << RESET
                  << CYAN << "@" << pkg.version << RESET;

        if (pkg.depth > 0) {
            std::cout << DIM << "  (transitive, depth " << pkg.depth << ")" << RESET;
        } else {
            std::cout << GREEN << "  (direct)" << RESET;
        }

        std::cout << "\n";
        ++index;
    }

    std::cout << "\n" << BOLD_GREEN << "  ✅ " << resolved.size() 
              << " packages resolved successfully" << RESET << "\n\n";

    return 0;
}

int CLI::cmd_install(const std::string& manifest_path) {
    using namespace color;

    auto registry = load_registry();
    auto root_pkg = ManifestParser::parse(manifest_path);

    std::cout << "\n" << BOLD_CYAN << "  📦 Resolving dependencies for: " 
              << RESET << BOLD_WHITE << root_pkg.id() << RESET << "\n";

    Resolver resolver(registry);
    auto resolved = resolver.resolve(root_pkg);

    if (resolved.empty()) {
        std::cout << GREEN << "\n  ✅ No dependencies to install\n" << RESET << "\n";
        return 0;
    }

    std::cout << GREEN << "  ✓ " << resolved.size() << " packages resolved" 
              << RESET << "\n";

    Installer installer;
    installer.install(resolved);

    return 0;
}

int CLI::cmd_graph(const std::string& manifest_path) {
    using namespace color;

    auto registry = load_registry();
    auto root_pkg = ManifestParser::parse(manifest_path);

    std::cout << "\n" << BOLD_CYAN << "  🌳 Dependency Tree for: " 
              << RESET << BOLD_WHITE << root_pkg.id() << RESET << "\n\n";

    std::cout << "  " << BOLD_WHITE << root_pkg.name() << "@" << root_pkg.version() 
              << RESET << "\n";

    print_dependency_tree(root_pkg, registry, "  ", true, 0);

    std::cout << "\n";
    return 0;
}

int CLI::cmd_check(const std::string& manifest_path) {
    using namespace color;

    auto registry = load_registry();
    auto root_pkg = ManifestParser::parse(manifest_path);

    std::cout << "\n" << BOLD_CYAN << "  🔍 Checking dependencies for: " 
              << RESET << BOLD_WHITE << root_pkg.id() << RESET << "\n\n";

    Resolver resolver(registry);

    try {
        auto resolved = resolver.resolve(root_pkg);

        std::cout << GREEN << "  ✅ No circular dependencies detected\n" << RESET;
        std::cout << DIM << "  " << resolved.size() << " packages checked\n" << RESET;

    } catch (const CircularDependencyError& e) {
        std::cout << BOLD_RED << "  ⛔ " << e.what() << RESET << "\n";

        auto cycle = e.cycle();
        std::cout << "\n" << YELLOW << "  Cycle path:\n" << RESET;
        for (size_t i = 0; i < cycle.size(); ++i) {
            std::cout << "    " << BOLD_WHITE << cycle[i] << RESET;
            if (i < cycle.size() - 1) {
                std::cout << RED << " → " << RESET;
            }
        }
        std::cout << "\n";
    }

    std::cout << "\n";
    return 0;
}

int CLI::cmd_list() {
    using namespace color;

    auto registry = load_registry();
    auto names = registry.get_all_names();

    std::cout << "\n" << BOLD_CYAN << "  📚 Available Packages (" 
              << names.size() << " packages, " 
              << registry.total_packages() << " versions)\n" << RESET << "\n";

    for (const auto& name : names) {
        auto versions = registry.get_all_versions(name);

        std::cout << "  " << BOLD_WHITE << name << RESET;

        if (!versions.empty()) {
            std::cout << GRAY << "  (";
            for (size_t i = 0; i < versions.size(); ++i) {
                std::cout << versions[i]->version();
                if (i < versions.size() - 1) std::cout << ", ";
            }
            std::cout << ")" << RESET;

            if (!versions[0]->description().empty()) {
                std::cout << "\n    " << DIM << versions[0]->description() << RESET;
            }
        }

        std::cout << "\n";
    }

    std::cout << "\n";
    return 0;
}

int CLI::cmd_info(const std::string& package_name) {
    using namespace color;

    auto registry = load_registry();
    auto versions = registry.get_all_versions(package_name);

    if (versions.empty()) {
        std::cerr << RED << "\n  ❌ Package not found: " << RESET 
                  << package_name << "\n\n";
        return 1;
    }

    const auto& latest = versions[0];

    std::cout << "\n" << BOLD_CYAN << "  📦 " << RESET 
              << BOLD_WHITE << latest->name() << RESET << "\n\n";

    std::cout << "  " << BOLD << "Version:     " << RESET << latest->version() << "\n";
    std::cout << "  " << BOLD << "Description: " << RESET << latest->description() << "\n";

    std::cout << "  " << BOLD << "Versions:    " << RESET;
    for (size_t i = 0; i < versions.size(); ++i) {
        std::cout << versions[i]->version();
        if (i == 0) std::cout << GREEN << " (latest)" << RESET;
        if (i < versions.size() - 1) std::cout << ", ";
    }
    std::cout << "\n";

    if (latest->has_dependencies()) {
        std::cout << "  " << BOLD << "Dependencies:" << RESET << "\n";
        for (const auto& dep : latest->dependencies()) {
            std::cout << "    " << CYAN << "├── " << RESET 
                      << dep.name << " " << YELLOW << dep.constraint << RESET << "\n";
        }
    } else {
        std::cout << "  " << BOLD << "Dependencies: " << RESET 
                  << DIM << "none" << RESET << "\n";
    }

    std::cout << "\n";
    return 0;
}

void CLI::cmd_help() const {
    using namespace color;

    std::cout << "\n" << BOLD << "  Usage:" << RESET << " pkgr <command> [options]\n\n";
    std::cout << BOLD << "  Commands:\n" << RESET;
    std::cout << "    " << CYAN << "resolve" << RESET << " <manifest>   Resolve dependencies & show install order\n";
    std::cout << "    " << CYAN << "install" << RESET << " <manifest>   Resolve + simulate package installation\n";
    std::cout << "    " << CYAN << "graph  " << RESET << " <manifest>   Show dependency tree visualization\n";
    std::cout << "    " << CYAN << "check  " << RESET << " <manifest>   Check for circular dependencies\n";
    std::cout << "    " << CYAN << "list   " << RESET << "              List all available packages\n";
    std::cout << "    " << CYAN << "info   " << RESET << " <package>    Show package details\n";
    std::cout << "    " << CYAN << "help   " << RESET << "              Show this help message\n";
    std::cout << "    " << CYAN << "version" << RESET << "              Show version\n";
    std::cout << "\n" << BOLD << "  Examples:\n" << RESET;
    std::cout << DIM << "    pkgr resolve sample_project/manifest.json\n";
    std::cout << "    pkgr install sample_project/manifest.json\n";
    std::cout << "    pkgr graph sample_project/manifest.json\n";
    std::cout << "    pkgr info web-framework\n" << RESET << "\n";
}

void CLI::cmd_version() const {
    using namespace color;
    std::cout << BOLD_CYAN << "  pkgr" << RESET << " version " 
              << BOLD << "1.0.0" << RESET << "\n";
}

void CLI::print_banner() const {
    using namespace color;
    std::cout << "\n";
    std::cout << BOLD_CYAN << "  ╔═══════════════════════════════════════╗\n";
    std::cout << "  ║   📦  PackageResolver v1.0.0          ║\n";
    std::cout << "  ║   Dependency Resolution Engine         ║\n";
    std::cout << "  ╚═══════════════════════════════════════╝" << RESET << "\n";
}

Registry CLI::load_registry() const {
    std::string reg_path = get_registry_path();
    Registry registry(reg_path);
    registry.load();
    return registry;
}

std::string CLI::get_registry_path() const {

    if (fs::exists("registry")) {
        return "registry";
    }

    fs::path exe_dir = fs::path(executable_path_).parent_path();
    fs::path reg_path = exe_dir / ".." / ".." / "registry";
    if (fs::exists(reg_path)) {
        return reg_path.string();
    }

    reg_path = exe_dir / ".." / "registry";
    if (fs::exists(reg_path)) {
        return reg_path.string();
    }

    throw PackageResolverError(
        "Registry directory not found. Expected 'registry/' in current directory.");
}

void CLI::print_dependency_tree(const Package& pkg, const Registry& registry,
                                 const std::string& prefix, bool is_last,
                                 int depth) const {
    using namespace color;

    (void)is_last; 
    const auto& deps = pkg.dependencies();

    for (size_t i = 0; i < deps.size(); ++i) {
        bool last = (i == deps.size() - 1);

        std::string connector = last ? "└── " : "├── ";
        std::string extension = last ? "    " : "│   ";

        auto found = registry.find_package(deps[i].name, deps[i].constraint);

        std::cout << prefix << GRAY << connector << RESET;
        std::cout << BOLD_WHITE << deps[i].name << RESET;

        if (found.has_value()) {
            std::cout << CYAN << "@" << found.value()->version() << RESET;

            std::cout << DIM << "  (" << deps[i].constraint << ")" << RESET;

            std::cout << "\n";

            if (depth < 10 && found.value()->has_dependencies()) {
                print_dependency_tree(*found.value(), registry, 
                                     prefix + extension, last, depth + 1);
            }
        } else {
            std::cout << RED << "  ✗ not found" << RESET << "\n";
        }
    }
}

} 