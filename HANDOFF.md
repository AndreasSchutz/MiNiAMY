# MiNi-AMY — Project Handoff & Status (2026-11-26)

## Hardware & Build Target
- **Board:** LCDWIKI ES3C28P (ESP32-S3R8, 16MB QIO Flash, 8MB Octal OPI PSRAM, 2.8" ILI9341 320x240 capacitive touch FT6336G, ES8311 audio codec on I2S/I2C).
- **Build Environment:** PlatformIO (`env:esp32-s3-display`), `ARDUINO_USB_MODE=1` & `ARDUINO_USB_CDC_ON_BOOT=1` (USB CDC auto-upload & serial active; `startUsbMidiHost()` temporarily commented out in `src/main.cpp` until OTG power cable arrives).
- **Audio Output:** 44.1 kHz stereo I2S to ES8311 + 88.2 kHz silent PWM backlight (`TFT_BL = GPIO 45`) to eliminate coil whine.
- **Storage:** MicroSD on `HSPI` (`CS=47, MOSI=40, SCK=38, MISO=39`) with automatic fallback to internal `SPIFFS` / `NVS`.

## Verified & Completed Features
1. **DX7 6-Op FM Engine (`src/amy_bridge.h`, `src/synth_gui.h`):**
   - Fixed `setOpLvl` and `syncDx7GuiFromRomPatch` to match AMY's `patches.h` coefficient layout (`COEF_CONST = (lv99/99)*2`, `COEF_EG0 = 1.0`).
   - Moved DX7 Pitch EG (`paramId 420..427`) to `eg1 / COEF_EG1` so `osc 0` amplitude envelope (`eg0`) is never overwritten.
   - Clean DX7 Init reset (`long-press DX7 tab -> YES`): sets soft musical EG rates (`95, 60, 50, 65`), clears global Chorus (`bus = 0`, `paramId 13`), mutes LFO (`osc 1`) at speed `0`, and leaves ROM patches intact when switching engine to DX7.
   - Hardware-verified with C3/C5 sequencer playback and `OP1..6` Level/Ratio modulation.
2. **Global FX Modals (`src/synth_gui.h`):**
   - Double-tap on `Echo Mix` or `Rev Mix` opens modal with `Echo Time` (`10..1000 ms`), `Echo Fdbk`, `Echo Tone`, `Rev Size`, `Rev Damp`, and `Rev Xover` (`200..8000 Hz`).
   - All 6 level/feedback/tone/size/damp FX controls (`IDs 46, 149, 48, 148, 150, 151`) step cleanly `0..10` in integer increments synced to AMY's internal ranges.
   - Hardware-verified.
3. **UI Task & Diagnostics (`src/main.cpp`, `src/synth_gui.h`, `src/storage_mgr.h`):**
   - `UI_Task` stack set to `32768` bytes on Core 0; AMY audio task pinned to Core 1.
   - Static 15 ms runner (`pendDueMs`) for double-click handling.
   - `esp_reset_reason()` and NVS crash checkpoint (`Chk X`) readout in `SYSTEM STATISTICS` modal (double-tap `OSC` meter).
4. **SD Card Project Save & `.pat` Patch Export (`src/lv_conf.h`, `src/storage_mgr.h`) [RESOLVED & VERIFIED]:**
   - **Root Cause & Fix:** LVGL previously allocated ~197 KB from internal SRAM (`MALLOC_CAP_INTERNAL`), leaving only `924 B` free after `create_synth_gui()`, causing `SD.open(..., FILE_WRITE)` -> `fopen()` to abort when creating its FreeRTOS mutex (`lock_init_generic`). Routed `LV_MEM_CUSTOM_ALLOC` / `LV_MEM_CUSTOM_REALLOC` in `src/lv_conf.h` to `heap_caps_malloc(..., MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)`, freeing ~202 KB internal SRAM (`202168 B` free after GUI) and fixing both SD save/export and GPIO 42 RMT LED mutex init.
   - Hardware-verified saving `proj_01.bin` and exporting `.pat` files to SD card without panics.
5. **PCM / Sampler Engine (`src/amy_bridge.h`, `src/synth_gui.h`):**
   - PCM sample browsing (`0..154`) verified on hardware.
   - Fixed `Tune` (`paramId 131`, `-24..+24` st) and `Fine Tune` (`paramId 132`, `-50..+50` ct) in `applySamplerParamToAmy`: AMY ignores `e.ratio` for `PCM` waves and uses `freq_coefs[COEF_CONST]` (`semitones / 12.0f`). Pitch offset is now also preserved when changing `Sample` (`paramId 130`).
6. **Piano Engine Streamlining & Reset (`src/synth_gui.h`, `src/amy_bridge.h`):**
   - Streamlined `MODE_PIANO` (`patch 256`, 21-oscillator additive model) to **6 musical play/mix controls on a single page (`PAGE_OSC`)**: `Body Level` (`119`), `Brightness` (`104`, bypasses LPF via `FILTER_NONE` when `>= 7800 Hz`, default `8000 Hz`), `Fine Tune` (`118`, mapped to `freq_coefs[COEF_CONST]` for `±100` cents across all piano oscillators), `Stereo Wd` (`114`), `Vel Sens` (`107`), and `Release` (`111`). Removed destructive per-oscillator envelope controls and global `bus = 0` EQ controls.
   - **Piano Patch Reload Reset:** Long-pressing the `PIANO` sub-tab (`createStyledSubTabview`, `sec == 0 && engine == ENG_PIANO`) resets the 6 Piano parameters to defaults and reloads `patch 256` cleanly via `amyEnqueuePartConfig()`.

## Remaining Hardware Verification & Next Steps
1. **Verify Latest Flashed Tweaks on Hardware:**
   - Test **long-press on `PIANO` sub-tab** to confirm clean reload of `patch 256` and parameter reset.
   - Test **PCM / Sampler `Tune` (`-24..+24`)**, `Fine Tune` (`-50..+50`), and logarithmic `Cutoff` (`FLT`).
2. **End-to-End SD Load/Import Verification:**
   - Verify loading saved `proj_01.bin` (`Load Project`) and importing exported `.pat` files from `/AMY/patches/` back into active synth parts.
3. **Pending External Hardware:**
   - **USB-Host OTG MIDI Keyboard:** Once OTG power cable arrives, uncomment `startUsbMidiHost();` in `src/main.cpp` and switch `platformio.ini` back to `-DARDUINO_USB_MODE=0 -DARDUINO_USB_CDC_ON_BOOT=0`.
   - **External Stereo DAC:** Test PCM5102 I2S DAC module once received.
