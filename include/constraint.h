#pragma once

#include "version.h"
#include <string>

namespace pkgr {

enum class ConstraintOp {
    EQ,     
    NEQ,    
    LT,     
    GT,     
    LTE,    
    GTE,    
    CARET,  
    TILDE   
};

class VersionConstraint {
public:

    explicit VersionConstraint(const std::string& constraint_str);

    bool satisfies(const Version& version) const;

    ConstraintOp op() const { return op_; }
    const Version& target() const { return target_; }

    std::string to_string() const;

    friend std::ostream& operator<<(std::ostream& os, const VersionConstraint& c);

private:
    ConstraintOp op_;
    Version target_;

    static std::string op_to_string(ConstraintOp op);

    static std::pair<ConstraintOp, std::string> parse_operator(const std::string& str);
};

} 