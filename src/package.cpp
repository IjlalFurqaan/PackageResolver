
#include "package.h"
#include <utility>

namespace pkgr {

Package::Package(std::string name, Version version, std::string description,
                 std::vector<Dependency> dependencies)
    : name_(std::move(name)),
      version_(std::move(version)),
      description_(std::move(description)),
      dependencies_(std::move(dependencies))
{}

Package::Package(const Package& other)
    : name_(other.name_),
      version_(other.version_),
      description_(other.description_),
      dependencies_(other.dependencies_)
{}

Package::Package(Package&& other) noexcept
    : name_(std::move(other.name_)),
      version_(std::move(other.version_)),
      description_(std::move(other.description_)),
      dependencies_(std::move(other.dependencies_))
{}

Package& Package::operator=(const Package& other) {
    if (this != &other) {
        name_ = other.name_;
        version_ = other.version_;
        description_ = other.description_;
        dependencies_ = other.dependencies_;
    }
    return *this;
}

Package& Package::operator=(Package&& other) noexcept {
    if (this != &other) {
        name_ = std::move(other.name_);
        version_ = std::move(other.version_);
        description_ = std::move(other.description_);
        dependencies_ = std::move(other.dependencies_);
    }
    return *this;
}

std::string Package::id() const {
    return name_ + "@" + version_.to_string();
}

std::ostream& operator<<(std::ostream& os, const Package& pkg) {
    os << pkg.name_ << "@" << pkg.version_;
    if (!pkg.description_.empty()) {
        os << " — " << pkg.description_;
    }
    return os;
}

} 