#include<algorithm>

#include "catch.hpp"
#include "../layering.hpp"

TEST_CASE("initial ranking") {
    graph g = graph_builder()
                .add_edge(0, 2).add_edge(0, 4)
                .add_edge(1, 2).add_edge(1, 4).add_edge(1, 3)
                .add_edge(2, 5)
                .add_edge(3, 4).add_edge(3, 5).add_edge(3, 6)
                .add_edge(4, 6)
                .add_edge(5, 6)
                .build();

    auto ranking = initialize_ranking(g);

    for (vertex_t u = 0; u < g.size(); ++u) {
        for (auto v : g.out_edges(u)) {
            REQUIRE(ranking[v] > ranking[u]);
        }
    }
}

bool check_node(tight_tree& tree, vertex_t u, vertex_t parent, std::vector<vertex_t> children) {
    if (tree.node(u).parent != parent || tree.node(u).u != u) {
        return false;
    }

    if (tree.node(u).children.size() != children.size()) {
        return false;
    }

    for (auto v : children) {
        if (std::find(tree.node(u).children.begin(), tree.node(u).children.end(), v) == tree.node(u).children.end()) {
            return false;
        }
    }
    return true;
}

TEST_CASE("basic tree") {
    graph g = graph_builder()
                .add_edge(0, 3)
                .add_edge(1, 3)
                .add_edge(2, 4).add_edge(2, 7).add_edge(2,5)
                .add_edge(3, 5).add_edge(3, 6)
                .add_edge(4, 9)
                .add_edge(7, 8)
                .build();
    std::vector<int> ranking = { 0, 0, 1, 1, 2, 2, 2, 3, 4, 0 };
    tight_tree tree(g.size());
    tree.root = 2;
    std::vector<bool> done(g.size(), false);

    int finished = basic_tree(g, ranking, done, tree, 2);

    REQUIRE( finished == 7 );
    REQUIRE( check_node(tree, 2, -1, { 4, 5 }) );
    REQUIRE( check_node(tree, 4, 2, {}) );
    REQUIRE( check_node(tree, 5, 2, { 3 }) );
    REQUIRE( check_node(tree, 3, 5, { 0, 1, 6 }) );
    REQUIRE( check_node(tree, 0, 3, {}) );
    REQUIRE( check_node(tree, 1, 3, {}) );
    REQUIRE( check_node(tree, 6, 3, {}) );
    REQUIRE( check_node(tree, 7, -1, {}) );
    REQUIRE( check_node(tree, 8, -1, {}) );
    REQUIRE( check_node(tree, 9, -1, {}) );
}