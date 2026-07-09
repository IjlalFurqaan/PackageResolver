#pragma once

#include "package.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>

namespace pkgr {

class Registry {
public:

    explicit Registry(const std::string& registry_path);

    void load();

    std::optional<std::shared_ptr<Package>> find_package(
        const std::string& name, 
        const VersionConstraint& constraint) const;

    std::vector<std::shared_ptr<Package>> get_all_versions(
        const std::string& name) const;

    std::vector<std::string> get_all_names() const;

    bool has_package(const std::string& name) const;

    size_t total_packages() const;

    size_t version_count(const std::string& name) const;

private:
    std::string registry_path_;

    std::unordered_map<std::string, std::vector<std::shared_ptr<Package>>> packages_;

    void load_package_file(const std::string& dir_name, const std::string& file_path);
};

} 