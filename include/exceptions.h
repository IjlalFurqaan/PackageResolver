#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace pkgr {

class PackageResolverError : public std::runtime_error {
public:
    explicit PackageResolverError(const std::string& message)
        : std::runtime_error(message) {}

    virtual ~PackageResolverError() = default;
};

class PackageNotFoundError : public PackageResolverError {
    std::string package_name_;
public:
    explicit PackageNotFoundError(const std::string& package_name)
        : PackageResolverError("Package not found: " + package_name),
          package_name_(package_name) {}

    [[nodiscard]] const std::string& package_name() const { return package_name_; }
};

class VersionConflictError : public PackageResolverError {
    std::string package_name_;
    std::string constraint_;
public:
    VersionConflictError(const std::string& package_name, 
                         const std::string& constraint)
        : PackageResolverError(
            "No version of '" + package_name + 
            "' satisfies constraint: " + constraint),
          package_name_(package_name),
          constraint_(constraint) {}

    [[nodiscard]] const std::string& package_name() const { return package_name_; }
    [[nodiscard]] const std::string& constraint() const { return constraint_; }
};

class CircularDependencyError : public PackageResolverError {
    std::vector<std::string> cycle_;
public:
    explicit CircularDependencyError(const std::vector<std::string>& cycle)
        : PackageResolverError(build_message(cycle)),
          cycle_(cycle) {}

    [[nodiscard]] const std::vector<std::string>& cycle() const { return cycle_; }

private:
    static std::string build_message(const std::vector<std::string>& cycle) {
        std::string msg = "Circular dependency detected: ";
        bool first = true;
        for (const auto& node : cycle) {
            if (!first) msg += " -> ";
            msg += node;
            first = false;
        }
        return msg;
    }
};

class ManifestParseError : public PackageResolverError {
    std::string file_path_;
    int line_;
public:
    ManifestParseError(const std::string& file_path, int line, 
                       const std::string& detail)
        : PackageResolverError(
            "Parse error in " + file_path + 
            " (line " + std::to_string(line) + "): " + detail),
          file_path_(file_path),
          line_(line) {}

    [[nodiscard]] const std::string& file_path() const { return file_path_; }
    [[nodiscard]] int line() const { return line_; }
};

class JsonParseError : public PackageResolverError {
    int line_;
    int column_;
public:
    JsonParseError(const std::string& detail, int line = 0, int column = 0)
        : PackageResolverError(
            "JSON parse error" + 
            (line > 0 ? " (line " + std::to_string(line) + 
                        ", col " + std::to_string(column) + ")" : "") +
            ": " + detail),
          line_(line),
          column_(column) {}

    [[nodiscard]] int line() const { return line_; }
    [[nodiscard]] int column() const { return column_; }
};

} 