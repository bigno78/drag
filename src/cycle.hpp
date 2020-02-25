#pragma once

#include <vector>

#include "subgraph.hpp"

/** 
 * Interface for a cycle removal algorithm.
 */
struct cycle_removal {
    virtual void run(subgraph& g) = 0;
    virtual ~cycle_removal() = default;
};

/**
 * Algorithm for removing cycles in a graph using a depth first search.
 */
class dfs_removal : public cycle_removal {
    /**
     * Marks for each verter of the graph:
     * 0  undiscovered vertex
     * -1 vertex which is being processed ("is on the stack")
     * 1 finished vertex
     */
    std::vector<int8_t> marks;

public:
    void run(subgraph& g) override {
        marks.resize(g.size(), 0);
        for (vertex_t u = 0; u < g.size(); ++u) {
            if (marks[u] == 0) {
                dfs(g, u);
            }
        }
        marks.clear();
    }

private:
    void dfs(subgraph& g, vertex_t u) {
        marks[u] = -1;
        for (auto v : g.out_edges(u)) {
            if (marks[v] == -1) { // there is a cycle
                g.remove_edge(u, v);
                g.add_edge(v, u);
            } else if (marks[v] == 0) {
                dfs(g, v);
            }
        }
        marks[u] = 1;
    }
};
