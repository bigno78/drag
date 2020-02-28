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

public:
    sugiyama_layout(graph& g) : g(g) {}

    void build();

    void node_separation(float d);
    void layer_separation(float d);

private:
    void split();
};