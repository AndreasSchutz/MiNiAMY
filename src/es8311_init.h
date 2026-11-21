#pragma once
#include <Arduino.h>
#include <Wire.h>

#define ES8311_ADDR 0x18
#define ES8311_PA_PIN 1

inline void es8311Write(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(ES8311_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

inline void es8311SetMasterStep(int step) {
    int s = constrain(step, 0, 10);
    uint8_t regVal = (s == 0) ? 0x00 : (uint8_t)(0xB0 + ((s - 1) * 0x3E) / 9);
    es8311Write(0x32, regVal);
}

inline void es8311SetMasterPct(float pct) {
    es8311SetMasterStep((int)roundf(constrain(pct, 0.0f, 100.0f) / 10.0f));
}

inline bool initEs8311Audio() {
    pinMode(ES8311_PA_PIN, OUTPUT);
    digitalWrite(ES8311_PA_PIN, HIGH);
    es8311Write(0x00, 0x1F); delay(20);
    es8311Write(0x45, 0x00);
    es8311Write(0x01, 0x30); es8311Write(0x02, 0x00);
    es8311Write(0x03, 0x10); es8311Write(0x04, 0x10);
    es8311Write(0x05, 0x00); es8311Write(0x09, 0x0C);
    es8311Write(0x0A, 0x0C); es8311Write(0x0B, 0x00);
    es8311Write(0x0C, 0x00); es8311Write(0x10, 0x1F);
    es8311Write(0x11, 0x7F); es8311Write(0x00, 0x80);
    es8311Write(0x01, 0x3F); es8311Write(0x0D, 0x01);
    es8311Write(0x0E, 0x02); es8311Write(0x12, 0x00);
    es8311Write(0x13, 0x10); es8311Write(0x1C, 0x6A);
    es8311Write(0x37, 0x08); es8311SetMasterStep(8);
    delay(20);
    digitalWrite(ES8311_PA_PIN, LOW);
    Serial.println("[Audio] ES8311 0-10 Master Vol redo!");
    return true;
}
