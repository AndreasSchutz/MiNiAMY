# HANDOFF.md — Project Continuation Context (ESP32-S3-8048S043C + AMY Synthesizer)

## 1. Current State & Working Features
- **Hardware / Firmware**: Waveshare ESP32-S3-8048S043C (800×480 RGB, GT911 touch, I2S MAX98357A, SD card).
- **Audio Engine**: Dual-core FreeRTOS architecture (`UI_Task` on Core 0, `AMY_Audio_Task` on Core 1 via lock-free ring buffer `amy_bridge.h`).
- **Engines Implemented**:
  - `OFF` (`0`), `Juno-106` (`1`, patches `0..127`), `DX7` (`2`, patches `128..255`), `PCM` (`3`, 67 samples), `Drums` (`4`, 16-track TR-808), `Piano` (`5`, patch `256` with `NUM_COMBO_COEFS = 10`).
- **Sequencer & UI**:
  - 16-step TR-808-style grid (`seq_view.h`) with per-step Note, Gate, Accent, Slide, Chord, Ratchet, Sub-step, Probability, Swing, and Polyrhythm.
  - Persistent NVS + SD card `autosave.bin` + named project presets.
  - Dedicated `DX7` operator modal (`OP1..6` Level, Coarse, Fine, Detune, Fixed, Vel, 4-stage EG Rate/Level, AMD, KeyScale) + live `dx7_algo_view.h` diagram (`ALGO #1..#32`).

## 2. Completed in This Session
1. **Verified AMY DX7 Oscillator Mapping**:
   - Confirmed via AMY's `patches.h` and `algorithms.c` (MSFA table) that `algo_source` `O2,3,4,5,6,7` maps in reverse order: `v2` (`osc 2`) = `OP6` (with feedback) down to `v7` (`osc 7`) = `OP1`.
   - Updated both `applyDx7ParamToAmy()` in `src/amy_bridge.h` (including `AMD Trem` `paramId == 53`) and `syncDx7GuiFromRomPatch()` in `src/synth_gui.h` to use `7 - op` / `7 - curOsc`.
2. **Engine Isolation & Static State Fixes in `src/amy_bridge.h`**:
   - Added `g_partEngine[16]` tracking in `AMY_MSG_PART_CONFIG` so `processAmyBridgeMsg()` only dispatches parameter changes to the active engine for that synth slot.
   - Fixed `Fixed = OFF` in DX7 operator frequency calculation to explicitly reset `*(e.freq_coefs + COEF_CONST) = 0.0f` alongside `COEF_NOTE = 1.0f`.
   - Synced DX7 `OP1..6 Level` between main matrix boxes and modal boxes in `synthParamRam`.
3. **Identified Root Cause of AMY DX7 `osc = 2..7` Filtering**:
   - In AMY's `patches_event_has_voices` / `amy.c` (`osc_ref_within_voice(d.osc, oscs_per_voice)`), `e.osc = 2..7` is rejected unless the synth's `oscs_per_voice` is set to `8` *after* `e.patch_number` loads (setting `e.oscs_per_voice = 8` in the same event as `e.patch_number` caused duplicate voice allocation / 220 Hz drone).

## 3. Next Steps (Exact Order)
1. **Verify Separate `oscs_per_voice = 8` Event for DX7 (`src/amy_bridge.h`)**:
   - Ensure `AMY_MSG_PART_CONFIG` sends `e.patch_number = m.patchOrNote;` first, followed by a separate `amy_event` with `e2.synth = amySynth; e2.oscs_per_voice = 8;` (without `patch_number`), then compile & test DX7 Algorithm 1 (`OP1..6` Level = 0 -> silence; `OP1` only -> clean sine).
2. **Sync DX7 Operator Frequency State (`opCoarse`, `opFine`, `opDetune`, `opFixed`)**:
   - Make `opCoarse[6]`, `opFine[6]`, `opDetune[6]`, `opFixed[6]` in `src/amy_bridge.h` per-part (`[16][6]`) and initialize them from `syncDx7GuiFromRomPatch()` on patch load/reset so editing `Coarse`/`Fine`/`Detune`/`Fixed` never uses stale hardcoded values.
3. **Cold-Read Audit of Piano Editor (`patch 256`)**:
   - Read `applyPianoParamToAmy()` in `src/amy_bridge.h` and `PIANO` parameter mappings in `src/synth_gui.h` to verify `NUM_COMBO_COEFS = 10` and `COEF_CONST` behavior before any edits.

## 4. Critical Developer Rules
- **Terminal / Markdown Bracket Rule**: Never put literal `[` or `]` inside Python one-liners pasted into markdown code blocks (they get stripped). Always construct brackets via `chr(91)` and `chr(93)`.
- **Single-Step Discipline**: Execute only one small, self-contained terminal command per turn, and always pipe `grep` output with `| head -n 22`.
- **Cold-Read Before Edit**: Always inspect target code via `sed`/`grep` before writing patches; do not compile until all edits for a logical step are verified.
