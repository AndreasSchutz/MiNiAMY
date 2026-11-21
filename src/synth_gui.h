#pragma once
#include <Arduino.h>
#include <lvgl.h>
#include "dx7_algo_view.h"
#include "seq_view.h"
#include "amy_bridge.h"
#include "storage_mgr.h"
#include "help_manual.h"
#include "patch_names.h"

extern void updateSystemHardware(int systemParamId, float value);
extern float mapValue(float x, float in_min, float in_max, float out_min, float out_max);

enum MainMode { MODE_JUNO, MODE_DX7, MODE_DRUMS, MODE_SEQ, MODE_SAMPLER, MODE_PIANO, MODE_SYSTEM };
enum SubPage  { PAGE_OSC, PAGE_LFO, PAGE_FILTER, PAGE_ENV, PAGE_FX, PAGE_SYS_DISPLAY, PAGE_SYS_MIDI, PAGE_SYS_FILE, PAGE_DX7_OP_DETAIL };
enum ParamType { TYPE_AMY, TYPE_SYSTEM };

struct Parameter {
    ParamType pType;
    int internalId;
    const char* name;
    float minVal;
    float maxVal;
    float currentVal;
    uint8_t mappedCC;
    MainMode belongsToMode;
    SubPage belongsToPage;
    float defaultVal;
};

#define SYS_PARAM_BRIGHTNESS 0
#define SYS_PARAM_MASTER_VOL 1
#define SYS_PARAM_PART_SEL 2
#define SYS_PARAM_PART_ENGINE 3
#define SYS_PARAM_PART_CHAN 4
#define SYS_PARAM_PART_VOICES 5
#define SYS_PARAM_PART_PATCH 6
#define SYS_PARAM_PART_LVL 7
#define SYS_PARAM_DBLCLICK 8
#define SYS_PARAM_PROJ_SLOT 9
#define SYS_PARAM_PROJ_SAVE 10
#define SYS_PARAM_PROJ_LOAD 11
#define SYS_PARAM_FACTORY_RST 12
#define SYS_PARAM_FLIP 13
#define SYS_PARAM_DEMO_LOAD 14
#define SYS_PARAM_DIM_TIMEOUT 15
#define SYS_PARAM_PATCH_FILE 16
#define SYS_PARAM_LED_BRIGHT 17

#define BASE_PARAMS 150
#define OP_DETAIL_COUNT 16
#define PEG_DETAIL_COUNT 10
#define FX_DETAIL_COUNT 6
#define TOTAL_PARAMS (BASE_PARAMS + 6 * OP_DETAIL_COUNT + PEG_DETAIL_COUNT + FX_DETAIL_COUNT)

Parameter paramMatrix[TOTAL_PARAMS] = {
    // --- JUNO-106: DCO (6 st) ---
    {TYPE_AMY, 0,  "Saw",        0.0,   1.0,    1.0,    255, MODE_JUNO,   PAGE_OSC},
    {TYPE_AMY, 1,  "Pulse",      0.0,   1.0,    0.0,    255, MODE_JUNO,   PAGE_OSC},
    {TYPE_AMY, 2,  "PWM",        0.05,  0.95,   0.5,    255, MODE_JUNO,   PAGE_OSC},
    {TYPE_AMY, 35, "PWM Mode",   0.0,   1.0,    0.0,    255, MODE_JUNO,   PAGE_OSC},
    {TYPE_AMY, 3,  "Sub Osc",    0.0,   1.0,    0.3,    255, MODE_JUNO,   PAGE_OSC},
    {TYPE_AMY, 4,  "Noise",      0.0,   1.0,    0.0,    255, MODE_JUNO,   PAGE_OSC},
    // --- JUNO-106: LFO (5 st) ---
    {TYPE_AMY, 36, "LFO Rate",   0.1,   20.0,   4.5,    255, MODE_JUNO,   PAGE_LFO},
    {TYPE_AMY, 37, "LFO Delay",  0.0,   3000.0, 0.0,    255, MODE_JUNO,   PAGE_LFO},
    {TYPE_AMY, 38, "DCO LFO",    0.0,   1.0,    0.0,    255, MODE_JUNO,   PAGE_LFO},
    {TYPE_AMY, 39, "Octave",    -1.0,   1.0,    0.0,    255, MODE_JUNO,   PAGE_LFO},
    {TYPE_AMY, 40, "Portamento", 0.0,   1000.0, 0.0,    255, MODE_JUNO,   PAGE_LFO},
    {TYPE_AMY, 112, "LFO Wave",  0.0,   4.0,    0.0,    255, MODE_JUNO,   PAGE_LFO},
    // --- JUNO-106: VCF (6 st) ---
    {TYPE_AMY, 5,  "Cutoff",     20.0,  8000.0, 1200.0, 255, MODE_JUNO,   PAGE_FILTER},
    {TYPE_AMY, 6,  "Res",        0.5,   10.0,   1.5,    255, MODE_JUNO,   PAGE_FILTER},
    {TYPE_AMY, 7,  "Env Mod",    0.0,   1.0,    0.5,    255, MODE_JUNO,   PAGE_FILTER},
    {TYPE_AMY, 41, "Env Pol",    0.0,   1.0,    0.0,    255, MODE_JUNO,   PAGE_FILTER},
    {TYPE_AMY, 42, "LFO Mod",    0.0,   1.0,    0.0,    255, MODE_JUNO,   PAGE_FILTER},
    {TYPE_AMY, 43, "Key Follow", 0.0,   1.0,    0.5,    255, MODE_JUNO,   PAGE_FILTER},
    // --- JUNO-106: ENV (6 st) ---
    {TYPE_AMY, 9,  "Attack",     1.0,   3000.0, 20.0,   255, MODE_JUNO,   PAGE_ENV},
    {TYPE_AMY, 10, "Decay",      10.0,  5000.0, 400.0,  255, MODE_JUNO,   PAGE_ENV},
    {TYPE_AMY, 11, "Sustain",    0.0,   1.0,    0.7,    255, MODE_JUNO,   PAGE_ENV},
    {TYPE_AMY, 12, "Release",    10.0,  5000.0, 300.0,  255, MODE_JUNO,   PAGE_ENV},
    {TYPE_AMY, 44, "VCA Mode",   0.0,   1.0,    0.0,    255, MODE_JUNO,   PAGE_ENV},
    {TYPE_AMY, 45, "VCA Level",  0.0,   1.0,    0.85,   255, MODE_JUNO,   PAGE_ENV},
    // --- JUNO-106: FX (2 st) ---
    {TYPE_AMY, 8,  "HPF",        0.0,   3.0,    0.0,    255, MODE_JUNO,   PAGE_FX},
    {TYPE_AMY, 13, "Chorus",     0.0,   3.0,    1.0,    255, MODE_JUNO,   PAGE_FX},
    // --- SYSTEM FX (2 st huvudknappar + 2 dolda i modal) ---
    {TYPE_AMY, 46,  "Echo Mix",   0.0,   1.0,    0.0,    255, MODE_SYSTEM, PAGE_FX},
    {TYPE_AMY, 149, "Rev Mix",    0.0,   1.0,    0.0,    255, MODE_SYSTEM, PAGE_FX},
    {TYPE_AMY, 47,  "Echo Time",  10.0,  1000.0, 320.0,  255, MODE_SYSTEM, PAGE_DX7_OP_DETAIL},

    // --- DX7 FM ---
    {TYPE_AMY, 14, "Algorithm",  1.0,   32.0,   1.0,    255, MODE_DX7,    PAGE_OSC},
    {TYPE_AMY, 15, "Feedback",   0.0,   7.0,    3.0,    255, MODE_DX7,    PAGE_OSC},
    {TYPE_AMY, 16, "OP1 Level",  0.0,   99.0,   90.0,   255, MODE_DX7,    PAGE_FILTER},
    {TYPE_AMY, 17, "OP1 Ratio",  0.5,   16.0,   1.0,    255, MODE_DX7,    PAGE_FILTER},
    {TYPE_AMY, 18, "OP2 Level",  0.0,   99.0,   75.0,   255, MODE_DX7,    PAGE_FILTER},
    {TYPE_AMY, 19, "OP2 Ratio",  0.5,   16.0,   2.0,    255, MODE_DX7,    PAGE_FILTER},
    {TYPE_AMY, 27, "OP3 Level",  0.0,   99.0,   65.0,   255, MODE_DX7,    PAGE_FILTER},
    {TYPE_AMY, 28, "OP3 Ratio",  0.5,   16.0,   1.0,    255, MODE_DX7,    PAGE_FILTER},
    {TYPE_AMY, 29, "OP4 Level",  0.0,   99.0,   85.0,   255, MODE_DX7,    PAGE_ENV},
    {TYPE_AMY, 30, "OP4 Ratio",  0.5,   16.0,   1.0,    255, MODE_DX7,    PAGE_ENV},
    {TYPE_AMY, 31, "OP5 Level",  0.0,   99.0,   70.0,   255, MODE_DX7,    PAGE_ENV},
    {TYPE_AMY, 32, "OP5 Ratio",  0.5,   16.0,   3.0,    255, MODE_DX7,    PAGE_ENV},
    {TYPE_AMY, 33, "OP6 Level",  0.0,   99.0,   60.0,   255, MODE_DX7,    PAGE_ENV},
    {TYPE_AMY, 34, "OP6 Ratio",  0.5,   16.0,   4.0,    255, MODE_DX7,    PAGE_ENV},
    // --- DX7 GLOBAL (6 st) ---
    {TYPE_AMY, 49, "DX Octave", -2.0,   2.0,    0.0,    255, MODE_DX7,    PAGE_LFO},
    {TYPE_AMY, 50, "LFO Speed",  0.0,   99.0,   35.0,   255, MODE_DX7,    PAGE_LFO},
    {TYPE_AMY, 51, "LFO Delay",  0.0,   99.0,   0.0,    255, MODE_DX7,    PAGE_LFO},
    {TYPE_AMY, 52, "PMD Vib",    0.0,   99.0,   0.0,    255, MODE_DX7,    PAGE_LFO},
    {TYPE_AMY, 53, "AMD Trem",   0.0,   99.0,   0.0,    255, MODE_DX7,    PAGE_LFO},
    {TYPE_AMY, 54, "Pitch EG",   0.0,   99.0,   0.0,    255, MODE_DX7,    PAGE_LFO},

    // --- DRUMS: KIT (PCM Patch 0-66) ---
    {TYPE_AMY, 20,  "Drum1", 0.0,   154.0,   1.0,    255, MODE_DRUMS, PAGE_OSC},
    {TYPE_AMY, 60,  "Drum2", 0.0,   154.0,   5.0,    255, MODE_DRUMS, PAGE_OSC},
    {TYPE_AMY, 61,  "Drum3", 0.0,   154.0,   14.0,   255, MODE_DRUMS, PAGE_OSC},
    {TYPE_AMY, 62,  "Drum4", 0.0,   154.0,   15.0,   255, MODE_DRUMS, PAGE_OSC},
    {TYPE_AMY, 63,  "Drum5", 0.0,   154.0,   10.0,   255, MODE_DRUMS, PAGE_OSC},
    {TYPE_AMY, 64,  "Drum6", 0.0,   154.0,   22.0,   255, MODE_DRUMS, PAGE_OSC},
    {TYPE_AMY, 89,  "Drum7", 0.0,   154.0,   25.0,   255, MODE_DRUMS, PAGE_OSC},
    {TYPE_AMY, 90,  "Drum8", 0.0,   154.0,   30.0,   255, MODE_DRUMS, PAGE_OSC},
    {TYPE_AMY, 91,  "Drum9", 0.0,   154.0,   35.0,   255, MODE_DRUMS, PAGE_OSC},
    // --- DRUMS: TUNE ---
    {TYPE_AMY, 21,  "Drum1", 0.5,   2.0,    1.0,    255, MODE_DRUMS, PAGE_LFO},
    {TYPE_AMY, 22,  "Drum2", 0.5,   2.0,    1.0,    255, MODE_DRUMS, PAGE_LFO},
    {TYPE_AMY, 65,  "Drum3", 0.5,   2.0,    1.0,    255, MODE_DRUMS, PAGE_LFO},
    {TYPE_AMY, 66,  "Drum4", 0.5,   2.0,    1.0,    255, MODE_DRUMS, PAGE_LFO},
    {TYPE_AMY, 67,  "Drum5", 0.5,   2.0,    1.0,    255, MODE_DRUMS, PAGE_LFO},
    {TYPE_AMY, 68,  "Drum6", 0.5,   2.0,    1.0,    255, MODE_DRUMS, PAGE_LFO},
    {TYPE_AMY, 92,  "Drum7", 0.5,   2.0,    1.0,    255, MODE_DRUMS, PAGE_LFO},
    {TYPE_AMY, 93,  "Drum8", 0.5,   2.0,    1.0,    255, MODE_DRUMS, PAGE_LFO},
    {TYPE_AMY, 94,  "Drum9", 0.5,   2.0,    1.0,    255, MODE_DRUMS, PAGE_LFO},
    // --- DRUMS: MIX ---
    {TYPE_AMY, 23,  "Drum1", 0.0,   1.0,    0.9,    255, MODE_DRUMS, PAGE_FILTER},
    {TYPE_AMY, 24,  "Drum2", 0.0,   1.0,    0.85,   255, MODE_DRUMS, PAGE_FILTER},
    {TYPE_AMY, 25,  "Drum3", 0.0,   1.0,    0.75,   255, MODE_DRUMS, PAGE_FILTER},
    {TYPE_AMY, 69,  "Drum4", 0.0,   1.0,    0.70,   255, MODE_DRUMS, PAGE_FILTER},
    {TYPE_AMY, 70,  "Drum5", 0.0,   1.0,    0.80,   255, MODE_DRUMS, PAGE_FILTER},
    {TYPE_AMY, 71,  "Drum6", 0.0,   1.0,    0.75,   255, MODE_DRUMS, PAGE_FILTER},
    {TYPE_AMY, 95,  "Drum7", 0.0,   1.0,    0.75,   255, MODE_DRUMS, PAGE_FILTER},
    {TYPE_AMY, 96,  "Drum8", 0.0,   1.0,    0.75,   255, MODE_DRUMS, PAGE_FILTER},
    {TYPE_AMY, 97,  "Drum9", 0.0,   1.0,    0.75,   255, MODE_DRUMS, PAGE_FILTER},
    // --- DRUMS: PAN ---
    {TYPE_AMY, 72,  "Drum1", 0.0,   1.0,    0.5,    255, MODE_DRUMS, PAGE_ENV},
    {TYPE_AMY, 73,  "Drum2", 0.0,   1.0,    0.5,    255, MODE_DRUMS, PAGE_ENV},
    {TYPE_AMY, 74,  "Drum3", 0.0,   1.0,    0.65,   255, MODE_DRUMS, PAGE_ENV},
    {TYPE_AMY, 75,  "Drum4", 0.0,   1.0,    0.35,   255, MODE_DRUMS, PAGE_ENV},
    {TYPE_AMY, 76,  "Drum5", 0.0,   1.0,    0.45,   255, MODE_DRUMS, PAGE_ENV},
    {TYPE_AMY, 77,  "Drum6", 0.0,   1.0,    0.60,   255, MODE_DRUMS, PAGE_ENV},
    {TYPE_AMY, 98,  "Drum7", 0.0,   1.0,    0.40,   255, MODE_DRUMS, PAGE_ENV},
    {TYPE_AMY, 99,  "Drum8", 0.0,   1.0,    0.60,   255, MODE_DRUMS, PAGE_ENV},
    {TYPE_AMY, 100, "Drum9", 0.0,   1.0,    0.50,   255, MODE_DRUMS, PAGE_ENV},
    // --- DRUMS: FLT ---
    {TYPE_AMY, 78,  "Drum1", 100.0, 8000.0, 8000.0, 255, MODE_DRUMS, PAGE_FX},
    {TYPE_AMY, 79,  "Drum2", 100.0, 8000.0, 8000.0, 255, MODE_DRUMS, PAGE_FX},
    {TYPE_AMY, 80,  "Drum3", 100.0, 8000.0, 8000.0, 255, MODE_DRUMS, PAGE_FX},
    {TYPE_AMY, 81,  "Drum4", 100.0, 8000.0, 8000.0, 255, MODE_DRUMS, PAGE_FX},
    {TYPE_AMY, 82,  "Drum5", 100.0, 8000.0, 8000.0, 255, MODE_DRUMS, PAGE_FX},
    {TYPE_AMY, 83,  "Drum6", 100.0, 8000.0, 8000.0, 255, MODE_DRUMS, PAGE_FX},
    {TYPE_AMY, 101, "Drum7", 100.0, 8000.0, 8000.0, 255, MODE_DRUMS, PAGE_FX},
    {TYPE_AMY, 102, "Drum8", 100.0, 8000.0, 8000.0, 255, MODE_DRUMS, PAGE_FX},
    {TYPE_AMY, 103, "Drum9", 100.0, 8000.0, 8000.0, 255, MODE_DRUMS, PAGE_FX},

    // --- SEQUENCER: PLAY (5 st) ---
    {TYPE_AMY, 84, "Seq Play",   0.0,   1.0,    0.0,    255, MODE_SEQ,    PAGE_DX7_OP_DETAIL},
    {TYPE_AMY, 85, "BPM",        40.0,  240.0,  120.0,  255, MODE_SEQ,    PAGE_OSC},
    {TYPE_AMY, 86, "Swing",      0.0,   75.0,   0.0,    255, MODE_SEQ,    PAGE_OSC},
    {TYPE_AMY, 88, "Steps",      1.0,   16.0,   16.0,   255, MODE_SEQ,    PAGE_OSC},
    {TYPE_AMY, 147, "",           0.0,   1.0,    0.0,    255, MODE_SEQ,    PAGE_OSC},
    {TYPE_AMY, 87, "Act Pat",    1.0,   16.0,   1.0,    255, MODE_SEQ,    PAGE_OSC},
    {TYPE_AMY, 146, "Max Pat",    1.0,   16.0,   16.0,   255, MODE_SEQ,    PAGE_OSC},

    // --- PIANO (17 st: TONE, TIMBRE, MIX, ENV) ---
    {TYPE_AMY, 104, "Brightness", 200.0, 8000.0, 2800.0, 255, MODE_PIANO, PAGE_OSC},
    {TYPE_AMY, 105, "Resonance",  0.5,   6.0,    0.8,    255, MODE_PIANO, PAGE_OSC},
    {TYPE_AMY, 106, "Key Follow", 0.0,   1.0,    0.6,    255, MODE_PIANO, PAGE_OSC},
    {TYPE_AMY, 107, "Vel Sens",   0.0,   1.5,    0.9,    255, MODE_PIANO, PAGE_OSC},
    {TYPE_AMY, 113, "Vel Filter", 0.0,   1.0,    0.6,    255, MODE_PIANO, PAGE_OSC},
    {TYPE_AMY, 114, "Stereo Wd",  0.0,   1.0,    0.4,    255, MODE_PIANO, PAGE_OSC},
    {TYPE_AMY, 115, "Body Wave",  0.0,   3.0,    0.0,    255, MODE_PIANO, PAGE_LFO},
    {TYPE_AMY, 116, "Harm Wave",  0.0,   3.0,    0.0,    255, MODE_PIANO, PAGE_LFO},
    {TYPE_AMY, 117, "Hamm Wave",  0.0,   3.0,    2.0,    255, MODE_PIANO, PAGE_LFO},
    {TYPE_AMY, 118, "Detune Spr", 0.0,   1.0,    0.25,   255, MODE_PIANO, PAGE_LFO},
    {TYPE_AMY, 119, "Fundamentl", 0.0,   1.0,    0.9,    255, MODE_PIANO, PAGE_FILTER},
    {TYPE_AMY, 120, "Overtones",  0.0,   1.0,    0.6,    255, MODE_PIANO, PAGE_FILTER},
    {TYPE_AMY, 121, "Hammer Clk", 0.0,   1.0,    0.45,   255, MODE_PIANO, PAGE_FILTER},
    {TYPE_AMY, 108, "Attack",     1.0,   500.0,  5.0,    255, MODE_PIANO, PAGE_ENV},
    {TYPE_AMY, 109, "Decay",      50.0,  4000.0, 1200.0, 255, MODE_PIANO, PAGE_ENV},
    {TYPE_AMY, 110, "Sustain",    0.0,   1.0,    0.3,    255, MODE_PIANO, PAGE_ENV},
    {TYPE_AMY, 111, "Release",    20.0,  4000.0, 450.0,  255, MODE_PIANO, PAGE_ENV},

    // --- SAMPLER (16 st: PLAY, FLT, ENV) ---
    {TYPE_AMY, 130, "Sample",     0.0,   154.0,   0.0,    255, MODE_SAMPLER, PAGE_OSC},
    {TYPE_AMY, 131, "Tune",      -24.0,  24.0,   0.0,    255, MODE_SAMPLER, PAGE_OSC},
    {TYPE_AMY, 132, "Fine Tune", -50.0,  50.0,   0.0,    255, MODE_SAMPLER, PAGE_OSC},
    {TYPE_AMY, 133, "Loop",       0.0,   1.0,    0.0,    255, MODE_SAMPLER, PAGE_OSC},
    {TYPE_AMY, 134, "Pan",        0.0,   1.0,    0.5,    255, MODE_SAMPLER, PAGE_OSC},
    {TYPE_AMY, 136, "Flt Type",   0.0,   3.0,    1.0,    255, MODE_SAMPLER, PAGE_FILTER},
    {TYPE_AMY, 137, "Cutoff",     40.0,  8000.0, 4500.0, 255, MODE_SAMPLER, PAGE_FILTER},
    {TYPE_AMY, 138, "Resonance",  0.5,   10.0,   1.0,    255, MODE_SAMPLER, PAGE_FILTER},
    {TYPE_AMY, 139, "Key Follow", 0.0,   1.0,    0.5,    255, MODE_SAMPLER, PAGE_FILTER},
    {TYPE_AMY, 140, "Env Mod",   -1.0,   1.0,    0.0,    255, MODE_SAMPLER, PAGE_FILTER},
    {TYPE_AMY, 141, "Attack",     1.0,   3000.0, 5.0,    255, MODE_SAMPLER, PAGE_ENV},
    {TYPE_AMY, 142, "Decay",      10.0,  5000.0, 500.0,  255, MODE_SAMPLER, PAGE_ENV},
    {TYPE_AMY, 143, "Sustain",    0.0,   1.0,    0.8,    255, MODE_SAMPLER, PAGE_ENV},
    {TYPE_AMY, 144, "Release",    10.0,  5000.0, 250.0,  255, MODE_SAMPLER, PAGE_ENV},
    {TYPE_AMY, 145, "Vel Sens",   0.0,   1.5,    1.0,    255, MODE_SAMPLER, PAGE_ENV},

    // --- SYSTEM ---
    {TYPE_SYSTEM, SYS_PARAM_BRIGHTNESS, "Brightness", 25.0, 100.0, 85.0, 255, MODE_SYSTEM, PAGE_SYS_DISPLAY},
    {TYPE_SYSTEM, SYS_PARAM_MASTER_VOL, "Master Vol", 0.0,  10.0,  8.0,  255, MODE_SYSTEM, PAGE_FX},
    {TYPE_SYSTEM, SYS_PARAM_DBLCLICK,   "DblClick",   0.0,  2.0,   1.0,  255, MODE_SYSTEM, PAGE_SYS_DISPLAY},
    {TYPE_SYSTEM, SYS_PARAM_FLIP,       "Flip",       0.0,  1.0,   0.0,  255, MODE_SYSTEM, PAGE_SYS_DISPLAY},
    {TYPE_SYSTEM, SYS_PARAM_DIM_TIMEOUT,"Screen Dim", 0.0,  10.0,  3.0,  255, MODE_SYSTEM, PAGE_SYS_DISPLAY},
    {TYPE_SYSTEM, SYS_PARAM_LED_BRIGHT, "LED Bright", 0.0,  20.0,  0.0,  255, MODE_SYSTEM, PAGE_SYS_DISPLAY},
    {TYPE_SYSTEM, SYS_PARAM_PART_SEL, "Synth", 1.0, 16.0, 1.0, 255, MODE_SYSTEM, PAGE_SYS_MIDI},
{TYPE_SYSTEM, SYS_PARAM_PART_ENGINE, "Engine", 0.0, 5.0, 1.0, 255, MODE_SYSTEM, PAGE_SYS_MIDI},
{TYPE_SYSTEM, SYS_PARAM_PART_CHAN, "MIDI Chan", 0.0, 16.0, 1.0, 255, MODE_SYSTEM, PAGE_SYS_MIDI},
{TYPE_SYSTEM, SYS_PARAM_PART_PATCH, "Patch/Note", 0.0, 255.0, 0.0, 255, MODE_SYSTEM, PAGE_SYS_MIDI},
{TYPE_SYSTEM, SYS_PARAM_PART_VOICES, "Voices", 0.0, 9.0, 4.0, 255, MODE_SYSTEM, PAGE_SYS_MIDI},
{TYPE_SYSTEM, SYS_PARAM_PART_LVL, "Synth Vol", 0.0, 10.0, 8.0, 255, MODE_SYSTEM, PAGE_SYS_MIDI},
    {TYPE_SYSTEM, SYS_PARAM_PROJ_SLOT, "Project File", 1.0, 16.0, 1.0, 255, MODE_SYSTEM, PAGE_SYS_FILE},
    {TYPE_SYSTEM, SYS_PARAM_PATCH_FILE,  "Patch File",  1.0, 16.0, 1.0, 255, MODE_SYSTEM, PAGE_SYS_FILE},
    {TYPE_SYSTEM, SYS_PARAM_DEMO_LOAD, "Demo Presets", 0.0, 1.0, 0.0, 255, MODE_SYSTEM, PAGE_SYS_FILE},
    {TYPE_SYSTEM, SYS_PARAM_FACTORY_RST, "Factory Rst", 0.0, 1.0, 0.0, 255, MODE_SYSTEM, PAGE_SYS_FILE}
};

