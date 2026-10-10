#include "rk/store.hpp"

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

namespace rk {

static const char* TAG = "rk.store";

static bool open_nvs(const char* ns, nvs_open_mode_t mode, nvs_handle_t& handle) {
    static bool ready = false;
    if (!ready) {
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            nvs_flash_erase();
            err = nvs_flash_init();
        }
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "nvs unavailable: %s", esp_err_to_name(err));
            return false;
        }
        ready = true;
    }
    return nvs_open(ns, mode, &handle) == ESP_OK;
}

int store_get(const char* ns, const char* key, int fallback) {
    nvs_handle_t h;
    if (!open_nvs(ns, NVS_READONLY, h)) return fallback;
    int32_t v = fallback;
    if (nvs_get_i32(h, key, &v) != ESP_OK) v = fallback;
    nvs_close(h);
    return v;
}

void store_set(const char* ns, const char* key, int value) {
    nvs_handle_t h;
    if (!open_nvs(ns, NVS_READWRITE, h)) return;
    nvs_set_i32(h, key, value);
    nvs_commit(h);
    nvs_close(h);
}

}  // namespace rk
