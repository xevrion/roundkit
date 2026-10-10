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
    // Hands the painted region to LVGL. The caller holds the display lock.
    void present();

private:
    lv_obj_t* canvas_ = nullptr;
    lv_draw_buf_t* buf_ = nullptr;
    std::unique_ptr<Surface> surface_;
};

}  // namespace rk
