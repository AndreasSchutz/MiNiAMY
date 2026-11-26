# HANDOFF.md — Project Continuation Context (ESP32-S3-8048S043C + AMY Synthesizer)

## 1. Current State & Working Features
- **Hardware / Firmware**: LCDWIKI ES3C28P 2.8" IPS ESP32-S3 (240x320 ILI9341V SPI, FT6336G capacitive touch, onboard I2S ES8311 + speaker amp, microSD SDIO + 16MB Flash + 8MB OPI PSRAM). Spec PDF: https://lcdwiki.com
- **Audio Engine**: Dual-core FreeRTOS architecture (`UI_Task` on Core 0, `AMY_Audio_Task` on Core 1 via lock-free queue `amy_bridge.h`, rate-limited to max 8 messages per audio block).
- **Engines Implemented & Verified**:
  - `OFF` (`0`), `Juno-106` (`1`, patches `0..127`), `DX7` (`2`, patches `128..255`), `PCM` (`3`, 155 samples), `Drums` (`4`, 9-track TR-808), `Piano` (`5`, patch `256` `INTERP_PARTIALS`).
- **Sequencer & UI**:
  - 16-step TR-808-style grid (`seq_view.h`) with Pattern & Song modes, per-step velocity, swing, and MIDI recording.
  - Crash-free auto-save to NVS Flash (`storage_mgr.h`) + manual project `.bin` and patch `.pat` export/import.

## 2. Completed in This Session
1. **DX7 `OP Ratio` Melodic Stepping & `Coarse`/`Fine` Sync (`src/synth_gui.h`)**:
   - Added `isDx7RatioParam()` so `OP1..6 Ratio` snaps to melodic DX7 steps (`0.5, 1.0, 2.0 ... 31.0`) on both drag and left/right tap, displays `%.2f`, scales bar `0.5..31.0`, and allows full `Coarse * (1 + Fine%)` range up to `62.0`.
   - Fixed `Coarse = 0` display (`0.5`) and eliminated `0.5` division-by-zero in bidirectional `Ratio <-> Coarse/Fine` sync.
2. **Global FX Float Parameters Fix (`src/synth_gui.h`)**:
   - Excluded `Echo Fdbk` (`id 48`), `Echo Tone` (`id 148`), `Rev Size` (`id 150`), and `Rev Damp` (`id 151`) from `isIntegerParam()` on `PAGE_DX7_OP_DETAIL` so they sweep smoothly instead of snapping to min/max.
3. **Piano & PCM Fine-Tuning (`src/amy_bridge.h`)**:
   - Fixed Piano `Low/Mid/High EQ` (`id 115..117`) to target global `bus = 0` instead of `e.synth`.
   - Preserved `COEF_EG0 = 1.0f` when updating Piano `Vel Sens` (`id 107`) & `Key Scale` (`id 121`), and mapped `Body Level` (`id 119`) to `e.synth_level` scaled by `g_partVol`.
   - Added logarithmic frequency scaling (`powf`) for Piano `Brightness` (`id 104`, `200..8000 Hz`) and PCM `Cutoff` (`id 137`, `40..8000 Hz`).
4. **USB-Host MIDI Support (`src/usb_midi_host.h`, `src/main.cpp`, `platformio.ini`)**:
   - Switched `-DARDUINO_USB_MODE=0` and `-DARDUINO_USB_CDC_ON_BOOT=0` in `platformio.ini` so UART flashing/serial remains untouched while native USB OTG is freed for Host mode.
   - Added hot-plug ESP-IDF USB Host MIDI driver (`src/usb_midi_host.h`) pinned to Core 0 feeding `processRawMidiByte()`.
5. **Help Manual Updated (`src/help_manual.h`)**:
   - Documented interactive `ALGO` diagram touch gestures, `Mod Balance`, `OP Ratio` melodic steps, `Coarse & Fine` multiplication formula (`Ratio = Coarse x (1 + Fine%)`), `PCM` (`0-154` samples + Key Follow/Portamento), and updated `Piano` `TIMBRE`/`MIX` controls.

