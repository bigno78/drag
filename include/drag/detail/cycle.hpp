#pragma once

#include <vector>
#include <limits>

#include <drag/detail/subgraph.hpp>

namespace drag {

namespace detail {

/**
 * Holds the edges that were reversed to remove cycles.
 * This struct contains the edges in their reversed form
 * i.e. if edge (u, v) cuased a cycle, it is saved as (v, u)
 */
struct rev_edges {
    edge_set reversed;   /**< The edges that were reversed. */
    edge_set collapsed;  /**< Reversed edges which resulted in a duplicated edge and thus were collapsed */
    std::vector<vertex_t> loops;  /**< Loops */
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

        // find cycles
        for (auto u : g.vertices()) {
            if (marks[u] == state::unvisited) {
                dfs(g, marks, std::numeric_limits<vertex_t>::max(), u, reversed_edges);
            }
        }

        for (auto u : g.vertices()) {
            if (reversed_edges.reversed.data.contains(u)){
                for (const auto& v : reversed_edges.reversed.data[u]) {
                    g.remove_edge(v, u);
                    g.add_edge(u, v);
                }
            }
            if (reversed_edges.collapsed.data.contains(u)) {
                for (const auto& v : reversed_edges.collapsed.data[u]) {
                    g.remove_edge(v, u);
                }
            }
        }

        for (auto u : reversed_edges.loops) {
            g.remove_edge(u, u);
        }

        return reversed_edges;
    }

private:
    void dfs(subgraph& g, vertex_map<state>& marks, vertex_t parent, vertex_t u, rev_edges& reversed_edges) {
        marks[u] = state::in_progress;
        
        for (auto v : g.out_neighbours(u)) {
            if (u == v) { // a loop
                reversed_edges.loops.push_back(u);
            } else if (marks[v] == state::in_progress) { // there is a cycle
                if (g.has_edge(v, u)) { // two-cycle
                    //std::cout << edge{u,v} << "\n";
                    reversed_edges.collapsed.insert({ v, u });
                } else { // regular cycle
                    //std::cout << edge{u,v} << " reg\n";
                    reversed_edges.reversed.insert({ v, u });
                }
            } else if (marks[v] == state::unvisited) {
                dfs(g, marks, u, v, reversed_edges);
            }
        }

        marks[u] = state::done;
    }
};

} // namespace detail

} // namespace drag
