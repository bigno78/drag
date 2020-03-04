#include <iostream>

#include "interface.hpp"
#include "stuff.hpp"
#include "layout.hpp"

#include "svg.hpp"

int main() {
    graph g = graph_builder()
                .add_edge(0, 1)
                .add_edge(1, 2)
                .add_edge(1, 4)

                .add_edge(3, 4)
                .add_edge(4, 0)
                .build();

    sugiyama_layout layout(g);

    layout.build();

    svg_img svg("img.svg");
    svg.draw_line({ 20, 20 }, { 50, 50 }, "blue");
    svg.draw_circle({ 20, 20}, 10);
    svg.draw_text({20, 20}, "A");
}