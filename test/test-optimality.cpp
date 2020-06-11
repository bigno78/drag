#include "../src/layering.hpp"
#include "../src/parser.hpp"
#include "../src/benchmarks/helper.hpp"
#include "test-utils.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Optimality test: missing input dir!\n";
        return 1;
    }

    std::string REFERECE_FILE = "opt.txt";

    std::ifstream ref_in(REFERECE_FILE);
    std::map<std::string, int> expected;
    std::string line;
    int i = 1;
    while(std::getline(ref_in, line)) {
        std::istringstream ss(line);
        std::string f;
        int val;
        ss >> f >> val;
        if (!std::cin) {
            std::cerr << "Optimality test: wrong reference file format on line" << i << "!\n";
            return 1;
        }
        expected[f] = val;
        ++i;
    }

    for (const auto& f : dir_contents(argv[1], ".gv")) {
        std::cout << f << "\n";
        graph g;
        parse(argv[1] + f, g);
        detail::subgraph sub = make_subgraph(g);
        detail::network_simplex_layering lay;
        auto h = lay.run(sub);

        int given = get_total_edge_length(h);
        std::cout << given << "\n";
        if (given != expected[f]) {
            std::cout << "TEST FAILED on: " << f << "\n";
            std::cout << "expected: " << expected[f] << "\n";
            std::cout << "given:    " << given << "\n";
            return 1;
        }
    }

    std::cout << "TEST PASSED\n";
    return 0;
}
