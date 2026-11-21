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

inline void applyJunoParamToAmy(uint8_t amySynth, int paramId, float v) {
    amy_event e = amy_default_event();
    e.synth = amySynth;
    if (paramId == 0) {
        e.osc = 0; *(e.amp_coefs + COEF_CONST) = v; amy_add_event(&e);
    } else if (paramId == 1) {
        e.osc = 1; *(e.amp_coefs + COEF_CONST) = v; amy_add_event(&e);
    } else if (paramId == 2) {
        e.osc = 1; *(e.duty_coefs + COEF_CONST) = v; amy_add_event(&e);
    } else if (paramId == 35) {
        e.osc = 1;
        *(e.duty_coefs + COEF_MOD) = (v >= 0.5f) ? 0.4f : 0.0f;
        *(e.duty_coefs + COEF_EG0) = 0.0f;
        amy_add_event(&e);
    } else if (paramId == 3) {
        e.osc = 2; *(e.amp_coefs + COEF_CONST) = v; amy_add_event(&e);
    } else if (paramId == 4) {
        e.osc = 3; *(e.amp_coefs + COEF_CONST) = v; amy_add_event(&e);
    } else if (paramId == 36) {
        e.osc = 4; *(e.freq_coefs + COEF_CONST) = v; amy_add_event(&e);
    } else if (paramId == 37) {
        e.osc = 4; *(e.eg0_times + 0) = (int32_t)v; *(e.eg0_values + 0) = 1.0f; amy_add_event(&e);
    } else if (paramId == 38) {
        for (uint16_t o = 0; o < 3; o++) {
            amy_event eo = amy_default_event(); eo.synth = amySynth; eo.osc = o;
            *(eo.freq_coefs + COEF_MOD) = v * 0.5f; amy_add_event(&eo);
        }
    } else if (paramId == 39) {
        float oct = (v < -0.5f) ? 0.5f : ((v > 0.5f) ? 2.0f : 1.0f);
        for (uint16_t o = 0; o < 2; o++) {
            amy_event eo = amy_default_event(); eo.synth = amySynth; eo.osc = o;
            eo.ratio = oct; amy_add_event(&eo);
        }
    } else if (paramId == 112) {
        static const uint8_t waves[5] = {TRIANGLE, SINE, SAW_UP, PULSE, NOISE};
        int wi = constrain((int)v, 0, 4);
        e.osc = 4; e.wave = *(waves + wi); amy_add_event(&e);
    } else if (paramId == 41) {
        static float lastPol = 1.0f;
        lastPol = (v >= 0.5f) ? -1.0f : 1.0f;
        *(e.filter_freq_coefs + COEF_EG0) = 2.0f * lastPol; amy_add_event(&e);
    } else if (paramId == 42) {
        *(e.filter_freq_coefs + COEF_MOD) = v * 3.0f; amy_add_event(&e);
    } else if (paramId == 43) {
        *(e.filter_freq_coefs + COEF_NOTE) = v; amy_add_event(&e);
    } else if (paramId == 9) {
        *(e.eg0_times + 0) = (int32_t)v; *(e.eg0_values + 0) = 1.0f; amy_add_event(&e);
    } else if (paramId == 10) {
        *(e.eg0_times + 1) = (int32_t)v; amy_add_event(&e);
    } else if (paramId == 11) {
        *(e.eg0_values + 1) = v; *(e.eg0_values + 2) = v; amy_add_event(&e);
    } else if (paramId == 12) {
        *(e.eg0_times + 3) = (int32_t)v; *(e.eg0_values + 3) = 0.0f; amy_add_event(&e);
    } else if (paramId == 44) {
        bool isGate = (v >= 0.5f);
        *(e.amp_coefs + COEF_EG0) = isGate ? 0.0f : 1.0f;
        *(e.amp_coefs + COEF_CONST) = isGate ? 0.85f : 0.0f;
        amy_add_event(&e);
    } else if (paramId == 8) {
        int hpf = constrain((int)v, 0, 3);
        e.filter_type = (hpf > 0) ? FILTER_HPF : FILTER_LPF;
        if (hpf > 0) *(e.filter_freq_coefs + COEF_CONST) = (float)(hpf * 180);
        amy_add_event(&e);
    } else if (paramId == 13) {
        static const float chLvl[4] = {0.0f, 0.5f, 0.8f, 1.0f};
        e.chorus_level = *(chLvl + constrain((int)v, 0, 3));
        amy_add_event(&e);
    }
}

