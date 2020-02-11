#pragma once

#include <vector>
#include <cmath>

#include "graph.hpp"


struct tree_node {
    int parent = -1;
    vertex_t u;
    int cut_value = 0;
    int out_cut_value = 0;
    std::vector<vertex_t> children;
};

struct tight_tree {
    std::vector<tree_node> nodes;
    vertex_t root;

    tight_tree(unsigned size) : nodes(size) {
        vertex_t u = 0;
        for (auto& n : nodes)
            n.u = u++;
    }

    std::vector<vertex_t>& children(vertex_t u) { return nodes[u].children; }
    const std::vector<vertex_t>& children(vertex_t u) const { return nodes[u].children; }

    tree_node& node(vertex_t u) {
        return nodes[u];
    }
    
    void add_child(vertex_t parent, vertex_t child) {
        nodes[parent].children.push_back(child);
        nodes[child].parent = parent;
    }

    void remove_child(vertex_t parent, vertex_t child) {

    }

    friend std::ostream& operator<<(std::ostream& out, tight_tree tree) {
        for (auto n : tree.nodes) {
            out << n.u << "(" << n.parent << "): {";
            const char* sep = "";
            for (auto u : n.children) {
                out << sep << u;
                sep = ",";
            }
            std::cout << "}\n";
        }
        return out;
    }
};


/**
 * Assignes each vertex a layer, such that each edge goes from a lower layer to higher one
 * and the source vertices are at the lowest layer.
 * The resulting ranking is not normalized, meaning that the lowest layer doesnt have to be 0.
 * It is also not neccesarly the optimal ranking.
 * 
 * Terminology:
 * Layering is a set of layers, each containing a set of vertices.
 * Ranking is a set of vertices, each with assigned layer/rank.
 * The terms layer and rank are interchangable.
 * 
 * @param g the graph whose vertices are to be assigned to layers
 * @return resulting ranking
 */
std::vector<int> initialize_ranking(const graph& g) {
    std::vector<int> ranking(g.size(), -1);
    int curr = g.size();
    unsigned processed = 0;

    while (processed < g.size()) {
        for (vertex_t u = 0; u < g.size(); ++u) {
            if (ranking[u] == -1) {
                // check if there are any edges going to unranked vertices
                for (auto v : g.out_edges(u)) {
                    if (ranking[v] == -1)
                        goto fail;
                } 
                ranking[u] = curr;
                ++processed;
            }
fail: ;
        }
        --curr;
    }

    return ranking;
}

/**
 * Return the span of an edge.
 * The span is the distance between layers it connects.
 * 
 * @param ranking ranking to use
 * @param e       edge whose span is to be calculated
 * @return        span of the edge
 */
int span(std::vector<int> ranking, edge e) {
    return ranking[e.head] - ranking[e.tail];
}


/**
 * Constructs a tree (it wont necessarly be the final spanning tree) of all vertices
 * reachable from the root through tight edges.
 * 
 */
int basic_tree(const graph& g, const std::vector<int>& ranking, std::vector<bool>& done, tight_tree& tree, vertex_t root) {
    done[root] = true;
    int added = 1;
    for (auto u : g.edges(root)) {
        if ( !done[u] && std::abs(span(ranking, { root, u })) == 1 ) {
            tree.add_child(root, u);
            added += basic_tree(g, ranking, done, tree, u);
        }
    }
    return added;
}

/**
 * Finds a spanning tree of tight edges. 
 * Tight edge is any edge (u, v) for which ranking[v] - ranking[u] == 1.
 * 
 * @param g the graph
 * @param ranking ranking of the vertices of the graph
 * @return the spanning tree
 */
tight_tree initialize_tree(const graph& g, std::vector<int> ranking) {
    tight_tree tree(g.size());
    tree.root = 0;
    std::vector<bool> done(g.size(), false);
    int finished = basic_tree(g, ranking, done, tree, tree.root);
    return tree;
}


