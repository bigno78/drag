//#define REPORTING

#include "helper.hpp"
#include "interface.hpp"
#include "svg.hpp"
#include "parser.hpp"
#include "report.hpp"

#include <iostream>
#include <string>

void draw_graph(const std::string& in, const std::string& out) {
    attributes attr;
    drawing_options opts;
	auto g = parse(in, attr, opts);

	sugiyama_layout l(g, attr);
	draw_to_svg(out, l, opts);
}


std::string usage_string =
R"(Create an SVG image of a graph or a set of graphs.
    
usage: ./draw [-d] <source> <destination>

  -d Draw all .gv files in the source directory to the destination directory.
)";

void print_help() {
    std::cout << usage_string;
}

int main(int argc, char **argv) {
    if (argc > 1 && std::string{ argv[1] } == "-h") {
        print_help();
        return 0;
    }
    if (argc < 3 || argc > 4) {
        print_help();
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
            print_help();
            return 1;
        }
    }

    path = argv[i++];
    out = argv[i];

    if (print_dir) {
        auto files = dir_contents(path, ".gv");
		for (const auto& f : files) {
            std::cout << f << "\n";
			draw_graph(path + f, out + f + ".svg");
		}
    } else {
		draw_graph(path, out);
    }

    return 0;
}
