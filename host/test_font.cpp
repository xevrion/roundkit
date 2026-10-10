#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "rk/font.hpp"

using namespace rk;

static int failures = 0;

#define CHECK(cond, ...)                                     \
    do {                                                     \
        if (!(cond)) {                                       \
            std::printf("FAIL %s:%d  ", __FILE__, __LINE__); \
            std::printf(__VA_ARGS__);                        \
            std::printf("\n");                               \
            ++failures;                                      \
        }                                                    \
    } while (0)

static std::string fold(const char* s) {
    char buf[128];
    fold_text(s, buf, sizeof buf);
    return buf;
}

static int lit_pixels(const char* text, int scale, const Rect* clip = nullptr, bool rotate = false) {
    std::vector<uint16_t> px(200 * 60, 0);
    Surface sf(reinterpret_cast<uint8_t*>(px.data()), 200, 60, 400);
    draw_text(sf, 4, 4, text, scale, 0xFFFF, clip, rotate);
    int n = 0;
    for (uint16_t p : px) n += p != 0;
    return n;
}

static void test_every_printable_has_a_glyph() {
    for (const char* p = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,'-()&!?:/%+#@*\"_<>="; *p; ++p) {
        const char s[2] = {*p, 0};
        CHECK(lit_pixels(s, 1) > 0, "no glyph for '%c'", *p);
    }
    CHECK(lit_pixels(" ", 1) == 0, "a space draws nothing");
}

static void test_widths() {
    CHECK(text_width("", 3) == 0, "empty string has no width");
    CHECK(text_width("00:00", 4) == text_width("11:11", 4), "digits are tabular, so a clock does not jitter");
    CHECK(text_width("MMM", 2) > text_width("III", 2), "wide letters take more room than narrow ones");
    CHECK(text_width("AB", 4) == 2 * text_width("AB", 2), "width scales linearly");
    CHECK(text_height(3) == 21, "seven rows at scale 3");
}

static void test_clip_and_rotate() {
    const int full = lit_pixels("HELLO", 2);
    const Rect half = {0, 0, 40, 59};
    const int clipped = lit_pixels("HELLO", 2, &half);
    CHECK(clipped > 0 && clipped < full, "a clip keeps some pixels and drops others: %d of %d", clipped, full);
    CHECK(lit_pixels("HELLO", 2, nullptr, true) == full, "rotating changes where pixels land, not how many");
}

static void test_fold() {
    CHECK(fold("Hello, World") == "HELLO, WORLD", "upper-cases ASCII");
    CHECK(fold("Beyonc\xC3\xA9") == "BEYONCE", "drops the accent on e: %s", fold("Beyonc\xC3\xA9").c_str());
    CHECK(fold("Mot\xC3\xB6rhead") == "MOTORHEAD", "drops the umlaut on o: %s", fold("Mot\xC3\xB6rhead").c_str());
    CHECK(fold("Don\xE2\x80\x99t") == "DON'T", "straightens a curly apostrophe");
    CHECK(fold("A\xE2\x80\x94" "B") == "A-B", "straightens an em dash");
    CHECK(fold("Wait\xE2\x80\xA6") == "WAIT...", "expands an ellipsis");
    CHECK(fold("\xE0\xA4\xA8\xE0\xA4\xAE\xE0\xA4\xB8\xE0\xA5\x8D\xE0\xA4\xA4\xE0\xA5\x87") == "?", "a whole word in another script becomes one '?', got %s",
          fold("\xE0\xA4\xA8\xE0\xA4\xAE\xE0\xA4\xB8\xE0\xA5\x8D\xE0\xA4\xA4\xE0\xA5\x87").c_str());
    CHECK(fold("Hi \xF0\x9F\x8E\xB5 there") == "HI ? THERE", "an emoji becomes '?': %s", fold("Hi \xF0\x9F\x8E\xB5 there").c_str());
    CHECK(fold("") == "", "empty in, empty out");
    CHECK(fold("a\tb") == "A B", "tabs become spaces");
}

static void test_fold_is_safe() {
    char tiny[4];
    const size_t n = fold_text("ABCDEFGH", tiny, sizeof tiny);
    CHECK(n == 3 && std::strcmp(tiny, "ABC") == 0, "truncates to the buffer and stays terminated: '%s' (%zu)", tiny, n);
    char one[1];
    CHECK(fold_text("x", one, 1) == 0 && one[0] == '\0', "a one-byte buffer holds just the terminator");
    CHECK(fold_text("x", nullptr, 0) == 0, "no buffer is fine");
    // Truncated multi-byte sequences and stray continuation bytes must not run off the end.
    CHECK(fold("\xE2\x80") == "?", "a cut-off sequence is one '?': %s", fold("\xE2\x80").c_str());
    CHECK(fold("\x80\x80\x80") == "?", "stray continuation bytes are one '?'");
    CHECK(fold("ok\xF0") == "OK?", "a lone lead byte at the end: %s", fold("ok\xF0").c_str());
}

static void test_write_row() {
    std::vector<uint16_t> px(10 * 4, 0);
    Surface sf(reinterpret_cast<uint8_t*>(px.data()), 10, 4, 20);
    const uint16_t src[6] = {1, 2, 3, 4, 5, 6};
    sf.write_row(2, 1, src, 6);
    CHECK(sf.get(2, 1) == 1 && sf.get(7, 1) == 6 && sf.get(1, 1) == 0 && sf.get(8, 1) == 0, "span lands where it should");
    Rect r;
    CHECK(sf.take_dirty(r) && r.x0 == 2 && r.x1 == 7 && r.y0 == 0 && r.y1 == 1, "dirty box covers the span: %d,%d..%d,%d", r.x0, r.y0, r.x1, r.y1);

    sf.write_row(-2, 2, src, 6);
    CHECK(sf.get(0, 2) == 3 && sf.get(3, 2) == 6, "a span starting off the left edge is clipped, not shifted");
    sf.write_row(7, 3, src, 6);
    CHECK(sf.get(7, 3) == 1 && sf.get(9, 3) == 3, "a span running off the right edge is clipped");
    sf.take_dirty(r);
    sf.write_row(0, -1, src, 6);
    sf.write_row(0, 4, src, 6);
    sf.write_row(-9, 0, src, 6);
    sf.write_row(10, 0, src, 6);
    sf.write_row(0, 0, src, 0);
    CHECK(!sf.take_dirty(r), "spans entirely outside change nothing");
}

int main() {
    test_write_row();
    test_every_printable_has_a_glyph();
    test_widths();
    test_clip_and_rotate();
    test_fold();
    test_fold_is_safe();
    if (failures) {
        std::printf("%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("all font checks passed\n");
    return 0;
}
