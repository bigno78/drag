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

std::ostream& operator<<(std::ostream& out, vec2 v) {
    out << "[" << v.x << ", " << v.y << "]";
    return out;
}

vec2 operator+(vec2 lhs, vec2 rhs) { return { lhs.x + rhs.x, lhs.y + rhs.y }; }
vec2 operator-(vec2 lhs, vec2 rhs) { return { lhs.x - rhs.x, lhs.y - rhs.y }; }
float to_radians(float deg) { return (M_PI*deg)/180; }
float magnitude(vec2 v) { return std::sqrt(v.x*v.x + v.y*v.y); }
vec2 normalized(vec2 v) { return { v.x/magnitude(v), v.y/magnitude(v) }; }

vec2 rotate(vec2 vec, float deg) {
    float rad = to_radians(deg);
    float sin = std::sin(rad);
    float cos = std::cos(rad);
    return { vec.x*cos + vec.y*sin, -vec.x*sin + vec.y*cos };
}
