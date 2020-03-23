#pragma once

#include <algorithm>
#include <iostream>
#include <random>

#include "layering.hpp"
#include "utils.hpp"

namespace detail {

struct crossing_reduction {
    virtual void run(hierarchy& h) = 0;
    virtual ~crossing_reduction() = default;
};

int count_layer_crossings(const hierarchy& h, const vertex_map<int>& positions, int layer) {
    const std::vector<vertex_t>& upper = h.layers[layer - 1];
    const std::vector<vertex_t>& lower = h.layers[layer];
    int count = 0;

    for (int i = 0; i < upper.size() - 1; ++i) {
        for ( auto u : h.g.out_neighbours(upper[i]) ) {
            int j = positions[u];
            for (int s = i + 1; s < upper.size(); ++s) {
                for ( auto v : h.g.out_neighbours(upper[s]) ) {
                    int t = positions[v];
                    if (s > i && t < j || s < i && t > j) {
                        ++count;
                    }
                }
            }
        }
    }

    return count;
}

int count_crossings(const hierarchy& h, const vertex_map<int>& positions) {
    int count = 0;
    for (int i = 1; i < h.size(); ++i) {
        count += count_layer_crossings(h, positions, i);
    }
    return count;
}

struct barycentric_heuristic : public crossing_reduction {
    unsigned max_iters = 10;
    unsigned forgiveness = 10;

    std::random_device dev;
    std::mt19937 mt{ dev() };

    std::vector< std::vector<vertex_t> > best_order;
    int min_cross;


    void run(hierarchy& h) override {
        best_order = h.layers;
        vertex_map<int> positions(h.g);
        for (auto l : h.layers) {
            int pos = 0;
            for (auto u : l) {
                positions[u] = pos;
                ++pos;
            }
        }
        min_cross = count_crossings(h, positions);

        for (int i = 0; i < max_iters; ++i) {
            reduce(h);
            for (auto& l : h.layers) {
                std::shuffle(l.begin(), l.end(), mt);
            }
        }
        h.layers = best_order;
    }

private:
    void reduce(hierarchy& h) {
        vertex_map<float> weights(h.g);
        vertex_map<int> positions(h.g);

        for (auto l : h.layers) {
            int pos = 0;
            for (auto u : l) {
                positions[u] = pos;
                ++pos;
            }
        }

        int fails = 0;

#ifdef DEBUG_CROSSING
        std::cout << "CROSS BEFORE: " << count_crossings(h, positions) << "\n";
#endif

        int i = 0;
        for (; true; ++i) {

            if (i % 2 == 0) { // top to bottom

                for (int j = 1; j < h.size(); ++j) {
                    for (vertex_t u : h.layers[j]) {
                        weights[u] = weight( positions, u, h.g.in_neighbours(u) );
                    }
                    std::sort(h.layers[j].begin(), h.layers[j].end(), [&weights] (const auto& u, const auto& v) {
                        return weights[u] < weights[v];
                    });
                    update_positions(positions, h.layers[j]);
                }

            } else { // from bottom up

                for (int j = h.size() - 2; j >= 0; --j) {
                    for (vertex_t u : h.layers[j]) {
                        weights[u] = weight( positions, u, h.g.out_neighbours(u) );
                    }
                    std::sort(h.layers[j].begin(), h.layers[j].end(), [&weights] (const auto& u, const auto& v) {
                        return weights[u] < weights[v];
                    });
                    update_positions(positions, h.layers[j]);
                }

            }

            int cross = count_crossings(h, positions);
            //std::cout << "min cross " << min_cross << " X cross: " << cross << "\n";
            if (cross < min_cross) {
                //std::cout << "better\n";
                fails = 0;
                best_order = h.layers;
                min_cross = cross;
            } else {
                //std::cout << "worse\n";
                fails++;
            }

            if (fails >= forgiveness) {
                break;
            }
        }
        //std::cout << "iters: " << i << "\n";

#ifdef DEBUG_CROSSING
        std::cout << "CROSS AFTER: " << count_crossings(h, positions) << "\n";
#endif
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

    void update_positions(vertex_map<int>& positions, const std::vector<vertex_t>& layer) {
        int pos = 0;
        for (auto u : layer) {
            positions[u] = pos++;
        }
    }
};

} // namespace detail