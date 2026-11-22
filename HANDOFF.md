# HANDOFF: MiNi-AMY (ESP32-S3-WROOM + AMY + LVGL 8.3.11)

## Arbetssätt & Regler
- **Språk**: Svenska i all dialog.
- **Korta steg**: Håll varje terminalblock/patch under **25–35 rader** (dela upp större ändringar i 2–3 numrerade delsteg).
- **Cold-Read Audit före kodändring**: Läs hela den berörda källkods-/bibliotekskedjan i ett rent inspektionssteg (0 kompileringar) innan någon patch skrivs.
- **Färre kompileringar (Buntning)**: Det stora PCM-biblioteket (`src/drums.bin`, 3.6 MB) gör uppladdningen långsam. Bunta ihop relaterade kodändringar i ett enda Python-skript före `pio run -t upload` (`upload_speed = 921600` är aktiverat i `platformio.ini`).
- **Hakparenteser & URL:er i markdown**: Använd **aldrig** råa hakparenteser för array-deklarationer i C++-kodblock (använd `chr(91)` + `chr(93)` i Python-patchar) och lägg långa GitHub-URL:er i en bash-variabel (`REPO=...`) så att markdown-parsern aldrig klipper dem.
- **Minneshantering**: Använd `ps_malloc` / `ps_calloc` / `alloca` för större buffertar och strängar för att hålla stack och intern SRAM fri.
- **Git-arbetsflöde**: Projektet är kopplat till `AndreasSchutz/MiNiAMY` (`main`) med `credential.helper store` och `.gitignore` som exkluderar `.pio/` och `src/drums.bin`. Använd `git add -A && git commit -m "..." && git push` vid milstolpar så filer kan hämtas via `://githubusercontent.com` i början av nästa session.

## Hårdvara & Miljö
- **MCU**: Freenove ESP32-S3-WROOM (16MB QIO Flash, 8MB OPI PSRAM, PlatformIO `espressif32`, `upload_speed = 921600`).
- **Display**: 2.4" ST7789 (320×240, SPI: MOSI=11, SCK=12, CS=10, DC=9, RST=14, BL=21). Vänsterflikar (`LV_DIR_LEFT`, bredd `64px`, höjd `150px` för 3×50px flikar) + Stat-bar längst ner till vänster.
- **Touch**: FT6336G I2C (SDA=47, SCL=48, RST=13, INT=3).
- **Audio**: ES8311 I2S DAC (MCLK=4, BCLK=5, LRC=7, DOUT=8, DIN=6, PA=18). AMY 120 oscs på Core 1 (`amy_start(cfg)` med `i2s_dout=-1`, manuell `amy_render_audio()` -> `amyI2s.write()`).
- **MIDI**: DIN/TRS UART RX=43, TX=44 (31250 baud).
- **Storage**: SD-kort SPI (CS=39, MOSI=38, MCLK/SCK=40, MISO=41) med fallback till LittleFS (`/AMY/autosave.bin`, `/AMY/seqvel.bin`, `/AMY/PROJECTS/`, `/AMY/PATCHES/`).
- **RGB Mood-LED**: WS2812 på **GPIO 42** via `rgbLedWrite(42, r, g, b)`.

## AMY Biblioteksfakta (Verifierat i `.pio/libdeps/.../AMY Synthesizer/src/`)
- **`NUM_COMBO_COEFS` är `10` (`0..9`)**: `COEF_CONST=0, COEF_NOTE=1, COEF_VEL=2, COEF_EG0=3, COEF_EG1=4, COEF_MOD0=5, COEF_BEND=6, COEF_EXT0=7, COEF_EXT1=8, COEF_MOD1=9`. Nollställ alltid samtliga `0..9` vid omskrivning av `amp_coefs` eller `duty_coefs`.
- **Juno-106 Röstarkitektur (`patch 0–127`, `v0–v5`)**:
  - **`v0`**: Master/Algo (`w20`), kedjad `c2->c3->c4->c5`. Bär huvudfiltret (`FILTER_LPF24`), VCA-envelopen (`eg0` med 3 breakpoints: Attack, Decay/Sustain, Release) och chorus-init.
  - **`v1`**: LFO (`TRIANGLE`/`SINE`/`SAW_UP`/`PULSE`/`NOISE`, fast frekvens `COEF_NOTE=0`).
  - **`v2`**: Pulse (`w1` / `PULSE`, grundfrekvens `220 Hz` vid `8'`, styrs via `amp_coefs` och `duty_coefs[0..9]`).
  - **`v3`**: Saw (`w3` / `SAW_UP`, grundfrekvens `220 Hz` vid `8'`, styrs via `amp_coefs`).
  - **`v4`**: Sub Osc (`w1` / `PULSE`, grundfrekvens `110 Hz` vid `8'`, styrs via `amp_coefs`).
  - **`v5`**: Noise (`w5` / `NOISE`, styrs via `amp_coefs`).
