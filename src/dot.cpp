#include <iostream>

#include "graph.hpp"
#include "layering.hpp"

template<typename Son>
struct base {
    base() { f(); }
    void f() { static_cast<Son&>(*this).g(); }
};

struct derived : base<derived> {
    derived() : base() { }
    void g() { std::cout << "derived::g\n"; }
};

int main() {
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

    auto ranking = initialize_ranking(g);
    
    for (vertex_t u = 0; u < g.size(); ++u) {
        std::cout << u << ": " << ranking[u] << "\n";
    }
    std::cout << "\n";

    auto tree = initialize_tree(g, ranking);
    std::cout << "\n";
    std::cout << tree;
}