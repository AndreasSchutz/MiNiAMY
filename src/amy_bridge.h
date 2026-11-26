#pragma once
static volatile uint32_t g_moodPulseMs = 0;
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <AMY-Arduino.h>
#include "seq_view.h"
#include "storage_mgr.h"
#include <ESP_I2S.h>
#include "es8311_init.h"
static I2SClass amyI2s;
static volatile bool seqExtClockActive = false;
static volatile uint8_t seqExtClockTicks = 0;
static volatile bool seqExtStepPulse = false;

enum AmyBridgeMsgType : uint8_t {
    AMY_MSG_PART_CONFIG = 1,
    AMY_MSG_PART_LEVEL = 6,
    AMY_MSG_PARAM_CHANGE = 2,
    AMY_MSG_DRUM_TRIGGER = 3,
    AMY_MSG_MASTER_VOL = 4,
    AMY_MSG_NOTE_ON_OFF = 5
};

struct AmyBridgeMsg {
    AmyBridgeMsgType type;
    uint8_t synthSlot;
    uint8_t engine;
    uint8_t voices;
    uint8_t patchOrNote;
    int paramId;
    float val1;
    float val2;
    float val3;
    float val4;
};

static QueueHandle_t amyCmdQueue = nullptr;
inline UBaseType_t amyGetQueueFree() {
    return amyCmdQueue ? uxQueueSpacesAvailable(amyCmdQueue) : 0;
}

inline void amyEnqueuePartLevel(uint8_t slot, float level) {
    if (!amyCmdQueue) return;
    AmyBridgeMsg msg = {AMY_MSG_PART_LEVEL, slot, 0, 0, 0, 0, level, 0, 0, 0};
    xQueueSend(amyCmdQueue, &msg, 0);
}

inline void amyEnqueuePartConfig(uint8_t slot, uint8_t engine, uint8_t voices, uint8_t patchOrNote, float level) {
    if (!amyCmdQueue) return;
    AmyBridgeMsg msg = {AMY_MSG_PART_CONFIG, slot, engine, voices, patchOrNote, 0, level, 0, 0, 0};
    xQueueSend(amyCmdQueue, &msg, 0);
}

inline void amyEnqueueParam(uint8_t activeSlot, int paramId, float value) {
    if (!amyCmdQueue) return;
    AmyBridgeMsg msg = {AMY_MSG_PARAM_CHANGE, activeSlot, 0, 0, 0, paramId, value, 0, 0, 0};
    xQueueSend(amyCmdQueue, &msg, 0);
}

inline void amyEnqueueDrum(uint8_t drumSlot, int pcmPatch, float tune, float vel, float pan, float fltHz) {
    if (!amyCmdQueue) return;
    AmyBridgeMsg msg = {AMY_MSG_DRUM_TRIGGER, drumSlot, 0, 0, 0, pcmPatch, tune, vel, pan, fltHz};
    xQueueSend(amyCmdQueue, &msg, 0);
}

inline void amyEnqueueMasterVol(float vol) {
    float v = constrain(vol, 0.0f, 1.0f);
    es8311SetMasterStep((int)roundf(v * 10.0f));
    if (!amyCmdQueue) return;
    AmyBridgeMsg msg = {AMY_MSG_MASTER_VOL, 0, 0, 0, 0, 0, vol, 0, 0, 0};
    xQueueSend(amyCmdQueue, &msg, 0);
}

inline void amyEnqueueNote(uint8_t slot, uint8_t midiNote, float velocity) {
    if (!amyCmdQueue) return;
    AmyBridgeMsg msg = {AMY_MSG_NOTE_ON_OFF, slot, 0, 0, midiNote, 0, velocity, 0, 0, 0};
    xQueueSend(amyCmdQueue, &msg, 0);
}

