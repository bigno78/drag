#pragma once

#include <chrono>
#include <vector>
#include <filesystem>
#include <iostream>
#include <string>

namespace drag {

auto now() {
    return std::chrono::high_resolution_clock::now();
}

template<typename T>
uint64_t to_micro(const T& dur) {
    return std::chrono::duration_cast<std::chrono::microseconds>( dur ).count();
}

bool ends_with(const std::string& str, const std::string& suffix) {
	if (str.size() < suffix.size()) return false;
	return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

/**
 * Return names of all regular files in the specified directory with the given extension.
 * 
 * The names don't contain the directory path.
 * If no extension or an empty extension is given, returns all regular files.
 * The extension string needs to start with a dot.
 * 
 * @param path            the path to the directory
 * @param file_extension  the extension to search for, starting with a dot
 * 
 * @return all file names in the directory with the extension
 */
std::vector< std::string > dir_contents(const std::string& path, const std::string file_extension="") {
    std::vector< std::string > contents;

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            if (file_extension == "" || entry.path().extension() == file_extension) {
                contents.push_back(entry.path().filename().string());
            }
        }
    }

    return contents;
}

} // namespace drag
