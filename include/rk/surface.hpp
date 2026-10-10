#pragma once

#include <algorithm>
#include <cstdint>

namespace rk {

struct Rect {
    int x0, y0, x1, y1;  // inclusive
};

constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return uint16_t(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

// An RGB565 pixel buffer that remembers which parts of it were painted since the last time
// they were taken. It keeps up to kMaxDirty separate boxes, merging boxes that touch or sit close
// together, so a spinning disc at the top and a scrolling line at the bottom are two small
// updates for the panel instead of one huge one. Pure C++, so it runs on a laptop for the simulator.
class Surface {
public:
    static constexpr int kMaxDirty = 4;

    Surface(uint8_t* base, int w, int h, int stride_bytes) : base_(base), w_(w), h_(h), stride_(stride_bytes) {}

    int width() const { return w_; }
    int height() const { return h_; }

    void set(int x, int y, uint16_t c) {
        if (x < 0 || y < 0 || x >= w_ || y >= h_) return;
        row(y)[x] = c;
        grow(x, y, x, y);
    }

    uint16_t get(int x, int y) const { return row(y)[x]; }

    // Copies n pixels into row y starting at x, clipped to the surface, with the dirty boxes
    // updated once for the whole span. Much cheaper than n calls to set().
    void write_row(int x, int y, const uint16_t* px, int n) {
        if (y < 0 || y >= h_ || n <= 0) return;
        int skip = 0;
        if (x < 0) skip = -x, x = 0;
        if (skip >= n) return;
        n -= skip;
        if (x + n > w_) n = w_ - x;
        if (n <= 0) return;
        std::copy(px + skip, px + skip + n, row(y) + x);
        grow(x, y, x + n - 1, y);
    }

    void fill_rect(int x, int y, int w, int h, uint16_t c) {
        const int x0 = std::max(x, 0), y0 = std::max(y, 0);
        const int x1 = std::min(x + w, w_) - 1, y1 = std::min(y + h, h_) - 1;
        if (x1 < x0 || y1 < y0) return;
        for (int yy = y0; yy <= y1; ++yy) std::fill(row(yy) + x0, row(yy) + x1 + 1, c);
        grow(x0, y0, x1, y1);
    }

    // Hands over the painted boxes, each widened to an even start and odd end (what the panel's
    // address window needs) and clipped to the surface, and starts afresh. Returns how many.
    int take_dirty_rects(Rect* out, int max) {
        const int n = std::min(count_, max);
        for (int i = 0; i < n; ++i) out[i] = even(boxes_[i]);
        count_ = 0;
        return n;
    }

    // One box around everything painted, or false when nothing was.
    bool take_dirty(Rect& out) {
        if (count_ == 0) return false;
        Rect u = boxes_[0];
        for (int i = 1; i < count_; ++i) u = merge(u, boxes_[i]);
        out = even(u);
        count_ = 0;
        return true;
    }

private:
    static long area(const Rect& r) { return long(r.x1 - r.x0 + 1) * long(r.y1 - r.y0 + 1); }
    static Rect merge(const Rect& a, const Rect& b) { return {std::min(a.x0, b.x0), std::min(a.y0, b.y0), std::max(a.x1, b.x1), std::max(a.y1, b.y1)}; }
    Rect even(const Rect& r) const { return {r.x0 & ~1, r.y0 & ~1, std::min(r.x1 | 1, w_ - 1), std::min(r.y1 | 1, h_ - 1)}; }

    uint16_t* row(int y) const { return reinterpret_cast<uint16_t*>(base_ + size_t(y) * size_t(stride_)); }

    void grow(int x0, int y0, int x1, int y1) {
        const Rect n = {x0, y0, x1, y1};
        // Most pixels land inside a box that already exists.
        for (int i = 0; i < count_; ++i)
            if (x0 >= boxes_[i].x0 && x1 <= boxes_[i].x1 && y0 >= boxes_[i].y0 && y1 <= boxes_[i].y1) return;

        // Merge into a box it overlaps or sits near, when that does not waste much area.
        for (int i = 0; i < count_; ++i) {
            const Rect u = merge(boxes_[i], n);
            if (area(u) <= (area(boxes_[i]) + area(n)) * 5 / 4 + 600) {
                boxes_[i] = u;
                coalesce(i);
                return;
            }
        }
        if (count_ < kMaxDirty) {
            boxes_[count_++] = n;
            return;
        }
        // Out of boxes: grow whichever one wastes the least.
        int best = 0;
        long least = area(merge(boxes_[0], n)) - area(boxes_[0]);
        for (int i = 1; i < count_; ++i) {
            const long waste = area(merge(boxes_[i], n)) - area(boxes_[i]);
            if (waste < least) least = waste, best = i;
        }
        boxes_[best] = merge(boxes_[best], n);
        coalesce(best);
    }

    // After a box grows it may now overlap others; fold those in.
    void coalesce(int i) {
        for (int j = 0; j < count_; ++j) {
            if (j == i) continue;
            const Rect& a = boxes_[i];
            const Rect& b = boxes_[j];
            if (a.x0 <= b.x1 && b.x0 <= a.x1 && a.y0 <= b.y1 && b.y0 <= a.y1) {
                boxes_[i] = merge(a, b);
                boxes_[j] = boxes_[--count_];
                if (i == count_) i = j;
                j = -1;
            }
        }
    }

    uint8_t* base_;
    int w_, h_, stride_;
    Rect boxes_[kMaxDirty] = {};
    int count_ = 0;
};

}  // namespace rk
