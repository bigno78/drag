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
                .add_edge(0, 2).add_edge(0, 4)
                .add_edge(1, 2).add_edge(1, 4).add_edge(1, 3)
                .add_edge(2, 5)
                .add_edge(3, 4).add_edge(3, 5).add_edge(3, 6)
                .add_edge(4, 6)
                .add_edge(5, 6)
                .build();

    auto ranking = initialize_ranking(g);

    for (vertex_t u = 0; u < g.size(); ++u) {
        std::cout << u << ": " << ranking[u] << "\n";
    }

    auto tree = initialize_tree(g, ranking);
}