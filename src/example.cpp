#include "interface.hpp"

#include "parser.hpp"
#include "svg.hpp"

int main() {
    /*
    graph g = graph_builder()
            .add_edge(0, 1).add_edge(0, 5).add_edge(0, 6)
            .add_edge(1, 2).add_edge(2, 3).add_edge(3, 4)
            .add_edge(5, 7).add_edge(6, 7).add_edge(7, 4)
            .build();
    */

    graph g;
    auto labels = parse("data/cyclic.gv", g);

    sugiyama_layout l(g);
    l.build();

    svg_img img { "example.svg" };
    draw_to_svg(img, l, labels);
}