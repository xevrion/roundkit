#pragma once

#include "qmi8658.h"

namespace rk {

// The QMI8658 six-axis sensor on the Waveshare ESP32-S3-Touch-AMOLED-1.75C.
// On that board +x points away from the USB-C port, which is the top of the screen.
class Imu {
public:
    bool begin();
    // Acceleration in m/s^2. At rest it reads +g on whichever axis points up.
    bool read_accel(float& ax, float& ay, float& az);
    // Rotation rate in degrees per second.
    bool read_gyro(float& gx, float& gy, float& gz);

private:
    qmi8658_dev_t dev_{};
};

}  // namespace rk