inline void applyPianoParamToAmy(uint8_t amySynth, int paramId, float v) {
    amy_event e = amy_default_event(); e.synth = amySynth;
    if (paramId == 104) { e.filter_type = FILTER_LPF; *(e.filter_freq_coefs + COEF_CONST) = v; amy_add_event(&e); }
    else if (paramId == 105) { e.resonance = v; amy_add_event(&e); }
    else if (paramId == 106) { *(e.filter_freq_coefs + COEF_NOTE) = v; amy_add_event(&e); }
    else if (paramId == 107) { *(e.amp_coefs + COEF_VEL) = v; amy_add_event(&e); }
    else if (paramId == 113) { *(e.filter_freq_coefs + COEF_VEL) = v * 3.0f; amy_add_event(&e); }
    else if (paramId == 114) {
        for (uint16_t o = 0; o < 11; o++) {
            amy_event eo = amy_default_event(); eo.synth = amySynth; eo.osc = o;
            float side = (o % 2 == 0) ? -1.0f : 1.0f;
            *(eo.pan_coefs + COEF_CONST) = constrain(0.5f + side * v * 0.45f * ((float)o / 10.0f), 0.0f, 1.0f);
            amy_add_event(&eo);
        }
    } else if (paramId >= 108 && paramId <= 111) {
        if (paramId == 108) { *(e.eg0_times + 0) = (int32_t)v; *(e.eg0_values + 0) = 1.0f; }
        else if (paramId == 109) { *(e.eg0_times + 1) = (int32_t)v; }
        else if (paramId == 110) { *(e.eg0_values + 1) = v; *(e.eg0_values + 2) = v; }
        else { *(e.eg0_times + 3) = (int32_t)v; *(e.eg0_values + 3) = 0.0f; }
        amy_add_event(&e);
    } else if (paramId >= 115 && paramId <= 117) {
        int wi = constrain((int)v, 0, 3);
        uint16_t o0 = (paramId == 115) ? 0 : ((paramId == 116) ? 4 : 9);
        uint16_t o1 = (paramId == 115) ? 3 : ((paramId == 116) ? 8 : 10);
        uint8_t w = (wi == 0) ? SINE : ((wi == 1) ? TRIANGLE : ((wi == 2) ? SAW_UP : PULSE));
        if (paramId == 117) w = (wi == 0) ? TRIANGLE : ((wi == 1) ? PULSE : ((wi == 2) ? NOISE : SINE));
        for (uint16_t o = o0; o <= o1; o++) { amy_event eo = amy_default_event(); eo.synth = amySynth; eo.osc = o; eo.wave = w; amy_add_event(&eo); }
    } else if (paramId == 118) {
        for (uint16_t o = 1; o < 11; o++) { amy_event eo = amy_default_event(); eo.synth = amySynth; eo.osc = o; eo.ratio = (float)(o + 1) * (1.0f + ((o % 2 == 0) ? 1.0f : -1.0f) * v * 0.008f * (float)o); amy_add_event(&eo); }
    } else if (paramId == 119) { e.osc = 0; *(e.amp_coefs + COEF_CONST) = v; amy_add_event(&e); }
    else if (paramId == 120) { for (uint16_t o = 1; o <= 6; o++) { amy_event eo = amy_default_event(); eo.synth = amySynth; eo.osc = o; *(eo.amp_coefs + COEF_CONST) = (v * 0.5f) / (float)o; amy_add_event(&eo); } }
    else if (paramId == 121) { for (uint16_t o = 7; o <= 10; o++) { amy_event eo = amy_default_event(); eo.synth = amySynth; eo.osc = o; *(eo.amp_coefs + COEF_CONST) = v * 0.25f; amy_add_event(&eo); } }
}

