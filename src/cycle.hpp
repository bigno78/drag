#pragma once

#include <vector>

#include "subgraph.hpp"

namespace detail {

/**
 * Holds the edges that were reversed to remove cycles.
 * This struct contains the edges in their reversed form
 * i.e. if edge (u, v) cuased a cycle, it would be saved as (v, u)
 */
struct rev_edges {
    std::vector<edge> reversed;   /**< The edges that were reversed. */
    std::vector<edge> collapsed;  /**< Reversed edges which resulted in a duplicated edge and thus were collapsed */
    std::vector<vertex_t> loops;
};


/** 
 * Interface for a cycle removal algorithm.
 */
struct cycle_removal {

    /**
     * Modifies the input graph by reversing certain edges to remove cycles.
     * 
     * @return the reversed edges
     */
    virtual rev_edges run(subgraph& g) = 0;

    virtual ~cycle_removal() = default;
};


/**
 * Algorithm for removing cycles in a graph using a depth first search.
 */
class dfs_removal : public cycle_removal {
    enum class state : char { done, in_progress, unvisited };
    
public:
    rev_edges run(subgraph& g) override {
        vertex_map<state> marks(g, state::unvisited);
        rev_edges reversed_edges;
        for (auto u : g.vertices()) {
            if (marks[u] == state::unvisited) {
                dfs(g, marks, u, reversed_edges);
            }
        }

        for (auto& e : reversed_edges.reversed) {
            g.remove_edge(e);
            e = reversed(e);
            g.add_edge(e);
        }

        for (auto& e : reversed_edges.collapsed) {
            g.remove_edge(e);
            e = reversed(e);
        }

        for (auto u : reversed_edges.loops) {
            g.remove_edge(u, u);
        }

        return reversed_edges;
    }

private:
    void dfs(subgraph& g, vertex_map<state>& marks, vertex_t u, rev_edges& reversed_edges) {
        marks[u] = state::in_progress;
        for (auto v : g.out_neighbours(u)) {
            if (u == v) {
                std::cout << "leep\n";
                reversed_edges.loops.push_back(u);
            } else if (marks[v] == state::in_progress) { // there is a cycle
                // Yes, I know. It should be saved as (v, u). But at this point I am just saving edges
                // to be reversed later. When that happens the order will be switched.
                if (g.has_edge(v, u))
                    reversed_edges.collapsed.push_back( { u, v } );
                else
                    reversed_edges.reversed.push_back( { u, v } );
            } else if (marks[v] == state::unvisited) {
                dfs(g, marks, v, reversed_edges);
            }
        }
        marks[u] = state::done;
    }
};

} //namespace detail
