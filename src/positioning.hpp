#pragma once

#include <algorithm>
#include <tuple>
#include <array>
#include <optional>

#include "utils.hpp"
#include "subgraph.hpp"
#include "vec2.hpp"
#include "layering.hpp"

#ifdef DEBUG_COORDINATE
int produce_layout = 0;
#endif

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
    vertex_map< std::vector<vertex_t> > data;

    bool contains(edge e) const { return contains(e.tail, e.head); }
    bool contains(vertex_t u, vertex_t v) const { 
        return data.contains(u) && std::find(data[u].begin(), data[u].end(), v) != data[u].end();
    }

    void insert(edge e) { insert(e.tail, e.head); }
    void insert(vertex_t u, vertex_t v) {
        data.add_vertex(u);
        data[u].push_back(v);
    }
};


class fast_and_simple_positioning : public positioning {
public:
    std::vector<node>& nodes;
    positioning_attributes attr;

    vertex_map< std::pair<vertex_t, vertex_t> > upper_medians;
    vertex_map< std::pair<vertex_t, vertex_t> > lower_medians;

    enum orient { upper_left, lower_left, upper_right, lower_right };
    std::array< vertex_map<vertex_t>, 4 > medians;
    std::array< vertex_map<vertex_t>, 4 > root;
    std::array< vertex_map<vertex_t>, 4 > align;
    std::array< vertex_map<vertex_t>, 4 > sink;
    std::array< vertex_map< std::optional<float> >, 4 > shift;
    std::array< vertex_map< std::optional<float> >, 4 > x;
    std::array< float, 4 > max;
    std::array< float, 4 > min;

    vertex_map<int> pos;

    vertex_map<float> layer_size;

    edge_set conflicting;


    fast_and_simple_positioning(positioning_attributes attr, std::vector<node>& nodes, const graph& g)
        : nodes(nodes)
        , attr(std::move(attr))
        , upper_medians(g)
        , lower_medians(g)
        , pos(g) 
    { }

    void init(const hierarchy& h) {
        for (int i = 0; i < 4; ++i) {
            medians[i].resize(h.g);
            root[i].resize(h.g);
            align[i].resize(h.g);
            sink[i].resize(h.g);
            shift[i].resize(h.g);
            x[i].resize(h.g);
        }
        layer_size.resize(h.g, std::numeric_limits<float>::lowest());

        for (auto u : h.g.vertices()) {
            for (int j = 0; j < 4; ++j) {
                root[j][u] = u;
                align[j][u] = u;
                sink[j][u] = u;
                nodes[u].size = h.g.node_size(u);
            }
        }

        // init positions on the layers
        pos.resize(h.g);
        for (const auto& layer : h.layers) {
            int i = 0;
            for (auto u : layer) {
                pos[u] = i++;
                if (nodes[u].size > layer_size[u]) {
                    layer_size[u] = nodes[u].size;
                }
            }
        }

        for (int i = 0; i < 4; ++i) {
            min[i] = std::numeric_limits<float>::lowest();
            max[i] = std::numeric_limits<float>::max();
        }
    }

