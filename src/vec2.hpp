#pragma once

#include <cmath>
#include <ostream>

struct vec2 {
    double x, y;

    template<typename T>
    friend vec2 operator*(T a, vec2 vec) { return { a*vec.x, a*vec.y}; }
    template<typename T>
    friend vec2 operator*(vec2 vec, T a) { return { a*vec.x, a*vec.y}; }
};

inline std::ostream& operator<<(std::ostream& out, vec2 v) {
    out << "[" << v.x << ", " << v.y << "]";
    return out;
}

inline vec2 operator+(vec2 lhs, vec2 rhs) { return { lhs.x + rhs.x, lhs.y + rhs.y }; }
inline vec2 operator+(float a, vec2 rhs) { return { a + rhs.x, a + rhs.y }; }
inline vec2 operator+(vec2 lhs, float a) { return { lhs.x + a, lhs.y + a }; }
inline vec2& operator+=(vec2& lhs, vec2 rhs) {
    lhs = lhs + rhs;
    return lhs;
}
inline vec2& operator+=(vec2& lhs, float a) {
    lhs = lhs + a;
    return lhs;
}
inline vec2 operator-(vec2 lhs, vec2 rhs) { return { lhs.x - rhs.x, lhs.y - rhs.y }; }
inline vec2 operator-(float a, vec2 rhs) { return { a - rhs.x, a - rhs.y }; }
inline vec2 operator-(vec2 lhs, float a) { return { lhs.x - a, lhs.y - a }; }
inline vec2& operator-=(vec2& lhs, vec2 rhs) {
    lhs = lhs - rhs;
    return lhs;
}
inline vec2& operator-=(vec2& lhs, float a) {
    lhs = lhs - a;
    return lhs;
}
inline float to_radians(float deg) { return (M_PI*deg)/180; }
inline float to_degrees(float rad) { return (180*rad)/M_PI; }
inline float magnitude(vec2 v) { return std::sqrt(v.x*v.x + v.y*v.y); }
inline vec2 normalized(vec2 v) { return { v.x/magnitude(v), v.y/magnitude(v) }; }

inline vec2 rotate(vec2 vec, float deg) {
    float rad = to_radians(deg);
    float sin = std::sin(rad);
    float cos = std::cos(rad);
    return { vec.x*cos + vec.y*sin, -vec.x*sin + vec.y*cos };
}
