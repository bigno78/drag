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

    /*graph g = graph_builder()
                .add_edge(2, 3).add_edge(3, 1)
                .build();*/

    /*detail::subgraph sub(g, { 0, 1, 2, 3 });
    sub.add_dummy();
    sub.add_dummy();
    sub.add_edge(0, 4);
    sub.add_edge(4, 5);
    sub.add_edge(5, 1);
    detail::hierarchy h(sub, 0);
    h.ranking[0] = 0;
    h.ranking[1] = 3;
    h.ranking[2] = 1;
    h.ranking[3] = 2;
    h.ranking[4] = 1;
    h.ranking[5] = 2;
    h.layers = { {0},
                 {4, 2},
                 {3, 5},
                 {1} };*/

    labels.resize(g.size());
    sugiyama_layout layout(g);

  /*  std::vector<node> neco;
    detail::fast_and_simple_positioning pos({20, 100}, neco, g);
    pos.run(sub, h, {});*/

    layout.build();
    draw_to_svg(layout, "neco.svg");
}