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
    unsigned original_size;

    // the final positions of vertices and control points of edges
    std::vector< node > nodes;
    std::vector< std::vector<vec2> > paths;

    detail::positioning_attributes attr { 10, 50 };

    // algorithms for individual steps of sugiyama framework
    std::unique_ptr< detail::cycle_removal > cycle_rem = std::make_unique< detail::dfs_removal >();
    std::unique_ptr< detail::layering > layering =       std::make_unique< detail::network_simplex_layering >(g);
    std::unique_ptr< detail::positioning > positioning = std::make_unique< detail::test_positioning >(attr, nodes);

public:
    sugiyama_layout(graph& g) : g(g), original_size(g.size()) {}

    void build() {
        subgraphs = detail::split(g);

        vec2 last = { 0, 0 };

        for (auto& g : subgraphs) {
            auto reversed_edges = cycle_rem->run(g);
            detail::hierarchy h = layering->run(g);
            auto long_edges = add_dummy_nodes(g, h);

            // check wheter any reversed edges have been split
            for (const auto& elem : long_edges) {
                auto i = std::find(reversed_edges.reversed.begin(), reversed_edges.reversed.end(), elem.orig);
                if (i != reversed_edges.reversed.end()) {
                    // save only the beginning of the path, the rest can be easily calculated,
                    // because it consists of dummy nodes and each dummy node has only one outgoing neighbour
                    i->head = elem.path[1];
                }
                auto j = std::find(reversed_edges.collapsed.begin(), reversed_edges.collapsed.end(), elem.orig);
                if (j != reversed_edges.collapsed.end()) {
                    j->head = elem.path[1];
                }
            }

            // now we know the final number of vertices so prepare nodes for positioning
            nodes.resize(this->g.size());
            last = positioning->run(g, h, { last.x, 0 });

            // restore the reversed edges
            for (auto e : reversed_edges.reversed) {
                reverse_path(e, g);
            }
            for (auto e : reversed_edges.collapsed) {
                reverse_path(e, g, false);
            }
        }

        // calculate the final control points for each edge
        construct_paths();

        // get rid of the dummy nodes
        nodes.resize(original_size);
    }

    void node_separation(float d) { attr.node_dist = d; }
    void layer_separation(float d) { attr.layer_dist = d; }

    /**
     * Returns the positions and sizes of all the vertices in the graph.
     * Calling this function before build was called is undefined.
     */
    const std::vector<node>& vertices() const { return nodes; }

    /**
     * Returns the control points for all the edges in the graph.
     * Calling this function before build was called is undefined.
     */
    const std::vector< std::vector<vec2> >& edges() const { return paths; }

private:

    void construct_paths() {
        for (auto& subg : subgraphs) {
            for (auto u : subg.vertices()) {
                for (auto v : subg.out_neighbours(u)) {
                    paths.emplace_back();
                    auto& path = paths.back();

                    path.push_back( endpoint( v, u ) );
                    vertex_t prev = u;
                    while (subg.is_dummy(v)) {
                        path.push_back( nodes[v].pos );
                        prev = v;
                        v = *g.out_neighbours(v).begin();
                    }
                    path.push_back( endpoint( prev, v ) );
                }
            }
        }
    }

    /**
     * Reverses the long edge which begins with edge e.
     */
    void reverse_path(edge e, detail::subgraph& g, bool remove = true) {
        vertex_t u = e.tail;
        vertex_t v = e.head;
        
        while (g.is_dummy(v)) {
            vertex_t next = *g.out_neighbours(v).begin();
            reverse_edge({u, v}, remove);
            u = v;
            v = next;
        }
        reverse_edge({u, v}, remove);
    }

    void reverse_edge(edge e, bool remove = true) {
        if (remove) {
            g.remove_edge(e.tail, e.head);
        }
        g.add_edge(e.head, e.tail);
    }

    /**
     * Calculate where the edge intresects the border of the head node.
     */
    vec2 endpoint(vertex_t u, vertex_t v) {
        vec2 dir = nodes[u].pos - nodes[v].pos;
        return nodes[v].pos + nodes[v].size * normalized(dir);
    }
};