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
1. **1-till-1 Sequencer (`T1–T16` = `Synth 1–16`)**:
   - Varje spår styr motsvarande synthplats (`T1:Kit`, `T2:Juno`, `T3:DX7` i `Demo Presets`).
   - Individuell spår-`MUTE` och oberoende rösthantering (`ENG_DRUMS` triggas via direkt AMY `e.osc = 100 + slot`, `e.wave = PCM`, `e.preset = pcm`).
2. **Polyfonisk 3×3 Drum-Pad + Velocity-modal på dubbeltryck**:
   - Dubbeltryck på steg i `Kit`-spår öppnar 3×3 pad-modal (`D1–D9`) med swipe för velocity (`1–127`, lagrat i PSRAM `seqDrumVel` + `seqStepVel`).
   - Smart "penselminne" (`seqKitBrush`): senaste valda trum-kombination/velocity i modalen blir förval vid efterföljande enkelklick på tomma steg.
3. **System -> FILE & DISP**:
   - `Factory Rst` sätter alla 16 syntar till `Off`, `LED Bright = 0` (`OFF`) och återställer allt till default.
   - `Demo Presets` laddar `Synth 1 = Drums`, `Synth 2 = Juno`, `Synth 3 = DX7` samt 3-spårs demopattern.
   - `Project File` & `Patch File`: enkelklick stegar utan flimmer (efter `dblWin`), dubbeltryck sparar/exporterar, long-press öppnar import-modal.
   - `LED Bright` (`0–20`, default `0`) under `System -> DISP` styr RGB Mood-LED (färg per läge/synthmotor + kraftig puls `20 -> 255 -> 20` vid musik/MIDI/touch).

## Kvarstående / Nästa steg
1. **Kodstädning**: Ta bort oanvända `_unusedDrumLog` i `src/synth_gui.h` och verifiera varningsfri kompilering i `src/`.
2. **Finjusteringar efter speltest** utifrån användarens feedback via Git-repot.

