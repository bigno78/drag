#pragma once

#include <algorithm>
#include <tuple>
#include <array>
#include <optional>

#include "stuff.hpp"
#include "subgraph.hpp"
#include "vec2.hpp"
#include "layering.hpp"
#include "stuff.hpp"

namespace detail {

/**
 * Attributes controlling spacing.
 */
struct positioning_attributes {
    float node_dist;
    float layer_dist;
};

/**
 * Interface for a positioning algorithm that determines the final positions of nodes.
 * The y coordinates of nodes on the same layer have to be the same.
 */
struct positioning {
    virtual vec2 run(const subgraph& g, const hierarchy& h, vec2 origin) = 0;
    virtual ~positioning() = default;
};


/**
 * Naive positioning for testing purpouses.
 */
struct test_positioning : public positioning {
    std::vector<node>& nodes;
    positioning_attributes attr;

    test_positioning(positioning_attributes attr, std::vector<node>& nodes)
        : nodes(nodes)
        , attr(std::move(attr)) {}  

    vec2 run(const subgraph& g, const hierarchy& h, vec2 origin) override {
        float y = origin.y + attr.layer_dist;
        float width = 0;
        for (auto layer : h.layers) {
            float x = origin.x;
            for (auto u : layer) {
                x += attr.node_dist + g.node_size(u);
                nodes[u].pos = { x, y };
                nodes[u].size = g.node_size(u);
                x += g.node_size(u);
            }
            if ((x - origin.x) > width) {
                width = x;
            }
            y += attr.layer_dist;
        }
        return { width, y - origin.y };
    }
};


struct edge_set {
    vertex_flags< std::vector<vertex_t> > data;

    bool contains(edge e) const { return contains(e.tail, e.head); }
    bool contains(vertex_t u, vertex_t v) const { 
        return data.contains(u) && std::find(data[u].begin(), data[u].end(), v) != data[u].end();
    }

    void insert(edge e) { insert(e.tail, e.head); }
    void insert(vertex_t u, vertex_t v) {
        data.add_vertex(u);
        data[u].push_back(v);
    }

    /*void clear() {  
        for (auto& vec : data) {
            vec.clear();
        }
    }*/

   /* struct iterator {
        const std::vector< std::vector<vertex_t> >& data;
        int layer = 0;
        int i = 0;

        iterator(const std::vector< std::vector<vertex_t> >& data, int layer = 0, int i = 0)
            : data(data)
            , layer(layer)
            , i(i) {}

        edge operator*() { return data[layer][i]; }
        friend bool operator==(const iterator& lhs, const iterator& rhs) { return rhs.layer == lhs.layer && rhs.i == lhs.i; }
        friend bool operator!=(const iterator& lhs, const iterator& rhs)
    };*/
};


class fast_and_simple_positioning : public positioning {
public:
    std::vector<node>& nodes;
    positioning_attributes attr;

    vertex_flags< std::pair<vertex_t, vertex_t> > upper_medians;
    vertex_flags< std::pair<vertex_t, vertex_t> > lower_medians;

    enum orient { upper_left, lower_left, upper_right, lower_right };
    std::array< vertex_flags<vertex_t>, 4 > root;
    std::array< vertex_flags<vertex_t>, 4 > align;
    std::array< vertex_flags<vertex_t>, 4 > sink;
    std::array< vertex_flags< std::optional<float> >, 4 > shift;
    std::array< vertex_flags< std::optional<float> >, 4 > x;

    vertex_flags<int> pos;

    edge_set conflicting;


    fast_and_simple_positioning(positioning_attributes attr, std::vector<node>& nodes, const graph& g)
        : nodes(nodes)
        , attr(std::move(attr))
        , upper_medians(g)
        , lower_medians(g)
        , pos(g) 
    {  
        for (int i = 0; i < 4; ++i) {
            root[i].resize(g);
            align[i].resize(g);
            sink[i].resize(g);
            shift[i].resize(g);
            x[i].resize(g);
        }
        for (int i = 0; i < g.size(); ++i) {
            for (int j = 0; j < 4; ++j) {
                root[j][i] = i;
                align[j][i] = i;
                sink[j][i] = i;
            }
        }
    }

