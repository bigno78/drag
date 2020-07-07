#include "interface.hpp"
#include "svg.hpp"
#include "parser.hpp"
#include "benchmarks/helper.hpp"

#include <tuple>
#include <fstream>
#include <string>
#include <sstream>

void prolong(std::vector<vec2>& points, node from, node to) {
	vec2 end = points.back();
	if ( (end.x - to.pos.x)*(end.x - to.pos.x) + (end.y - to.pos.y)*(end.y - to.pos.y) > to.size ) {
		vec2 dir = points.back() - to.pos;
		points.back() = to.pos + to.size * normalized(dir);
	}
}

template<typename A>
bool same_sgn(A a, A b) {
	return sgn(a) == sgn(b);
}

template<typename A, typename ... Args>
bool same_sgn(A a, A b, Args ... args) {
	return sgn(a) == sgn(b) && same_sgn(b, args...);
}

bool segments_intersect(vec2 p1, vec2 p2, vec2 q1, vec2 q2) {
	auto r = p2 - p1;
	auto s = q2 - q1;

	auto num = cross(q1 - p1, r);
	auto denom = cross(r, s);

	if (num == 0 && denom == 0) {
		//if (p1 == q1 || p1 == q2 || p2 == q1 || p2 == q2)
			//return true;

		return false;
	}

	if (denom == 0)
		return false;

	auto u = num / denom;
	auto t = cross(q1 - p1, s) / denom;

	return (t > 0) && (t <= 1) && (u > 0) && (u <= 1);
}

int polyline_crossings(const std::vector<vec2>& l1, const std::vector<vec2>& l2) {
	int total = 0;
	for (int i = 1; i < l1.size(); ++i) {
		for (int j = 1; j < l2.size(); ++j) {
			if (segments_intersect(l1[i-1], l1[i], l2[j-1], l2[j]))
				++total;
		}
	}
	return total;
}

auto parse_plain_dot(const std::string& file)
	-> std::tuple< std::vector<node>, std::vector<path>, vec2 > 
{
	std::ifstream in { file };
	
	std::vector<node> nodes;
	std::vector<path> paths;

	std::map<std::string, vertex_t> label_to_vertex;
	vertex_t u = 0;
	vec2 dims;

	std::string line;
	while(std::getline(in, line)) {
		std::istringstream line_stream { line };
		std::string key_word;
		line_stream >> key_word;

		if (key_word == "node") {
			std::string name;
			float x, y, size;
			line_stream >> name >> x >> y >> size;
			nodes.push_back( { u, vec2{ to_pt(x), dims.y - to_pt(y) }, to_pt(size)/2, std::to_string(u) } );
			label_to_vertex[name] = u++;
		} else if (key_word == "edge") {
			std::string from, to;
			int n;
			line_stream >> from >> to >> n;
			std::vector<vec2> points;

			for (int i = 0; i < n; ++i) {
				float x, y;
				line_stream >> x >> y;
				vec2 next { to_pt(x), dims.y - to_pt(y) };
				if (i == 1 || i == n - 2) continue;
				if (i == 0 || next != points.back()) {
					points.push_back(next);
				}
			}
			prolong(points, nodes[label_to_vertex[from]], nodes[label_to_vertex[to]]);

			paths.push_back( { label_to_vertex[from], label_to_vertex[to], points, false } );
		} else if (key_word == "graph") {
			float scale, w, h;
			line_stream >> scale >> w >> h;
			dims = vec2{ to_pt(w), to_pt(h) };
		}
	}

	return { nodes, paths, dims };
}

float get_total_bends(const std::vector<path>& paths) {
	float total = 0;
	for (const auto& p : paths) {
		vec2 prev { 0, 0};
		for (int i = 1; i < p.points.size(); ++i) {
			auto v = p.points[i] - p.points[i-1];
			if (cross(prev, v) != 0) {
				total++;
				//std::cout << p.from << " " << p.to << "\n";
			}
			prev = v;
		}
	}
	return total;
}