static float g_partVol[16] = {0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f};
inline void applyJunoParamToAmy(uint8_t amySynth, int paramId, float v) {
    static bool  junoGateMode = false;
    static float junoEnvPol = 1.0f;
    static float junoEnvMod = 5.0f;
    static float junoPwm = 0.5f;
    static bool  junoPwmLfo = false;
    static float junoAtk = 20.0f, junoDec = 400.0f, junoSus = 0.7f, junoRel = 300.0f;

    auto sendOsc = [&](uint16_t o, auto fn) {
        amy_event e = amy_default_event(); e.synth = amySynth; e.osc = o; fn(e); amy_add_event(&e);
    };
    auto sendEnv = [&]() {
        sendOsc(0, [&](amy_event &e) {
            *(e.eg0_times + 0) = junoGateMode ? 2 : (int32_t)junoAtk;
            *(e.eg0_values + 0) = 1.0f;
            *(e.eg0_times + 1) = junoGateMode ? 10 : (int32_t)junoDec;
            *(e.eg0_values + 1) = junoGateMode ? 1.0f : junoSus;
            *(e.eg0_times + 2) = junoGateMode ? 15 : (int32_t)junoRel;
            *(e.eg0_values + 2) = 0.0f;
        });
    };
    auto sendPwm = [&]() {
        sendOsc(2, [&](amy_event &e) {
            for (int c = 0; c < NUM_COMBO_COEFS; c++) *(e.duty_coefs + c) = 0.0f;
            *(e.duty_coefs + COEF_CONST) = junoPwm;
            *(e.duty_coefs + COEF_MOD0)  = junoPwmLfo ? 0.35f : 0.0f;
        });
    };

    auto setOscLvl = [&](uint16_t o, float lvl) {
        sendOsc(o, [&](amy_event &e) {
            for (int c = 0; c < NUM_COMBO_COEFS; c++) *(e.amp_coefs + c) = 0.0f;
            *(e.amp_coefs + COEF_CONST) = lvl;
        });
    };

    if (paramId == 0) setOscLvl(3, (v >= 0.5f) ? 1.0f : 0.0f);
    else if (paramId == 1) {
        setOscLvl(2, (v >= 0.5f) ? 1.0f : 0.0f);
        if (v >= 0.5f) sendPwm();
    } else if (paramId == 2 || paramId == 35) {
        if (paramId == 2) junoPwm = 0.5f + (v / 10.0f) * 0.45f;
        if (paramId == 35) junoPwmLfo = (v >= 0.5f);
        sendPwm();
    } else if (paramId == 3) setOscLvl(4, v / 10.0f);
    else if (paramId == 4) setOscLvl(5, v / 10.0f);
    else if (paramId == 36) sendOsc(1, [&](amy_event &e) { *(e.freq_coefs + COEF_NOTE) = 0.0f; *(e.freq_coefs + COEF_CONST) = 0.2f * powf(100.0f, v / 10.0f); });
    else if (paramId == 37) sendOsc(1, [&](amy_event &e) { *(e.eg0_times + 0) = (int32_t)((v / 10.0f) * (v / 10.0f) * 3000.0f); *(e.eg0_values + 0) = 1.0f; });
    else if (paramId == 38) {
        for (uint16_t o = 2; o <= 4; o++) sendOsc(o, [&](amy_event &e) { *(e.freq_coefs + COEF_MOD) = v * 0.15f; });
    } else if (paramId == 39) {
        float oct = (v < -0.5f) ? 0.5f : ((v > 0.5f) ? 2.0f : 1.0f);
        sendOsc(2, [&](amy_event &e) { *(e.freq_coefs + COEF_CONST) = 220.0f * oct; });
        sendOsc(3, [&](amy_event &e) { *(e.freq_coefs + COEF_CONST) = 220.0f * oct; });
        sendOsc(4, [&](amy_event &e) { *(e.freq_coefs + COEF_CONST) = 110.0f * oct; });
    } else if (paramId == 112) {
        static const uint8_t waves[] = {TRIANGLE, SINE, SAW_UP, PULSE, NOISE};
        sendOsc(1, [&](amy_event &e) { e.wave = waves[constrain((int)v, 0, 4)]; });
    } else if (paramId == 5) sendOsc(0, [&](amy_event &e) { *(e.filter_freq_coefs + COEF_CONST) = 20.0f * powf(400.0f, v / 10.0f); });
    else if (paramId == 6) sendOsc(0, [&](amy_event &e) { e.resonance = 0.5f + (v / 10.0f) * 9.5f; });
    else if (paramId == 7 || paramId == 41) {
        if (paramId == 7) junoEnvMod = v;
        if (paramId == 41) junoEnvPol = (v >= 0.5f) ? -1.0f : 1.0f;
        sendOsc(0, [&](amy_event &e) { *(e.filter_freq_coefs + COEF_EG0) = (junoEnvMod / 10.0f) * 6.0f * junoEnvPol; });
    } else if (paramId == 42) sendOsc(0, [&](amy_event &e) { *(e.filter_freq_coefs + COEF_MOD) = (v / 10.0f) * 4.0f; });
    else if (paramId == 43) sendOsc(0, [&](amy_event &e) { *(e.filter_freq_coefs + COEF_NOTE) = v / 10.0f; });
    else if (paramId == 9 || paramId == 10 || paramId == 11 || paramId == 12) {
        float r = constrain(v / 10.0f, 0.0f, 1.0f);
        if (paramId == 9) junoAtk = 1.0f + r * r * 2999.0f;
        else if (paramId == 10) junoDec = 10.0f + r * r * 4990.0f;
        else if (paramId == 11) junoSus = r;
        else junoRel = 10.0f + r * r * 4990.0f;
        sendEnv();
    } else if (paramId == 44) {
        junoGateMode = (v >= 0.5f);
        sendEnv();
    } else if (paramId == 45) {
        amy_event e = amy_default_event(); e.synth = amySynth;
        e.synth_level = (v / 10.0f) * g_partVol[(amySynth - 1) & 15];
        amy_add_event(&e);
    } else if (paramId == 8) {
        static const float hpfHz[] = {0.0f, 140.0f, 300.0f, 620.0f};
        int st = constrain((int)v, 0, 3);
        for (uint16_t o = 2; o <= 5; o++) sendOsc(o, [&](amy_event &e) {
            e.filter_type = (st == 0) ? FILTER_NONE : FILTER_HPF;
            if (st > 0) *(e.filter_freq_coefs + COEF_CONST) = hpfHz[st];
        });
    } else if (paramId == 13) {
        static const float chLvl[] = {0.0f, 0.6f, 0.85f, 1.0f};
        static const float chSpd[] = {0.5f, 0.5f, 0.85f, 3.8f};
        int st = constrain((int)v, 0, 3);
        amy_event e = amy_default_event(); e.bus = 0;
        e.chorus_level = chLvl[st];
        e.chorus_lfo_freq = chSpd[st];
        e.chorus_depth = 0.5f;
        amy_add_event(&e);
    }
}

