#pragma once

#include "package.h"
#include "registry.h"
#include "graph.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace pkgr {

struct ResolvedPackage {
    std::string name;
    Version version;
    int depth;  

    ResolvedPackage(std::string n, Version v, int d)
        : name(std::move(n)), version(std::move(v)), depth(d) {}
};

class Resolver {
public:
    explicit Resolver(const Registry& registry);

    std::vector<ResolvedPackage> resolve(const Package& root);

    const Graph<std::string>& dependency_graph() const { return dep_graph_; }

    bool has_circular_dependencies() const;

    std::vector<std::string> get_cycle() const;

private:
    const Registry& registry_;
    Graph<std::string> dep_graph_;
    std::map<std::string, ResolvedPackage> resolved_;

    void resolve_recursive(const Package& pkg, int depth);
};

} 