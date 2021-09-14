#include <drag/drawing/draw.hpp>

#include "utils/gen.hpp"
#include "utils/bruteforce.hpp"

#include <iostream>

template<typename Func>
bool run_randomized_test(Func test_func,
                         std::string test_name,
                         size_t n,
                         size_t m,
                         size_t count = 20,
                         size_t seed = 7)
{
    drag::dag_generator gen(seed);

    for (size_t i = 0; i < count; ++i) {
        drag::graph g = gen.generate_from_edges(n, m);
        
        if (!test_func(g)) {
            std::string filename = "error.svg";

            std::cerr << "TEST FAILED: " << test_name << "\n";
            std::cerr << "Drawing the graph to: '" << filename << "'\n";
            
            drag::draw_svg_file(filename, g);
            return false;
        }    
    }

    return true;
}

int main() {

}