    vec2 run(const subgraph& g, const hierarchy& h, vec2 origin) override {
        init_pos(h);
        init_medians(h);
        mark_conflicts(h);

        left_align(h, 
                   lower_medians, 
                   [this] (auto u, auto med, auto type) { lower_alignment_setter(u, med, type); },
                   [this] (auto u, auto med) { lower_validator(u, med); }, 
                   orient::lower_left);

        for (auto u : h.g.vertices()) {
            if (root[orient::lower_left][u] = u) {
                place_block(h, u, orient::lower_left);
            }
        }

        for (auto u : h.g.vertices()) {
            orient t = orient::lower_left;
            x[t][u] = x[t][ root[t][u] ];
            if (shift[t][ sink[t][ root[t][u] ] ]) {
                x[t][u] = *x[t][u] + *shift[t][ sink[t][ root[t][u] ] ];
            }
        }

        float y = origin.y;
        orient t = orient::lower_left;
        for (auto l : h.layers) {
            for (auto u : l) {
                nodes[u].pos = { origin.x + *x[t][u], y };
            }
            y += attr.layer_dist;
        }
        /*
        std::cout << h << "\n";
        std::cout << "MEDIAN: \n";
        for (auto u : h.g.vertices()) {
            std::cout << u << ": ";
            std::cout << "[" << upper_medians[u].first << ", " << upper_medians[u].second << " | ";
            std::cout << lower_medians[u].first << ", " << lower_medians[u].second << "]";
            std::cout << "\n";
        }
        std::cout << "\n";

        std::cout << h << "\n";
        std::cout << "CONFLICTS: \n";
        for (auto u : h.g.vertices()) {
            for (auto v : h.g.out_neighbours(u)) {
                if (conflicting.contains(u, v)) {
                    std::cout << edge{u, v} << "\n";
                }
            }
        }*/

        return { 0, 0 };
    }

    enum class bias { right, left };

    void init_pos(const hierarchy& h) {
        for (const auto& layer : h.layers) {
            int i = 0;
            for (auto u : layer) {
                pos[u] = i++;
            }
        }
    }

    void init_medians(const hierarchy& h) {
        for (const auto& layer : h.layers) {
            for (auto u : layer) {
                lower_medians[u] = { median(h.g.out_neighbours(u), bias::left), 
                                     median(h.g.out_neighbours(u), bias::right) };
                upper_medians[u] = { median(h.g.in_neighbours(u), bias::left), 
                                     median(h.g.in_neighbours(u), bias::right) };
            }
        }
    }

    template< typename Neighbours >
    vertex_t median(Neighbours neigh, bias b) {
        int count = neigh.size();
        int m = count / 2;
        if (count == 0) {
            return 0;
        }
        if (count % 2 == 0 && b == bias::left) {
            m -= 1;
        }
        std::nth_element(
                neigh.begin(), 
                neigh.begin() + m, 
                neigh.end(),
                [this] (const auto u, const auto v) {
                    return pos[u] < pos[v];
                });
        return *(neigh.begin() + m);
    }


    // Is 'u' tail of an inner segment?
    bool is_inner(const hierarchy& h, vertex_t u) {
        if (h.g.out_degree(u) != 1 || !h.g.is_dummy(u)) {
            return false;
        }
        return h.g.is_dummy( *h.g.out_neighbours(u).begin() );
    }

    /**
     * Vertex 'u' must be a tail of inner segment.
     * Returns the position of the head of this inner segment on its layer.
     */
    int inner_pos(const hierarchy& h, vertex_t u) {
        return pos[ *h.g.out_neighbours(u).begin() ];
    }