- **VCA & Volym i AMY**: Rör aldrig `amp_coefs` på `v0` vid volymändring (orsakar hängande toner om `COEF_CONST > 0`). Styr synthvolym och VCA Level enbart via `e.synth_level` och 3-stegs `eg0`.

## Senast färdigställt
1. **Utökad PCM- & Trumbank (155 samplingar, Gamma9001 + TR-808)**:
   - Aktiverade `-DGAMMA9001` i `platformio.ini`, genererade `src/drums.bin` (3.6 MB) + `src/drums_bin.c` (`.incbin` i Flash) och kopplade `amy_set_gamma9001_pcm(gamma9001_pcm_data)` i `src/amy_bridge.h`.
2. **Juno-106 Editor (`src/amy_bridge.h` & `src/synth_gui.h`)**:
   - Mappade alla 26 Juno-reglage till rätt oscillatorer (`v0–v5`) i `applyJunoParamToAmy()` med full 10-koefficienters rensning på DCO-nivåer och PWM.
   - Flyttade `VCA Level` (`paramId 45`) till `e.synth_level` och `VCA Mode` (`ENV`/`GATE`) till 3-stegs `eg0` (eliminerade hängande toner).
   - Flyttade Juno `HPF` (`paramId 8`) från globala `eq_l` till lokalt `FILTER_HPF` på källoscillatorerna `v2–v5`, samt kopplade `Chorus` (`paramId 13`) till buss 0.
   - Lade till `syncJunoGuiFromRomPatch()` i `src/synth_gui.h` (inkluderar `<patches.h>`) som läser in ROM-patchens faktiska parametrar till reglagen vid byte av `Patch/Note` (`0–127`) och synkar till AMY.
   - Gjorde **Reset Synth** på Juno helt patch-oberoende (`Init Juno`): återställer samtliga 26 parametrar till rena grundvärden (`defaultVal`, med `Chorus = 0`) och synkar dem direkt till AMY så ljud kan byggas från scratch.
   - Lade till tydliga UI-etiketter för Juno: `ON`/`OFF` (Saw, Pulse), `MAN`/`LFO` (PWM Mode), `16'`/`8'`/`4'` (Octave), `POS (+)`/`NEG (-)` (Env Pol), `ENV`/`GATE` (VCA Mode) och `OFF`/`LVL 1–3` (HPF).
   - Separerade `Synth Vol` (`SYS_PARAM_PART_LVL`) till ett eget `AMY_MSG_PART_LEVEL`-event så att volymändring inte laddar om ROM-patchen och skriver över skruvade reglage.
3. **Git- & Byggoptimering**:
   - Kopplade projektet till GitHub (`AndreasSchutz/MiNiAMY`), skapade `.gitignore` och satte `upload_speed = 921600` i `platformio.ini`.

## Kvarstående / Nästa steg
1. **Utöka `PCM -> PLAYER` (2 lediga platser i 3×2-rutnätet)**:
   - Lägg till **Key Follow / Key Track** (`freq_coefs[COEF_NOTE]` `0.0` vs `1.0`) och **Portamento / Glide** (`portamento_ms`, `0–1000 ms`).
   - Valfritt/framöver: Loop Start / Loop End (`loopstart` / `loopend`) samt laddning av egna `.wav`-filer från SD-kortet (`/AMY/samples/`) till PSRAM.
2. **Genomgång & verifiering av DX7- / Piano-editorerna**:
   - Gör en samlad Cold-Read Audit av `applyDx7ParamToAmy()` och `applyPianoParamToAmy()` mot `NUM_COMBO_COEFS = 10` och AMY:s FM/Piano-patchar.
3. **Fortsatta finjusteringar efter speltest**.
