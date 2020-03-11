#pragma once

#include "stuff.hpp"
#include "subgraph.hpp"
#include "vec2.hpp"
#include "layering.hpp"

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
                std::cout << g.node_size(u);
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


} // namespace detail