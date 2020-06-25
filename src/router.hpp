#pragma once

#include "types.hpp"
#include "subgraph.hpp"

#include <vector>
#include <cmath>
#include <optional>
#include <algorithm>


namespace detail {

struct edge_router {
    virtual void run(hierarchy& h, const rev_edges& rev) = 0;
    virtual ~edge_router() = default;
};

class router : public edge_router {
    std::vector<node>& nodes;
    std::vector<link>& links;

    vertex_map< std::array< float, 4 > > angles;
    vertex_map< std::array< float, 4 > > bound; 

public:
    router(std::vector<node>& nodes, std::vector<link>& links) : nodes(nodes), links(links) {}

    void run(hierarchy& h, const rev_edges& rev) override {
        angles.init(h.g, { 90, 90, 90, 90 });
        bound.init(h.g, { 0 });

        for (auto u : h.g.vertices()) {
            for (auto v : h.g.out_neighbours(u)) {
                if (!h.g.is_dummy(u) && !h.g.is_dummy(v)) update_angles(h, {u, v});
            }
        }

        for (auto u : h.g.vertices()) {
            for (auto v : h.g.out_neighbours(u)) {
                if (!h.g.is_dummy(u)) make_path(h, rev, u, v);
            }
        }

        for (auto u : rev.loops) {
            make_loop(h.g, u);
        }
    }

private:

    /**
     * Check if the edge <e> intersects any node.
     * If so, calculate the maximum angle without an intersection
     */
    void update_angles(const hierarchy& h, edge e) {
        auto dir = nodes[e.to].pos - nodes[e.from].pos;
        auto dirs = vec2{ sgn(dir.x), sgn(dir.y) };

        if (dirs.x != 0)
            check_intersection(h, e, dirs);

        dirs = -dirs;
        e = reversed(e);

        if (dirs.x != 0)
            check_intersection(h, e, dirs);
    }

    /**
     * Check if <e> intersects the node closes to <e.from>.
     * If it does, calculate the maximum angle without an interrsection.
     */
    void check_intersection(const hierarchy& h, edge e, vec2 dirs) {
        auto nxt = next_vertex(h, e.from, dirs.x);
        if (nxt) {
            auto a = find_angle(h, e, *nxt, dirs);
            if (a < angles[e.from][angle_idx(dirs)]) {
                angles[e.from][angle_idx(dirs)] = a;
                bound[e.from][angle_idx(dirs)] = pos(e.to).x;
                //std::cout << e << "\n"; 
            }
        }
    }

    /**
     * Find the maximum angle such that <e> does not intersect <u>.
     */
    float find_angle(const hierarchy& h, edge e, vertex_t u, vec2 dirs) {
        int a = 90;

        auto from = pos(e.from);
        auto to = pos(e.to);
        auto center = pos(u);
        auto r = nodes[u].size;

        while (a >= 0 && edge_intersects(from, to, center, r)) {
            a -= 5;

            float x = r * std::sin(to_radians(a));
            float y = r * std::cos(to_radians(a));

            from = pos(e.from) + vec2{ dirs.x*x, dirs.y*y };
        }

        if (a < 0) {
            return 0;
        }
        return a;
    }


    /**
     * Get the index of the quadrant <dirs> points to.
     * The indexes are as follows:
     * 
     *    2 | 1
     *   ---+--->
     *    3 | 0
     *      v
     */ 
    int angle_idx(vec2 dirs) {
        if (dirs.x == 1)
            return dirs.y != 1;
        return 2 + (dirs.y == 1);
    }

    vec2 pos(vertex_t u) { return nodes[u].pos; }

    // finds next vertex in the desired direction which is not a dummy vertex
    std::optional<vertex_t> next_vertex(const hierarchy& h, vertex_t u, int d) {
        int i = h.pos[u] + d;
        while (h.valid_pos(h.ranking[u], i) && h.g.is_dummy( h.layer(u)[i] )) {
            i += d;
        }

        if (h.valid_pos(h.ranking[u], i)) {
            return h.layer(u)[i];
        }
        return std::nullopt;
    }


