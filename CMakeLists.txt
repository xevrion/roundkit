idf_component_register(
    SRCS "src/imu.cpp" "src/battery.cpp" "src/store.cpp"
    INCLUDE_DIRS "include"
    REQUIRES waveshare__esp32_s3_touch_amoled_1_75c waveshare__qmi8658 nvs_flash driver
)