inline void applyPianoParamToAmy(uint8_t amySynth, int paramId, float v) {
    if (paramId < 104 || paramId > 121) return;
    amy_event e = amy_default_event(); e.synth = amySynth; e.osc = 0;
    if (paramId == 104) { float r = constrain((v - 200.0f) / 7800.0f, 0.0f, 1.0f); e.filter_type = FILTER_LPF; *(e.filter_freq_coefs + COEF_CONST) = 200.0f * powf(40.0f, r); amy_add_event(&e); }
    else if (paramId == 105) { e.resonance = v; amy_add_event(&e); }
    else if (paramId == 106) { *(e.filter_freq_coefs + COEF_NOTE) = v; amy_add_event(&e); }
    else if (paramId == 107) { *(e.amp_coefs + COEF_EG0) = 1.0f; *(e.amp_coefs + COEF_VEL) = v; amy_add_event(&e); }
    else if (paramId == 113) { *(e.filter_freq_coefs + COEF_VEL) = v * 3.0f; amy_add_event(&e); }
    else if (paramId == 114) { *(e.pan_coefs + COEF_CONST) = 0.5f; *(e.pan_coefs + COEF_NOTE) = v * 0.35f; amy_add_event(&e); }
    else if (paramId >= 108 && paramId <= 111) {
        if (paramId == 108) { *(e.eg0_times + 0) = (int32_t)v; *(e.eg0_values + 0) = 1.0f; }
        else if (paramId == 109) { *(e.eg0_times + 1) = (int32_t)v; }
        else if (paramId == 110) { *(e.eg0_values + 1) = v; *(e.eg0_values + 2) = v; }
        else { *(e.eg0_times + 2) = (int32_t)v; *(e.eg0_values + 2) = 0.0f; }
        amy_add_event(&e);
    } else if (paramId >= 115 && paramId <= 117) { amy_event ge = amy_default_event(); ge.bus = 0; if (paramId == 115) ge.eq_l = v; else if (paramId == 116) ge.eq_m = v; else ge.eq_h = v; amy_add_event(&ge); }
    else if (paramId == 118) { e.ratio = 1.0f + (v - 0.5f) * 0.04f; amy_add_event(&e); }
    else if (paramId == 119) { amy_event se = amy_default_event(); se.synth = amySynth; se.synth_level = v * *(g_partVol + ((amySynth - 1) & 15)); amy_add_event(&se); }
    else if (paramId == 120) { *(e.filter_freq_coefs + COEF_EG0) = v * 4.0f; amy_add_event(&e); }
    else if (paramId == 121) { *(e.amp_coefs + COEF_EG0) = 1.0f; *(e.amp_coefs + COEF_NOTE) = (v - 0.5f) * 0.4f; amy_add_event(&e); }
}

