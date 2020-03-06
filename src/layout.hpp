#pragma once

#include <vector>
#include <memory>

#include "interface.hpp"
#include "subgraph.hpp"

#include "cycle.hpp"
#include "layering.hpp"
#include "positioning.hpp"



class sugiyama_layout {
    graph& g;
    std::vector< detail::subgraph > subgraphs;
    std::vector< node > nodes;

    float node_dist, layer_dist;

    std::unique_ptr< detail::cycle_removal > cycle_rem = std::make_unique< detail::dfs_removal >();
    std::unique_ptr< detail::layering > layering =       std::make_unique< detail::network_simplex_layering >();
    std::unique_ptr< detail::positioning > positioning = 
                std::make_unique< detail::test_positioning >(nodes, detail::positioning_attributes{ node_dist, layer_dist } );

    std::vector< std::vector<vec2> > paths;

public:
    sugiyama_layout(graph& g) : g(g) {}

    void build() {
        subgraphs = detail::split(g);

        for (auto& g : subgraphs) {
            std::vector<edge> rev = cycle_rem->run(g);
            detail::hierarchy h = layering->run(g);
            add_dummy_nodes(g, h, rev);
            positioning->run(g, h);

            for (auto e : rev) {
                reverse_path(e, g);
            }
        }

        construct_paths();
    }

    void node_separation(float d) { node_dist = d; }
    void layer_separation(float d) { layer_dist = d; }

    const std::vector<node>& vertices() const { return nodes; }
    const std::vector< std::vector<vec2> >& edges() const { return paths; }

private:
    void construct_paths() {
        for (auto& subg : subgraphs) {
            for (auto u : subg.vertices()) {
                for (auto v : subg.out_neighbours(u)) {
                    paths.emplace_back(2);
                    while (subg.is_dummy(v)) {
                        paths.back().push_back( nodes[v].pos );
                        v = v = *g.out_neighbours(v).begin();
                    }
                    paths.back().push_back( nodes[v].pos );
                }
            }
        }
    }

    /**
     * Reverses the path which begins with edge e.
     */
    void reverse_path(edge e, detail::subgraph& g) {
        vertex_t u = e.tail;
        vertex_t v = e.head;
        
        while (g.is_dummy(v)) {
            reverse_edge({u, v});
            u = v;
            v = *g.out_neighbours(v).begin();
        }
        reverse_edge({u, v});
    }

    void reverse_edge(edge e) {
        g.remove_edge(e.tail, e.head);
        g.add_edge(e.head, e.tail);
    }

    /**
     * Checks if the vertex u is the last vertex on the path of dummy vertices of length 0 or more
     * which begins with edge e.
     */
    bool ends_with(edge e, vertex_t u, const detail::subgraph& g) const {
        return end_point(e, g) == u;
    }

    /**
     * Finds the endpoint of a path of dummy vertices which begins with edge e.
     */
    vertex_t end_point(edge e, const detail::subgraph& g) const {
        vertex_t u = e.head;
        while (g.is_dummy(u)) {
            u = *g.out_neighbours(u).begin(); // dummy vetex has only one neightbour
        }
        return u;
    }

    /**
     * Splits all edges that span across more than one layer in the provided hierarchy
     * into segments that each span across just one layer.
     * ( aka converts the hierachy into proper hierarchy )
     */
    void add_dummy_nodes(detail::subgraph& g, detail::hierarchy& h, std::vector<edge>& rev) {
        for (auto u : g.vertices()) {
            for (auto v : g.out_neighbours(u)) {
                int span = h.span(u, v);
                if (span > 1) {
                    vertex_t t = v;
                    for (int i = 0; i < span - 1; ++i) {
                        vertex_t s = g.add_dummy();
                        h.ranking.add_vertex(s);
                        h.ranking[s] = h.ranking[t] - 1;
                        h.layers[ h.ranking[s] ].push_back(s);
                        g.add_edge(s, t);
                        t = s;
                    }
                    g.add_edge(u, t);
                    g.remove_edge(u, v);

                    for (edge e : rev) {
                        if (e.tail == u && e.head == v) {
                            e = { u, t };
                        }
                    }
                }
            }
        }
    }
};