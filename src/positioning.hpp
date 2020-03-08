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
    virtual vec2 run(const subgraph& g, const hierarchy& h, vec2 origin) = 0;
    virtual ~positioning() = default;
};

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
            if (x > width) {
                width = x;
            }
            y += attr.layer_dist;
        }
        return { width, y };
    }
};


} // namespace detail