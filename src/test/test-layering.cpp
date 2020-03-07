#include<algorithm>

#include "catch.hpp"
#include "../layering.hpp"
#include "test-utils.hpp"

using namespace detail;

void check_hierarchy(const subgraph& g, const hierarchy& h) {
    int l = 0;
    for (auto layer : h.layers) {
        for (auto u : layer) {
            REQUIRE( h.ranking[u] == l);
            for (auto v : g.in_neighbours(u)) {
                std::cout << edge{v, u} << "\n";
                REQUIRE( h.ranking[v] < l);
            }
            for (auto v : g.out_neighbours(u)) {
                REQUIRE( h.ranking[v] > l);
            }
        }
        ++l;
    }
}

TEST_CASE("Layering stuff.") {
    graph source = graph_builder()
                .add_edge(0, 1).add_edge(0, 2).add_edge(0, 6)
                .add_edge(1, 6).add_edge(1, 3).add_edge(6, 4)
                .add_edge(6, 3).add_edge(5, 4)
                .build();
    subgraph g = make_sub(source);

    network_simplex_layering layering(source);
    hierarchy h = layering.run(g);

    check_hierarchy(g, h);
}

TEST_CASE("Splitting long edges.") {
    graph source = graph_builder()
                .add_edge(0, 3).add_edge(0, 4).add_edge(0, 5)
                .add_edge(3, 1).add_edge(3, 2).add_edge(3, 4)
                .build();
    subgraph g = make_sub(source);

    vertex_flags<int> ranking(source);
    ranking[0] = 0;
    ranking[1] = 3;
    ranking[2] = 3;
    ranking[3] = 1;
    ranking[4] = 2;
    ranking[5] = 4;
    hierarchy h = { ranking, { { 0 },
                               { 3 },
                               { 4 },
                               { 1, 2 },
                               { 5 } } };
    
    auto edges = add_dummy_nodes(g, h);

    check_hierarchy(g, h);

    for (auto u : g.vertices()) {
        for (auto v : g.out_neighbours(u)) {
            std::cout << u << " " << v << "\n";
            REQUIRE(h.span(u, v) == 1);
        }
    }

    REQUIRE(ranking[0] == 0);
    REQUIRE(ranking[1] == 3);
    REQUIRE(ranking[2] == 3);
    REQUIRE(ranking[3] == 1);
    REQUIRE(ranking[4] == 2);
    REQUIRE(ranking[5] == 4);

    for (const auto& [ e, path ] : edges) {
        REQUIRE( e.tail == path.front() );
        REQUIRE( e.head == path.back() );
        for (int i = 1; i < path.size(); ++i) {
            REQUIRE( has_edge(g, { path[i - 1], path[i] }) );
        }
    }
}

/*
TEST_CASE("initial ranking") {
    graph g = graph_builder()
                .add_edge(0, 5).add_edge(0, 4)
                .add_edge(1, 5).add_edge(1, 4).add_edge(1, 3)
                .add_edge(5, 2)
                .add_edge(3, 4).add_edge(3, 2).add_edge(3, 6)
                .add_edge(4, 6)
                .add_edge(2, 6)
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

bool check_tree(std::vector<bool>& reachable, tight_tree& tree, std::vector<int>& ranking, vertex_t current, int parent) {
    reachable[current] = true;
    if (tree.node(current).parent != parent) {
        return false;
    }

    for (auto u : tree.node(current).children) {
        if (std::abs(edge_span(ranking, {current, u})) != 1) {
            return false;
        }
        if (!check_tree(reachable, tree, ranking, u, current)) {
            return false;
        }
    }
    return true;
}

TEST_CASE("initializing the spanning tree") {
    graph g = graph_builder()
                .add_edge(0, 11).add_edge(0, 6).add_edge(0, 4)
                .add_edge(1, 2).add_edge(1, 5)
                .add_edge(2, 5)
                .add_edge(3, 8).add_edge(3, 6)
                .add_edge(4, 9).add_edge(4, 10).add_edge(4, 7)
                .add_edge(5, 8)
                .add_edge(6, 9)
                .add_edge(7, 10)
                .add_edge(11, 1)
                .build();
    
    std::vector<int> ranking = initialize_ranking(g);
    tight_tree tree = initialize_tree(g, ranking);

    for (vertex_t u = 0; u < g.size(); ++u) {
        for (auto v : g.out_edges(u)) {
            REQUIRE( ranking[v] > ranking[u] );
        }
    }

    std::vector<bool> reachable(g.size(), false);
    REQUIRE( check_tree(reachable, tree, ranking, tree.root, -1) );

    for (vertex_t u = 0; u < g.size(); ++u) {
        REQUIRE( reachable[u] );
    }
}

*/