# HANDOFF: MiNi-AMY (ESP32-S3-WROOM + AMY + LVGL 8.3.11)

## Arbetssätt & Regler
- **Språk**: Svenska i all dialog.
- **Korta steg**: Håll varje terminalblock/patch under **25–35 rader** (dela upp större ändringar i 2–3 numrerade delsteg).
- **Direkt verifiering**: Avsluta kodändringar med `pio run -t upload` och låt användaren testa direkt på hårdvaran mellan stegen.
- **Minneshantering**: Använd `ps_malloc` / `ps_calloc` / `alloca` för större buffertar och strängar för att hålla stack och intern SRAM fri.
- **Kodåtkomst**: Läs källkodsfiler direkt från användarens Git-repo innan patchar genereras (undvik onödiga `sed`/`grep`-rundor när filen finns i repot).

## Hårdvara & Miljö
- **MCU**: Freenove ESP32-S3-WROOM (16MB QIO Flash, 8MB OPI PSRAM, PlatformIO `espressif32`).
- **Display**: 2.4" ST7789 (320×240, SPI: MOSI=11, SCK=12, CS=10, DC=9, RST=14, BL=21). Vänsterflikar (`LV_DIR_LEFT`, bredd `64px`, höjd `150px` för 3×50px flikar) + Stat-bar längst ner till vänster.
- **Touch**: FT6336G I2C (SDA=47, SCL=48, RST=13, INT=3).
- **Audio**: ES8311 I2S DAC (MCLK=4, BCLK=5, LRC=7, DOUT=8, DIN=6, PA=18). AMY 120 oscs på Core 1 (`amy_start(cfg)` med `i2s_dout=-1`, manuell `amy_render_audio()` -> `amyI2s.write()`).
- **MIDI**: DIN/TRS UART RX=43, TX=44 (31250 baud).
- **Storage**: SD-kort SPI (CS=39, MOSI=38, MCLK/SCK=40, MISO=41) med fallback till LittleFS (`/AMY/autosave.bin`, `/AMY/seqvel.bin`, `/AMY/PROJECTS/`, `/AMY/PATCHES/`).
- **RGB Mood-LED**: WS2812 på **GPIO 42** via `rgbLedWrite(42, r, g, b)`.

## Senast färdigställt
1. **Utökad PCM- & Trumbank (155 samplingar, Gamma9001 + TR-808)**:
   - Aktiverade `-DGAMMA9001` i `platformio.ini`, genererade `src/drums.bin` (3.6 MB) + `src/drums_bin.c` (`.incbin` i Flash) och kopplade `amy_set_gamma9001_pcm(gamma9001_pcm_data)` i `src/amy_bridge.h`.
   - Uppdaterade `src/patch_names.h`, `src/amy_bridge.h` och `src/synth_gui.h` med alla **155 ljuden (`0–154`)**: `0–18` (TR-808 ROM) samt `19–154` mappade till AMY-preset `256–391` (TR-909, LinnDrum 9000, MR-12, Power Kit, Percussion, Acoustic & Synthetics).
2. **PCM-synthens editor (`PCM -> PLAYER`)**:
   - Fixade uppspelning i `ENG_SAMPLER` genom att sätta `e.wave = PCM` och `e.preset` korrekt i `src/amy_bridge.h`.
   - Tog bort dubbletterna `Sample` (ID `130`) och `Level` (ID `135`) från `PCM -> PLAYER` i `src/synth_gui.h` så att sampling väljs via `System -> SYNTH -> Patch/Note` och volym via `Synth Vol` (konsekvent med Juno/DX7/Piano).
3. **UI- & Kodstädning**:
   - Lade till `id == 146` (`Max Pat`) i `isIntegerParam()` i `src/synth_gui.h` så värdet visas som heltal (`1–16` istället för `1.0–16.0`).
   - Tog bort oanvända `_unusedDrumLog` i `src/synth_gui.h`.

## Kvarstående / Nästa steg
1. **Utöka `PCM -> PLAYER` (2 lediga platser i 3×2-rutnätet)**:
   - Lägg till **Key Follow / Key Track** (`freq_coefs[COEF_NOTE]` `0.0` vs `1.0`, för att kunna spela samplingar både kromatiskt och med fast original-pitch) och **Portamento / Glide** (`portamento_ms`, `0–1000 ms`).
   - Valfritt/framöver: Loop Start / Loop End (`loopstart` / `loopend`) samt laddning av egna `.wav`-filer från SD-kortet (`/AMY/samples/`) till PSRAM.
2. **Git-hantering & Snabbare flashning**:
   - Lägg till `src/drums.bin` i `.gitignore` (kan återskapas vid behov från `shorepine/amy`) eller överväg egen flash-partition / `upload_speed = 921600` i `platformio.ini` för snabbare uppladdning.
3. **Fortsatta finjusteringar efter speltest**.

