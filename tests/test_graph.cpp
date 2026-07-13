#include "test_framework.h"
#include "graph.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

using pkgr::Graph;

namespace {

long index_of(const std::vector<std::string>& values, const std::string& value) {
    auto it = std::find(values.begin(), values.end(), value);
    if (it == values.end()) return -1;
    return static_cast<long>(it - values.begin());
}

bool contains(const std::vector<std::string>& values, const std::string& value) {
    return index_of(values, value) >= 0;
}

// a -> b -> d, a -> c -> d
Graph<std::string> diamond() {
    Graph<std::string> g;
    g.add_edge("a", "b");
    g.add_edge("a", "c");
    g.add_edge("b", "d");
    g.add_edge("c", "d");
    return g;
}

}  // namespace

TEST(graph, counts_nodes_and_edges) {
    auto g = diamond();
    EXPECT_EQ(g.node_count(), size_t(4));
    EXPECT_EQ(g.edge_count(), size_t(4));
    EXPECT_TRUE(g.has_node("a"));
    EXPECT_FALSE(g.has_node("z"));
    EXPECT_TRUE(g.has_edge("a", "b"));
    EXPECT_FALSE(g.has_edge("b", "a"));
}

TEST(graph, add_node_is_idempotent) {
    Graph<std::string> g;
    g.add_node("a");
    g.add_node("a");
    EXPECT_EQ(g.node_count(), size_t(1));
}

TEST(graph, neighbors_throws_for_unknown_node) {
    Graph<std::string> g;
    EXPECT_THROWS(g.neighbors("ghost"), std::out_of_range);
}

TEST(graph, dependents_finds_incoming_edges) {
    auto g = diamond();
    auto deps = g.dependents("d");
    EXPECT_EQ(deps.size(), size_t(2));
    EXPECT_TRUE(contains(deps, "b"));
    EXPECT_TRUE(contains(deps, "c"));
}

TEST(graph, bfs_and_dfs_visit_all_reachable_nodes) {
    auto g = diamond();
    EXPECT_EQ(g.bfs("a").size(), size_t(4));
    EXPECT_EQ(g.dfs("a").size(), size_t(4));
    EXPECT_EQ(g.bfs("b").size(), size_t(2));  // b, d
    EXPECT_TRUE(g.bfs("ghost").empty());
    EXPECT_TRUE(g.dfs("ghost").empty());
}

TEST(graph, topological_sort_respects_every_edge) {
    auto g = diamond();
    auto sorted = g.topological_sort();
    EXPECT_EQ(sorted.size(), size_t(4));
    EXPECT_TRUE(index_of(sorted, "a") < index_of(sorted, "b"));
    EXPECT_TRUE(index_of(sorted, "a") < index_of(sorted, "c"));
    EXPECT_TRUE(index_of(sorted, "b") < index_of(sorted, "d"));
    EXPECT_TRUE(index_of(sorted, "c") < index_of(sorted, "d"));
}

TEST(graph, topological_sort_returns_empty_on_cycle) {
    auto g = diamond();
    g.add_edge("d", "a");
    EXPECT_TRUE(g.topological_sort().empty());
}

TEST(graph, detects_cycles) {
    auto g = diamond();
    EXPECT_FALSE(g.has_cycle());
    g.add_edge("d", "a");
    EXPECT_TRUE(g.has_cycle());
}

TEST(graph, self_loop_is_a_cycle) {
    Graph<std::string> g;
    g.add_edge("a", "a");
    EXPECT_TRUE(g.has_cycle());
}

TEST(graph, find_cycle_returns_closed_path) {
    Graph<std::string> g;
    g.add_edge("a", "b");
    g.add_edge("b", "c");
    g.add_edge("c", "a");

    auto cycle = g.find_cycle();
    EXPECT_EQ(cycle.size(), size_t(4));  // e.g. a -> b -> c -> a
    EXPECT_TRUE(!cycle.empty() && cycle.front() == cycle.back());
    EXPECT_TRUE(contains(cycle, "a"));
    EXPECT_TRUE(contains(cycle, "b"));
    EXPECT_TRUE(contains(cycle, "c"));
}

TEST(graph, find_cycle_returns_empty_for_dag) {
    auto g = diamond();
    EXPECT_TRUE(g.find_cycle().empty());
}

TEST(graph, transitive_closure_collects_all_reachable) {
    auto g = diamond();
    auto from_b = g.transitive_closure("b");
    EXPECT_EQ(from_b.size(), size_t(2));  // b itself and d
    EXPECT_TRUE(from_b.count("b") == 1);
    EXPECT_TRUE(from_b.count("d") == 1);
    EXPECT_EQ(g.transitive_closure("a").size(), size_t(4));
}
