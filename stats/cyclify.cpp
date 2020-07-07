#include "graph.hpp"
#include "parser.hpp"
#include "benchmarks/helper.hpp"

#include <random>
#include <set>

void dfs_sort(const graph& g, vertex_t u, std::vector<std::vector<vertex_t>>& tp) {
	if (!tp[u].empty()) {
		return;
	}

	for (auto v : g.in_neighbours(u)) {
		dfs_sort(g, v, tp);
		for (auto x : tp[v]) {
			tp[u].push_back(x);
		}
		tp[u].push_back(v);
	} 
}

std::vector<std::vector<vertex_t>> dfs_sort(const graph& g) {
	std::vector<std::vector<vertex_t>> tp(g.size());
	for (auto u : g.vertices()) {
		dfs_sort(g, u, tp);
	}
	return tp;
}

void write_dot(const graph& g, std::map<vertex_t, std::string>& labels, const std::string& file) {
	std::ofstream out { file };

	out << "digraph G{\n";
	//out << "splines=polyline\n";
	//out << "node [shape=circle fixedsize=shape width=0.7]\n";

	for (auto u : g.vertices()) {
		for (auto v : g.out_neighbours(u)) {
			out << labels[u] << " -> " << labels[v] << "\n";
		}
	}

	out << "}\n";
}

int main(int argc, char **argv) {
	std::mt19937 mt;

	auto files = dir_contents(argv[1], ".gv");
	for (const auto& f : files) {
		std::map<vertex_t, std::string> labels;
		attributes attr;
		float size;
		graph g = parse(argv[1] + std::string{"/"} + f, labels, attr, size);

		auto tp = dfs_sort(g);

		/*int u = 0;
		for (auto preds : tp) {
			std::cout << labels[u] << ": ";
			for (auto v : preds) {
				std::cout << labels[v] << " ";
			} 
			std::cout << "\n";
			++u;
		}*/

		//std::random_device rd;
		std::uniform_int_distribution<vertex_t> dist(0, g.size() - 1);

		std::set<vertex_t> used;

		int n = 0.3 * g.size();
		while(n) {

			vertex_t u = dist(mt);

			if (used.count(u) > 0 || tp[u].empty()) continue;
			used.insert(u);

			std::uniform_int_distribution<int> pred(0, tp[u].size() - 1);
			vertex_t v = tp[u][ pred(mt) ];

			g.add_edge(u, v);
			std::cout << labels[u] << " " << labels[v] << "\n";

			--n;
		}

		write_dot(g, labels, argv[1] + std::string{"/"} + f);
	}
}