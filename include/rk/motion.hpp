#pragma once

#include <cmath>
#include <cstdint>

namespace rk {

inline float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline float smooth01(float t) {
    t = clampf(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// A damped spring chasing a target. Every number an animation moves lives in one of these, so a
// change is never a jump: things accelerate, overshoot a little and settle.
struct Spring {
    float x = 0.0f, v = 0.0f, to = 0.0f;

    void snap(float value) { x = to = value, v = 0.0f; }

    // freq in Hz, zeta 1 settles without overshoot, below 1 it bounces.
    void step(float dt, float freq, float zeta) {
        const float w = 6.2831853f * freq;
        float left = dt;
        while (left > 0.0f) {
            const float h = left > (1.0f / 240.0f) ? (1.0f / 240.0f) : left;
            v += (w * w * (to - x) - 2.0f * zeta * w * v) * h;
            x += v * h;
            left -= h;
        }
    }
};

// Small deterministic random numbers, so a test can replay exactly what an animation did.
class Rng {
public:
    explicit Rng(uint32_t seed = 1) : s_(seed ? seed : 1) {}
    uint32_t next() {
        s_ ^= s_ << 13, s_ ^= s_ >> 17, s_ ^= s_ << 5;
        return s_;
    }
    float unit() { return float(next() >> 8) / 16777216.0f; }  // [0, 1)
    float range(float lo, float hi) { return lo + (hi - lo) * unit(); }
    int below(int n) { return int(next() % uint32_t(n)); }
    bool chance(float p) { return unit() < p; }

private:
    uint32_t s_;
};

// Angle of (y, x) in turns, 0 to 1, about 0.002 turns off. The spiral eyes call it for every pixel.
inline float fast_turns(float y, float x) {
    const float ay = std::fabs(y) + 1e-9f;
    float a;
    if (x >= 0.0f) {
        const float r = (x - ay) / (x + ay);
        a = 0.1963f * r * r * r - 0.9817f * r + 0.7853982f;
    } else {
        const float r = (x + ay) / (ay - x);
        a = 0.1963f * r * r * r - 0.9817f * r + 2.3561945f;
    }
    if (y < 0.0f) a = -a;
    float t = a * 0.15915494f;
    if (t < 0.0f) t += 1.0f;
    return t;
}

}  // namespace rk
