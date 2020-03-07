#pragma once

#include <vector>
#include <iostream>
#include <algorithm>

#include "stuff.hpp"

/**
 * graph g;
 * g.add_node();
 * ...
 * g.add_node();
 * g.add_edge();
 * ...
 * g.add_edge();
 * 
 * g.build();
 * for (auto u : g.nodes()) {
 *     draw the node;
 * }
 * for (auto e : g.edges()) {
 *     draw the edge;
 * }
 * 
 * ---------------- g.build() --------------------------
 * layout_engine.build();
 * 
 * ------------- layout_engine.build() -------------------
 * split_into_subgraphs(g);
 * for ( auto g : subgraphs ) {
 *     layout = process_subgraph();
 * }
 * 
 * ------------- process_subgraph() ---------------------
 * reverse_edges();
 * hierarchy = layer();
 * paths = split_long_edges();
 * minimize_crossing( hierarchy );
 * node_positions = position_nodes( hierarchy );
 * return layout { paths, node_positions };
 */

class graph {
    std::vector< std::vector<vertex_t> > m_out_neighbours;
    std::vector< std::vector<vertex_t> > m_in_neighbours;
    std::vector< float > m_node_size;

    float default_radius = 10;

public:
    vertex_t add_node() { return add_node(default_radius); }
    vertex_t add_node(float radius) {
        m_out_neighbours.emplace_back();
        m_in_neighbours.emplace_back();
        m_node_size.push_back(radius);
        return m_node_size.size() - 1;
    }

    void add_edge(vertex_t tail, vertex_t head) { 
        m_out_neighbours[tail].push_back(head);
        m_in_neighbours[head].push_back(tail); 
    }

    void remove_edge(vertex_t tail, vertex_t head) {
        remove_neighour(m_out_neighbours[tail], head);
        remove_neighour(m_in_neighbours[head], tail);
    }

    float node_size(vertex_t u) const { return m_node_size[u]; }


    unsigned size() const { return m_node_size.size(); }

    const std::vector<vertex_t>& out_neighbours(vertex_t u) const { return m_out_neighbours[u]; }
    const std::vector<vertex_t>& in_neighbours(vertex_t u) const { return m_in_neighbours[u]; }
    range vertices() const { return range(size()); }

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
