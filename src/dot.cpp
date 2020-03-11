#include <iostream>

#include "interface.hpp"
#include "stuff.hpp"
#include "layout.hpp"

#include "svg.hpp"

int main() {
    graph g = graph_builder()
                .add_edge(0, 1).add_edge(0, 5).add_edge(0, 6)
                .add_edge(1, 2).add_edge(2, 3).add_edge(3, 4)
                .add_edge(5, 7).add_edge(6, 7).add_edge(7, 4)
                .build();

    /*std::vector<vertex_t> a = { };
    std::vector<vertex_t> b = { 1, 0 };

    std::cout << *std::begin(b) << "\n";

    for (auto u : chain_range< std::vector<vertex_t> >(a, b))
        std::cout << u << " " << "\n";*/

    labels.resize(g.size());
    sugiyama_layout layout(g);

    layout.build();
    draw_to_svg(layout, "neco.svg");
}