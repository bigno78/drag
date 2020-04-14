#pragma once

#include <string>
#include <vector>

#include "vec2.hpp"

using vertex_t = unsigned;

struct node {
    vertex_t u;
    vec2 pos;
    float size;
    std::string default_label;
};

struct link {
    vertex_t from, to;
    std::vector< vec2 > points;
};

namespace defaults {
    const float margin = 10;
    const float layer_dist = 60;
    const float node_dist = 20;
    const float node_size = 25;
    const float loop_size = 30;

    inline int iters = 1;
    inline int forgv = 4;
    inline bool trans = true;
}
