#include "catch.hpp"
#include "../graph.hpp"


TEST_CASE("edge tests") {
    edge e = { 0, 1 };
    edge rev = reversed(e);
    REQUIRE( rev.tail == e.head );
    REQUIRE( rev.head == e.tail );
}

TEST_CASE("graph size") {
    SECTION("") {
        graph g = graph_builder()
                .add_edge(0, 1)
                .add_edge(9, 2)
                .add_edge(4, 3)
                .build();
    REQUIRE( g.size() == 10 );
    }
    SECTION("") {
        graph g = graph_builder()
                .add_edge(0, 1)
                .add_edge(2, 8)
                .add_edge(4, 3)
                .build();
    REQUIRE( g.size() == 9 );
    }
}

bool is_edge(const graph& g, vertex_t u, vertex_t v) {
    return g.is_edge(u, v) && g.is_edge( { u, v } );
}
bool is_connected(const graph& g, vertex_t u, vertex_t v) {
    return g.is_connected(u, v) && g.is_connected( { u, v } );
}

TEST_CASE("checking for edges") {
    graph g = graph_builder()
                .add_edge(0, 1)
                .add_edge(1, 0)
                .add_edge(4, 3)
                .build();

    REQUIRE( is_edge(g, 0, 1) );
    REQUIRE( is_edge(g, 1, 0) );
    REQUIRE( is_edge(g, 4, 3) );
    REQUIRE( is_connected(g, 4, 3) );
    REQUIRE( is_connected(g, 3, 4) );
    REQUIRE( is_connected(g, 0, 1) );
    REQUIRE( is_connected(g, 1, 0) );
    REQUIRE_FALSE( is_edge(g, 3, 4) );
    REQUIRE_FALSE( is_edge(g, 2, 1) );
    REQUIRE_FALSE( is_connected(g, 2, 1) );
}

TEST_CASE("adding edges") {
    graph g = graph_builder()
                .add_edge(5, 8)
                .build();

    SECTION("two vetices") {
        g.add_edge(2,4);
        REQUIRE( is_edge(g, 2, 4) );
        REQUIRE( is_connected(g, 2, 4) );
        REQUIRE( is_connected(g, 4, 2) );
        REQUIRE_FALSE( is_edge(g, 4, 2) );

        g.add_edge(1, 1);
        REQUIRE( is_edge(g, 1, 1) );
        REQUIRE( is_connected(g, 1, 1) );

        g.add_edge(2, 4);
        REQUIRE( is_edge(g, 2, 4) );
        REQUIRE( is_connected(g, 2, 4) );
        REQUIRE( is_connected(g, 4, 2) );
        REQUIRE_FALSE( is_edge(g, 4, 2) );

        g.add_edge(4, 2);
        REQUIRE( is_edge(g, 2, 4) );
        REQUIRE( is_connected(g, 2, 4) );
        REQUIRE( is_connected(g, 4, 2) );
        REQUIRE( is_edge(g, 4, 2) );
    }

    SECTION("edge struct") {
        g.add_edge({2,4});
        REQUIRE( is_edge(g, 2, 4) );
        REQUIRE( is_connected(g, 2, 4) );
        REQUIRE( is_connected(g, 4, 2) );
        REQUIRE_FALSE( is_edge(g, 4, 2) );

        g.add_edge({1, 1});
        REQUIRE( is_edge(g, 1, 1) );
        REQUIRE( is_connected(g, 1, 1) );

        g.add_edge({2, 4});
        REQUIRE( is_edge(g, 2, 4) );
        REQUIRE( is_connected(g, 2, 4) );
        REQUIRE( is_connected(g, 4, 2) );
        REQUIRE_FALSE( is_edge(g, 4, 2) );

        g.add_edge({4, 2});
        REQUIRE( is_edge(g, 2, 4) );
        REQUIRE( is_connected(g, 2, 4) );
        REQUIRE( is_connected(g, 4, 2) );
        REQUIRE( is_edge(g, 4, 2) );
    }
}

TEST_CASE("removing edges") {
    graph g = graph_builder()
                .add_edge(8, 5)
                .add_edge(5, 8)
                .add_edge(1, 6)
                .add_edge(1, 1)
                .build();

    SECTION("two vertices") {
        g.remove_edge(8, 5);
        REQUIRE(is_edge(g, 5, 8));
        REQUIRE(is_edge(g, 1, 6));
        REQUIRE(is_connected(g, 5, 8));
        REQUIRE_FALSE(is_edge(g, 8, 5));

        g.remove_edge(5, 8);
        REQUIRE_FALSE(is_edge(g, 5, 8));
        REQUIRE(is_edge(g, 1, 6));
        REQUIRE_FALSE(is_connected(g, 5, 8));
        REQUIRE_FALSE(is_edge(g, 8, 5));

        g.remove_edge(1, 1);
        REQUIRE(is_edge(g, 1, 6));
        REQUIRE_FALSE(is_edge(g, 1, 1));
    }

    SECTION("edge struct") {
        g.remove_edge({8, 5});
        REQUIRE(is_edge(g, 5, 8));
        REQUIRE(is_edge(g, 1, 6));
        REQUIRE(is_connected(g, 5, 8));
        REQUIRE_FALSE(is_edge(g, 8, 5));

        g.remove_edge({5, 8});
        REQUIRE_FALSE(is_edge(g, 5, 8));
        REQUIRE(is_edge(g, 1, 6));
        REQUIRE_FALSE(is_connected(g, 5, 8));
        REQUIRE_FALSE(is_edge(g, 8, 5));

        g.remove_edge({1, 1});
        REQUIRE(is_edge(g, 1, 6));
        REQUIRE_FALSE(is_edge(g, 1, 1));
    }
}