6. **Silent Backlight PWM & Hardware Pinout Documentation (`src/main.cpp`, `src/synth_gui.h`, `HANDOFF.md`)**:
   - Phase-locked LCD backlight PWM (`TFT_BL = GPIO 45`) to `AMY_SAMPLE_RATE * 2` (`88.2 kHz`, 8-bit) and set default brightness to `100%` to eliminate 1 kHz / aliasing buzz in the onboard speaker amp.
   - Verified `Expand Pin` 4P 1.25mm header provides free `GPIO 2`, `GPIO 3`, `GPIO 14`, `GPIO 21` (ready for external PCM5102 I2S DAC: `BCK=GPIO 2`, `LCK/WS=GPIO 3`, `DIN=GPIO 14`, `SCK=GND`, `3.3V/GND` from adjacent `I2C`/`UART` header).

## 3. Next Steps — Hardware Verification Checklist
1. **DX7 `Ratio` / `Coarse` / `Fine` & Help Manual**:
   - Verify `OP1..6 Ratio` snaps cleanly (`0.5, 1, 2 ... 31`) on drag/tap and displays `%.2f`.
   - Verify in OP Detail modal that `Coarse = 0` displays `0.5`, and `Coarse = 1` + `Fine = 41` updates `OP Ratio` to `1.41`.
   - Long-press sub-tabs (`DX7`, `PCM`, `PIANO`) to verify the updated context help text.
2. **Global FX Modal (`Rev Size`, `Rev Damp`, `Echo Fdbk`, `Echo Tone`)**:
   - Open `System -> FX`, double-tap `Echo Mix` and `Rev Mix` (blue frame), and verify `Rev Size` (`0.1..0.98`) and the other float FX controls sweep smoothly without snapping to min/max.
3. **End-to-End Storage Verification (`proj_01..16.bin` & `.pat`)**:
   - Configure 5 active parts (`Juno`, `DX7` with `Mod Balance` & `Ratio 1.41`, `PCM`, `Drums`, `Piano`) and verify `BASE_PARAMS = 157` (`TOTAL_PARAMS = 269`) round-trips cleanly across `.pat` export/import and `proj_01.bin` save/load.
   - *(Optional)*: Add `amyEnqueuePartConfig(...)` inside `openPatchImportModal()` in `src/synth_gui.h` so importing a `.pat` onto an `OFF` slot immediately initializes voices on Core 1.
4. **Piano & PCM Audio Audition**:
   - Test Piano `Low/Mid/High EQ`, `Body Level`, and logarithmic `Brightness` (`200..8000 Hz`) plus PCM `Cutoff` (`40..8000 Hz`) under live MIDI/sequencer playback.
5. **USB-Host MIDI Hardware Test (when USB-C OTG + Power arrives)**:
   - Connect USB-C OTG splitter with external power to the native **USB** port (keep **UART** port for flashing/debug) and verify hot-plug USB MIDI keyboard Note On/Off, CC Learn, and live sequencer step recording.
   - Test Piano `Low/Mid/High EQ`, `Body Level`, and logarithmic `Brightness` (`200..8000 Hz`) plus PCM `Cutoff` (`40..8000 Hz`) under live MIDI/sequencer playback.

## 4. Critical Developer Rules
- **Terminal / Markdown Bracket Rule**: Never put literal `[` or `]` inside Python one-liners pasted into markdown code blocks (they get stripped). Always construct brackets via `chr(91)` and `chr(93)`.
- **Single-Step Discipline**: Execute only one small, self-contained terminal command per turn, and always pipe `grep` output with `| head -n 22`.
- **Cold-Read Before Edit**: Always inspect target code via `sed`/`grep` before writing patches; do not compile until all edits for a logical step are verified.
