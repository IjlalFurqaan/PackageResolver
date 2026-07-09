#pragma once

#include <string>
#include <iostream>

namespace pkgr {

class Version {
public:

    Version();

    Version(int major, int minor, int patch);

    explicit Version(const std::string& version_str);

    int major() const { return major_; }
    int minor() const { return minor_; }
    int patch() const { return patch_; }

    /// Returns true for 0.x.y versions (pre-1.0 semver).
    [[nodiscard]] bool is_prerelease() const { return major_ == 0; }

    [[nodiscard]] bool operator==(const Version& other) const;
    [[nodiscard]] bool operator!=(const Version& other) const;
    [[nodiscard]] bool operator<(const Version& other) const;
    [[nodiscard]] bool operator>(const Version& other) const;
    [[nodiscard]] bool operator<=(const Version& other) const;
    [[nodiscard]] bool operator>=(const Version& other) const;

    [[nodiscard]] std::string to_string() const;

    friend std::ostream& operator<<(std::ostream& os, const Version& v);

private:
    int major_ = 0;
    int minor_ = 0;
    int patch_ = 0;

    void parse(const std::string& version_str);
};

} 