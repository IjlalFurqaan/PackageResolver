
#include "registry.h"
#include "manifest.h"
#include "exceptions.h"
#include "colors.h"
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

namespace pkgr {

Registry::Registry(const std::string& registry_path) 
    : registry_path_(registry_path) {}

void Registry::load() {
    packages_.clear();

    if (!fs::exists(registry_path_)) {
        throw PackageResolverError("Registry directory not found: " + registry_path_);
    }

    for (const auto& entry : fs::directory_iterator(registry_path_)) {
        if (entry.is_directory()) {
            std::string dir_name = entry.path().filename().string();

            for (const auto& file_entry : fs::directory_iterator(entry.path())) {
                if (file_entry.is_regular_file() && 
                    file_entry.path().extension() == ".json") {
                    try {
                        load_package_file(dir_name, file_entry.path().string());
                    } catch (const std::exception& e) {
                        std::cerr << color::YELLOW << "Warning: " << color::RESET
                                  << "Failed to load " 
                                  << file_entry.path() << ": " << e.what() << "\n";
                    }
                }
            }
        }
    }

    // Sort all version lists once after the full registry is loaded.
    // This is more efficient than sorting on every individual insert.
    for (auto& [name, versions] : packages_) {
        std::sort(versions.begin(), versions.end(),
            [](const std::shared_ptr<Package>& a, const std::shared_ptr<Package>& b) {
                return a->version() > b->version(); // newest first
            });
    }
}

void Registry::load_package_file(const std::string& dir_name, const std::string& file_path) {
    auto pkg = std::make_shared<Package>(ManifestParser::parse(file_path));

    if (pkg->name() != dir_name) {
        std::cerr << color::YELLOW << "Warning: " << color::RESET
                  << "Package name '" << pkg->name() 
                  << "' doesn't match directory '" << dir_name << "'\n";
    }

    packages_[pkg->name()].push_back(pkg);
    // Version sorting is deferred to load() for efficiency.
}

std::optional<std::shared_ptr<Package>> Registry::find_package(
    const std::string& name, 
    const VersionConstraint& constraint) const 
{
    auto it = packages_.find(name);
    if (it == packages_.end()) {
        return std::nullopt;
    }

    const auto& versions = it->second;
    auto found = std::find_if(versions.begin(), versions.end(),
        [&constraint](const std::shared_ptr<Package>& pkg) {
            return constraint.satisfies(pkg->version());
        });

    if (found != versions.end()) {
        return *found;
    }

    return std::nullopt;
}

std::vector<std::shared_ptr<Package>> Registry::get_all_versions(
    const std::string& name) const 
{
    auto it = packages_.find(name);
    if (it == packages_.end()) {
        return {};
    }
    return it->second;
}

std::vector<std::string> Registry::get_all_names() const {
    std::vector<std::string> names;
    names.reserve(packages_.size());

    for (const auto& [name, _] : packages_) {
        names.push_back(name);
    }

    std::sort(names.begin(), names.end());
    return names;
}

bool Registry::has_package(const std::string& name) const {
    return packages_.find(name) != packages_.end();
}

size_t Registry::version_count(const std::string& name) const {
    auto it = packages_.find(name);
    if (it == packages_.end()) return 0;
    return it->second.size();
}

size_t Registry::total_packages() const {
    size_t count = 0;
    for (const auto& [_, versions] : packages_) {
        count += versions.size();
    }
    return count;
}

} 