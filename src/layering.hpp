#pragma once

#include <vector>
#include <cmath>
#include <limits>
#include <utility>

#include "subgraph.hpp"

namespace detail {


/**
 * Represents the partitioning of a graph into a set of layers.
 * Each vertex is part of a layer - the layer is its rank.
 */
struct hierarchy {
    detail::vertex_flags<int> ranking;
    std::vector< std::vector<vertex_t> > layers;
    subgraph& g;

    hierarchy(subgraph& g) : hierarchy(g, -1) {}
    hierarchy(subgraph& g, int val) : ranking(g, val), g(g) {}

    /**
     * Calculates the span of an edge - the number of layers the edge crosses.
     * The span can be negative if the edge goes from higher layer to a lower one.
     */
    int span(vertex_t u, vertex_t v) const { return ranking[v] - ranking[u]; }

    unsigned size() const { return layers.size(); }

    const std::vector<vertex_t>& layer(vertex_t u) const { return layers[ranking[u]]; }
};

std::ostream& operator<<(std::ostream& out, const hierarchy& h) {
    out << "ranking: [";
    for (auto u : h.g.vertices()) {
        out << h.ranking[u] << ", ";    
    }
    out << "]\n";
    for (const auto& l : h.layers) {
        for (auto u : l) {
            out << u << " ";
        }
        out << "\n";
    }
    return out;
}


/**
 * Edge which crosses several layers and had to by split into multiple "subedges".
 */
struct long_edge {
    edge orig;
    std::vector<vertex_t> path;

