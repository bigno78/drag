#include <fstream>
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
#include <vector>
#include <dirent.h>
#include <algorithm>

bool parse_label(int& lbl, std::istringstream& in) {
	char n;
	in >> n;
	if (!in || n != 'n') return false;

	in >> lbl;
	return static_cast<bool>(in);
}

int total_size(const std::string& file) {
	std::map<int, std::vector<int>> edges;
	std::map<int, int> levels; // y-coord -> level
	std::map<int, int> nodes; // node -> level

	std::ifstream in(file);
    if (!in) {
        std::cerr << "Layout parser: failed to open '" << file << "'\n";
    }

	std::string line;
	while(std::getline(in, line)) {
		std::istringstream ss(line);

		int u;
		if (!parse_label(u, ss)) {
			//std::cout << "discarding: " << line << "\n";
			continue;
		}

		std::string token;
		ss >> token;
		if (!ss) {
			//std::cout << "discarding: " << line << "\n";
			continue;
		}

		if (token == "->") {
			int v;
			if (!parse_label(v, ss)) {
				//std::cout << "discarding: " << line << "\n";
				continue;
			}

			edges[u].push_back(v);
			//std::cout << u << ", " << v << "\n";
		} else {
			if (!std::getline(in, line)) break;
			std::stringstream sss(line);
			//std::cout << sss.str() << "\n";
			char c = sss.peek();
			int i = 0;
			while(c != '-' && (c > '9' || c < '0')) {
				//std::cout << c << "\n";
				sss >> c;
				c = sss.peek();
				assert(i++ < 20);
			}
			
			double x, y;
			char comma;
			sss >> x;
			if (!sss) {
				//std::cout << sss.str() << "\n";
				assert(false);
			} 
			sss >> comma >> y;
			levels[static_cast<int>(y)] = 0;
			nodes[u] = static_cast<int>(y);
			//std::cout << u << ": " << y << "\n";
		}
	}

	int i = 0;
	for (auto& [ key, val ] : levels) {
		val = i++;
	}

	for (auto& [ key, val ] : levels) {
		//std::cout << key << " -> " << val << "\n";
	}

	for (auto& [ key, val ] : nodes) {
		val = levels[val];
	}

	/*std::cout << "-----------\n";
	for (auto& [ key, val ] : nodes) {
		std::cout << key << " -> " << val << "\n";
	}*/

	int size = 0;
	for (auto& [ key, val ] : edges) {
		for (auto x : val) {
			size += std::abs( nodes[key] - nodes[x] );
		}
	}

	return size;
}

bool ends_with(const std::string& str, const std::string& suffix) {
	if (str.size() < suffix.size()) return false;
	return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

std::vector< std::string > dir_contents(const std::string& path, const std::string file_extension) {
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
        closedir (dir);

    } else {
        std::cerr << "cant open dir: " << path << "\n";
    }

    return contents;
}

void print_usage() {
	std::cout << "usage: source_file\n";
}

int main(int argc, char** argv) {
	if (argc != 2) {
		print_usage();
		return 1;
	}

	std::cout << total_size(argv[1]) << "\n";
}
