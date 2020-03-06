#pragma once

#include <fstream>

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
        file << "fill=\"white\" ";
        file << "/>\n";
    }

    void draw_text(vec2 pos, const std::string& text, const std::string& color="black") {
        file << "<text ";
        file << "x=\"" << pos.x << "\" ";
        file << "y=\"" << pos.y << "\" ";
        file << "stroke=\"" << color << "\" ";
        file << "dominant-baseline=\"middle\" ";
        file << "text-anchor=\"middle\" ";
        file << ">";
        file << text;
        file << "</text>\n";
    }
};

void draw_to_svg(const sugiyama_layout& l, const std::string& name) {
    svg_img img(name);

    for (const auto& e : l.edges()) {
        vec2 prev = *e.begin();
        for (auto pos : e) {
            img.draw_line(prev, pos);
            prev = pos;
        }
    }

    int i = 0;
    for (const auto& node : l.vertices()) {
        img.draw_circle(node.pos, node.size);
        img.draw_text(node.pos, std::to_string(i));
    }
}