    vec2 run(const subgraph& g, const hierarchy& h, vec2 origin) override {
        init(h);
        init_medians(h);

        mark_conflicts(h);

        /*for (auto u : h.g.vertices()) {
            for (auto v : h.g.out_neighbours(u)) {
                if (conflicting.contains(u, v)) {
                    std::cout << edge{u, v} << "\n";
                }
            }
        }*/

#ifdef DEBUG_COORDINATE
        std::cout << "TYPE: " << produce_layout << "\n";
        if (produce_layout < 4) {
            /*for (auto u : h.g.vertices()) {
                std::cout << u << "[" << medians[produce_layout][u] << "][" 
                          << medians[invert_horizontal(static_cast<orient>(produce_layout))][u] << "]\n";
            }*/
            vertical_align(h, static_cast<orient>(produce_layout));

            /*for (auto u : h.g.vertices()) {
                if (root[produce_layout][u] == u) {
                    vertex_t v = u;
                    const char* sep = "";
                    do {
                        std::cout << sep << v;
                        sep = " -> ";
                        v = align[produce_layout][v];
                    } while (v != u);
                    std::cout << "\n";
                }
            }
            std::cout << "\n";*/

            horizontal_compaction(h, static_cast<orient>(produce_layout));

            origin = origin + vec2{ layer_size[0], layer_size[0] };
            float y = origin.y;
            for (auto l : h.layers) {
                for (auto u : l) {
                    nodes[u].pos = { *x[static_cast<orient>(produce_layout)][u], y };
                    nodes[u].size = h.g.node_size(u);
                }
                y += attr.layer_dist;
            }
            float width = normalize(h, origin.x);

            return { width, y };
        }
#endif

        for (int i = 0; i < 4; ++i) {
            vertical_align(h, static_cast<orient>(i));
            horizontal_compaction(h, static_cast<orient>(i));
        }

        // find the layout with smallest width
        orient min_width_layout = static_cast<orient>(0);
        for (int i = 1; i < 4; ++i) {
            if ( max[min_width_layout] - min[min_width_layout] < max[i] - min[i] ) {
                min_width_layout = static_cast<orient>(i);
            }
        }

        // calculate how much other layouts need to be shifted to align them to the smallest width one
        float shift[4];
        for (int i = 0; i < 4; ++i) {
            if ( left(static_cast<orient>(i)) ) {
                shift[i] = min[min_width_layout] - min[i];
            } else {
                shift[i] = max[min_width_layout] - max[i];
            }
        }
        

        float y = origin.y;
        std::vector<float> vals;
        for (int l = 0; l < h.size(); ++l) {
            y += layer_size[l];
            for (auto u : h.layers[l]) {
                vals = { *x[0][u] + shift[0],
                         *x[1][u] + shift[1],
                         *x[2][u] + shift[2],
                         *x[3][u] + shift[3] };
                std::sort(vals.begin(), vals.end());
                nodes[u].pos = { (vals[1] + vals[2])/2, y };
                nodes[u].size = h.g.node_size(u);
            }
            y += layer_size[l] + attr.layer_dist;
        }

        float width = normalize(h, origin.x);

        return { width, y - attr.layer_dist };
    }

    void horizontal_compaction(const hierarchy& h, orient dir) {
        auto& sink = this->sink[dir];
        auto& root = this->root[dir];
        auto& shift = this->shift[dir];
        auto& x = this->x[dir];

        for (auto u : h.g.vertices()) {
            if (root[u] == u) {
                place_block(h, u, dir);
            }
        }

        for (auto i : idx_range(h.size(), up(dir))) {
            auto layer = h.layers[i];
            for (auto u : layer) {
                x[u] = x[ root[u] ];
                if ( shift[ sink[ root[u] ] ] ) {
                    x[u] = *x[u] + *shift[ sink[ root[u] ] ];

                    if (*x[u] > max[dir]) {
                        max[dir] = *x[u];
                    }

                    if (*x[u] < min[dir]) {
                        min[dir] = *x[u];
                    }
                }
            }
        }
    }

    float normalize(const hierarchy& h, float start) {
        float min = 0, max = 0;
        for(auto u : h.g.vertices()) {
            if (nodes[u].pos.x + nodes[u].size > max) {
                max = nodes[u].pos.x + nodes[u].size;
            }
            if (nodes[u].pos.x - nodes[u].size < min) {
                min = nodes[u].pos.x - nodes[u].size;
            }
        }

        for(auto u : h.g.vertices()) {
            nodes[u].pos.x = start + nodes[u].pos.x - min;
        }
        return max - min;
    }

    void init_medians(const hierarchy& h) {
        std::vector<vertex_t> neighbours;
        for (auto u : h.g.vertices()) {
            {
                neighbours.insert(neighbours.begin(), h.g.out_neighbours(u).begin(), h.g.out_neighbours(u).end());
                auto [ left, right ] = median(u, neighbours);
                medians[orient::lower_left][u] = left;
                medians[orient::lower_right][u] = right;
                neighbours.clear();
            }

            neighbours.insert(neighbours.begin(), h.g.in_neighbours(u).begin(), h.g.in_neighbours(u).end());
            auto [ left, right ] = median(u, neighbours);
            medians[orient::upper_left][u] = left;
            medians[orient::upper_right][u] = right;
            neighbours.clear();
        }
    }

