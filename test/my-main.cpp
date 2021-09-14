
#include <drag/drawing/svg.hpp>
#include <drag/drawing/draw.hpp>

#include "bruteforce.hpp"
#include "test-utils.hpp"
#include "../example/helper.hpp"

#include "../generator/gen.hpp"

using namespace drag;
using namespace drag::detail;

int main() {
    graph g = graph_builder()
                .add_edge(0, 1).add_edge(1, 2)
                .add_edge(0, 2).add_edge(0, 3)
                .add_edge(3, 4).add_edge(4, 5)
                .add_edge(4, 6).add_edge(1, 6).add_edge(6, 7)
                .add_edge(2, 7).add_edge(4, 8).add_edge(8, 9)
                .add_edge(9, 10).add_edge(5, 10).add_edge(7, 9)
                .add_edge(3, 11).add_edge(11, 5).add_edge(0, 12)
                .add_edge(12, 13).add_edge(13, 9)
                .build();
    subgraph sub = make_subgraph(g);
    drawing_options opts;
    dag_generator gen;
    network_simplex_layering layering_module;

    for (int i = 0; i < 20; ++i) {
        //auto g = gen.generate_from_density(15, 0.2);
        auto g = gen.generate_from_edges(15, 20);
        auto sub = make_subgraph(g);

        auto h = layering_module.run(sub);
        size_t simplex_len = get_total_edge_length(h);

        std::cout << "starting " << g.size() << " INDEX " << i << "\n";
        
        auto start = now();
        size_t brute_len = bruteforce_layering_total_length(g);
        auto end = now();

        std::cout << "Simplex: " << simplex_len << "\n";
        std::cout << "Brute:   " << brute_len << "\n";
        std::cout << "time: " << to_mili(end - start) << "\n\n";

        if (simplex_len != brute_len) {
            std::cout << "FAIL!!!!\n";
        }
        
        draw_svg_file("stuff/" + std::to_string(i) + ".svg", g, {});
    }
}