inline void applySamplerParamToAmy(uint8_t amySynth, int paramId, float v) {
    if ((paramId < 130 || paramId > 145) && paramId != 153 && paramId != 154) return;
    uint8_t slot = (amySynth - 1) & 15;
    static float sTune[16] = {0}, sFine[16] = {0};
    static float sAtk[16], sDec[16], sSus[16], sRel[16];
    static bool sInit = false;
    if (!sInit) { sInit = true; for (int i = 0; i < 16; i++) { *(sAtk + i) = 5.0f; *(sDec + i) = 500.0f; *(sSus + i) = 0.8f; *(sRel + i) = 250.0f; } }
    amy_event e = amy_default_event(); e.synth = amySynth;
    if (paramId == 130) { e.wave = PCM; int16_t pv = (int16_t)v; e.preset = (pv >= 19) ? (int16_t)(256 + (pv - 19)) : pv; amy_add_event(&e); }
    else if (paramId == 131 || paramId == 132) {
        if (paramId == 131) *(sTune + slot) = v; else *(sFine + slot) = v;
        e.ratio = powf(2.0f, (*(sTune + slot) + *(sFine + slot) * 0.01f) / 12.0f);
        amy_add_event(&e);
    } else if (paramId == 133) { e.feedback = (v >= 0.5f) ? 1.0f : 0.0f; amy_add_event(&e); }
    else if (paramId == 134) { *(e.pan_coefs + COEF_CONST) = v; amy_add_event(&e); }
    else if (paramId == 136) {
        int ft = constrain((int)v, 0, 3);
        e.filter_type = (ft == 0) ? FILTER_NONE : ((ft == 1) ? FILTER_LPF : ((ft == 2) ? FILTER_BPF : FILTER_HPF));
        amy_add_event(&e);
    } else if (paramId == 137) { float r = constrain((v - 40.0f) / 7960.0f, 0.0f, 1.0f); *(e.filter_freq_coefs + COEF_CONST) = 40.0f * powf(200.0f, r); amy_add_event(&e); }
    else if (paramId == 138) { e.resonance = v; amy_add_event(&e); }
    else if (paramId == 139) { *(e.filter_freq_coefs + COEF_NOTE) = v; amy_add_event(&e); }
    else if (paramId == 140) { *(e.filter_freq_coefs + COEF_EG0) = v * 4.0f; amy_add_event(&e); }
    else if (paramId >= 141 && paramId <= 144) {
        if (paramId == 141) *(sAtk + slot) = v;
        else if (paramId == 142) *(sDec + slot) = v;
        else if (paramId == 143) *(sSus + slot) = v;
        else *(sRel + slot) = v;
        *(e.eg0_times + 0) = (int32_t)*(sAtk + slot); *(e.eg0_values + 0) = 1.0f;
        *(e.eg0_times + 1) = (int32_t)*(sDec + slot); *(e.eg0_values + 1) = *(sSus + slot);
        *(e.eg0_times + 2) = (int32_t)*(sRel + slot); *(e.eg0_values + 2) = 0.0f;
        amy_add_event(&e);
    } else if (paramId == 145) { *(e.amp_coefs + COEF_VEL) = v; amy_add_event(&e); }
    else if (paramId == 153) { *(e.freq_coefs + COEF_NOTE) = (v >= 0.5f) ? 1.0f : 0.0f; amy_add_event(&e); }
    else if (paramId == 154) { float r = constrain(v / 10.0f, 0.0f, 1.0f); e.portamento_ms = (uint16_t)(r * r * 1000.0f); amy_add_event(&e); }
}

