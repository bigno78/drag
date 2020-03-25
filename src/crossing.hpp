#pragma once

#include <algorithm>
#include <iostream>
#include <random>

#include "layering.hpp"
#include "utils.hpp"

namespace detail {


/**
 * Interface for a crossing reduction algorithm.
 * It reorders each layer of the hierarchy to avoid crossings.
 */
struct crossing_reduction {
    virtual void run(hierarchy& h) = 0;
    virtual ~crossing_reduction() = default;
};


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


class barycentric_heuristic : public crossing_reduction {
    unsigned max_iters = 10;
    unsigned forgiveness = 10;

    std::random_device dev;
    std::mt19937 mt{ dev() };

    vertex_map<int> best_order;
    int min_cross;

public:
    void run(hierarchy& h) override {
        best_order = h.pos;
        min_cross = count_crossings(h);

        for (int i = 0; i < max_iters; ++i) {
            reduce(h);
            for (auto& l : h.layers) {
                std::shuffle(l.begin(), l.end(), mt);
            }
            h.update_pos();
        }

        for (auto u : h.g.vertices()) {
            h.layer(u)[ best_order[u] ] = u;
        }
    }

private:
    void reduce(hierarchy& h) {
        vertex_map<float> weights(h.g);

        int fails = 0;

#ifdef DEBUG_CROSSING
        std::cout << "CROSS BEFORE: " << count_crossings(h, positions) << "\n";
#endif

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

            int cross = count_crossings(h);
            //std::cout << "min cross " << min_cross << " X cross: " << cross << "\n";
            if (cross < min_cross) {
                //std::cout << "better\n";
                fails = 0;
                best_order = h.pos;
                min_cross = cross;
            } else {
                //std::cout << "worse\n";
                fails++;
            }

            if (fails >= forgiveness) {
                break;
            }
        }

#ifdef DEBUG_CROSSING
        std::cout << "iters: " << i << "\n";
        std::cout << "CROSS AFTER: " << count_crossings(h, positions) << "\n";
#endif
    }

    void reorder_layer(hierarchy& h, vertex_map<float>& weights, int i, bool downward) {
        for (vertex_t u : h.layers[i]) {
            weights[u] = weight( h.pos, u, downward ? h.g.in_neighbours(u) : h.g.out_neighbours(u) );
        }
        std::sort(h.layers[i].begin(), h.layers[i].end(), [&weights] (const auto& u, const auto& v) {
            return weights[u] < weights[v];
        });
        h.update_pos();
    }

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

};

} // namespace detail