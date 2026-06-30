#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <algorithm>
#include <functional>
#include <stdexcept>

namespace pkgr {

template <typename T>
class Graph {
public:

    Graph() = default;

    Graph(const Graph& other) 
        : adjacency_list_(other.adjacency_list_), 
          nodes_(other.nodes_) {}

    Graph(Graph&& other) noexcept
        : adjacency_list_(std::move(other.adjacency_list_)),
          nodes_(std::move(other.nodes_)) {}

    Graph& operator=(const Graph& other) {
        if (this != &other) {
            adjacency_list_ = other.adjacency_list_;
            nodes_ = other.nodes_;
        }
        return *this;
    }

    Graph& operator=(Graph&& other) noexcept {
        if (this != &other) {
            adjacency_list_ = std::move(other.adjacency_list_);
            nodes_ = std::move(other.nodes_);
        }
        return *this;
    }

    ~Graph() = default;

    void add_node(const T& node) {
        if (adjacency_list_.find(node) == adjacency_list_.end()) {
            adjacency_list_[node] = {};
            nodes_.push_back(node);
        }
    }

    void add_edge(const T& from, const T& to) {
        add_node(from);
        add_node(to);
        adjacency_list_[from].push_back(to);
    }

    size_t node_count() const { return nodes_.size(); }

    size_t edge_count() const {
        size_t count = 0;
        for (const auto& [node, neighbors] : adjacency_list_) {
            count += neighbors.size();
        }
        return count;
    }

    bool has_node(const T& node) const {
        return adjacency_list_.find(node) != adjacency_list_.end();
    }

    bool has_edge(const T& from, const T& to) const {
        auto it = adjacency_list_.find(from);
        if (it == adjacency_list_.end()) return false;
        const auto& neighbors = it->second;
        return std::find(neighbors.begin(), neighbors.end(), to) != neighbors.end();
    }

    const std::vector<T>& neighbors(const T& node) const {
        auto it = adjacency_list_.find(node);
        if (it == adjacency_list_.end()) {
            throw std::out_of_range("Node not found in graph");
        }
        return it->second;
    }

    const std::vector<T>& get_nodes() const { return nodes_; }

    std::vector<T> dependents(const T& node) const {
        std::vector<T> result;
        for (const auto& [n, neighbors] : adjacency_list_) {
            if (std::find(neighbors.begin(), neighbors.end(), node) != neighbors.end()) {
                result.push_back(n);
            }
        }
        return result;
    }

    std::vector<T> bfs(const T& start) const {
        if (!has_node(start)) return {};

        std::vector<T> result;
        std::unordered_set<T> visited;
        std::queue<T> queue;

        queue.push(start);
        visited.insert(start);

        while (!queue.empty()) {
            T current = queue.front();
            queue.pop();
            result.push_back(current);

            auto it = adjacency_list_.find(current);
            if (it != adjacency_list_.end()) {
                for (const auto& neighbor : it->second) {
                    if (visited.find(neighbor) == visited.end()) {
                        visited.insert(neighbor);
                        queue.push(neighbor);
                    }
                }
            }
        }

        return result;
    }

    std::vector<T> dfs(const T& start) const {
        if (!has_node(start)) return {};

        std::vector<T> result;
        std::unordered_set<T> visited;
        dfs_helper(start, visited, result);
        return result;
    }

    bool has_cycle() const {

        std::unordered_map<T, int> color;
        for (const auto& node : nodes_) {
            color[node] = 0;
        }

        for (const auto& node : nodes_) {
            if (color[node] == 0) {
                if (has_cycle_dfs(node, color)) {
                    return true;
                }
            }
        }
        return false;
    }

    std::vector<T> find_cycle() const {
        std::unordered_map<T, int> color;
        std::unordered_map<T, T> parent;

        for (const auto& node : nodes_) {
            color[node] = 0;
        }

        for (const auto& node : nodes_) {
            if (color[node] == 0) {
                std::vector<T> cycle;
                if (find_cycle_dfs(node, color, parent, cycle)) {
                    return cycle;
                }
            }
        }
        return {};
    }

    std::vector<T> topological_sort() const {

        std::unordered_map<T, int> in_degree;
        for (const auto& node : nodes_) {
            in_degree[node] = 0;
        }
        for (const auto& [node, neighbors] : adjacency_list_) {
            for (const auto& neighbor : neighbors) {
                ++in_degree[neighbor];
            }
        }

        std::queue<T> queue;
        for (const auto& node : nodes_) {
            if (in_degree[node] == 0) {
                queue.push(node);
            }
        }

        std::vector<T> sorted;

        while (!queue.empty()) {
            T current = queue.front();
            queue.pop();
            sorted.push_back(current);

            auto it = adjacency_list_.find(current);
            if (it != adjacency_list_.end()) {
                for (const auto& neighbor : it->second) {
                    --in_degree[neighbor];
                    if (in_degree[neighbor] == 0) {
                        queue.push(neighbor);
                    }
                }
            }
        }

        if (sorted.size() != nodes_.size()) {
            return {}; 
        }

        return sorted;
    }

    std::unordered_set<T> transitive_closure(const T& start) const {
        std::unordered_set<T> reachable;
        std::queue<T> queue;

        queue.push(start);
        reachable.insert(start);

        while (!queue.empty()) {
            T current = queue.front();
            queue.pop();

            auto it = adjacency_list_.find(current);
            if (it != adjacency_list_.end()) {
                for (const auto& neighbor : it->second) {
                    if (reachable.find(neighbor) == reachable.end()) {
                        reachable.insert(neighbor);
                        queue.push(neighbor);
                    }
                }
            }
        }

        return reachable;
    }

private:
    std::unordered_map<T, std::vector<T>> adjacency_list_;
    std::vector<T> nodes_; 

    void dfs_helper(const T& node, std::unordered_set<T>& visited,
                    std::vector<T>& result) const {
        visited.insert(node);
        result.push_back(node);

        auto it = adjacency_list_.find(node);
        if (it != adjacency_list_.end()) {
            for (const auto& neighbor : it->second) {
                if (visited.find(neighbor) == visited.end()) {
                    dfs_helper(neighbor, visited, result);
                }
            }
        }
    }

    bool has_cycle_dfs(const T& node, std::unordered_map<T, int>& color) const {
        color[node] = 1; 

        auto it = adjacency_list_.find(node);
        if (it != adjacency_list_.end()) {
            for (const auto& neighbor : it->second) {
                if (color[neighbor] == 1) {
                    return true; 
                }
                if (color[neighbor] == 0 && has_cycle_dfs(neighbor, color)) {
                    return true;
                }
            }
        }

        color[node] = 2; 
        return false;
    }

    bool find_cycle_dfs(const T& node, std::unordered_map<T, int>& color,
                        std::unordered_map<T, T>& parent,
                        std::vector<T>& cycle) const {
        color[node] = 1;

        auto it = adjacency_list_.find(node);
        if (it != adjacency_list_.end()) {
            for (const auto& neighbor : it->second) {
                if (color[neighbor] == 1) {

                    cycle.clear();
                    cycle.push_back(neighbor);
                    T current = node;
                    while (current != neighbor) {
                        cycle.push_back(current);
                        current = parent[current];
                    }
                    cycle.push_back(neighbor); 
                    std::reverse(cycle.begin(), cycle.end());
                    return true;
                }
                if (color[neighbor] == 0) {
                    parent[neighbor] = node;
                    if (find_cycle_dfs(neighbor, color, parent, cycle)) {
                        return true;
                    }
                }
            }
        }

        color[node] = 2;
        return false;
    }
};

} 