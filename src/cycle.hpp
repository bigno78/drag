#pragma once

#include <vector>

#include "subgraph.hpp"

namespace detail {

/** 
 * Interface for a cycle removal algorithm.
 */
struct cycle_removal {

    /**
     * Modifies the input graph by reversing certain edges to remove cycles.
     * 
     * @return the reversed edges in their new direction
     */
    virtual std::vector<edge> run(subgraph& g) = 0;

    virtual ~cycle_removal() = default;
};

/**
 * Algorithm for removing cycles in a graph using a depth first search.
 */
class dfs_removal : public cycle_removal {
    
public:
    std::vector<edge> run(subgraph& g) override {
        // Marks for each vertex of the graph:
        //   0  undiscovered vertex
        //  -1 vertex which is being processed ("is on the stack")
        //   1 finished vertex
        vertex_flags<int8_t> marks(g, 0);
        std::vector<edge> reversed_edges;
        for (auto u : g.vertices()) {
            if (marks[u] == 0) {
                dfs(g, marks, u, reversed_edges);
            }
        }
        return reversed_edges;
    }

private:
    void dfs(subgraph& g, vertex_flags<int8_t>& marks, vertex_t u, std::vector<edge>& reversed_edges) {
        marks[u] = -1;
        for (auto v : g.out_neighbours(u)) {
            if (marks[v] == -1) { // there is a cycle
                reversed_edges.push_back({ v, u });
                g.remove_edge(u, v);
                g.add_edge(v, u);
            } else if (marks[v] == 0) {
                dfs(g, marks, v, reversed_edges);
            }
        }
        marks[u] = 1;
    }
};

} //namespace detail