    template< typename Neighbours >
    std::pair<vertex_t, vertex_t> median(vertex_t u, Neighbours neigh) {
        int count = neigh.size();
        int m = count / 2;
        if (count == 0) {
            return { u, u };
        }
        std::nth_element(
                neigh.begin(), 
                neigh.begin() + m, 
                neigh.end(),
                [this] (const auto u, const auto v) {
                    return pos[u] < pos[v];
                });
        vertex_t right = *(neigh.begin() + m);
        if (count % 2 == 1) {
            return { right, right };
        }
        std::nth_element(
                neigh.begin(), 
                neigh.begin() + m - 1, 
                neigh.end(),
                [this] (const auto u, const auto v) {
                    return pos[u] < pos[v];
                });
        return { *(neigh.begin() + m - 1), right };
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
        if (h.size() < 4) {
            return;
        }
        for (int i = 1; i < h.size() - 2; ++i) {
            int last_pos = 0;
            int p = 0;

            for ( int j = 0; j < h.layers[i].size(); ++j ) {
                auto& lay = h.layers[i];
                vertex_t u = lay[j];

                if ( j == h.layers[i].size() - 1 || is_inner(h, u) ) {
                    int curr_pos = h.layers[i + 1].size();

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

    // for each vertex choose the vertex it will be verticaly aligned to
    void vertical_align(const hierarchy& h, orient dir) {
        vertex_map<vertex_t>& align = this->align[dir];
        vertex_map<vertex_t>& root = this->root[dir];

        for ( auto l : idx_range(h.size(), !up(dir)) ) {
            auto layer = h.layers[l];

            int m_pos = left(dir) ? 0 : h.g.size();
            int d = left(dir) ? 1 : -1;

            for ( auto k : idx_range(layer.size(), !left(dir)) ) {
                vertex_t u = layer[k];
                
                for ( auto m : { medians[dir][u], medians[invert_horizontal(dir)][u] } ) {
                    if (m != u && align[u] == u && !is_conflicting(u, m, dir) && d*pos[m] >= d*m_pos) {
                        align[m] = u;
                        root[u] = root[m];
                        align[u] = root[m];

                        m_pos = pos[m] + d;
                    }
                }
            }
        }
    }

    void place_block(const hierarchy& h, vertex_t u, orient type) {
        if (x[type][u]) {
            return;
        }

        auto& sink = this->sink[type];
        auto& root = this->root[type];
        auto& shift = this->shift[type];
        auto& align = this->align[type];
        auto& x = this->x[type];

        x[u] = 0;
        int d = left(type) ? -1 : 1;
        vertex_t w = u;
        do {
            if ( !is_last_idx(pos[w], h.layer(w).size(), !left(type)) ) {
                vertex_t v = h.layer(w)[ pos[w] + d ];
                vertex_t rv = root[v];

                place_block(h, rv, type);

                if (sink[u] == u)
                    sink[u] = sink[rv];

                if (sink[u] != sink[rv]) {
                    float new_shift = *x[u] - *x[rv] - node_dist(w, v);
                    if (!shift[ sink[rv] ]) {
                        shift[ sink[rv] ] = new_shift;
                    } else {
                        shift[ sink[rv] ] = left(type) ? std::min(*shift[ sink[rv] ], new_shift)
                                                       : std::max(*shift[ sink[rv] ], new_shift);
                    }
                } else {
                    float new_x = *x[rv] - d*node_dist(w, v);
                    x[u] = !left(type) ? std::min(*x[u], new_x)
                                      : std::max(*x[u], new_x);
                }
            }
            w = align[w];
        } while (w != u);
    }
    

    bool left(orient dir) const { return dir == orient::lower_left || dir == orient::upper_left; }
    bool up(orient dir) const { return dir == orient::upper_left || dir == orient::upper_right; }
    
    // do the vertex 'u' and its median 'med' participate in type 1 conflict?
    bool is_conflicting(vertex_t u, vertex_t med, orient dir) {
        return (  up(dir) && conflicting.contains(med, u) ) || 
               ( !up(dir) && conflicting.contains(u, med) );
    }

    float node_dist(vertex_t u, vertex_t v) {
        return nodes[u].size + nodes[v].size + attr.node_dist;
    }

    // Get the range of indexes.
    range<int> idx_range(std::size_t size, bool desc) const {
        if ( desc ) {
            return { static_cast<int>(size) - 1, -1, -1 };
        }
        return { 0, static_cast<int>(size), 1 };
    }

    // Is i one of the last index in the interval <0, size - 1>?
    bool is_last_idx(int i, std::size_t size, bool desc) const {
        return (desc && i == size - 1) || (!desc && i == 0);
    }

    orient invert_horizontal(orient dir) {
        switch(dir) {
            case orient::upper_left:
                return orient::upper_right;
            case orient::upper_right:
                return orient::upper_left;
            case orient::lower_left:
                return orient::lower_right;
            case orient::lower_right:
                return orient::lower_left;      
        }
    }
    
    

    
    // FOR DEBUG PURPOUSES
    
    void lower_left_align(const hierarchy& h) {
        vertex_map<vertex_t>& align = this->align[orient::lower_left];
        vertex_map<vertex_t>& root = this->root[orient::lower_left];

        for (int l = h.size() - 1; l >= 0; --l) {
            auto layer = h.layers[l];
            int m_pos = -1;

            for (int k = 0; k < layer.size(); ++k) {
                vertex_t u = layer[k];
                
                for ( auto m : { medians[orient::lower_left][u], medians[orient::lower_right][u] } ) {
                    if (m != u && align[u] == u && !conflicting.contains(u, m) && pos[m] > m_pos) {
                        align[m] = u;
                        root[u] = root[m];
                        align[u] = root[u];

                        m_pos = pos[m];
                    }
                }
            }
        }
    }

    void lower_right_align(const hierarchy& h) {
        vertex_map<vertex_t>& align = this->align[orient::lower_right];
        vertex_map<vertex_t>& root = this->root[orient::lower_right];

        for (int l = h.size() - 1; l >= 0; --l) {
            auto layer = h.layers[l];
            int m_pos = h.g.size();

            for (int k = layer.size() - 1; k >= 0; --k) {
                vertex_t u = layer[k];
                
                for ( auto m : { medians[orient::lower_right][u], medians[orient::lower_left][u] } ) {
                    if (m != u && align[u] == u && !conflicting.contains(u, m) && pos[m] < m_pos) {
                        align[m] = u;
                        root[u] = root[m];
                        align[u] = root[u];

                        m_pos = pos[m];
                    }
                }
            }
        }
    }

    void upper_right_align(const hierarchy& h) {
        vertex_map<vertex_t>& align = this->align[orient::upper_right];
        vertex_map<vertex_t>& root = this->root[orient::upper_right];

        for (int l = 0; l < h.size(); ++l) {
            auto layer = h.layers[l];
            int m_pos = h.g.size();

            for (int k = layer.size() - 1; k >= 0; --k) {
                vertex_t u = layer[k];
                
                for ( auto m : { medians[orient::upper_right][u], medians[orient::upper_left][u] } ) {
                    if (m != u && align[u] == u && !conflicting.contains(u, m) && pos[m] < m_pos) {
                        align[m] = u;
                        root[u] = root[m];
                        align[u] = root[u];

                        m_pos = pos[m];
                    }
                }
            }
        }
    }

    void upper_left_align(const hierarchy& h) {
        vertex_map<vertex_t>& align = this->align[orient::upper_left];
        vertex_map<vertex_t>& root = this->root[orient::upper_left];

        for (int l = 0; l < h.size(); ++l) {
            auto layer = h.layers[l];
            int m_pos = -1;

            for (int k = 0; k < layer.size(); ++k) {
                vertex_t u = layer[k];
                
                for ( auto m : { medians[orient::upper_left][u], medians[orient::upper_right][u] } ) {
                    if (m != u && align[u] == u && !conflicting.contains(u, m) && pos[m] > m_pos) {
                        align[m] = u;
                        root[u] = root[m];
                        align[u] = root[u];

                        m_pos = pos[m];
                    }
                }
            }
        }
    }

     void left_place_block(const hierarchy& h, vertex_t u, orient type) {
        if (x[type][u]) {
            return;
        }

        x[type][u] = 0;
        vertex_t w = u;
        do {
            if (pos[w] > 0) {
                vertex_t v = h.layer(w)[pos[w] - 1];
                vertex_t rv = root[ type ][ v ];
                left_place_block(h, rv, type);
                if (sink[type][u] == u)
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

    void right_place_block(const hierarchy& h, vertex_t u, orient type) {
        if (x[type][u]) {
            return;
        }

        x[type][u] = 0;
        vertex_t w = u;
        do {
            if (pos[w] < h.layer(w).size() - 1) {
                vertex_t v = h.layer(w)[pos[w] + 1];
                vertex_t rv = root[ type ][ v ];
                right_place_block(h, rv, type);
                if (sink[type][u] == u)
                    sink[type][u] = sink[type][rv];
                if (sink[type][u] != sink[type][rv]) {
                    shift[type][ sink[type][rv] ] = std::max(*shift[type][ sink[type][rv] ], 
                                                             *x[type][u] - *x[type][rv] - node_dist(w, v));
                } else {
                    x[type][u] = std::min(*x[type][u], *x[type][rv] - node_dist(w, v));
                }
            }
            w = align[type][w];
        } while (w != u);
    }
};


} // namespace detail