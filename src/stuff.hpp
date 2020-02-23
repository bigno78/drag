#pragma once

/**
 * Data structures and useful functions that are needed everywhere.
 * File name is in progress.
 */

using vertex_t = unsigned;

struct node {
    float widht, height;
};

struct edge {
    vertex_t tail, head;
};

inline edge reversed(edge e) { return {e.head, e.tail}; }

inline std::ostream& operator<<(std::ostream& out, edge e) {
    out << "(" << e.tail << ", " << e.head << ")";
    return out;
}