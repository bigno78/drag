#pragma once

#include<string>

#include "vec2.hpp"

using vertex_t = unsigned;

struct node {
    vertex_t u;
    vec2 pos;
    float size;
    std::string default_label;
};

namespace defaults {
    const float margin = 10;
}
