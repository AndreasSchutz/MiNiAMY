#pragma once
#include <Arduino.h>
#include <lvgl.h>

#define SEQ_PATTERNS 16
#define SEQ_TRACKS   16
#define SEQ_STEPS    16

static uint8_t seqPatterns[SEQ_PATTERNS][SEQ_TRACKS][SEQ_STEPS] = {
    {
        {1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0},
        {0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0},
        {0,0,1,0, 0,0,1,0, 0,0,1,0, 0,0,1,0}
    }
};
static uint8_t *seqDrumVel = (uint8_t*)ps_calloc(SEQ_PATTERNS * SEQ_TRACKS * SEQ_STEPS * 9, 1);
static uint8_t *seqStepVel = (uint8_t*)ps_calloc(SEQ_PATTERNS * SEQ_TRACKS * SEQ_STEPS, 1);
inline uint8_t& drumVelAt(int p, int t, int s, int d) { return seqDrumVel[(((p * SEQ_TRACKS + t) * SEQ_STEPS + s) * 9) + d]; }
inline uint8_t& stepVelAt(int p, int t, int s) { return seqStepVel[(p * SEQ_TRACKS + t) * SEQ_STEPS + s]; }
static uint8_t seqKitBrush[SEQ_TRACKS][9] = {{100,0,0,0,0,0,0,0,0}};
static uint8_t seqTrackNote[SEQ_TRACKS] = {48, 60, 60, 60, 60, 60, 60, 60, 60, 36, 36, 60, 60, 60, 60, 60};
static bool seqTrackMuted[SEQ_TRACKS] = {false};
static int currentSeqPattern = 0;
static int currentSeqTrack = 0;
static int seqMaxSteps = 16;
static int seqNumPatterns = 16;
static volatile int seqPlayheadStep = -1;
static volatile bool seqIsPlaying = false;
static volatile bool seqMidiRecActive = false;
#define SEQ_SONG_STEPS 32
static uint8_t seqSongChain[SEQ_SONG_STEPS] = {1, 2, 0};
static volatile bool seqSongModeActive = false;
static volatile int seqSongPos = 0;

static lv_obj_t **seqSongStepDrops = (lv_obj_t**)ps_calloc(SEQ_SONG_STEPS, sizeof(lv_obj_t*));
static lv_obj_t *seqRecBtn = nullptr;

static lv_obj_t *seqTrackDropdown = nullptr;
static lv_obj_t *seqPlayBtn = nullptr;
static lv_obj_t *seqPlayLbl = nullptr;
static lv_obj_t *seqMuteBtn = nullptr;
static lv_obj_t *seqStepBtns[SEQ_STEPS] = {nullptr};
static lv_obj_t *seqStepLbls[SEQ_STEPS] = {nullptr};

static lv_obj_t *seqNoteModal = nullptr;
static lv_obj_t *seqNoteModalLbl = nullptr;
static int seqNoteEditStep = -1;
static bool seqStepLongHandled = false;

extern void onSeqTogglePlayFromGrid();
extern void onSeqGridChanged();
extern const char* getSynthSlotEngineShortName(int slotIdx);

inline void formatMidiNoteName(uint8_t note, char *out) {
    const char **nNames = (const char**)alloca(12 * sizeof(char*));
    *(nNames+0)="C"; *(nNames+1)="C#"; *(nNames+2)="D"; *(nNames+3)="D#"; *(nNames+4)="E"; *(nNames+5)="F";
    *(nNames+6)="F#"; *(nNames+7)="G"; *(nNames+8)="G#"; *(nNames+9)="A"; *(nNames+10)="A#"; *(nNames+11)="B";
    int n = constrain((int)note, 0, 127);
    snprintf(out, 8, "%s%d", *(nNames + (n % 12)), (n / 12) - 1);
}

inline void getDynamicSeqTrackName(int t, char *out, size_t len) {
    snprintf(out, len, "T%d:%s", t + 1, getSynthSlotEngineShortName(t));
}

inline void rebuildSeqDropdownOptions() {
    if (!seqTrackDropdown) return;
    char *opts = (char*)alloca(384);
    int pos = 0;
    for (int t = 0; t < SEQ_TRACKS; t++) {
        char *tName = (char*)alloca(16);
        getDynamicSeqTrackName(t, tName, 16);
        if (seqSongModeActive) {
            pos += snprintf(opts + pos, 384 - pos, "S%d:P%d %s%s", seqSongPos + 1, currentSeqPattern + 1, tName, (t < SEQ_TRACKS - 1) ? "\n" : "");
        } else {
            pos += snprintf(opts + pos, 384 - pos, "P%d %s%s", currentSeqPattern + 1, tName, (t < SEQ_TRACKS - 1) ? "\n" : "");
        }
    }
    lv_dropdown_set_options(seqTrackDropdown, opts);
    lv_dropdown_set_selected(seqTrackDropdown, currentSeqTrack);
}