float get_total_length(const std::vector<path>& paths) {
	float total = 0;
	for (const auto& p : paths) {
		for (int i = 1; i < p.points.size(); ++i) {
			total += distance(p.points[i], p.points[i-1]);
		}
	}
	return total;
}

float get_total_reversed(const std::vector<path>& paths) {
	float total = 0;
	for (const auto& p : paths) {
		if (p.points.front().y > p.points.back().y || p.bidirectional)
			++total;
	}
	return total;
}

int get_total_cross(const std::vector<path>& paths, const std::vector<node>& nodes) {
	int total = 0;
	for(int p = 0; p < paths.size(); ++p) {
		for(int q = p + 1; q < paths.size(); ++q) {
			auto neco = polyline_crossings(paths[p].points, paths[q].points);
			//if (neco > 0)
				//std::cout << neco << ": " << paths[p].from << " " << paths[p].to << " | " << paths[q].from << " " << paths[q].to << "\n";
			total += neco;
		}
	}
	return total;
}

enum { cros, len, rev, bend };

std::string print(int i) {
	switch (i) {
		case cros: 
			return  "cross";
		case len: 
			return "len";
		case rev: 
			return "rev";
		case bend: 
			return "bend";
	}
}

struct stats {
	float min;
	float max;
	float median;
	float upper;
	float lower;
};

std::ostream& operator<<(std::ostream& out, stats st) {
	out << st.min << " (" << st.lower << " | " << st.median << " | " << st.upper << ") " << st.max;
	return out;
}

float median(const std::vector<float>& data, int from, int to) {
	int n = to - from + 1;
	if (n % 2 == 1) {
		return data[ from + n/2 ];
	}
	return (data[from + n/2] + data[from + n/2 - 1])/2;
}

std::tuple<float, float, float> quartiles(const std::vector<float>& data) {
	int n = data.size();
	float low, up;
	float med = median(data, 0, data.size() - 1);
	if (n % 2 == 1) {
		low = median(data, 0, n/2 - 1);
		up = median(data, n/2 + 1, n - 1);
	} else {
		low = median(data, 0, n/2 - 1);
		up = median(data, n/2, n - 1);
	}
	return { low, med, up };
}

void do_stat(const std::string& in) {
	std::array<std::vector<float>, 4> props;

	auto files = dir_contents(in, ".plain");
	for (const auto& f : files) {
		
		auto [ nodes, paths, dims ] = parse_plain_dot( in + "/" + f );

		props[cros].push_back( get_total_cross(paths, nodes) );
		props[len].push_back( get_total_length(paths) );
		props[rev].push_back( get_total_reversed(paths) );
		props[bend].push_back( get_total_bends(paths) );

		std::cout << f << " " << props[cros].back() << "\n";
	}

	std::array< stats, 4 > res;

	for (int i = 0; i < 4; ++i) {
		auto& data = props[i];
		std::sort(data.begin(), data.end());

		stats st;
		st.min = data.front();
		st.max = data.back();
		std::tie(st.lower, st.median, st.upper) = quartiles(data);
		res[i] = st;
		std::cout << print(i) << ": " << st << "\n";
	}
}



int main(int argc, char **argv) {
	//do_stat(argv[1]);

	if (std::string{argv[1]} == "-p"){
		auto [ nodes, paths, dims ] = parse_plain_dot(argv[2]);

		std::cout << "bends:     " << get_total_bends(paths) << "\n";
		std::cout << "length:    " << get_total_length(paths) << "\n";
		std::cout << "reversed:  " << get_total_reversed(paths) << "\n";
		std::cout << "crossings: " << get_total_cross(paths, nodes) << "\n";
	} else {
		attributes at;
		std::map<vertex_t, std::string> labels;
		float size;
		auto g = parse(argv[1], labels, at, size);

		sugiyama_layout l(g, at);

		std::cout << "bends:     " << get_total_bends(l.edges()) << "\n";
		std::cout << "length:    " << get_total_length(l.edges()) << "\n";
		std::cout << "reversed:  " << get_total_reversed(l.edges()) << "\n";
		std::cout << "crossings: " << get_total_cross(l.edges(), l.vertices()) << "\n";
	}
}