#pragma once

#include <algorithm>

#include "layering.hpp"
#include "stuff.hpp"

namespace detail {

struct crossing_reduction {
    virtual void run(hierarchy& h) = 0;
    virtual ~crossing_reduction() = default;
};


struct barycentric_heuristic : public crossing_reduction {
    unsigned max_iters = 25;

    void run(hierarchy& h) override {
        vertex_flags<float> weights(h.g);
        vertex_flags<int> positions(h.g);

        for (auto l : h.layers) {
            int pos = 0;
            for (auto u : l) {
                positions[u] = pos;
                ++pos;
            }
        }

        for (int i = 0; i < max_iters; ++i) {
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
        }
    }

private:
    template<typename T>
    float weight(const vertex_flags<int>& positions, vertex_t u, const T& neighbours) {
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

    void update_positions(vertex_flags<int>& positions, const std::vector<vertex_t>& layer) {
        int pos = 0;
        for (auto u : layer) {
            positions[u] = pos++;
        }
    }
};

} // namespace detail