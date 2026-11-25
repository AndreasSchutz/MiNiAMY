#pragma once
#include <Arduino.h>
#include <lvgl.h>

static lv_obj_t *g_helpModal = nullptr;
static lv_obj_t *g_helpTitle = nullptr;
static lv_obj_t *g_helpBody  = nullptr;

inline void buildContextHelpModal(lv_obj_t *parent) {
    g_helpModal = lv_obj_create(parent);
    lv_obj_set_size(g_helpModal, 250, 226);
    lv_obj_center(g_helpModal);
    lv_obj_set_style_bg_color(g_helpModal, lv_color_hex(0x25252A), 0);
    lv_obj_set_style_border_color(g_helpModal, lv_color_hex(0xFF5500), 0);
    lv_obj_set_style_border_width(g_helpModal, 2, 0);
    lv_obj_set_style_pad_all(g_helpModal, 6, 0);
    lv_obj_clear_flag(g_helpModal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_helpModal, LV_OBJ_FLAG_HIDDEN);

    g_helpTitle = lv_label_create(g_helpModal);
    lv_obj_set_style_text_color(g_helpTitle, lv_color_hex(0xFF5500), 0);
    lv_obj_align(g_helpTitle, LV_ALIGN_TOP_LEFT, 2, 2);

    lv_obj_t *btnClose = lv_btn_create(g_helpModal);
    lv_obj_set_size(btnClose, 54, 24);
    lv_obj_align(btnClose, LV_ALIGN_TOP_RIGHT, 0, -2);
    lv_obj_set_style_bg_color(btnClose, lv_color_hex(0xE63900), 0);
    lv_obj_t *lClose = lv_label_create(btnClose);
    lv_label_set_text(lClose, "CLOSE");
    lv_obj_center(lClose);
    lv_obj_add_event_cb(btnClose, [](lv_event_t *e) {
        lv_obj_add_flag(g_helpModal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *scrollBox = lv_obj_create(g_helpModal);
    lv_obj_set_size(scrollBox, 234, 182);
    lv_obj_align(scrollBox, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(scrollBox, lv_color_hex(0x1E1E22), 0);
    lv_obj_set_style_border_width(scrollBox, 0, 0);
    lv_obj_set_style_pad_all(scrollBox, 6, 0);

    g_helpBody = lv_label_create(scrollBox);
    lv_label_set_long_mode(g_helpBody, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(g_helpBody, 218);
    lv_obj_set_style_text_color(g_helpBody, lv_color_hex(0xEFECE6), 0);
}

inline void showContextHelp(const char *title, const char *body) {
    if (!g_helpModal || !g_helpTitle || !g_helpBody) return;
    lv_label_set_text(g_helpTitle, title);
    lv_label_set_text(g_helpBody, body);
    lv_obj_clear_flag(g_helpModal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(g_helpModal);
}

inline void openHelpForSection(int secId, uint16_t tabIdx) {
    if (secId == 0) { // JUNO
        if (tabIdx == 0) showContextHelp("HELP: JUNO DCO", "Mix Saw, Pulse & Sub Osc.\nPWM sets pulse width.\nPWM Mode switches manual vs LFO modulation.\n\nGestures:\n- Drag box: smooth sweep\n- Tap left/right: fine step\n- Double-tap (blue frame): edit/sub-page\n- Double-tap standard box: reset default\n- Long-press box: MIDI Learn");
        else if (tabIdx == 1) showContextHelp("HELP: JUNO LFO", "Modulates pitch & filter.\nLFO Rate sets speed (Hz).\nLFO Delay fades in modulation.\nOctave shifts -1/0/+1.\nLFO Wave selects TRI, SIN, SAW, SQR or RND.");
        else if (tabIdx == 2) showContextHelp("HELP: JUNO VCF", "Resonant 24dB low-pass filter.\nCutoff & Res shape brightness.\nEnv Mod & Env Pol control envelope sweep.\nKey Follow tracks keyboard pitch.");
        else if (tabIdx == 3) showContextHelp("HELP: JUNO ENV", "ADSR envelope for VCF & VCA.\nAttack, Decay, Sustain, Release shape volume & filter.\nVCA Mode switches between ENV and Gate.");
        else showContextHelp("HELP: JUNO FX", "HPF cuts low-end rumble (0-3).\nChorus activates classic stereo BBD chorus (Off, I, II, I+II).");
    } else if (secId == 1) { // DX7
        if (tabIdx == 0) showContextHelp("HELP: DX7 ALG", "Touch/drag the ALGO diagram directly to select FM routing (1-32), double-tap to reset, or long-press for MIDI Learn.\nFeedback (0-7) sets OP feedback.\nMod Balance (0-100) scales all modulator levels at once.");
        else if (tabIdx == 1) showContextHelp("HELP: DX7 OP1-3", "Level (0-99) sets volume/mod depth.\nRatio snaps to melodic steps (0.5, 1..31).\n\nCoarse & Fine (in OP menu):\nRatio = Coarse x (1 + Fine%). Use Coarse (0.5, 1..31) for harmonic pitch and Fine (0-99%) for detune or bell/metallic tones.\n\nDouble-tap OP Level (blue frame) to open Operator editor.");
        else if (tabIdx == 2) showContextHelp("HELP: DX7 OP4-6", "Level (0-99) & melodic Ratio (0.5, 1..31) for Operators 4-6.\n\nCoarse & Fine:\nFine adds +0% to +99% on top of Coarse (e.g. Coarse 1 + Fine 41 = 1.41). Changing Coarse keeps the same timbre across octaves.\n\nDouble-tap OP Level (blue frame) for Coarse, Fine, Detune, Fixed & 4-stage EG.");
        else showContextHelp("HELP: DX7 GLOBAL", "DX Octave shifts pitch (-2..+2).\nLFO Speed, Delay, PMD & AMD control vibrato/tremolo.\n\nShortcut:\nDouble-tap Pitch EG to edit Pitch Envelope, LFO Wave & LFO Sync.");
    } else if (secId == 2) { // DRUMKIT
        showContextHelp("HELP: DRUMS", "9-part ROM drum kit (Drum 1-9).\nKIT: Select PCM sample (0-66).\nTUNE: Pitch speed (0.5-2.0x).\nMIX: Individual level.\nPAN: Stereo position.\nFLT: Low-pass cutoff.\n\nShortcut:\nDouble-tap the Drums tab on the left to open 3x3 Drum Pads!");
    } else if (secId == 3) { // PCM
        showContextHelp("HELP: PCM ENGINE", "Chromatic PCM sample player.\nPLAYER: Choose sample (0-154), Tune (+/-24 st), Fine Tune, Loop, Pan, Key Follow & Portamento.\nFLT: LPF/BPF/HPF filter.\nENV: Full ADSR & velocity sensitivity.");
    } else if (secId == 4) { // PIANO
        showContextHelp("HELP: PIANO ENGINE", "Interpolated partial piano.\nTONE: Brightness, Res, Key Follow, Vel Sens/Filter & Stereo Width.\nTIMBRE: Low EQ, Mid EQ, High EQ & Fine Tune.\nMIX: Body Level, Filter Env & Key Scale.\nENV: ADSR.");
    } else if (secId == 5) { // SEQ
        if (tabIdx == 0) showContextHelp("HELP: SEQ PATTERN", "16-step sequencer (16 tracks).\nT1-T9 trigger Drum 1-9.\nT10-T16 play Synth 10-16.\nREC: Live MIDI step record while playing.\n\nShortcuts:\n- Double-tap Seq tab: PLAY/STOP\n- Long-press step (T10-T16): Set pitch\n- Long-press Seq tab: Clear Pattern or ALL");
        else if (tabIdx == 1) showContextHelp("HELP: SEQ SONG", "Chain up to 16 patterns into a song.\nDouble-tap the left Seq/Song tab while on this page to PLAY/STOP the song.\nTap any slot dropdown to choose active patterns or END.\n- Long-press Song tab: Clear SONG chain");
        else showContextHelp("HELP: SEQ SETUP", "BPM: Tempo (40-240).\nSwing: Groove shuffle (0-75%).\nAct Pat: Active pattern (1-16).\nSteps: Loop length (1-16).\nMax Pat: Available patterns in SONG (1-16).");
    } else if (secId == 6) { // SYSTEM
        if (tabIdx == 0) showContextHelp("HELP: SYNTH SLOTS", "Configure 16 independent synths.\nTotal budget: 120 OSCs (shown bottom-left).\n\nShortcuts:\n- Double-tap Synth (blue frame): Rename patch\n- Long-press Synth engine tab (left): Reset synth\n- Double-tap OSC meter: System Stats\n- Long-press OSC meter: UI & Frame Guide");
        else if (tabIdx == 1) showContextHelp("HELP: GLOBAL FX", "Master stereo Echo & Reverb shared across all synths.\nEcho Mix (blue frame): Double-tap to edit Time, Fdbk & Tone.\nRev Mix (blue frame): Double-tap to edit Size, Damp & Xover.\nMaster Vol: Global output.");
        else if (tabIdx == 2) showContextHelp("HELP: FILE & STORAGE", "Auto-saves after 10s idle (paused during playback).\nProject File (green frame): Slide/tap slot 1-16, double-tap to SAVE, long-press to LOAD/DELETE.\nPatch File (green frame): Syncs with active synth, double-tap to EXPORT .pat, long-press to IMPORT/DELETE.\nDemo Presets: Load 4 demo grooves from Flash.\nFactory Rst: Hold 800ms to wipe.");
        else showContextHelp("HELP: DISPLAY", "Brightness: Backlight level.\nDblClick: Double-tap speed.\nFlip: Rotate screen 180 deg.\nScreen Dim: Auto-dim to 18% after 30s-9m idle.");
    } else if (secId == 7) { // UI & FRAME COLOR GUIDE
        showContextHelp("HELP: UI & FRAMES", "Frame Color Coding:\n- SLATE BLUE FRAME: Double-tap opens extended editor, rename popup or System Stats (OSC meter).\n- OLIVE GREEN FRAME: File actions (Project & Patch). Slide/tap to select, double-tap to SAVE/EXPORT, long-press to LOAD/DELETE.\n- ORANGE BAR / BORDER: Active value or MIDI Learn active.\n\nStandard Controls:\n- Drag horizontally: Smooth sweep\n- Tap left/right half: Fine -/+ step\n- Double-tap: Reset default value\n- Long-press: Toggle MIDI Learn");
    }
}
