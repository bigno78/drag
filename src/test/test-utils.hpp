#pragma once

#include "../interface.hpp"
#include "../subgraph.hpp"

inline detail::subgraph make_sub(graph& g) {
    std::vector<vertex_t> vec;
    for (int i = 0; i < g.size(); ++i) {
        vec.push_back(i);
    }
    return detail::subgraph(g, vec);
}

inline bool has_edge(const detail::subgraph& g, edge e) {
    for (auto v : g.out_neighbours(e.tail)) {
        if (v == e.head)
            return true;
    }
    return false;
}