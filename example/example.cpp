#include <drag/drag.hpp>
#include "../test/test-utils.hpp"

#include "parser.hpp"
#include "svg.hpp"

int main() {
    
    drag::graph g = drag::graph_builder()
                .add_edge(1, 0).add_edge(5, 0).add_edge(6, 0)
                .add_edge(2, 1).add_edge(3, 2).add_edge(4, 3)
                .add_edge(7, 5).add_edge(7, 6).add_edge(4, 7)
                .build();
    
   /* detail::subgraph g = make_subgraph(source);
    detail::network_simplex_layering lay;

    lay.run(g);*/

    //graph g;
    //auto labels = parse("../data/simple/tree.gv", g);

    drag::sugiyama_layout l(g);

    drag::drawing_options opts;
    opts.use_labels = false;
    
    drag::draw_to_svg("img.svg", l, opts);
}