inline void applySamplerParamToAmy(uint8_t amySynth, int paramId, float v) {
    if (paramId < 130 || paramId > 145) return;
    static float sTune = 0.0f;
    static float sFine = 0.0f;
    amy_event e = amy_default_event(); e.synth = amySynth;
    if (paramId == 130) { e.wave = PCM; e.patch_number = (uint16_t)v; amy_add_event(&e); }
    else if (paramId == 131 || paramId == 132) {
        if (paramId == 131) sTune = v; else sFine = v;
        e.ratio = powf(2.0f, (sTune + sFine * 0.01f) / 12.0f);
        amy_add_event(&e);
    } else if (paramId == 133) { e.feedback = (v >= 0.5f) ? 1.0f : 0.0f; amy_add_event(&e); }
    else if (paramId == 134) { *(e.pan_coefs + COEF_CONST) = v; amy_add_event(&e); }
    else if (paramId == 135) { *(e.amp_coefs + COEF_CONST) = v; amy_add_event(&e); }
    else if (paramId == 136) {
        int ft = constrain((int)v, 0, 3);
        e.filter_type = (ft == 0) ? FILTER_NONE : ((ft == 1) ? FILTER_LPF : ((ft == 2) ? FILTER_BPF : FILTER_HPF));
        amy_add_event(&e);
    } else if (paramId == 137) { *(e.filter_freq_coefs + COEF_CONST) = v; amy_add_event(&e); }
    else if (paramId == 138) { e.resonance = v; amy_add_event(&e); }
    else if (paramId == 139) { *(e.filter_freq_coefs + COEF_NOTE) = v; amy_add_event(&e); }
    else if (paramId == 140) { *(e.filter_freq_coefs + COEF_EG0) = v * 4.0f; amy_add_event(&e); }
    else if (paramId == 141) { *(e.eg0_times + 0) = (int32_t)v; *(e.eg0_values + 0) = 1.0f; amy_add_event(&e); }
    else if (paramId == 142) { *(e.eg0_times + 1) = (int32_t)v; amy_add_event(&e); }
    else if (paramId == 143) { *(e.eg0_values + 1) = v; *(e.eg0_values + 2) = v; amy_add_event(&e); }
    else if (paramId == 144) { *(e.eg0_times + 3) = (int32_t)v; *(e.eg0_values + 3) = 0.0f; amy_add_event(&e); }
    else if (paramId == 145) { *(e.amp_coefs + COEF_VEL) = v; amy_add_event(&e); }
}

