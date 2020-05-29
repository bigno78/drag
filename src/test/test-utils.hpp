#pragma once

#include "../interface.hpp"
#include "../subgraph.hpp"
#include "../layering.hpp"


inline detail::subgraph make_subgraph(graph& g) {
    std::vector<vertex_t> vec;
    for (int i = 0; i < g.size(); ++i) {
        vec.push_back(i);
    }
    return detail::subgraph(g, vec);
}

inline bool has_edge(const detail::subgraph& g, detail::edge e) {
    for (auto v : g.out_neighbours(e.from)) {
        if (v == e.to)
            return true;
    }
    return false;
}

inline int get_total_edge_length(const detail::hierarchy& h) {
    int total = 0;
    for (auto u : h.g.vertices()) {
        for (auto v : h.g.out_neighbours(u)) {
            total += h.span(u, v);
        }
    }
    return total;
}

inline graph reversed(const graph& g) {
    graph output;
    for (auto u : g.vertices()) {
        output.add_node();
    }
    for (auto u : g.vertices()) {
        for (auto v : g.out_neighbours(u)) {
            output.add_edge(v, u);
        }
    }
    return output;
}
