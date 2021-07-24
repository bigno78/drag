#pragma once

#include <drag/types.hpp>
#include <drag/detail/subgraph.hpp>

#include <vector>
#include <cmath>
#include <optional>
#include <algorithm>

namespace drag {

namespace detail {

struct edge_router {
    virtual void run(hierarchy& h, const rev_edges& rev) = 0;
    virtual ~edge_router() = default;
};

class router : public edge_router {
    const float min_sep = 5;
    const float loop_angle_sep = 5;
    float loop_height;
    float min_shift;
    
    const attributes& attr;
    std::vector<node>& nodes;
    std::vector<path>& links;

    vertex_map< std::array< float, 4 > > angles;
    vertex_map< std::array< float, 4 > > bound;

    vertex_map< std::array< float, 4> > shifts;

    vertex_map< bool > loop;

public:
    router(std::vector<node>& nodes, std::vector<path>& paths, const attributes& attr) 
        : attr(attr)
        , nodes(nodes)
        , links(paths) {}

    void run(hierarchy& h, const rev_edges& rev) override {
        angles.init( h.g, { 90, 90, 90, 90 } );
        bound.init( h.g, { 0 } );
        shifts.init( h.g, { 0 } );
        min_shift = attr.node_size/2;
        loop_height = attr.node_size/2;
        loop.init(h.g, false);

        for (auto u : rev.loops) {
            loop.set(u, true);
            make_loop_square(h.g, u);
        }

        //set_dummy_shifts(h);

        for (const auto l : h.layers) {
            for (int d : { 1, -1 }) {
                for (auto u : l) {
                    for (auto v : h.g.out_neighbours(u)) {
                        if (!h.g.is_dummy(u) || !h.g.is_dummy(v)) set_regular_shifts2(h, {u, v});
                    }
                }
            }
        }

        /*for (auto u : h.g.vertices()) {
            for (auto v : h.g.out_neighbours(u)) {
                if (!h.g.is_dummy(u) || !h.g.is_dummy(v)) set_regular_shifts2(h, {u, v});
            }
        }*/

        unify_dummy_shifts(h);

        for (auto u : h.g.vertices()) {
            for (auto v : h.g.out_neighbours(u)) {
                if (!h.g.is_dummy(u)) make_path(h, rev, u, v);
            }
        }
    }

private:

    void set_regular_shifts2(const hierarchy& h, edge e) {
        auto dirs = get_dirs(e);
        if (dirs.x == 0)
            return;
        set_reg_shift2(h, e, dirs);
    }


    void set_reg_shift2(const hierarchy& h, edge e, vec2 dirs) {
        check_loop(e);
        check_loop(reversed(e));
        
        auto up = next_non_dummy(h, e.from, dirs.x);
        auto down = next_non_dummy(h, e.to, -dirs.x);
        auto up_d = next_vertex(h, e.from, dirs.x);
        auto down_d = next_vertex(h, e.to, -dirs.x);

        set_shift2(h, e, up, down, dirs);
        set_shift2(h, e, up, down_d, dirs);
        set_shift2(h, e, up_d, down, dirs);
        set_shift2(h, e, up_d, down_d, dirs);
    }

    void set_shift2(const hierarchy& h, edge e, std::optional<vertex_t> up, std::optional<vertex_t> down, vec2 dirs) {
        auto from = get_center(e.from, dirs);
        auto to = get_center(e.to, -dirs);
        
        vec2 c_up = pos(e.from) - dirs.x*10;
        float r_up = 0;
        if (up) {
            c_up = pos(*up);
            r_up = h.g.is_dummy(*up) ? shifts[*up][angle_idx(dirs)] : attr.node_size;
        }

        vec2 c_down = pos(e.from) - dirs.x*10;
        float r_down = 0;
        if (down) {
            c_down = pos(*down);
            r_down = h.g.is_dummy(*down) ? shifts[*down][angle_idx(-dirs)] : attr.node_size;
        }

        bool can_inter_up = sgn(c_up.x - from.x) != sgn(c_up.x - to.x);
        bool can_inter_down = sgn(c_down.x - from.x) != sgn(c_down.x - to.x);

        float node_size = attr.node_size;

        // if it is possible for the edge to intersect the vertex
        if ( can_inter_up || can_inter_down ) {
            float s = get_shift(e.from, dirs);
            float t = get_shift(e.to, -dirs);

            bool up_done = true, down_done = true;
            while (true) {
                if (can_inter_up && s <= node_size && line_point_dist(from, to, c_up) <= r_up + min_sep) {
                    s += 5;
                    from = pos(e.from) + vec2{ 0, dirs.y*s };
                    up_done = false;
                } else {
                    up_done = true;
                }

                if (can_inter_down && t <= node_size && line_point_dist(to, from, c_down) <= r_down + min_sep) {
                    t += 5;
                    to = pos(e.to) + vec2{ 0, -dirs.y*t };
                    down_done = false;
                } else {
                    down_done = true;
                }

                if (up_done && down_done)
                    break;
            }

            // clip it
            if (s > node_size) s = node_size;
            if (t > node_size) t = node_size;

            shifts[e.from][angle_idx(dirs)] = std::max(shifts[e.from][angle_idx(dirs)], s);
            shifts[e.to][angle_idx(-dirs)] = std::max(shifts[e.to][angle_idx(-dirs)], t);
        }
    }

