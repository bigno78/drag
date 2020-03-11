#pragma once

#include <vector>
#include <algorithm>

#include "stuff.hpp"
#include "interface.hpp"


namespace detail {


/**
 * Subgraph of a given graph - subset of its vertices + all edges between those vertices.
 * 
 * Vertices which are added after the construction are called dummy vertices. 
 * They are equal to the original vertices (there can be eges added or removed between them), but their size is always zero.
 * They can be used to distinguis between the vertices of the original graph and vertices which were added for "algorithmic" purpouses.
 */
class subgraph {
    graph& m_source;
    std::vector< vertex_t > m_vertices;
    vertex_t m_dummy_border;
    
public:
    subgraph(graph& g, std::vector< vertex_t > vertices) 
        : m_source(g)
        , m_vertices(std::move(vertices)) {
            m_dummy_border = *std::max_element(m_vertices.begin(), m_vertices.end());
        }

    unsigned size() const { return m_vertices.size(); }

    void add_edge(edge e) { m_source.add_edge(e.tail, e.head); }
    void add_edge(vertex_t u, vertex_t v) { add_edge( { u, v } ); }

    vertex_t add_dummy() { 
        auto u = m_source.add_node(0);
        m_vertices.push_back(u);
        return u;
    }

    bool is_dummy(vertex_t u) const { return u > m_dummy_border; }

    void remove_edge(edge e) { m_source.remove_edge(e.tail, e.head); }
    void remove_edge(vertex_t u, vertex_t v) { remove_edge( { u, v } ); }

    float node_size(vertex_t u) const { return m_source.node_size(u); }

    bool has_edge(edge e) const { 
        auto out = out_neighbours(e.tail);
        return std::find(out.begin(), out.end(), e.head) != out.end();
    }
    bool has_edge(vertex_t u, vertex_t v) { return has_edge( { u, v } ); }

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
};


inline std::ostream& operator<<(std::ostream& out, const subgraph& g) {
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


/**
 * Recursively assign u and all vertices reachable from u in the underlying undirected graph to the same component.
 */
inline void split(const graph& g, std::vector<bool>& done, std::vector<vertex_t>& component, vertex_t u) {
    done[u] = true;
    component.push_back(u);
    for (auto v : g.out_neighbours(u)) {
        if (!done[v]) {
            split(g, done, component, v);
        }
    }
    for (auto v : g.in_neighbours(u)) {
        if (!done[v]) {
            split(g, done, component, v);
        }
    }
}


/**
 * Split the given graph into connected components represented by subgrapgs.
 */
inline std::vector<subgraph> split(graph& g) {
    std::vector< std::vector<vertex_t> > components;
    std::vector< bool > done(g.size(), false);

    for (auto u : g.vertices()) {
        if (!done[u]) {
            components.emplace_back();
            split(g, done, components.back(), u);
        }
    }

    std::vector<subgraph> subgraphs;
    for (auto component : components) {
        subgraphs.emplace_back(g, component);
    }

    return subgraphs;
}


/**
 * Stores a flag of type T for each vertex in the given subgraph
 */
template< typename T >
struct vertex_flags {
    std::vector< T > flags;

    vertex_flags() = default;

    vertex_flags(const graph& g) : flags(g.size(), T{}) {}
    vertex_flags(const graph& g, T val) : flags(g.size(), val) {}

    vertex_flags(const subgraph& g) : vertex_flags(g, T{}) {}
    vertex_flags(const subgraph& g, T val) {
        for (auto u : g.vertices()) {
            if (u >= flags.size()) {
                flags.resize(u + 1);
            }
            flags[u] = val;
        }
    }

    // only to be used with T = bool, because the operator[] doesnt work :(
    T at(vertex_t u) const { return flags[u]; }
    void set(vertex_t u, T val) { flags[u] = val; }

    T& operator[](vertex_t u) { return flags[u]; }
    const T& operator[](vertex_t u) const { return flags[u]; }

    void add_vertex(vertex_t u) {
        if (u >= flags.size()) {
            flags.resize(u + 1);
        }
    }
};



} //namespace detail
