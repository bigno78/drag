#include <memory>
#include <iostream>

#include "catch.hpp"
#include "../cycle.hpp"

bool check_acyclic_(const subgraph& g, std::vector<int>& marks, vertex_t u) {
    marks[u] = -1;
    for (auto v : g.out_edges(u)) {
        if (marks[v] == -1) {
            return false;
        }
        if (marks[v] == 0 && !check_acyclic_(g, marks, v)) {
            return false;
        }
    }
    marks[u] = 1;
    return true;
}

bool check_acyclic(const subgraph& g) {
    std::vector<int> marks(g.size(), 0);
    for (vertex_t u = 0; u < g.size(); ++u) {
        if (marks[u] == 0 && !check_acyclic_(g, marks, u)) {
            return false;
        }
    }
    return true;
}

bool check_edge_count(const subgraph& g, int count) {
    
    for (vertex_t u = 0; u < g.size(); ++u) {
        for (auto v : g.out_edges(u)) {
            --count;
        }
    }

    return count == 0;
}

TEST_CASE("one cycle") {
    subgraph g = graph_builder()
                    .add_edge(0, 1)
                    .add_edge(1, 0)
                    .build();

    std::unique_ptr<cycle_removal> c = std::make_unique<dfs_removal>();
    c->run(g);

    REQUIRE( check_acyclic(g) );
}

TEST_CASE("long cycle") {
    subgraph g = graph_builder()
                    .add_edge(0, 1)
                    .add_edge(1, 2)
                    .add_edge(2, 3)
                    .add_edge(3, 4)
                    .add_edge(4, 0)
                    .build();

    std::unique_ptr<cycle_removal> c = std::make_unique<dfs_removal>();
    c->run(g);

    REQUIRE( check_acyclic(g) );
}

TEST_CASE("two cycles with shared edge") {
    subgraph g = graph_builder()
                    .add_edge(0, 1)
                    .add_edge(1, 2)
                    .add_edge(2, 0)
                    .add_edge(1, 3)
                    .add_edge(3, 0)
                    .build();

    std::unique_ptr<cycle_removal> c = std::make_unique<dfs_removal>();
    c->run(g);

    REQUIRE( check_acyclic(g) );
}


TEST_CASE("two cycles without shared edge") {
    subgraph g = graph_builder()
                    .add_edge(0, 1)
                    .add_edge(1, 2)
                    .add_edge(2, 0)
                    .add_edge(0, 3)
                    .add_edge(3, 4)
                    .add_edge(4, 0)
                    .build();

    std::unique_ptr<cycle_removal> c = std::make_unique<dfs_removal>();
    c->run(g);

    REQUIRE( check_acyclic(g) );
}
