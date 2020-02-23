#pragma once

#include <vector>
#include <algorithm>

#include "stuff.hpp"

class subgraph {
    std::vector< std::vector<bool> > matrix;
    
public:
    unsigned dummy_border = 0;

    subgraph(unsigned n) : matrix( n, std::vector<bool>(n, false) ) {}

    unsigned size() const { return matrix.size(); }

    // ------ graph building --------------------
    void add_edge(edge e) { matrix[e.tail][e.head] = true; }
    void add_edge(vertex_t u, vertex_t v) { matrix[u][v] = true; }
    void remove_edge(edge e) { matrix[e.tail][e.head] = false; }
    void remove_edge(vertex_t u, vertex_t v) { matrix[u][v] = false; }


    bool is_edge(edge e) const { return matrix[e.tail][e.head]; }
    bool is_edge(vertex_t u, vertex_t v) const { return matrix[u][v]; }

    bool is_connected(edge e) const { return matrix[e.head][e.tail] || matrix[e.tail][e.head]; }
    bool is_connected(vertex_t u, vertex_t v) const { return matrix[u][v] || matrix[v][u]; }

    int edge_orientation(edge e) const { 
        if ( is_edge(e) )
            return 1;
        if ( is_edge(reversed(e)) )
            return -1;
        return 0;
    }
    int edge_orientation(vertex_t u, vertex_t v) const { return edge_orientation( {u, v} ); }

    bool has_out_edges(unsigned u) const {
        for (unsigned v = 0; v < size(); ++v) {
            if (is_edge(u, v)) {
                return false;
            }
        }
        return true;
    }
    bool has_in_edges(unsigned u) const {
        for (unsigned v = 0; v < size(); ++v) {
            if (is_edge(v, u)) {
                return false;
            }
        }
        return true;
    }

    void add_vertices(unsigned count) {
        unsigned new_size = size() + count;
        for (auto& row : matrix) {
            row.resize(new_size, false);
        }
        for (int i = 0; i < count; ++i) {
            matrix.emplace_back(new_size, false);
        }
    }

    friend std::ostream& operator<<(std::ostream& out, const subgraph& g) {
        for (unsigned u = 0; u < g.size(); ++u) {
            out << u << ": {";
            const char* sep = "";
            for (unsigned v = 0; v < g.size(); ++v) {
                if (g.is_edge(u, v)) {
                    out << sep << v;
                    sep = ", ";
                }
            }
            out << "}\n";
        }
        return out;
    }

    struct give_me_end_iterator {};

    /**
     * Structure which sole purpouse is to iterate through edges in range based for loop
     */
    template<typename It>
    struct edge_iterable {
        const subgraph& g;
        vertex_t u;

        edge_iterable(const subgraph& g, vertex_t u) : g(g), u(u) {}

        It begin() { return { g, u }; }
        It begin() const { return { g, u }; }

        It end() { return It( g, u, give_me_end_iterator() ); }
        It end() const { return { g, u, give_me_end_iterator() }; }
    };

    /**
     * Generic implementation of iterator for iterating through edges of given vertex.
     * Each derived class needs to provide method 'bool is_edge(vertex_t u, vertex_t v)' which returns true 
     * iff (u, v) is one of the edges the iterator iterates through.
     */
    template<typename Son>
    struct iterator {
        const subgraph& g;
        vertex_t u;
        vertex_t curr = 0;

        iterator(const subgraph& g, vertex_t u) : g(g), u(u) {
            if (!static_cast<Son&>(*this).is_edge(u, curr))
                 ++(*this);
        }
        iterator(const subgraph& g, vertex_t u, give_me_end_iterator /* its just a tag*/) 
            : g(g)
            , u(u)
            , curr(g.size()) {}
        
        bool operator==(const iterator& rhs) const { return u == rhs.u && curr == rhs.curr; }
        bool operator!=(const iterator& rhs) const { return !(*this == rhs); }
        vertex_t operator*() const { return curr; }
        iterator& operator++() {
            if (curr == g.size()) {
                return *this;
            }
            while (++curr < g.size() && !static_cast<Son&>(*this).is_edge(u, curr)) {}
            return *this;
        }
    };

    struct out_iterator : iterator<out_iterator> {
        out_iterator(const subgraph& g, vertex_t u) : iterator(g, u) {}
        out_iterator(const subgraph& g, vertex_t u, give_me_end_iterator tag) : iterator(g, u, tag) {}
        bool is_edge(vertex_t u, vertex_t v) const { return g.is_edge(u, v); }
    };

    struct in_iterator : iterator<in_iterator> {
        in_iterator(const subgraph& g, vertex_t u) : iterator(g, u) {}
        in_iterator(const subgraph& g, vertex_t u, give_me_end_iterator tag) : iterator(g, u, tag) {}
        bool is_edge(vertex_t u, vertex_t v) const { return g.is_edge(v, u); }
    };

    struct edge_iterator : iterator<edge_iterator> {
        edge_iterator(const subgraph& g, vertex_t u) : iterator(g, u) {}
        edge_iterator(const subgraph& g, vertex_t u, give_me_end_iterator tag) : iterator(g, u, tag) {}
        bool is_edge(vertex_t u, vertex_t v) const { return g.is_connected(u, v); }
    };


    edge_iterable<out_iterator> out_edges(unsigned u) const { return { *this, u }; }
    edge_iterable<in_iterator> in_edges(unsigned u) const { return { *this, u }; }
    edge_iterable<edge_iterator> edges(unsigned u) const { return { *this, u }; }
};


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