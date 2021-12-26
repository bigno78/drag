#include <drag/drag.hpp>
#include <drag/drawing/draw.hpp>


int main() {
    drag::graph g;

    auto a = g.add_node();
    auto b = g.add_node();
    auto c = g.add_node();

    g.add_edge(a, b);
    g.add_edge(a, c);

    drag::drawing_options opts;

    opts.labels[a] = "a";
    opts.labels[b] = "b";
    opts.labels[c] = "c";

    opts.colors[a] = "blue";
    opts.colors[b] = "green";

    opts.edge_colors[{a, b}] = "red";

    auto image = drag::draw_svg_image(g, opts);
    image.save("image.svg");
}
