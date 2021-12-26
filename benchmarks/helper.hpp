#pragma once

#include <chrono>

namespace drag {

auto now() {
    return std::chrono::steady_clock::now();
}

template<typename T>
uint64_t to_micro(const T& dur) {
    return std::chrono::duration_cast<std::chrono::microseconds>( dur ).count();
}

template<typename T>
uint64_t to_mili(const T& dur) {
    return std::chrono::duration_cast<std::chrono::milliseconds>( dur ).count();
}

} // namespace drag
