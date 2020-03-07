#pragma once

#include <vector>
#include <cmath>
#include <limits>
#include <utility>

#include "subgraph.hpp"

namespace detail {

struct hierarchy {
    detail::vertex_flags<int> ranking;
    std::vector< std::vector<vertex_t> > layers;

    int span(vertex_t u, vertex_t v) const {
        return ranking[v] - ranking[u];
    }
};

struct long_edge {
    edge orig;
    std::vector<vertex_t> path;

    long_edge(edge orig, std::vector<vertex_t> path) : orig(orig), path(std::move(path)) {}
};

/**
 * Splits all edges that span across more than one layer in the provided hierarchy
 * into segments that each span across just one layer.
 * ( aka converts the hierachy into proper hierarchy )
 */
std::vector< long_edge > add_dummy_nodes(detail::subgraph& g, detail::hierarchy& h) {
    std::vector< long_edge > split_edges;

    for (auto u : g.vertices()) {
        for (auto v : g.out_neighbours(u)) {
            int span = h.span(u, v);
            if (span > 1) {
                split_edges.emplace_back( edge{u, v}, std::vector<vertex_t>{} );
            }
        }
    }

    for (auto& [ orig, path ] : split_edges) {
        int span = h.span(orig.tail, orig.head);
        path.push_back(orig.tail);

        vertex_t s = orig.tail;
        for (int i = 0; i < span - 1; ++i) {
            vertex_t t = g.add_dummy();

            h.ranking.add_vertex(t);
            h.ranking[t] = h.ranking[s] + 1;
            h.layers[ h.ranking[t] ].push_back(t);

            g.add_edge(s, t);
            path.push_back(t);

            s = t;
        }
        path.push_back(orig.head);
        g.add_edge(s, orig.head);
        g.remove_edge(orig);
    }

    return split_edges;
}


struct layering {
    virtual hierarchy run(const detail::subgraph&) = 0;
    virtual ~layering() = default;
};


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


    tight_tree() = default;
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


    friend std::ostream& operator<<(std::ostream& out, tight_tree tree) {
        for (auto n : tree.nodes) {
            out << n.u << "(" << n.parent << "): {";
            const char* sep = "";
            for (auto u : n.children) {
                out << sep << u;
                sep = ",";
            }
            out << "}\n";
        }
        return out;
    }
};


class network_simplex_layering : public layering {
    detail::vertex_flags<int> ranking;
    tight_tree tree;

public:
    network_simplex_layering(const graph& g) : ranking(g, -1) {}

    hierarchy run(const detail::subgraph& g) override {
        initialize_ranking(g);

        int min = std::numeric_limits<int>::max();
        int max = std::numeric_limits<int>::min();
        for (auto u : g.vertices()) {
            if (ranking[u] > max)
                max = ranking[u];
            if (ranking[u] < min)
                min = ranking[u];
        }

        std::vector< std::vector<vertex_t> > layers(max - min + 1);

        for (auto u : g.vertices()) {
            ranking[u] -= min;
            layers[ ranking[u] ].push_back(u);
        }

        return { ranking, layers };
    }

private:
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
    void initialize_ranking(const detail::subgraph& g) {
        int curr = g.size();
        unsigned processed = 0;

        std::vector<vertex_t> to_rank;
        while (processed < g.size()) {
            to_rank.clear();
            for (vertex_t u = 0; u < g.size(); ++u) {
                if (ranking[u] == -1) {
                    // check if there are any edges going to unranked vertices
                    for (auto v : g.out_neighbours(u)) {
                        if (ranking[v] == -1) {
                            goto fail;
                        }
                    }
                    to_rank.push_back(u);
                    ++processed;
                }
    fail: ;
            }
            for (auto u : to_rank) {
                ranking[u] = curr;
            }
            --curr;
        }
    }

    /**
     * Return the span of an edge.
     * The span is the distance between layers it connects.
     * 
     * @param ranking ranking to use
     * @param e       edge whose span is to be calculated
     * @return        span of the edge
     */
    int edge_span(edge e) {
        return ranking[e.head] - ranking[e.tail];
    }

    /**
     * Constructs a tree (it wont necessarly be the final spanning tree) of all vertices
     * reachable from the root through tight edges.
     * 
     */
    int basic_tree(const detail::subgraph& g, std::vector<bool>& done, vertex_t root) {
        done[root] = true;
        int added = 1;
        for (auto u : g.neighbours(root)) {
            if ( !done[u] && std::abs(edge_span({ root, u })) == 1 ) {
                tree.add_child(root, u);
                added += basic_tree(g, done, u);
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
    void initialize_tree(const detail::subgraph& g) {
        tree =  tight_tree(g.size());
        tree.root = 0;
        std::vector<bool> done(g.size(), false);

        int finished = basic_tree(g, done, tree.root);

        while(finished < g.size()) {
            // in the underlying undirected graph find the shortest edge (u, v) 
            // with u already in the tree and v not in thte tree
            edge e = { 0, 0 };
            int span = g.size();
            for (vertex_t u = 0; u < done.size(); ++u) {
                if (done[u]) {
                    for (auto v : g.neighbours(u)) {
                        int sp = edge_span({ u, v });
                        if (!done[v] && std::abs(sp) < std::abs(span)) {
                            e = { u, v };
                            span = sp;
                        }
                    }
                }
            }

            // make the edge tight by moving all the vertices in the tree
            span -= sgn(span);
            for (vertex_t u = 0; u < g.size(); ++u) {
                if (done[u]) {
                    ranking[u] += span;
                }
            }
            tree.add_child(e.tail, e.head);
            done[e.head] = true;
            ++finished;
        }
    }
    
};


} // namespace detail
