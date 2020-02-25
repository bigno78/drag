#pragma once

#include <vector>
#include <iostream>

#include "stuff.hpp"
#include "subgraph.hpp"

class graph {
    std::vector< std::vector<vertex_t> > m_out_neighbours;
    std::vector< std::vector<vertex_t> > m_in_neighbours;
    std::vector< node > m_nodes;
    std::vector< int > m_components;
    std::vector< subgraph > m_subgraphs;
    std::vector< vertex_t > m_mapping;
    int m_component_count;

    float default_width, default_height;

public:
    vertex_t add_node() { return add_node(default_width, default_height); }
    vertex_t add_node(unsigned radius) { return add_node(radius, radius); }
    vertex_t add_node(unsigned width, unsigned haight) {
        m_out_neighbours.emplace_back();
        m_in_neighbours.emplace_back();
        m_nodes.push_back( { default_width, default_height } );
        return m_nodes.size() - 1;
    }
    void add_edge(vertex_t tail, vertex_t head) { 
        m_out_neighbours[tail].push_back(head);
        m_in_neighbours[head].push_back(tail); 
    }

    void operator()() {
        split_components();
        init_subgraphs();
        for (auto& g : m_subgraphs) {
            std::cout << g << "\n\n";
        }
    }

    unsigned size() const { return m_nodes.size(); }

private:
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