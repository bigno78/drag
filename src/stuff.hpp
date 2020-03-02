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

struct range {
    vertex_t end;

    range(vertex_t end) : end(end) {}

    struct iterator {
        vertex_t val = 0;
        iterator(vertex_t val) : val(val) {}

        vertex_t operator*() { return val; }
        friend bool operator==(const iterator& lhs, const iterator& rhs) { lhs.val == rhs.val; }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) { !(lhs == rhs); }
        iterator& operator++() { val++; return *this; }
    };
    
    iterator begin() { return iterator(0); }
    iterator end() { return iterator(end); }
};