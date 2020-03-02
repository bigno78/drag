#pragma once

#include <vector>
#include <memory>

#include "interface.hpp"
#include "subgraph.hpp"
#include "cycle.hpp"

class sugiyama_layout {
    graph& g;
    std::vector< detail::subgraph > subgraphs;

    std::unique_ptr< detail::cycle_removal > cycle_rem;

    std::vector< std::vector<vertex_t> > edges;
    std::vector< std::vector< std::vector<vertex_t> > > long_edges;


public:
    sugiyama_layout(graph& g) : g(g) {}

    void build();

    void node_separation(float d);
    void layer_separation(float d);

private:
    void split() {
        std::vector< std::vector<vertex_t> > components;
        std::vector< bool > done(g.size(), false);
        for (auto u : g.vertices()) {
            if (!done[u]) {
                components.emplace_back();
                split(done, components.back(), u);
            }
        }

        for (auto component : components) {
            subgraphs.emplace_back(g, component);
        }
    }

    void split(std::vector<bool>& done, std::vector<vertex_t>& component, vertex_t u) {
        component.push_back(u);
        for (auto v : g.out_neighbours(u)) {
            if (!done[v]) {
                split(done, component, v);
            }
        }
        for (auto v : g.in_neighbours(u)) {
            if (!done[v]) {
                split(done, component, v);
            }
        }
    }

    void add_dummy_nodes(detail::subgraph& g, std::vector< std::vector<vertex_t> > hierarchy) {
        
    }
};