inline void applyDx7ParamToAmy(uint8_t amySynth, int paramId, float v) {
    static const int lvlIds[6] = {16, 18, 27, 29, 31, 33};
    static const int ratIds[6] = {17, 19, 28, 30, 32, 34};
    static float opCoarse[6] = {1, 2, 1, 1, 3, 4};
    static float opFine[6]   = {0, 0, 0, 0, 0, 0};
    static float opDetune[6] = {0, 0, 0, 0, 0, 0};
    static bool  opFixed[6]  = {false, false, false, false, false, false};

    for (uint16_t op = 0; op < 6; op++) {
        if (paramId == *(lvlIds + op)) {
            amy_event e = amy_default_event(); e.synth = amySynth; e.osc = op;
            *(e.amp_coefs + COEF_CONST) = v / 99.0f;
            amy_add_event(&e);
            return;
        }
        if (paramId == *(ratIds + op)) {
            amy_event e = amy_default_event(); e.synth = amySynth; e.osc = op;
            e.ratio = v;
            amy_add_event(&e);
            return;
        }
    }

    if (paramId >= 300 && paramId < 420) {
        uint16_t op = (uint16_t)((paramId - 300) / 20);
        int k = (paramId - 300) % 20;
        if (op >= 6) return;
        amy_event e = amy_default_event(); e.synth = amySynth; e.osc = op;
        if (k == 0) {
            *(e.amp_coefs + COEF_CONST) = v / 99.0f;
        } else if (k >= 1 && k <= 4) {
            if (k == 1) *(opCoarse + op) = v;
            else if (k == 2) *(opFine + op) = v;
            else if (k == 3) *(opDetune + op) = v;
            else *(opFixed + op) = (v >= 0.5f);
            float c = (*(opCoarse + op) < 0.5f) ? 0.5f : *(opCoarse + op);
            float mult = c * (1.0f + *(opFine + op) * 0.01f) + *(opDetune + op) * 0.002f;
            if (*(opFixed + op)) {
                *(e.freq_coefs + COEF_NOTE) = 0.0f;
                *(e.freq_coefs + COEF_CONST) = 440.0f * mult;
            } else {
                *(e.freq_coefs + COEF_NOTE) = 1.0f;
                e.ratio = mult;
            }
        } else if (k == 5) {
            *(e.amp_coefs + COEF_VEL) = v / 7.0f;
        } else if (k >= 6 && k <= 9) {
            int st = k - 6;
            *(e.eg0_times + st) = (int32_t)((100.0f - v) * 35.0f);
        } else if (k >= 10 && k <= 13) {
            int st = k - 10;
            *(e.eg0_values + st) = v / 99.0f;
        } else if (k == 14) {
            *(e.amp_coefs + COEF_MOD) = v / 3.0f;
        } else if (k == 15) {
            *(e.amp_coefs + COEF_NOTE) = -v * 0.05f;
        }
        amy_add_event(&e);
        return;
    }

    if (paramId >= 49 && paramId <= 54) {
        if (paramId == 49) {
            for (uint16_t op = 0; op < 6; op++) {
                amy_event e = amy_default_event(); e.synth = amySynth; e.osc = op;
                *(e.freq_coefs + COEF_CONST) = v * 12.0f;
                amy_add_event(&e);
            }
        } else if (paramId == 50) {
            amy_event e = amy_default_event(); e.synth = amySynth; e.osc = 6;
            *(e.freq_coefs + COEF_CONST) = 0.2f + (v / 99.0f) * 18.0f;
            amy_add_event(&e);
        } else if (paramId == 51) {
            amy_event e = amy_default_event(); e.synth = amySynth; e.osc = 6;
            *(e.eg0_times + 0) = (int32_t)(v * 30.0f); *(e.eg0_values + 0) = 1.0f;
            amy_add_event(&e);
        } else if (paramId == 52) {
            for (uint16_t op = 0; op < 6; op++) {
                amy_event e = amy_default_event(); e.synth = amySynth; e.osc = op;
                *(e.freq_coefs + COEF_MOD) = (v / 99.0f) * 0.5f;
                amy_add_event(&e);
            }
        } else if (paramId == 53) {
            for (uint16_t op = 0; op < 6; op++) {
                amy_event e = amy_default_event(); e.synth = amySynth; e.osc = op;
                *(e.amp_coefs + COEF_MOD) = (v / 99.0f) * 0.8f;
                amy_add_event(&e);
            }
        }
        return;
    }

    if (paramId >= 420 && paramId <= 429) {
        int pk = paramId - 420;
        if (pk == 8) {
            static const uint8_t dWaves[6] = {TRIANGLE, SAW_DOWN, SAW_UP, PULSE, SINE, NOISE};
            amy_event e = amy_default_event(); e.synth = amySynth; e.osc = 6;
            e.wave = *(dWaves + constrain((int)v, 0, 5));
            amy_add_event(&e);
        } else if (pk < 8) {
            for (uint16_t op = 0; op < 6; op++) {
                amy_event e = amy_default_event(); e.synth = amySynth; e.osc = op;
                if (pk < 4) *(e.eg1_times + pk) = (int32_t)((100.0f - v) * 30.0f);
                else *(e.eg1_values + (pk - 4)) = (v - 50.0f) / 50.0f;
                *(e.freq_coefs + COEF_EG1) = 1.0f;
                amy_add_event(&e);
            }
        }
    }
}

static float g_partVol[16] = {0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f,0.85f};
static uint32_t g_lastPartSig[16] = {0};

