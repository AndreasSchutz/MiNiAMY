#include <Arduino.h>
#include <lvgl.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include <bb_captouch.h>
#include "synth_gui.h"

#define TFT_BL    45
#define LCD_CS    10
#define LCD_DC    46
#define LCD_SCK   12
#define LCD_MOSI  11
#define LCD_MISO  13
#define TOUCH_SDA 16
#define TOUCH_SCL 15
#define TOUCH_INT 17
#define TOUCH_RST 18
#define UI_WIDTH  320
#define UI_HEIGHT 240

Arduino_DataBus *bus = nullptr;
Arduino_GFX *gfx = nullptr;
BBCapTouch bbct;
bool g_touchReady = false;
bool g_flipScreen = false;
static uint32_t g_lastTouchMs = 0;
static bool g_isScreenDimmed = false;
static uint8_t g_activeBrightnessPct = 85;
static uint8_t g_dimTimeoutMin = 2;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = nullptr;

inline void processRawMidiByte(uint8_t b) {
    if (b >= 0xF8) {
        if (b == 0xFA || b == 0xFB) { seqExtClockActive = true; seqExtClockTicks = 0; seqExtStepPulse = true; seqIsPlaying = true; refreshSeqGridUI(); }
        else if (b == 0xFC) { seqIsPlaying = false; seqExtClockActive = false; seqPlayheadStep = -1; refreshSeqGridUI(); }
        else if (b == 0xF8 && seqIsPlaying && seqExtClockActive) { if (seqExtClockTicks == 0) seqExtStepPulse = true; seqExtClockTicks = (seqExtClockTicks + 1) % 6; }
        return;
    }
    static uint8_t runStatus = 0, d1 = 0, dCount = 0;
    if (b & 0x80) { if (b < 0xF0) { runStatus = b; dCount = 0; } return; }
    if (runStatus == 0) return;
    uint8_t cmd = runStatus & 0xF0, ch = (runStatus & 0x0F) + 1;
    if (cmd == 0xC0 || cmd == 0xD0) return;
    if (dCount == 0) { d1 = b; dCount = 1; }
    else { dCount = 0; if (cmd == 0x90) onMidiNoteReceived(ch, d1, b); else if (cmd == 0x80) onMidiNoteReceived(ch, d1, 0); else if (cmd == 0xB0) onMidiCCReceived(d1, b); }
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void create_synth_gui();
void updateSystemHardware(int systemParamId, float value);
float mapValue(float x, float in_min, float in_max, float out_min, float out_max);

void uiTask(void *pvParameters) {
    initStorageSubsystem();
    bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI, LCD_MISO);
    gfx = new Arduino_ILI9341(bus, -1, 1, true);
    gfx->begin(40000000);
    bus->beginWrite();
    bus->writeCommand(0xF2); bus->write(0x02);
    bus->writeCommand(0x26); bus->write(0x01);
    bus->writeCommand(0xE0);
    { uint8_t pg[15]={0x0F,0x2A,0x26,0x0B,0x0E,0x08,0x4E,0xF6,0x3A,0x09,0x14,0x05,0x16,0x12,0x00}; bus->writeBytes(pg,15); }
    bus->writeCommand(0xE1);
    { uint8_t ng[15]={0x00,0x15,0x19,0x04,0x11,0x07,0x31,0x39,0x45,0x06,0x0B,0x0A,0x29,0x2D,0x0F}; bus->writeBytes(ng,15); }
    bus->endWrite();
    gfx->fillScreen(0x0000);
    ledcAttach(TFT_BL, 1000, 8);
    ledcWrite(TFT_BL, 220);
    int ctRes = bbct.init(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_INT);
    g_touchReady = (ctRes == CT_SUCCESS);
    Serial.printf("[Touch] FT6336G init: %d (type=%d)\n", ctRes, bbct.sensorType());
    Wire.begin(TOUCH_SDA, TOUCH_SCL, 400000);
    initEs8311Audio();
    for (uint8_t a = 1; a < 127; a++) { Wire.beginTransmission(a); if (Wire.endTransmission() == 0) Serial.printf("[I2C] 0x%02X\n", a); }
    lv_init();
    buf1 = (lv_color_t *)ps_malloc(UI_WIDTH * 60 * sizeof(lv_color_t));
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, UI_WIDTH * 60);
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = UI_WIDTH; disp_drv.ver_res = UI_HEIGHT;
    disp_drv.flush_cb = my_disp_flush; disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER; indev_drv.read_cb = my_touchpad_read; indev_drv.long_press_time = 800;
    lv_indev_drv_register(&indev_drv);
    initDx7OperatorParams();
    initPerSynthRam();
    pollStorageHotSwap();
    loadStateFromNvs(paramMatrix, TOTAL_PARAMS, synthParts, synthParamRam, (uint8_t*)seqPatterns, sizeof(seqPatterns), seqSongChain, sizeof(seqSongChain)); if (activeStorageFS && activeStorageFS->exists("/AMY/seqvel.bin") && seqDrumVel && seqStepVel) { File vf = activeStorageFS->open("/AMY/seqvel.bin", FILE_READ); if (vf && vf.size() == SEQ_PATTERNS * SEQ_TRACKS * SEQ_STEPS * 10) { vf.read(seqDrumVel, SEQ_PATTERNS * SEQ_TRACKS * SEQ_STEPS * 9); vf.read(seqStepVel, SEQ_PATTERNS * SEQ_TRACKS * SEQ_STEPS); } if (vf) vf.close(); }
    delay(100);
    for (int i = 0; i < 16; i++) amyEnqueuePartConfig((uint8_t)i, synthParts[i].engine, synthParts[i].voices, synthParts[i].patchOrNote, synthParts[i].level);
    create_synth_gui();
    g_lastTouchMs = millis();
    int mBr = findMatrixIdxBySysId(SYS_PARAM_BRIGHTNESS);
    if (mBr >= 0) updateSystemHardware(SYS_PARAM_BRIGHTNESS, (paramMatrix + mBr)->currentVal);
    int mVol = findMatrixIdxBySysId(SYS_PARAM_MASTER_VOL);
    if (mVol >= 0) {
        if ((paramMatrix + mVol)->currentVal > 0.0f && (paramMatrix + mVol)->currentVal <= 1.0f) {
            (paramMatrix + mVol)->currentVal = roundf((paramMatrix + mVol)->currentVal * 10.0f);
        }
        amyEnqueueMasterVol((paramMatrix + mVol)->currentVal / 10.0f);
    }
    int mDim = findMatrixIdxBySysId(SYS_PARAM_DIM_TIMEOUT);
    if (mDim >= 0) updateSystemHardware(SYS_PARAM_DIM_TIMEOUT, (paramMatrix + mDim)->currentVal);
    for (;;) {
        lv_tick_inc(5); lv_timer_handler();
        uint32_t dimMs = (g_dimTimeoutMin == 1) ? 30000UL : ((uint32_t)(g_dimTimeoutMin - 1) * 60000UL);
        if (g_dimTimeoutMin > 0 && !g_isScreenDimmed && (millis() - g_lastTouchMs) >= dimMs) { g_isScreenDimmed = true; ledcWrite(TFT_BL, 46); }
        if (Serial.available()) {
            String cmd = Serial.readStringUntil('\n'); cmd.trim(); int a, b, c;
            if (sscanf(cmd.c_str(), "cc %d %d", &a, &b) == 2) onMidiCCReceived((uint8_t)a, (uint8_t)b);
            else if (sscanf(cmd.c_str(), "note %d %d %d", &a, &b, &c) == 3) onMidiNoteReceived((uint8_t)a, (uint8_t)b, (uint8_t)c);
        }
        if (!seqIsPlaying) {
            if (pollStorageHotSwap()) { int mS = findMatrixIdxBySysId(SYS_PARAM_PROJ_SAVE), mL = findMatrixIdxBySysId(SYS_PARAM_PROJ_LOAD); if (mS >= 0) refreshParamBox(mS); if (mL >= 0) refreshParamBox(mL); }
            if (storageDirty && (millis() - storageLastChangeMs) > 2500) { saveActivePartParamsToRam(); saveStateToNvs(paramMatrix, TOTAL_PARAMS, synthParts, synthParamRam, (uint8_t*)seqPatterns, sizeof(seqPatterns), seqSongChain, sizeof(seqSongChain)); if (activeStorageFS && seqDrumVel && seqStepVel) { File vf = activeStorageFS->open("/AMY/seqvel.bin", FILE_WRITE); if (vf) { vf.write(seqDrumVel, SEQ_PATTERNS * SEQ_TRACKS * SEQ_STEPS * 9); vf.write(seqStepVel, SEQ_PATTERNS * SEQ_TRACKS * SEQ_STEPS); vf.close(); } } }
        }
        if (fileFeedbackId != -1 && millis() >= fileFeedbackUntil) { int fi = findMatrixIdxBySysId(fileFeedbackId); fileFeedbackId = -1; if (fi >= 0) refreshParamBox(fi); }
        updateMoodLed();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
void setup() { Serial.begin(115200); delay(1500); startAmyBridge(); xTaskCreatePinnedToCore(uiTask, "UI_Task", 16384, nullptr, 1, nullptr, 0); }
void loop() { vTaskDelete(nullptr); }
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) { gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)color_p, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1); lv_disp_flush_ready(disp); }
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    TOUCHINFO ti;
    if (g_touchReady && bbct.getSamples(&ti) && ti.count > 0) {
        int tx = *ti.x, ty = *ti.y;
        int x = g_flipScreen ? ((UI_WIDTH - 1) - ty) : ty, y = g_flipScreen ? tx : ((UI_HEIGHT - 1) - tx);
        g_lastTouchMs = millis();
        if (g_isScreenDimmed) { g_isScreenDimmed = false; ledcWrite(TFT_BL, (uint32_t)mapValue((float)g_activeBrightnessPct, 0, 100, 0, 255)); data->state = LV_INDEV_STATE_REL; }
        else { data->state = LV_INDEV_STATE_PR; data->point.x = constrain(x, 0, UI_WIDTH - 1); data->point.y = constrain(y, 0, UI_HEIGHT - 1); }
    } else data->state = LV_INDEV_STATE_REL;
}
void updateSystemHardware(int id, float v) {
    if (id == SYS_PARAM_BRIGHTNESS) { g_activeBrightnessPct = (uint8_t)constrain((int)v, 25, 100); g_isScreenDimmed = false; g_lastTouchMs = millis(); ledcWrite(TFT_BL, (uint32_t)mapValue((float)g_activeBrightnessPct, 0, 100, 0, 255)); }
    else if (id == SYS_PARAM_DIM_TIMEOUT) { g_dimTimeoutMin = (uint8_t)constrain((int)v, 0, 10); g_lastTouchMs = millis(); if (g_dimTimeoutMin == 0 && g_isScreenDimmed) { g_isScreenDimmed = false; ledcWrite(TFT_BL, (uint32_t)mapValue((float)g_activeBrightnessPct, 0, 100, 0, 255)); } }
    else if (id == SYS_PARAM_FLIP) { g_flipScreen = (v >= 0.5f); if (gfx) gfx->setRotation(g_flipScreen ? 3 : 1); lv_obj_invalidate(lv_scr_act()); }
}
float mapValue(float x, float a, float b, float c, float d) { return (x - a) * (d - c) / (b - a) + c; }
