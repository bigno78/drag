#include "helper.hpp"

#include <iostream>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Specify a directory!\n";
        return 1;
    }

    std::string extension = "";
    if (argc > 2) {
        extension = argv[2];
    }

    std::cout << "dir: " << argv[1] << "\n";
    std::cout << "extension: " << extension << "\n";

    auto files = drag::dir_contents(argv[1], extension);

    for (const auto& file : files) {
        std::cout << file << "\n";
    }
}
