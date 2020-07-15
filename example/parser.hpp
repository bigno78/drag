#pragma once

#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <stdexcept>

#include "graph.hpp"
#include "types.hpp"
#include "svg.hpp"

float to_pt(float x) {
	return x*72;
}

float pt_to_int(float x) {
    return x/72;
}

bool contains(const std::map<std::string, vertex_t>& nodes, const std::string& n) {
    return nodes.count(n) > 0;
}

std::string read_word(std::istream& in) {
    std::string word;
    while( in && std::isalnum(in.peek()) ) {
        word.push_back( in.get() );
    }

    if (word.empty()) {
        throw std::invalid_argument("Expected a word.");
    }

    return word;
}

float read_float(std::istream& in) {
    float x;
    in >> x;
    if (!in) {
        throw std::invalid_argument("Expected a float.");
    }
    return x;
}

graph parse(const std::string& file, attributes& attr, drawing_options& opts) {
    std::ifstream in(file);

    if (!in) {
        //std::cerr << "Failed to open '" << file << "'\n";
        throw std::invalid_argument("Failed to open '" + file + "'.");
    }

    graph g;
    std::map<std::string, vertex_t> nodes;

    std::string line;
    while ( std::getline(in, line) ) {
        std::stringstream line_stream(line);

        line_stream >> std::ws;
        if (!std::isalpha(line_stream.peek()))
            continue;
        std::string first = read_word(line_stream);
        line_stream >> std::ws;

        char a = line_stream.get();
        if (a == '=') {
            line_stream >> std::ws;
            if (first == "ranksep") {
                attr.layer_dist = to_pt(read_float(line_stream));
            } else if (first == "nodesep") {
                attr.node_dist = to_pt(read_float(line_stream));
            } else if (first == "nodesize") {
                attr.node_size = to_pt(read_float(line_stream));
            } else if (first == "fontsize") {
                opts.font_size = read_float(line_stream);
            } else if (first == "loopangle") {
                attr.loop_angle = read_float(line_stream);
            } else if (first == "loopsize") {
                attr.loop_size = to_pt(read_float(line_stream));
            }
        } else if (a == '-' && line_stream.get() == '>') {
            line_stream >> std::ws;
            std::string second = read_word(line_stream);
            //std::cout << first << " -> " << second << "\n";

            if (!contains(nodes, second)) {
                auto u = g.add_node();
                nodes.insert( { second, u } );
                opts.labels.insert( { u, second } );  
            }
            if (!contains(nodes, first)) {
                auto u = g.add_node();
                nodes.insert( { first, u } );
                opts.labels.insert( { u, first } );   
            }

            g.add_edge(nodes[first], nodes[second]);
        } else if (!line_stream || a == ';') {
            if (!contains(nodes, first)) {
                auto u = g.add_node();
                nodes.insert( { first, u } );
                opts.labels.insert( { u, first } );   
            }
        }
    }
    return g;
}
