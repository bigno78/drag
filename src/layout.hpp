#ifndef LAYOUT_HPP
#define LAYOUT_HPP

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
    graph g;
    unsigned original_vertex_count;

    detail::vertex_map<detail::bounding_box> boxes;
    float loop_size = defaults::loop_size;

    // the final positions of vertices and control points of edges
    std::vector< node > nodes;
    std::vector< path > paths;
    vec2 size = { 0, 0 };

    // attributes controling spacing
    attributes attrs;

    // algorithms for individual steps of sugiyama framework
    std::unique_ptr< detail::cycle_removal > cycle_module =     
                        std::make_unique< detail::dfs_removal >();
    
    std::unique_ptr< detail::layering > layering_module =
                        std::make_unique< detail::network_simplex_layering >();
    
    std::unique_ptr< detail::crossing_reduction > crossing_module = 
                        std::make_unique< detail::barycentric_heuristic >();
    
    std::unique_ptr< detail::positioning > positioning_module = 
                        std::make_unique< detail::fast_and_simple_positioning >(attrs, nodes, boxes, g);
    
    std::unique_ptr< detail::edge_router > routing_module = 
                        std::make_unique< detail::router >(nodes, paths, attrs);

public:
    sugiyama_layout(graph g) : g(g), original_vertex_count(g.size()) {
        build();
    }

    sugiyama_layout(graph g, attributes attr) 
        : g(g)
        , original_vertex_count(g.size())
        , attrs(attr) 
    {
        build();
    }

    void build() {
        std::vector< detail::subgraph > subgraphs = detail::split(g);
        init_nodes();

        vec2 start = { defaults::margin, defaults::margin };
        for (auto& g : subgraphs) {
            vec2 dim = process_subgraph(g, start);
            start.x += dim.x + attrs.node_dist;
            size.x += dim.x;
            size.y = std::max(size.y, dim.y);
        }

        // get rid of the dummy nodes
        nodes.resize(original_vertex_count);
        size += vec2{ defaults::margin, defaults::margin };
        //init_nodes();
    }

    const attributes& get_attributes() const { return attrs; }

    /**
     * Returns the positions and sizes of all the vertices in the graph.
     * Calling this function before build was called is undefined.
     */
    const std::vector<node>& vertices() const { return nodes; }

    /**
     * Returns the control points for all the edges in the graph.
     * Calling this function before build was called is undefined.
     */
    const std::vector<path>& edges() const { return paths; }

    float width() const { return size.x; }
    float height() const { return size.y; }
    vec2 dimensions() const { return size; } 

private:

    vec2 process_subgraph(detail::subgraph& g, vec2 start) {
        //std::cout << g << "\n";
        auto reversed_edges = cycle_module->run(g);

        /*for (auto e : reversed_edges.reversed) {
            std::cout << e << "\n";
        }
        for (auto e : reversed_edges.collapsed) {
            std::cout << e << "\n";
        }*/

        detail::hierarchy h = layering_module->run(g);
        //std::cout << h << "\n";

        auto long_edges = add_dummy_nodes(h);
        update_reversed_edges(reversed_edges, long_edges);
        
#ifdef CONTROL_CROSSING
        if (crossing_enabled) {
            crossing->run(h);    
        }
#else
        crossing_module->run(h);
#endif

        // resize the nodes to make space for new dummy nodes
        nodes.resize(this->g.size());
        boxes.resize(g);
        for (auto u : reversed_edges.loops) {
            boxes[u].size.x += loop_size;
        }
        vec2 dimensions = positioning_module->run(h, start);

        routing_module->run(h, reversed_edges);

        return dimensions;
    }


    void init_nodes() {
        nodes.resize( g.size() );
        boxes.resize( g );
        for ( auto u : g.vertices() ) {
            nodes[u].u = u;
            nodes[u].default_label = std::to_string(u);
            nodes[u].size = attrs.node_size;
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
            if (reversed_edges.reversed.remove(elem.orig)) {
                reversed_edges.reversed.insert(elem.path[0], elem.path[1]);
            } else if (reversed_edges.collapsed.remove(elem.orig)) {
                reversed_edges.collapsed.insert(elem.path[0], elem.path[1]);
            }
        }
    }
};


#endif