    float get_shift(edge e) { return get_shift(e.from, get_dirs(e)); }
    float get_shift(vertex_t u, vec2 dirs) { return get_shift(u, angle_idx(dirs)); }
    float get_shift(vertex_t u, int quadrant) { return shifts[u][quadrant]; }

    void unify_dummy_shifts(const hierarchy& h) {
        for (int layer_idx = 0; layer_idx < h.size(); ++layer_idx) {
            const auto& l = h.layers[layer_idx];
            int j = -1;
            for (int i = 0; i < l.size(); ++i) {
                if ( !h.g.is_dummy(l[i]) ) {
                    calculate_block_shifts(h, layer_idx, j + 1, i - 1);
                    j = i; 
                }
            }
            calculate_block_shifts(h, layer_idx, j + 1, l.size() - 1);
        }
    }

    void calculate_block_shifts(const hierarchy& h, int layer, int start, int end) {
        if (end < start)
            return;
        
        const auto& l = h.layers[layer];

        std::array<float, 4> s = { 0 };
        for (int i = start; i <= end; ++i) {
            vertex_t u = l[i];
            assert( h.g.is_dummy(u) );

            s[0] = std::max(s[0], shifts[u][0]);
            s[1] = std::max(s[1], shifts[u][1]);
            s[2] = std::max(s[2], shifts[u][2]);
            s[3] = std::max(s[3], shifts[u][3]);
        }

        for (int i = start; i <= end; ++i) {
            shifts[l[i]][0] = s[0];
            shifts[l[i]][1] = s[1];
            shifts[l[i]][2] = s[2];
            shifts[l[i]][3] = s[3];
        }

        fix_reg_ends(h, layer, start, end);
    }

    void fix_reg_ends(const hierarchy& h, int layer, int start, int end) {
        const auto& l = h.layers[layer];

        if (h.has_prev(l[start])) {
            auto u = h.prev(l[start]);
            for (auto v : h.g.neighbours(u)) {
                auto dirs = get_dirs(edge{u, v});
                if (dirs.x != 0)
                    set_reg_shift2(h, { u, v }, dirs);
            }
        }

        if (h.has_next(l[end])) {
            auto u = h.next(l[end]);
            for (auto v : h.g.neighbours(u)) {
                auto dirs = get_dirs(edge{u, v});
                if (dirs.x != 0)
                    set_reg_shift2(h, { u, v }, dirs);
            }
        }
    }

    void check_loop(edge e) {
        auto dirs = get_dirs(e);
        if (!loop.at(e.from) || angle_idx(dirs) == 2 || angle_idx(dirs) == 3)
            return;

        float s = get_shift(e.from, dirs);
        auto from = get_center(e.from, dirs);
        auto to = get_center(e.to, -dirs);
        auto intersection = *edge_intersects(from, to, nodes[e.from].pos, nodes[e.from].size);
        float a = angle(pos(e.from), intersection);

        if (a > attr.loop_angle - loop_angle_sep) {
            a = attr.loop_angle - loop_angle_sep;
            auto p = angle_point(a, e.from, dirs);

            float t = (pos(e.from).x - p.x)/(p.x - to.x);
            s = fabs( pos(e.from).y - (p.y + t*(p.y - to.y)) );

            assert(s >= 0 && s <= nodes[e.from].size);

            shifts[e.from][angle_idx(dirs)] = s;
        }
    }

    vec2 get_center(vertex_t u, vec2 dirs) {
        return nodes[u].pos + vec2{ 0, dirs.y*get_shift(u, dirs) };
    }

    float centered_angle(edge e) {
        auto dir = pos(e.to) - pos(e.from);
        return to_degrees( std::atan(fabs(dir.x)/fabs(dir.y)) );
    }

    float angle(vec2 from, vec2 to) {
        auto dir = to - from;
        return to_degrees( std::atan(fabs(dir.x)/fabs(dir.y)) );
    }