inline void processAmyBridgeMsg(const AmyBridgeMsg &m) {
    uint8_t amySynth = m.synthSlot + 1;
    if (m.type == AMY_MSG_PART_CONFIG) {
        if (m.engine == 0 || m.voices == 0) {
            amy_event offE = amy_default_event(); offE.synth = amySynth; offE.num_voices = 0; amy_add_event(&offE); return;
        }
        g_partVol[m.synthSlot & 15] = constrain(m.val1, 0.0f, 1.5f);
        uint32_t sig = ((uint32_t)m.engine << 24) | ((uint32_t)m.voices << 16) | m.patchOrNote;
        amy_event e = amy_default_event();
        e.synth = amySynth;
        e.num_voices = m.voices;
        if (m.engine == 3) { e.oscs_per_voice = 1; e.wave = PCM; }
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
        e.preset = (int16_t)m.paramId;
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
        applyJunoParamToAmy(amySynth, m.paramId, m.val1);
        applyPianoParamToAmy(amySynth, m.paramId, m.val1);
        applySamplerParamToAmy(amySynth, m.paramId, m.val1);
        applyDx7ParamToAmy(amySynth, m.paramId, m.val1);
        switch (m.paramId) {
            case 5:  *(e.filter_freq_coefs + COEF_CONST) = m.val1; amy_add_event(&e); break;
            case 6:  e.resonance = m.val1; amy_add_event(&e); break;
            case 7:  *(e.filter_freq_coefs + COEF_EG0) = m.val1 * 4.0f; amy_add_event(&e); break;
            case 40: e.portamento_ms = (uint16_t)m.val1; amy_add_event(&e); break;
            case 45: *(e.amp_coefs + COEF_CONST) = m.val1; amy_add_event(&e); break;
            case 14: e.algorithm = (uint8_t)m.val1; amy_add_event(&e); break;
            case 15: e.feedback = m.val1 / 7.0f; amy_add_event(&e); break;
            case 46: { amy_event ge = amy_default_event(); ge.bus = 0; ge.echo_level = m.val1; amy_add_event(&ge); break; }
            case 47: { amy_event ge = amy_default_event(); ge.bus = 0; ge.echo_delay_ms = m.val1; amy_add_event(&ge); break; }
            case 48: { amy_event ge = amy_default_event(); ge.bus = 0; ge.echo_feedback = m.val1; amy_add_event(&ge); break; }
            case 148: { amy_event ge = amy_default_event(); ge.bus = 0; ge.echo_filter_coef = m.val1; amy_add_event(&ge); break; }
            case 149: { amy_event ge = amy_default_event(); ge.bus = 0; ge.reverb_level = m.val1; amy_add_event(&ge); break; }
            case 150: { amy_event ge = amy_default_event(); ge.bus = 0; ge.reverb_liveness = m.val1; amy_add_event(&ge); break; }
            case 151: { amy_event ge = amy_default_event(); ge.bus = 0; ge.reverb_damping = m.val1; amy_add_event(&ge); break; }
            case 152: { amy_event ge = amy_default_event(); ge.bus = 0; ge.reverb_xover_hz = m.val1; amy_add_event(&ge); break; }
            case 104: e.filter_type = FILTER_LPF; *(e.filter_freq_coefs + COEF_CONST) = m.val1; amy_add_event(&e); break;
            case 105: e.resonance = m.val1; amy_add_event(&e); break;
            case 106: *(e.filter_freq_coefs + COEF_NOTE) = m.val1; amy_add_event(&e); break;
            case 107: *(e.amp_coefs + COEF_VEL) = m.val1; amy_add_event(&e); break;
            case 111: *(e.eg0_times + 3) = (int32_t)m.val1; *(e.eg0_values + 3) = 0.0f; amy_add_event(&e); break;
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
    amy_start(cfg);
    delay(30);

    static float seqBpm = 120.0f, seqSwing = 0.0f;
    static uint32_t nextStepMs = 0;
    static int curStep = 0;
    static uint8_t *activeNotes = (uint8_t*)ps_calloc(16, 1);
    AmyBridgeMsg msg;
    for (;;) {
        while (amyCmdQueue && xQueueReceive(amyCmdQueue, &msg, 0) == pdTRUE) {
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
