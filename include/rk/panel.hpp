#pragma once

#include <memory>

#include "lvgl.h"
#include "rk/surface.hpp"

namespace rk {

// A full-screen LVGL canvas that pure C++ drawing code paints into through a Surface.
class Panel {
public:
    bool begin(lv_obj_t* parent, int width = 466, int height = 466);
    Surface& surface() { return *surface_; }
    // Hands the painted region to LVGL and returns true, or returns false when nothing was
    // painted since the last call. The caller holds the display lock.
    bool present();

private:
    lv_obj_t* canvas_ = nullptr;
    lv_draw_buf_t* buf_ = nullptr;
    std::unique_ptr<Surface> surface_;
};

}  // namespace rk
