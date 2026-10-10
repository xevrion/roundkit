#pragma once

#include <memory>

#include "lvgl.h"
#include "rk/surface.hpp"

namespace rk {

// A framebuffer that goes to the panel without LVGL drawing anything. LVGL keeps running for touch
// input, but its renderer is switched off (the LVGL adapter's own "dummy draw" mode), and pixels
// are sent straight to the panel in bands through a small buffer in internal RAM. That avoids
// LVGL's per-refresh copying and the SPI driver bouncing every transfer out of PSRAM.
//
// Use it instead of Panel when a scene changes every frame. Panel is simpler and fine for a
// scene that mostly sits still.
class DirectPanel {
public:
    static constexpr int kSliceRows = 16;

    // Call after bsp_display_start(). Clears the screen to black.
    bool begin(int width = 466, int height = 466);
    Surface& surface() { return *surface_; }
    // Sends the painted region to the panel and returns true, or returns false when nothing was
    // painted since the last call. Needs no display lock.
    bool present();

private:
    void send(const Rect& r);

    lv_display_t* disp_ = nullptr;
    uint8_t* fb_ = nullptr;
    uint8_t* slice_ = nullptr;
    int width_ = 0, height_ = 0;
    std::unique_ptr<Surface> surface_;
};

}  // namespace rk
