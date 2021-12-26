#include "catch.hpp"
#include "utils/test-utils.hpp"

#include <drag/detail/subgraph.hpp>

using namespace drag;
using namespace drag::detail;


template<typename T>
bool vector_contains(const std::vector<T>& vec, const T& elem) {
    auto it = std::find(vec.begin(), vec.end(), elem);
    return it != vec.end();
} 

static void assert_vertices_equal(const graph& g, const std::vector<vertex_t>& expected) {
    int count = 0;
    for (auto u : g.vertices()) {
        ++count;
        REQUIRE( vector_contains(expected, u) );
    }
    REQUIRE( count == expected.size() );
}

void assert_has_edge(const subgraph& g, vertex_t u, vertex_t v) {
    REQUIRE( g.has_edge(u, v) );
    REQUIRE( g.has_edge({u, v}) );
}

static void assert_neighbours_equal(const std::vector<vertex_t>& given, const std::vector<vertex_t>& expected) {
    int count = 0;
    for (auto u : given) {
        ++count;
        REQUIRE( vector_contains(expected, u) );
    }
    REQUIRE( count == expected.size() );
}

TEST_CASE("creating subgrap") {
    graph source = graph_builder()
                .add_edge(0, 1)
                .add_edge(0, 2)
                .build();
    
    subgraph g(source, { 0, 1, 2 });

    REQUIRE( g.size() == 3 );
    REQUIRE( g.size() == 3 );
    assert_has_edge(g, 0, 1);
    assert_has_edge(g, 0, 2);
}

// ------------------------  OLD STUFF -----------------------------------  

graph source = graph_builder()
            .add_edge(0, 3).add_edge(0, 5)
            .add_edge(3, 5)

            .add_edge(1, 4).add_edge(2, 4)
            .add_edge(4, 6).add_edge(6, 2)
            .build();

subgraph g( source, {1, 4, 2, 6} );

template<typename Iterable>
bool contains(const std::vector<vertex_t>& expected, const Iterable& obj) {
    for (const auto& x : obj) {
        if ( std::find(expected.begin(), expected.end(), x) == expected.end() ) {
            return false;
        }
    }
    return true;
}

TEST_CASE("Graph access tests.") {
    REQUIRE( g.size() == 4 );
    REQUIRE( contains({ 1, 2, 4, 6 }, g.vertices()) );
    REQUIRE( contains({ 1, 2}, g.in_neighbours(4)) );
    REQUIRE( contains({ 6 }, g.out_neighbours(4)) );
    REQUIRE( contains({ 1, 2, 6}, g.neighbours(4)) );
    for (auto u : g.vertices()) {
        REQUIRE(!g.is_dummy(u));
    }
}

TEST_CASE("Graph modification tests.") {
    graph source = graph_builder()
            .add_edge(2, 0).add_edge(3, 0)
            .add_edge(1, 4)
            .build();
    subgraph g( source, {0, 2, 3} );

    auto u = g.add_dummy();
    REQUIRE( u == 5 );
    REQUIRE( g.size() == 4 );
    REQUIRE( g.is_dummy(u) );
    REQUIRE( contains({}, g.neighbours(u)) );

    g.add_edge(0, u);
    REQUIRE( contains({ u }, g.out_neighbours(0)) );
    REQUIRE( contains({ 0 }, g.in_neighbours(u)) );
    REQUIRE( contains({}, g.out_neighbours(u)) );

    g.remove_edge(3, 0);
    REQUIRE( contains({}, g.out_neighbours(3)) );
    REQUIRE( contains({}, g.in_neighbours(3)) );
    REQUIRE( contains({ 2 }, g.in_neighbours(0)) );
}

TEST_CASE("Splitting graph into components - connected graph.") {
    graph source = graph_builder()
            .add_edge(1, 0).add_edge(0, 2).add_edge(0, 3)
            .add_edge(2, 0).add_edge(3, 1)
            .build();

    std::vector<subgraph> subs = split(source);

    REQUIRE(subs.size() == 1);
    REQUIRE( contains({ 0, 1, 2, 3}, subs.front().vertices()) );
}


bool compare_components(std::vector< std::vector<vertex_t> > expected, const std::vector<subgraph>& subs) {
    for (const auto& g : subs) {
        bool found = false;
        for (const auto& vec : expected) {
            found = found || contains(vec, g.vertices());
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

TEST_CASE("Splitting graph into components - multiple components.") {
    std::vector<subgraph> subs = split(source);

    REQUIRE( subs.size() == 2 );
    REQUIRE( compare_components({ { 1, 2, 4, 6 }, { 0, 3, 5 } }, subs) );
}
