#pragma once

#include <fstream>
#include <map>

#include "vec2.hpp"
#include "layout.hpp"

class svg_img {
    std::ofstream file;

public:
    svg_img(const std::string& filename) : file(filename) {
        file << "<svg xmlns=\"http://www.w3.org/2000/svg\">\n";
    }

    svg_img(const std::string& filename, vec2 dims) : file(filename) {
        file << "<svg xmlns=\"http://www.w3.org/2000/svg\"\n";
        file << "\txmlns:xlink=\"http://www.w3.org/1999/xlink\"\n";
        file << "\txmlns:ev=\"http://www.w3.org/2001/xml-events\"\n";
        file << "\twidth=\"" << dims.x << "\" height=\"" << dims.y << "\"\n";
        file << "\tviewBox=\"0.00 0.00 " << dims.x << " " << dims.y << "\">\n";
        file << "<rect width=\"" << dims.x << "\" height=\"" << dims.y << "\" fill=\"green\" />";
    }

    ~svg_img() { file << "</svg>\n"; }

    void draw_line(vec2 start, vec2 end, const std::string& color="black") {
        file << "<line ";
        file << "x1=\"" << start.x << "\" ";
        file << "y1=\"" << start.y << "\" ";
        file << "x2=\"" << end.x << "\" ";
        file << "y2=\"" << end.y << "\" ";
        file << "stroke=\"" << color << "\" ";
        file << "/>\n";
    }

    void draw_circle(vec2 center, float r, const std::string& color="black") {
        file << "<circle ";
        file << "cx=\"" << center.x << "\" ";
        file << "cy=\"" << center.y << "\" ";
        file << "r=\"" << r << "\" ";
        file << "stroke=\"" << color << "\" ";
        file << "stroke-width=\"2\" ";
        file << "fill=\"white\" ";
        file << "/>\n";
    }

    void draw_text(vec2 pos, const std::string& text, const std::string& color="black") {
        file << "<text ";
        file << "x=\"" << pos.x << "\" ";
        file << "y=\"" << pos.y << "\" ";
        file << "fill=\"" << color << "\" ";
        file << "dominant-baseline=\"middle\" ";
        file << "text-anchor=\"middle\" ";
        file << "style=\"font-size: 14px; font-family: Times,serif;\" ";
        file << ">";
        file << text;
        file << "</text>\n";
    }

    void draw_polygon(const std::vector<vec2>& points, const std::string& color = "black") {
        file << "<polygon ";
        file << "points=\"";
        for (auto p : points) {
            file << p.x << "," << p.y << " ";
        }
        file << "\" ";
        file << "stroke=\"" << color << "\" ";
        file << "/>\n";
    }
};

void draw_arrow(svg_img& img, vec2 from, vec2 to, float size) {
    vec2 dir = from - to;
    dir = normalized(dir);
    img.draw_polygon( { to, to + size * rotate(dir, 25), to + size * rotate(dir, -25) } );
}


void draw_to_svg( svg_img& img, 
                  const sugiyama_layout& l,
                  const std::map<vertex_t, std::string>& labels,
                  vec2 start = {0,0} ) 
{
    for (const auto& node : l.vertices()) {
        img.draw_circle(start + node.pos, node.size);
        img.draw_text(start + node.pos, labels.at( node.u ) );
    }

    for (const auto& e : l.edges()) {
        vec2 prev = *e.points.begin();
        for (auto pos : e.points) {
            img.draw_line(start + prev, start + pos);
            prev = pos;
        }
        draw_arrow(img, start + e.points[e.points.size() - 2], start + e.points.back(), 10);
    }
}


void draw_to_svg(svg_img& img, const sugiyama_layout& l, vec2 start = {0,0}) {
    for (const auto& node : l.vertices()) {
        img.draw_circle(start + node.pos, node.size);
        img.draw_text(start + node.pos, node.default_label );
    }

    for (const auto& e : l.edges()) {
        vec2 prev = *e.points.begin();
        for (auto pos : e.points) {
            img.draw_line(start + prev, start + pos);
            prev = pos;
        }
        draw_arrow(img, start + e.points[e.points.size() - 2], start + e.points.back(), 5);
    }
}
