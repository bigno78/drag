#pragma once

#include <drag/layout.hpp>
#include <drag/drawing/svg.hpp>

#include <string>
#include <map>
#include <tuple>

namespace drag {

struct drawing_options {
    std::map<drag::vertex_t, std::string> labels;
    std::map<drag::vertex_t, std::string> colors;
    std::map<std::pair<drag::vertex_t, drag::vertex_t>, std::string> edge_colors;
    float font_size = 12;
    float margin = 10;
    bool use_labels = true;

    static drawing_options from_colors(const std::map<drag::vertex_t, std::string>& colors) {
        drawing_options opts;
        opts.colors = colors;
        return opts;
    }
};

namespace detail {

void draw_edge(svg_image& img, const path& p, const drawing_options& opts, float arrow_size) {
    // get the color
    auto it = opts.edge_colors.find( {p.from, p.to} );
    const auto& color = it == opts.edge_colors.end() ? "black" : it->second;

    // draw the lines
    img.draw_polyline(p.points, color);

    // draw the arrow
    img.draw_arrow(p.points[p.points.size() - 2], p.points.back(), arrow_size, color);
    if (p.bidirectional) {
        img.draw_arrow(p.points[1], p.points.front(), arrow_size, color);
    }
}

} // namespace detail

svg_image draw_svg_image(const drag::sugiyama_layout& l, const drawing_options& opts) {
    svg_image img(l.dimensions(), opts.margin);

    for (const auto& node : l.vertices()) {
        const auto& label = opts.labels.count(node.u) ? opts.labels.at(node.u) : std::to_string(node.u);
        const auto& color = opts.colors.count(node.u) ? opts.colors.at(node.u) : "black"; 
        img.draw_circle(node.pos, l.attribs().node_size, color);
        img.draw_text(node.pos, label, opts.font_size, color);
    }

    float arrow_size = 0.4 * l.attribs().node_size;
    for (const auto& path : l.edges()) {
        detail::draw_edge(img, path, opts, arrow_size);
    }

    return img;
}

svg_image draw_svg_image(const graph& g, const drawing_options& opts) {
    sugiyama_layout layout(g);
    return draw_svg_image(layout, opts);
}


void draw_svg_file(const std::string& filename,
                   const graph& g,
                   const drawing_options& opts)
{
    auto img = draw_svg_image(g, opts);
    img.save(filename);
}

} // namespace drag
