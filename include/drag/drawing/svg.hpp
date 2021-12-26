#pragma once

#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <array>
#include <stdexcept>

#include <drag/vec2.hpp>
#include <drag/layout.hpp>

namespace drag {


class svg_file;

class svg_image {
public:
    svg_image(vec2 size, float margin) 
        : m_size(size)
        , m_margin(margin) {}

    svg_image(vec2 size) : svg_image(size, 0) {}

    svg_image() : svg_image({0, 0}, 0) {};
    
    void draw_polyline(std::vector<drag::vec2> points, const std::string& color="black") {
        m_data << "<polyline ";
        m_data << "points=\"";
        
        const char* sep = "";
        for (auto [ x, y ] : points) {
            m_data << sep << x << " " << y;
            sep = " ";
        }
        
        m_data << "\" stroke=\"" << color << "\" ";
        m_data << "fill=\"none\" ";
        m_data << "/>\n";
    }

    void draw_circle(drag::vec2 center, float r, const std::string& color="black") {
        m_data << "<circle ";
        m_data << "cx=\"" << center.x << "\" ";
        m_data << "cy=\"" << center.y << "\" ";
        m_data << "r=\"" << r << "\" ";
        m_data << "stroke=\"" << color << "\" ";
        m_data << "stroke-width=\"1\" ";
        m_data << "fill=\"white\" ";
        m_data << "/>\n";
    }

    void draw_text(drag::vec2 pos, const std::string& text, float size, const std::string& color="black") {
        m_data << "<text ";
        m_data << "x=\"" << pos.x << "\" ";
        m_data << "y=\"" << pos.y << "\" ";
        m_data << "fill=\"" << color << "\" ";
        m_data << "dominant-baseline=\"middle\" ";
        m_data << "text-anchor=\"middle\" ";
        m_data << "font-size=\"" << size << "\" font-family=\"Times,serif\" ";
        m_data << ">";
        m_data << text;
        m_data << "</text>\n";
    }

    void draw_polygon(const std::vector<drag::vec2>& points, const std::string& color = "black") {
        m_data << "<polygon ";
        m_data << "points=\"";
        for (auto p : points) {
            m_data << p.x << "," << p.y << " ";
        }
        m_data << "\" ";
        m_data << "stroke=\"" << color << "\" ";
        m_data << "fill=\"" << color << "\" ";
        m_data << "/>\n";
    }

    void draw_arrow(drag::vec2 from, drag::vec2 to, float size, const std::string& color = "black") {
        auto dir = from - to;
        dir = normalized(dir);
        draw_polygon( { to, to + size * rotate(dir, 20), to + size * rotate(dir, -20) }, color );
    }

    void add_image(const svg_image& img) {
        m_data << "<g transform='translate(" << m_size.x + img.m_margin << ", " << img.m_margin << ")'>\n";
        m_data << img.m_data.str();
        m_data << "</g>\n";

        m_size.x += img.m_size.x + 2*img.m_margin; 
        m_size.y = std::max(m_size.y, img.m_size.y + 2*img.m_margin); 
    }

    void save(const std::string& filename) const {
        std::ofstream file(filename);

        if (!file) {
            throw std::invalid_argument("Failed to save svg immage. Cannot open file '" + filename + "'");
        }

        float w = m_size.x + 2*m_margin;
        float h = m_size.y + 2*m_margin;

        file << "<svg xmlns='http://www.w3.org/2000/svg'\n";
        file << "\twidth='" << w << "pt' height='" << h << "pt'\n";
        file << "\tviewBox='0 0 " << m_size.x + 2*m_margin << " " << m_size.y + 2*m_margin << "'>\n";
        
        // draw a background polygon
        file << "<polygon fill='white' stroke='transparent' ";
        file << "points='";
        file << "0,0 ";
        file << "0," << h << " ";
        file << w << "," << h << " ";
        file << w << ",0";
        file << "'/>\n";
        
        file << "<g transform='translate(" << m_margin << ", " << m_margin << ")'>\n";
        
        file << m_data.str();

        file << "</g>\n";

        file << "</svg>\n";
    }

private:
    std::stringstream m_data;
    vec2 m_size;
    float m_margin;

    friend svg_file;
};


class svg_file {
public:
    svg_file(const std::string& filename) : m_file(filename) {
        m_file << "<svg xmlns='http://www.w3.org/2000/svg'>\n";
    }

    ~svg_file() {
        m_file << "</svg>\n";
    }

    void add_image(const svg_image& img) {
        m_file << "<g transform='translate(" << m_size.x + img.m_margin << ", " << img.m_margin << ")'>\n";
        m_file << img.m_data.str();
        m_file << "</g>\n";

        m_size.x += img.m_size.x + 2*img.m_margin; 
        m_size.y = std::max(m_size.y, img.m_size.y + 2*img.m_margin); 
    }

private:
    std::ofstream m_file;
    vec2 m_size = { 0, 0 };
};


/*
} // namespace detail


void draw_to_svg(detail::svg_img& img,
                 const std::vector<drag::node>& nodes,
                 const std::vector<drag::path>& paths,
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

void draw_to_svg(const std::string& file, const drag::sugiyama_layout& l, const drawing_options& opts) {
    detail::svg_img img(file, l.dimensions(), opts.margin);
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
*/


} // namespace drag
