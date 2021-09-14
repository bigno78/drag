#include "bruteforce.hpp"

#include <algorithm>


using namespace drag;
using namespace drag::detail;


// Make graph of size <n> containing the edges in <edges>.
graph make_graph_with_edges(size_t n, const std::vector<edge>& edges) {
    graph g;

    for (size_t i = 0; i < n; ++i) {
        g.add_node();
    }

    for (auto [u, v] : edges) {
        g.add_edge(u, v);
    }

    return g;
}

/**
 * 
 */
bool spanning_tree_to_ranks(const graph& g, vertex_map<int>& rank) {
    std::vector<vertex_t> todo;
    vertex_map<bool> done(g);
    size_t count = 0;

    done[0] = true;
    todo.push_back(0);
    rank[0] = 0;

    while (!todo.empty()) {
        auto u = todo.back();
        todo.pop_back();
        count++;

        for (auto v : g.out_neighbours(u)) {
            if (!done[v]) {
                todo.push_back(v);
                done[v] = true;
                rank[v] = rank[u] + 1;
            }
        }

        for (auto v : g.in_neighbours(u)) {
            if (!done[v]) {
                todo.push_back(v);
                done[v] = true;
                rank[v] = rank[u] - 1;
            }
        }
    }

    return count == g.size();
}

size_t get_height(const graph& g, std::vector<edge>& picked) {
    const auto tree = make_graph_with_edges(g.size(), picked);
    vertex_map<int> rank(g);

    if (!spanning_tree_to_ranks(tree, rank)) {
        return -1;
    }

    size_t total = 0;
    for (auto u : g.vertices()) {
        for (auto v : g.out_neighbours(u)) {
            if (rank[u] >= rank[v]) {
                return -1;
            }
            total += rank[v] - rank[u];
        }
    }

    return total;
}

std::vector<edge> list_edges(const graph& g) {
    std::vector<edge> edges;
    for (auto u : g.vertices()) {
        for (auto v : g.out_neighbours(u)) {
            edges.push_back({u, v});
        }
    }
    return edges;
}

size_t bruteforce_layering_total_length(const graph& g, const std::vector<edge>& edges, std::vector<edge>& picked, size_t i) {
    if (g.size() - 1 - picked.size() > edges.size() - i) {
        return -1; // max value
    }

    if (picked.size() == g.size() - 1) {
        return get_height(g, picked);
    }

    auto a = bruteforce_layering_total_length(g, edges, picked, i + 1);

    picked.push_back(edges[i]);
    auto b = bruteforce_layering_total_length(g, edges, picked, i + 1);
    picked.pop_back();

    return std::min(a, b);
}

size_t bruteforce_layering_total_length(const graph& g) {
    const std::vector<edge> edges = list_edges(g);
    std::vector<edge> picked;
    return bruteforce_layering_total_length(g, edges, picked, 0);
}
