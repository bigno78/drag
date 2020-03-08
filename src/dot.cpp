#include <iostream>

#include "interface.hpp"
#include "stuff.hpp"
#include "layout.hpp"

#include "svg.hpp"

int main() {
    graph g = graph_builder()
                .add_edge(0, 1)
                .add_edge(1, 0)
                .add_edge(1, 2)
                .add_edge(2, 0)

                .add_edge(3, 4)
                .add_edge(5, 4)
                .build();

    sugiyama_layout layout(g);

    layout.build();
    draw_to_svg(layout, "neco.svg");
}