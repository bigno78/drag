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
    std::vector< node > m_node_size;

    float default_radius;

public:
    vertex_t add_node() { return add_node(default_radius); }
    vertex_t add_node(unsigned radius) {
        m_out_neighbours.emplace_back();
        m_in_neighbours.emplace_back();
        m_node_size.push_back( { radius } );
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


    void split_components() {
        m_components.resize(size(), -1);

        int component = 0;
        for (vertex_t u = 0; u < size(); ++u) {
            if (m_components[u] == -1) {
                split_components(u, component);
                ++component;
            }
        }
        m_component_count = component;
    }

    void split_components(vertex_t u, int component) {
        m_components[u] = component;
        for (auto v : m_out_neighbours[u]) {
            if (m_components[v] == -1) {
                split_components(v, component);
            }
        }
        for (auto v : m_in_neighbours[u]) {
            if (m_components[v] == -1) {
                split_components(v, component);
            }
        }
    }

    void init_subgraphs() {
        m_subgraphs.reserve(m_component_count);
        m_mapping.resize(size());

        std::vector<int> counts(m_component_count, 0);
        for (vertex_t u = 0; u < size(); ++u) {
            m_mapping[u] = counts[m_components[u]]++;
            std::cout << u << " -> " << m_mapping[u] << "\n";
        }

        for (int i = 0; i < m_component_count; ++i) {
            m_subgraphs.emplace_back(counts[i]);
        }

        for (vertex_t u = 0; u < size(); ++u) {
            for (auto v : m_out_neighbours[u]) {
                m_subgraphs[m_components[u]].add_edge(m_mapping[u], m_mapping[v]);
            }
        }
    }
};