#include "helper.hpp"

#define REPORTING

#include "../interface.hpp"
#include "../svg.hpp"
#include "../parser.hpp"
#include "../report.hpp"

#include <iostream>
#include <string>

void print_usage() {
    std::cout << "usage: ./draw [-d] source destination\n";
}

void draw_graph(const std::string& in, const std::string& out) {
	graph g;
	auto lbls = parse(in, g);

	sugiyama_layout l(g);
	l.build();

	svg_img img(out);
	draw_to_svg(img, l/*, lbls*/);
}

int main(int argc, char **argv) {
    if (argc < 3 || argc > 4) {
        print_usage();
        return 1;
    }

    bool print_dir = false;
    std::string path;
    std::string out;

    int i = 1;
    if ( std::string{ argv[i] } == std::string{ "-d" } ) {
        print_dir = true;
        ++i;
        if (argc != 4) {
            print_usage();
            return 1;
        }
    }

    path = argv[i++];
    out = argv[i];

    if (print_dir) {
        auto files = dir_contents(path);
		for (const auto& f : files) {
			draw_graph(path + f, out + f + ".svg");
            std::cout << f << " " << report::simplex::iters << "\n";
		}
    } else {
		draw_graph(path, out);
    }

    return 0;
}
