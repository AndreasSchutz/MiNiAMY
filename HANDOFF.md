# MiNi-AMY — Project Handoff & Status (2026-11-26)

## Hardware & Build Target
- **Board:** LCDWIKI ES3C28P (ESP32-S3R8, 16MB QIO Flash, 8MB Octal OPI PSRAM, 2.8" ILI9341 320x240 capacitive touch FT6336G, ES8311 audio codec on I2S/I2C).
- **Build Environment:** PlatformIO (`env:esp32-s3-display`), `ARDUINO_USB_MODE=0` (USB Host MIDI active on GPIO 19/20).
- **Audio Output:** 44.1 kHz stereo I2S to ES8311 + 88.2 kHz silent PWM backlight (`TFT_BL = GPIO 45`) to eliminate coil whine.

## Verified & Completed Features
1. **DX7 6-Op FM Engine (`src/amy_bridge.h`, `src/synth_gui.h`):**
   - Fixed `setOpLvl` and `syncDx7GuiFromRomPatch` to match AMY's `patches.h` coefficient layout (`COEF_CONST = (lv99/99)*2`, `COEF_EG0 = 1.0`).
   - Moved DX7 Pitch EG (`paramId 420..427`) to `eg1 / COEF_EG1` so `osc 0` amplitude envelope (`eg0`) is never overwritten.
   - Clean DX7 Init reset (`long-press DX7 tab -> YES`): sets soft musical EG rates (`95, 60, 50, 65`), clears global Chorus (`bus = 0`, `paramId 13`), mutes LFO (`osc 1`) at speed `0`, and leaves ROM patches intact when switching engine to DX7.
   - Hardware-verified with C3/C5 sequencer playback and `OP1..6` Level/Ratio modulation.
2. **Global FX Modals (`src/synth_gui.h`):**
   - Double-tap on `Echo Mix` or `Rev Mix` opens modal with `Echo Time` (`10..1000 ms`), `Echo Fdbk`, `Echo Tone`, `Rev Size`, `Rev Damp`, and `Rev Xover` (`200..8000 Hz`).
   - All 6 level/feedback/tone/size/damp FX controls (`IDs 46, 149, 48, 148, 150, 151`) now step cleanly `0..10` in integer increments synced to AMY's internal ranges.
   - Hardware-verified.
3. **UI Task & Diagnostics (`src/main.cpp`, `src/synth_gui.h`, `src/storage_mgr.h`):**
   - `UI_Task` stack increased to `32768` bytes.
   - Replaced dynamic `lv_timer_create` / `lv_timer_del` double-click handling with a static 15 ms runner (`pendDueMs`).
   - Added `esp_reset_reason()` and NVS crash checkpoint (`Chk X`) readout to the `SYSTEM STATISTICS` modal (double-tap `OSC` meter).

## Open Issue & Exact Next Steps
### 1. Fix `ESP_RST_PANIC` (Reset Reason 4) on `Project File` / `Patch File` Save
- **Symptom:** MicroSD mounts cleanly at boot (`Storage: SD CARD (FAT32)` on `FSPI`, pins `CS=47, MOSI=40, SCK=38, MISO=39`), and `Demo Presets` loads/plays without issues. However, double-tapping `Project File` (`saveProjectToDisk`) or `Patch File` (`saveSinglePatchToDisk`) triggers an immediate `ESP_RST_PANIC` (`Reset Reason: 4`).
- **Next Actions:**
  1. Test `SPIFFS` redirection (or root path `/proj_01.bin`) in `src/storage_mgr.h` to isolate whether `SD.open("/AMY/projects/proj_01.bin", FILE_WRITE)` is panicking inside FatFs/SPI DMA on Core 0.
  2. If needed, temporarily set `-DARDUINO_USB_MODE=1 -DARDUINO_USB_CDC_ON_BOOT=1` in `platformio.ini` for a single flash to capture the exact `Guru Meditation Error` + `Backtrace` over USB serial, then restore `ARDUINO_USB_MODE=0`.
  3. Complete end-to-end verification of `proj_01.bin` save/load and `.pat` export/import.

### 2. Remaining Hardware Verification
- **Piano & PCM Audio Audition:** Test Piano `Low/Mid/High EQ` (`TIMBRE`), `Body Level` (`MIX`), logarithmic `Brightness` (`TONE`), and PCM sample browsing (`0..154`) + logarithmic `Cutoff` (`FLT`).
- **Pending External Hardware:** USB-Host OTG MIDI keyboard test (waiting on OTG adapter) and external stereo DAC test (waiting on PCM5102 module).
