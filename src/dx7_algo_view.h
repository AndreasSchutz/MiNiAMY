#pragma once
#include <Arduino.h>
#include <lvgl.h>

struct Dx7AlgoDef {
    uint8_t pos[6];
    uint8_t carriers;
    uint8_t fb_op;
    uint8_t links[6];
};

static const Dx7AlgoDef dx7Algos[32] = {
    {{0x30, 0x20, 0x32, 0x22, 0x12, 0x02}, 0x05, 5, {0x10, 0x32, 0x43, 0x54, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x32, 0x22, 0x12, 0x02}, 0x05, 1, {0x10, 0x32, 0x43, 0x54, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x10, 0x32, 0x22, 0x12}, 0x09, 5, {0x10, 0x21, 0x43, 0x54, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x10, 0x32, 0x22, 0x12}, 0x09, 5, {0x10, 0x21, 0x43, 0x54, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x31, 0x21, 0x32, 0x22}, 0x15, 5, {0x10, 0x32, 0x54, 0xFF, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x31, 0x21, 0x32, 0x22}, 0x15, 5, {0x10, 0x32, 0x54, 0xFF, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x32, 0x21, 0x23, 0x13}, 0x05, 5, {0x10, 0x32, 0x42, 0x54, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x32, 0x21, 0x23, 0x13}, 0x05, 3, {0x10, 0x32, 0x42, 0x54, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x32, 0x21, 0x23, 0x13}, 0x05, 1, {0x10, 0x32, 0x42, 0x54, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x10, 0x32, 0x21, 0x23}, 0x09, 2, {0x10, 0x21, 0x43, 0x53, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x10, 0x32, 0x21, 0x23}, 0x09, 5, {0x10, 0x21, 0x43, 0x53, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x32, 0x21, 0x22, 0x23}, 0x05, 1, {0x10, 0x32, 0x42, 0x52, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x32, 0x21, 0x22, 0x23}, 0x05, 5, {0x10, 0x32, 0x42, 0x52, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x32, 0x22, 0x11, 0x13}, 0x05, 5, {0x10, 0x32, 0x43, 0x53, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x32, 0x22, 0x11, 0x13}, 0x05, 1, {0x10, 0x32, 0x43, 0x53, 0xFF, 0xFF}},
    {{0x31, 0x20, 0x21, 0x11, 0x22, 0x12}, 0x01, 5, {0x10, 0x20, 0x32, 0x40, 0x54, 0xFF}},
    {{0x31, 0x20, 0x21, 0x11, 0x22, 0x12}, 0x01, 1, {0x10, 0x20, 0x32, 0x40, 0x54, 0xFF}},
    {{0x31, 0x20, 0x21, 0x22, 0x12, 0x02}, 0x01, 2, {0x10, 0x20, 0x30, 0x43, 0x54, 0xFF}},
    {{0x30, 0x20, 0x10, 0x31, 0x32, 0x22}, 0x19, 5, {0x10, 0x21, 0x53, 0x54, 0xFF, 0xFF}},
    {{0x30, 0x31, 0x20, 0x32, 0x22, 0x23}, 0x0B, 2, {0x20, 0x21, 0x43, 0x53, 0xFF, 0xFF}},
    {{0x30, 0x31, 0x20, 0x32, 0x33, 0x22}, 0x1B, 2, {0x20, 0x21, 0x53, 0x54, 0xFF, 0xFF}},
    {{0x30, 0x20, 0x31, 0x32, 0x33, 0x22}, 0x1D, 0x05, {0x10, 0x52, 0x53, 0x54, 0xFF, 0xFF}},
    {{0x30, 0x31, 0x21, 0x32, 0x33, 0x22}, 0x1B, 5, {0x21, 0x53, 0x54, 0xFF, 0xFF, 0xFF}},
    {{0x30, 0x31, 0x32, 0x33, 0x33, 0x22}, 0x1F, 5, {0x52, 0x53, 0x54, 0xFF, 0xFF, 0xFF}},
    {{0x30, 0x31, 0x32, 0x33, 0x33, 0x22}, 0x1F, 5, {0x53, 0x54, 0xFF, 0xFF, 0xFF, 0xFF}},
    {{0x30, 0x31, 0x21, 0x32, 0x22, 0x23}, 0x0B, 5, {0x21, 0x43, 0x53, 0xFF, 0xFF, 0xFF}},
    {{0x30, 0x31, 0x21, 0x32, 0x22, 0x23}, 0x0B, 2, {0x21, 0x43, 0x53, 0xFF, 0xFF, 0xFF}},
    {{0x30, 0x31, 0x21, 0x32, 0x22, 0x12}, 0x0B, 2, {0x21, 0x43, 0x54, 0xFF, 0xFF, 0xFF}},
    {{0x30, 0x31, 0x32, 0x22, 0x33, 0x23}, 0x17, 5, {0x32, 0x54, 0xFF, 0xFF, 0xFF, 0xFF}},
    {{0x30, 0x31, 0x32, 0x33, 0x23, 0x13}, 0x0F, 5, {0x43, 0x54, 0xFF, 0xFF, 0xFF, 0xFF}},
    {{0x30, 0x31, 0x32, 0x33, 0x33, 0x23}, 0x1F, 5, {0x54, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
    {{0x30, 0x30, 0x31, 0x31, 0x32, 0x33}, 0x3F, 5, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}
};

static lv_obj_t *algoCanvas = nullptr;
static lv_obj_t *opBoxes[6] = {nullptr};
static lv_obj_t *opLabels[6] = {nullptr};
static lv_obj_t *algoLines[6] = {nullptr};
static lv_point_t algoLinePts[6][2];
static lv_obj_t *algoTitleLbl = nullptr;
static lv_obj_t *algoFbLoopLine = nullptr;
static lv_point_t algoFbLoopPts[4];

inline void updateDx7AlgoDiagram(int algoNum, uint8_t mappedCC = 255, bool isLearning = false) {
    if (!algoCanvas) return;
    int idx = constrain(algoNum, 1, 32) - 1;
    const Dx7AlgoDef &def = *(dx7Algos + idx);

    int32_t permille = constrain((int32_t)(((algoNum - 1) * 1000) / 31), 0, 1000);
    lv_bar_set_value(algoCanvas, permille, LV_ANIM_OFF);

    char *tbuf = (char*)alloca(48);
    const char *fbTag = (algoNum == 4) ? "OP4-6 FB" : ((algoNum == 6) ? "OP5-6 FB" : nullptr);
    char *fbBuf = (char*)alloca(12);
    if (!fbTag) { snprintf(fbBuf, 12, "OP%d FB", def.fb_op + 1); fbTag = fbBuf; }
    if (isLearning) {
        lv_obj_set_style_bg_color(algoCanvas, lv_color_hex(0x3A1414), 0);
        lv_obj_set_style_border_color(algoCanvas, lv_color_hex(0xFFF8E7), 0);
        lv_obj_set_style_border_width(algoCanvas, 2, 0);
        lv_obj_set_style_bg_color(algoCanvas, lv_color_hex(0x8A2B2B), LV_PART_INDICATOR);
        snprintf(tbuf, 48, "ALGO #%d (%s) | \x5bMIDI LEARN\x5d", algoNum, fbTag);
    } else if (mappedCC != 255) {
        lv_obj_set_style_bg_color(algoCanvas, lv_color_hex(0x222228), 0);
        lv_obj_set_style_border_color(algoCanvas, lv_color_hex(0xFF5100), 0);
        lv_obj_set_style_border_width(algoCanvas, 2, 0);
        lv_obj_set_style_bg_color(algoCanvas, lv_color_hex(0xA65A2A), LV_PART_INDICATOR);
        snprintf(tbuf, 48, "ALGO #%d (%s) | CC #%u", algoNum, fbTag, mappedCC);
    } else {
        lv_obj_set_style_bg_color(algoCanvas, lv_color_hex(0x222228), 0);
        lv_obj_set_style_border_color(algoCanvas, lv_color_hex(0x5A5A64), 0);
        lv_obj_set_style_border_width(algoCanvas, 1, 0);
        lv_obj_set_style_bg_color(algoCanvas, lv_color_hex(0x8C4E27), LV_PART_INDICATOR);
        snprintf(tbuf, 48, "ALGO #%d (%s) | CC --", algoNum, fbTag);
    }
    lv_label_set_text(algoTitleLbl, tbuf);

    int cx[6], cy[6];
    int col[6], row[6];
    int rowCount[4] = {0, 0, 0, 0};
    int maxCol = 0;
    for (int op = 0; op < 6; op++) {
        int r = (*(def.pos + op) >> 4) & 0x03;
        int c = *(def.pos + op) & 0x03;
        if (idx >= 23 && r == 3) c = (*(rowCount + r))++;
        *(row + op) = r;
        *(col + op) = c;
        if (c > maxCol) maxCol = c;
    }
    int stepX = (maxCol >= 5) ? 40 : ((maxCol == 4) ? 48 : 56);
    int startX = (252 - maxCol * stepX) / 2;

    for (int op = 0; op < 6; op++) {
        *(cx + op) = startX + *(col + op) * stepX;
        *(cy + op) = 31 + *(row + op) * 29;

        lv_obj_set_pos(*(opBoxes + op), *(cx + op) - 14, *(cy + op) - 9);
        bool isCarrier = (def.carriers & (1 << op)) != 0;
        bool isFb = (def.fb_op == op);

        lv_obj_set_style_bg_color(*(opBoxes + op), isCarrier ? lv_color_hex(0xFF5500) : lv_color_hex(0x323238), 0);
        lv_obj_set_style_border_color(*(opBoxes + op), isFb ? lv_color_hex(0xFFF8E7) : lv_color_hex(0x6E6E78), 0);
        lv_obj_set_style_border_width(*(opBoxes + op), isFb ? 2 : 1, 0);
    }

    for (int l = 0; l < 6; l++) {
        uint8_t lk = *(def.links + l);
        if (lk == 0xFF) {
            lv_obj_add_flag(*(algoLines + l), LV_OBJ_FLAG_HIDDEN);
        } else {
            int fromOp = (lk >> 4) & 0x07;
            int toOp   = lk & 0x07;
            (*(algoLinePts + l) + 0)->x = *(cx + fromOp);
            (*(algoLinePts + l) + 0)->y = *(cy + fromOp) + 9;
            (*(algoLinePts + l) + 1)->x = *(cx + toOp);
            (*(algoLinePts + l) + 1)->y = *(cy + toOp) - 9;
            lv_line_set_points(*(algoLines + l), *(algoLinePts + l), 2);
            lv_obj_clear_flag(*(algoLines + l), LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (algoFbLoopLine) {
        if (algoNum == 4 || algoNum == 6) {
            int srcOp = (algoNum == 4) ? 3 : 4;
            lv_coord_t rx = *(cx + 5) + 22;
            (algoFbLoopPts + 0)->x = *(cx + srcOp) + 14; (algoFbLoopPts + 0)->y = *(cy + srcOp);
            (algoFbLoopPts + 1)->x = rx;                 (algoFbLoopPts + 1)->y = *(cy + srcOp);
            (algoFbLoopPts + 2)->x = rx;                 (algoFbLoopPts + 2)->y = *(cy + 5);
            (algoFbLoopPts + 3)->x = *(cx + 5) + 14;     (algoFbLoopPts + 3)->y = *(cy + 5);
            lv_line_set_points(algoFbLoopLine, algoFbLoopPts, 4);
            lv_obj_clear_flag(algoFbLoopLine, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(algoFbLoopLine, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

inline lv_obj_t* createDx7AlgoWidget(lv_obj_t *parent, lv_event_cb_t eventCb, int paramIdx) {
    algoCanvas = lv_bar_create(parent);
    lv_obj_set_size(algoCanvas, 252, 138);
    lv_bar_set_range(algoCanvas, 0, 1000);
    lv_obj_set_style_radius(algoCanvas, 6, 0);
    lv_obj_set_style_radius(algoCanvas, 5, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(algoCanvas, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_opa(algoCanvas, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(algoCanvas, 0, 0);
    lv_obj_clear_flag(algoCanvas, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(algoCanvas, LV_OBJ_FLAG_CLICKABLE);
    if (eventCb) {
        lv_obj_add_event_cb(algoCanvas, eventCb, LV_EVENT_ALL, (void*)(intptr_t)paramIdx);
    }

    algoTitleLbl = lv_label_create(algoCanvas);
    lv_obj_set_style_text_color(algoTitleLbl, lv_color_white(), 0);
    lv_obj_set_pos(algoTitleLbl, 8, 4);

    for (int l = 0; l < 6; l++) {
        *(algoLines + l) = lv_line_create(algoCanvas);
        lv_obj_set_style_line_color(*(algoLines + l), lv_color_hex(0xE8E6E0), 0);
        lv_obj_set_style_line_width(*(algoLines + l), 2, 0);
    }
    algoFbLoopLine = lv_line_create(algoCanvas);
    lv_obj_set_style_line_color(algoFbLoopLine, lv_color_hex(0xFFF8E7), 0);
    lv_obj_set_style_line_width(algoFbLoopLine, 2, 0);
    lv_obj_add_flag(algoFbLoopLine, LV_OBJ_FLAG_HIDDEN);

    for (int op = 0; op < 6; op++) {
        *(opBoxes + op) = lv_obj_create(algoCanvas);
        lv_obj_set_size(*(opBoxes + op), 28, 18);
        lv_obj_set_style_radius(*(opBoxes + op), 3, 0);
        lv_obj_set_style_pad_all(*(opBoxes + op), 0, 0);
        lv_obj_clear_flag(*(opBoxes + op), LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

        *(opLabels + op) = lv_label_create(*(opBoxes + op));
        char *num = (char*)alloca(4);
        snprintf(num, 4, "%d", op + 1);
        lv_label_set_text(*(opLabels + op), num);
        lv_obj_set_style_text_color(*(opLabels + op), lv_color_white(), 0);
        lv_obj_center(*(opLabels + op));
    }

    updateDx7AlgoDiagram(1, 255, false);
    return algoCanvas;
}
