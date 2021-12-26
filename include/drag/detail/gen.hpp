#pragma once

#include <drag/graph.hpp>
#include <drag/detail/subgraph.hpp> // edge
#include <drag/detail/algo.hpp> // split
#include <drag/detail/utils.hpp>

#include <random>
#include <algorithm>
#include <assert.h>

namespace drag {

class dag_generator {
public:
    dag_generator() : m_engine(7) {}

    dag_generator(size_t seed) : m_engine(seed) {}

    drag::graph generate_from_density(size_t n, double density) {
        drag::graph g;
        std::vector<drag::vertex_t> vertices;

        for (size_t i = 0; i < n; ++i) {
            vertices.push_back(g.add_node());
        }

        std::uniform_real_distribution<double> dist(0.0, 1.0);

        for (size_t i = 0; i < n; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                if (dist(m_engine) < density) {
                    g.add_edge(vertices[i], vertices[j]);
                }
            }
        }
        
        connect_components(g);

        return g;
    }

    drag::graph generate_from_degree(size_t n, size_t deg) {
        double density = (n - 1) / (float) deg;
        return generate_from_density(n, density);
    }

    drag::graph generate_from_edges(size_t vertex_count, size_t edge_count) {
        drag::graph g;

        std::vector<drag::vertex_t> vertices;

        for (size_t i = 0; i < vertex_count; ++i) {
            vertices.push_back(g.add_node());
        }

        auto tree_edges = generate_tree_edges(vertices.size());

        for (auto [u, v] : tree_edges) {
            g.add_edge(vertices[u], vertices[v]);
        }

        std::vector<drag::detail::edge> remaining_edges;
        for (auto u : g.vertices()) {
            drag::detail::vertex_map<bool> is_succ(g);
            for (auto v : g.out_neighbours(u)) {
                is_succ[v] = true;
            }
            for (auto v : g.vertices()) {
                if (!is_succ[v] && u != v && u < v) {
                    remaining_edges.push_back({u, v});
                }
            }
        }

        std::shuffle(remaining_edges.begin(), remaining_edges.end(), m_engine);

        for (size_t i = 0; i + vertex_count - 1 < edge_count; ++i) {
            auto e = remaining_edges[i];
            g.add_edge(e.from, e.to);
        }

        return g;
    }

private:
    std::mt19937 m_engine;

    std::vector<std::pair<size_t, size_t>> generate_tree_edges(size_t n) {
        if (n <= 1) return {};
        if (n == 2) return { std::pair(0, 1) };

        std::vector<std::pair<size_t, size_t>> edges;

        // first generate a random Prüfer code - sequence of n - 2 node labels
        std::vector<size_t> seq(n - 2);

        // in this case we have labels 0...n-1
        std::uniform_int_distribution<size_t> seq_dist(0, n-1);
        
        for (size_t i = 0; i < seq.size(); ++i) {
            seq[i] = seq_dist(m_engine);
        }

        // now convert the code to the corresponding tree
        // first deduce the degrees of each vertex
        std::vector<size_t> degrees(n, 1);
        for (auto x : seq) {
            degrees[x]++;
        }

        // reconstruct the edges of the tree
        for (auto x : seq) {
            // find the smallest node with degree 1
            for (size_t i = 0; i < n; ++i) {
                if (degrees[i] == 1) {
                    size_t u = x, v = i;
                    if (u > v) std::swap(u, v);
                    edges.push_back({u, v});
                    degrees[x]--;
                    degrees[i]--;
                    break;
                }
            }
        }

        // add the last edge
        size_t u = -1;
        size_t v = -1;
        for (size_t i = 0; i < n; ++i) {
            if (degrees[i] == 1) {
                if (u == -1) {
                    u = i;
                } else {
                    v = i;
                }
            }
        }
        edges.push_back({u, v});

        return edges;
    }

    void connect_components(drag::graph& g) {
        auto components = drag::detail::split(g);
        auto n = components.size();

        if (components.size() == 1) {
            return;
        }

        if (components.size() == 2) {
            add_component_edge(g, components[0].vertices(), components[1].vertices());
            return;
        }

        // first generate a random Prüfer code - sequence of n - 2 numbers from 0 to n - 3
        std::uniform_int_distribution<size_t> dist(0, components.size() - 3);
        std::vector<size_t> seq(components.size() - 2);
        for (size_t i = 0; i < components.size() - 2; ++i) {
            seq[i] = dist(m_engine);
        }

        // now convert the code to the corresponding tree
        // 1) compute degrees
        std::vector<size_t> degrees(n, 1);
        for (auto x : seq) {
            degrees[x]++;
        }

        // 2) process the sequence
        std::uniform_int_distribution<int> orientation(0, 1);
        for (auto x : seq) {
            for (size_t i = 0; i < n; ++i) {
                if (degrees[i] == 1) {
                    auto from = x;
                    auto to = i;
                    if (orientation(m_engine) == 1) {
                        std::swap(from, to);
                    }
                    add_component_edge(g, components[from].vertices(), components[to].vertices());
                    degrees[x]--;
                    degrees[i]--;
                    break;
                }
            }
        }

        // 3) add the last edge
        size_t from = -1;
        size_t to = -1;
        for (size_t i = 0; i < n; ++i) {
            if (degrees[i] == 1) {
                if (from == -1) {
                    from = i;
                } else {
                    to = i;
                }
            }
        }
        assert(from != -1 && to != -1);
        if (orientation(m_engine) == 1) {
            std::swap(from, to);
        }
        add_component_edge(g, components[from].vertices(), components[to].vertices());
    }

    void add_component_edge(drag::graph& g, const std::vector<drag::vertex_t>& from, const std::vector<drag::vertex_t>& to) {
        //assert(!from.empty());
        //assert(!to.empty());
        std::uniform_int_distribution<size_t> from_dist(0, from.size() - 1);
        std::uniform_int_distribution<size_t> to_dist(0, to.size() - 1);
        auto u = from[from_dist(m_engine)];
        auto v = to[to_dist(m_engine)];
        g.add_edge(u, v);
    }
};

}
