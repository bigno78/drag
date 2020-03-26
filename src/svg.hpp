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
};

void draw_arrow(svg_img& img, vec2 from, vec2 to, float size) {
    vec2 dir = from - to;
    dir = normalized(dir);
    img.draw_line(to, to + size * rotate(dir, 45));
    img.draw_line(to, to + size * rotate(dir, -45));
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
        vec2 prev = *e.begin();
        for (auto pos : e) {
            img.draw_line(start + prev, start + pos);
            prev = pos;
        }
        draw_arrow(img, start + e[e.size() - 2], start + e.back(), 5);
    }
}


void draw_to_svg(svg_img& img, const sugiyama_layout& l, vec2 start = {0,0}) 
{
    for (const auto& node : l.vertices()) {
        img.draw_circle(start + node.pos, node.size);
        img.draw_text(start + node.pos, node.default_label );
    }

    for (const auto& e : l.edges()) {
        vec2 prev = *e.begin();
        for (auto pos : e) {
            img.draw_line(start + prev, start + pos);
            prev = pos;
        }
        draw_arrow(img, start + e[e.size() - 2], start + e.back(), 5);
    }
}