inline void loadDemoSeqPatterns() {
    memset(seqPatterns, 0, sizeof(seqPatterns));
    memset(seqTrackMuted, 0, sizeof(seqTrackMuted));
    if (seqDrumVel) memset(seqDrumVel, 0, SEQ_PATTERNS * SEQ_TRACKS * SEQ_STEPS * 9);
    if (seqStepVel) memset(seqStepVel, 100, SEQ_PATTERNS * SEQ_TRACKS * SEQ_STEPS);
    uint8_t *p = (uint8_t*)seqPatterns;
    const int d0[] = {0,2,4,6,8,10,12,14}, n0[] = {36,38,37,38,36,38,37,38};
    for (int i = 0; i < 8; i++) { *(p + 0*256 + 0*16 + d0[i]) = n0[i]; if (seqDrumVel) drumVelAt(0,0,d0[i],n0[i]-36) = (i%2==0)?115:85; }
    const int b0[] = {0,3,6,8,10,14}, bn0[] = {48,48,55,58,55,51};
    for (int i = 0; i < 6; i++) *(p + 0*256 + 1*16 + b0[i]) = bn0[i];
    const int x0[] = {2,6,10,12,14}, xn0[] = {67,70,67,65,63};
    for (int i = 0; i < 5; i++) *(p + 0*256 + 2*16 + x0[i]) = xn0[i];
}

inline void rebuildSongDropdownOptions() {
    int maxP = constrain(seqNumPatterns, 1, SEQ_PATTERNS);
    char opts[256];
    int pos = snprintf(opts, sizeof(opts), "END");
    for (int p = 1; p <= maxP; p++) pos += snprintf(opts + pos, sizeof(opts) - pos, "\nP%d", p);
    for (int i = 0; i < SEQ_SONG_STEPS; i++) {
        if (seqSongStepDrops && seqSongStepDrops[i]) {
            uint8_t cur = seqSongChain[i];
            lv_dropdown_set_options(seqSongStepDrops[i], opts);
            lv_dropdown_set_selected(seqSongStepDrops[i], (cur <= maxP) ? cur : 0);
        }
    }
}

inline void refreshSeqSongUI() {
    if (!seqSongStepDrops) return;
    static int lastNumPat = -1;
    int maxP = constrain(seqNumPatterns, 1, SEQ_PATTERNS);
    if (lastNumPat != maxP) { lastNumPat = maxP; rebuildSongDropdownOptions(); }
    for (int i = 0; i < 16; i++) {
        lv_obj_t *dd = *(seqSongStepDrops + i);
        if (!dd) continue;
        uint8_t pat = constrain((int)*(seqSongChain + i), 0, SEQ_PATTERNS);
        if ((int)lv_dropdown_get_selected(dd) != pat) {
            lv_dropdown_set_selected(dd, pat);
        }
        bool isCur = (seqIsPlaying && seqSongModeActive && seqSongPos == i);
        lv_color_t bg = isCur ? lv_color_hex(0x8CE600) : ((pat > 0) ? lv_color_hex(0x323238) : lv_color_hex(0x222226));
        lv_obj_set_style_bg_color(dd, bg, 0);
        lv_obj_set_style_border_color(dd, isCur ? lv_color_white() : ((pat > 0) ? lv_color_hex(0xFF5500) : lv_color_hex(0x5A5A64)), 0);
        lv_obj_set_style_border_width(dd, isCur ? 2 : 1, 0);
    }
}

