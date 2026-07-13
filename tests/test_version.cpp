#include "test_framework.h"
#include "version.h"

#include <stdexcept>
#include <string>

using pkgr::Version;

TEST(version, default_constructs_to_zero) {
    Version v;
    EXPECT_EQ(v.major(), 0);
    EXPECT_EQ(v.minor(), 0);
    EXPECT_EQ(v.patch(), 0);
    EXPECT_EQ(v.to_string(), std::string("0.0.0"));
}

TEST(version, parses_major_minor_patch) {
    Version v("1.2.3");
    EXPECT_EQ(v.major(), 1);
    EXPECT_EQ(v.minor(), 2);
    EXPECT_EQ(v.patch(), 3);
    EXPECT_EQ(v.to_string(), std::string("1.2.3"));
}

TEST(version, accepts_v_prefix) {
    EXPECT_EQ(Version("v2.10.7"), Version(2, 10, 7));
    EXPECT_EQ(Version("V0.1.0"), Version(0, 1, 0));
}

TEST(version, orders_by_major_then_minor_then_patch) {
    EXPECT_TRUE(Version("1.0.0") < Version("1.0.1"));
    EXPECT_TRUE(Version("1.0.9") < Version("1.1.0"));
    EXPECT_TRUE(Version("1.9.9") < Version("2.0.0"));
    EXPECT_TRUE(Version("2.0.0") > Version("1.9.9"));
    EXPECT_TRUE(Version("1.2.3") <= Version("1.2.3"));
    EXPECT_TRUE(Version("1.2.3") >= Version("1.2.3"));
    EXPECT_TRUE(Version("1.2.3") == Version("1.2.3"));
    EXPECT_TRUE(Version("1.2.3") != Version("1.2.4"));
    EXPECT_FALSE(Version("1.2.3") < Version("1.2.3"));
}

TEST(version, prerelease_is_any_zero_major_version) {
    EXPECT_TRUE(Version("0.9.1").is_prerelease());
    EXPECT_TRUE(Version("0.0.1").is_prerelease());
    EXPECT_FALSE(Version("1.0.0").is_prerelease());
}

TEST(version, rejects_malformed_strings) {
    EXPECT_THROWS(Version(""), std::invalid_argument);
    EXPECT_THROWS(Version("1.2"), std::invalid_argument);
    EXPECT_THROWS(Version("not-a-version"), std::invalid_argument);
    EXPECT_THROWS(Version("1.2.x"), std::invalid_argument);
}

TEST(version, rejects_negative_components) {
    EXPECT_THROWS(Version(1, -1, 0), std::invalid_argument);
    EXPECT_THROWS(Version("1.-2.3"), std::invalid_argument);
}