inline void applyDx7ParamToAmy(uint8_t amySynth, int paramId, float v) {
    static const int lvlIds[] = {16,18,27,29,31,33}, ratIds[] = {17,19,28,30,32,34};
    static float opCoarse[16][6], opFine[16][6], opDet[16][6], opAms[16][6];
    static float opEgR[16][6][4], opEgL[16][6][4];
    static bool opFix[16][6]; static float gAmd[16] = {0};
    uint8_t slot = (amySynth - 1) & 15;
    static const uint8_t cMask[32] = {0x05,0x05,0x09,0x09,0x15,0x15,0x05,0x05,0x05,0x09,0x09,0x05,0x05,0x05,0x05,0x01,0x01,0x01,0x19,0x1B,0x1B,0x1D,0x1B,0x1F,0x1F,0x0B,0x0B,0x0B,0x17,0x0F,0x1F,0x3F};
    static float opBase[16][6], curAlg[16], modBal[16];
    static bool stInit = false;
    if (!stInit) { stInit = true; static const float dR[4] = {95,70,40,50}, dL[4] = {99,80,60,0}; for (int i = 0; i < 16; i++) { *(curAlg + i) = 1.0f; *(modBal + i) = 50.0f; *(*(opBase + i) + 0) = 99.0f; for (int o = 0; o < 6; o++) { *(*(opCoarse + i) + o) = 1.0f; for (int k = 0; k < 4; k++) { *(*(*(opEgR + i) + o) + k) = *(dR + k); *(*(*(opEgL + i) + o) + k) = *(dL + k); } } } }
    auto sendOpEnv = [&](uint16_t op) {
        amy_event e = amy_default_event(); e.synth = amySynth; e.osc = (uint16_t)(7 - op);
        float l4 = (opEgL[slot][op][3] < 1.0f) ? 0.000188f : (opEgL[slot][op][3] / 99.0f);
        *(e.eg0_times + 0) = 0; *(e.eg0_values + 0) = l4;
        for (int st = 0; st < 4; st++) {
            float nr = constrain((99.0f - opEgR[slot][op][st]) / 99.0f, 0.0f, 1.0f);
            *(e.eg0_times + st + 1) = (int32_t)(powf(nr, 2.2f) * 15000.0f);
            float lv = opEgL[slot][op][st] / 99.0f;
            *(e.eg0_values + st + 1) = (st == 3 && opEgL[slot][op][st] < 1.0f) ? 0.000188f : lv;
        }
        amy_add_event(&e);
    };
    auto setOpLvl = [&](uint16_t op, float raw99) {
        *(*(opBase + slot) + op) = raw99;
        int ai = constrain((int)*(curAlg + slot), 1, 32) - 1;
        float lv99 = raw99, bal = *(modBal + slot);
        if (!((*(cMask + ai) >> op) & 1)) {
            if (bal < 50.0f) lv99 = raw99 * (bal / 50.0f);
            else if (bal > 50.0f) lv99 = raw99 + (99.0f - raw99) * ((bal - 50.0f) / 50.0f);
        }
        amy_event e = amy_default_event(); e.synth = amySynth; e.osc = (uint16_t)(7 - op);
        *(e.amp_coefs + COEF_CONST) = 0.0f;
        *(e.amp_coefs + COEF_EG0) = (lv99 <= 0.5f) ? 0.0f : ((lv99 / 99.0f) * 2.0f);
        *(e.amp_coefs + COEF_MOD0) = (*(gAmd + slot) / 99.0f) * (*(*(opAms + slot) + op) / 3.0f) * 0.5f;
        amy_add_event(&e);
    };
    if (paramId == 14 || paramId == 155) {
        if (paramId == 14) { *(curAlg + slot) = v; amy_event ea = amy_default_event(); ea.synth = amySynth; ea.algorithm = (uint8_t)v; amy_add_event(&ea); }
        else *(modBal + slot) = constrain(v, 0.0f, 100.0f);
        for (uint16_t op = 0; op < 6; op++) setOpLvl(op, *(*(opBase + slot) + op));
        return;
    }
    for (uint16_t op = 0; op < 6; op++) {
        if (paramId == lvlIds[op]) { setOpLvl(op, v); return; }
        if (paramId == ratIds[op]) { amy_event e = amy_default_event(); e.synth = amySynth; e.osc = (uint16_t)(7 - op); e.ratio = v; amy_add_event(&e); return; }
    }
    if (paramId >= 300 && paramId < 420) {
        uint16_t op = (uint16_t)((paramId - 300) / 20); int k = (paramId - 300) % 20; if (op >= 6) return;
        amy_event e = amy_default_event(); e.synth = amySynth; e.osc = (uint16_t)(7 - op);
        if (k == 0) { setOpLvl(op, v); return; }
        else if (k >= 1 && k <= 4) {
            if (k == 1) opCoarse[slot][op] = v; else if (k == 2) opFine[slot][op] = v; else if (k == 3) opDet[slot][op] = v; else opFix[slot][op] = (v >= 0.5f);
            if (opFix[slot][op]) { e.ratio = 0.0f; *(e.freq_coefs + COEF_NOTE) = 0.0f; *(e.freq_coefs + COEF_CONST) = powf(9772.0f, (opCoarse[slot][op] * 100.0f + opFine[slot][op]) / 3199.0f); }
            else { float c = (opCoarse[slot][op] < 0.5f) ? 0.5f : opCoarse[slot][op]; *(e.freq_coefs + COEF_NOTE) = 1.0f; *(e.freq_coefs + COEF_CONST) = 0.0f; e.ratio = c * (1.0f + opFine[slot][op] * 0.01f) + opDet[slot][op] * 0.0015f; }
        } else if (k == 5) { e.osc = 0; *(e.amp_coefs + COEF_VEL) = v / 7.0f; }
        else if (k >= 6 && k <= 9) { opEgR[slot][op][k - 6] = v; sendOpEnv(op); return; }
        else if (k >= 10 && k <= 13) { opEgL[slot][op][k - 10] = v; sendOpEnv(op); return; }
        else if (k == 14) { opAms[slot][op] = v; *(e.amp_coefs + COEF_MOD0) = (*(gAmd + slot) / 99.0f) * (v / 3.0f) * 0.5f; }
        else if (k == 15) { *(e.amp_coefs + COEF_NOTE) = -v * 0.04f; }
        amy_add_event(&e); return;
    }
    if (paramId >= 49 && paramId <= 53) {
        amy_event e = amy_default_event(); e.synth = amySynth;
        if (paramId == 49) { e.osc = 0; *(e.freq_coefs + COEF_CONST) = 261.625565f * powf(2.0f, v); amy_add_event(&e); }
        else if (paramId == 50) { e.osc = 1; *(e.freq_coefs + COEF_NOTE) = 0.0f; *(e.freq_coefs + COEF_CONST) = 0.06f * powf(600.0f, v / 99.0f); amy_add_event(&e); }
        else if (paramId == 51) { e.osc = 1; *(e.amp_coefs + COEF_CONST) = 1.0f; *(e.amp_coefs + COEF_EG0) = 1.0f; *(e.eg0_times + 0) = 0; *(e.eg0_values + 0) = 0.0f; *(e.eg0_times + 1) = (int32_t)((v / 99.0f) * (v / 99.0f) * 3000.0f); *(e.eg0_values + 1) = 1.0f; amy_add_event(&e); }
        else if (paramId == 52) { e.osc = 0; *(e.freq_coefs + COEF_MOD0) = (v / 99.0f) * 0.08f; amy_add_event(&e); }
        else if (paramId == 53) { *(gAmd + slot) = v; for (uint16_t op = 0; op < 6; op++) { amy_event eo = amy_default_event(); eo.synth = amySynth; eo.osc = (uint16_t)(7 - op); *(eo.amp_coefs + COEF_MOD0) = (*(gAmd + slot) / 99.0f) * (opAms[slot][op] / 3.0f) * 0.5f; amy_add_event(&eo); } }
    } else if (paramId >= 420 && paramId <= 429) {
        int pk = paramId - 420; amy_event e = amy_default_event(); e.synth = amySynth;
        if (pk == 8) { static const uint8_t dw[] = {TRIANGLE, SAW_DOWN, SAW_UP, PULSE, SINE, NOISE}; e.osc = 1; e.wave = dw[constrain((int)v, 0, 5)]; amy_add_event(&e); }
        else if (pk == 9) { e.osc = 1; e.trigger_phase = (v >= 0.5f) ? 0.25f : AMY_UNSET_FLOAT; amy_add_event(&e); }
        else if (pk < 8) { e.osc = 0; *(e.freq_coefs + COEF_EG0) = 1.0f; *(e.eg0_times + 0) = 0; *(e.eg0_values + 0) = 0.0f; if (pk < 4) *(e.eg0_times + pk + 1) = (int32_t)(powf((99.0f - v) / 99.0f, 2.2f) * 15000.0f); else *(e.eg0_values + pk - 3) = (v - 50.0f) / 50.0f; amy_add_event(&e); }
    }
}

