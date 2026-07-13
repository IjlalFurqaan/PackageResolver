#include "test_framework.h"
#include "exceptions.h"
#include "package.h"
#include "registry.h"
#include "resolver.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using pkgr::CircularDependencyError;
using pkgr::Dependency;
using pkgr::Package;
using pkgr::PackageNotFoundError;
using pkgr::Registry;
using pkgr::ResolvedPackage;
using pkgr::Resolver;
using pkgr::Version;
using pkgr::VersionConflictError;
using pkgr::VersionConstraint;

namespace {

// Writes manifest fixtures into a throwaway on-disk registry so tests
// exercise the real Registry/JSON loading path end to end.
class TempRegistry {
public:
    TempRegistry() : root_(fs::temp_directory_path() / "pkgr_test_registry") {
        fs::remove_all(root_);
        fs::create_directories(root_);
    }

    ~TempRegistry() {
        std::error_code ec;
        fs::remove_all(root_, ec);  // best effort; never throw from a dtor
    }

    void add(const std::string& name, const std::string& version,
             const std::string& deps_json = "{}") {
        fs::create_directories(root_ / name);
        std::ofstream out(root_ / name / (version + ".json"));
        out << "{\n"
            << "    \"name\": \"" << name << "\",\n"
            << "    \"version\": \"" << version << "\",\n"
            << "    \"description\": \"test fixture\",\n"
            << "    \"dependencies\": " << deps_json << "\n"
            << "}\n";
    }

    Registry load() const {
        Registry registry(root_.string());
        registry.load();
        return registry;
    }

private:
    fs::path root_;
};

Package make_root(std::vector<Dependency> deps) {
    return Package("test-app", Version(1, 0, 0), "test root", std::move(deps));
}

long position_of(const std::vector<ResolvedPackage>& resolved,
                 const std::string& name) {
    for (size_t i = 0; i < resolved.size(); ++i) {
        if (resolved[i].name == name) return static_cast<long>(i);
    }
    return -1;
}

}  // namespace

TEST(resolver, installs_dependencies_before_dependents) {
    // Regression test: the install order used to be reversed.
    // app -> a -> {b, c}; b -> c
    TempRegistry fixtures;
    fixtures.add("a", "1.0.0", R"({"b": ">=1.0.0", "c": ">=1.0.0"})");
    fixtures.add("b", "1.0.0", R"({"c": ">=1.0.0"})");
    fixtures.add("c", "1.0.0");
    auto registry = fixtures.load();

    Resolver resolver(registry);
    auto result = resolver.resolve(
        make_root({Dependency("a", VersionConstraint(">=1.0.0"))}));

    EXPECT_EQ(result.size(), size_t(3));
    EXPECT_TRUE(position_of(result, "c") < position_of(result, "b"));
    EXPECT_TRUE(position_of(result, "b") < position_of(result, "a"));
}

TEST(resolver, excludes_root_from_install_order) {
    TempRegistry fixtures;
    fixtures.add("a", "1.0.0");
    auto registry = fixtures.load();

    Resolver resolver(registry);
    auto result = resolver.resolve(
        make_root({Dependency("a", VersionConstraint(">=1.0.0"))}));

    EXPECT_EQ(result.size(), size_t(1));
    EXPECT_EQ(result[0].name, std::string("a"));
    EXPECT_EQ(position_of(result, "test-app"), long(-1));
}

TEST(resolver, picks_newest_satisfying_version) {
    TempRegistry fixtures;
    fixtures.add("lib", "1.0.0");
    fixtures.add("lib", "1.5.0");
    fixtures.add("lib", "2.0.0");
    auto registry = fixtures.load();

    Resolver resolver(registry);
    auto result = resolver.resolve(
        make_root({Dependency("lib", VersionConstraint("^1.0.0"))}));

    EXPECT_EQ(result.size(), size_t(1));
    EXPECT_EQ(result[0].version, Version(1, 5, 0));
}

TEST(resolver, shared_dependency_is_resolved_once) {
    // Diamond: app -> {x, y}; x -> z; y -> z
    TempRegistry fixtures;
    fixtures.add("x", "1.0.0", R"({"z": ">=1.0.0"})");
    fixtures.add("y", "1.0.0", R"({"z": ">=1.0.0"})");
    fixtures.add("z", "1.0.0");
    auto registry = fixtures.load();

    Resolver resolver(registry);
    auto result = resolver.resolve(
        make_root({Dependency("x", VersionConstraint(">=1.0.0")),
                   Dependency("y", VersionConstraint(">=1.0.0"))}));

    EXPECT_EQ(result.size(), size_t(3));
    EXPECT_TRUE(position_of(result, "z") < position_of(result, "x"));
    EXPECT_TRUE(position_of(result, "z") < position_of(result, "y"));
}

TEST(resolver, reports_resolution_depth) {
    TempRegistry fixtures;
    fixtures.add("outer", "1.0.0", R"({"inner": ">=1.0.0"})");
    fixtures.add("inner", "1.0.0");
    auto registry = fixtures.load();

    Resolver resolver(registry);
    auto result = resolver.resolve(
        make_root({Dependency("outer", VersionConstraint(">=1.0.0"))}));

    EXPECT_EQ(result.size(), size_t(2));
    long outer = position_of(result, "outer");
    long inner = position_of(result, "inner");
    EXPECT_EQ(result[static_cast<size_t>(outer)].depth, 1);
    EXPECT_EQ(result[static_cast<size_t>(inner)].depth, 2);
}

TEST(resolver, throws_when_package_is_missing) {
    TempRegistry fixtures;
    fixtures.add("a", "1.0.0", R"({"no-such-package": ">=1.0.0"})");
    auto registry = fixtures.load();

    Resolver resolver(registry);
    EXPECT_THROWS(
        resolver.resolve(make_root({Dependency("a", VersionConstraint(">=1.0.0"))})),
        PackageNotFoundError);
}

TEST(resolver, throws_when_no_version_satisfies_constraint) {
    TempRegistry fixtures;
    fixtures.add("lib", "1.0.0");
    auto registry = fixtures.load();

    Resolver resolver(registry);
    EXPECT_THROWS(
        resolver.resolve(make_root({Dependency("lib", VersionConstraint(">=2.0.0"))})),
        VersionConflictError);
}

TEST(resolver, throws_on_circular_dependencies) {
    TempRegistry fixtures;
    fixtures.add("ping", "1.0.0", R"({"pong": ">=1.0.0"})");
    fixtures.add("pong", "1.0.0", R"({"ping": ">=1.0.0"})");
    auto registry = fixtures.load();

    Resolver resolver(registry);
    EXPECT_THROWS(
        resolver.resolve(make_root({Dependency("ping", VersionConstraint(">=1.0.0"))})),
        CircularDependencyError);
}
