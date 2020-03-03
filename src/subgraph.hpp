#pragma once

#include <vector>
#include <algorithm>

#include "stuff.hpp"
#include "interface.hpp"


namespace detail {

class subgraph {
    graph& m_source;
    std::vector< vertex_t > m_vertices;
    
public:
    subgraph(graph& g, std::vector< vertex_t > vertices) 
        : m_source(g)
        , m_vertices(std::move(vertices)) {}

    unsigned size() const { return m_vertices.size(); }

    void add_edge(edge e) { m_source.add_edge(e.tail, e.head); }
    void add_edge(vertex_t u, vertex_t v) { add_edge( { u, v } ); }

    vertex_t add_vertex() { 
        auto u = m_source.add_node();
        m_vertices.push_back(u); 
    }
    vertex_t add_vertex(float radius) { 
        auto u = m_source.add_node(radius);
        m_vertices.push_back(u);
    }

    void remove_edge(edge e) { m_source.remove_edge(e.tail, e.head); }
    void remove_edge(vertex_t u, vertex_t v) { remove_edge( { u, v } ); }


    //bool is_edge(edge e) const { ... }
    //bool is_edge(vertex_t u, vertex_t v) const { ... }

    //bool is_connected(edge e) const { ... }
    //bool is_connected(vertex_t u, vertex_t v) const { ... }

    /*int edge_orientation(edge e) const { 
        if ( is_edge(e) )
            return 1;
        if ( is_edge(reversed(e)) )
            return -1;
        return 0;
    }
    int edge_orientation(vertex_t u, vertex_t v) const { return edge_orientation( {u, v} ); }*/

    /*void add_vertices(unsigned count) {
        unsigned new_size = size() + count;
        for (auto& row : matrix) {
            row.resize(new_size, false);
        }
        for (int i = 0; i < count; ++i) {
            matrix.emplace_back(new_size, false);
        }
    }*/

    const std::vector<vertex_t>& out_neighbours(vertex_t u) const { return m_source.out_neighbours(u); }
    const std::vector<vertex_t>& in_neighbours(vertex_t u) const { return m_source.in_neighbours(u); }
    chain_range< std::vector<vertex_t> > neighbours(vertex_t u) const { return { out_neighbours(u), in_neighbours(u) }; }

    const std::vector<vertex_t>& vertices() const { return m_vertices; }

    friend std::ostream& operator<<(std::ostream& out, const subgraph& g) {
        for (auto u : g.vertices()) {
            out << u << ": {";
            const char* sep = "";
            for (auto v : g.out_neighbours(u)) {
                out << sep << v;
                sep = ", ";
            }
            out << "}\n";
        }
        return out;
    }
};

/**
 * Stores a flag of type T for each vertex in the given subgraph
 */
template< typename T >
struct vertex_flags {
    std::vector< T > flags;

    vertex_flags(const subgraph& g) : vertex_flags(g, T{}) {}
    
    vertex_flags(const subgraph& g, T val) {
        for (auto u : g.vertices()) {
            if (u >= flags.size()) {
                flags.resize(u + 1);
            }
            flags[u] = val;
        }
    }

    T& operator[](vertex_t u) { return flags[ mapping[u] ]; }
    const T& operator[](vertex_t u) const { return flags[ mapping[u] ]; }
};

/*
struct graph_builder {
    std::vector<edge> edges;
    unsigned size = 0;

    graph_builder& add_edge(unsigned u, unsigned v) {
        size = std::max(size, std::max(u + 1, v + 1));
        edges.push_back({u, v});
        return *this;
    }

    subgraph build() {
        subgraph g(size);
        for (auto e : edges) {
            g.add_edge(e);
        }
        return g;
    }
};
*/

} //namespace detail
