#pragma once

#include <algorithm>
#include <iostream>
#include <random>
#include <cassert>

#include "layering.hpp"
#include "utils.hpp"
#include "report.hpp"

namespace detail {

// ----------------------------------------------------------------------------------------------
// -------------------------------  COUNTING CROSSINGS  -----------------------------------------
// ----------------------------------------------------------------------------------------------

/**
 * Counts the number of crossings between edges indident on two *adjacent* vertices.
 * Takes into account both the ingoing and outgoing edges.
 * 
 * 'u' must be to the left of 'v'
 */
int count_vertex_crossings(const hierarchy& h, vertex_t u, vertex_t v) {
    int count = 0;
    for (auto out_u : h.g.out_neighbours(u)) {
        for (auto out_v : h.g.out_neighbours(v)) {
            if (h.pos[out_v] < h.pos[out_u]) {
                ++count;
            }
        }
    }
    for (auto in_u : h.g.in_neighbours(u)) {
        for (auto in_v : h.g.in_neighbours(v)) {
            if (h.pos[in_v] < h.pos[in_u]) {
                ++count;
            }
        }
    }
    return count;
}

// counts the number of crossings between layers with index 'layer' and 'layer - 1'
int count_layer_crossings(const hierarchy& h, int layer) {
    const std::vector<vertex_t>& upper = h.layers[layer - 1];
    const std::vector<vertex_t>& lower = h.layers[layer];
    int count = 0;

    for (int i = 0; i < upper.size() - 1; ++i) {
        for ( auto u : h.g.out_neighbours(upper[i]) ) {
            int j = h.pos[u];
            for (int s = i + 1; s < upper.size(); ++s) {
                for ( auto v : h.g.out_neighbours(upper[s]) ) {
                    int t = h.pos[v];
                    if ( (s > i && t < j) || (s < i && t > j) ) {
                        ++count;
                    }
                }
            }
        }
    }

    return count;
}


// counts the total number of crossings in the hierarchy
int count_crossings(const hierarchy& h) {
    int count = 0;
    for (int i = 1; i < h.size(); ++i) {
        count += count_layer_crossings(h, i);
    }
    return count;
}


// ----------------------------------------------------------------------------------------------
// -------------------------------  CROSSING REDUCTION  -----------------------------------------
// ----------------------------------------------------------------------------------------------

/**
 * Interface for a crossing reduction algorithm.
 * It reorders each layer of the hierarchy to avoid crossings.
 */
struct crossing_reduction {
    
    /**
     * Executes the algorithm. 
     * It should leave h in a consistent state - ranking, layers and pos all agree with each other.
     */
    virtual void run(hierarchy& h) = 0;
    virtual ~crossing_reduction() = default;
};


/**
 * Barycentric heurictic for crossing reduction.
 * Vertices on each layer are order based on their barycenters - the average position of its neighbours
 */
class barycentric_heuristic : public crossing_reduction {
    unsigned max_iters = defaults::iters;
    unsigned forgiveness = defaults::forgv;
    bool trans = defaults::trans;

    std::random_device dev;
    std::mt19937 mt{ dev() };

    vertex_map<int> best_order;
    int min_cross;

public:
    void run(hierarchy& h) override {
        best_order = h.pos;
        min_cross = count_crossings(h);

#ifdef REPORTING
        report::base = min_cross;
        report::random_runs = max_iters;
        report::forgivness = forgiveness;
        report::transpose = trans;
        report::random_final.clear();
        report::iters = 0;
#endif

        int base = min_cross;
        for (int i = 0; i < max_iters; ++i) {
            reduce(h, base);
            if (i != max_iters - 1) {
                for (auto& l : h.layers) {
                    std::shuffle(l.begin(), l.end(), mt);
                }
                h.update_pos();
                base = count_crossings(h);
            }
        }

        for (auto u : h.g.vertices()) {
            h.layer(u)[ best_order[u] ] = u;
        }
        h.update_pos();

#ifdef REPORTING
        report::final = min_cross;
#endif
    }

    void set_trans(bool a) { trans = a; }

private:
    // attempts to reduce the number of crossings
    void reduce(hierarchy& h, int base_cross) {
        vertex_map<float> weights(h.g);

        int local_min = base_cross;
        auto local_order = h.pos;
        int fails = 0;

        int i = 0;
        for (; true; ++i) {

            if (i % 2 == 0) { // top to bottom
                for (int j = 1; j < h.size(); ++j) {
                    reorder_layer(h, weights, j, true);
                }
            } else { // from bottom up
                for (int j = h.size() - 2; j >= 0; --j) {
                    reorder_layer(h, weights, j, false);
                }
            }

            if (trans) {
                transpose(h);
            }

            int cross = count_crossings(h);
            if (cross < local_min) {
                fails = 0;
                local_order = h.pos;
                local_min = cross;
            } else {
                fails++;
            }

            if (fails >= forgiveness) {
                break;
            }
        }

        if (local_min < min_cross) {
            best_order = local_order;
            min_cross = local_min;
        }

#ifdef REPORTING
        report::iters += i;
        report::random_final.push_back(local_min);
#endif
    }

    // reorders vertices on a layer 'i' based on their weights
    void reorder_layer(hierarchy& h, vertex_map<float>& weights, int i, bool downward) {
        auto& layer = h.layers[i];
        for (vertex_t u : layer) {
            weights[u] = weight( h.pos, u, downward ? h.g.in_neighbours(u) : h.g.out_neighbours(u) );
        }
        std::sort(layer.begin(), layer.end(), [&weights] (const auto& u, const auto& v) {
            return weights[u] < weights[v];
        });
        h.update_pos();
    }

    /**
     * Heuristic for reducing crossing 
     * which repeatedly attempts to swap all ajacent vertices
     * until no improvement is made.
     */
    void transpose(hierarchy& h) {
        bool improved = true;
        while (improved) {
            improved = false;
            
            for (auto& layer : h.layers) {
                assert(layer.size() >= 1);
                for (int i = 0; i < layer.size() - 1; ++i) {
                    int old = count_vertex_crossings(h, layer[i], layer[i + 1]);
                    int next = count_vertex_crossings(h, layer[i + 1], layer[i]);
                    int diff =  old - next;
                    
                    if ( diff > 0 ) {
                        //std::cout << "old: " << old << " next: " << next << "\n";
                        improved = true;
                        int before = count_crossings(h);
                        exchange(h, layer[i], layer[i + 1]);
                        assert( (before - diff == count_crossings(h)) );
                        //min_cross -= diff;
                    }
                }
            }
        }
    }

    // calculates the weight of vertex as an average of the positions of its neighbour
    template<typename T>
    float weight(const vertex_map<int>& positions, vertex_t u, const T& neighbours) {
        unsigned count = 0;
        unsigned sum = 0;
        for (auto v : neighbours) {
            sum += positions[v];
            count++;
        }
        if (count == 0) {
            return positions[u];
        }
        return sum / (float)count;
    }

    // exchange the positions of two vertices on the same layer
    void exchange(hierarchy& h, vertex_t u, vertex_t v) {
        h.layer(v)[ h.pos[v] ] = u;
        h.layer(u)[ h.pos[u] ] = v;
        int tmp = h.pos[u];
        h.pos[u] = h.pos[v];
        h.pos[v] = tmp;
    }

};

} // namespace detail