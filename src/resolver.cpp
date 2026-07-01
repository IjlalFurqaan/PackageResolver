
#include "resolver.h"
#include "exceptions.h"
#include "colors.h"
#include <iostream>
#include <algorithm>

namespace pkgr {

Resolver::Resolver(const Registry& registry) : registry_(registry) {}

std::vector<ResolvedPackage> Resolver::resolve(const Package& root) {

    dep_graph_ = Graph<std::string>();
    resolved_.clear();

    dep_graph_.add_node(root.name());

    resolve_recursive(root, 0);

    if (dep_graph_.has_cycle()) {
        auto cycle = dep_graph_.find_cycle();
        throw CircularDependencyError(cycle);
    }

    auto sorted = dep_graph_.topological_sort();

    if (sorted.empty() && dep_graph_.node_count() > 0) {
        throw PackageResolverError("Failed to determine install order (possible cycle)");
    }

    std::vector<ResolvedPackage> result;
    for (const auto& name : sorted) {
        if (name == root.name()) continue; 

        auto it = resolved_.find(name);
        if (it != resolved_.end()) {
            result.push_back(it->second);
        }
    }

    return result;
}

void Resolver::resolve_recursive(const Package& pkg, int depth) {
    for (const auto& dep : pkg.dependencies()) {

        if (resolved_.find(dep.name) != resolved_.end()) {

            dep_graph_.add_edge(pkg.name(), dep.name);
            continue;
        }

        auto found = registry_.find_package(dep.name, dep.constraint);

        if (!found.has_value()) {

            if (!registry_.has_package(dep.name)) {
                throw PackageNotFoundError(dep.name);
            } else {
                throw VersionConflictError(dep.name, dep.constraint.to_string());
            }
        }

        auto& resolved_pkg = found.value();

        resolved_.emplace(dep.name, ResolvedPackage(
            dep.name, resolved_pkg->version(), depth + 1));

        dep_graph_.add_edge(pkg.name(), dep.name);

        resolve_recursive(*resolved_pkg, depth + 1);
    }
}

bool Resolver::has_circular_dependencies() const {
    return dep_graph_.has_cycle();
}

std::vector<std::string> Resolver::get_cycle() const {
    return dep_graph_.find_cycle();
}

} 