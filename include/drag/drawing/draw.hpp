#pragma once

#include <drag/layout.hpp>
#include <drag/drawing/svg.hpp>

namespace drag {

struct drawing_options {
    std::map<drag::vertex_t, std::string> labels;
    std::map<drag::vertex_t, std::string> colors;
    float font_size = 12;
    float margin = 15;
    bool use_labels = true;

    static drawing_options from_colors(const std::map<drag::vertex_t, std::string>& colors) {
        drawing_options opts;
        opts.colors = colors;
        return opts;
    }
};

namespace detail {


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
        img.draw_polyline(path.points);
        img.draw_arrow(path.points[path.points.size() - 2], path.points.back(), arrow_size);
        if (path.bidirectional) {
            img.draw_arrow(path.points[1], path.points.front(), arrow_size);
        }
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
