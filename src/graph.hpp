#ifndef GRAPH_HPP
#define GRAPH_HPP

#pragma once

#include <vector>
#include <iostream>
#include <ostream>
#include <algorithm>

#include "utils.hpp"
#include "types.hpp"


class graph {
    std::vector< std::vector<vertex_t> > m_out_neighbours;
    std::vector< std::vector<vertex_t> > m_in_neighbours;
    std::vector< float > m_node_size;

    float m_default_radius = defaults::node_size;

public:
    // add node with default radius
    vertex_t add_node() { return add_node(m_default_radius); }
    
    // add node with a given radius
    vertex_t add_node(float radius) {
        m_out_neighbours.emplace_back();
        m_in_neighbours.emplace_back();
        m_node_size.push_back(radius);
        return m_node_size.size() - 1;
    }

    graph& add_edge(vertex_t from, vertex_t to) { 
        m_out_neighbours[from].push_back(to);
        m_in_neighbours[to].push_back(from);
        return *this;
    }

    void remove_edge(vertex_t from, vertex_t to) {
        remove_neighour(m_out_neighbours[from], to);
        remove_neighour(m_in_neighbours[to], from);
    }

    // get the size of <u>
    float node_size(vertex_t u) const { return m_node_size[u]; }

    // set the size of <u> to <r>
    void node_size(vertex_t u, float r) { m_node_size[u] = r; }

    // set the default size of nodes to <r>
    void default_size(float r) { m_default_radius = r; }

    // get the default size
    float default_size() const { return m_default_radius; }

    unsigned size() const { return m_node_size.size(); }

    const std::vector<vertex_t>& out_neighbours(vertex_t u) const { return m_out_neighbours[u]; }
    const std::vector<vertex_t>& in_neighbours(vertex_t u) const { return m_in_neighbours[u]; }
    range<vertex_t> vertices() const { return range<vertex_t>(0, size(), 1); }

    friend std::ostream& operator<<(std::ostream& out, const graph& g) {
        for (auto u : g.vertices()) {
            out << u << "(" << g.node_size(u) << "): [";

            const char* sep = "";
            for (auto v : g.out_neighbours(u)) {
                out << sep << v;
                sep = ", ";
            }

            out << "]\n";
        }
        return out;
    }

private:

    void remove_neighour(std::vector<vertex_t>& neighbours, vertex_t u) {
        auto it = std::find(neighbours.begin(), neighbours.end(), u);
        if (it != neighbours.end()) {
            neighbours.erase(it);
        }
    }

};


struct graph_builder {
    graph g;

    graph_builder& add_edge(vertex_t u, vertex_t v) {
        add_vertex(u);
        add_vertex(v);
        g.add_edge(u, v);
        return *this;
    }

    graph build() { return g; }
    
private:
    void add_vertex(vertex_t u) {
        while (u >= g.size()) {
            g.add_node();
        }
    }
};

#endif
