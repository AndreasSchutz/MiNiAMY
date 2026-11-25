# HANDOFF.md — Project Continuation Context (ESP32-S3-8048S043C + AMY Synthesizer)

## 1. Current State & Working Features
- **Hardware / Firmware**: Waveshare ESP32-S3-8048S043C (320x240 / 800x480 RGB, GT911/FT6336G touch, I2S ES8311/MAX98357A, SD card + NVS Flash).
- **Audio Engine**: Dual-core FreeRTOS architecture (`UI_Task` on Core 0, `AMY_Audio_Task` on Core 1 via lock-free queue `amy_bridge.h`, rate-limited to max 8 messages per audio block).
- **Engines Implemented & Verified**:
  - `OFF` (`0`), `Juno-106` (`1`, patches `0..127`), `DX7` (`2`, patches `128..255`), `PCM` (`3`, 155 samples), `Drums` (`4`, 9-track TR-808), `Piano` (`5`, patch `256` `INTERP_PARTIALS`).
- **Sequencer & UI**:
  - 16-step TR-808-style grid (`seq_view.h`) with Pattern & Song modes, per-step velocity, swing, and MIDI recording.
  - Crash-free auto-save to NVS Flash (`storage_mgr.h`) + manual project `.bin` and patch `.pat` export/import.

## 2. Completed in This Session
1. **DX7 Algorithm Diagram & Combined Control (`src/dx7_algo_view.h`, `src/synth_gui.h`)**:
   - Merged `ALGO` diagram (`252x138 px`) and `Algorithm` (`id 14`) into one full-width interactive `lv_bar` with lighter orange progress bar (`0x8C4E27`), CC mapping / MIDI Learn status text, auto-centered layout fitting all 32 algorithms, visual multi-op feedback loop lines for Algo 4 (`OP4-6 FB`) & Algo 6 (`OP5-6 FB`), and fixed Algo 20 carrier/link topology.
2. **DX7 `Mod Balance` (`id 155`) & Clean Sine Reset**:
   - Added `Mod Balance` (`0..100`, default `50`) next to `Feedback` on `ALG` page; scales modulator levels per-part (`[16]`) based on `dx7CarrierMask` for the active algorithm.
   - Updated `resetCurrentSynthToDefaults()` for DX7 to initialize a clean Algorithm 1 sine wave (`OP1 = 99`, `OP2..6 = 0`, `Ratio = 1`, `Feedback = 0`, neutral envelopes).
   - Synced `OP1..6 Ratio` <-> `Coarse`/`Fine`, added readable labels for `Fixed` (`ON/OFF`), `LFO Wave` (`TRI/SAW-/SAW+/SQR/SIN/NOISE`), and `LFO Sync` (`ON/OFF`), and made all DX7 operator state arrays per-part (`[16]`).
3. **PCM & Piano Editor Audits (`src/amy_bridge.h`, `src/synth_gui.h`)**:
   - **PCM (`MODE_SAMPLER`)**: Added sample name display on `Sample` (`id 130`), bidirectional sync with `Patch/Note`, per-part tune/envelope state, and fixed ADSR envelope stage mapping (`eg0` stages `0..2`).
   - **Piano (`MODE_PIANO`, `patch 256`)**: Audited `patches.h` (`w11` = `INTERP_PARTIALS` on `osc 0`), removed destructive multi-oscillator wave overrides, set voice cost to `4`, and mapped `TIMBRE`/`MIX` parameters (`Low EQ`, `Mid EQ`, `High EQ`, `Fine Tune`, `Body Level`, `Filter Env`, `Key Scale`).
4. **Core 0 & Core 1 Stability Fixes (`src/amy_bridge.h`, `src/storage_mgr.h`, `src/main.cpp`, `src/synth_gui.h`)**:
   - Eliminated Core 0 `abort()` during auto-save by switching periodic auto-save to NVS Flash (avoiding FATFS/SD DMA allocation crashes) and chunking `seqvel.bin` writes via 512-byte SRAM buffers.
   - Eliminated Core 1 `instrument_note_off` `abort()` during engine switches/resets by sending `note_off` for active sequencer notes (`g_activeNotes`) before freeing voices (`num_voices = 0`) in `AMY_MSG_PART_CONFIG`.
   - Added duplicate-value guard in `applyParameterValue()` and label text diffing in `styleParamWidget()` to prevent LVGL heap churn and AMY event queue overflow.

## 3. Next Steps (Exact Order)
1. **End-to-End Hardware Verification**:
   - Test saving/loading full projects (`proj_01..16.bin`) and single patches (`.pat`) on SD/SPIFFS across all 5 synth engines (`Juno`, `DX7`, `PCM`, `Drums`, `Piano`) to confirm `BASE_PARAMS = 157` (`TOTAL_PARAMS = 269`) round-trips cleanly.
2. **Piano & PCM Fine-Tuning**:
   - Audition all 17 Piano parameters (`TONE`, `TIMBRE`, `MIX`, `ENV`) and PCM loop/filter parameters under live MIDI/sequencer playback and adjust scaling curves if needed.
3. **Help Manual Update (`src/help_manual.h`)**:
   - Document the combined `ALGO` touch control, `Mod Balance`, DX7 sine-wave reset, and updated Piano `TIMBRE`/`MIX` controls in the built-in context help.

## 4. Critical Developer Rules
- **Terminal / Markdown Bracket Rule**: Never put literal `[` or `]` inside Python one-liners pasted into markdown code blocks (they get stripped). Always construct brackets via `chr(91)` and `chr(93)`.
- **Single-Step Discipline**: Execute only one small, self-contained terminal command per turn, and always pipe `grep` output with `| head -n 22`.
- **Cold-Read Before Edit**: Always inspect target code via `sed`/`grep` before writing patches; do not compile until all edits for a logical step are verified.