    /* so emplace_back can be used */
    long_edge(edge orig, std::vector<vertex_t> path) : orig(orig), path(std::move(path)) {}
};

/**
 * Splits all edges that span across more than one layer in the provided hierarchy
 * into segments that each span across just one layer.
 * ( aka converts the hierachy into proper hierarchy )
 * 
 * @return list of edges which had to be split
 */
std::vector< long_edge > add_dummy_nodes(detail::subgraph& g, detail::hierarchy& h) {
    std::vector< long_edge > split_edges;

    // find edges to be split
    for (auto u : g.vertices()) {
        for (auto v : g.out_neighbours(u)) {
            int span = h.span(u, v);
            if (span > 1) {
                split_edges.emplace_back( edge{u, v}, std::vector<vertex_t>{} );
            }
        }
    }

    // split them
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


/**
 * Interface for an algorithm which createch a proper hierarchy for a given graph.
 * 
 * Proper hierarchy is a hierarchy where each edge has a positive span.
 */
struct layering {
    virtual hierarchy run(detail::subgraph&) = 0;
    virtual ~layering() = default;
};



struct tree_edge {
    vertex_t u, v;
    int dir;
};


struct tree_node {
    int parent = -1;
    vertex_t u;
    int dir;
    int cut_value = 0;
    int out_cut_value = 0;
    std::vector<vertex_t> children;
    int min; 
    int order;
};


struct tight_tree {
    vertex_flags<tree_node> nodes;
    vertex_t root;

    tight_tree(const graph& g) : nodes(g) {
        vertex_t u = 0;
        for (auto u : g.vertices()) {
            nodes[u].u = u;
        }
    }

    std::vector<vertex_t>& children(vertex_t u) { return nodes[u].children; }
    const std::vector<vertex_t>& children(vertex_t u) const { return nodes[u].children; }

    tree_node& node(vertex_t u) { return nodes[u]; }
    
    void add_child(vertex_t parent, vertex_t child) {
        nodes[parent].children.push_back(child);
        nodes[child].parent = parent;
    }

    void remove_child(vertex_t parent, vertex_t child) {
        nodes[parent].children.erase(std::find(nodes[parent].children.begin(), nodes[parent].children.end(), child));
        nodes[child].parent = -1;
    }

    vertex_t component(edge e, vertex_t u) {
        if (nodes[e.head].min > nodes[u].min || nodes[e.head].order < nodes[u].order) {
            return e.tail;
        }
        return e.head;
    }

    int dir(const hierarchy& h, vertex_t u, vertex_t v) {
        return sgn(h.span(u, v));
    }

    vertex_t common_acestor(vertex_t u, vertex_t v) {
        vertex_t ancestor = u;
        int left_min;
        int right_order;
        if (nodes[u].order < nodes[v].order) {
            left_min = nodes[u].min;
            right_order = nodes[v].order;
        } else {
            left_min = nodes[v].min;
            right_order = nodes[u].order;
        }

        // the common acestor is the first node such that min <= left_min && order >= right_order
        while (nodes[ancestor].min > left_min || nodes[ancestor].order < right_order) {
            ancestor = nodes[ancestor].parent;
        }

        return ancestor;
    }

    friend std::ostream& operator<<(std::ostream& out, tight_tree tree) {
        for (auto n : tree.nodes.flags) {
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
    tight_tree tree;

public:
    network_simplex_layering(const graph& g) : tree(g) {}

    hierarchy run(detail::subgraph& g) override {
        hierarchy h = initialize_ranking(g);
        initialize_tree(g, h);
        postorder_search();
        init_cut_values(g, h);

        optimize_edge_length(g, h);
        //switch_tree_edges(g, h, { 4, 7, -1 }, { 0, 5, 1 });

        for (auto u : g.vertices()) {
            /*labels[u] = std::to_string(u) + " [" +  
                        std::to_string(tree.node(u).parent) + ":" + 
                        std::to_string(tree.node(u).order) + "," + 
                        std::to_string(tree.node(u).min) + "]";*/
            labels[u] = std::to_string(u);
        }

        int min = std::numeric_limits<int>::max();
        int max = std::numeric_limits<int>::min();
        for (auto u : g.vertices()) {
            if (h.ranking[u] > max)
                max = h.ranking[u];
            if (h.ranking[u] < min)
                min = h.ranking[u];
        }

        h.layers.resize(max - min + 1);

        for (auto u : g.vertices()) {
            h.ranking[u] -= min;
            h.layers[ h.ranking[u] ].push_back(u);
        }

        return h;
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
    hierarchy initialize_ranking(detail::subgraph& g) {
        hierarchy h(g, -1);
        int curr = g.size();
        unsigned processed = 0;

        std::vector<vertex_t> to_rank;
        while (processed < g.size()) {
            to_rank.clear();
            for (auto u : g.vertices()) {
                if (h.ranking[u] == -1) {
                    // check if there are any edges going to unranked vertices
                    for (auto v : g.out_neighbours(u)) {
                        if (h.ranking[v] == -1) {
                            goto fail;
                        }
                    }
                    to_rank.push_back(u);
                    ++processed;
                }
fail: ;
            }
            for (auto u : to_rank) {
                h.ranking[u] = curr;
            }
            --curr;
        }
        return h;
    }

    /**
     * Constructs a tree (it wont necessarly be the final spanning tree) of all vertices
     * reachable from the root through tight edges. 
     */
    int basic_tree(const detail::subgraph& g, vertex_flags<bool>& done, const hierarchy& h, vertex_t root) {
        done.set(root, true);
        int added = 1;

        for (auto u : g.neighbours(root)) {
            int span = h.span(root, u);
            if ( !done.at(u) && std::abs(span) == 1 ) {
                tree.add_child(root, u);
                tree.node(u).dir = sgn(span);
                added += basic_tree(g, done, h, u);
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
    void initialize_tree(const detail::subgraph& g, hierarchy& h) {
        tree.root = *g.vertices().begin();
        vertex_flags<bool> done(g, false);

        int finished = basic_tree(g, done, h, tree.root);

        while(finished < g.size()) {
            // in the underlying undirected graph find the shortest edge (u, v) 
            // with u already in the tree and v not in thte tree
            edge e = { 0, 0 };
            int min_span = std::numeric_limits<int>::max();
            for (auto u : g.vertices()) {
                if (done.at(u)) {
                    for (auto v : g.neighbours(u)) {
                        int span = h.span(u, v);
                        if (!done.at(v) && std::abs(span) < std::abs(min_span)) {
                            e = { u, v };
                            min_span = span;
                        }
                    }
                }
            }

            // make the edge tight by moving all the vertices in the tree
            int dir = sgn(min_span);
            min_span -= dir;
            for (auto u : g.vertices()) {
                if (done.at(u)) {
                    h.ranking[u] += min_span;
                }
            }
            tree.add_child(e.tail, e.head);
            tree.node(e.head).dir = dir;
            done.set(e.head, true);
            ++finished;
        }
    }

    /**
     * Performs a postorder search of the tight tree.
     * Fills the 'order' and 'min' members of each node in the tree.
     */
    void postorder_search() { postorder_search(tree.root, 0); }

    /**
     * Performs a postorder search of the subtree rooted at 'u'.
     * The first leaf node is given the order 'order'.
     */
    int postorder_search(vertex_t u, int order) {
        tree.node(u).min = order;
        for (auto child : tree.children(u)) {
            order = postorder_search(child, order);
        }
        tree.node(u).order = order;
        return order + 1;
    }

    /**
     * Calculates the initial cut values of all edges in the tight tree.
     */
    void init_cut_values(const subgraph& g, hierarchy& h) {
        for (auto child : tree.children(tree.root)) {
            init_cut_values(g, h, tree.root, child);
        }
    }

    /**
     * Calculates the initial cut value of edge ('u', 'v') and all the edges in the subtree rooted at 'v'.
     */
    void init_cut_values(const subgraph& g, hierarchy& h, vertex_t u, vertex_t v) {
        for (auto child : tree.children(v)) {
            init_cut_values(g, h, v, child);
        }
        set_cut_value(g, h, u, v);
    }

    /**
     * Calculates cut value of the edge ('u', 'v'). 
     * Requires the cut values of the edges between 'v' and its children to be already calculated.
     */
    void set_cut_value(const subgraph& g, hierarchy& h, vertex_t u, vertex_t v) {
        int val = 0;

        for (auto child : tree.children(v)) {
            val += tree.node(v).dir * tree.node(child).dir * tree.node(child).out_cut_value;
        }
        for (auto x : g.neighbours(v)) {
            int dir = sgn(h.span(x, v));
            if (tree.component( { u, v }, x ) == u) {
                val += dir * tree.node(v).dir;
            }
        }
        tree.node(v).cut_value = val;

        for (auto x : g.neighbours(u)) {
            int dir = sgn(h.span(u, x));
            if (tree.component( { u, v }, x ) == v) {
                val -= dir * tree.node(v).dir;
            }
        }
        tree.node(v).out_cut_value = val;  
    }

    /**
     * Reverses all the edges on the path from 'end' to 'v'.
     * @param end   the first node on the path
     * @param u     the parent of v
     * @param v     the last node on the path
     */
    void reverse_parents(vertex_t end, vertex_t u, vertex_t v) {
        vertex_t parent = tree.node(u).parent;
        if (u != end) {
            reverse_parents(end, parent, u);
        }
        tree.remove_child(u ,v);
        tree.add_child(v, u);
        tree.node(u).dir = -tree.node(v).dir;
    }

    void switch_tree_edges(const subgraph& g, hierarchy& h, tree_edge orig, tree_edge entering) {
        auto ancestor = tree.common_acestor(entering.u, entering.v);

        tree.remove_child(orig.u, orig.v);
        reverse_parents(orig.v, tree.node(entering.v).parent, entering.v);
        tree.add_child(entering.u, entering.v);
        tree.node(entering.v).dir = entering.dir;

        postorder_search(ancestor, tree.node(ancestor).min);

        fix_cut_values(g, h, ancestor, orig.u);
        fix_cut_values(g, h, ancestor, orig.v);
    }

    void fix_cut_values(const subgraph& g, hierarchy& h, vertex_t root, vertex_t u) {
        while (u != root) {
            vertex_t parent = tree.node(u).parent;
            set_cut_value(g, h, parent, u);
            u = parent;
        }
    }

    void move_subtree(hierarchy& h, vertex_t root, int d) {
        h.ranking[root] += d;
        for (auto child : tree.children(root)) {
            move_subtree(h, child, d);
        }
    }

    tree_edge find_entering_edge(const subgraph& g, hierarchy& h, tree_edge leaving) {
        tree_edge entering { 0, 0, -leaving.dir };
        int span = std::numeric_limits<int>::max();
        for_each_node(leaving.v, [&] (vertex_t u) {
            for (auto v : g.neighbours(u)) {
                if (tree.component( { leaving.u, leaving.v }, v ) == leaving.u) {
                    if (sgn(h.span(v, u)) != leaving.dir && std::abs(h.span(v, u)) < span) {
                        entering.u = v;
                        entering.v = u;
                        span = h.span(u, v);
                    }
                }
            }
        });
        return entering;
    }

    template< typename Func >
    void for_each_node(vertex_t root, Func f) {
        f(root);
        for (auto child : tree.children(root)) {
            for_each_node(child, f);
        }
    }

    void optimize_edge_length(const subgraph& g, hierarchy& h) {
repeat:
        for (auto u : g.vertices()) {
            if (u != tree.root && tree.node(u).cut_value < 0) {
                tree_edge leaving = { static_cast<vertex_t>(tree.node(u).parent), u, sgn(h.span(tree.node(u).parent, u)) };
                tree_edge entering = find_entering_edge(g, h, leaving);
                switch_tree_edges(g, h, leaving, entering);
                int d = h.span( entering.u, entering.v );
                d = -d + sgn(d);
                move_subtree(h, entering.v, d);
                goto repeat;
            }
        }
    }
};


} // namespace detail
