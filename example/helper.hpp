#pragma once

#include <chrono>
#include <vector>
#include <dirent.h>
#include <iostream>
#include <string>

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

std::vector< std::string > dir_contents(const std::string& path, const std::string file_extension="") {
    std::vector< std::string > contents;
    DIR *dir;
    struct dirent *ent;
    if ( (dir = opendir(path.c_str())) != NULL ) {
        
        while ( (ent = readdir(dir)) != NULL ) {
           
            std::string name{ ent->d_name };
            if (name == "." || name == ".." || !ends_with(name, file_extension)) {
                continue;
            }
            contents.push_back(name);
        }
        closedir(dir);

    } else {
        std::cerr << "cant open dir: " << path << "\n";
    }

    return contents;
}
