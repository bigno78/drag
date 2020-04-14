#pragma once

#include <vector>
#include <memory>

#include "interface.hpp"
#include "subgraph.hpp"

#include "cycle.hpp"
#include "layering.hpp"
#include "positioning.hpp"
#include "crossing.hpp"
#include "router.hpp"

#ifdef CONTROL_CROSSING
bool crossing_enabled = true;
#endif


class sugiyama_layout {
    graph& g;
    unsigned original_size;

    detail::vertex_map<detail::bounding_box> boxes;
    float loop_size = defaults::loop_size;

    // the final positions of vertices and control points of edges
    std::vector< node > nodes;
    std::vector< std::vector<vec2> > paths;
    std::vector< link > links;
    vec2 size = { 0, 0 };

    // attributes controling spacing
    detail::positioning_attributes attr { defaults::node_dist, defaults::layer_dist };

    // algorithms for individual steps of sugiyama framework
    std::unique_ptr< detail::cycle_removal > cycle_rem =     
                        std::make_unique< detail::dfs_removal >();
    std::unique_ptr< detail::layering > layering =
                        std::make_unique< detail::network_simplex_layering >(g);
    std::unique_ptr< detail::crossing_reduction > crossing = 
                        std::make_unique< detail::barycentric_heuristic >();
    std::unique_ptr< detail::positioning > positioning = 
                        std::make_unique< detail::fast_and_simple_positioning >(attr, nodes, boxes, g);
    std::unique_ptr< detail::edge_router > routing = 
                        std::make_unique< detail::router >(nodes, links);

public:
    sugiyama_layout(graph& g) : g(g), original_size(g.size()) {}

    void build() {
        std::vector< detail::subgraph > subgraphs = detail::split(g);
        init_nodes();

        vec2 start = { defaults::margin, defaults::margin };
        for (auto& g : subgraphs) {
            vec2 dim = process_subgraph(g, start);
            start.x += dim.x + attr.node_dist;
            size.x += dim.x;
            size.y = std::max(size.y, dim.y);
        }

        // get rid of the dummy nodes
        nodes.resize(original_size);
        //init_nodes();
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
    const std::vector<link>& edges() const { return links; }

    float width() const { return size.x; }
    float height() const { return size.y; }

private:

    vec2 process_subgraph(detail::subgraph& g, vec2 start) {
        //std::cout << g << "\n";
        auto reversed_edges = cycle_rem->run(g);

        /*for (auto e : reversed_edges.reversed) {
            std::cout << e << "\n";
        }
        for (auto e : reversed_edges.collapsed) {
            std::cout << e << "\n";
        }*/

        detail::hierarchy h = layering->run(g);
        //std::cout << h << "\n";

        auto long_edges = add_dummy_nodes(h);
        update_reversed_edges(reversed_edges, long_edges);
        
#ifdef CONTROL_CROSSING
        if (crossing_enabled) {
            crossing->run(h);    
        }
#else
        crossing->run(h);
#endif

        // resize the nodes to make space for new dummy nodes
        nodes.resize(this->g.size());
        boxes.resize(g);
        for (auto u : reversed_edges.loops) {
            boxes[u].size.x += loop_size;
        }
        vec2 dimensions = positioning->run(h, start);

        // restore the reversed edges
        for (auto e : reversed_edges.reversed) {
            reverse_path(e, g);
        }
        for (auto e : reversed_edges.collapsed) {
            reverse_path(e, g, false);
        }
        for (auto u : reversed_edges.loops) {
            g.add_edge(u, u);
        }

        //construct_paths(g, reversed_edges.loops);
        routing->run(h);

        return dimensions;
    }


    void init_nodes() {
        nodes.resize( g.size() );
        boxes.resize( g );
        for ( auto u : g.vertices() ) {
            nodes[u].u = u;
            nodes[u].default_label = std::to_string(u);
            nodes[u].size = g.node_size(u);
            boxes[u] = { { 2*nodes[u].size, 2*nodes[u].size },
                         { nodes[u].size, nodes[u].size } };
        }
    }

    /**
     * Checks if any of the reversed edges have been split into a path.
     * For each such edge (u, v) saves the first vertex on the path 
     * from 'u' to 'v' instead of 'v'.
     * When reversing the path back, the rest of the path can be easily determined by folowing the dummy nodes
     * until the first non-dummy node is reached.
     */
    void update_reversed_edges(detail::rev_edges& reversed_edges, const std::vector< detail::long_edge >& long_edges) {
        for (const auto& elem : long_edges) {
            auto i = std::find(reversed_edges.reversed.begin(), reversed_edges.reversed.end(), elem.orig);
            if (i != reversed_edges.reversed.end()) {
                i->to = elem.path[1];
            }
            auto j = std::find(reversed_edges.collapsed.begin(), reversed_edges.collapsed.end(), elem.orig);
            if (j != reversed_edges.collapsed.end()) {
                j->to = elem.path[1];
            }
        }
    }

    void construct_paths(const detail::subgraph& g, const std::vector<vertex_t>& loops) {
        for (auto u : g.vertices()) {
            for (auto v : g.out_neighbours(u)) {
                paths.emplace_back();
                auto& path = paths.back();

                path.push_back( endpoint( v, u ) );
                vertex_t prev = u;
                while (g.is_dummy(v)) {
                    path.push_back( nodes[v].pos );
                    prev = v;
                    v = *g.out_neighbours(v).begin(); // a dummy node can only have one outgoing neighbour
                }
                path.push_back( endpoint( prev, v ) );
            }
        }

        for (auto u : loops) {
            paths.emplace_back(5);
            auto& path = paths.back();

            path[1] = nodes[u].pos + vec2{ nodes[u].size + loop_size/2, nodes[u].size };
            path[2] = nodes[u].pos + vec2{ nodes[u].size + loop_size, 0 };
            path[3] = nodes[u].pos + vec2{ nodes[u].size + loop_size/2, -nodes[u].size };

            path[0] = endpoint( nodes[u].pos, nodes[u].size, path[1] - nodes[u].pos );
            path[4] = endpoint( nodes[u].pos, nodes[u].size, path[3] - nodes[u].pos );
        }
    }

    /**
     * Reverses the long edge which begins with edge e.
     */
    void reverse_path(detail::edge e, detail::subgraph& g, bool remove = true) {
        vertex_t u = e.from;
        vertex_t v = e.to;
        
        while (g.is_dummy(v)) {
            vertex_t next = *g.out_neighbours(v).begin();
            reverse_edge({u, v}, remove);
            u = v;
            v = next;
        }
        reverse_edge({u, v}, remove);
    }

    void reverse_edge(detail::edge e, bool remove = true) {
        if (remove) {
            g.remove_edge(e.from, e.to);
        }
        g.add_edge(e.to, e.from);
    }

    /**
     * Calculate where the edge (u, v) intersects the border of the 'v' node.
     */
    vec2 endpoint(vertex_t u, vertex_t v) {
        vec2 dir = nodes[u].pos - nodes[v].pos;
        return nodes[v].pos + nodes[v].size * normalized(dir);
    }

    vec2 endpoint(vec2 center, float r, vec2 dir) {
        return center + r * normalized(dir);
    }
};

