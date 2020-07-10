#include "catch.hpp"
#include "../graph.hpp"

template<typename T>
bool vector_contains(const std::vector<T>& vec, const T& elem) {
    auto it = std::find(vec.begin(), vec.end(), elem);
    return it != vec.end();
} 

void assert_vertices_equal(const graph& g, const std::vector<vertex_t>& expected) {
    int count = 0;
    for (auto u : g.vertices()) {
        ++count;
        REQUIRE( vector_contains(expected, u) );
    }
    REQUIRE( count == expected.size() );
}

void assert_neighbours_equal(const std::vector<vertex_t>& given, const std::vector<vertex_t>& expected) {
    int count = 0;
    for (auto u : given) {
        ++count;
        REQUIRE( vector_contains(expected, u) );
    }
    REQUIRE( count == expected.size() );
}

TEST_CASE("changing default node size") {
    graph g;
    float old_default_size = g.default_size();
    float new_default_size = old_default_size + 10;
    
    g.default_size(new_default_size);

    REQUIRE(g.default_size() == new_default_size);
}

TEST_CASE("empty graph") {
    graph g;

    REQUIRE( g.size() == 0 );
    assert_vertices_equal(g, {});
}

TEST_CASE("adding nodes") {
    graph g;    
    float old_default_size = g.default_size();

    auto a = g.add_node();
    
    float new_default_size = old_default_size + 10;
    g.default_size(new_default_size);
    auto b = g.add_node();

    float custom_size = new_default_size + 10;
    auto c = g.add_node(custom_size);

    REQUIRE( g.size() == 3 );
    
    REQUIRE( g.node_size(a) == old_default_size );
    REQUIRE( g.node_size(b) == new_default_size );
    REQUIRE( g.node_size(c) == custom_size );
    
    assert_vertices_equal(g, { a, b, c });

    assert_neighbours_equal(g.out_neighbours(a), {});
    assert_neighbours_equal(g.in_neighbours(a), {});
    assert_neighbours_equal(g.out_neighbours(b), {});
    assert_neighbours_equal(g.in_neighbours(b), {});
    assert_neighbours_equal(g.out_neighbours(c), {});
    assert_neighbours_equal(g.in_neighbours(c), {});
}

TEST_CASE("adding edges") {
    graph g;
    auto a = g.add_node();
    auto b = g.add_node();
    auto c = g.add_node();
    auto d = g.add_node();

    g.add_edge(a, c);
    g.add_edge(d, b);
    g.add_edge(d, a);

    assert_neighbours_equal(g.out_neighbours(a), {c});
    assert_neighbours_equal(g.out_neighbours(b), {});
    assert_neighbours_equal(g.out_neighbours(c), {});
    assert_neighbours_equal(g.out_neighbours(d), {b, a});

    assert_neighbours_equal(g.in_neighbours(a), {d});
    assert_neighbours_equal(g.in_neighbours(b), {d});
    assert_neighbours_equal(g.in_neighbours(c), {a});
    assert_neighbours_equal(g.in_neighbours(d), {});
}

TEST_CASE("removing edges") {
    graph g;
    auto a = g.add_node();
    auto b = g.add_node();
    auto c = g.add_node();
    auto d = g.add_node();
    g.add_edge(a, b);
    g.add_edge(a, c);
    g.add_edge(c, d);

    g.remove_edge(a, c);

    assert_neighbours_equal(g.out_neighbours(a), {b});
    assert_neighbours_equal(g.out_neighbours(b), {});
    assert_neighbours_equal(g.out_neighbours(c), {d});
    assert_neighbours_equal(g.out_neighbours(d), {});

    assert_neighbours_equal(g.in_neighbours(a), {});
    assert_neighbours_equal(g.in_neighbours(b), {a});
    assert_neighbours_equal(g.in_neighbours(c), {});
    assert_neighbours_equal(g.in_neighbours(d), {c});
}

TEST_CASE("graph builder") {
    graph g = graph_builder()
                .add_edge(0, 1)
                .add_edge(1, 3)
                .build();

    REQUIRE(g.size() == 4);
    assert_vertices_equal(g, {0, 1, 2, 3});

    assert_neighbours_equal(g.out_neighbours(0), {1});
    assert_neighbours_equal(g.out_neighbours(1), {3});
    assert_neighbours_equal(g.out_neighbours(2), {});
    assert_neighbours_equal(g.out_neighbours(3), {});

    assert_neighbours_equal(g.in_neighbours(0), {});
    assert_neighbours_equal(g.in_neighbours(1), {0});
    assert_neighbours_equal(g.in_neighbours(2), {});
    assert_neighbours_equal(g.in_neighbours(3), {1});
}
