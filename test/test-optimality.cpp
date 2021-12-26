#include "utils/test-utils.hpp"
#include "utils/randomized.hpp"
#include "utils/bruteforce.hpp"

#include <drag/drag.hpp>

#include <iostream>
#include <fstream>
#include <sstream>


int main(int argc, char **argv) {
    
    // let's just use a fixed seed so the tests are replicable
    size_t seed = 42;

    Tester tester(seed);

    auto test_function = [] (drag::graph& g) {
        drag::detail::subgraph sub = make_subgraph(g);
        drag::detail::network_simplex_layering layering;
        auto hierarchy = layering.run(sub);

        auto res = get_total_edge_length(hierarchy);

        auto expected = bruteforce_layering_total_length(g);

        return res == expected;
    };

    test_config small_config;
    small_config.n = 15;
    small_config.m = 22;
    small_config.count = 20;

    tester.register_test("low_density", small_config, test_function);

    return tester.run_tests();
}
