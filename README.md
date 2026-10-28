# producer-consumer-esp32

ESP-IDF application, in C, for the **ESP32-C6-WROOM-1** module. The pioarduino board is `esp32-c6-devkitc-1` (Espressif ESP32-C6-DevKitC-1), which uses that module. Open this folder in VS Code with the [pioarduino IDE](https://marketplace.visualstudio.com/items?itemName=pioarduino.pioarduino-ide) extension.

A producer task and a consumer task exchange `pc_item_t` values through the `pc_queue` component, a bounded FreeRTOS queue. Unity tests for that component live next to it and run on the device.

## Layout

```
producer-consumer-esp32/
├── platformio.ini
├── CMakeLists.txt
├── sdkconfig.defaults
├── main/                          application component (app_main)
│   ├── CMakeLists.txt
│   └── main.c
└── components/
    └── pc_queue/                  sample component
        ├── CMakeLists.txt
        ├── include/pc_queue.h
        ├── pc_queue.c
        └── test/test_pc_queue.c  Unity tests, run with pio test
```

Reusable code stays under `components/`. The application entry point stays in `main/`. PlatformIO is pointed at those directories with `src_dir` and `test_dir` in `platformio.ini`.

## What the firmware does

`app_main` creates a 4-slot queue and starts two tasks:

- **producer** posts an incrementing item once a second
- **consumer** blocks until an item is available, then logs it

Console output uses the USB Serial/JTAG port at 115200 baud. On the DevKitC-1 that is the USB connector used for both flashing and the serial monitor.

## Tests

`components/pc_queue/test/test_pc_queue.c` uses the Unity framework (`test_framework = unity`). The cases cover a zero capacity, FIFO order, empty and full timeouts, rejected NULL arguments, and a two-task exchange through a queue that is smaller than the number of items.

`pio test` builds this file in place of `main/main.c`, flashes the test image, and reads the Unity summary from the serial port. A board has to be connected. This environment has no attached ESP32-C6, so the test image can be compiled here and has to be executed on hardware.

## Build and flash

Install [VS Code](https://code.visualstudio.com/) and the **pioarduino IDE** extension (`pioarduino.pioarduino-ide`). pioarduino does not install a C/C++ language server for you; install Microsoft C/C++ (`ms-vscode.cpptools`) as well. Then open this folder (the directory that contains `platformio.ini`), not the parent repository root.

From the pioarduino toolbar or the command palette:

- **Build** compiles the demo
- **Upload** flashes it
- **Monitor** opens the serial log
- **Test** builds, flashes, and runs the Unity suite
- **Debug** uses the on-board USB-JTAG probe (`debug_tool = esp-builtin`)

CLI equivalents, from this directory, after the [pioarduino core installer](https://github.com/pioarduino/platform-espressif32#cli):

```bash
pio run
pio run -t upload
pio device monitor
pio test
```

The platform package is the current pioarduino stable Espressif 32 release (ESP-IDF 5.5.x). The first build downloads the toolchain and the framework.

`sdkconfig.defaults` selects 8 MB QIO flash at 80 MHz, matching the `esp32-c6-devkitc-1` board definition. If your WROOM-1 is a 4 MB (N4) or 16 MB (N16) part, change `CONFIG_ESPTOOLPY_FLASHSIZE_*` in `sdkconfig.defaults`, delete `sdkconfig`, and build again. Menuconfig is `pio run -t menuconfig`.
