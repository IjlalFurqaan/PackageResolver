#include "test_framework.h"
#include "constraint.h"

#include <stdexcept>
#include <string>

using pkgr::Version;
using pkgr::VersionConstraint;

namespace {

bool satisfied(const std::string& constraint, const std::string& version) {
    return VersionConstraint(constraint).satisfies(Version(version));
}

}  // namespace

TEST(constraint, greater_equal) {
    EXPECT_TRUE(satisfied(">=1.5.0", "1.5.0"));
    EXPECT_TRUE(satisfied(">=1.5.0", "2.0.0"));
    EXPECT_FALSE(satisfied(">=1.5.0", "1.4.9"));
}

TEST(constraint, less_equal) {
    EXPECT_TRUE(satisfied("<=2.0.0", "2.0.0"));
    EXPECT_TRUE(satisfied("<=2.0.0", "1.9.9"));
    EXPECT_FALSE(satisfied("<=2.0.0", "2.0.1"));
}

TEST(constraint, strictly_greater) {
    EXPECT_TRUE(satisfied(">1.0.0", "1.0.1"));
    EXPECT_FALSE(satisfied(">1.0.0", "1.0.0"));
}

TEST(constraint, strictly_less) {
    EXPECT_TRUE(satisfied("<2.0.0", "1.9.9"));
    EXPECT_FALSE(satisfied("<2.0.0", "2.0.0"));
}

TEST(constraint, not_equal) {
    EXPECT_TRUE(satisfied("!=1.0.0", "1.0.1"));
    EXPECT_FALSE(satisfied("!=1.0.0", "1.0.0"));
}

TEST(constraint, explicit_equals) {
    EXPECT_TRUE(satisfied("=1.2.3", "1.2.3"));
    EXPECT_FALSE(satisfied("=1.2.3", "1.3.0"));
}

TEST(constraint, bare_version_means_exact_match) {
    EXPECT_TRUE(satisfied("1.2.3", "1.2.3"));
    EXPECT_FALSE(satisfied("1.2.3", "1.2.4"));
}

TEST(constraint, caret_allows_same_major) {
    EXPECT_TRUE(satisfied("^1.2.0", "1.2.0"));
    EXPECT_TRUE(satisfied("^1.2.0", "1.9.9"));
    EXPECT_FALSE(satisfied("^1.2.0", "2.0.0"));
    EXPECT_FALSE(satisfied("^1.2.0", "1.1.9"));
}

TEST(constraint, caret_zero_major_pins_minor) {
    // semver: ^0.2.3 means >=0.2.3 <0.3.0
    EXPECT_TRUE(satisfied("^0.2.3", "0.2.3"));
    EXPECT_TRUE(satisfied("^0.2.3", "0.2.9"));
    EXPECT_FALSE(satisfied("^0.2.3", "0.3.0"));
    EXPECT_FALSE(satisfied("^0.2.3", "0.2.2"));
}

TEST(constraint, caret_zero_zero_requires_exact_patch) {
    // semver: ^0.0.3 means >=0.0.3 <0.0.4
    EXPECT_TRUE(satisfied("^0.0.3", "0.0.3"));
    EXPECT_FALSE(satisfied("^0.0.3", "0.0.4"));
    EXPECT_FALSE(satisfied("^0.0.3", "0.1.0"));
}

TEST(constraint, tilde_pins_minor) {
    EXPECT_TRUE(satisfied("~1.2.0", "1.2.0"));
    EXPECT_TRUE(satisfied("~1.2.0", "1.2.9"));
    EXPECT_FALSE(satisfied("~1.2.0", "1.3.0"));
    EXPECT_FALSE(satisfied("~1.2.0", "1.1.9"));
}

TEST(constraint, wildcard_matches_everything) {
    EXPECT_TRUE(satisfied("*", "0.0.1"));
    EXPECT_TRUE(satisfied("*", "1.2.3"));
    EXPECT_TRUE(satisfied("*", "99.99.99"));
}

TEST(constraint, trims_surrounding_whitespace) {
    EXPECT_TRUE(satisfied("  >=1.0.0  ", "1.0.0"));
    EXPECT_TRUE(satisfied("\t^2.0.0", "2.5.0"));
}

TEST(constraint, empty_string_throws) {
    EXPECT_THROWS(VersionConstraint(""), std::invalid_argument);
    EXPECT_THROWS(VersionConstraint("   "), std::invalid_argument);
}

TEST(constraint, to_string_round_trips) {
    EXPECT_EQ(VersionConstraint(">=1.5.0").to_string(), std::string(">=1.5.0"));
    EXPECT_EQ(VersionConstraint("^2.0.0").to_string(), std::string("^2.0.0"));
    EXPECT_EQ(VersionConstraint("~1.2.0").to_string(), std::string("~1.2.0"));
    EXPECT_EQ(VersionConstraint("*").to_string(), std::string("*"));
}
