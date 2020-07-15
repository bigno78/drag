#pragma once

#include <fstream>
#include <map>

#include "vec2.hpp"
#include "layout.hpp"

struct drawing_options {
    std::map<vertex_t, std::string> labels;
    float font_size = 12;
    bool use_labels = true;
    float margin = 15;
};

class svg_img {
    std::ofstream file;

public:

    svg_img(const std::string& filename, vec2 dims, float margin) : file(filename) {
        float w = dims.x + 2*margin;
        float h = dims.y + 2*margin;
        file << "<svg xmlns=\"http://www.w3.org/2000/svg\"\n";
        file << "\txmlns:xlink=\"http://www.w3.org/1999/xlink\"\n";
        file << "\txmlns:ev=\"http://www.w3.org/2001/xml-events\"\n";
        file << "\twidth=\"" << w << "pt\" height=\"" << h << "pt\"\n";
        file << "\tviewBox=\"0.00 0.00 " << w << " " << h << "\">\n";
        file << "<rect width=\"" << w << "\" height=\"" << h << "\" fill=\"white\" stroke=\"transparent\" />";
        file << "<g transform=\"scale(1 1) rotate(0) translate(" << margin << " " << margin << ")\">";
    }

    ~svg_img() { 
        file << "</g>\n";
        file << "</svg>\n";
    }

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
        file << "font-size=\"" << size << "\" font-family=\"Times,serif\" ";
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
                 const std::vector<node>& nodes,
                 const std::vector<path>& paths,
                 const drawing_options& opts)
{
    for (const auto& node : nodes) {
        img.draw_circle(node.pos, node.size);
        img.draw_text(node.pos, opts.use_labels ? opts.labels.at(node.u) : std::to_string(node.u), opts.font_size  );
    }

    
    for (const auto& path : paths) {
        float arrow_size = nodes[path.from].size * 0.4;
        img.draw_polyline(path.points);
        draw_arrow(img, path.points[path.points.size() - 2], path.points.back(), arrow_size);
        if (path.bidirectional) {
            draw_arrow(img, path.points[1], path.points.front(), arrow_size);
        }
    }
}

void draw_to_svg(const std::string& file, const sugiyama_layout& l, const drawing_options& opts) {
    svg_img img(file, l.dimensions(), opts.margin);
    for (const auto& node : l.vertices()) {
        img.draw_circle(node.pos, l.attribs().node_size);
        img.draw_text(node.pos, opts.use_labels ? opts.labels.at(node.u) : std::to_string(node.u), opts.font_size );
    }

    float arrow_size = 0.4 * l.attribs().node_size;
    for (const auto& path : l.edges()) {
        img.draw_polyline(path.points);
        draw_arrow(img, path.points[path.points.size() - 2], path.points.back(), arrow_size);
        if (path.bidirectional) {
            draw_arrow(img, path.points[1], path.points.front(), arrow_size);
        }
    }
}
