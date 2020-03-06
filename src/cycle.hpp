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
    enum class state : char { done, in_progress, unvisited };
    
public:
    std::vector<edge> run(subgraph& g) override {
        vertex_flags<state> marks(g, state::unvisited);
        std::vector<edge> reversed_edges;
        for (auto u : g.vertices()) {
            if (marks[u] == state::unvisited) {
                dfs(g, marks, u, reversed_edges);
            }
        }

        for (auto& e : reversed_edges) {
            g.remove_edge(e);
            e = reversed(e);
            g.add_edge(e);
        }

        return reversed_edges;
    }

private:
    void dfs(subgraph& g, vertex_flags<state>& marks, vertex_t u, std::vector<edge>& reversed_edges) {
        marks[u] = state::in_progress;
        for (auto v : g.out_neighbours(u)) {
            if (marks[v] == state::in_progress) { // there is a cycle
                reversed_edges.push_back({ u, v });
            } else if (marks[v] == state::unvisited) {
                dfs(g, marks, v, reversed_edges);
            }
        }
        marks[u] = state::done;
    }
};

} //namespace detail
