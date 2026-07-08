#pragma once

#include "version.h"
#include "constraint.h"
#include <string>
#include <vector>
#include <iostream>

namespace pkgr {

struct Dependency {
    std::string name;
    VersionConstraint constraint;

    Dependency(std::string dep_name, VersionConstraint dep_constraint)
        : name(std::move(dep_name)), constraint(std::move(dep_constraint)) {}
};

class Package {
public:

    Package(std::string name, Version version, std::string description,
            std::vector<Dependency> dependencies = {});

    Package(const Package& other);

    Package(Package&& other) noexcept;

    Package& operator=(const Package& other);

    Package& operator=(Package&& other) noexcept;

    ~Package() = default;

    const std::string& name() const { return name_; }
    const Version& version() const { return version_; }
    const std::string& description() const { return description_; }
    const std::string& author() const { return author_; }
    const std::vector<Dependency>& dependencies() const { return dependencies_; }

    std::string id() const;

    bool has_dependencies() const { return !dependencies_.empty(); }

    friend std::ostream& operator<<(std::ostream& os, const Package& pkg);

private:
    std::string name_;
    Version version_;
    std::string description_;
    std::string author_;
    std::vector<Dependency> dependencies_;
};

} 