    void make_path(hierarchy& h, const rev_edges& rev, vertex_t u, vertex_t v) {
        auto& g = h.g;
        link l{ u, v, {} };
        auto orig = edge{ u, v };
        //std::cout << u << " " << v << "\n";

        int dir = sgn(nodes[u].pos.y - nodes[v].pos.y);

        // add the first one
        l.points.push_back( calculate_port(u, nodes[v].pos - nodes[u].pos) );
/*
        if (g.is_dummy(v)) {
            
            if ( ( h.has_prev(v) && edge_intersects(pos(u), pos(v), pos(h.prev(v)), nodes[h.prev(v)].size) ) || 
                 ( h.has_next(v) && edge_intersects(pos(u), pos(v), pos(h.next(v)), nodes[h.next(v)].size) ) ) {
                //auto new_node = g.add_dummy();
                //nodes.push_back( { new_node, nodes[v].pos + vec2{0, -g.node_size()}, 0, "" } );
                nodes[v].pos = nodes[v].pos + vec2{0, -g.node_size()};
                l.points.push_back(nodes[v].pos);
                u = v;
                v = next(g, v);
            }
        }
*/
        while (g.is_dummy(v)) {
            l.points.push_back( nodes[v].pos );
            u = v;
            //std::cout << v << "\n";
            v = next(g, v);
        }
/*
        if (g.is_dummy(u)) {
            if ( ( h.has_prev(u) && edge_intersects(pos(u), pos(v), pos(h.prev(u)), nodes[h.prev(u)].size) ) || 
                 ( h.has_next(u) && edge_intersects(pos(u), pos(v), pos(h.prev(u)), nodes[h.prev(u)].size) ) ) {
                nodes[u].pos = vec2{ nodes[u].pos.x, h.layer_pos[h.ranking[u]] + g.node_size() };
                l.points.push_back(nodes[u].pos);
            }
        }
*/
        l.points.push_back( calculate_port(v, nodes[u].pos - nodes[v].pos) );

        if (rev.reversed.contains(orig)) {
            reverse(l);
        } else if (rev.collapsed.contains(orig)) {
            links.push_back(l);
            reverse(links.back());
        }

        links.push_back(std::move(l));
    }

    void reverse(link& l) {
        std::swap(l.from, l.to);
        for (int i = 0; i < l.points.size()/2; ++i) {
            std::swap(l.points[i], l.points[l.points.size() - i - 1]);
        }
    }

    bool edge_intersects(vec2 from, vec2 to, vec2 center, float r) {
        auto d = to - from;
        auto f = from - center;

        auto a = dot(d, d);
        auto b = 2*dot(f, d);
        auto c = dot(f, f) - r*r;

        float discriminant = b*b - 4*a*c;
        if (discriminant < 0) {
            return false;
        }

        discriminant = sqrt(discriminant);
        float t1 = (-b - discriminant)/(2*a);
        float t2 = (-b + discriminant)/(2*a);

        return (t1 >= 0 && t1 <= 1) || (t2 >= 0 && t2 <= 1);
    }

    void make_loop(subgraph& g, vertex_t u) {
        link l{ u, u, {} };
        l.points.resize(5);

        l.points[1] = nodes[u].pos + vec2{ nodes[u].size + defaults::loop_size/2, nodes[u].size };
        l.points[2] = nodes[u].pos + vec2{ nodes[u].size + defaults::loop_size, 0 };
        l.points[3] = nodes[u].pos + vec2{ nodes[u].size + defaults::loop_size/2, -nodes[u].size };

        l.points[0] = calculate_port_centered( u, l.points[1] - nodes[u].pos );
        l.points[4] = calculate_port_centered( u, l.points[3] - nodes[u].pos );

        links.push_back(std::move(l));
    }

    vertex_t next(const subgraph& g, vertex_t u) { return *g.out_neighbours(u).begin(); }

    // dir is the edge leaving from u
    vec2 calculate_port(vertex_t u, vec2 dir) {
        vec2 dirs { sgn(dir.x), sgn(dir.y) };
        auto a = angles[u][angle_idx(dirs)];

        if (a < 90) {
            std::cout << u << " angler:" << a << "\n";
            auto max_x = bound[u][angle_idx(dirs)];
            auto center_x = pos(u).x;

            float angle = a*abs(dir.x)/abs(center_x - max_x);
            std::cout << angle << "\n";

            angle = to_radians(angle);
            float x = dirs.x * nodes[u].size * std::sin(angle);
            float y = dirs.y * nodes[u].size * std::cos(angle);

            return nodes[u].pos + vec2{ x, y };
        }

        return calculate_port_centered(u, dir);
    }

    vec2 calculate_port_centered(vertex_t u, vec2 dir) {
        return nodes[u].pos + nodes[u].size * normalized(dir);
    }

    vec2 calculate_port_single(vertex_t u, vec2 dir) {
        return nodes[u].pos + vec2{0, sgn(dir.y) * nodes[u].size};
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
