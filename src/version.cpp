
#include "version.h"
#include "exceptions.h"
#include <sstream>
#include <stdexcept>

namespace pkgr {

Version::Version() : major_(0), minor_(0), patch_(0) {}

Version::Version(int major, int minor, int patch) 
    : major_(major), minor_(minor), patch_(patch) 
{
    // Guard the direct-integer path; parse() handles the string path independently.
    if (major < 0 || minor < 0 || patch < 0) {
        throw std::invalid_argument("Version components cannot be negative");
    }
}

Version::Version(const std::string& version_str) {
    parse(version_str);
}

bool Version::operator==(const Version& other) const {
    return major_ == other.major_ && 
           minor_ == other.minor_ && 
           patch_ == other.patch_;
}

bool Version::operator!=(const Version& other) const {
    return !(*this == other);
}

bool Version::operator<(const Version& other) const {
    if (major_ != other.major_) return major_ < other.major_;
    if (minor_ != other.minor_) return minor_ < other.minor_;
    return patch_ < other.patch_;
}

bool Version::operator>(const Version& other) const {
    return other < *this;
}

bool Version::operator<=(const Version& other) const {
    return !(other < *this);
}

bool Version::operator>=(const Version& other) const {
    return !(*this < other);
}

std::string Version::to_string() const {
    return std::to_string(major_) + "." + 
           std::to_string(minor_) + "." + 
           std::to_string(patch_);
}

std::ostream& operator<<(std::ostream& os, const Version& v) {
    os << v.to_string();
    return os;
}

void Version::parse(const std::string& version_str) {

    std::string str = version_str;
    if (!str.empty() && (str[0] == 'v' || str[0] == 'V')) {
        str = str.substr(1);
    }

    std::istringstream iss(str);
    char dot1, dot2;

    if (!(iss >> major_ >> dot1 >> minor_ >> dot2 >> patch_) ||
        dot1 != '.' || dot2 != '.') {
        throw std::invalid_argument(
            "Invalid version format: '" + version_str + 
            "' (expected 'major.minor.patch')");
    }

    if (major_ < 0 || minor_ < 0 || patch_ < 0) {
        throw std::invalid_argument(
            "Version components cannot be negative: " + version_str);
    }
}

} 