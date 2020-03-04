#pragma once

/**
 * Data structures and useful functions that are needed everywhere.
 * File name is in progress.
 */

using vertex_t = unsigned;

struct edge {
    vertex_t tail, head;
};

inline edge reversed(edge e) { return {e.head, e.tail}; }

inline std::ostream& operator<<(std::ostream& out, edge e) {
    out << "(" << e.tail << ", " << e.head << ")";
    return out;
}

template <typename T> 
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

struct range {
    vertex_t range_end;

    range(vertex_t end) : range_end(end) {}

    struct iterator {
        vertex_t val = 0;
        iterator(vertex_t val) : val(val) {}

        vertex_t operator*() { return val; }
        friend bool operator==(const iterator& lhs, const iterator& rhs) { return lhs.val == rhs.val; }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) { return !(lhs == rhs); }
        iterator& operator++() { val++; return *this; }
    };
    
    iterator begin() { return iterator(0); }
    iterator end() { return iterator(range_end); }
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
            , nd_beg(nd_beg) {}

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
    iterator end() { return iterator(std::end(second), std::end(first), std::begin(second)); }
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
