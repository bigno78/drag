#include "bruteforce.hpp"

#include <algorithm>


using namespace drag;
using namespace drag::detail;


/**
 * Construct a graph on n vertices from the given set of edges.
 * 
 * Assumes that the edges use only vertices 0...n-1.
 * 
 * @param n     number of vertices of the resulting graph
 * @param edges the set of edges of the graph
 * @return the graph with given edge set
 */
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
 * Attempts to assign ranks to the vertices in the graph g by performing
 * a search of the graph from the vertex 0. The produced ranking is not
 * necessarily normalized.
 * 
 * This function assumes that g is a tree and it also checks this fact.
 * That is achieved by counting the number of vertices and edges reachable
 * from vertex 0. If it is 'g.size()' and 'g.size() - 1' respectively, the graph
 * must be a tree.
 * 
 * If the graph is not a tree the content of 'rank' after calling this
 * function is undefined.
 */
bool spanning_tree_to_ranks(const graph& g, vertex_map<int>& rank) {
    std::vector<vertex_t> todo;
    vertex_map<bool> done(g);
    
    size_t vertex_count = 0;
    size_t edge_count = 0;

    done[0] = true;
    todo.push_back(0);
    rank[0] = 0;

    while (!todo.empty()) {
        auto u = todo.back();
        todo.pop_back();
        vertex_count++;

        for (auto v : g.out_neighbours(u)) {
            edge_count++;
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

    return edge_count == g.size() - 1 && vertex_count == g.size();
}

size_t get_height(const graph& g, std::vector<edge>& picked) {
    const auto tree = make_graph_with_edges(g.size(), picked);
    vertex_map<int> rank(g);

    if (!spanning_tree_to_ranks(tree, rank)) {
        // the picked edges don't form a tree
        return size_t(-1);
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

size_t bruteforce_layering_total_length(
                        const graph& g, 
                        const std::vector<edge>& edges, 
                        std::vector<edge>& picked, 
                        vertex_map<size_t>& freq,
                        size_t vertex_count,
                        size_t i) 
{
    if (g.size() - 1 - picked.size() > edges.size() - i) {
        return -1; // max value
    }

    // if we 
    if (picked.size() == g.size() - 1) {
        if (vertex_count != g.size()) {
            return -1;
        }
        return get_height(g, picked);
    }

    auto a = bruteforce_layering_total_length(g, edges, picked, freq, vertex_count, i + 1);

    auto e = edges[i];
    
    if (freq[e.from] == 0) {
        vertex_count++;
    }
    freq[e.from]++;

    if (freq[e.to] == 0) {
        vertex_count++;
    }
    freq[e.to]++;

    picked.push_back(edges[i]);

    auto b = bruteforce_layering_total_length(g, edges, picked, freq, vertex_count, i + 1);

    picked.pop_back();
    freq[e.from]--;
    freq[e.to]--;

    return std::min(a, b);
    
}

size_t bruteforce_layering_total_length(const graph& g) {
    const std::vector<edge> edges = list_edges(g);
    std::vector<edge> picked;
    vertex_map<size_t> freq(g);
    return bruteforce_layering_total_length(g, edges, picked, freq, 0, 0);
}
