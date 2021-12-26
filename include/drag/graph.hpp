#pragma once

#include <vector>
#include <iostream>
#include <ostream>
#include <algorithm>
#include <set>
#include <tuple>
#include <map>

#include <drag/types.hpp>
#include <drag/detail/utils.hpp>


namespace drag {

/**
 * Basic clas for representing a directed graph.
 */
class graph {
public:

    float node_size = 25;         /**< radius of all nodes */
    float node_dist = 20;         /**< minimum distance between borders of 2 nodes */
    float layer_dist = 40;        /**< minimum distance between borders of nodes in 2 different layers */
    float loop_angle = 55;        /**< angle determining the point on the node where a loop connects to it */
    float loop_size = node_size;  /**< distance which the loop extends from the node*/

    /**
     * Add a new vertex to the graph.
     * 
     * @return the identifier of the vertex 
     */
    vertex_t add_node() {
        m_out_neighbours.emplace_back();
        m_in_neighbours.emplace_back();
        return m_out_neighbours.size() - 1;
    }

    /**
     * Add a new edge to the graph.
     * 
     * The behavious is undefined if the same edge is added twice 
     * or if an identifier other then one returned by add_node() is used.
     * 
     * @param from the identifier of the starting vertex
     * @param to   the identifier of the ending vertex
     * 
     * @return a reference to the graph for chaining multiple calls
     */
    graph& add_edge(vertex_t from, vertex_t to) { 
        m_out_neighbours[from].push_back(to);
        m_in_neighbours[to].push_back(from);
        return *this;
    }

    /**
     * Get the number of vertices in the graph.
     * 
     * @return the number of vertices
     */
    unsigned size() const { return m_out_neighbours.size(); }

    /**
     * Get an immutable list of all successors.
     */
    const std::vector<vertex_t>& out_neighbours(vertex_t u) const { return m_out_neighbours[u]; }
    /**
     * Get an immutable list of all predecessors.
     */
    const std::vector<vertex_t>& in_neighbours(vertex_t u) const { return m_in_neighbours[u]; }
   
    /**
     * Get an implementation defined object which can be used for iterating through the vertices.
     * 
     * The returned object can be used in a range for loop.
     * It provides begin() and end() methods which yield forward iterators for iterating through the vertices.
     */
    range<vertex_t> vertices() const { return range<vertex_t>(0, size(), 1); }

    /**
     * Remove the given edge.
     * Slow operation - should be avoided if possible.
     */
    void remove_edge(vertex_t from, vertex_t to) {
        remove_neighour(m_out_neighbours[from], to);
        remove_neighour(m_in_neighbours[to], from);
    }

    friend std::ostream& operator<<(std::ostream& out, const graph& g) {
        for (auto u : g.vertices()) {
            out << u << ": [";

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
    std::vector< std::vector<vertex_t> > m_out_neighbours;
    std::vector< std::vector<vertex_t> > m_in_neighbours;

    void remove_neighour(std::vector<vertex_t>& neighbours, vertex_t u) {
        auto it = std::find(neighbours.begin(), neighbours.end(), u);
        if (it != neighbours.end()) {
            neighbours.erase(it);
        }
    }

};

/**
 * Builder class for creating graphs by hand.
 * 
 * The graph can be created by gradually adding edges which are
 * specified by its endpoints - two non-negative numbers representing
 * the vertices. Vertices don't need to be added, they are added 
 * implicitely when creating edges.
 * 
 * If the vertices are not in the range `0 ... n-1`, where `n`
 * is the number of vertices, they will be mapped to this range.
 */
struct graph_builder {
    std::set<vertex_t> nodes;
    std::set<std::pair<vertex_t, vertex_t>> edges;

    /**
     * Add a new edge.
     */
    graph_builder& add_edge(vertex_t u, vertex_t v) {
        nodes.insert(u);
        nodes.insert(v);
        edges.emplace(u, v);
        return *this;
    }

    /**
     * Get the resulting graph.
     */
    graph build() { 
        graph g;

        std::map<vertex_t, vertex_t> to_id;

        for (auto u : nodes) {
            to_id[u] = g.add_node();
        }

        for (auto [ u, v ] : edges) {
            g.add_edge( to_id[u], to_id[v] );
        }

        return g; 
    }
};

} // namespace drag