static uint8_t g_partEngine[16] = {0};
static uint8_t *g_activeNotes = nullptr;
inline void processAmyBridgeMsg(const AmyBridgeMsg &m) {
    uint8_t amySynth = m.synthSlot + 1;
    if (m.type == AMY_MSG_PART_LEVEL) {
        g_partVol[m.synthSlot & 15] = constrain(m.val1, 0.0f, 1.5f);
        amy_event e = amy_default_event(); e.synth = amySynth;
        e.synth_level = g_partVol[m.synthSlot & 15];
        amy_add_event(&e);
    } else if (m.type == AMY_MSG_PART_CONFIG) {
        uint8_t sl = m.synthSlot & 15;
        if (g_activeNotes && *(g_activeNotes + sl) > 0) {
            amy_event ne = amy_default_event(); ne.synth = amySynth; ne.midi_note = *(g_activeNotes + sl); ne.velocity = 0.0f; amy_add_event(&ne);
            *(g_activeNotes + sl) = 0;
        }
        *(g_partEngine + sl) = m.engine;
        amy_event offE = amy_default_event(); offE.synth = amySynth; offE.num_voices = 0; amy_add_event(&offE);
        if (m.engine == 0 || m.voices == 0) return;
        g_partVol[m.synthSlot & 15] = constrain(m.val1, 0.0f, 1.5f);
        uint32_t sig = ((uint32_t)m.engine << 24) | ((uint32_t)m.voices << 16) | m.patchOrNote;
        amy_event e = amy_default_event();
        e.synth = amySynth;
        e.num_voices = m.voices;
        if (m.engine == 3) { int16_t pv = (int16_t)m.patchOrNote; e.oscs_per_voice = 1; e.wave = PCM; e.preset = (pv >= 19) ? (int16_t)(256 + (pv - 19)) : pv; }
        else if (m.engine == 4) { e.oscs_per_voice = 1; e.wave = PCM; }
        else if (m.engine == 5) { e.patch_number = 256; }
        else { e.patch_number = m.patchOrNote; }
        e.synth_level = g_partVol[m.synthSlot & 15];
        amy_add_event(&e);
    } else if (m.type == AMY_MSG_MASTER_VOL) {
        amy_event e = amy_default_event();
        e.volume = m.val1 * 2.2f;
        amy_add_event(&e);
    } else if (m.type == AMY_MSG_NOTE_ON_OFF) { if (m.val1 > 0.01f) g_moodPulseMs = millis();
        amy_event e = amy_default_event();
        e.synth = amySynth;
        e.midi_note = m.patchOrNote;
        e.velocity = m.val1;
        amy_add_event(&e);
    } else if (m.type == AMY_MSG_DRUM_TRIGGER) { g_moodPulseMs = millis();
        amy_event e = amy_default_event();
        e.osc = (uint16_t)(100 + m.synthSlot);
        e.wave = PCM;
        int16_t pv = (int16_t)m.paramId; e.preset = (pv >= 19) ? (int16_t)(256 + (pv - 19)) : pv;
        e.ratio = m.val1;
        e.velocity = m.val2;
        *(e.pan_coefs + COEF_CONST) = m.val3;
        if (m.val4 < 7900.0f) {
            e.filter_type = FILTER_LPF;
            *(e.filter_freq_coefs + COEF_CONST) = m.val4;
        }
        amy_add_event(&e);
    } else if (m.type == AMY_MSG_PARAM_CHANGE) {
        amy_event e = amy_default_event();
        e.synth = amySynth;
        uint8_t eng = *(g_partEngine + (m.synthSlot & 15));
        if (eng == 1) applyJunoParamToAmy(amySynth, m.paramId, m.val1);
        else if (eng == 5) applyPianoParamToAmy(amySynth, m.paramId, m.val1);
        else if (eng == 3) applySamplerParamToAmy(amySynth, m.paramId, m.val1);
        else if (eng == 2) applyDx7ParamToAmy(amySynth, m.paramId, m.val1);
        switch (m.paramId) {
                                                case 40: { float r = constrain(m.val1 / 10.0f, 0.0f, 1.0f); e.portamento_ms = (uint16_t)(r * r * 1000.0f); amy_add_event(&e); break; }
                        case 14: e.algorithm = (uint8_t)m.val1; amy_add_event(&e); break;
            case 15: e.feedback = (m.val1 / 7.0f) * 0.32f; amy_add_event(&e); break;
            case 46: { amy_event ge = amy_default_event(); ge.bus = 0; ge.echo_level = m.val1; amy_add_event(&ge); break; }
            case 47: { amy_event ge = amy_default_event(); ge.bus = 0; ge.echo_delay_ms = m.val1; amy_add_event(&ge); break; }
            case 48: { amy_event ge = amy_default_event(); ge.bus = 0; ge.echo_feedback = m.val1; amy_add_event(&ge); break; }
            case 148: { amy_event ge = amy_default_event(); ge.bus = 0; ge.echo_filter_coef = m.val1; amy_add_event(&ge); break; }
            case 149: { amy_event ge = amy_default_event(); ge.bus = 0; ge.reverb_level = m.val1; amy_add_event(&ge); break; }
            case 150: { amy_event ge = amy_default_event(); ge.bus = 0; ge.reverb_liveness = m.val1; amy_add_event(&ge); break; }
            case 151: { amy_event ge = amy_default_event(); ge.bus = 0; ge.reverb_damping = m.val1; amy_add_event(&ge); break; }
            case 152: { amy_event ge = amy_default_event(); ge.bus = 0; ge.reverb_xover_hz = m.val1; amy_add_event(&ge); break; }

            default: break;
        }
    }
}

