#include <iostream>

#include "interface.hpp"

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
    graph g;
    g.add_node();
    g.add_node();
    g.add_node();
    g.add_node();
    g.add_node();
    g.add_edge(1, 2);
    g.add_edge(3, 4);
    g();
}