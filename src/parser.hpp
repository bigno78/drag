#pragma once

#include <fstream>
#include <string>
#include <map>
#include <sstream>

#include "graph.hpp"

bool contains(const std::map<std::string, vertex_t>& nodes, const std::string& n) {
    return nodes.count(n) > 0;
}

std::map<vertex_t, std::string> parse(const std::string& file, graph& g) {
    std::ifstream in(file);

    std::map<std::string, vertex_t> nodes;
    std::map<vertex_t, std::string> labels;

    std::string line;
    while ( std::getline(in, line) ) {
        std::stringstream line_stream(line);
        std::string tail, head, sep;

        line_stream >> tail >> sep >> head;
        if (!line_stream || sep != "->") {
            //std::cout << "discarding: " << line << "\n";
            continue;
        }
        if (head.back() == ';') {
            head.pop_back();
        }

        if (!contains(nodes, tail)) {
            auto u = g.add_node();
            nodes.insert( { tail, u } );
            labels.insert( { u, tail } );  
        }
        if (!contains(nodes, head)) {
            auto u = g.add_node();
            nodes.insert( { head, u } );
            labels.insert( { u, head } );   
        }

        //std::cout << "adding: " << tail << "[" << nodes[tail] << "] -> " << head << "[" << nodes[head] << "]\n";
        g.add_edge(nodes[tail], nodes[head]);
    }
    return labels;
}
