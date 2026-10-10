#include "rk/panel.hpp"

#include <cstring>

namespace rk {

bool Panel::begin(lv_obj_t* parent, int width, int height) {
    buf_ = lv_draw_buf_create(width, height, LV_COLOR_FORMAT_RGB565, 0);
    if (!buf_) return false;
    std::memset(buf_->data, 0, size_t(buf_->header.stride) * size_t(height));

    canvas_ = lv_canvas_create(parent);
    lv_canvas_set_draw_buf(canvas_, buf_);
    lv_obj_center(canvas_);

    surface_ = std::make_unique<Surface>(static_cast<uint8_t*>(buf_->data), width, height, int(buf_->header.stride));
    return true;
}

void Panel::present() {
    Rect r;
    if (!surface_->take_dirty(r)) return;
    lv_area_t area = {r.x0, r.y0, r.x1, r.y1};
    lv_obj_invalidate_area(canvas_, &area);
}

}  // namespace rk
