#pragma once

#include <chrono>
#include <vector>
#include <dirent.h>
#include <string>

auto now() {
    return std::chrono::high_resolution_clock::now();
}

template<typename T>
uint64_t to_micro(const T& dur) {
    return std::chrono::duration_cast<std::chrono::microseconds>( dur ).count();
}

std::vector< std::string > dir_contents(const std::string& path) {
    std::vector< std::string > contents;
    DIR *dir;
    struct dirent *ent;
    if ( (dir = opendir(path.c_str())) != NULL ) {
        
        while ( (ent = readdir(dir)) != NULL ) {
           
            std::string name{ ent->d_name };
            if (name == "." || name == "..") {
                continue;
            }
            contents.push_back(name);
        }
        closedir (dir);

    } else {
        perror (path.c_str());
    }

    return contents;
}