inline void refreshSeqGridUI() {
    static int lastSongPos = -2;
    static bool lastSongAct = false;
    static bool lastSongPlay = false;
    if (lastSongPos != seqSongPos || lastSongAct != seqSongModeActive || lastSongPlay != seqIsPlaying) {
        lastSongPos = seqSongPos;
        lastSongAct = seqSongModeActive;
        lastSongPlay = seqIsPlaying;
        refreshSeqSongUI();
    }
    if (seqTrackDropdown) {
        static int lastPat = -1;
        static uint32_t lastHash = 0;
        uint32_t h = (uint32_t)(currentSeqPattern + 1) + (seqSongModeActive ? (uint32_t)(seqSongPos + 1) * 977u : 0u);
        for (int k = 0; k < 16; k++) h = h * 33u + (uint32_t)(uintptr_t)getSynthSlotEngineShortName(k);
        if (lastPat != currentSeqPattern || lastHash != h) {
            lastPat = currentSeqPattern; lastHash = h;
            rebuildSeqDropdownOptions();
        } else if ((int)lv_dropdown_get_selected(seqTrackDropdown) != currentSeqTrack) {
            lv_dropdown_set_selected(seqTrackDropdown, currentSeqTrack);
        }
    }

    if (seqRecBtn) {
        lv_obj_set_style_bg_color(seqRecBtn, seqMidiRecActive ? lv_color_hex(0xD91E18) : lv_color_hex(0x3A3A40), 0);
    }
    if (seqMuteBtn) {
        lv_obj_set_style_bg_color(seqMuteBtn, *(seqTrackMuted + currentSeqTrack) ? lv_color_hex(0xD91E18) : lv_color_hex(0x3A3A40), 0);
    }
    static uint32_t *lastStepSig = (uint32_t*)ps_calloc(SEQ_STEPS, sizeof(uint32_t));
    for (int s = 0; s < SEQ_STEPS; s++) {
        lv_obj_t *btn = *(seqStepBtns + s);
        lv_obj_t *lbl = *(seqStepLbls + s);
        if (!btn) continue;
        uint8_t val = seqPatterns[currentSeqPattern][currentSeqTrack][s];
        bool active = (val > 0);
        bool inRange = (s < seqMaxSteps);
        bool isHead = (seqIsPlaying && s == seqPlayheadStep);
        uint32_t sig = (uint32_t)val | ((uint32_t)inRange << 8) | ((uint32_t)isHead << 9) | ((uint32_t)(uintptr_t)getSynthSlotEngineShortName(currentSeqTrack) << 10) | 0x80000000u;
        if (lastStepSig && *(lastStepSig + s) == sig) continue;
        if (lastStepSig) *(lastStepSig + s) = sig;
        if (inRange) lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
        bool isBeat = (s % 4 == 0);
        lv_color_t bg = isHead ? (active ? lv_color_hex(0x8CE600) : lv_color_hex(0x4E8A3E)) : (active ? lv_color_hex(0xFF5500) : lv_color_hex(0x323238));
        lv_obj_set_style_bg_color(btn, bg, 0);
        lv_obj_set_style_border_color(btn, isHead ? lv_color_white() : (isBeat ? lv_color_hex(0xFFF8E7) : lv_color_hex(0x5A5A64)), 0);
        lv_obj_set_style_border_width(btn, (isHead || isBeat) ? 2 : 1, 0);
        if (lbl) {
            char *txt = (char*)alloca(12);
            if (active) {
                if (strcmp(getSynthSlotEngineShortName(currentSeqTrack), "Kit") == 0) {
                    int cnt = 0, lastD = 0;
                    for (int d = 0; d < 9; d++) if (seqDrumVel && drumVelAt(currentSeqPattern, currentSeqTrack, s, d) > 0) { cnt++; lastD = d + 1; }
                    if (cnt > 1) snprintf(txt, 12, "%d\n%dxKit", s + 1, cnt);
                    else if (cnt == 1) snprintf(txt, 12, "%d\nD%d", s + 1, lastD);
                    else snprintf(txt, 12, "%d\nD%d", s + 1, ((val >= 36 ? val - 36 : 0) % 9) + 1);
                }
                else { char *nStr = (char*)alloca(8); formatMidiNoteName(val, nStr); snprintf(txt, 12, "%d\n%s", s + 1, nStr); }
            }
            else snprintf(txt, 12, "%d", s + 1);
            lv_label_set_text(lbl, txt);
        }
    }
}

static lv_obj_t *seqDrumModal = nullptr, *seqDrumModalLbl = nullptr;
static lv_obj_t **seqDrumPadBars = (lv_obj_t**)ps_calloc(9, sizeof(lv_obj_t*)), **seqDrumPadLbls = (lv_obj_t**)ps_calloc(9, sizeof(lv_obj_t*));
static bool seqDrumPadDragged = false; static lv_coord_t seqDrumDragX = 0;
inline void syncDrumStepActiveFlag(int pat, int tr, int step) {
    bool any = false;
    for (int d = 0; d < 9; d++) if (seqDrumVel && drumVelAt(pat, tr, step, d) > 0) any = true;
    seqPatterns[pat][tr][step] = any ? 36 : 0;
}
inline void refreshSeqDrumModal() {
    if (!seqDrumModal || seqNoteEditStep < 0) return;
    char *t = (char*)alloca(32); snprintf(t, 32, "Step %d Drum Pads", seqNoteEditStep + 1);
    lv_label_set_text(seqDrumModalLbl, t);
    for (int d = 0; d < 9; d++) {
        uint8_t v = seqDrumVel ? drumVelAt(currentSeqPattern, currentSeqTrack, seqNoteEditStep, d) : 0;
        if (seqDrumPadBars[d]) { lv_bar_set_value(seqDrumPadBars[d], v, LV_ANIM_OFF); lv_obj_set_style_border_color(seqDrumPadBars[d], v > 0 ? lv_color_hex(0xFF5100) : lv_color_hex(0x5A5A64), 0); }
        if (seqDrumPadLbls[d]) { char *b = (char*)alloca(16); if (v > 0) snprintf(b, 16, "D%d\n%d%%", d + 1, (v * 100) / 127); else snprintf(b, 16, "D%d\nOFF", d + 1); lv_label_set_text(seqDrumPadLbls[d], b); }
    }
}
inline void updateSeqNoteModalText() {
    if (!seqNoteModalLbl || seqNoteEditStep < 0) return;
    uint8_t note = seqPatterns[currentSeqPattern][currentSeqTrack][seqNoteEditStep];
    char *nStr = (char*)alloca(8);
    formatMidiNoteName(note, nStr);
    char *buf = (char*)alloca(48);
    if (strcmp(getSynthSlotEngineShortName(currentSeqTrack), "Kit") == 0) snprintf(buf, 48, "Step %d Drum\nDrum %d (MIDI %u)", seqNoteEditStep + 1, ((note >= 36 ? note - 36 : 0) % 9) + 1, note);
    else snprintf(buf, 48, "Step %d Pitch\n%s (MIDI %u)", seqNoteEditStep + 1, nStr, note);
    lv_label_set_text(seqNoteModalLbl, buf);
}

