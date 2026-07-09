
#include "constraint.h"
#include <stdexcept>
#include <algorithm>

namespace pkgr {

VersionConstraint::VersionConstraint(const std::string& constraint_str) {
    auto [op, version_str] = parse_operator(constraint_str);
    op_ = op;
    target_ = Version(version_str);
}

bool VersionConstraint::satisfies(const Version& version) const {
    switch (op_) {
        case ConstraintOp::EQ:
            return version == target_;

        case ConstraintOp::NEQ:
            return version != target_;

        case ConstraintOp::LT:
            return version < target_;

        case ConstraintOp::GT:
            return version > target_;

        case ConstraintOp::LTE:
            return version <= target_;

        case ConstraintOp::GTE:
            return version >= target_;

        case ConstraintOp::CARET:

            if (target_.major() == 0) {
                if (target_.minor() == 0) {
                    // ^0.0.x only allows the patch to match exactly (semver: >=0.0.x <0.0.(x+1))
                    return version == target_;
                }
                return version >= target_ &&
                       version.major() == target_.major() &&
                       version.minor() == target_.minor();
            }
            return version >= target_ &&
                   version.major() == target_.major();

        case ConstraintOp::TILDE:

            return version >= target_ && 
                   version.major() == target_.major() &&
                   version.minor() == target_.minor();
    }

    return false; 
}

std::string VersionConstraint::to_string() const {
    return op_to_string(op_) + target_.to_string();
}

std::ostream& operator<<(std::ostream& os, const VersionConstraint& c) {
    os << c.to_string();
    return os;
}

std::string VersionConstraint::op_to_string(ConstraintOp op) {
    switch (op) {
        case ConstraintOp::EQ:    return "=";
        case ConstraintOp::NEQ:   return "!=";
        case ConstraintOp::LT:    return "<";
        case ConstraintOp::GT:    return ">";
        case ConstraintOp::LTE:   return "<=";
        case ConstraintOp::GTE:   return ">=";
        case ConstraintOp::CARET: return "^";
        case ConstraintOp::TILDE: return "~";
    }
    return "?";
}

std::pair<ConstraintOp, std::string> VersionConstraint::parse_operator(const std::string& str) {

    auto trimmed = str;
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    trimmed.erase(trimmed.find_last_not_of(" \t") + 1);

    if (trimmed.empty()) {
        throw std::invalid_argument("Empty constraint string");
    }

    if (trimmed.size() >= 2) {
        std::string prefix2 = trimmed.substr(0, 2);
        if (prefix2 == ">=") return { ConstraintOp::GTE, trimmed.substr(2) };
        if (prefix2 == "<=") return { ConstraintOp::LTE, trimmed.substr(2) };
        if (prefix2 == "!=") return { ConstraintOp::NEQ, trimmed.substr(2) };
    }

    char first = trimmed[0];
    if (first == '>') return { ConstraintOp::GT, trimmed.substr(1) };
    if (first == '<') return { ConstraintOp::LT, trimmed.substr(1) };
    if (first == '=') return { ConstraintOp::EQ, trimmed.substr(1) };
    if (first == '^') return { ConstraintOp::CARET, trimmed.substr(1) };
    if (first == '~') return { ConstraintOp::TILDE, trimmed.substr(1) };

    return { ConstraintOp::EQ, trimmed };
}

} 