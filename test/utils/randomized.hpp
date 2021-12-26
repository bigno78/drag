#include <drag/detail/gen.hpp>
#include <drag/drawing/draw.hpp>

#include <string>
#include <vector>
#include <functional>


struct test_config {
    size_t n = 15;
    size_t m = 25;
    size_t count = 50;
};


struct RandomizedTest {

    std::function<bool(drag::graph&)> func;
    std::string name;
    test_config config;

    template<typename Func>
    RandomizedTest(const std::string& name, test_config config, Func func)
        : func(func)
        , name(name)
        , config(config) { }
};


struct Tester {

    Tester(size_t seed) : m_gen(seed) {}
    Tester(test_config default_config, size_t seed) 
        : m_config(default_config)
        , m_gen(seed) { }

    template<typename Func>
    void register_test(const std::string& name, Func func) {
        register_test(name, m_config, std::move(func));
    }

    template<typename Func>
    void register_test(const std::string& name, test_config config, Func func) {
        m_tests.emplace_back(name, config, func);
    }

    bool run_tests(bool stop_on_error = true) {
        size_t failed_count = 0;
        
        int i = 0;
        for (const auto& test : m_tests) {
            bool passed = execute_test(test, i);
            
            if (!passed) {
                failed_count++;
                if (stop_on_error)
                    return false;
            }

            i++;
        }

        if (failed_count > 0) {
            std::cerr << failed_count << " TESTS FAILED\n";
            return false;
        }

        std::cerr << "ALL TESTS PASSED\n";
        return true;
    }

private:
    std::vector<RandomizedTest> m_tests;
    test_config m_config;
    drag::dag_generator m_gen;
    drag::drawing_options m_opts;

    bool execute_test(const RandomizedTest& test, int id) {
        for (size_t i = 0; i < test.config.count; ++i) {
            drag::graph g = m_gen.generate_from_edges(test.config.n, test.config.m);
            
            bool passed = test.func(g);
            
            if (!passed) {
                std::string filename = test.name + "_" + std::to_string(id) + "_error.svg";

                std::cerr << "TEST FAILED";
                if (!test.name.empty()) {
                    std::cerr << ": " << test.name << "\n";
                }
                std::cerr << "Saving the graph to: '" << filename << "'\n";
                
                drag::draw_svg_file(filename, g, m_opts);

                return false;
            }
        }

        return true;
    }
};
