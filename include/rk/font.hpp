#pragma once

#include <cstddef>

#include "rk/surface.hpp"

namespace rk {

constexpr int kGlyphW = 5;
constexpr int kGlyphH = 7;

// Width in pixels of a string at the given scale. Digits are all the same width so a
// clock does not jitter as it counts; everything else takes the room it needs.
int text_width(const char* s, int scale);
int text_height(int scale);

// Draws s with its top-left at (x, y), skipping any pixel outside clip when one is given.
// With rotate180 the whole block is turned half a revolution inside the same box.
void draw_text(Surface& sf, int x, int y, const char* s, int scale, uint16_t colour, const Rect* clip = nullptr, bool rotate180 = false);

// Turns UTF-8 into something the font can draw: upper case, accents dropped, curly quotes
// and dashes straightened, and any run of characters it cannot draw (other scripts, emoji)
// collapsed into a single '?'. Returns the length written, always null terminated.
size_t fold_text(const char* utf8, char* out, size_t capacity);

}  // namespace rk
