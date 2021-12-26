#include "parser.hpp"

#include <drag/drag.hpp>
#include <drag/drawing/draw.hpp>

#include <iostream>
#include <string>
#include <string_view>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * Return all regular files in the specified directory with the given extension.
 * 
 * The names don't contain the directory path.
 * If no extension or an empty extension is given, returns all regular files.
 * 
 * @param path            the path to the directory
 * @param file_extension  the extension to search for
 * 
 * @return all files in the directory with the given extension
 */
std::vector<fs::path> dir_contents(const fs::path& dir_path, const std::string file_extension="") {
    std::vector<fs::path> contents;

    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        if (entry.is_regular_file()) {
            if (file_extension == "" || entry.path().extension() == file_extension) {
                contents.push_back( entry.path() );
            }
        }
    }

    return contents;
}


float to_pt(float x) {
	return x*72;
}


void draw_graph(const fs::path& input_path, const fs::path& output_path, bool convert_to_pt) {
    drag::drawing_options opts;
    drag::graph g = drag::parse(input_path.string(), opts);

    if (convert_to_pt) {
        g.node_size = to_pt(g.node_size);
        g.node_dist = to_pt(g.node_dist);
        g.layer_dist = to_pt(g.layer_dist);
        g.loop_size = to_pt(g.loop_size);
    }

    auto image = drag::draw_svg_image(g, opts);
    image.save(output_path.string());
}


std::string usage_string =
R"(Create an SVG image of a graph or a set of graphs.
    
USAGE: ./draw [ options ] -d <input_dir> <output_dir>
       ./draw [ options ] <input_file> <output_file>

OPTIONS:
  -d   Draw all .gv files in the input directory to the output directory.
  -i   Interpret the sizes as inches (for compatibility with dot).
       The default unit are points (pt).
)";

void print_help() {
    std::cout << usage_string;
}

int main(int argc, char **argv) {
    std::vector< std::string_view > args;
    
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    argc--;

    if (argc > 0 && (args.front() == "-h" || args.front() == "--help")) {
        std::cout << usage_string;
        return 0;
    }

    if (argc < 2) {
        std::cerr << "Not enough arguments.\n\n";
        std::cerr << usage_string;
    }

    fs::path input(args[argc - 2]);
    fs::path output(args[argc - 1]);
    bool dir_input = false;
    bool inches = false;
    
    for (int i = 0; i < argc - 2; ++i) {
        auto arg = args[i];

        if (arg == "-h" | arg == "--help") {
            std::cout << usage_string;
            return 0;
        } else if (arg == "-d") {
            dir_input = true;
        } else if (arg == "-i") {
            inches = true;
        } else {
            std::cerr << "Uknown option: '" << arg << "'\n\n";
            std::cerr << usage_string;
        }
    }

    if (dir_input) {
		for (auto& f : dir_contents(input, ".gv")) {
            auto out_file = f.filename().replace_extension(".svg");
			draw_graph(f, output/out_file, inches);
		}
    } else {
        draw_graph(input, output, inches);
    }

    return 0;
}
