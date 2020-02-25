#include <memory>

#include "catch.hpp"
#include "../cycle.hpp"

TEST_CASE("one cycle") {
    subgraph g = graph_builder()
                    .add_edge(0, 1)
                    .add_edge(1, 0)
                    .build();

    std::unique_ptr<cycle_removal> c = std::make_unique<dfs_removal>();
    c->run(g);

    
}