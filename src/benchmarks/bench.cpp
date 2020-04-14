#define REPORTING

#include "../interface.hpp"
#include "../parser.hpp"
#include "../svg.hpp"
#include "helper.hpp"

#include <iostream>

int main(int argc, char** argv) {
    graph g;
    parse(argv[1], g);
    
    uint64_t without;
    {
        graph copy = g;
        defaults::trans = true;
        sugiyama_layout layout(copy);

        auto start = now();
        layout.build();
        without = to_micro( now() - start );
        std::cout << report::base << "\n";
        std::cout << "time: " << without << "\n";
        std::cout << report::final << "\n";
    }
    /*
    uint64_t with;
    {
        defaults::trans = true;
        sugiyama_layout layout(g);

        auto start = now();
        layout.build();
        with = to_micro( now() - start );
        std::cout << "transpose: " << with << "\n";
        std::cout << report::final << "\n";
    }
    std::cout << with/without << "x more\n";*/

    //svg_img img("bench.svg");
    //draw_to_svg(img, layout);
}