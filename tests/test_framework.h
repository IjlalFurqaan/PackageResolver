#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// Minimal zero-dependency test harness.
//
// Usage:
//     TEST(suite, name) {
//         EXPECT_EQ(1 + 1, 2);
//     }
//
// test_main.cpp runs every registered test and exits non-zero if any
// fail, which is all CTest needs.

namespace pkgr_test {

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& all_tests() {
    static std::vector<TestCase> tests;
    return tests;
}

inline int& failure_count() {
    static int count = 0;
    return count;
}

struct Registrar {
    Registrar(std::string name, std::function<void()> fn) {
        all_tests().push_back({std::move(name), std::move(fn)});
    }
};

inline std::string stringify(bool value) {
    return value ? "true" : "false";
}

template <typename T>
std::string stringify(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

inline void fail(const char* file, int line, const std::string& message) {
    ++failure_count();
    std::cout << "        " << file << ":" << line << ": " << message << "\n";
}

inline int run_all_tests() {
    int failed = 0;

    for (const auto& test : all_tests()) {
        std::cout << "  [ RUN  ] " << test.name << "\n";
        int failures_before = failure_count();

        try {
            test.fn();
        } catch (const std::exception& e) {
            fail(test.name.c_str(), 0,
                 std::string("unhandled exception: ") + e.what());
        } catch (...) {
            fail(test.name.c_str(), 0, "unhandled non-standard exception");
        }

        if (failure_count() == failures_before) {
            std::cout << "  [  OK  ] " << test.name << "\n";
        } else {
            std::cout << "  [ FAIL ] " << test.name << "\n";
            ++failed;
        }
    }

    std::cout << "\n  " << (all_tests().size() - failed) << "/"
              << all_tests().size() << " tests passed\n";

    return failed == 0 ? 0 : 1;
}

}  // namespace pkgr_test

#define PKGR_CONCAT_INNER(a, b) a##b
#define PKGR_CONCAT(a, b) PKGR_CONCAT_INNER(a, b)

#define TEST(suite, name)                                                     \
    static void PKGR_CONCAT(test_, PKGR_CONCAT(suite, PKGR_CONCAT(_, name)))(); \
    static const ::pkgr_test::Registrar                                       \
        PKGR_CONCAT(registrar_, PKGR_CONCAT(suite, PKGR_CONCAT(_, name)))(    \
            #suite "." #name,                                                 \
            &PKGR_CONCAT(test_, PKGR_CONCAT(suite, PKGR_CONCAT(_, name))));   \
    static void PKGR_CONCAT(test_, PKGR_CONCAT(suite, PKGR_CONCAT(_, name)))()

#define EXPECT_TRUE(cond)                                                     \
    do {                                                                      \
        if (!(cond)) {                                                        \
            ::pkgr_test::fail(__FILE__, __LINE__,                             \
                              "EXPECT_TRUE(" #cond ") failed");               \
        }                                                                     \
    } while (0)

#define EXPECT_FALSE(cond)                                                    \
    do {                                                                      \
        if (cond) {                                                           \
            ::pkgr_test::fail(__FILE__, __LINE__,                             \
                              "EXPECT_FALSE(" #cond ") failed");              \
        }                                                                     \
    } while (0)

#define EXPECT_EQ(actual, expected)                                           \
    do {                                                                      \
        const auto& pkgr_actual_ = (actual);                                  \
        const auto& pkgr_expected_ = (expected);                              \
        if (!(pkgr_actual_ == pkgr_expected_)) {                              \
            ::pkgr_test::fail(__FILE__, __LINE__,                             \
                "EXPECT_EQ(" #actual ", " #expected ") failed: "              \
                + ::pkgr_test::stringify(pkgr_actual_) + " != "               \
                + ::pkgr_test::stringify(pkgr_expected_));                    \
        }                                                                     \
    } while (0)

#define EXPECT_THROWS(expr, exception_type)                                   \
    do {                                                                      \
        bool pkgr_caught_ = false;                                            \
        try {                                                                 \
            (void)(expr);                                                     \
        } catch (const exception_type&) {                                     \
            pkgr_caught_ = true;                                              \
        } catch (...) {                                                       \
        }                                                                     \
        if (!pkgr_caught_) {                                                  \
            ::pkgr_test::fail(__FILE__, __LINE__,                             \
                "EXPECT_THROWS(" #expr ", " #exception_type                   \
                ") failed: no " #exception_type " was thrown");               \
        }                                                                     \
    } while (0)