inline void adjustSeqEditedNote(int delta) {
    if (seqNoteEditStep < 0) return;
    int cur = (int)seqPatterns[currentSeqPattern][currentSeqTrack][seqNoteEditStep];
    bool isKit = (strcmp(getSynthSlotEngineShortName(currentSeqTrack), "Kit") == 0);
    if (cur == 0) cur = *(seqTrackNote + currentSeqTrack);
    uint8_t next = isKit ? (uint8_t)constrain(cur + delta, 36, 44) : (uint8_t)constrain(cur + delta, 24, 96);
    seqPatterns[currentSeqPattern][currentSeqTrack][seqNoteEditStep] = next;
    *(seqTrackNote + currentSeqTrack) = next;
    updateSeqNoteModalText();
    refreshSeqGridUI();
    onSeqGridChanged();
}

inline void buildSeqGridPage(lv_obj_t *parent) {
    lv_obj_set_style_bg_color(parent, lv_color_hex(0xC8C6BE), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(parent, 2, 0);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    seqTrackDropdown = lv_dropdown_create(parent);
    lv_obj_set_size(seqTrackDropdown, 126, 30);
    lv_obj_set_pos(seqTrackDropdown, 0, 0);
    lv_obj_set_style_radius(seqTrackDropdown, 5, 0);
    lv_obj_set_style_pad_top(seqTrackDropdown, 6, 0);
    lv_obj_set_style_pad_left(seqTrackDropdown, 4, 0);
    lv_obj_set_style_pad_right(seqTrackDropdown, 4, 0);
    lv_obj_set_style_bg_color(seqTrackDropdown, lv_color_hex(0x25252A), 0);
    lv_obj_set_style_text_color(seqTrackDropdown, lv_color_white(), 0);
    lv_obj_set_style_border_color(seqTrackDropdown, lv_color_hex(0x7A9A65), 0);
    lv_obj_set_style_border_width(seqTrackDropdown, 2, 0);
    lv_obj_add_event_cb(seqTrackDropdown, [](lv_event_t *e) {
        currentSeqTrack = constrain((int)lv_dropdown_get_selected(lv_event_get_target(e)), 0, SEQ_TRACKS - 1);
        refreshSeqGridUI();
    }, LV_EVENT_VALUE_CHANGED, nullptr);

    seqRecBtn = lv_obj_create(parent);
    lv_obj_set_size(seqRecBtn, 58, 30);
    lv_obj_set_pos(seqRecBtn, 129, 0);
    lv_obj_set_style_radius(seqRecBtn, 5, 0);
    lv_obj_set_style_pad_all(seqRecBtn, 0, 0);
    lv_obj_clear_flag(seqRecBtn, LV_OBJ_FLAG_SCROLLABLE); lv_obj_add_flag(seqRecBtn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *lRec = lv_label_create(seqRecBtn);
    lv_label_set_text(lRec, "REC");
    lv_obj_set_style_text_color(lRec, lv_color_white(), 0);
    lv_obj_center(lRec);
    lv_obj_add_event_cb(seqRecBtn, [](lv_event_t *e) { seqMidiRecActive = !seqMidiRecActive; refreshSeqGridUI(); }, LV_EVENT_CLICKED, nullptr);

    seqMuteBtn = lv_obj_create(parent);
    lv_obj_set_size(seqMuteBtn, 61, 30);
    lv_obj_set_pos(seqMuteBtn, 190, 0);
    lv_obj_set_style_radius(seqMuteBtn, 5, 0);
    lv_obj_set_style_pad_all(seqMuteBtn, 0, 0);
    lv_obj_clear_flag(seqMuteBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *lMute = lv_label_create(seqMuteBtn);
    lv_label_set_text(lMute, "MUTE");
    lv_obj_set_style_text_color(lMute, lv_color_white(), 0);
    lv_obj_center(lMute);
    lv_obj_add_flag(seqMuteBtn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(seqMuteBtn, [](lv_event_t *e) {
        bool m = !(*(seqTrackMuted + currentSeqTrack));
        *(seqTrackMuted + currentSeqTrack) = m;
        refreshSeqGridUI();
    }, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *grid = lv_obj_create(parent);
    lv_obj_set_size(grid, 252, 174);
    lv_obj_set_pos(grid, 0, 32);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(grid, 0, 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_row(grid, 2, 0);
    lv_obj_set_style_pad_column(grid, 2, 0);

    for (int s = 0; s < SEQ_STEPS; s++) {
        lv_obj_t *stepBtn = lv_obj_create(grid);
        lv_obj_set_size(stepBtn, 61, 42);
        lv_obj_set_style_radius(stepBtn, 5, 0);
        lv_obj_set_style_pad_all(stepBtn, 0, 0);
        lv_obj_clear_flag(stepBtn, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t *sl = lv_label_create(stepBtn);
        lv_obj_set_style_text_color(sl, lv_color_white(), 0);
        lv_obj_set_style_text_align(sl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(sl);
        lv_obj_add_event_cb(stepBtn, [](lv_event_t *e) {
            lv_event_code_t code = lv_event_get_code(e);
            int stepIdx = (int)(intptr_t)lv_event_get_user_data(e);
            static lv_timer_t *sTmr = nullptr; static int sPend = -1;
            if (code == LV_EVENT_SHORT_CLICKED) {
                bool isKit = (strcmp(getSynthSlotEngineShortName(currentSeqTrack), "Kit") == 0);
                if (sTmr && sPend == stepIdx) {
                    lv_timer_del(sTmr); sTmr = nullptr; sPend = -1; seqNoteEditStep = stepIdx;
                    if (isKit && seqDrumModal) {
                        uint8_t pv = seqPatterns[currentSeqPattern][currentSeqTrack][stepIdx];
                        bool any = false; for (int d = 0; d < 9; d++) if (seqDrumVel && drumVelAt(currentSeqPattern, currentSeqTrack, stepIdx, d) > 0) any = true;
                        if (!any && pv >= 36 && seqDrumVel) { drumVelAt(currentSeqPattern, currentSeqTrack, stepIdx, (pv - 36) % 9) = 100; syncDrumStepActiveFlag(currentSeqPattern, currentSeqTrack, stepIdx); }
                        refreshSeqDrumModal(); lv_obj_clear_flag(seqDrumModal, LV_OBJ_FLAG_HIDDEN); lv_obj_move_foreground(seqDrumModal);
                    } else if (seqNoteModal) { updateSeqNoteModalText(); lv_obj_clear_flag(seqNoteModal, LV_OBJ_FLAG_HIDDEN); lv_obj_move_foreground(seqNoteModal); }
                } else {
                    if (sTmr) { lv_timer_del(sTmr); sTmr = nullptr; }
                    sPend = stepIdx;
                    sTmr = lv_timer_create([](lv_timer_t *t) {
                        int st = sPend; sTmr = nullptr; sPend = -1; if (st < 0) return;
                        bool kit = (strcmp(getSynthSlotEngineShortName(currentSeqTrack), "Kit") == 0);
                        uint8_t cur = seqPatterns[currentSeqPattern][currentSeqTrack][st];
                        if (cur > 0) { seqPatterns[currentSeqPattern][currentSeqTrack][st] = 0; if (kit && seqDrumVel) for (int d = 0; d < 9; d++) drumVelAt(currentSeqPattern, currentSeqTrack, st, d) = 0; }
                        else {
                            seqPatterns[currentSeqPattern][currentSeqTrack][st] = *(seqTrackNote + currentSeqTrack);
                            if (kit && seqDrumVel) {
                                bool hasB = false; for (int d = 0; d < 9; d++) if (seqKitBrush[currentSeqTrack][d] > 0) hasB = true;
                                if (hasB) for (int d = 0; d < 9; d++) drumVelAt(currentSeqPattern, currentSeqTrack, st, d) = seqKitBrush[currentSeqTrack][d];
                                else drumVelAt(currentSeqPattern, currentSeqTrack, st, 0) = 100;
                            }
                        }
                        refreshSeqGridUI(); onSeqGridChanged();
                    }, 260, nullptr);
                    lv_timer_set_repeat_count(sTmr, 1);
                }
            }
        }, LV_EVENT_ALL, (void*)(intptr_t)s);
        *(seqStepBtns + s) = stepBtn; *(seqStepLbls + s) = sl;
    }

    seqNoteModal = lv_obj_create(parent);
    lv_obj_set_size(seqNoteModal, 210, 126);
    lv_obj_center(seqNoteModal);
    lv_obj_set_style_bg_color(seqNoteModal, lv_color_hex(0x2B2B30), 0);
    lv_obj_set_style_border_color(seqNoteModal, lv_color_hex(0xFF5500), 0);
    lv_obj_set_style_border_width(seqNoteModal, 2, 0);
    lv_obj_set_style_pad_all(seqNoteModal, 6, 0);
    lv_obj_clear_flag(seqNoteModal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(seqNoteModal, LV_OBJ_FLAG_HIDDEN);

    seqNoteModalLbl = lv_label_create(seqNoteModal);
    lv_obj_set_style_text_color(seqNoteModalLbl, lv_color_white(), 0);
    lv_obj_set_style_text_align(seqNoteModalLbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(seqNoteModalLbl, LV_ALIGN_TOP_MID, 0, 2);

    const char **bTxts = (const char**)alloca(4 * sizeof(char*));
    *(bTxts+0)="-12"; *(bTxts+1)="-1"; *(bTxts+2)="+1"; *(bTxts+3)="+12";
    int *bDeltas = (int*)alloca(4 * sizeof(int));
    *(bDeltas+0)=-12; *(bDeltas+1)=-1; *(bDeltas+2)=1; *(bDeltas+3)=12;
    for (int k = 0; k < 4; k++) {
        lv_obj_t *nb = lv_btn_create(seqNoteModal);
        lv_obj_set_size(nb, 44, 30);
        lv_obj_set_pos(nb, 4 + k * 48, 46);
        lv_obj_set_style_bg_color(nb, lv_color_hex(0x3A3A40), 0);
        lv_obj_t *nl = lv_label_create(nb);
        lv_label_set_text(nl, *(bTxts + k));
        lv_obj_center(nl);
        lv_obj_add_event_cb(nb, [](lv_event_t *e) { adjustSeqEditedNote((int)(intptr_t)lv_event_get_user_data(e)); }, LV_EVENT_CLICKED, (void*)(intptr_t)(*(bDeltas + k)));
    }

    lv_obj_t *btnOk = lv_btn_create(seqNoteModal);
    lv_obj_set_size(btnOk, 80, 28);
    lv_obj_align(btnOk, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_obj_set_style_bg_color(btnOk, lv_color_hex(0x4E8A3E), 0);
    lv_obj_t *lOk = lv_label_create(btnOk);
    lv_label_set_text(lOk, "OK");
    lv_obj_center(lOk);
    lv_obj_add_event_cb(btnOk, [](lv_event_t *e) { lv_obj_add_flag(seqNoteModal, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *vD = lv_btn_create(seqNoteModal); lv_obj_set_size(vD, 50, 28); lv_obj_align(vD, LV_ALIGN_BOTTOM_LEFT, 4, -2); lv_label_set_text(lv_label_create(vD), "Vel-");
    lv_obj_add_event_cb(vD, [](lv_event_t *e) { if (seqStepVel && seqNoteEditStep >= 0) { uint8_t &v = stepVelAt(currentSeqPattern, currentSeqTrack, seqNoteEditStep); v = (uint8_t)constrain((int)v - 16, 16, 127); updateSeqNoteModalText(); onSeqGridChanged(); } }, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *vU = lv_btn_create(seqNoteModal); lv_obj_set_size(vU, 50, 28); lv_obj_align(vU, LV_ALIGN_BOTTOM_RIGHT, -4, -2); lv_label_set_text(lv_label_create(vU), "Vel+");
    lv_obj_add_event_cb(vU, [](lv_event_t *e) { if (seqStepVel && seqNoteEditStep >= 0) { uint8_t &v = stepVelAt(currentSeqPattern, currentSeqTrack, seqNoteEditStep); v = (uint8_t)constrain((int)v + 16, 16, 127); updateSeqNoteModalText(); onSeqGridChanged(); } }, LV_EVENT_CLICKED, nullptr);
    seqDrumModal = lv_obj_create(parent); lv_obj_set_size(seqDrumModal, 240, 198); lv_obj_center(seqDrumModal);
    lv_obj_set_style_bg_color(seqDrumModal, lv_color_hex(0x1E1E24), 0); lv_obj_set_style_border_color(seqDrumModal, lv_color_hex(0xFF5100), 0);
    lv_obj_set_style_border_width(seqDrumModal, 2, 0); lv_obj_set_style_pad_all(seqDrumModal, 4, 0);
    lv_obj_clear_flag(seqDrumModal, LV_OBJ_FLAG_SCROLLABLE); lv_obj_add_flag(seqDrumModal, LV_OBJ_FLAG_HIDDEN);
    seqDrumModalLbl = lv_label_create(seqDrumModal); lv_obj_set_style_text_color(seqDrumModalLbl, lv_color_white(), 0); lv_obj_align(seqDrumModalLbl, LV_ALIGN_TOP_MID, 0, 0);
    for (int d = 0; d < 9; d++) {
        lv_obj_t *bar = lv_bar_create(seqDrumModal); lv_obj_set_size(bar, 72, 42); lv_obj_set_pos(bar, 4 + (d % 3) * 76, 20 + (d / 3) * 46);
        lv_bar_set_range(bar, 0, 127); lv_obj_set_style_radius(bar, 4, 0); lv_obj_set_style_radius(bar, 4, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x2B2B32), 0); lv_obj_set_style_bg_color(bar, lv_color_hex(0xF85D00), LV_PART_INDICATOR);
        lv_obj_set_style_border_width(bar, 1, 0); lv_obj_add_flag(bar, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_t *pl = lv_label_create(bar); lv_obj_set_style_text_color(pl, lv_color_white(), 0); lv_obj_set_style_text_align(pl, LV_TEXT_ALIGN_CENTER, 0); lv_obj_center(pl);
        seqDrumPadBars[d] = bar; seqDrumPadLbls[d] = pl;
        lv_obj_add_event_cb(bar, [](lv_event_t *e) {
            lv_event_code_t c = lv_event_get_code(e); int di = (int)(intptr_t)lv_event_get_user_data(e);
            if (!seqDrumVel || seqNoteEditStep < 0) return;
            lv_indev_t *ind = lv_indev_get_act(); lv_point_t pt; if (ind) lv_indev_get_point(ind, &pt);
            if (c == LV_EVENT_PRESSED) { seqDrumPadDragged = false; seqDrumDragX = pt.x; }
            else if (c == LV_EVENT_PRESSING && ind) {
                int dx = pt.x - seqDrumDragX;
                if (abs(dx) > 6) { seqDrumPadDragged = true; seqDrumDragX = pt.x; uint8_t &v = drumVelAt(currentSeqPattern, currentSeqTrack, seqNoteEditStep, di); v = (uint8_t)constrain((v == 0 ? 64 : (int)v) + dx * 2, 12, 127); syncDrumStepActiveFlag(currentSeqPattern, currentSeqTrack, seqNoteEditStep); refreshSeqDrumModal(); }
            } else if (c == LV_EVENT_SHORT_CLICKED && !seqDrumPadDragged) {
                uint8_t &v = drumVelAt(currentSeqPattern, currentSeqTrack, seqNoteEditStep, di); v = (v > 0) ? 0 : 100; syncDrumStepActiveFlag(currentSeqPattern, currentSeqTrack, seqNoteEditStep); refreshSeqDrumModal();
            }
        }, LV_EVENT_ALL, (void*)(intptr_t)d);
    }
    lv_obj_t *dOk = lv_btn_create(seqDrumModal); lv_obj_set_size(dOk, 90, 26); lv_obj_align(dOk, LV_ALIGN_BOTTOM_MID, 0, 0); lv_obj_set_style_bg_color(dOk, lv_color_hex(0x469632), 0);
    lv_obj_t *ld = lv_label_create(dOk); lv_label_set_text(ld, "DONE"); lv_obj_center(ld);
    lv_obj_add_event_cb(dOk, [](lv_event_t *e) {
        if (seqDrumVel && seqNoteEditStep >= 0) {
            bool any = false;
            for (int d = 0; d < 9; d++) { uint8_t v = drumVelAt(currentSeqPattern, currentSeqTrack, seqNoteEditStep, d); if (v > 0) any = true; }
            if (any) for (int d = 0; d < 9; d++) seqKitBrush[currentSeqTrack][d] = drumVelAt(currentSeqPattern, currentSeqTrack, seqNoteEditStep, d);
        }
        lv_obj_add_flag(seqDrumModal, LV_OBJ_FLAG_HIDDEN); refreshSeqGridUI(); onSeqGridChanged();
    }, LV_EVENT_CLICKED, nullptr);

    refreshSeqGridUI();
}

static lv_obj_t *seqCtxClearModal = nullptr;
static lv_obj_t *seqCtxClearLbl = nullptr;
static lv_obj_t *seqCtxBtnPat = nullptr;
static lv_obj_t *seqCtxBtnAll = nullptr;
static lv_obj_t *seqCtxBtnSong = nullptr;

inline void openSeqContextClearModal(bool isSongTab) {
    if (!seqCtxClearModal) return;
    if (isSongTab) {
        lv_label_set_text(seqCtxClearLbl, "Clear SONG chain?");
        lv_obj_add_flag(seqCtxBtnPat, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(seqCtxBtnAll, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(seqCtxBtnSong, LV_OBJ_FLAG_HIDDEN);
    } else {
        char *buf = (char*)alloca(48);
        snprintf(buf, 48, "Clear Pattern P%d\nor ALL patterns?", currentSeqPattern + 1);
        lv_label_set_text(seqCtxClearLbl, buf);
        lv_obj_clear_flag(seqCtxBtnPat, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(seqCtxBtnAll, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(seqCtxBtnSong, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_clear_flag(seqCtxClearModal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(seqCtxClearModal);
}

inline void buildSeqContextClearModal(lv_obj_t *scrParent) {
    seqCtxClearModal = lv_obj_create(scrParent);
    lv_obj_set_size(seqCtxClearModal, 224, 108);
    lv_obj_center(seqCtxClearModal);
    lv_obj_set_style_bg_color(seqCtxClearModal, lv_color_hex(0x2B2B30), 0);
    lv_obj_set_style_border_color(seqCtxClearModal, lv_color_hex(0xD91E18), 0);
    lv_obj_set_style_border_width(seqCtxClearModal, 2, 0);
    lv_obj_set_style_pad_all(seqCtxClearModal, 6, 0);
    lv_obj_clear_flag(seqCtxClearModal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(seqCtxClearModal, LV_OBJ_FLAG_HIDDEN);

    seqCtxClearLbl = lv_label_create(seqCtxClearModal);
    lv_obj_set_style_text_color(seqCtxClearLbl, lv_color_white(), 0);
    lv_obj_set_style_text_align(seqCtxClearLbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(seqCtxClearLbl, LV_ALIGN_TOP_MID, 0, 4);

    seqCtxBtnPat = lv_btn_create(seqCtxClearModal);
    lv_obj_set_size(seqCtxBtnPat, 64, 32);
    lv_obj_align(seqCtxBtnPat, LV_ALIGN_BOTTOM_LEFT, 2, -2);
    lv_obj_set_style_bg_color(seqCtxBtnPat, lv_color_hex(0xFF5500), 0);
    lv_obj_t *lP = lv_label_create(seqCtxBtnPat); lv_label_set_text(lP, "PAT"); lv_obj_center(lP);
    lv_obj_add_event_cb(seqCtxBtnPat, [](lv_event_t *e) {
        memset(&seqPatterns[currentSeqPattern], 0, SEQ_TRACKS * SEQ_STEPS);
        refreshSeqGridUI(); onSeqGridChanged();
        lv_obj_add_flag(seqCtxClearModal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, nullptr);

    seqCtxBtnAll = lv_btn_create(seqCtxClearModal);
    lv_obj_set_size(seqCtxBtnAll, 64, 32);
    lv_obj_align(seqCtxBtnAll, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_obj_set_style_bg_color(seqCtxBtnAll, lv_color_hex(0xD91E18), 0);
    lv_obj_t *lA = lv_label_create(seqCtxBtnAll); lv_label_set_text(lA, "ALL"); lv_obj_center(lA);
    lv_obj_add_event_cb(seqCtxBtnAll, [](lv_event_t *e) {
        memset(seqPatterns, 0, sizeof(seqPatterns)); memset(seqTrackMuted, 0, sizeof(seqTrackMuted));
        refreshSeqGridUI(); onSeqGridChanged();
        lv_obj_add_flag(seqCtxClearModal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, nullptr);

    seqCtxBtnSong = lv_btn_create(seqCtxClearModal);
    lv_obj_set_size(seqCtxBtnSong, 90, 32);
    lv_obj_align(seqCtxBtnSong, LV_ALIGN_BOTTOM_LEFT, 8, -2);
    lv_obj_set_style_bg_color(seqCtxBtnSong, lv_color_hex(0xD91E18), 0);
    lv_obj_t *lS = lv_label_create(seqCtxBtnSong); lv_label_set_text(lS, "YES"); lv_obj_center(lS);
    lv_obj_add_event_cb(seqCtxBtnSong, [](lv_event_t *e) {
        memset(seqSongChain, 0, sizeof(seqSongChain)); *(seqSongChain + 0) = 1; seqSongPos = 0;
        refreshSeqSongUI(); onSeqGridChanged();
        lv_obj_add_flag(seqCtxClearModal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *btnCancel = lv_btn_create(seqCtxClearModal);
    lv_obj_set_size(btnCancel, 64, 32);
    lv_obj_align(btnCancel, LV_ALIGN_BOTTOM_RIGHT, -2, -2);
    lv_obj_set_style_bg_color(btnCancel, lv_color_hex(0x5A5A64), 0);
    lv_obj_t *lC = lv_label_create(btnCancel); lv_label_set_text(lC, "NO"); lv_obj_center(lC);
    lv_obj_add_event_cb(btnCancel, [](lv_event_t *e) { lv_obj_add_flag(seqCtxClearModal, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, nullptr);
}

inline void buildSeqSongPage(lv_obj_t *parent) {
    lv_obj_set_style_bg_color(parent, lv_color_hex(0xC8C6BE), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(parent, 2, 0);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *grid = lv_obj_create(parent);
    lv_obj_set_size(grid, 252, 206);
    lv_obj_set_pos(grid, 0, 0);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(grid, 0, 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_row(grid, 2, 0);
    lv_obj_set_style_pad_column(grid, 2, 0);

    for (int i = 0; i < 16; i++) {
        lv_obj_t *dd = lv_dropdown_create(grid);
        lv_dropdown_set_symbol(dd, NULL);
        lv_obj_set_size(dd, 61, 50);
        lv_obj_set_style_radius(dd, 5, 0);
        lv_obj_set_style_pad_top(dd, 16, 0);
        lv_obj_set_style_pad_left(dd, 2, 0);
        lv_obj_set_style_pad_right(dd, 2, 0);
        lv_obj_set_style_text_align(dd, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(dd, lv_color_white(), 0);
        lv_obj_add_event_cb(dd, [](lv_event_t *e) {
            int idx = (int)(intptr_t)lv_event_get_user_data(e);
            uint16_t sel = lv_dropdown_get_selected(lv_event_get_target(e));
            *(seqSongChain + idx) = (uint8_t)constrain((int)sel, 0, constrain(seqNumPatterns, 1, SEQ_PATTERNS));
            refreshSeqSongUI(); onSeqGridChanged();
        }, LV_EVENT_VALUE_CHANGED, (void*)(intptr_t)i);
        if (seqSongStepDrops) *(seqSongStepDrops + i) = dd;
    }
    rebuildSongDropdownOptions();
    refreshSeqSongUI();
}
