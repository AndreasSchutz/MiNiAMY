# MiNiAMY

MiNiAMY is a compact, touch-controlled synthesizer built with AMY, ESP32-S3, LVGL, and Arduino. The firmware combines a graphical 320 × 240 interface with a polyphonic synthesizer, sequencer, patch management, MIDI input, and persistent project storage.

## Features

- AMY synthesizer engine with 120 oscillators
- 16-part synthesizer setup
- LVGL-based 320 × 240 touchscreen interface
- On-screen sequencer and song chain
- DX7/FM algorithm controls and patch browser
- MIDI note and MIDI CC input over USB serial
- External MIDI clock start, stop, and timing support
- ES8311 audio codec control and master volume
- Project save/load through attached storage
- NVS persistence for parameters and patterns
- Adjustable screen brightness, dim timeout, and orientation
- Hot-swap storage support

## Hardware

The firmware is configured for:

- ESP32-S3 development board
- 16 MB flash
- 8 MB Octal PSRAM
- 320 × 240 ILI9341-compatible LCD
- Capacitive touch controller
- ES8311 audio codec
- ST7701-compatible display initialization support

The current PlatformIO environment is:

```text
board = esp32-s3-devkitc-1
framework = arduino
platform = espressif32
```

> The exact pins and hardware definitions are in `src/main.cpp`, `src/es8311_init.h`, and `src/guition_init.h`. Check them against your board before flashing.

## Requirements

- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- USB data cable
- ESP32-S3 board with at least 16 MB flash and 8 MB PSRAM
- 3.3 V-compatible display, touch controller, and audio hardware

## Build and upload

### Visual Studio Code

1. Open the project in VS Code.
2. Install the PlatformIO IDE extension.
3. Connect the ESP32-S3 board over USB.
4. Select the `esp32-s3-display` environment.
5. Run **PlatformIO: Upload**.

### PlatformIO CLI

```bash
pio run -e esp32-s3-display
pio run -e esp32-s3-display -t upload
```

Open the serial monitor:

```bash
pio device monitor -b 115200
```

## Usage

The touchscreen provides access to synth parameters, patches, performance controls, the sequencer, and system settings.

Basic MIDI commands can be sent through the USB serial connection:

```text
note <channel> <note> <velocity>
cc <controller> <value>
```

Example:

```text
note 1 60 100
cc 1 64
```

The firmware also accepts standard MIDI timing and transport bytes over serial for external-clock playback.

## Project structure

```text
.
├── platformio.ini       # Board, build, memory, and dependency configuration
├── src
│   ├── main.cpp         # Application setup and main hardware loop
│   ├── amy_bridge.h     # AMY synthesizer integration
│   ├── synth_gui.h      # Main LVGL user interface
│   ├── seq_view.h       # Sequencer interface
│   ├── storage_mgr.h    # Project and storage management
│   ├── dx7_algo_view.h  # DX7/FM algorithm interface
│   ├── es8311_init.h    # ES8311 audio codec setup
│   ├── guition_init.h   # ST7701 display initialization
│   ├── patch_names.h    # Patch names and definitions
│   ├── help_manual.h    # On-device help text
│   └── lv_conf.h        # LVGL configuration
└── README.md
```

## Dependencies

Managed automatically by PlatformIO:

- AMY Synthesizer
- GFX Library for Arduino
- bb_captouch
- LVGL 8.3

## Configuration

Memory and synthesis settings are defined in `platformio.ini`, including:

- 16 MB flash
- Octal PSRAM
- 240 MHz CPU
- 120 AMY oscillators
- I2S audio support
- Serial monitor at 115200 baud

## Contributing

Contributions are welcome. Please open an issue for hardware compatibility problems or a pull request for improvements.

Before submitting hardware changes, include:

- Board and display model
- Pin configuration
- Tested firmware behavior
- Relevant serial output

## License

No license is currently included in this repository. A license should be added before reuse or redistribution.
