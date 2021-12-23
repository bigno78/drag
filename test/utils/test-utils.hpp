#pragma once

#include <drag/detail/subgraph.hpp>
#include <drag/drag.hpp>
#include <drag/detail/layering.hpp>


inline drag::detail::subgraph make_subgraph(drag::graph& g) {
    std::vector<drag::vertex_t> vec;
    for (int i = 0; i < g.size(); ++i) {
        vec.push_back(i);
    }
    return drag::detail::subgraph(g, vec);
}

inline bool has_edge(const drag::detail::subgraph& g, drag::detail::edge e) {
    for (auto v : g.out_neighbours(e.from)) {
        if (v == e.to)
            return true;
    }
    return false;
}

inline int get_total_edge_length(const drag::detail::hierarchy& h) {
    int total = 0;
    for (auto u : h.g.vertices()) {
        for (auto v : h.g.out_neighbours(u)) {
            total += h.span(u, v);
        }
    }
    return total;
}

inline drag::graph reversed(const drag::graph& g) {
    drag::graph output;
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
