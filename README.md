# roundkit

The parts that every project on the Waveshare ESP32-S3-Touch-AMOLED-1.75C ends up needing, so they get written once.

| Piece | What it does |
|---|---|
| `rk/imu.hpp` | Tilt and rotation from the QMI8658. On this board +x points away from the USB-C port, the top of the screen. |
| `rk/battery.hpp` | Battery percent and charging from the AXP2101 power chip, and a clear answer when no battery is fitted. |
| `rk/store.hpp` | A few integers that survive a power cycle, in NVS, one namespace per app. |
| `rk/gesture.hpp` | Tap and long-press from raw touch state. Header only, so it also builds on a laptop. |
| `rk/display_lock.hpp` | Holds the LVGL mutex for a scope. A timeout of 0 means "don't wait" and fails silently, so this always blocks. |
| `scripts/idf` | Runs `idf.py` in Espressif's Docker image, with the SELinux and serial-port workarounds, so the host needs only Docker. |

## Using it

Add it to a project as a submodule and point ESP-IDF at it:

    git submodule add https://github.com/xevrion/roundkit.git roundkit

    # top-level CMakeLists.txt, before including project.cmake
    set(EXTRA_COMPONENT_DIRS roundkit)

Then list `roundkit` in a component's `REQUIRES` and `#include "rk/battery.hpp"`.

## Tests

    make test

Only the gesture logic runs off the board.

## Projects using it

- [hourglass](https://github.com/xevrion/hourglass), a pomodoro timer that is an hourglass

## License

MIT