    /**
     * Saves edges causing type 1 conflicts.
     * Type 1 conflict occur when non-inner edge crosses inner segment.
     * Inner segment is an edge between two dummy vertices.
     */
    void mark_conflicts(const hierarchy& h) {
        for (int i = 1; i < h.size() - 2; ++i) {
            int last_pos = 0;
            int p = 0;
            for ( int j = 0; j < h.layers[i].size(); ++j ) {
                vertex_t u = h.layers[i][j];
                if ( j == h.layers[i].size() - 1 || is_inner(h, u) ) {
                    int curr_pos = h.layers[i].size();
                    if (is_inner(h, u)) {
                        curr_pos = inner_pos(h, u);
                    }
                    while(p <= j) {
                        vertex_t pth = h.layers[i][p];
                        for (auto v : h.g.out_neighbours(pth)) {
                            if (pos[v] < last_pos || pos[v] > curr_pos) {
                                conflicting.insert(pth, v);
                            }
                        }
                        ++p;
                    }
                    last_pos = curr_pos; 
                }
            }
        }
    }

    template<typename AlignSetter, typename AlignValidator>
    void left_align(const hierarchy& h,
               vertex_flags< std::pair<vertex_t, vertex_t> >& medians,
               AlignSetter setter,
               AlignValidator validator,
               orient layout_type)
    {
        for (const auto& layer : h.layers) {
            int r = 0;
            for ( int k = 0; k < layer.size(); ++k ) {
                vertex_t u = layer[k];
                for (auto m : { medians[u].first, medians[u].second }) {
                    if ( align[layout_type][u] = u && validator(u, m) && pos[m] > r ) {
                        setter(u, m);
                        r = pos[m];
                    }
                }
            }
        }
    }

    template<typename RangeGetter, typename AlignSetter, typename AlignValidator>
    void right_align(const hierarchy& h,
               vertex_flags< std::pair<vertex_t, vertex_t> >& medians,
               AlignSetter setter,
               AlignValidator validator,
               orient layout_type)
    {
        for (const auto& layer : h.layers) {
            int r = 0;
            for ( int k = layer.size() - 1; k >= 0; --k ) {
                vertex_t u = layer[k];
                for (auto m : { medians[u].second, medians[u].first }) {
                    if ( align[layout_type][u] = u && validator(u, m) && pos[m] < r) {
                        setter(u, m);
                        r = pos[m];
                    }
                }
            }
        }
    }

    /**
     * Validates that its possible to align vertex 'u', with vertex 'med'
     * which is located on a layer above 'u'.
     */
    bool upper_validator(vertex_t u, vertex_t med) { return !conflicting.contains(med, u); }
    bool lower_validator(vertex_t u, vertex_t med) { return !conflicting.contains(u, med); }

    void upper_alignment_setter(vertex_t u, vertex_t med, orient layout_type) { 
        alignment_setter(med, u, layout_type);
    }

    void lower_alignment_setter(vertex_t u, vertex_t med, orient layout_type) { 
        alignment_setter(u, med, layout_type);
    }

    void alignment_setter(vertex_t u, vertex_t v, orient layout_type) {
        align[layout_type][u] = v;
        root[layout_type][v] = root[layout_type][u];
        align[layout_type][v] = root[layout_type][v];
    }

    void place_block(const hierarchy& h, vertex_t u, orient type) {
        if (x[type][u]) {
            return;
        }

        x[type][u] = 0;
        vertex_t w = u;
        do {
            if (pos[w] > 0) {
                vertex_t v = h.layer(w)[pos[w] - 1];
                vertex_t rv = root[ type ][ v ];
                place_block(h, rv, type);
                if (sink[type][u] = u)
                    sink[type][u] = sink[type][rv];
                if (sink[type][u] != sink[type][rv]) {
                    shift[type][ sink[type][rv] ] = std::min(*shift[type][ sink[type][rv] ], 
                                                             *x[type][u] - *x[type][rv] - node_dist(w, v));
                } else {
                    x[type][u] = std::max(*x[type][u], *x[type][rv] + node_dist(w, v));
                }
            }
            w = align[type][w];
        } while (w != u);
    }

    float node_dist(vertex_t u, vertex_t v) {
        return nodes[u].size + nodes[v].size + attr.node_dist;
    }
};


} // namespace detail