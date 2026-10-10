#include <cstdio>

#include "rk/gesture.hpp"

using namespace rk;

static int failures = 0;

#define CHECK(cond, msg)                                         \
    do {                                                         \
        if (!(cond)) {                                           \
            std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, msg); \
            ++failures;                                          \
        }                                                        \
    } while (0)

int main() {
    Gesture g;
    CHECK(g.update(true, 100, 100, 0.1f) == Tap::None, "no event while pressing");
    CHECK(g.update(false, 100, 100, 0.0f) == Tap::Short, "quick release is a tap");

    g = Gesture();
    Tap t = Tap::None;
    for (int i = 0; i < 20 && t == Tap::None; ++i) t = g.update(true, 50, 50, 0.1f);
    CHECK(t == Tap::Long, "holding fires a long press");
    CHECK(g.update(false, 50, 50, 0.0f) == Tap::None, "releasing after a long press is not also a tap");

    g = Gesture();
    g.update(true, 10, 10, 0.1f);
    g.update(true, 200, 10, 0.05f);
    CHECK(g.update(false, 200, 10, 0.0f) == Tap::None, "a drag is not a tap");

    g = Gesture();
    g.update(true, 10, 10, 0.5f);
    CHECK(g.update(false, 10, 10, 0.0f) == Tap::None, "a press too slow for a tap and too short for a long press is neither");

    if (failures) return 1;
    std::printf("all gesture checks passed\n");
    return 0;
}
