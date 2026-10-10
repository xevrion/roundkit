idf_component_register(
    SRCS "src/imu.cpp" "src/battery.cpp" "src/store.cpp" "src/font.cpp" "src/panel.cpp" "src/direct_panel.cpp"
    INCLUDE_DIRS "include"
    REQUIRES waveshare__esp32_s3_touch_amoled_1_75c waveshare__qmi8658 lvgl__lvgl espressif__esp_lvgl_adapter nvs_flash driver
)
