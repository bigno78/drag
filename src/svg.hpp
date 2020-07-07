#pragma once

#include <fstream>
#include <map>

#include "vec2.hpp"
#include "layout.hpp"

class svg_img {
    std::ofstream file;

public:

    svg_img(const std::string& filename, vec2 dims) : file(filename) {
        file << "<svg xmlns=\"http://www.w3.org/2000/svg\"\n";
        file << "\txmlns:xlink=\"http://www.w3.org/1999/xlink\"\n";
        file << "\txmlns:ev=\"http://www.w3.org/2001/xml-events\"\n";
        file << "\twidth=\"" << dims.x << "pt\" height=\"" << dims.y << "pt\"\n";
        file << "\tviewBox=\"0.00 0.00 " << dims.x << " " << dims.y << "\">\n";
        file << "<rect width=\"" << dims.x << "\" height=\"" << dims.y << "\" fill=\"white\" stroke=\"transparent\" />";
    }

    ~svg_img() { file << "</svg>\n"; }

    void draw_polyline(std::vector<vec2> points, const std::string& color="black") {
        file << "<polyline ";
        file << "points=\"";
        const char* sep = "";
        for (auto [ x, y ] : points) {
            file << sep << x << " " << y;
            sep = " ";
        }
        file << "\" stroke=\"" << color << "\" ";
        file << "fill=\"none\" ";
        file << "/>\n";
        for (int i = 1; i < points.size() - 1; ++i) {
            draw_circle(points[i], 1, "red");
        }
    }

    void draw_circle(vec2 center, float r, const std::string& color="black") {
        file << "<circle ";
        file << "cx=\"" << center.x << "\" ";
        file << "cy=\"" << center.y << "\" ";
        file << "r=\"" << r << "\" ";
        file << "stroke=\"" << color << "\" ";
        file << "stroke-width=\"1\" ";
        file << "fill=\"white\" ";
        file << "/>\n";
    }

    void draw_text(vec2 pos, const std::string& text, float size, const std::string& color="black") {
        file << "<text ";
        file << "x=\"" << pos.x << "\" ";
        file << "y=\"" << pos.y << "\" ";
        file << "fill=\"" << color << "\" ";
        file << "dominant-baseline=\"middle\" ";
        file << "text-anchor=\"middle\" ";
        file << "style=\"font-size: " << size << "; font-family: Times,serif;\" ";
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
    img.draw_polygon( { to, to + size * rotate(dir, 20), to + size * rotate(dir, -20) } );
}


void draw_to_svg(svg_img& img,
                 std::vector<node> nodes, std::vector<path> paths,
                 float font_size,
                 float node_size,
                 const std::map<vertex_t, std::string>* lbls = nullptr)
{
    //float font_siz = 0.5*node_size;
    for (const auto& node : nodes) {
        img.draw_circle(node.pos, node.size);
        img.draw_text(node.pos, lbls ? lbls->at(node.u) : node.default_label, font_size );
    }

    float arrow_size = node_size*0.4;
    for (const auto& path : paths) {
        img.draw_polyline(path.points);
        draw_arrow(img, path.points[path.points.size() - 2], path.points.back(), arrow_size);
        if (path.bidirectional) {
            draw_arrow(img, path.points[1], path.points.front(), arrow_size);
        }
    }
}

void draw_to_svg(const std::string& file, const sugiyama_layout& l, float font_size=14) {
    svg_img img(file, l.dimensions());
    draw_to_svg(img, l.vertices(), l.edges(), font_size, l.get_attributes().node_size);
}

void draw_to_svg(const std::string& file, const sugiyama_layout& l, const std::map<vertex_t, std::string>& lbls, float font_size=14) {
    svg_img img(file, l.dimensions());
    draw_to_svg(img, l.vertices(), l.edges(), font_size, l.get_attributes().node_size, &lbls);
}
