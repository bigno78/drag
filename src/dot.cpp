#include <iostream>
#include <dirent.h>

#define DEBUG_COORDINATE
//#define CONTROL_CROSSING
//#define DEBUG_CROSSING
//#define DEBUG_GRAPH

#include "interface.hpp"
#include "layout.hpp"
#include "parser.hpp"

#include "svg.hpp"



int main(int argc, char** argv) {
    /*graph g = graph_builder()
                .add_edge(0, 1).add_edge(0, 5).add_edge(0, 6)
                .add_edge(1, 2).add_edge(2, 3).add_edge(3, 4)
                .add_edge(5, 7).add_edge(6, 7).add_edge(7, 4)
                .build();*/
    /*graph g = graph_builder()
                .add_edge(0, 6).add_edge(2, 6).add_edge(3, 6).add_edge(6, 7)
                .add_edge(7, 9).add_edge(4, 5).add_edge(5, 9)
                .add_edge(1, 8).add_edge(8, 9)
                .build();*/

    attributes attr;
    drawing_options opt;
    auto g = parse(argv[1], attr, opt); 
    //auto labels = parse("data/disconnected.gv", g);

#if defined(CONTROL_CROSSING)

    svg_img img("cross.svg");
    graph g2 = g;

    sugiyama_layout l1(g);
    sugiyama_layout l2(g2);
  
    crossing_enabled = false;
    l1.build();

    crossing_enabled = true;
    l2.build();

    draw_to_svg(img, l1);
    draw_to_svg(img, l2, vec2{ l1.width() + 20, 0 });
    
#elif defined(DEBUG_COORDINATE)

    for (int i = 0; i < 4; ++i) {
        produce_layout = i;
        sugiyama_layout layout(g, attr);
        draw_to_svg("coord" + std::to_string(i) + ".svg", layout, opt);
    }
    produce_layout = 4;
    sugiyama_layout layout(g, attr);
    draw_to_svg("coord-final.svg", layout, opt);

#elif defined(DEBUG_GRAPH)

    svg_img img{ "debug.svg" };
    sugiyama_layout layout(g);
    layout.build();
    draw_to_svg(img, layout/*, labels*/);

#else

    layout_files("../../input_graphs_dot/", "../../my_output/");

#endif

}