#pragma once

#include "types.hpp"
#include "subgraph.hpp"

#include <vector>
#include <cmath>

// 1/2 arctan(x/8)

namespace detail {

struct edge_router {
    virtual void run(hierarchy& h) = 0;
    virtual ~edge_router() = default;
};

class router : public edge_router {
    const std::vector<node>& nodes;
    std::vector<link>& links;

public:
    router(const std::vector<node>& nodes, std::vector<link>& links) : nodes(nodes), links(links) {}

    void run(hierarchy& h) override {
        for (auto u : h.g.vertices()) {
            for (auto v : h.g.out_neighbours(u)) {
                if (u == v) {
                    links.push_back( make_loop(h.g, u) );
                } else {
                    links.push_back( make_path(h.g, u, v) );
                }
            }
        }
    }

    link make_path(subgraph& g, vertex_t u, vertex_t v) {
        link l{ u, v, {} };

        int dir = sgn(nodes[u].pos.y - nodes[v].pos.y);

        // add the first one
        l.points.push_back( calculate_port_arctan(u, nodes[v].pos - nodes[u].pos) );

        while (g.is_dummy(v)) {
            l.points.push_back( nodes[v].pos );
            u = v;
            v = next(g, v);
        }

        // add the last one
        l.points.push_back( calculate_port_arctan(v, nodes[u].pos - nodes[v].pos) );

        return l;
    }

    link make_loop(subgraph& g, vertex_t u) {
        link l{ u, u, {} };
        l.points.resize(5);

        l.points[1] = nodes[u].pos + vec2{ nodes[u].size + defaults::loop_size/2, nodes[u].size };
        l.points[2] = nodes[u].pos + vec2{ nodes[u].size + defaults::loop_size, 0 };
        l.points[3] = nodes[u].pos + vec2{ nodes[u].size + defaults::loop_size/2, -nodes[u].size };

        l.points[0] = calculate_port( u, l.points[1] - nodes[u].pos );
        l.points[4] = calculate_port( u, l.points[3] - nodes[u].pos );

        return l;
    }

    vertex_t next(const subgraph& g, vertex_t u) { return *g.out_neighbours(u).begin(); }

    vec2 calculate_port(vertex_t u, vec2 dir) {
        return nodes[u].pos + nodes[u].size * normalized(dir);

    }

    vec2 calculate_port_arctan(vertex_t u, vec2 dir) {
        float angle = ( 1/(float)2 ) * ( std::atan( dir.x/256 ) );
        //std::cout << "f(" << dir.x << ") = " << to_degrees(angle) << "\n";

        float x = nodes[u].size * std::sin(angle);
        float y = nodes[u].size * std::cos(angle);

        //std::cout << x << " " << y << "\n";

        return nodes[u].pos + vec2{ x, sgn(dir.y)*y };
    }
};

} // namespace detail
