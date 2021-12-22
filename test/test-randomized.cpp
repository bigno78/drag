#include "utils/randomized.hpp"

int main() {

    Tester tester(7);

    tester.register_test("thingy", [] (const drag::graph& g) {
        return true;
    });

    tester.register_test("mingy", [] (const drag::graph& g) {
        return false;
    });

    return tester.run_tests();
}
