#include <iostream>
#include <dirent.h>

//#define DEBUG_COORDINATE
//#define CONTROL_CROSSING
//#define DEBUG_CROSSING

#include "interface.hpp"
#include "layout.hpp"
#include "parser.hpp"

#include "svg.hpp"

int ej = 0;
void layout(graph g, std::string label) {
    sugiyama_layout l(g);
    l.build();
    std::cout << label << "\n";
    ++ej;
}

void generate(graph g, vertex_t u, std::string label) {
    if (u == g.size() - 1) {
        layout(g, label);
        return;
    }
    unsigned n = g.size() - u - 1;

    int iter = 0;
    for (int k = 1; k <= n; ++k) {
        std::vector<bool> stuff( n, false );
        std::fill( stuff.end() - k, stuff.end(), true );

        do {
            for (int i = 0; i < stuff.size(); ++i) {
                if (stuff[i]) {
                    g.add_edge(u, u + 1 + i);
                }
            }

            generate(g, u + 1, label + std::to_string(iter));

            for (int i = 0; i < stuff.size(); ++i) {
                if (stuff[i]) {
                    g.remove_edge(u, u + 1 + i);
                }
            }
            ++iter;
        } while( std::next_permutation(stuff.begin(), stuff.end()) );
    }
}

void do_it(int n) {
    graph g;
    for (int i = 0; i < n; ++i) {
        g.add_node();
    }
    std::cout << g.size() << "\n";
    generate(g, 0, "");
}

int main() {
    do_it(8);
    std::cout << "ej: " << ej << "\n";
    return 0;
    /*graph g = graph_builder()
                .add_edge(0, 1).add_edge(0, 5).add_edge(0, 6)
                .add_edge(1, 2).add_edge(2, 3).add_edge(3, 4)
                .add_edge(5, 7).add_edge(6, 7).add_edge(7, 4)
                .build();*/
    /*graph g = graph_builder()
                .add_edge(0, 2).add_edge(0, 3).add_edge(1, 3).add_edge(1, 4)
                .add_edge(4, 6).add_edge(5, 7).add_edge(6, 7)
                .add_edge(6, 8)
                .build();*/

    DIR *dir;
    struct dirent *ent;
    if ( (dir = opendir ("../../input_graphs_dot/")) != NULL ) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            std::string name{ ent->d_name };
            if (name == "." || name == ".." || name != "g.11.16.dot") {
                continue;
            }
            std::cout << name << "\n";

            graph g;
            auto labels = parse("../../input_graphs_dot/" + name, g);
            svg_img img(std::string{"../../my_output/"} + ent->d_name + ".svg");

            /*sugiyama_layout l(g);
            l.build();
            draw_to_svg(img, l, labels);*/

            graph tmp = g;
            sugiyama_layout l1(g);
            sugiyama_layout l2(tmp);
            //crossing_enabled = false;
            l1.build();
            //crossing_enabled = true;
            l2.build();
            draw_to_svg(img, l1, labels, vec2{ 0, 0 } );
            draw_to_svg(img, l2, labels, vec2{ l1.width() + 20, 0 });

           /* for (int i = 0; i < 4; ++i) {
                svg_img img("neco" + std::to_string(i) + ".svg");
                graph gr = g;
                produce_layout = i;
                sugiyama_layout layout(gr);
                layout.build();
                draw_to_svg(img, layout);
            }
            svg_img img("neco.svg");
            produce_layout = 4;
            sugiyama_layout layout(g);
            layout.build();
            draw_to_svg(img, layout );*/

        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
        return EXIT_FAILURE;
    }

    
    
    /*sugiyama_layout layout(debug_graph);
    layout.build();
    labels = debug_labels;
    draw_to_svg(layout, "debug.svg");*/

#ifdef CONTROL_CROSSING
   /* graph g;
    svg_img img("neco.svg");
    vec2 start = { 0, 0 };*/
    
#endif

#ifdef DEBUG_COORDINATE
   /* svg_img img("neco.svg");
    vec2 start = { 0, 0 };
    float w = 0;
    for (int i = 0; i < 4; ++i) {
        graph gr = g;
        produce_layout = i;
        labels.resize(gr.size());
        sugiyama_layout layout(gr);
        layout.build();
        draw_to_svg(img, layout, start);
        start.x += layout.width();
        w = layout.height();
    }
    produce_layout = 4;
    sugiyama_layout layout(g);
    layout.build();
    draw_to_svg(img, layout, { 0, w } );*/
#endif

}