#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

#define REPORTING

#include "../interface.hpp"
#include "../svg.hpp"
#include "../parser.hpp"
#include "helper.hpp"

struct iter {
    int forgv = 1;
    int rnd = 1;
    bool trans = false;
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "wrong usage!\n";
        return 1;
    }

    std::string prefix = "../data/cross_data/";
    std::string path = prefix + argv[1] + "/";

    std::string in_path = path + "gv/";
    std::string out_path = path + "out/";
    std::string log_path = prefix + "data.txt";

    std::vector<iter> iters;
    for (int i : { 1, 2, 4, 8 }) {
        iters.push_back( { i, 1, false } );
        iters.push_back( { i, 15, false } );
        iters.push_back( { i, 1, true } );
    }

    std::vector<uint64_t> times( iters.size() );
    std::vector<long double> decrease( iters.size() );
    std::vector<long double> rnd_it( iters.size() );

    auto files = dir_contents(in_path);

    if (files.empty()) {
        return 1;
    }

    for (auto& f : files) {
        
        graph g;
        auto lbls = parse(in_path + f, g);

        std::vector<vertex_t> vertices;
        for (auto u : g.vertices()) {
            vertices.push_back(u);
        }
        detail::subgraph sub(g, vertices);
        
        detail::network_simplex_layering lay(g);
        detail::hierarchy h = lay.run(sub);
        detail::add_dummy_nodes(h);
        
        int base = detail::count_crossings(h);

        for (int i = 0; i < iters.size(); ++i) {
                defaults::forgv = iters[i].forgv;
                defaults::trans = iters[i].trans;
                defaults::iters = iters[i].rnd;
                
                detail::hierarchy hi = h;
                detail::barycentric_heuristic bar;

                auto start = now();
                bar.run(hi);
                times[i] += to_micro( now() - start );
                decrease[i] += (report::base - report::final) / (long double) report::base;
                rnd_it[i] += report::iters;

                /*sugiyama_layout lay(g);
                lay.build();
                svg_img img{ out + f + ".svg" };
                draw_to_svg(img, lay, lbls);*/
        }
    }

    std::ofstream o{ log_path, std::ios::app | std::ios::out };

    for(int i = 0; i < iters.size(); ++i) {
        decrease[i] /= files.size();
        times[i] /= files.size();
        rnd_it[i] /= files.size();
    }

   /* for (int i = 0; i < iters.size(); ++i) {
        const auto* it = &iters[i];
        o <<  it->forgv << (it->trans ? "T_" : "_") << it->rnd << " ";
        //o <<  it->forgv << (it->trans ? "T_" : "_") << it->rnd << "time ";
    }
    o << "\n";*/

    /*o << argv[1] << " ";
    for (int i = 0; i < iters.size(); ++i) {
        o <<  decrease[i] << " " << times[i] << " ";
    }
    o << "\n";*/

    std::cout << std::setprecision(2);
    for (int i = 0; i < iters.size() - 2; i += 3) {
        const auto* it = &iters[i];
        std::cout << it->forgv << (it->trans ? "T_" : "_") << it->rnd << ": ";
        std::cout << decrease[i] << "(" << times[i] << ") " << rnd_it[i] << " iters\n";

        it = &iters[i + 1];
        std::cout << it->forgv << (it->trans ? "T_" : "_") << it->rnd << ": ";
        std::cout << decrease[i + 1] << "(" << times[i + 1] << ") ";
        std::cout << decrease[i + 1]/decrease[i] - 1 << " less, " << times[i + 1]/(float)times[i] << "x longer " 
                  << rnd_it[i+1] << " iters\n";;

        it = &iters[i + 2];
        std::cout << it->forgv << (it->trans ? "T_" : "_") << it->rnd << ": ";
        std::cout << decrease[i + 2] << "(" << times[i + 2] << ") ";
        std::cout << decrease[i + 2]/decrease[i] - 1 << " less, " << times[i + 2]/(float)times[i] << "x longer "
                  << rnd_it[i+2] << " iters\n\n";
    }
}