static lv_obj_t *paramBoxes[TOTAL_PARAMS] = {nullptr};
static lv_obj_t *paramBars[TOTAL_PARAMS] = {nullptr};
static lv_obj_t *paramLabels[TOTAL_PARAMS] = {nullptr};
static int parameterUnderLearning = -1;
static bool isDraggingFader = false;
static lv_coord_t dragStartX = 0;
static lv_coord_t dragStartY = 0;

static lv_obj_t *opModal = nullptr;
static lv_obj_t *opModalTitle = nullptr;
static lv_obj_t *opModalBoxes[OP_DETAIL_COUNT] = {nullptr};
static lv_obj_t *opModalLabels[OP_DETAIL_COUNT] = {nullptr};
static int activeOpDetail = -1;

inline void initDx7OperatorParams() {
    for (int i = 0; i < TOTAL_PARAMS; i++) {
        paramMatrix[i].defaultVal = paramMatrix[i].currentVal;
    }
    static const char* names[OP_DETAIL_COUNT] = {
        "Level", "Coarse", "Fine", "Detune", "Fixed", "Vel Sens",
        "EG Rate 1", "EG Rate 2", "EG Rate 3", "EG Rate 4",
        "EG Lvl 1", "EG Lvl 2", "EG Lvl 3", "EG Lvl 4", "Amp Mod", "Key Scale"
    };
    static const float mins[OP_DETAIL_COUNT] = {0, 0, 0, -7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static const float maxs[OP_DETAIL_COUNT] = {99, 31, 99, 7, 1, 7, 99, 99, 99, 99, 99, 99, 99, 99, 3, 7};
    static const float defs[OP_DETAIL_COUNT] = {85, 1, 0, 0, 0, 2, 95, 70, 40, 50, 99, 80, 60, 0, 0, 0};
    for (int op = 0; op < 6; op++) {
        for (int k = 0; k < OP_DETAIL_COUNT; k++) {
            int idx = BASE_PARAMS + op * OP_DETAIL_COUNT + k;
            paramMatrix[idx] = {TYPE_AMY, 300 + op * 20 + k, names[k], mins[k], maxs[k], defs[k], 255, MODE_DX7, PAGE_DX7_OP_DETAIL};
        }
    }
    static const char* pNames[PEG_DETAIL_COUNT] = {
        "P Rate 1", "P Rate 2", "P Rate 3", "P Rate 4",
        "P Lvl 1",  "P Lvl 2",  "P Lvl 3",  "P Lvl 4",
        "LFO Wave", "LFO Sync"
    };
    static const float pMins[PEG_DETAIL_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static const float pMaxs[PEG_DETAIL_COUNT] = {99, 99, 99, 99, 99, 99, 99, 99, 5, 1};
    static const float pDefs[PEG_DETAIL_COUNT] = {99, 99, 99, 99, 50, 50, 50, 50, 0, 1};
    int pegBase = BASE_PARAMS + 6 * OP_DETAIL_COUNT;
    for (int k = 0; k < PEG_DETAIL_COUNT; k++) {
        paramMatrix[pegBase + k] = {TYPE_AMY, 420 + k, pNames[k], pMins[k], pMaxs[k], pDefs[k], 255, MODE_DX7, PAGE_DX7_OP_DETAIL};
    }
    int fxBase = pegBase + PEG_DETAIL_COUNT;
    paramMatrix[fxBase + 0] = {TYPE_AMY, 47,  "Echo Time", 10.0,  1000.0, 320.0,  255, MODE_SYSTEM, PAGE_DX7_OP_DETAIL};
    paramMatrix[fxBase + 1] = {TYPE_AMY, 48,  "Echo Fdbk", 0.0,   0.9,    0.4,    255, MODE_SYSTEM, PAGE_DX7_OP_DETAIL};
    paramMatrix[fxBase + 2] = {TYPE_AMY, 148, "Echo Tone", 0.0,   0.95,   0.5,    255, MODE_SYSTEM, PAGE_DX7_OP_DETAIL};
    paramMatrix[fxBase + 3] = {TYPE_AMY, 150, "Rev Size",  0.1,   0.98,   0.85,   255, MODE_SYSTEM, PAGE_DX7_OP_DETAIL};
    paramMatrix[fxBase + 4] = {TYPE_AMY, 151, "Rev Damp",  0.0,   1.0,    0.5,    255, MODE_SYSTEM, PAGE_DX7_OP_DETAIL};
    paramMatrix[fxBase + 5] = {TYPE_AMY, 152, "Rev Xover", 200.0, 8000.0, 3000.0, 255, MODE_SYSTEM, PAGE_DX7_OP_DETAIL};
}

inline void refreshParamBox(int idx);
enum EngineType { ENG_OFF = 0, ENG_JUNO = 1, ENG_DX7 = 2, ENG_SAMPLER = 3, ENG_DRUMS = 4, ENG_PIANO = 5 };
struct SynthPartConfig {
  uint8_t engine;
  uint8_t midiChan;
  uint8_t voices;
  uint8_t patchOrNote;
  float level;
};
static SynthPartConfig synthParts[16] = {
  {0,0,0,0,0.8f},{0,0,0,0,0.8f},{0,0,0,0,0.8f},{0,0,0,0,0.8f},
  {0,0,0,0,0.8f},{0,0,0,0,0.8f},{0,0,0,0,0.8f},{0,0,0,0,0.8f},
  {0,0,0,0,0.8f},{0,0,0,0,0.8f},{0,0,0,0,0.8f},{0,0,0,0,0.8f},
  {0,0,0,0,0.8f},{0,0,0,0,0.8f},{0,0,0,0,0.8f},{0,0,0,0,0.8f}
};
static int activePartIdx = 0;
static float *synthParamRam = nullptr;

inline bool isGlobalParam(const Parameter &p) {
    if (p.pType == TYPE_SYSTEM) return true;
    if (p.belongsToMode == MODE_SEQ) return true;
    return ((p.internalId >= 46 && p.internalId <= 48) || (p.internalId >= 148 && p.internalId <= 152));
}

inline void initPerSynthRam() {
    if (!synthParamRam) {
        synthParamRam = (float*)ps_malloc(16 * TOTAL_PARAMS * sizeof(float));
    }
    if (!synthParamRam) return;
    for (int s = 0; s < 16; s++) {
        for (int i = 0; i < TOTAL_PARAMS; i++) {
            *(synthParamRam + s * TOTAL_PARAMS + i) = paramMatrix[i].currentVal;
        }
    }
}

inline void saveActivePartParamsToRam() {
    if (!synthParamRam) return;
    float *dst = synthParamRam + activePartIdx * TOTAL_PARAMS;
    for (int i = 0; i < TOTAL_PARAMS; i++) {
        *(dst + i) = paramMatrix[i].currentVal;
    }
}

inline bool paramMatchesEngine(MainMode m, uint8_t eng) {
    if (eng == ENG_JUNO) return m == MODE_JUNO;
    if (eng == ENG_DX7) return m == MODE_DX7;
    if (eng == ENG_SAMPLER) return m == MODE_SAMPLER;
    if (eng == ENG_DRUMS) return m == MODE_DRUMS;
    if (eng == ENG_PIANO) return m == MODE_PIANO;
    return false;
}

inline void syncAllPartParamsToAmy(int pIdx) {
    uint8_t eng = synthParts[pIdx].engine;
    if (eng == ENG_OFF || synthParts[pIdx].voices == 0) return;
    for (int i = 0; i < TOTAL_PARAMS; i++) {
        if (paramMatrix[i].pType == TYPE_AMY && paramMatchesEngine(paramMatrix[i].belongsToMode, eng)) {
            amyEnqueueParam((uint8_t)pIdx, paramMatrix[i].internalId, paramMatrix[i].currentVal);
        }
    }
}

inline int findMatrixIdxBySysId(int sysId) {
  for (int i = 0; i < BASE_PARAMS; i++) {
    if (paramMatrix[i].pType == TYPE_SYSTEM && paramMatrix[i].internalId == sysId) return i;
  }
  return -1;
}

static lv_obj_t *g_mainTv = nullptr;
static lv_obj_t *g_synthEditFrame = nullptr;
static lv_obj_t *g_mainBtns = nullptr;
static lv_obj_t *g_junoTv = nullptr;
static lv_obj_t *g_dx7Tv = nullptr;
static lv_obj_t *g_drumsTv = nullptr;
static lv_obj_t *g_sampTv = nullptr;
static lv_obj_t *g_pianoTv = nullptr;
static lv_obj_t *g_logoPanel = nullptr;
static lv_obj_t *g_sysTv = nullptr;
static lv_obj_t *g_seqTv = nullptr;
static lv_obj_t *g_oscBar = nullptr;
static lv_obj_t *g_oscLbl = nullptr;

inline int getOscCostPerVoice(uint8_t eng) {
    if (eng == ENG_JUNO) return 5;
    if (eng == ENG_DX7) return 6;
    if (eng == ENG_SAMPLER) return 1;
    if (eng == ENG_DRUMS) return 1;
    if (eng == ENG_PIANO) return 11;
    return 0;
}

inline int calcTotalUsedOscs(int excludeSlot = -1) {
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        if (i == excludeSlot) continue;
        sum += synthParts[i].voices * getOscCostPerVoice(synthParts[i].engine);
    }
    return sum;
}

inline void updateOscBudgetMeter() {
    if (!g_oscBar || !g_oscLbl) return;
    int used = calcTotalUsedOscs();
    lv_bar_set_value(g_oscBar, constrain(used, 0, 120), LV_ANIM_OFF);
    lv_color_t col = lv_color_hex(0x469632);
    if (used >= 108) col = lv_color_hex(0xEB150F);
    else if (used >= 90) col = lv_color_hex(0xF85D00);
    lv_obj_set_style_bg_color(g_oscBar, col, LV_PART_INDICATOR);
    char *b = (char*)alloca(24);
    snprintf(b, 24, "OSC\n%d/120", used);
    lv_label_set_text(g_oscLbl, b);
}

inline void updateEngineTabLocks() {
    if (!g_mainTv || !g_mainBtns) return;
    uint8_t eng = synthParts[activePartIdx].engine;
    const char *lbl = "Off";
    if (eng == ENG_JUNO) lbl = "Juno";
    else if (eng == ENG_DX7) lbl = "DX7";
    else if (eng == ENG_DRUMS) lbl = "Drumkit";
    else if (eng == ENG_SAMPLER) lbl = "PCM";
    else if (eng == ENG_PIANO) lbl = "Piano";
    lv_tabview_rename_tab(g_mainTv, 0, lbl);

    if (eng == ENG_OFF) lv_btnmatrix_set_btn_ctrl(g_mainBtns, 0, LV_BTNMATRIX_CTRL_DISABLED);
    else lv_btnmatrix_clear_btn_ctrl(g_mainBtns, 0, LV_BTNMATRIX_CTRL_DISABLED);

    if (g_junoTv) { if (eng == ENG_JUNO) lv_obj_clear_flag(g_junoTv, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(g_junoTv, LV_OBJ_FLAG_HIDDEN); }
    if (g_dx7Tv)  { if (eng == ENG_DX7)  lv_obj_clear_flag(g_dx7Tv, LV_OBJ_FLAG_HIDDEN);  else lv_obj_add_flag(g_dx7Tv, LV_OBJ_FLAG_HIDDEN); }
    if (g_drumsTv){ if (eng == ENG_DRUMS)lv_obj_clear_flag(g_drumsTv, LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(g_drumsTv, LV_OBJ_FLAG_HIDDEN); }
    if (g_sampTv) { if (eng == ENG_SAMPLER)lv_obj_clear_flag(g_sampTv, LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(g_sampTv, LV_OBJ_FLAG_HIDDEN); }
    if (g_pianoTv){ if (eng == ENG_PIANO)  lv_obj_clear_flag(g_pianoTv, LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(g_pianoTv, LV_OBJ_FLAG_HIDDEN); }
    if (g_logoPanel){ if (eng == ENG_OFF) lv_obj_clear_flag(g_logoPanel, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(g_logoPanel, LV_OBJ_FLAG_HIDDEN); }
    updateOscBudgetMeter();
}

inline void loadPartIntoMatrix(int pIdx) {
  int nextIdx = constrain(pIdx, 0, 15);
  if (synthParamRam) {
    if (nextIdx != activePartIdx) saveActivePartParamsToRam();
    float *src = synthParamRam + nextIdx * TOTAL_PARAMS;
    for (int i = 0; i < TOTAL_PARAMS; i++) {
      if (!isGlobalParam(paramMatrix[i]) && paramMatrix[i].currentVal != *(src + i)) {
        paramMatrix[i].currentVal = *(src + i);
        refreshParamBox(i);
      }
    }
  }
  activePartIdx = nextIdx;
  syncAllPartParamsToAmy(activePartIdx);
  SynthPartConfig &sp = synthParts[activePartIdx];
  int ids[] = {SYS_PARAM_PART_ENGINE, SYS_PARAM_PART_CHAN, SYS_PARAM_PART_VOICES, SYS_PARAM_PART_PATCH, SYS_PARAM_PART_LVL};
  float vals[] = {(float)sp.engine, (float)sp.midiChan, (float)sp.voices, (float)sp.patchOrNote, roundf(sp.level * 10.0f)};
  for (int k = 0; k < 5; k++) {
    int m = findMatrixIdxBySysId(ids[k]);
    if (m >= 0) { paramMatrix[m].currentVal = vals[k]; refreshParamBox(m); }
  }
  updateEngineTabLocks();
  if (g_synthEditFrame) {
    if (sp.engine == ENG_OFF || sp.engine == ENG_DRUMS) lv_obj_add_flag(g_synthEditFrame, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_clear_flag(g_synthEditFrame, LV_OBJ_FLAG_HIDDEN);
  }
  int mPf = findMatrixIdxBySysId(SYS_PARAM_PATCH_FILE);
  if (mPf >= 0) { paramMatrix[mPf].currentVal = (float)(activePartIdx + 1); refreshParamBox(mPf); }
}

inline void triggerDrumOnSlot(int drumSlot, int drumIdx, float velScale = 1.0f) {
  if (drumIdx < 0 || drumIdx >= 9) return;
  if (drumSlot < 0 || drumSlot >= 16) return;
  static const int kitIds[9] = {20, 60, 61, 62, 63, 64, 89, 90, 91};
  static const int tuneIds[9] = {21, 22, 65, 66, 67, 68, 92, 93, 94};
  static const int mixIds[9] = {23, 24, 25, 69, 70, 71, 95, 96, 97};
  static const int panIds[9] = {72, 73, 74, 75, 76, 77, 98, 99, 100};
  static const int fltIds[9] = {78, 79, 80, 81, 82, 83, 101, 102, 103};
  auto getAmyVal = [](int aid) -> float {
    for (int i = 0; i < BASE_PARAMS; i++) if (paramMatrix[i].pType == TYPE_AMY && paramMatrix[i].internalId == aid) return paramMatrix[i].currentVal;
    return 0.0f;
  };
  int pcm = (int)getAmyVal(kitIds[drumIdx]);
  float tune = getAmyVal(tuneIds[drumIdx]);
  float vel = getAmyVal(mixIds[drumIdx]) * synthParts[drumSlot].level * velScale;
  float pan = getAmyVal(panIds[drumIdx]);
  float flt = getAmyVal(fltIds[drumIdx]);
  amyEnqueueDrum((uint8_t)((drumSlot % 2) * 9 + drumIdx), pcm, tune, vel, pan, flt);
  }
inline void triggerDrum(int drumIdx, float velScale = 1.0f) {
  int drumSlot = (synthParts[activePartIdx].engine == ENG_DRUMS) ? activePartIdx : 9;
  triggerDrumOnSlot(drumSlot, drumIdx, velScale);
}

static int fileFeedbackId = -1;
static uint32_t fileFeedbackUntil = 0;
static const char* fileFeedbackTxt = "";
inline bool isSystemActionParam(const Parameter &p) {
    return (p.pType == TYPE_SYSTEM && (p.internalId == SYS_PARAM_PROJ_SAVE || p.internalId == SYS_PARAM_PROJ_LOAD || p.internalId == SYS_PARAM_DEMO_LOAD || p.internalId == SYS_PARAM_FACTORY_RST || p.internalId == SYS_PARAM_PATCH_FILE));
}

inline bool isIntegerParam(const Parameter &p) {
    if (p.belongsToPage == PAGE_DX7_OP_DETAIL) return true;
    if (p.pType == TYPE_SYSTEM) {
        return true;
    }
    int id = p.internalId;
    return (id == 0 || id == 1 || id == 5 || id == 8 || id == 9 || id == 10 || id == 12 ||
            id == 13 || id == 14 || id == 15 || id == 16 || id == 18 || id == 20 || (id >= 60 && id <= 64) || (id >= 89 && id <= 91) || (id >= 101 && id <= 103) || (id >= 78 && id <= 88) || id == 26 ||
            id == 27 || id == 29 || id == 31 || id == 33 || id == 35 || id == 37 || id == 39 ||
            id == 40 || id == 41 || id == 112 || (id >= 115 && id <= 117) || (id >= 130 && id <= 133) || id == 136 || id == 137 || id == 141 || id == 142 || id == 144 || id == 146 || id == 44 || id == 47 || id == 152 || (id >= 49 && id <= 54) || id == 104 || id == 108 || id == 109 || id == 111);
}

inline bool isFxModalTrigger(const Parameter &p) {
    return (p.pType == TYPE_AMY && p.belongsToMode == MODE_SYSTEM && (p.internalId == 46 || p.internalId == 149));
}

inline bool isOpMenuTrigger(const Parameter &p) {
    if (p.pType != TYPE_AMY || p.belongsToPage == PAGE_DX7_OP_DETAIL) return false;
    int id = p.internalId;
    return (id == 16 || id == 18 || id == 27 || id == 29 || id == 31 || id == 33 || id == 54);
}

inline void formatParamDisplay(const Parameter &p, char *outName, char *outVal) {
  static const char* engNames[] = {"OFF", "JUNO", "DX7", "PCM", "DRUMS", "PIANO"};
  if (!p.name) { *outName = 0; *outVal = 0; return; }
  snprintf(outName, 24, "%s", p.name);
  if (p.pType == TYPE_SYSTEM) {
    if (p.internalId == SYS_PARAM_PART_SEL) {
      int idx = constrain((int)p.currentVal - 1, 0, 15);
      uint8_t eng = constrain(synthParts[idx].engine, 0, 5);
      snprintf(outName, 24, "Synth %d (%s)", idx + 1, engNames[eng]);
      char *customName = ((char*)synthPatchNames) + idx * 16;
      if (*customName != 0) {
        snprintf(outVal, 24, "%s", customName);
      } else {
        const char *pName = getEnginePatchName(eng, synthParts[idx].patchOrNote);
        snprintf(outVal, 24, "%s", pName ? pName : "-");
      }
      return;
    } else if (p.internalId == SYS_PARAM_PART_ENGINE) {
      snprintf(outVal, 24, "%s", engNames[constrain((int)p.currentVal, 0, 5)]);
      return;
    } else if (p.internalId == SYS_PARAM_DBLCLICK) {
      static const int ms[] = {380, 300, 220};
      snprintf(outVal, 24, "%d ms", ms[constrain((int)p.currentVal, 0, 2)]);
      return;
    } else if (p.internalId == SYS_PARAM_FLIP) {
      snprintf(outVal, 24, "%s", (p.currentVal >= 0.5f) ? "180" : "OFF");
      return;
    } else if (p.internalId == SYS_PARAM_DIM_TIMEOUT) {
      int m = (int)p.currentVal;
      if (m <= 0) snprintf(outVal, 24, "OFF");
      else if (m == 1) snprintf(outVal, 24, "30 sec");
      else snprintf(outVal, 24, "%d min", m - 1);
      return;
    } else if (p.internalId == SYS_PARAM_PROJ_SLOT) { if (fileFeedbackId == p.internalId && millis() < fileFeedbackUntil) snprintf(outVal, 24, "%s", fileFeedbackTxt); else snprintf(outVal, 24, "Proj %02d", constrain((int)p.currentVal, 1, 16)); return; } else if (isSystemActionParam(p)) { if (fileFeedbackId == p.internalId && millis() < fileFeedbackUntil) snprintf(outVal, 24, "%s", fileFeedbackTxt); else if (p.internalId == SYS_PARAM_FACTORY_RST) snprintf(outVal, 24, "HOLD 800ms"); else if (p.internalId == SYS_PARAM_DEMO_LOAD) snprintf(outVal, 24, "FLASH"); else if (p.internalId == SYS_PARAM_PATCH_FILE) snprintf(outVal, 24, "Synth %d", activePartIdx + 1); else snprintf(outVal, 24, "%s", usingPhysicalSD ? "SD CARD" : "NVS / FLASH"); return; } else if (p.internalId == SYS_PARAM_PART_CHAN) {
      int ch = (int)p.currentVal;
      if (ch == 0) snprintf(outVal, 24, "OFF");
      else snprintf(outVal, 24, "Ch %d", ch);
      return;
    } else if (p.internalId == SYS_PARAM_PART_PATCH) {
      uint8_t eng = synthParts[activePartIdx].engine;
      uint8_t pNum = (uint8_t)p.currentVal;
      if (eng == ENG_DRUMS) {
        snprintf(outName, 24, "Base Note");
        snprintf(outVal, 24, "%u", pNum);
      } else {
        snprintf(outName, 24, "Patch: %u", pNum);
        const char *pName = getEnginePatchName(eng, pNum);
        snprintf(outVal, 24, "%s", pName ? pName : "-");
      }
      return;
    }
  }
  if (p.pType == TYPE_AMY && p.internalId == 112) {
    static const char* lfoW[] = {"TRI", "SIN", "SAW", "SQR", "RND"};
    snprintf(outVal, 24, "%s", lfoW[constrain((int)p.currentVal, 0, 4)]);
    return;
  }
  if (p.pType == TYPE_AMY && (p.internalId == 115 || p.internalId == 116)) {
    static const char* pW[] = {"SIN", "TRI", "SAW", "SQR"};
    snprintf(outVal, 24, "%s", pW[constrain((int)p.currentVal, 0, 3)]);
    return;
  } else if (p.pType == TYPE_AMY && p.internalId == 117) {
    static const char* hW[] = {"TRI", "PULSE", "NOISE", "SINE"};
    snprintf(outVal, 24, "%s", hW[constrain((int)p.currentVal, 0, 3)]);
    return;
  }
  if (p.pType == TYPE_AMY && p.internalId == 133) {
    snprintf(outVal, 24, "%s", (p.currentVal >= 0.5f) ? "ON" : "OFF");
    return;
  } else if (p.pType == TYPE_AMY && p.internalId == 136) {
    static const char* fltT[] = {"OFF", "LPF", "BPF", "HPF"};
    snprintf(outVal, 24, "%s", fltT[constrain((int)p.currentVal, 0, 3)]);
    return;
  }
  if (p.belongsToMode == MODE_DRUMS && p.belongsToPage == PAGE_OSC) {
    uint8_t pcmIdx = (uint8_t)constrain((int)p.currentVal, 0, 154);
    snprintf(outName, 24, "%s (%u)", p.name, pcmIdx);
    const char *pName = getEnginePatchName(ENG_SAMPLER, pcmIdx);
    snprintf(outVal, 24, "%s", pName ? pName : "-");
    return;
  }
  if (isIntegerParam(p)) snprintf(outVal, 24, "%.0f", p.currentVal);
  else snprintf(outVal, 24, "%.1f", p.currentVal);
}

inline void styleParamWidget(int idx, lv_obj_t *box, lv_obj_t *lbl) {
    if (!box || !lbl) return;
    Parameter &p = paramMatrix[idx];
    char *buf = (char*)alloca(64);
    int32_t permille = constrain((int32_t)mapValue(p.currentVal, p.minVal, p.maxVal, 0.0f, 1000.0f), 0, 1000);
    lv_bar_set_value(box, permille, LV_ANIM_OFF);

    if (parameterUnderLearning == idx) {
        lv_obj_set_style_bg_color(box, lv_color_hex(0xEB150F), 0);
        lv_obj_set_style_border_color(box, lv_color_hex(0xFFF8E7), 0);
        lv_obj_set_style_border_width(box, 2, 0);
        lv_obj_set_style_bg_color(box, lv_color_hex(0xFF3A3A), LV_PART_INDICATOR);
        snprintf(buf, 64, "%s\n[MIDI LEARN]", p.name);
    } else if (p.mappedCC != 255) {
        lv_obj_set_style_bg_color(box, lv_color_hex(0x1E1E24), 0);
        lv_obj_set_style_border_color(box, lv_color_hex(0xFF5100), 0);
        lv_obj_set_style_border_width(box, 2, 0);
        lv_obj_set_style_bg_color(box, lv_color_hex(0xDE4300), LV_PART_INDICATOR);
        char *dN=(char*)alloca(24),*dV=(char*)alloca(24); formatParamDisplay(p,dN,dV); snprintf(buf,64,"%s\n%s\nCC #%u",dN,dV,p.mappedCC);
    } else {
        lv_obj_set_style_bg_color(box, lv_color_hex(0x2B2B32), 0);
        lv_obj_set_style_border_color(box, lv_color_hex(0x5A5A64), 0);
        lv_obj_set_style_border_width(box, 1, 0);
        lv_obj_set_style_bg_color(box, lv_color_hex(0xF85D00), LV_PART_INDICATOR);
        char *dN=(char*)alloca(24),*dV=(char*)alloca(24); formatParamDisplay(p,dN,dV); snprintf(buf,64,"%s\n%s\nCC --",dN,dV);
    }
    lv_label_set_text(lbl, buf);
}

inline void refreshParamBox(int idx) {
    if (idx < 0 || idx >= TOTAL_PARAMS) return;
    if (paramBoxes[idx]) styleParamWidget(idx, paramBoxes[idx], paramLabels[idx]);
    if (activeOpDetail >= 0) {
        if (activeOpDetail == 7 || activeOpDetail == 8) {
            int fxBase = BASE_PARAMS + 6 * OP_DETAIL_COUNT + PEG_DETAIL_COUNT + (activeOpDetail == 8 ? 3 : 0);
            if (idx >= fxBase && idx < fxBase + 3) styleParamWidget(idx, opModalBoxes[idx - fxBase], opModalLabels[idx - fxBase]);
        } else {
            int count = (activeOpDetail == 6) ? PEG_DETAIL_COUNT : OP_DETAIL_COUNT;
            int base = BASE_PARAMS + activeOpDetail * OP_DETAIL_COUNT;
            if (idx >= base && idx < base + count) styleParamWidget(idx, opModalBoxes[idx - base], opModalLabels[idx - base]);
        }
    }
}

inline void applyParameterValue(int idx, float newVal) {
    if (idx < 0 || idx >= TOTAL_PARAMS) return;
    Parameter &p = paramMatrix[idx];
    p.currentVal = constrain(newVal, p.minVal, p.maxVal);
    if (isIntegerParam(p)) p.currentVal = roundf(p.currentVal);
    if (p.pType == TYPE_AMY && p.internalId == 14) updateDx7AlgoDiagram((int)p.currentVal);
    // Synka OPx Level mellan huvudruta och detalj-Level
    int opLvlIds[6] = {16, 18, 27, 29, 31, 33};
    for (int op = 0; op < 6; op++) {
        int detIdx = BASE_PARAMS + op * OP_DETAIL_COUNT;
        if (p.pType == TYPE_AMY && p.internalId == opLvlIds[op]) {
            paramMatrix[detIdx].currentVal = p.currentVal;
            refreshParamBox(detIdx);
        } else if (idx == detIdx) {
            for (int m = 0; m < BASE_PARAMS; m++) {
                if (paramMatrix[m].pType == TYPE_AMY && paramMatrix[m].internalId == opLvlIds[op]) {
                    paramMatrix[m].currentVal = p.currentVal;
                    refreshParamBox(m);
                }
            }
        }
    }
    if (synthParamRam) *(synthParamRam + activePartIdx * TOTAL_PARAMS + idx) = p.currentVal;
    refreshParamBox(idx);
    if (p.pType == TYPE_SYSTEM) {
  if (p.internalId == SYS_PARAM_PART_SEL) loadPartIntoMatrix((int)p.currentVal - 1);
  else if (p.internalId == SYS_PARAM_PART_ENGINE) {
    if (g_synthEditFrame) {
      uint8_t ne = (uint8_t)p.currentVal;
      if (ne == ENG_OFF || ne == ENG_DRUMS) lv_obj_add_flag(g_synthEditFrame, LV_OBJ_FLAG_HIDDEN);
      else lv_obj_clear_flag(g_synthEditFrame, LV_OBJ_FLAG_HIDDEN);
    }
    memset(((char*)synthPatchNames) + activePartIdx * 16, 0, 16);
    uint8_t eng = (uint8_t)p.currentVal;
    SynthPartConfig &sp = synthParts[activePartIdx];
    sp.engine = eng;
    if (eng == ENG_OFF) { sp.voices = 0; }
    else if (eng == ENG_JUNO) { if (sp.patchOrNote > 127) sp.patchOrNote = 0; if (sp.voices == 0) sp.voices = 4; }
    else if (eng == ENG_DX7) { if (sp.patchOrNote < 128) sp.patchOrNote = 128; if (sp.voices == 0) sp.voices = 4; }
    else if (eng == ENG_SAMPLER) { sp.patchOrNote = 0; if (sp.voices == 0) sp.voices = 2; }
    else if (eng == ENG_DRUMS) { sp.patchOrNote = 36; if (sp.voices == 0) sp.voices = 9; }
    else if (eng == ENG_PIANO) { sp.patchOrNote = 0; if (sp.voices == 0 || sp.voices > 4) sp.voices = 3; }
    int c = getOscCostPerVoice(eng);
    int rem = max(0, 120 - calcTotalUsedOscs(activePartIdx));
    if (c > 0 && sp.voices * c > rem) sp.voices = (uint8_t)(rem / c);
    for (int i = 0; i < BASE_PARAMS; i++) {
        if (paramMatrix[i].pType == TYPE_AMY && paramMatrix[i].internalId == 87) currentSeqPattern = constrain((int)paramMatrix[i].currentVal - 1, 0, SEQ_PATTERNS - 1);
        else if (paramMatrix[i].pType == TYPE_AMY && paramMatrix[i].internalId == 88) seqMaxSteps = constrain((int)paramMatrix[i].currentVal, 1, SEQ_STEPS);
    }
    refreshSeqGridUI();
    loadPartIntoMatrix(activePartIdx);
    int mSel = findMatrixIdxBySysId(SYS_PARAM_PART_SEL);
    if (mSel >= 0) refreshParamBox(mSel);
  }
  else if (p.internalId == SYS_PARAM_PART_CHAN) synthParts[activePartIdx].midiChan = (uint8_t)p.currentVal;
  else if (p.internalId == SYS_PARAM_PART_VOICES) {
    int cost = getOscCostPerVoice(synthParts[activePartIdx].engine);
    int rem = max(0, 120 - calcTotalUsedOscs(activePartIdx));
    int maxV = (cost > 0) ? min(9, rem / cost) : 9;
    if ((int)p.currentVal > maxV) { p.currentVal = (float)maxV; refreshParamBox(idx); }
    synthParts[activePartIdx].voices = (uint8_t)p.currentVal;
    updateOscBudgetMeter();
  }
  else if (p.internalId == SYS_PARAM_PART_PATCH) {
    uint8_t eng = synthParts[activePartIdx].engine;
    int v = (int)p.currentVal;
    if (eng == ENG_JUNO) v = constrain(v, 0, 127);
    else if (eng == ENG_DX7) v = constrain(v, 128, 255);
    else if (eng == ENG_SAMPLER) v = constrain(v, 0, 154);
    else if (eng == ENG_DRUMS) v = constrain(v, 0, 127);
    else v = 0;
    if ((float)v != p.currentVal) { p.currentVal = (float)v; refreshParamBox(idx); }
    synthParts[activePartIdx].patchOrNote = (uint8_t)v;
    memset(((char*)synthPatchNames) + activePartIdx * 16, 0, 16);
    int mSel = findMatrixIdxBySysId(SYS_PARAM_PART_SEL);
    if (mSel >= 0) refreshParamBox(mSel);
  }
  else if (p.internalId == SYS_PARAM_PART_LVL) synthParts[activePartIdx].level = p.currentVal / 10.0f;
  else if (p.internalId == SYS_PARAM_PATCH_FILE) {
    int ms = findMatrixIdxBySysId(SYS_PARAM_PART_SEL);
    if (ms >= 0 && (int)paramMatrix[ms].currentVal != (int)p.currentVal) applyParameterValue(ms, p.currentVal);
    else loadPartIntoMatrix((int)p.currentVal - 1);
  }
  if (p.internalId == SYS_PARAM_MASTER_VOL) amyEnqueueMasterVol(p.currentVal / 10.0f);
  else if (p.internalId >= SYS_PARAM_PART_ENGINE && p.internalId <= SYS_PARAM_PART_LVL) {
    SynthPartConfig &sp = synthParts[activePartIdx];
    amyEnqueuePartConfig((uint8_t)activePartIdx, sp.engine, sp.voices, sp.patchOrNote, sp.level);
  }
  if (p.internalId == SYS_PARAM_PROJ_SAVE) { int sl = (int)paramMatrix[findMatrixIdxBySysId(SYS_PARAM_PROJ_SLOT)].currentVal; saveActivePartParamsToRam(), saveProjectToDisk(sl, paramMatrix, TOTAL_PARAMS, synthParts, synthParamRam, (uint8_t*)seqPatterns, sizeof(seqPatterns), seqSongChain, sizeof(seqSongChain)); } else if (p.internalId == SYS_PARAM_PROJ_LOAD) { int sl = (int)paramMatrix[findMatrixIdxBySysId(SYS_PARAM_PROJ_SLOT)].currentVal; if (loadProjectFromDisk(sl, paramMatrix, TOTAL_PARAMS, synthParts, synthParamRam, (uint8_t*)seqPatterns, sizeof(seqPatterns), seqSongChain, sizeof(seqSongChain))) { for (int k = 0; k < TOTAL_PARAMS; k++) refreshParamBox(k); loadPartIntoMatrix(activePartIdx); } } else if (p.internalId == SYS_PARAM_FACTORY_RST) { factoryResetNvs(); }
  markStorageDirty();
  updateSystemHardware(p.internalId, p.currentVal);
}
    else {
      if (p.belongsToMode == MODE_SEQ) {
        if (p.internalId == 84) { seqIsPlaying = (p.currentVal >= 0.5f); if (!seqIsPlaying) seqPlayheadStep = -1; refreshSeqGridUI(); }
        else if (p.internalId == 87) {
            int maxP = constrain(seqNumPatterns, 1, SEQ_PATTERNS);
            if ((int)p.currentVal > maxP) {
                p.currentVal = (float)maxP;
                if (synthParamRam) *(synthParamRam + activePartIdx * TOTAL_PARAMS + idx) = p.currentVal;
                refreshParamBox(idx);
            }
            currentSeqPattern = constrain((int)p.currentVal - 1, 0, maxP - 1);
            refreshSeqGridUI();
        }
        else if (p.internalId == 88) { seqMaxSteps = constrain((int)p.currentVal, 1, SEQ_STEPS); refreshSeqGridUI(); }
        else if (p.internalId == 146) {
            seqNumPatterns = constrain((int)p.currentVal, 1, SEQ_PATTERNS);
            for (int m = 0; m < BASE_PARAMS; m++) {
                if (paramMatrix[m].pType == TYPE_AMY && paramMatrix[m].internalId == 87) {
                    if ((int)paramMatrix[m].currentVal > seqNumPatterns) {
                        paramMatrix[m].currentVal = (float)seqNumPatterns;
                        if (synthParamRam) *(synthParamRam + activePartIdx * TOTAL_PARAMS + m) = paramMatrix[m].currentVal;
                    }
                    currentSeqPattern = constrain((int)paramMatrix[m].currentVal - 1, 0, seqNumPatterns - 1);
                    refreshParamBox(m);
                }
            }
            rebuildSongDropdownOptions(); refreshSeqSongUI(); refreshSeqGridUI();
        }
      }
      amyEnqueueParam((uint8_t)activePartIdx, p.internalId, p.currentVal);
      markStorageDirty();
      Serial.printf("[AMY Param] %s (ID %d) = %.2f\n", p.name, p.internalId, p.currentVal);
    }
}

inline void onMidiNoteReceived(uint8_t ch, uint8_t note, uint8_t vel) {
    if (ch < 1 || ch > 16) return;
    if (vel > 0 && seqMidiRecActive && seqIsPlaying && seqPlayheadStep >= 0 && seqPlayheadStep < seqMaxSteps) {
        seqPatterns[currentSeqPattern][currentSeqTrack][seqPlayheadStep] = (currentSeqTrack < 9) ? 1 : note;
        if (currentSeqTrack >= 9) *(seqTrackNote + currentSeqTrack) = note;
        refreshSeqGridUI();
        onSeqGridChanged();
    }
    float fVel = (float)constrain((int)vel, 0, 127) / 127.0f;
    for (int i = 0; i < 16; i++) {
        SynthPartConfig *sp = synthParts + i;
        if (sp->engine == ENG_OFF || sp->voices == 0 || sp->midiChan != ch) continue;
        if (sp->engine == ENG_DRUMS) {
            if (vel > 0) {
                int dIdx = (int)note - (int)sp->patchOrNote;
                if (dIdx >= 0 && dIdx < 9) triggerDrum(dIdx, fVel);
            }
        } else {
            amyEnqueueNote((uint8_t)i, note, fVel);
        }
    }
}

inline void onMidiCCReceived(uint8_t ccNumber, uint8_t ccValue) {
    if (parameterUnderLearning >= 0 && parameterUnderLearning < TOTAL_PARAMS) {
        int learnedIdx = parameterUnderLearning;
        paramMatrix[learnedIdx].mappedCC = ccNumber;
        parameterUnderLearning = -1;
        Serial.printf("[MIDI Learn] Kopplade CC #%u -> %s\n", ccNumber, paramMatrix[learnedIdx].name);
        refreshParamBox(learnedIdx);
    }
    for (int i = 0; i < TOTAL_PARAMS; i++) {
        if (paramMatrix[i].mappedCC == ccNumber) {
            applyParameterValue(i, mapValue((float)ccValue, 0.0f, 127.0f, paramMatrix[i].minVal, paramMatrix[i].maxVal));
        }
    }
}


static lv_obj_t *stdValModal = nullptr;
static lv_obj_t *stdValTitle = nullptr;
static int stdValTargetIdx = -1;

inline void resetCurrentSynthToDefaults() {
    memset(((char*)synthPatchNames) + activePartIdx * 16, 0, 16);
    uint8_t eng = synthParts[activePartIdx].engine;
    for (int i = 0; i < TOTAL_PARAMS; i++) {
        Parameter *pp = paramMatrix + i;
        if (pp->pType == TYPE_AMY && paramMatchesEngine(pp->belongsToMode, eng)) {
            pp->currentVal = pp->defaultVal;
            if (synthParamRam) *(synthParamRam + activePartIdx * TOTAL_PARAMS + i) = pp->defaultVal;
            refreshParamBox(i);
        }
    }
    syncAllPartParamsToAmy(activePartIdx);
    int mSel = findMatrixIdxBySysId(SYS_PARAM_PART_SEL);
    if (mSel >= 0) refreshParamBox(mSel);
    markStorageDirty();
}

static lv_obj_t *renameModal = nullptr;
static lv_obj_t *renameTxtLbl = nullptr;
static char *renameEditBuf = (char*)ps_calloc(16, sizeof(char));

inline void updateRenameModalText() {
    if (!renameTxtLbl || !renameEditBuf) return;
    char *disp = (char*)alloca(24);
    snprintf(disp, 24, "%s_", renameEditBuf);
    lv_label_set_text(renameTxtLbl, disp);
}

inline void openPatchRenameModal() {
    if (!renameModal || !renameEditBuf) return;
    uint8_t eng = synthParts[activePartIdx].engine;
    if (eng == ENG_OFF || eng == ENG_DRUMS) return;
    char *cur = ((char*)synthPatchNames) + activePartIdx * 16;
    if (*cur != 0) {
        snprintf(renameEditBuf, 14, "%s", cur);
    } else {
        const char *rom = getEnginePatchName(eng, synthParts[activePartIdx].patchOrNote);
        snprintf(renameEditBuf, 14, "%s", rom ? rom : "User Patch");
    }
    updateRenameModalText();
    lv_obj_clear_flag(renameModal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(renameModal);
}

inline void buildPatchRenameModal(lv_obj_t *parent) {
    renameModal = lv_obj_create(parent);
    lv_obj_set_size(renameModal, 252, 232);
    lv_obj_align(renameModal, LV_ALIGN_TOP_RIGHT, 0, 2);
    lv_obj_set_style_bg_color(renameModal, lv_color_hex(0x1E1E24), 0);
    lv_obj_set_style_border_color(renameModal, lv_color_hex(0xFF5100), 0);
    lv_obj_set_style_border_width(renameModal, 2, 0);
    lv_obj_set_style_pad_all(renameModal, 4, 0);
    lv_obj_clear_flag(renameModal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(renameModal, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *topBox = lv_obj_create(renameModal);
    lv_obj_set_size(topBox, 240, 28);
    lv_obj_set_pos(topBox, 0, 0);
    lv_obj_set_style_bg_color(topBox, lv_color_hex(0x111115), 0);
    lv_obj_set_style_border_color(topBox, lv_color_hex(0x73A456), 0);
    lv_obj_set_style_border_width(topBox, 1, 0);
    lv_obj_set_style_pad_all(topBox, 2, 0);
    lv_obj_clear_flag(topBox, LV_OBJ_FLAG_SCROLLABLE);
    renameTxtLbl = lv_label_create(topBox);
    lv_obj_set_style_text_color(renameTxtLbl, lv_color_hex(0x82FA00), 0);
    lv_obj_center(renameTxtLbl);

    static const char *kbdMap[] = {
        "1","2","3","4","5","6","7","8","9","0","\n",
        "Q","W","E","R","T","Y","U","I","O","P","\n",
        "A","S","D","F","G","H","J","K","L","-","\n",
        "Z","X","C","V","B","N","M","SPC","DEL",""
    };
    lv_obj_t *km = lv_btnmatrix_create(renameModal);
    lv_btnmatrix_set_map(km, kbdMap);
    lv_obj_set_size(km, 240, 152);
    lv_obj_set_pos(km, 0, 32);
    lv_obj_set_style_bg_opa(km, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(km, 0, 0);
    lv_obj_set_style_pad_all(km, 1, 0);
    lv_obj_set_style_pad_row(km, 2, 0);
    lv_obj_set_style_pad_column(km, 2, 0);
    lv_obj_set_style_bg_color(km, lv_color_hex(0x34343B), LV_PART_ITEMS);
    lv_obj_set_style_text_color(km, lv_color_white(), LV_PART_ITEMS);
    lv_obj_add_event_cb(km, [](lv_event_t *e) {
        lv_obj_t *obj = lv_event_get_target(e);
        uint16_t id = lv_btnmatrix_get_selected_btn(obj);
        const char *txt = lv_btnmatrix_get_btn_text(obj, id);
        if (!txt || !renameEditBuf) return;
        size_t len = strlen(renameEditBuf);
        if (strcmp(txt, "DEL") == 0) {
            if (len > 0) *(renameEditBuf + len - 1) = 0;
        } else if (strcmp(txt, "SPC") == 0) {
            if (len < 13) { *(renameEditBuf + len) = ' '; *(renameEditBuf + len + 1) = 0; }
        } else if (len < 13) {
            *(renameEditBuf + len) = *txt;
            *(renameEditBuf + len + 1) = 0;
        }
        updateRenameModalText();
    }, LV_EVENT_VALUE_CHANGED, nullptr);

    lv_obj_t *bClr = lv_btn_create(renameModal);
    lv_obj_set_size(bClr, 74, 30);
    lv_obj_align(bClr, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(bClr, lv_color_hex(0xAE2021), 0);
    lv_obj_t *lCl = lv_label_create(bClr); lv_label_set_text(lCl, "CLR"); lv_obj_center(lCl);
    lv_obj_add_event_cb(bClr, [](lv_event_t *e) {
        if (renameEditBuf) memset(renameEditBuf, 0, 16);
        updateRenameModalText();
    }, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *bCan = lv_btn_create(renameModal);
    lv_obj_set_size(bCan, 76, 30);
    lv_obj_align(bCan, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bCan, lv_color_hex(0x5A5A64), 0);
    lv_obj_t *lCa = lv_label_create(bCan); lv_label_set_text(lCa, "CANCEL"); lv_obj_center(lCa);
    lv_obj_add_event_cb(bCan, [](lv_event_t *e) {
        lv_obj_add_flag(renameModal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *bOk = lv_btn_create(renameModal);
    lv_obj_set_size(bOk, 74, 30);
    lv_obj_align(bOk, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(bOk, lv_color_hex(0x469632), 0);
    lv_obj_t *lOk = lv_label_create(bOk); lv_label_set_text(lOk, "OK"); lv_obj_center(lOk);
    lv_obj_add_event_cb(bOk, [](lv_event_t *e) {
        if (renameEditBuf) {
            char *dst = ((char*)synthPatchNames) + activePartIdx * 16;
            snprintf(dst, 16, "%s", renameEditBuf);
            int mSel = findMatrixIdxBySysId(SYS_PARAM_PART_SEL);
            if (mSel >= 0) refreshParamBox(mSel);
            rebuildSeqDropdownOptions();
            markStorageDirty();
        }
        lv_obj_add_flag(renameModal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, nullptr);
}

static lv_obj_t *patchImportModal = nullptr;
static lv_obj_t *patchImportList = nullptr;

static lv_obj_t *projImportModal = nullptr;
static lv_obj_t *projImportList = nullptr;
static lv_obj_t *fileDelModal = nullptr;
static lv_obj_t *fileDelTitle = nullptr;
static char *fileDelTargetPath = (char*)ps_calloc(64, sizeof(char));
static bool fileDelIsProject = false;
static bool fileRowLongHandled = false;
inline void openProjImportModal();
inline void openPatchImportModal();
inline void openFileDeleteConfirm(const char *fullPath, const char *dispName, bool isProj) {
    if (!fileDelModal || !fileDelTitle || !fileDelTargetPath) return;
    snprintf(fileDelTargetPath, 64, "%s", fullPath);
    fileDelIsProject = isProj;
    char *msg = (char*)alloca(64);
    snprintf(msg, 64, "Delete\n%s?", dispName);
    lv_label_set_text(fileDelTitle, msg);
    lv_obj_clear_flag(fileDelModal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(fileDelModal);
}
inline void buildFileDeleteModal(lv_obj_t *parent) {
    fileDelModal = lv_obj_create(parent);
    lv_obj_set_size(fileDelModal, 196, 108);
    lv_obj_center(fileDelModal);
    lv_obj_set_style_bg_color(fileDelModal, lv_color_hex(0x24242A), 0);
    lv_obj_set_style_border_color(fileDelModal, lv_color_hex(0xEB150F), 0);
    lv_obj_set_style_border_width(fileDelModal, 2, 0);
    lv_obj_clear_flag(fileDelModal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(fileDelModal, LV_OBJ_FLAG_HIDDEN);
    fileDelTitle = lv_label_create(fileDelModal);
    lv_obj_set_style_text_color(fileDelTitle, lv_color_white(), 0);
    lv_obj_set_style_text_align(fileDelTitle, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(fileDelTitle, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_t *btnDel = lv_btn_create(fileDelModal);
    lv_obj_set_size(btnDel, 74, 30);
    lv_obj_align(btnDel, LV_ALIGN_BOTTOM_LEFT, 6, -2);
    lv_obj_set_style_bg_color(btnDel, lv_color_hex(0xAE2021), 0);
    lv_obj_t *lDel = lv_label_create(btnDel); lv_label_set_text(lDel, "DELETE"); lv_obj_center(lDel);
    lv_obj_add_event_cb(btnDel, [](lv_event_t *e) {
        if (activeStorageFS && fileDelTargetPath && *fileDelTargetPath != 0 && activeStorageFS->exists(fileDelTargetPath)) activeStorageFS->remove(fileDelTargetPath);
        lv_obj_add_flag(fileDelModal, LV_OBJ_FLAG_HIDDEN);
        if (fileDelIsProject) openProjImportModal(); else openPatchImportModal();
    }, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *btnCan = lv_btn_create(fileDelModal);
    lv_obj_set_size(btnCan, 74, 30);
    lv_obj_align(btnCan, LV_ALIGN_BOTTOM_RIGHT, -6, -2);
    lv_obj_set_style_bg_color(btnCan, lv_color_hex(0x5A5A64), 0);
    lv_obj_t *lCan = lv_label_create(btnCan); lv_label_set_text(lCan, "CANCEL"); lv_obj_center(lCan);
    lv_obj_add_event_cb(btnCan, [](lv_event_t *e) { lv_obj_add_flag(fileDelModal, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, nullptr);
}

inline void openProjImportModal() {
    if (!projImportModal || !projImportList || !activeStorageFS) return;
    lv_obj_clean(projImportList);
    ensureStorageDirs();
    File dir = activeStorageFS->open("/AMY/projects");
    int count = 0;
    if (dir && dir.isDirectory()) {
        File entry = dir.openNextFile();
        while (entry && count < 32) {
            if (!entry.isDirectory()) {
                const char *fullName = entry.name();
                const char *baseName = strrchr(fullName, '/');
                baseName = baseName ? (baseName + 1) : fullName;
                int slotNum = 0;
                if (sscanf(baseName, "proj_%d.bin", &slotNum) == 1 && slotNum >= 1 && slotNum <= 16) {
                    lv_obj_t *btn = lv_list_add_btn(projImportList, LV_SYMBOL_DIRECTORY, baseName);
                    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2B2B32), 0);
                    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
                    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
                        lv_event_code_t code = lv_event_get_code(e);
                        int sl = (int)(intptr_t)lv_event_get_user_data(e);
                        if (code == LV_EVENT_PRESSED) { fileRowLongHandled = false; return; }
                        if (code == LV_EVENT_LONG_PRESSED) { fileRowLongHandled = true; char *fp = (char*)alloca(40), *bn = (char*)alloca(20); snprintf(fp, 40, "/AMY/projects/proj_%02d.bin", sl); snprintf(bn, 20, "proj_%02d.bin", sl); openFileDeleteConfirm(fp, bn, true); return; }
                        if (code != LV_EVENT_CLICKED || fileRowLongHandled) return;
                        if (loadProjectFromDisk(sl, paramMatrix, TOTAL_PARAMS, synthParts, synthParamRam, (uint8_t*)seqPatterns, sizeof(seqPatterns), seqSongChain, sizeof(seqSongChain))) {
                            for (int k = 0; k < TOTAL_PARAMS; k++) refreshParamBox(k);
                            int mPartLoad = findMatrixIdxBySysId(SYS_PARAM_PART_SEL);
                            if (mPartLoad >= 0) activePartIdx = constrain((int)paramMatrix[mPartLoad].currentVal - 1, 0, 15);
                            loadPartIntoMatrix(activePartIdx);
                            int mFlip = findMatrixIdxBySysId(SYS_PARAM_FLIP);
                            if (mFlip >= 0) updateSystemHardware(SYS_PARAM_FLIP, paramMatrix[mFlip].currentVal);
                            for (int i = 0; i < 16; i++) amyEnqueuePartConfig((uint8_t)i, synthParts[i].engine, synthParts[i].voices, synthParts[i].patchOrNote, synthParts[i].level);
                            markStorageDirty();
                            fileFeedbackId = SYS_PARAM_PROJ_SLOT; fileFeedbackTxt = "LOADED!"; fileFeedbackUntil = millis() + 1200;
                        }
                        if (projImportModal) lv_obj_add_flag(projImportModal, LV_OBJ_FLAG_HIDDEN);
                    }, LV_EVENT_ALL, (void*)(intptr_t)slotNum);
                    count++;
                }
            }
            entry.close();
            entry = dir.openNextFile();
        }
        dir.close();
    }
    if (count == 0) {
        lv_obj_t *emptyLbl = lv_label_create(projImportList);
        lv_label_set_text(emptyLbl, "No projects in /AMY/projects");
        lv_obj_set_style_text_color(emptyLbl, lv_color_hex(0xB0B0B8), 0);
    }
    lv_obj_clear_flag(projImportModal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(projImportModal);
}

inline void buildProjImportModal(lv_obj_t *parent) {
    projImportModal = lv_obj_create(parent);
    lv_obj_set_size(projImportModal, 248, 224);
    lv_obj_align(projImportModal, LV_ALIGN_TOP_RIGHT, -2, 6);
    lv_obj_set_style_bg_color(projImportModal, lv_color_hex(0x1E1E24), 0);
    lv_obj_set_style_border_color(projImportModal, lv_color_hex(0x73A456), 0);
    lv_obj_set_style_border_width(projImportModal, 2, 0);
    lv_obj_set_style_pad_all(projImportModal, 6, 0);
    lv_obj_clear_flag(projImportModal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(projImportModal, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *hdr = lv_label_create(projImportModal);
    lv_label_set_text(hdr, "LOAD PROJECT (.bin)");
    lv_obj_set_style_text_color(hdr, lv_color_hex(0x82FA00), 0);
    lv_obj_align(hdr, LV_ALIGN_TOP_LEFT, 2, 0);

    projImportList = lv_list_create(projImportModal);
    lv_obj_set_size(projImportList, 232, 160);
    lv_obj_set_pos(projImportList, 0, 22);
    lv_obj_set_style_bg_color(projImportList, lv_color_hex(0x151519), 0);
    lv_obj_set_style_border_width(projImportList, 1, 0);
    lv_obj_set_style_border_color(projImportList, lv_color_hex(0x45454E), 0);

    lv_obj_t *bClose = lv_btn_create(projImportModal);
    lv_obj_set_size(bClose, 232, 26);
    lv_obj_align(bClose, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bClose, lv_color_hex(0x5A5A64), 0);
    lv_obj_t *lCl = lv_label_create(bClose); lv_label_set_text(lCl, "CLOSE"); lv_obj_center(lCl);
    lv_obj_add_event_cb(bClose, [](lv_event_t *e) {
        lv_obj_add_flag(projImportModal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, nullptr);
}

inline void openPatchImportModal() {
    if (!patchImportModal || !patchImportList || !activeStorageFS) return;
    lv_obj_clean(patchImportList);
    ensureStorageDirs();
    File dir = activeStorageFS->open("/AMY/patches");
    int count = 0;
    if (dir && dir.isDirectory()) {
        File entry = dir.openNextFile();
        while (entry && count < 32) {
            if (!entry.isDirectory()) {
                const char *fullName = entry.name();
                const char *baseName = strrchr(fullName, '/');
                baseName = baseName ? (baseName + 1) : fullName;
                size_t nLen = strlen(baseName);
                if (nLen > 4 && strcasecmp(baseName + nLen - 4, ".pat") == 0) {
                    lv_obj_t *btn = lv_list_add_btn(patchImportList, LV_SYMBOL_AUDIO, baseName);
                    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2B2B32), 0);
                    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
                    char *copyName = (char*)ps_malloc(40);
                    if (copyName) snprintf(copyName, 40, "%s", baseName);
                    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
                        lv_event_code_t code = lv_event_get_code(e);
                        char *fn = (char*)lv_event_get_user_data(e);
                        if (code == LV_EVENT_PRESSED) { fileRowLongHandled = false; return; }
                        if (code == LV_EVENT_LONG_PRESSED) { fileRowLongHandled = true; if (fn) { char *fp = (char*)alloca(56); snprintf(fp, 56, "/AMY/patches/%s", fn); openFileDeleteConfirm(fp, fn, false); } return; }
                        if (code != LV_EVENT_CLICKED || fileRowLongHandled) return;
                        if (fn && loadSinglePatchFromDisk(fn, activePartIdx, paramMatrix, TOTAL_PARAMS, synthParts, synthParamRam)) {
                            loadPartIntoMatrix(activePartIdx);
                            syncAllPartParamsToAmy(activePartIdx);
                            int mSel = findMatrixIdxBySysId(SYS_PARAM_PART_SEL);
                            if (mSel >= 0) refreshParamBox(mSel);
                            markStorageDirty();
                            fileFeedbackId = SYS_PARAM_PATCH_FILE; fileFeedbackTxt = "IMPORTED!"; fileFeedbackUntil = millis() + 1200;
                        }
                        if (patchImportModal) lv_obj_add_flag(patchImportModal, LV_OBJ_FLAG_HIDDEN);
                    }, LV_EVENT_ALL, copyName);
                    count++;
                }
            }
            entry.close();
            entry = dir.openNextFile();
        }
        dir.close();
    }
    if (count == 0) {
        lv_obj_t *emptyLbl = lv_label_create(patchImportList);
        lv_label_set_text(emptyLbl, "No .pat files in /AMY/patches");
        lv_obj_set_style_text_color(emptyLbl, lv_color_hex(0xB0B0B8), 0);
    }
    lv_obj_clear_flag(patchImportModal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(patchImportModal);
}

inline void buildPatchImportModal(lv_obj_t *parent) {
    patchImportModal = lv_obj_create(parent);
    lv_obj_set_size(patchImportModal, 248, 224);
    lv_obj_align(patchImportModal, LV_ALIGN_TOP_RIGHT, -2, 6);
    lv_obj_set_style_bg_color(patchImportModal, lv_color_hex(0x1E1E24), 0);
    lv_obj_set_style_border_color(patchImportModal, lv_color_hex(0x73A456), 0);
    lv_obj_set_style_border_width(patchImportModal, 2, 0);
    lv_obj_set_style_pad_all(patchImportModal, 6, 0);
    lv_obj_clear_flag(patchImportModal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(patchImportModal, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *hdr = lv_label_create(patchImportModal);
    lv_label_set_text(hdr, "IMPORT PATCH (.pat)");
    lv_obj_set_style_text_color(hdr, lv_color_hex(0x82FA00), 0);
    lv_obj_align(hdr, LV_ALIGN_TOP_LEFT, 2, 0);

    patchImportList = lv_list_create(patchImportModal);
    lv_obj_set_size(patchImportList, 232, 160);
    lv_obj_set_pos(patchImportList, 0, 22);
    lv_obj_set_style_bg_color(patchImportList, lv_color_hex(0x151519), 0);
    lv_obj_set_style_border_width(patchImportList, 1, 0);
    lv_obj_set_style_border_color(patchImportList, lv_color_hex(0x45454E), 0);

    lv_obj_t *bClose = lv_btn_create(patchImportModal);
    lv_obj_set_size(bClose, 232, 26);
    lv_obj_align(bClose, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bClose, lv_color_hex(0x5A5A64), 0);
    lv_obj_t *lCl = lv_label_create(bClose); lv_label_set_text(lCl, "CLOSE"); lv_obj_center(lCl);
    lv_obj_add_event_cb(bClose, [](lv_event_t *e) {
        lv_obj_add_flag(patchImportModal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, nullptr);
}

inline void openSynthResetModal() {
    if (!stdValModal || !stdValTitle) return;
    stdValTargetIdx = -2;
    char *t = (char*)alloca(56);
    snprintf(t, 56, "Reset Synth %d\nto defaults?", activePartIdx + 1);
    lv_label_set_text(stdValTitle, t);
    lv_obj_clear_flag(stdValModal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(stdValModal);
}

inline void openStdValModal(int idx) {
    if (!stdValModal || !stdValTitle || idx < 0 || idx >= TOTAL_PARAMS) return;
    stdValTargetIdx = idx;
    char *t = (char*)alloca(48);
    snprintf(t, 48, "%s\nSet std value?", paramMatrix[idx].name);
    lv_label_set_text(stdValTitle, t);
    lv_obj_clear_flag(stdValModal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(stdValModal);
}

inline void buildStdValModal(lv_obj_t *parent) {
    stdValModal = lv_obj_create(parent);
    lv_obj_set_size(stdValModal, 180, 104);
    lv_obj_center(stdValModal);
    lv_obj_set_style_bg_color(stdValModal, lv_color_hex(0x24242A), 0);
    lv_obj_set_style_border_color(stdValModal, lv_color_hex(0xFF5100), 0);
    lv_obj_set_style_border_width(stdValModal, 2, 0);
    lv_obj_clear_flag(stdValModal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(stdValModal, LV_OBJ_FLAG_HIDDEN);

    stdValTitle = lv_label_create(stdValModal);
    lv_obj_set_style_text_color(stdValTitle, lv_color_white(), 0);
    lv_obj_set_style_text_align(stdValTitle, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(stdValTitle, LV_ALIGN_TOP_MID, 0, 2);

    lv_obj_t *btnYes = lv_btn_create(stdValModal);
    lv_obj_set_size(btnYes, 68, 30);
    lv_obj_align(btnYes, LV_ALIGN_BOTTOM_LEFT, 6, -2);
    lv_obj_set_style_bg_color(btnYes, lv_color_hex(0x469632), 0);
    lv_obj_t *lYes = lv_label_create(btnYes);
    lv_label_set_text(lYes, "YES");
    lv_obj_center(lYes);
    lv_obj_add_event_cb(btnYes, [](lv_event_t *e) {
        if (stdValTargetIdx == -2) {
            resetCurrentSynthToDefaults();
        } else if (stdValTargetIdx >= 0 && stdValTargetIdx < TOTAL_PARAMS) {
            applyParameterValue(stdValTargetIdx, paramMatrix[stdValTargetIdx].defaultVal);
        }
        lv_obj_add_flag(stdValModal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *btnNo = lv_btn_create(stdValModal);
    lv_obj_set_size(btnNo, 68, 30);
    lv_obj_align(btnNo, LV_ALIGN_BOTTOM_RIGHT, -6, -2);
    lv_obj_set_style_bg_color(btnNo, lv_color_hex(0x5A5A64), 0);
    lv_obj_t *lNo = lv_label_create(btnNo);
    lv_label_set_text(lNo, "NO");
    lv_obj_center(lNo);
    lv_obj_add_event_cb(btnNo, [](lv_event_t *e) {
        lv_obj_add_flag(stdValModal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, nullptr);
}

inline void openOpDetailModal(int opIdx);

inline void triggerSystemAction(int idx) {
    int sysId = paramMatrix[idx].internalId;
    int sl = (int)paramMatrix[findMatrixIdxBySysId(SYS_PARAM_PROJ_SLOT)].currentVal;
    if (sysId == SYS_PARAM_PROJ_SLOT || sysId == SYS_PARAM_PROJ_SAVE) {
        saveActivePartParamsToRam();
        bool ok = saveProjectToDisk(sl, paramMatrix, TOTAL_PARAMS, synthParts, synthParamRam, (uint8_t*)seqPatterns, sizeof(seqPatterns), seqSongChain, sizeof(seqSongChain));
        fileFeedbackId = sysId; fileFeedbackTxt = ok ? "SAVED!" : "ERR!"; fileFeedbackUntil = millis() + 1200;
    } else if (sysId == SYS_PARAM_PROJ_LOAD) {
        bool ok = loadProjectFromDisk(sl, paramMatrix, TOTAL_PARAMS, synthParts, synthParamRam, (uint8_t*)seqPatterns, sizeof(seqPatterns), seqSongChain, sizeof(seqSongChain));
        if (ok) {
            for (int k = 0; k < TOTAL_PARAMS; k++) refreshParamBox(k);
            int mPartLoad = findMatrixIdxBySysId(SYS_PARAM_PART_SEL);
            if (mPartLoad >= 0) activePartIdx = constrain((int)paramMatrix[mPartLoad].currentVal - 1, 0, 15);
            loadPartIntoMatrix(activePartIdx);
            int mFlip = findMatrixIdxBySysId(SYS_PARAM_FLIP);
            if (mFlip >= 0) updateSystemHardware(SYS_PARAM_FLIP, paramMatrix[mFlip].currentVal);
            for (int i = 0; i < 16; i++) amyEnqueuePartConfig((uint8_t)i, synthParts[i].engine, synthParts[i].voices, synthParts[i].patchOrNote, synthParts[i].level);
            markStorageDirty();
        }
        fileFeedbackId = sysId; fileFeedbackTxt = ok ? "LOADED!" : "EMPTY!"; fileFeedbackUntil = millis() + 1200;
    } else if (sysId == SYS_PARAM_DEMO_LOAD) {
        memset(synthParts, 0, sizeof(synthParts));
        memset(synthPatchNames, 0, sizeof(synthPatchNames));
        for (int i = 0; i < 16; i++) synthParts[i].level = 0.8f;
        loadDemoSeqPatterns();
        *(synthParts + 0) = SynthPartConfig{ENG_DRUMS, 10, 9, 36, 0.90f};
        *(synthParts + 1) = SynthPartConfig{ENG_JUNO, 1, 4, 31, 0.85f};
        *(synthParts + 2) = SynthPartConfig{ENG_DX7, 2, 4, 138, 0.85f};
        for (int i = 0; i < TOTAL_PARAMS; i++) {
            if ((paramMatrix + i)->pType == TYPE_AMY) (paramMatrix + i)->currentVal = (paramMatrix + i)->defaultVal;
        }
        initPerSynthRam();
        for (int i = 0; i < 16; i++) {
            SynthPartConfig *sp = synthParts + i;
            amyEnqueuePartConfig((uint8_t)i, sp->engine, sp->voices, sp->patchOrNote, sp->level);
        }
        currentSeqPattern = 0; seqMaxSteps = 16;
        loadPartIntoMatrix(activePartIdx);
        for (int k = 0; k < TOTAL_PARAMS; k++) {
            int pid = paramMatrix[k].internalId;
            if (pid == 46 || pid == 47 || pid == 48 || pid == 148 || pid == 149 || pid == 150 || pid == 151 || pid == 152) {
                amyEnqueueParam(0, pid, paramMatrix[k].currentVal);
            }
        }
        refreshSeqGridUI();
        markStorageDirty();
        fileFeedbackId = sysId; fileFeedbackTxt = "LOADED!"; fileFeedbackUntil = millis() + 1200;
    } else if (sysId == SYS_PARAM_PATCH_FILE) {
        saveActivePartParamsToRam();
        bool ok = saveSinglePatchToDisk(activePartIdx, paramMatrix, TOTAL_PARAMS, synthParts, synthParamRam);
        fileFeedbackId = sysId; fileFeedbackTxt = ok ? "EXPORTED!" : "ERR!"; fileFeedbackUntil = millis() + 1200;
    } else if (sysId == SYS_PARAM_FACTORY_RST) {
        fileFeedbackId = sysId; fileFeedbackTxt = "RESETTING..."; fileFeedbackUntil = millis() + 1200;
        refreshParamBox(idx);
        lv_timer_handler();
        factoryResetNvs();
    }
    refreshParamBox(idx);
}

inline void param_box_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    lv_obj_t *box = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED) {
        isDraggingFader = false;
        lv_point_t pt;
        if (lv_indev_get_act()) {
            lv_indev_get_point(lv_indev_get_act(), &pt);
            dragStartX = pt.x; dragStartY = pt.y;
        }
    } else if (code == LV_EVENT_PRESSING && lv_indev_get_act()) {
        lv_point_t pt;
        lv_indev_get_point(lv_indev_get_act(), &pt);
        int dx = abs(pt.x - dragStartX), dy = abs(pt.y - dragStartY);
        if (!isDraggingFader && dx >= 6 && dx > dy && !isSystemActionParam(paramMatrix[idx])) {
            isDraggingFader = true;
            if (parameterUnderLearning == idx) parameterUnderLearning = -1;
        }
        if (isDraggingFader && !isSystemActionParam(paramMatrix[idx]) && !(paramMatrix[idx].pType == TYPE_SYSTEM && paramMatrix[idx].internalId == SYS_PARAM_PROJ_SLOT)) {
            lv_area_t coords;
            lv_obj_get_coords(box, &coords);
            float ratio = constrain((float)(pt.x - coords.x1) / (float)(coords.x2 - coords.x1), 0.0f, 1.0f);
            applyParameterValue(idx, paramMatrix[idx].minVal + ratio * (paramMatrix[idx].maxVal - paramMatrix[idx].minVal));
        }
    } else if (code == LV_EVENT_LONG_PRESSED && !isDraggingFader) {
        int sysId = (paramMatrix[idx].pType == TYPE_SYSTEM) ? paramMatrix[idx].internalId : -1;
        if (sysId == SYS_PARAM_FACTORY_RST) { triggerSystemAction(idx); return; }
        if (sysId == SYS_PARAM_PROJ_SLOT) { openProjImportModal(); return; }
        if (sysId == SYS_PARAM_PATCH_FILE) { openPatchImportModal(); return; }
        if (isSystemActionParam(paramMatrix[idx])) return;
        int prev = parameterUnderLearning;
        parameterUnderLearning = (parameterUnderLearning == idx) ? -1 : idx;
        if (prev != -1 && prev != idx) refreshParamBox(prev);
        refreshParamBox(idx);
    } else if (code == LV_EVENT_SHORT_CLICKED && !isDraggingFader) {
        static lv_timer_t *pendTmr = nullptr;
        static int pendIdx = -1;
        static float pendDir = 1.0f;
        auto cancelPend = [&]() { if (pendTmr) { lv_timer_del(pendTmr); pendTmr = nullptr; } pendIdx = -1; };
        int sysId = (paramMatrix[idx].pType == TYPE_SYSTEM) ? paramMatrix[idx].internalId : -1;
        if (sysId == SYS_PARAM_FACTORY_RST || sysId == SYS_PARAM_DEMO_LOAD) { cancelPend(); if (sysId == SYS_PARAM_DEMO_LOAD) triggerSystemAction(idx); return; }
        if (parameterUnderLearning == idx) { cancelPend(); parameterUnderLearning = -1; refreshParamBox(idx); return; }
        uint32_t dblWin = (uint32_t)(380 - (int)paramMatrix[findMatrixIdxBySysId(SYS_PARAM_DBLCLICK)].currentVal * 80);
        if (pendTmr != nullptr && pendIdx == idx) {
            cancelPend();
            if (sysId == SYS_PARAM_PROJ_SLOT || sysId == SYS_PARAM_PATCH_FILE) { triggerSystemAction(idx); return; }
            if (isFxModalTrigger(paramMatrix[idx])) { openOpDetailModal(paramMatrix[idx].internalId == 46 ? 7 : 8); return; }
            if (isOpMenuTrigger(paramMatrix[idx])) {
                if (paramMatrix[idx].internalId == 54) { openOpDetailModal(6); return; }
                static const int opIds[] = {16, 18, 27, 29, 31, 33};
                for (int op = 0; op < 6; op++) if (paramMatrix[idx].internalId == opIds[op]) { openOpDetailModal(op); return; }
            }
            if (paramMatrix[idx].pType == TYPE_AMY) { openStdValModal(idx); return; }
            if (sysId == SYS_PARAM_PART_SEL) { openPatchRenameModal(); return; }
        }
        cancelPend();
        lv_area_t coords; lv_obj_get_coords(box, &coords);
        float dir = (dragStartX >= (coords.x1 + coords.x2) / 2) ? 1.0f : -1.0f;
        bool hasDbl = (sysId == SYS_PARAM_PROJ_SLOT || sysId == SYS_PARAM_PATCH_FILE || sysId == SYS_PARAM_PART_SEL || isFxModalTrigger(paramMatrix[idx]) || isOpMenuTrigger(paramMatrix[idx]) || paramMatrix[idx].pType == TYPE_AMY);
        if (!hasDbl) {
            float step = isIntegerParam(paramMatrix[idx]) ? 1.0f : ((paramMatrix[idx].maxVal - paramMatrix[idx].minVal) * 0.02f);
            applyParameterValue(idx, paramMatrix[idx].currentVal + dir * step);
            return;
        }
        pendIdx = idx; pendDir = dir;
        pendTmr = lv_timer_create([](lv_timer_t *t) {
            int ti = pendIdx; float td = pendDir; pendTmr = nullptr; pendIdx = -1; lv_timer_del(t);
            if (ti < 0 || ti >= TOTAL_PARAMS) return;
            int ts = (paramMatrix[ti].pType == TYPE_SYSTEM) ? paramMatrix[ti].internalId : -1;
            if (ts == SYS_PARAM_PATCH_FILE) {
                int np = (activePartIdx + (td > 0 ? 1 : 15)) % 16;
                int ms = findMatrixIdxBySysId(SYS_PARAM_PART_SEL);
                if (ms >= 0) applyParameterValue(ms, (float)(np + 1)); else loadPartIntoMatrix(np);
                refreshParamBox(ti);
            } else {
                float step = isIntegerParam(paramMatrix[ti]) ? 1.0f : ((paramMatrix[ti].maxVal - paramMatrix[ti].minVal) * 0.02f);
                applyParameterValue(ti, paramMatrix[ti].currentVal + td * step);
            }
        }, dblWin, nullptr);
    }
}

inline lv_obj_t* createParamBarWidget(lv_obj_t *parent, int paramIdx, lv_obj_t **outLbl) {
    lv_obj_t *box = lv_bar_create(parent);
    lv_obj_set_size(box, 125, 67);
    lv_bar_set_range(box, 0, 1000);
    lv_obj_set_style_radius(box, 6, 0);
    lv_obj_set_style_radius(box, 5, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(box, 0, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(box, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(box, param_box_event_cb, LV_EVENT_ALL, (void *)(intptr_t)paramIdx);
    bool hasDblTap = isOpMenuTrigger(paramMatrix[paramIdx]) || isFxModalTrigger(paramMatrix[paramIdx]) || (paramMatrix[paramIdx].pType == TYPE_SYSTEM && (paramMatrix[paramIdx].internalId == SYS_PARAM_PART_SEL || paramMatrix[paramIdx].internalId == SYS_PARAM_PATCH_FILE || paramMatrix[paramIdx].internalId == SYS_PARAM_PROJ_SLOT));
    if (hasDblTap) {
        lv_obj_t *frame = lv_obj_create(box);
        if (paramMatrix[paramIdx].pType == TYPE_SYSTEM && paramMatrix[paramIdx].internalId == SYS_PARAM_PART_SEL) g_synthEditFrame = frame;
        lv_obj_set_size(frame, 125, 67);
        lv_obj_center(frame);
        lv_obj_set_style_bg_opa(frame, LV_OPA_TRANSP, 0);
        bool isSaveFrame = (paramMatrix[paramIdx].pType == TYPE_SYSTEM && (paramMatrix[paramIdx].internalId == SYS_PARAM_PROJ_SLOT || paramMatrix[paramIdx].internalId == SYS_PARAM_PATCH_FILE));
        lv_obj_set_style_border_color(frame, lv_color_hex(isSaveFrame ? 0x73A456 : 0x5783AB), 0);
        lv_obj_set_style_border_width(frame, 4, 0);
        lv_obj_set_style_radius(frame, 6, 0);
        lv_obj_clear_flag(frame, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    }
    lv_obj_t *lbl = lv_label_create(box);
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(lbl);
    *outLbl = lbl;
    return box;
}

static lv_obj_t *opModalTv = nullptr;

inline void openOpDetailModal(int opIdx) {
    if (!opModal || !opModalTv) return;
    activeOpDetail = opIdx;
    char *backTabTxt = (char*)alloca(16);
    if (opIdx == 7 || opIdx == 8) {
        snprintf(backTabTxt, 16, (opIdx == 7) ? "< ECHO" : "< REV");
        lv_tabview_rename_tab(opModalTv, 1, "PARAM");
        lv_tabview_rename_tab(opModalTv, 2, "");
        lv_tabview_rename_tab(opModalTv, 3, "");
    } else if (opIdx == 6) {
        snprintf(backTabTxt, 16, "< PEG");
        lv_tabview_rename_tab(opModalTv, 1, "RATE");
        lv_tabview_rename_tab(opModalTv, 2, "LVL");
        lv_tabview_rename_tab(opModalTv, 3, "LFO");
    } else {
        snprintf(backTabTxt, 16, "< OP%d", opIdx + 1);
        lv_tabview_rename_tab(opModalTv, 1, "OSC");
        lv_tabview_rename_tab(opModalTv, 2, "RATE");
        lv_tabview_rename_tab(opModalTv, 3, "LVL");
    }
    lv_tabview_rename_tab(opModalTv, 0, backTabTxt);
    lv_tabview_set_act(opModalTv, 1, LV_ANIM_OFF);

    int base = (opIdx >= 7) ? (BASE_PARAMS + 6 * OP_DETAIL_COUNT + PEG_DETAIL_COUNT + (opIdx == 8 ? 3 : 0)) : (BASE_PARAMS + opIdx * OP_DETAIL_COUNT);
    static const int opMap[OP_DETAIL_COUNT]  = {0,1,2,3,4,5, 6,7,8,9, 10,11,12,13,14,15};
    static const int pegMap[OP_DETAIL_COUNT] = {0,1,2,3,-1,-1, 4,5,6,7, 8,9,-1,-1,-1,-1};
    static const int fxMap[OP_DETAIL_COUNT]  = {0,1,2,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,-1,-1};
    const int *mapArr = (opIdx >= 7) ? fxMap : ((opIdx == 6) ? pegMap : opMap);

    for (int k = 0; k < OP_DETAIL_COUNT; k++) {
        lv_obj_remove_event_cb(opModalBoxes[k], param_box_event_cb);
        if (mapArr[k] >= 0) {
            lv_obj_clear_flag(opModalBoxes[k], LV_OBJ_FLAG_HIDDEN);
            int pIdx = base + mapArr[k];
            lv_obj_add_event_cb(opModalBoxes[k], param_box_event_cb, LV_EVENT_ALL, (void *)(intptr_t)pIdx);
            styleParamWidget(pIdx, opModalBoxes[k], opModalLabels[k]);
        } else {
            lv_obj_add_flag(opModalBoxes[k], LV_OBJ_FLAG_HIDDEN);
        }
    }
    lv_obj_clear_flag(opModal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(opModal);
}

inline void buildOpDetailModal(lv_obj_t *parent) {
    opModal = lv_obj_create(parent);
    lv_obj_set_size(opModal, 256, 240);
    lv_obj_align(opModal, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(opModal, lv_color_hex(0x24242A), 0);
    lv_obj_set_style_pad_all(opModal, 0, 0);
    lv_obj_set_style_border_width(opModal, 0, 0);
    lv_obj_clear_flag(opModal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(opModal, LV_OBJ_FLAG_HIDDEN);

    opModalTv = lv_tabview_create(opModal, LV_DIR_TOP, 30);
    lv_obj_set_size(opModalTv, 256, 240);
    lv_obj_set_pos(opModalTv, 0, 0);
    lv_obj_clear_flag(lv_tabview_get_content(opModalTv), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(opModalTv, lv_color_hex(0xC8C6BE), 0);
    lv_obj_set_style_pad_all(opModalTv, 0, 0);
    lv_obj_set_style_pad_all(lv_tabview_get_content(opModalTv), 0, 0);

    lv_obj_t *btns = lv_tabview_get_tab_btns(opModalTv);
    lv_obj_set_style_bg_color(btns, lv_color_hex(0x34343B), 0);
    lv_obj_set_style_text_color(btns, lv_color_hex(0xD4D2CC), 0);
    lv_obj_set_style_text_color(btns, lv_color_hex(0xFF5100), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(btns, lv_color_hex(0xFF5100), LV_PART_ITEMS | LV_STATE_CHECKED);

    lv_tabview_add_tab(opModalTv, "< OP1");
    lv_obj_t *tabs[] = {
        lv_tabview_add_tab(opModalTv, "OSC"),
        lv_tabview_add_tab(opModalTv, "RATE"),
        lv_tabview_add_tab(opModalTv, "LVL")
    };

    lv_obj_add_event_cb(opModalTv, [](lv_event_t *e) {
        if (lv_tabview_get_tab_act(opModalTv) == 0) {
            activeOpDetail = -1;
            lv_obj_add_flag(opModal, LV_OBJ_FLAG_HIDDEN);
        }
    }, LV_EVENT_VALUE_CHANGED, nullptr);

    static const int counts[] = {6, 4, 6};
    int offset = 0;
    for (int t = 0; t < 3; t++) {
        lv_obj_set_style_bg_color(tabs[t], lv_color_hex(0xC8C6BE), 0);
        lv_obj_set_style_pad_all(tabs[t], 2, 0);
        lv_obj_set_style_border_width(tabs[t], 0, 0);
        lv_obj_set_scrollbar_mode(tabs[t], LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_flex_flow(tabs[t], LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_style_pad_row(tabs[t], 2, 0);
        lv_obj_set_style_pad_column(tabs[t], 2, 0);
        for (int c = 0; c < counts[t]; c++) {
            int k = offset + c;
            opModalBoxes[k] = createParamBarWidget(tabs[t], BASE_PARAMS + k, &opModalLabels[k]);
        }
        offset += counts[t];
    }
}

inline void populateSubPage(lv_obj_t *parent, MainMode mode, SubPage page) {
    lv_obj_set_style_bg_color(parent, lv_color_hex(0xC8C6BE), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(parent, 2, 0);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_row(parent, 2, 0);
    lv_obj_set_style_pad_column(parent, 2, 0);

    for (int i = 0; i < BASE_PARAMS; i++) {
        if (paramMatrix[i].name != nullptr && paramMatrix[i].belongsToMode == mode && paramMatrix[i].belongsToPage == page) {
            paramBoxes[i] = createParamBarWidget(parent, i, &paramLabels[i]);
            if (mode == MODE_DRUMS) lv_obj_set_size(paramBoxes[i], 82, 67);
            paramBars[i] = paramBoxes[i];
            refreshParamBox(i);
            if (paramMatrix[i].pType == TYPE_AMY && paramMatrix[i].belongsToMode == MODE_SEQ && paramMatrix[i].internalId == 147) {
                lv_obj_set_style_bg_opa(paramBoxes[i], LV_OPA_TRANSP, 0);
                lv_obj_set_style_bg_opa(paramBoxes[i], LV_OPA_TRANSP, LV_PART_INDICATOR);
                lv_obj_set_style_border_width(paramBoxes[i], 0, 0);
                lv_obj_clear_flag(paramBoxes[i], LV_OBJ_FLAG_CLICKABLE);
                lv_obj_add_flag(paramLabels[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

inline lv_obj_t* createStyledSubTabview(lv_obj_t *parent, int helpSecId = -1) {
    lv_obj_t *tv = lv_tabview_create(parent, LV_DIR_TOP, 30);
    lv_obj_clear_flag(lv_tabview_get_content(tv), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(tv, lv_color_hex(0xC8C6BE), 0);
    lv_obj_set_style_pad_all(tv, 0, 0);
    lv_obj_set_style_pad_all(lv_tabview_get_content(tv), 0, 0);
    lv_obj_t *btns = lv_tabview_get_tab_btns(tv);
    lv_obj_set_style_bg_color(btns, lv_color_hex(0x34343B), 0);
    lv_obj_set_style_text_color(btns, lv_color_hex(0xD4D2CC), 0);
    lv_obj_set_style_text_color(btns, lv_color_hex(0xFF5100), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(btns, lv_color_hex(0xFF5100), LV_PART_ITEMS | LV_STATE_CHECKED);
    if (helpSecId >= 0) {
        lv_obj_add_event_cb(btns, [](lv_event_t *e) {
            int sec = (int)(intptr_t)lv_event_get_user_data(e);
            uint16_t act = lv_btnmatrix_get_selected_btn(lv_event_get_target(e));
            openHelpForSection(sec, act);
        }, LV_EVENT_LONG_PRESSED, (void*)(intptr_t)helpSecId);
    }
    return tv;
}

inline const char* getSynthSlotEngineShortName(int slotIdx) {
    if (slotIdx < 0 || slotIdx >= 16) return "Off";
    uint8_t eng = (synthParts + slotIdx)->engine;
    if (eng == ENG_JUNO) return "Juno";
    if (eng == ENG_DX7) return "DX7";
    if (eng == ENG_SAMPLER) return "PCM";
    if (eng == ENG_DRUMS) return "Kit";
    if (eng == ENG_PIANO) return "Piano";
    return "Off";
}

inline void onSeqTogglePlayFromGrid() {
    bool willPlay = !seqIsPlaying;
    if (willPlay && g_mainTv && g_seqTv && lv_tabview_get_tab_act(g_mainTv) == 1) {
        uint16_t sub = lv_tabview_get_tab_act(g_seqTv);
        if (sub == 1) {
            seqSongModeActive = true;
            seqSongPos = 0;
            uint8_t p0 = *(seqSongChain + 0);
            if (p0 >= 1 && p0 <= SEQ_PATTERNS) currentSeqPattern = p0 - 1;
        } else {
            seqSongModeActive = false;
        }
        refreshSeqGridUI();
    }
    for (int i = 0; i < BASE_PARAMS; i++) {
        if (paramMatrix[i].pType == TYPE_AMY && paramMatrix[i].internalId == 84) {
            applyParameterValue(i, willPlay ? 1.0f : 0.0f);
            return;
        }
    }
}

inline void onSeqGridChanged() {
    markStorageDirty();
}

inline void create_synth_gui() {
    initDx7OperatorParams();
    lv_obj_t *main_tv = lv_tabview_create(lv_scr_act(), LV_DIR_LEFT, 64);
    g_mainTv = main_tv;
    lv_obj_clear_flag(lv_tabview_get_content(main_tv), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(main_tv, lv_color_hex(0xB8B5AC), 0);
    lv_obj_t *main_btns = lv_tabview_get_tab_btns(main_tv);
    lv_obj_set_height(main_btns, 150);
    lv_obj_align(main_btns, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(main_btns, lv_color_hex(0xB8B5AC), 0);
    g_mainBtns = main_btns;
    lv_obj_set_style_text_color(main_btns, lv_color_hex(0x24242A), 0);
    lv_obj_set_style_text_color(main_btns, lv_color_hex(0x7A7872), LV_PART_ITEMS | LV_STATE_DISABLED);
    lv_obj_set_style_bg_color(main_btns, lv_color_hex(0x9E9B92), LV_PART_ITEMS | LV_STATE_DISABLED);
    lv_obj_set_style_text_color(main_btns, lv_color_hex(0xF83800), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(main_btns, lv_color_hex(0xF83800), LV_PART_ITEMS | LV_STATE_CHECKED);

    lv_obj_t *tab_engine = lv_tabview_add_tab(main_tv, "Juno");
    g_logoPanel = lv_obj_create(tab_engine);
    lv_obj_set_size(g_logoPanel, 256, 240);
    lv_obj_set_pos(g_logoPanel, 0, 0);
    lv_obj_set_style_bg_color(g_logoPanel, lv_color_hex(0x1E1E24), 0);
    lv_obj_set_style_border_color(g_logoPanel, lv_color_hex(0xFF5100), 0);
    lv_obj_set_style_border_width(g_logoPanel, 2, 0);
    lv_obj_clear_flag(g_logoPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_logoPanel, LV_OBJ_FLAG_HIDDEN);

    lv_point_t *wavePts = (lv_point_t*)ps_malloc(12 * sizeof(lv_point_t));
    for (int i = 0; i < 6; i++) {
        (wavePts + i * 2)->x = (lv_coord_t)(38 + i * 30);
        (wavePts + i * 2)->y = 82;
        (wavePts + i * 2 + 1)->x = (lv_coord_t)(68 + i * 30);
        (wavePts + i * 2 + 1)->y = 42;
    }
    lv_obj_t *waveLine = lv_line_create(g_logoPanel);
    lv_line_set_points(waveLine, wavePts, 12);
    lv_obj_set_style_line_color(waveLine, lv_color_hex(0xFF5100), 0);
    lv_obj_set_style_line_width(waveLine, 3, 0);

    lv_obj_t *logoTitle = lv_label_create(g_logoPanel);
    lv_label_set_text(logoTitle, "MiNi-AMY");
    lv_obj_set_style_text_color(logoTitle, lv_color_white(), 0);
    lv_obj_set_style_text_font(logoTitle, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_letter_space(logoTitle, 4, 0);
    lv_obj_align(logoTitle, LV_ALIGN_CENTER, 0, -2);

    lv_obj_t *logoSub = lv_label_create(g_logoPanel);
    lv_label_set_text(logoSub, "by S.C.H.U.T.Z");
    lv_obj_set_style_text_color(logoSub, lv_color_hex(0xA8A6A0), 0);
    lv_obj_set_style_text_align(logoSub, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(logoSub, LV_ALIGN_CENTER, 0, 18);

    for (int d = 0; d < 2; d++) {
        lv_obj_t *dot = lv_obj_create(g_logoPanel);
        lv_obj_remove_style_all(dot);
        lv_obj_set_size(dot, 3, 3);
        lv_obj_set_style_bg_color(dot, lv_color_hex(0x82FA00), 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(dot, 1, 0);
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(dot, LV_ALIGN_CENTER, 16 + d * 5, 9);
    }

    lv_obj_t *logoHint = lv_label_create(g_logoPanel);
    lv_label_set_text(logoHint, "[Synth OFF - Tap to setup]");
    lv_obj_set_style_text_color(logoHint, lv_color_hex(0x7A7872), 0);
    lv_obj_set_style_pad_bottom(g_logoPanel, 3, 0);
    lv_obj_align(logoHint, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(g_logoPanel, [](lv_event_t *e) {
        if (g_mainTv) lv_tabview_set_act(g_mainTv, 2, LV_ANIM_OFF);
        if (g_sysTv) lv_tabview_set_act(g_sysTv, 1, LV_ANIM_OFF);
    }, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *tab_seq    = lv_tabview_add_tab(main_tv, "Seq");
    lv_obj_t *tab_system = lv_tabview_add_tab(main_tv, "System");
    lv_obj_add_event_cb(main_tv, [](lv_event_t *e) {
        if (opModal && !lv_obj_has_flag(opModal, LV_OBJ_FLAG_HIDDEN)) {
            activeOpDetail = -1;
            lv_obj_add_flag(opModal, LV_OBJ_FLAG_HIDDEN);
        }
    }, LV_EVENT_VALUE_CHANGED, nullptr);
    for (lv_obj_t *t : {tab_engine, tab_seq, tab_system}) {
        lv_obj_set_style_pad_all(t, 0, 0);
        lv_obj_set_scrollbar_mode(t, LV_SCROLLBAR_MODE_OFF);
    }

    lv_obj_t *juno_tv = createStyledSubTabview(tab_engine, 0);
    g_junoTv = juno_tv;
    populateSubPage(lv_tabview_add_tab(juno_tv, "DCO"), MODE_JUNO, PAGE_OSC);
    populateSubPage(lv_tabview_add_tab(juno_tv, "LFO"), MODE_JUNO, PAGE_LFO);
    populateSubPage(lv_tabview_add_tab(juno_tv, "VCF"), MODE_JUNO, PAGE_FILTER);
    populateSubPage(lv_tabview_add_tab(juno_tv, "ENV"), MODE_JUNO, PAGE_ENV);
    populateSubPage(lv_tabview_add_tab(juno_tv, "FX"),  MODE_JUNO, PAGE_FX);

    lv_obj_t *dx7_tv = createStyledSubTabview(tab_engine, 1);
    g_dx7Tv = dx7_tv;
    lv_obj_t *dx7_alg_page = lv_tabview_add_tab(dx7_tv, "ALG");
    populateSubPage(dx7_alg_page, MODE_DX7, PAGE_OSC);
    createDx7AlgoWidget(dx7_alg_page);
    populateSubPage(lv_tabview_add_tab(dx7_tv, "OP1-3"),  MODE_DX7, PAGE_FILTER);
    populateSubPage(lv_tabview_add_tab(dx7_tv, "OP4-6"),  MODE_DX7, PAGE_ENV);
    populateSubPage(lv_tabview_add_tab(dx7_tv, "GLOBAL"), MODE_DX7, PAGE_LFO);

    lv_obj_t *drums_tv = createStyledSubTabview(tab_engine, 2);
    g_drumsTv = drums_tv;
    populateSubPage(lv_tabview_add_tab(drums_tv, "KIT"),  MODE_DRUMS, PAGE_OSC);
    populateSubPage(lv_tabview_add_tab(drums_tv, "TUNE"), MODE_DRUMS, PAGE_LFO);
    populateSubPage(lv_tabview_add_tab(drums_tv, "MIX"),  MODE_DRUMS, PAGE_FILTER);
    populateSubPage(lv_tabview_add_tab(drums_tv, "PAN"),  MODE_DRUMS, PAGE_ENV);
    populateSubPage(lv_tabview_add_tab(drums_tv, "FLT"),  MODE_DRUMS, PAGE_FX);

    lv_obj_t *samp_tv = createStyledSubTabview(tab_engine, 3);
    g_sampTv = samp_tv;
    populateSubPage(lv_tabview_add_tab(samp_tv, "PLAYER"), MODE_SAMPLER, PAGE_OSC);
    populateSubPage(lv_tabview_add_tab(samp_tv, "FLT"),  MODE_SAMPLER, PAGE_FILTER);
    populateSubPage(lv_tabview_add_tab(samp_tv, "ENV"),  MODE_SAMPLER, PAGE_ENV);

    lv_obj_t *piano_tv = createStyledSubTabview(tab_engine, 4);
    g_pianoTv = piano_tv;
    populateSubPage(lv_tabview_add_tab(piano_tv, "TONE"),   MODE_PIANO, PAGE_OSC);
    populateSubPage(lv_tabview_add_tab(piano_tv, "TIMBRE"), MODE_PIANO, PAGE_LFO);
    populateSubPage(lv_tabview_add_tab(piano_tv, "MIX"),    MODE_PIANO, PAGE_FILTER);
    populateSubPage(lv_tabview_add_tab(piano_tv, "ENV"),    MODE_PIANO, PAGE_ENV);

    lv_obj_t *seq_tv = createStyledSubTabview(tab_seq, 5);
    g_seqTv = seq_tv;
    buildSeqGridPage(lv_tabview_add_tab(seq_tv, "PATTERN"));
    buildSeqSongPage(lv_tabview_add_tab(seq_tv, "SONG"));
    populateSubPage(lv_tabview_add_tab(seq_tv, "SETUP"), MODE_SEQ, PAGE_OSC);

    lv_obj_t *sys_tv = createStyledSubTabview(tab_system, 6);
    g_sysTv = sys_tv;
    populateSubPage(lv_tabview_add_tab(sys_tv, "SYNTH"), MODE_SYSTEM, PAGE_SYS_MIDI);
    populateSubPage(lv_tabview_add_tab(sys_tv, "FX"),   MODE_SYSTEM, PAGE_FX);
    populateSubPage(lv_tabview_add_tab(sys_tv, "FILE"), MODE_SYSTEM, PAGE_SYS_FILE);
    populateSubPage(lv_tabview_add_tab(sys_tv, "DISP"), MODE_SYSTEM, PAGE_SYS_DISPLAY);

    static lv_obj_t *drumPadModal = lv_obj_create(lv_scr_act());
    lv_obj_set_size(drumPadModal, 256, 240);
    lv_obj_align(drumPadModal, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(drumPadModal, lv_color_hex(0x24242A), 0);
    lv_obj_set_style_pad_all(drumPadModal, 0, 0);
    lv_obj_set_style_border_width(drumPadModal, 0, 0);
    lv_obj_clear_flag(drumPadModal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(drumPadModal, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *padBack = lv_btn_create(drumPadModal);
    lv_obj_set_size(padBack, 76, 26);
    lv_obj_set_pos(padBack, 2, 2);
    lv_obj_set_style_bg_color(padBack, lv_color_hex(0xF83800), 0);
    lv_obj_t *pbLbl = lv_label_create(padBack);
    lv_label_set_text(pbLbl, "< BACK");
    lv_obj_center(pbLbl);
    lv_obj_add_event_cb(padBack, [](lv_event_t *e) {
        lv_obj_add_flag((lv_obj_t*)lv_event_get_user_data(e), LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, drumPadModal);

    lv_obj_t *padTitle = lv_label_create(drumPadModal);
    lv_label_set_text(padTitle, "DRUM PADS");
    lv_obj_set_style_text_color(padTitle, lv_color_hex(0xFF5100), 0);
    lv_obj_set_pos(padTitle, 95, 7);

    lv_obj_t *padGrid = lv_obj_create(drumPadModal);
    lv_obj_set_size(padGrid, 256, 210);
    lv_obj_set_pos(padGrid, 0, 30);
    lv_obj_set_style_bg_color(padGrid, lv_color_hex(0xC8C6BE), 0);
    lv_obj_set_style_pad_all(padGrid, 2, 0);
    lv_obj_set_style_border_width(padGrid, 0, 0);
    lv_obj_clear_flag(padGrid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(padGrid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_row(padGrid, 2, 0);
    lv_obj_set_style_pad_column(padGrid, 2, 0);

    for (int d = 0; d < 9; d++) {
        lv_obj_t *pad = lv_obj_create(padGrid);
        lv_obj_clear_flag(pad, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(pad, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(pad, 82, 67);
        lv_obj_set_style_radius(pad, 6, 0);
        lv_obj_set_style_bg_color(pad, lv_color_hex(0x2B2B32), 0);
        lv_obj_set_style_bg_color(pad, lv_color_hex(0xFF5100), LV_STATE_PRESSED);
        lv_obj_set_style_border_color(pad, lv_color_hex(0x73A456), 0);
        lv_obj_set_style_border_width(pad, 2, 0);
        lv_obj_t *pl = lv_label_create(pad);
        char *pTxt = (char*)alloca(24);
        snprintf(pTxt, 24, "PAD %d\nDrum%d", d + 1, d + 1);
        lv_label_set_text(pl, pTxt);
        lv_obj_set_style_text_align(pl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(pl);
        lv_obj_add_event_cb(pad, [](lv_event_t *e) {
            int drumIdx = (int)(intptr_t)lv_event_get_user_data(e);
            triggerDrum(drumIdx, 1.0f);
        }, LV_EVENT_PRESSED, (void*)(intptr_t)d);
    }

    buildSeqContextClearModal(lv_scr_act());
    buildPatchRenameModal(lv_scr_act());
    buildPatchImportModal(lv_scr_act());
    buildProjImportModal(lv_scr_act());
    buildFileDeleteModal(lv_scr_act());
    static uint32_t drumPadOpenedAt = 0;
    static bool mainTabLongHandled = false;
    lv_obj_add_event_cb(main_btns, [](lv_event_t *e) {
        static uint32_t lastTabTap = 0;
        static uint16_t lastTabBtn = 255;
        lv_event_code_t code = lv_event_get_code(e);
        lv_obj_t *btns = lv_event_get_target(e);
        uint16_t sel = lv_btnmatrix_get_selected_btn(btns);
        if (code == LV_EVENT_PRESSED) { mainTabLongHandled = false; return; }
        else if (code == LV_EVENT_LONG_PRESSED) {
            mainTabLongHandled = true; lastTabTap = 0;
            if (sel == 0 && synthParts[activePartIdx].engine != ENG_OFF) openSynthResetModal();
            else if (sel == 1) openSeqContextClearModal(g_seqTv && lv_tabview_get_tab_act(g_seqTv) == 1);
            return;
        } else if (code != LV_EVENT_CLICKED || mainTabLongHandled) return;
        uint32_t now = millis();
        int mDbl = findMatrixIdxBySysId(SYS_PARAM_DBLCLICK);
        uint32_t thr = (mDbl >= 0) ? (uint32_t)(380 - (int)paramMatrix[mDbl].currentVal * 80) : 380;
        if (sel == lastTabBtn && (now - lastTabTap) < thr) {
            lastTabTap = 0;
            if (sel == 0 && synthParts[activePartIdx].engine == ENG_DRUMS) {
                lv_obj_t *modal = (lv_obj_t*)lv_event_get_user_data(e);
                drumPadOpenedAt = now;
                lv_obj_clear_flag(modal, LV_OBJ_FLAG_HIDDEN);
                lv_obj_move_foreground(modal);
            } else if (sel == 1) onSeqTogglePlayFromGrid();
        } else { lastTabBtn = sel; lastTabTap = now; }
    }, LV_EVENT_ALL, drumPadModal);

    lv_obj_add_event_cb(main_tv, [](lv_event_t *e) {
        if (millis() - drumPadOpenedAt > 400) {
            lv_obj_add_flag((lv_obj_t*)lv_event_get_user_data(e), LV_OBJ_FLAG_HIDDEN);
        }
    }, LV_EVENT_VALUE_CHANGED, drumPadModal);

    g_oscBar = lv_bar_create(lv_scr_act());
    lv_obj_set_size(g_oscBar, 56, 46);
    lv_obj_align(g_oscBar, LV_ALIGN_BOTTOM_LEFT, 4, -6);
    lv_bar_set_range(g_oscBar, 0, 120);
    lv_obj_set_style_radius(g_oscBar, 5, 0);
    lv_obj_set_style_radius(g_oscBar, 4, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(g_oscBar, lv_color_hex(0x2B2B32), 0);
    lv_obj_set_style_bg_opa(g_oscBar, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_opa(g_oscBar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_border_color(g_oscBar, lv_color_hex(0x5783AB), 0);
    lv_obj_set_style_border_width(g_oscBar, 4, 0);
    lv_obj_clear_flag(g_oscBar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_oscBar, LV_OBJ_FLAG_CLICKABLE);
    g_oscLbl = lv_label_create(g_oscBar);
    lv_obj_set_style_text_color(g_oscLbl, lv_color_white(), 0);
    lv_obj_set_style_text_align(g_oscLbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(g_oscLbl);

    static lv_obj_t *sysStatsModal = lv_obj_create(lv_scr_act());
    lv_obj_set_size(sysStatsModal, 248, 226);
    lv_obj_center(sysStatsModal);
    lv_obj_set_style_bg_color(sysStatsModal, lv_color_hex(0x24242A), 0);
    lv_obj_set_style_border_color(sysStatsModal, lv_color_hex(0xFF5100), 0);
    lv_obj_set_style_border_width(sysStatsModal, 2, 0);
    lv_obj_set_style_pad_all(sysStatsModal, 8, 0);
    lv_obj_clear_flag(sysStatsModal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(sysStatsModal, LV_OBJ_FLAG_HIDDEN);

    static lv_obj_t *sysStatsTitle = lv_label_create(sysStatsModal);
    lv_label_set_text(sysStatsTitle, "SYSTEM STATISTICS");
    lv_obj_set_style_text_color(sysStatsTitle, lv_color_hex(0xFF5100), 0);
    lv_obj_align(sysStatsTitle, LV_ALIGN_TOP_LEFT, 0, 2);

    lv_obj_t *btnCloseStats = lv_btn_create(sysStatsModal);
    lv_obj_set_size(btnCloseStats, 54, 24);
    lv_obj_align(btnCloseStats, LV_ALIGN_TOP_RIGHT, 0, -2);
    lv_obj_set_style_bg_color(btnCloseStats, lv_color_hex(0xF83800), 0);
    lv_obj_t *lblCloseStats = lv_label_create(btnCloseStats);
    lv_label_set_text(lblCloseStats, "CLOSE");
    lv_obj_center(lblCloseStats);
    lv_obj_add_event_cb(btnCloseStats, [](lv_event_t *e) {
        lv_obj_add_flag((lv_obj_t*)lv_event_get_user_data(e), LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, sysStatsModal);

    static lv_obj_t *sysStatsBody = lv_label_create(sysStatsModal);
    lv_obj_set_style_text_color(sysStatsBody, lv_color_white(), 0);
    lv_obj_align(sysStatsBody, LV_ALIGN_TOP_LEFT, 0, 30);

    auto refreshSysStats = [](lv_timer_t *tmr) {
        if (lv_obj_has_flag(sysStatsModal, LV_OBJ_FLAG_HIDDEN)) return;
        int activeSynths = 0, activeVoices = 0;
        for (int i = 0; i < 16; i++) {
            SynthPartConfig *sp = synthParts + i;
            if (sp->engine != ENG_OFF && sp->voices > 0) {
                activeSynths++;
                activeVoices += sp->voices;
            }
        }
        uint32_t upSec = millis() / 1000;
        char *buf = (char*)alloca(320);
        snprintf(buf, 320,
            "CPU: %u MHz (Dual Core)\n"
            "Internal RAM: %u / %u KB\n"
            "Min Free Heap: %u KB\n"
            "PSRAM Free: %u / %u KB\n"
            "AMY OSCs: %d / 120 (%d voices)\n"
            "Active Synths: %d / 16\n"
            "AMY Queue Free: %u / 64\n"
            "Storage: %s\n"
            "Uptime: %02u:%02u:%02u",
            ESP.getCpuFreqMHz(),
            ESP.getFreeHeap() / 1024, ESP.getHeapSize() / 1024,
            ESP.getMinFreeHeap() / 1024,
            ESP.getFreePsram() / 1024, ESP.getPsramSize() / 1024,
            calcTotalUsedOscs(), activeVoices,
            activeSynths,
            (unsigned int)amyGetQueueFree(),
            usingPhysicalSD ? "SD CARD (FAT32)" : "INTERNAL FLASH (NVS)",
            upSec / 3600, (upSec % 3600) / 60, upSec % 60);
        lv_label_set_text(sysStatsBody, buf);
    };
    lv_timer_create(refreshSysStats, 500, nullptr);

    lv_obj_add_event_cb(g_oscBar, [](lv_event_t *e) {
        static uint32_t lastOscTap = 0;
        static bool oscLongHandled = false;
        lv_event_code_t code = lv_event_get_code(e);
        if (code == LV_EVENT_PRESSED) { oscLongHandled = false; return; }
        if (code == LV_EVENT_LONG_PRESSED) {
            oscLongHandled = true;
            lastOscTap = 0;
            openHelpForSection(7, 0);
            return;
        }
        if (code != LV_EVENT_CLICKED || oscLongHandled) return;
        uint32_t now = millis();
        if (now - lastOscTap < 380) {
            lastOscTap = 0;
            lv_obj_clear_flag(sysStatsModal, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(sysStatsModal);
        } else {
            lastOscTap = now;
        }
    }, LV_EVENT_ALL, nullptr);

    buildOpDetailModal(lv_scr_act());
    buildStdValModal(lv_scr_act());
    buildContextHelpModal(lv_scr_act());
    int mPartInit = findMatrixIdxBySysId(SYS_PARAM_PART_SEL);
    if (mPartInit >= 0) activePartIdx = constrain((int)paramMatrix[mPartInit].currentVal - 1, 0, 15);
    loadPartIntoMatrix(activePartIdx);
    lv_timer_create([](lv_timer_t *t) {
        static int lastShownStep = -2;
        static int lastShownSong = -2;
        static int lastTabMode = -1;
        int curTabMode = (seqSongModeActive ? 2 : 0) + (seqIsPlaying ? 1 : 0);
        if (curTabMode != lastTabMode && g_mainTv) {
            lastTabMode = curTabMode;
            if (seqSongModeActive && seqIsPlaying) lv_tabview_rename_tab(g_mainTv, 1, "Song >");
            else if (seqSongModeActive) lv_tabview_rename_tab(g_mainTv, 1, "Song");
            else if (seqIsPlaying) lv_tabview_rename_tab(g_mainTv, 1, "Seq >");
            else lv_tabview_rename_tab(g_mainTv, 1, "Seq");
        }
        static int lastShownPat = -1;
        if (seqPlayheadStep != lastShownStep || seqSongPos != lastShownSong || currentSeqPattern != lastShownPat) {
            lastShownStep = seqPlayheadStep;
            lastShownSong = seqSongPos;
            lastShownPat = currentSeqPattern;
            refreshSeqGridUI();
        }
    }, 12, nullptr);

    int mFlipInit = findMatrixIdxBySysId(SYS_PARAM_FLIP);
    if (mFlipInit >= 0) updateSystemHardware(SYS_PARAM_FLIP, paramMatrix[mFlipInit].currentVal);
}


inline void updateMoodLed() {
    static uint32_t lastLedTick = 0, lastPacked = 0xFFFFFFFFu;
    uint32_t now = millis(); if (now - lastLedTick < 25) return; lastLedTick = now;
    int li = findMatrixIdxBySysId(SYS_PARAM_LED_BRIGHT);
    int lvl = (li >= 0) ? constrain((int)paramMatrix[li].currentVal, 0, 20) : 12;
    uint8_t br = 220, bg = 220, bb = 230;
    if ((g_mainTv ? lv_tabview_get_tab_act(g_mainTv) : 0) != 2) {
        uint8_t eng = (synthParts + activePartIdx)->engine;
        if (eng == ENG_JUNO) { br = 0; bg = 230; bb = 45; }
        else if (eng == ENG_DX7) { br = 255; bg = 80; bb = 0; }
        else if (eng == ENG_DRUMS) { br = 0; bg = 200; bb = 255; }
        else if (eng == ENG_PIANO) { br = 250; bg = 200; bb = 0; }
        else if (eng == ENG_SAMPLER) { br = 190; bg = 40; bb = 255; }
    }
    uint32_t dt = now - g_moodPulseMs;
    int pulse = (dt < 240) ? (255 - (int)((dt * 235) / 240)) : 20;
    int scale = (pulse * lvl) / 20;
    uint8_t r = (uint8_t)((br * scale) >> 8), g = (uint8_t)((bg * scale) >> 8), b = (uint8_t)((bb * scale) >> 8);
    uint32_t packed = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    if (packed != lastPacked) { lastPacked = packed; rgbLedWrite(42, r, g, b); }
}
