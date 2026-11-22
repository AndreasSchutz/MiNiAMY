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
- **Display**: 2.4" ST7789 (320×240, SPI: MOSI=11, SCK=12, CS=10, DC=46, RST=-1, BL=45). Vänsterflikar (`LV_DIR_LEFT`, bredd `64px`, höjd `150px` för 3×50px flikar) + Stat-bar längst ner till vänster.
- **Touch**: FT6336G I2C (SDA=47, SCL=48, RST=13, INT=3).
- **Audio**: ES8311 I2S DAC (MCLK=4, BCLK=5, LRC=7, DOUT=8, DIN=6, PA=18). AMY 120 oscs på Core 1 (`amy_start(cfg)` med `i2s_dout=-1`, manuell `amy_render_audio()` -> `amyI2s.write()`).
- **MIDI**: DIN/TRS UART RX=43, TX=44 (31250 baud).
- **Storage**: SD-kort SPI med fallback till SPIFFS/NVS (`/AMY/autosave.bin`, `/AMY/seqvel.bin`, `/AMY/projects/`, `/AMY/patches/`, samt dedikerad NVS-namespace `"amy_sys"` för `DISP`/systeminställningar).
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
- **DX7 Röstarkitektur (`patch 128–255`, `v0–v7`)**:
  - **`v0`**: Master ALGO (`w8`), bär `o` (Algorithm 1–32), `b` (Feedback `0.0–1.0`), `f` (PMD Vibrato via `COEF_MOD`) och `A` (Pitch EG via `eg0`).
  - **`v1`**: LFO (`w0..w5`), bär `f` (LFO Speed) och `A` (LFO Delay).
  - **`v2..v7`**: Operatorerna **OP1..OP6** (`osc = op + 2`), där `a` = Level (`0..2.0`), `I` = Ratio och `A` = 5-pars Rate/Level-envelope (`eg0_times[1..4]`, `eg0_values[1..4]`).
- **VCA & Volym i AMY**: Rör aldrig `amp_coefs` på `v0` vid volymändring (orsakar hängande toner om `COEF_CONST > 0`). Styr synthvolym och VCA Level enbart via `e.synth_level` och 3-stegs `eg0`.

## Senast färdigställt
1. **Juno-106 Editor – Autentisk 0–10-skala, 3×3 DCO & Vertikala ADSR-reglar (`src/synth_gui.h` & `src/amy_bridge.h`)**:
   - Gav samtliga kontinuerliga Juno-reglage (`PWM`, `DCO LFO`, `Sub Osc`, `Noise`, `Portamento`, `LFO Rate`, `LFO Delay`, `Cutoff`, `Res`, `Env Mod`, `LFO Mod`, `Key Follow`, `Attack`, `Decay`, `Sustain`, `Release`, `VCA Level`) en enhetlig **0–10-skala i rena heltal** (inga decimaler) med musikalisk logaritmisk/kvadratisk mappning till AMY och exakt invers vid ROM-patchladdning.
   - Gjorde **DCO-fliken** till ett komplett **3×3-rutnät (`82×67 px`)** i originalordning: Rad 1 (`Saw`, `Pulse`, `PWM Mode`), Rad 2 (`Octave`, `DCO LFO`, `PWM`), Rad 3 (`Sub Osc`, `Noise`, `Portamento`).
   - Bytte plats på `Env Pol` och `Env Mod` under **VCF** (`Cutoff`, `Res`, `Env Pol` / `Env Mod`, `LFO Mod`, `Key Follow`).
   - Gjorde om **ENV-fliken** med `VCA Mode` och `VCA Level` överst samt **4 stående ADSR-reglar (`61×136 px`)** i rad längst ner (`Attack`, `Decay`, `Sustain`, `Release`) med vertikal touch-dragning nerifrån och upp.
   - Uppdaterade `syncJunoGuiFromRomPatch()` så att samtliga 26 Juno-reglage (inkl. `LFO Wave` och `Portamento`) nollställs till grundvärden före patchinläsning.
2. **DX7 Editor – Rätt oscillatorer (`v2..v7`), ROM-synk & Live ENV-graf (`src/amy_bridge.h` & `src/synth_gui.h`)**:
   - Rättade `applyDx7ParamToAmy()` så OP1–OP6 styr **`osc = 2..7` (`v2..v7`)**, LFO styr **`osc = 1` (`v1`)** och Algo/Feedback/Octave/PMD/Pitch EG styr **`osc = 0` (`v0`)**.
   - Lade till `syncDx7GuiFromRomPatch()` som synkar `Algorithm`, `Feedback`, `PMD Vib`, `LFO Speed/Delay/Wave` samt alla 6 operatorernas `Level`, `Ratio/Coarse` och 8-parametrs `EG Rate/Lvl 1–4` vid byte av DX7-patch (`128–255`).
   - Byggde om operator-modalen (`< OPx | OSC | ENV | MOD`) med en samlad **`ENV`-flik**: en **live-uppdaterad `lv_line`-envelopegraf (`250×64 px`)** med streckad sustain-linje och **5 lysande LED-kulor (`0x82FA00`)** på brytpunkterna, samt **8 höga stående reglar (`29×138 px`)** i ordningen `R1 | L1 | R2 | L2 | R3 | L3 | R4 | L4` direkt under grafen.
3. **System / DISP NVS-persistens & 10% Bakgrundsljus (`src/synth_gui.h` & `src/main.cpp`)**:
   - Sparar systeminställningar (`SYS_PARAM_BRIGHTNESS`, `SYS_PARAM_MASTER_VOL`, `SYS_PARAM_DBLCLICK`, `SYS_PARAM_FLIP`, `SYS_PARAM_DIM_TIMEOUT`, `SYS_PARAM_LED_BRIGHT`) omedelbart i egen NVS-namespace (`"amy_sys"`) så de överlever både omstarter och reflash (`pio run -t upload`).
   - Sänkte lägsta bakgrundsljus på både `Brightness`-knappen och `autodim` till **10% (PWM `26`)** anpassat för nya displaymodulen.

## Kvarstående / Nästa steg
1. **Utöka `PCM -> PLAYER` (2 lediga platser i 3×2-rutnätet)**:
   - Lägg till **Key Follow / Key Track** (`freq_coefs[COEF_NOTE]` `0.0` vs `1.0`) och **Portamento / Glide** (`portamento_ms`, `0–10` -> `0–1000 ms`).
   - Valfritt/framöver: Loop Start / Loop End (`loopstart` / `loopend`) samt laddning av egna `.wav`-filer från SD-kortet (`/AMY/samples/`) till PSRAM.
2. **Genomgång & verifiering av Piano-editorn**:
   - Gör en samlad Cold-Read Audit av `applyPianoParamToAmy()` mot `NUM_COMBO_COEFS = 10` och AMY:s `patch 256` (`dpwe piano`).
3. **Fortsatta finjusteringar efter speltest** (bl.a. speltest av nya DX7 `ENV`-grafen & NVS-persistens för `DblClick`).
