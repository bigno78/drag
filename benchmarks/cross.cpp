#include "stats.hpp"

#include <drag/detail/gen.hpp>
#include <drag/detail/subgraph.hpp>
#include <drag/detail/layering.hpp>
#include <drag/detail/crossing.hpp>

#include <iostream>
#include <algorithm> // sort

int main() {
    drag::dag_generator gen;
    drag::detail::network_simplex_layering layering;
    drag::detail::barycentric_heuristic crossmin;
    
    std::vector<size_t> before;
    std::vector<size_t> results;

    for (size_t i = 0; i < 100; ++i) {
        auto g = gen.generate_from_edges(50, 90);
        drag::detail::subgraph sub(g);
        auto hierarchy = layering.run(sub);
        
        before.push_back( drag::detail::count_crossings(hierarchy) );

        crossmin.run(hierarchy);
        results.push_back( drag::detail::count_crossings(hierarchy) );
    }

    std::sort(before.begin(), before.end());
    std::sort(results.begin(), results.end());

    std::cout << "BEFORE CROSSSING MINIMIZATION\n";
    std::cout << "average:     " << average(before) << "\n";
    std::cout << "median:      " << median(before) << "\n";
    std::cout << "95-quantile: " << quantile(before, 0.95) << "\n";

    std::cout << "\n";

    std::cout << "AFTER MINIMIZATION\n";
    std::cout << "average:     " << average(results) << "\n";
    std::cout << "median:      " << median(results) << "\n";
    std::cout << "95-quantile: " << quantile(results, 0.95) << "\n";
}