    int shift_idx(vec2 dirs) const {
        return dirs.y == 1;
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
    int angle_idx(vec2 dirs) const {
        if (dirs.x == 1)
            return dirs.y != 1;
        return 2 + (dirs.y == 1);
    }

    vec2 pos(vertex_t u) const { return nodes[u].pos; }
    vec2 get_dirs(vec2 v) const { return { sgn(v.x), sgn(v.y) }; }
    vec2 get_dirs(edge e) const { return get_dirs(pos(e.to) - pos(e.from)); }
    int get_xdir(edge e) const { return sgn(pos(e.to).x - pos(e.from).x); }

    // finds next vertex in the desired direction which is not a dummy vertex
    std::optional<vertex_t> next_non_dummy(const hierarchy& h, vertex_t u, int d) {
        int i = h.pos[u] + d;
        while (h.valid_pos(h.ranking[u], i) && h.g.is_dummy( h.layer(u)[i] )) {
            i += d;
        }

        if (h.valid_pos(h.ranking[u], i)) {
            return h.layer(u)[i];
        }
        return std::nullopt;
    }

    std::optional<vertex_t> next_vertex(const hierarchy& h, vertex_t u, int d) {
        if (d == -1 && h.has_prev(u)) {
            return h.prev(u);
        }
        if (d == 1 && h.has_next(u)) {
            return h.next(u);
        }
        return std::nullopt;
    }


    void make_path(hierarchy& h, const rev_edges& rev, vertex_t u, vertex_t v) {
        auto& g = h.g;
        path l{ u, v, {} };
        auto orig = edge{ u, v };

        l.points.push_back( calculate_port_shifted(u, nodes[v].pos - nodes[u].pos) );

        while (g.is_dummy(v)) {
            auto s = shifts[v][angle_idx(get_dirs(edge{v, u}))];
            if (s > 0)
                l.points.push_back( nodes[v].pos + vec2{ 0, -s } );
            
            l.points.push_back( nodes[v].pos );

            auto n = next(g, v);
            s = shifts[v][angle_idx(get_dirs(edge{v, n}))];
            if (s > 0)
                l.points.push_back( nodes[v].pos + vec2{ 0, s } );

            u = v;
            v = n;
        }

        l.points.push_back( calculate_port_shifted(v, nodes[u].pos - nodes[v].pos) );
        l.to = v;

        if (rev.reversed.contains(orig)) {
            reverse(l);
        } else if (rev.collapsed.contains(orig)) {
            l.bidirectional = true;
        }

        links.push_back(std::move(l));
    }

    void reverse(path& l) {
        std::swap(l.from, l.to);
        for (int i = 0; i < l.points.size()/2; ++i) {
            std::swap(l.points[i], l.points[l.points.size() - i - 1]);
        }
    }

    // https://stackoverflow.com/questions/1073336/circle-line-segment-collision-detection-algorithm
    std::optional<vec2> edge_intersects(vec2 from, vec2 to, vec2 center, float r) {
        auto d = to - from;
        auto f = from - center;

        auto a = dot(d, d);
        auto b = 2*dot(f, d);
        auto c = dot(f, f) - r*r;

        float discriminant = b*b - 4*a*c;
        if (discriminant < 0) {
            return std::nullopt;
        }

        discriminant = sqrt(discriminant);
        float t1 = (-b - discriminant)/(2*a);
        float t2 = (-b + discriminant)/(2*a);

        if (t1 >= 0 && t1 <= 1) {
            return { from + t1*d };
        }

        if (t2 >= 0 && t2 <= 1) {
            return { from + t2*d };
        }

        return std::nullopt;
    }

    // http://geomalgorithms.com/a02-_lines.html
    float line_point_dist(vec2 from, vec2 to, vec2 p) {
        
        auto v = to - from;
        auto w = p - from;

        float c1 = dot(w,v);
        float c2 = dot(v,v);
        float t = c1 / c2;

        auto x = from + t*v;
        return distance(p, x);
    }

    void make_loop_square(subgraph& g, vertex_t u) {
        path l{ u, u, {} };
        l.points.resize(4);

        l.points[0] = angle_point(attr.loop_angle, u, { 1, -1 });
        l.points[3] = angle_point(attr.loop_angle, u, { 1, 1 });

        l.points[1] = vec2{ pos(u).x + attr.node_size + attr.loop_size/2, l.points[0].y };
        l.points[2] = vec2{ pos(u).x + attr.node_size + attr.loop_size/2, l.points[3].y };

        links.push_back(std::move(l));
    }

    vec2 angle_point(float angle, vertex_t u, vec2 dirs) {
        return pos(u) + vec2{ dirs.x * attr.node_size * std::sin(to_radians(angle)),
                              dirs.y * attr.node_size * std::cos(to_radians(angle)) };
    }

    vertex_t next(const subgraph& g, vertex_t u) { return *g.out_neighbours(u).begin(); }
    vertex_t prev(const subgraph& g, vertex_t u) { return *g.in_neighbours(u).begin(); }

    vec2 calculate_port_shifted(vertex_t u, vec2 dir) {
        vec2 dirs { sgn(dir.x), sgn(dir.y) };
        auto s = get_shift(u, dirs);
        if (s == attr.node_size) {
            return get_center(u, dirs);
        }
        auto center = get_center(u, dirs);
        return *edge_intersects(center, center + dir, pos(u), nodes[u].size);
    }

    vec2 calculate_port_centered(vertex_t u, vec2 dir) {
        return nodes[u].pos + nodes[u].size * normalized(dir);
    }

    vec2 calculate_port_single(vertex_t u, vec2 dir) {
        return nodes[u].pos + vec2{0, sgn(dir.y) * nodes[u].size};
    }

    vec2 calculate_port_arctan(vertex_t u, vec2 dir) {
        float angle = ( 1/(float)2 ) * ( std::atan( dir.x/256 ) );

        float x = nodes[u].size * std::sin(angle);
        float y = nodes[u].size * std::cos(angle);

        return nodes[u].pos + vec2{ x, sgn(dir.y)*y };
    }

};

} // namespace detail

} // namespace drag
