#pragma once

#include <drag/detail/subgraph.hpp>

namespace drag {

namespace detail {

/**
 * Recursively assign u and all vertices reachable from u in the underlying undirected graph to the same component.
 */
inline void split(const graph& g, vertex_map<bool>& done, std::vector<vertex_t>& component, vertex_t u) {
    done[u] = true;
    component.push_back(u);
    for (auto v : g.out_neighbours(u)) {
        if (!done[v]) {
            split(g, done, component, v);
        }
    }
    for (auto v : g.in_neighbours(u)) {
        if (!done[v]) {
            split(g, done, component, v);
        }
    }
}


/**
 * Split the given graph into connected components represented by subgrapgs.
 */
inline std::vector<subgraph> split(graph& g) {
    std::vector< std::vector<vertex_t> > components;
    vertex_map<bool> done(g, false);

    for (auto u : g.vertices()) {
        if (!done[u]) {
            components.emplace_back();
            split(g, done, components.back(), u);
        }
    }

    std::vector<subgraph> subgraphs;
    for (auto component : components) {
        subgraphs.emplace_back(g, component);
    }

    return subgraphs;
}

} // drag

} // detail
