#pragma once

#include "stuff.hpp"
#include "subgraph.hpp"
#include "vec2.hpp"
#include "layering.hpp"

namespace detail {

struct positioning_attributes {
    float node_dist;
    float layer_dist;
};

struct positioning {
    virtual void run(const subgraph& g, const hierarchy& h) = 0;
    virtual ~positioning() = default;
};

struct test_positioning : public positioning {
    std::vector<vec2>& positions;
    positioning_attributes attr;

    test_positioning(std::vector<vec2>& positions, positioning_attributes attr)
        : positions(positions)
        , attr(std::move(attr)) {}  

    void run(const subgraph& g, const hierarchy& h) override {
        float y = attr.layer_dist;
        for (auto layer : h.layers) {
            float x = attr.node_dist;
            for (auto u : layer) {
                x += g.node_size(u);
                positions[u] = { x, y };
                x += g.node_size(u) + attr.node_dist;
            }
            y += attr.layer_dist;
        }
    }
};


} // namespace detail