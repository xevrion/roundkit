#include "rk/direct_panel.hpp"

#include <algorithm>
#include <cstring>

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_lv_adapter.h"

namespace rk {

static const char* TAG = "rk.direct";

bool DirectPanel::begin(int width, int height) {
    width_ = width, height_ = height;
    disp_ = lv_display_get_default();
    if (!disp_) {
        ESP_LOGE(TAG, "no LVGL display, call bsp_display_start() first");
        return false;
    }
    fb_ = static_cast<uint8_t*>(heap_caps_calloc(1, size_t(width) * size_t(height) * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    // The slice has to be internal DMA memory: from PSRAM the SPI driver bounces every chunk.
    slice_ = static_cast<uint8_t*>(heap_caps_malloc(size_t(kSliceRows) * size_t(width) * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    if (!fb_ || !slice_) {
        ESP_LOGE(TAG, "out of memory (framebuffer %p, slice %p)", fb_, slice_);
        return false;
    }
    surface_ = std::make_unique<Surface>(fb_, width, height, width * 2);

    if (esp_lv_adapter_set_dummy_draw(disp_, true) != ESP_OK) {
        ESP_LOGE(TAG, "could not switch LVGL drawing off");
        return false;
    }
    send({0, 0, width - 1, height - 1});
    return true;
}

// One painted box, in bands of kSliceRows. The panel wants each 16 bit pixel with its bytes swapped.
void DirectPanel::send(const Rect& r) {
    const int w = r.x1 - r.x0 + 1;  // even: Surface widens boxes to an even start and an odd end
    const int words = w / 2;
    for (int y = r.y0; y <= r.y1; y += kSliceRows) {
        const int rows = std::min(kSliceRows, r.y1 - y + 1);
        for (int k = 0; k < rows; ++k) {
            const uint32_t* src = reinterpret_cast<const uint32_t*>(fb_ + (size_t(y + k) * size_t(width_) + size_t(r.x0)) * 2);
            uint32_t* dst = reinterpret_cast<uint32_t*>(slice_ + size_t(k) * size_t(w) * 2);
            for (int i = 0; i < words; ++i) {
                const uint32_t v = src[i];
                dst[i] = ((v & 0x00FF00FFu) << 8) | ((v >> 8) & 0x00FF00FFu);
            }
        }
        esp_lv_adapter_dummy_draw_blit(disp_, r.x0, y, r.x1 + 1, y + rows, slice_, true);
    }
}

bool DirectPanel::present() {
    Rect boxes[Surface::kMaxDirty];
    const int n = surface_->take_dirty_rects(boxes, Surface::kMaxDirty);
    for (int i = 0; i < n; ++i) send(boxes[i]);
    return n > 0;
}

}  // namespace rk