inline void amyEngineTask(void *pvParameters) {
    Serial.printf("[AMY Task] Startad pa Core %d\n", xPortGetCoreID());
    amyI2s.setPins(5, 7, 8, 6, 4);
    amyI2s.begin(I2S_MODE_STD, AMY_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
    amy_config_t cfg = amy_default_config();
    cfg.features.startup_bleep = 1;
    cfg.i2s_dout = -1; cfg.i2s_bclk = -1; cfg.i2s_lrc = -1;
    extern const int16_t gamma9001_pcm_data[];
    amy_set_gamma9001_pcm(gamma9001_pcm_data);
    amy_start(cfg);
    delay(30);

    static float seqBpm = 120.0f, seqSwing = 0.0f;
    static uint32_t nextStepMs = 0;
    static int curStep = 0;
    if (!g_activeNotes) g_activeNotes = (uint8_t*)ps_calloc(16, 1);
    uint8_t *activeNotes = g_activeNotes;
    AmyBridgeMsg msg;
    for (;;) {
        int msgBudget = 8;
        while (msgBudget-- > 0 && amyCmdQueue && xQueueReceive(amyCmdQueue, &msg, 0) == pdTRUE) {
            if (msg.type == AMY_MSG_PARAM_CHANGE) {
                if (msg.paramId == 85) seqBpm = constrain(msg.val1, 40.0f, 240.0f);
                else if (msg.paramId == 86) seqSwing = constrain(msg.val1, 0.0f, 75.0f);
                else if (msg.paramId == 84 && msg.val1 < 0.5f) {
                    for (uint8_t k = 0; k < 16; k++) if (*(activeNotes + k) > 0) { amy_event offE = amy_default_event(); offE.synth = k + 1; offE.midi_note = *(activeNotes + k); offE.velocity = 0.0f; amy_add_event(&offE); *(activeNotes + k) = 0; }
                    curStep = 0;
                }
            }
            processAmyBridgeMsg(msg);
        }
        uint32_t now = millis();
        if (seqIsPlaying) {
            bool due = seqExtClockActive ? seqExtStepPulse : (nextStepMs == 0 || (int32_t)(now - nextStepMs) >= 0);
            if (due) {
                seqExtStepPulse = false;
                if (curStep >= seqMaxSteps) curStep = 0;
                seqPlayheadStep = curStep;
                for (uint8_t k = 0; k < 16; k++) if (*(activeNotes + k) > 0) { amy_event offE = amy_default_event(); offE.synth = k + 1; offE.midi_note = *(activeNotes + k); offE.velocity = 0.0f; amy_add_event(&offE); *(activeNotes + k) = 0; }
                for (int tr = 0; tr < SEQ_TRACKS; tr++) {
                    if (*(seqTrackMuted + tr)) continue;
                    uint8_t val = seqPatterns[currentSeqPattern][tr][curStep];
                    if (val == 0) continue; g_moodPulseMs = now;
                    if (strcmp(getSynthSlotEngineShortName(tr), "Kit") == 0) {
                        extern void triggerDrumOnSlot(int drumSlot, int drumIdx, float velScale);
                        bool anyD = false;
                        for (int d = 0; d < 9; d++) {
                            uint8_t dv = seqDrumVel ? drumVelAt(currentSeqPattern, tr, curStep, d) : 0;
                            if (dv > 0) { triggerDrumOnSlot(tr, d, dv / 127.0f); anyD = true; }
                        }
                        if (!anyD) { int dIdx = (val >= 36) ? ((val - 36) % 9) : ((val - 1) % 9); triggerDrumOnSlot(tr, dIdx, 0.85f); }
                    } else {
                        uint8_t slot = (uint8_t)tr;
                        uint8_t sv = seqStepVel ? stepVelAt(currentSeqPattern, tr, curStep) : 100;
                        amy_event onE = amy_default_event(); onE.synth = slot + 1; onE.midi_note = val; onE.velocity = (sv > 0 ? sv : 100) / 127.0f; amy_add_event(&onE); *(activeNotes + slot) = val;
                    }
                }
                float baseStepMs = 15000.0f / seqBpm;
                float swingOffset = ((curStep % 2) == 0) ? (baseStepMs * (seqSwing / 150.0f)) : -(baseStepMs * (seqSwing / 150.0f));
                nextStepMs = now + (uint32_t)max(20.0f, baseStepMs + swingOffset);
                curStep = (curStep + 1) % seqMaxSteps;
                if (curStep == 0 && seqSongModeActive) {
                    int nextPos = (seqSongPos + 1) % SEQ_SONG_STEPS; uint8_t nextPat = *(seqSongChain + nextPos);
                    if (nextPat == 0 || nextPat > SEQ_PATTERNS) { nextPos = 0; nextPat = *(seqSongChain + 0); }
                    if (nextPat >= 1 && nextPat <= SEQ_PATTERNS) { seqSongPos = nextPos; currentSeqPattern = nextPat - 1; }
                }
            }
        } else { nextStepMs = 0; curStep = 0; seqSongPos = 0; }
        int16_t *pcm = amy_render_audio();
        if (pcm) { for (int k = 0; k < AMY_BLOCK_SIZE * 2; k++) *(pcm + k) = (*(pcm + k)) >> 2; amyI2s.write((uint8_t *)pcm, AMY_BLOCK_SIZE * 4); }
    }
}
inline void startAmyBridge() { if (!amyCmdQueue) amyCmdQueue = xQueueCreate(128, sizeof(AmyBridgeMsg)); xTaskCreatePinnedToCore(amyEngineTask, "AMY_Task", 16384, nullptr, 2, nullptr, 1); }
