#pragma once

#include "bsp/esp32_s3_touch_amoled_1_75c.h"

namespace rk {

// Holds the LVGL mutex for a scope. The adapter treats a timeout of 0 as "do not
// wait", so a lock taken with 0 can silently fail; only -1 reliably blocks until free.
class DisplayLock {
public:
    DisplayLock() { bsp_display_lock(-1); }
    ~DisplayLock() { bsp_display_unlock(); }
    DisplayLock(const DisplayLock&) = delete;
    DisplayLock& operator=(const DisplayLock&) = delete;
};

}  // namespace rk
