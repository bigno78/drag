#pragma once

#include <string>

#include "vec2.hpp"

/**
 * Data structures and useful functions that are needed everywhere.
 * File name is in progress.
 */

using vertex_t = unsigned;

struct edge {
    vertex_t tail, head;
};

inline bool operator==(edge lhs, edge rhs) { return lhs.head == rhs.head && lhs.tail == rhs.tail; }
inline bool operator!=(edge lhs, edge rhs) { return !(lhs == rhs); }
inline edge reversed(edge e) { return {e.head, e.tail}; }

inline std::ostream& operator<<(std::ostream& out, edge e) {
    out << "(" << e.tail << ", " << e.head << ")";
    return out;
}

struct node {
    vec2 pos;
    float size;
    std::string label;
};

template <typename T> 
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

template<typename T>
struct range {
    T  r_start;
    T r_end; 
    int step;

    range(T start, T end, int step) : r_start(start), r_end(end), step(step) {}

    struct iterator {
        T val;
        int step;
        iterator(T val, int step) : val(val), step(step) {}

        vertex_t operator*() { return val; }
        friend bool operator==(const iterator& lhs, const iterator& rhs) { return lhs.val == rhs.val; }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) { return !(lhs == rhs); }
        iterator& operator++() { val += step; return *this; }
    };
    
    iterator begin() const { return iterator(r_start, step); }
    iterator end() const { return iterator(r_end, step); }
};

template<typename T>
struct chain_range {
    const T& first;
    const T& second;

    chain_range(const T& first, const T& second) : first(first), second(second) {}
        
    struct iterator {
        using It = typename T::const_iterator;
        It curr;
        It st_end;
        It nd_beg;

        iterator(It curr, It st_end, It nd_beg)
            : curr(curr)
            , st_end(st_end)
            , nd_beg(nd_beg) 
        { 
            if (curr == st_end) {
                this->curr = nd_beg;
            }    
        }

        vertex_t operator*() { return *curr; }
        friend bool operator==(const iterator& lhs, const iterator& rhs) { return lhs.curr == rhs.curr; }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) { return !(lhs == rhs); }
        iterator& operator++() {
            if (++curr == st_end) {
                curr = nd_beg;
            }
            return *this;
        }
    };

    iterator begin() { return iterator(std::begin(first), std::end(first), std::begin(second)); }
    iterator begin() const { return iterator(std::begin(first), std::end(first), std::begin(second)); }
    iterator end() { return iterator(std::end(second), std::end(first), std::begin(second)); }
    iterator end() const { return iterator(std::end(second), std::end(first), std::begin(second)); }
};

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec) {
    const char* sep = "";
    for (const auto& x : vec) {
        out << sep << x;
        sep = ", ";
    }
    return out;
}
