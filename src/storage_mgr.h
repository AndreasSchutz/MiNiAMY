#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <SPI.h>
#include <SD.h>
#include <SPIFFS.h>
#include "patch_names.h"

#ifndef SD_CS_PIN
#define SD_CS_PIN   47
#define SD_MOSI_PIN 40
#define SD_SCK_PIN  38
#define SD_MISO_PIN 39
#endif

static Preferences synthPrefs;
static SPIClass sdSpi(HSPI);
inline fs::FS *activeStorageFS = nullptr;
inline bool usingPhysicalSD = false;
inline bool storageDirty = false;
inline uint32_t storageLastChangeMs = 0;
inline char synthPatchNames[16][16] = {{0}};

struct StoredParam {
    float val;
    uint8_t cc;
};

struct StoredPart {
    uint8_t engine;
    uint8_t midiChan;
    uint8_t voices;
    uint8_t patchOrNote;
    float level;
};

inline void markStorageDirty() {
    storageDirty = true;
    storageLastChangeMs = millis();
}

inline void ensureStorageDirs() {
    if (!activeStorageFS) return;
    if (!activeStorageFS->exists("/AMY")) activeStorageFS->mkdir("/AMY");
    if (!activeStorageFS->exists("/AMY/projects")) activeStorageFS->mkdir("/AMY/projects");
    if (!activeStorageFS->exists("/AMY/samples")) activeStorageFS->mkdir("/AMY/samples");
    if (!activeStorageFS->exists("/AMY/patches")) activeStorageFS->mkdir("/AMY/patches");
}

inline bool mountSdWithRetry(int attempts, uint32_t delayMs) {
    for (int i = 0; i < attempts; i++) {
        if (SD.begin(SD_CS_PIN, sdSpi, 1000000)) {
            SD.end();
            if (SD.begin(SD_CS_PIN, sdSpi, 4000000)) return true;
        }
        SD.end();
        if (i + 1 < attempts) delay(delayMs);
    }
    return false;
}

inline void initStorageSubsystem() {
    sdSpi.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    SPIFFS.begin(true);
    delay(150);
    if (mountSdWithRetry(4, 120)) {
        activeStorageFS = &SD;
        usingPhysicalSD = true;
        Serial.println("[Storage] MicroSD-kort monterat direkt vid boot!");
    } else {
        activeStorageFS = &SPIFFS;
        usingPhysicalSD = false;
        Serial.println("[Storage] SD ej FAT32/saknas - använder intern SPIFFS Flash");
    }
    ensureStorageDirs();
}

inline bool pollStorageHotSwap() {
    static uint32_t lastCheckMs = 0;
    uint32_t now = millis();
    if (now - lastCheckMs < 4000) return false;
    lastCheckMs = now;
    if (usingPhysicalSD) {
        if (SD.cardType() == CARD_NONE || !SD.exists("/")) {
            SD.end();
            activeStorageFS = &SPIFFS;
            usingPhysicalSD = false;
            ensureStorageDirs();
            Serial.println("[Storage] MicroSD uttaget - byter till SPIFFS/NVS");
            return true;
        }
    } else {
        if (SD.begin(SD_CS_PIN, sdSpi, 1000000)) {
            SD.end();
            if (SD.begin(SD_CS_PIN, sdSpi, 4000000)) {
                activeStorageFS = &SD;
                usingPhysicalSD = true;
                ensureStorageDirs();
                Serial.println("[Storage] MicroSD isatt on-the-fly - monterat!");
                return true;
            }
        }
        SD.end();
    }
    return false;
}

template<typename ParamT, typename PartT>
inline void saveStateToNvs(ParamT *matrix, int paramCount, PartT *parts, float *partRam = nullptr, uint8_t *seqData = nullptr, size_t seqBytes = 0, uint8_t *songData = nullptr, size_t songBytes = 0) {
    if (usingPhysicalSD && activeStorageFS) {
        size_t ramBytes = partRam ? (16 * paramCount * sizeof(float)) : 0;
        size_t sBytes = (seqData && seqBytes > 0) ? seqBytes : 0;
        size_t sgBytes = (songData && songBytes > 0) ? songBytes : 0;
        size_t nameBytes = sizeof(synthPatchNames);
            size_t totalBytes = sizeof(uint32_t) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes + sBytes + sgBytes + nameBytes;
        uint8_t *buf = (uint8_t*)ps_malloc(totalBytes);
        if (buf) {
            uint32_t magic = partRam ? 0x53594E32 : 0x53594E31;
            memcpy(buf, &magic, sizeof(magic));
            StoredParam *sp = (StoredParam*)(buf + sizeof(magic));
            for (int i = 0; i < paramCount; i++) {
                (sp + i)->val = (matrix + i)->currentVal;
                (sp + i)->cc = (matrix + i)->mappedCC;
            }
            StoredPart *pt = (StoredPart*)(buf + sizeof(magic) + paramCount * sizeof(StoredParam));
            for (int i = 0; i < 16; i++) {
                (pt + i)->engine = (parts + i)->engine;
                (pt + i)->midiChan = (parts + i)->midiChan;
                (pt + i)->voices = (parts + i)->voices;
                (pt + i)->patchOrNote = (parts + i)->patchOrNote;
                (pt + i)->level = (parts + i)->level;
            }
            if (partRam) memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart), partRam, ramBytes);
    if (sBytes > 0) memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes, seqData, sBytes);
    if (sgBytes > 0) memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes + sBytes, songData, sgBytes);
    memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes + sBytes + sgBytes, synthPatchNames, nameBytes);
    if (sBytes > 0) memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes, seqData, sBytes);
            if (sgBytes > 0) memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes + sBytes, songData, sgBytes);
            memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes + sBytes + sgBytes, synthPatchNames, nameBytes);
    if (sgBytes > 0) memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes + sBytes, songData, sgBytes);
    if (sBytes > 0) memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes, seqData, sBytes);
            if (sgBytes > 0) memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes + sBytes, songData, sgBytes);
            if (sBytes > 0) memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes, seqData, sBytes);
            if (activeStorageFS->exists("/AMY/autosave.bin")) activeStorageFS->remove("/AMY/autosave.bin");
            File f = activeStorageFS->open("/AMY/autosave.bin", FILE_WRITE);
            if (f) { size_t w = f.write(buf, totalBytes); f.close(); if (w != totalBytes) { SD.end(); activeStorageFS = &SPIFFS; usingPhysicalSD = false; } }
            free(buf);
            storageDirty = false;
            Serial.println("[Storage] Auto-sparade till SD (/AMY/autosave.bin)");
            return;
        }
    }
    if (!synthPrefs.begin("amy_synth", false)) return;
    synthPrefs.putUInt("magic", 0x53594E31);
    StoredParam *pBuf = (StoredParam*)ps_malloc(paramCount * sizeof(StoredParam));
    if (pBuf) {
        for (int i = 0; i < paramCount; i++) {
            (pBuf + i)->val = (matrix + i)->currentVal;
            (pBuf + i)->cc = (matrix + i)->mappedCC;
        }
        synthPrefs.putBytes("params", pBuf, paramCount * sizeof(StoredParam));
        free(pBuf);
    }
    StoredPart *sBuf = (StoredPart*)alloca(16 * sizeof(StoredPart));
    for (int i = 0; i < 16; i++) {
        (sBuf + i)->engine = (parts + i)->engine;
        (sBuf + i)->midiChan = (parts + i)->midiChan;
        (sBuf + i)->voices = (parts + i)->voices;
        (sBuf + i)->patchOrNote = (parts + i)->patchOrNote;
        (sBuf + i)->level = (parts + i)->level;
    }
    synthPrefs.putBytes("parts", sBuf, 16 * sizeof(StoredPart));
    synthPrefs.end();
    storageDirty = false;
    Serial.println("[Storage] Sparade till NVS Flash");
}

template<typename ParamT, typename PartT>
inline bool loadStateFromNvs(ParamT *matrix, int paramCount, PartT *parts, float *partRam = nullptr, uint8_t *seqData = nullptr, size_t seqBytes = 0, uint8_t *songData = nullptr, size_t songBytes = 0) {
    if (usingPhysicalSD && activeStorageFS && activeStorageFS->exists("/AMY/autosave.bin")) {
        File f = activeStorageFS->open("/AMY/autosave.bin", FILE_READ);
        if (f) {
            size_t baseBytes = sizeof(uint32_t) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart);
            size_t ramBytes = 16 * paramCount * sizeof(float);
            size_t fSize = f.size();
            uint8_t *buf = (uint8_t*)ps_malloc(fSize);
            if (buf) {
                size_t got = f.read(buf, fSize);
                f.close();
                uint32_t magic = 0;
                if (got >= baseBytes) memcpy(&magic, buf, sizeof(magic));
                if ((got == baseBytes && magic == 0x53594E31) || ((got == baseBytes + ramBytes || got == baseBytes + ramBytes + seqBytes || got == baseBytes + ramBytes + seqBytes + songBytes || got == baseBytes + ramBytes + seqBytes + songBytes + sizeof(synthPatchNames)) && magic == 0x53594E32)) {
                    StoredParam *sp = (StoredParam*)(buf + sizeof(magic));
                    for (int i = 0; i < paramCount; i++) {
                        (matrix + i)->currentVal = constrain((sp + i)->val, (matrix + i)->minVal, (matrix + i)->maxVal);
                        (matrix + i)->mappedCC = (sp + i)->cc;
                    }
                    StoredPart *pt = (StoredPart*)(buf + sizeof(magic) + paramCount * sizeof(StoredParam));
                    for (int i = 0; i < 16; i++) {
                        (parts + i)->engine = (pt + i)->engine;
                        (parts + i)->midiChan = (pt + i)->midiChan;
                        (parts + i)->voices = (pt + i)->voices;
                        (parts + i)->patchOrNote = (pt + i)->patchOrNote;
                        (parts + i)->level = (pt + i)->level;
                    }
                    if (partRam && magic == 0x53594E32) memcpy(partRam, buf + baseBytes, ramBytes);
                    if (seqData && seqBytes > 0 && got >= baseBytes + ramBytes + seqBytes) memcpy(seqData, buf + baseBytes + ramBytes, seqBytes);
                    if (songData && songBytes > 0 && got >= baseBytes + ramBytes + seqBytes + songBytes) memcpy(songData, buf + baseBytes + ramBytes + seqBytes, songBytes);
                    if (got >= baseBytes + ramBytes + seqBytes + songBytes + sizeof(synthPatchNames)) memcpy(synthPatchNames, buf + baseBytes + ramBytes + seqBytes + songBytes, sizeof(synthPatchNames));
                    free(buf);
                    Serial.println("[Storage] Laddade auto-save fran SD (/AMY/autosave.bin)");
                    return true;
                }
                free(buf);
            } else {
                f.close();
            }
        }
    }
    if (!synthPrefs.begin("amy_synth", false)) return false;
    if (synthPrefs.getUInt("magic", 0) != 0x53594E31) {
        synthPrefs.end();
        return false;
    }
    size_t pLen = paramCount * sizeof(StoredParam);
    StoredParam *pBuf = (StoredParam*)ps_malloc(pLen);
    if (pBuf && synthPrefs.getBytes("params", pBuf, pLen) == pLen) {
        for (int i = 0; i < paramCount; i++) {
            (matrix + i)->currentVal = constrain((pBuf + i)->val, (matrix + i)->minVal, (matrix + i)->maxVal);
            (matrix + i)->mappedCC = (pBuf + i)->cc;
        }
    }
    if (pBuf) free(pBuf);

    size_t sLen = 16 * sizeof(StoredPart);
    StoredPart *sBuf = (StoredPart*)alloca(sLen);
    if (synthPrefs.getBytes("parts", sBuf, sLen) == sLen) {
        for (int i = 0; i < 16; i++) {
            (parts + i)->engine = (sBuf + i)->engine;
            (parts + i)->midiChan = (sBuf + i)->midiChan;
            (parts + i)->voices = (sBuf + i)->voices;
            (parts + i)->patchOrNote = (sBuf + i)->patchOrNote;
            (parts + i)->level = (sBuf + i)->level;
        }
    }
    synthPrefs.end();
    Serial.println("[Storage] Laddade läge från NVS Flash");
    return true;
}

template<typename ParamT, typename PartT>
inline bool saveProjectToDisk(int slot1To16, ParamT *matrix, int paramCount, PartT *parts, float *partRam = nullptr, uint8_t *seqData = nullptr, size_t seqBytes = 0, uint8_t *songData = nullptr, size_t songBytes = 0) {
    if (!activeStorageFS) return false;
    char *path = (char*)alloca(32);
    snprintf(path, 32, "/AMY/projects/proj_%02d.bin", constrain(slot1To16, 1, 16));
    size_t ramBytes = partRam ? (16 * paramCount * sizeof(float)) : 0;
    size_t sBytes = (seqData && seqBytes > 0) ? seqBytes : 0;
    size_t sgBytes = (songData && songBytes > 0) ? songBytes : 0;
    size_t nameBytes = sizeof(synthPatchNames);
    size_t totalBytes = sizeof(uint32_t) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes + sBytes + sgBytes + nameBytes;
    uint8_t *buf = (uint8_t*)ps_malloc(totalBytes);
    if (!buf) return false;
    uint32_t magic = partRam ? 0x50524A32 : 0x50524A31;
    memcpy(buf, &magic, sizeof(magic));
    StoredParam *sp = (StoredParam*)(buf + sizeof(magic));
    for (int i = 0; i < paramCount; i++) {
        (sp + i)->val = (matrix + i)->currentVal;
        (sp + i)->cc = (matrix + i)->mappedCC;
    }
    StoredPart *pt = (StoredPart*)(buf + sizeof(magic) + paramCount * sizeof(StoredParam));
    for (int i = 0; i < 16; i++) {
        (pt + i)->engine = (parts + i)->engine;
        (pt + i)->midiChan = (parts + i)->midiChan;
        (pt + i)->voices = (parts + i)->voices;
        (pt + i)->patchOrNote = (parts + i)->patchOrNote;
        (pt + i)->level = (parts + i)->level;
    }
    if (partRam) memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart), partRam, ramBytes);
    if (sBytes > 0) memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes, seqData, sBytes);
    if (sgBytes > 0) memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes + sBytes, songData, sgBytes);
    memcpy(buf + sizeof(magic) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart) + ramBytes + sBytes + sgBytes, synthPatchNames, nameBytes);
    if (activeStorageFS->exists(path)) activeStorageFS->remove(path);
    File f = activeStorageFS->open(path, FILE_WRITE);
    if (!f) { free(buf); return false; }
    size_t written = f.write(buf, totalBytes);
    f.close();
    free(buf);
    Serial.printf("[Storage] Sparade %s (%u B, %s)\n", path, (unsigned)written, usingPhysicalSD ? "SD CARD" : "INT FLASH");
    return (written == totalBytes);
}

template<typename ParamT, typename PartT>
inline bool loadProjectFromDisk(int slot1To16, ParamT *matrix, int paramCount, PartT *parts, float *partRam = nullptr, uint8_t *seqData = nullptr, size_t seqBytes = 0, uint8_t *songData = nullptr, size_t songBytes = 0) {
    if (!activeStorageFS) return false;
    char *path = (char*)alloca(32);
    snprintf(path, 32, "/AMY/projects/proj_%02d.bin", constrain(slot1To16, 1, 16));
    File f = activeStorageFS->open(path, FILE_READ);
    if (!f) return false;
    size_t baseBytes = sizeof(uint32_t) + paramCount * sizeof(StoredParam) + 16 * sizeof(StoredPart);
    size_t ramBytes = 16 * paramCount * sizeof(float);
    size_t fSize = f.size();
    uint8_t *buf = (uint8_t*)ps_malloc(fSize);
    if (!buf) { f.close(); return false; }
    size_t got = f.read(buf, fSize);
    f.close();
    uint32_t magic = 0;
    if (got >= baseBytes) memcpy(&magic, buf, sizeof(magic));
    if (!((got == baseBytes && magic == 0x50524A31) || ((got == baseBytes + ramBytes || got == baseBytes + ramBytes + seqBytes || got == baseBytes + ramBytes + seqBytes + songBytes || got == baseBytes + ramBytes + seqBytes + songBytes + sizeof(synthPatchNames)) && magic == 0x50524A32))) { free(buf); return false; }
    StoredParam *sp = (StoredParam*)(buf + sizeof(magic));
    for (int i = 0; i < paramCount; i++) {
        (matrix + i)->currentVal = constrain((sp + i)->val, (matrix + i)->minVal, (matrix + i)->maxVal);
        (matrix + i)->mappedCC = (sp + i)->cc;
    }
    StoredPart *pt = (StoredPart*)(buf + sizeof(magic) + paramCount * sizeof(StoredParam));
    for (int i = 0; i < 16; i++) {
        (parts + i)->engine = (pt + i)->engine;
        (parts + i)->midiChan = (pt + i)->midiChan;
        (parts + i)->voices = (pt + i)->voices;
        (parts + i)->patchOrNote = (pt + i)->patchOrNote;
        (parts + i)->level = (pt + i)->level;
    }
    if (partRam && magic == 0x50524A32) memcpy(partRam, buf + baseBytes, ramBytes);
    if (seqData && seqBytes > 0 && got >= baseBytes + ramBytes + seqBytes) memcpy(seqData, buf + baseBytes + ramBytes, seqBytes);
    if (songData && songBytes > 0 && got >= baseBytes + ramBytes + seqBytes + songBytes) memcpy(songData, buf + baseBytes + ramBytes + seqBytes, songBytes);
    if (got >= baseBytes + ramBytes + seqBytes + songBytes + sizeof(synthPatchNames)) memcpy(synthPatchNames, buf + baseBytes + ramBytes + seqBytes + songBytes, sizeof(synthPatchNames));
    free(buf);
    Serial.printf("[Storage] Laddade %s (%s)\n", path, usingPhysicalSD ? "SD CARD" : "INT FLASH");
    return true;
}

template<typename ParamT, typename PartT>
inline bool saveSinglePatchToDisk(int partIdx, ParamT *matrix, int paramCount, PartT *parts, float *partRam) {
    if (!activeStorageFS || partIdx < 0 || partIdx >= 16) return false;
    ensureStorageDirs();
    PartT *sp = parts + partIdx;
    const char *rawName = ((char*)synthPatchNames) + partIdx * 16;
    if (!rawName || *rawName == 0) {
        const char *romName = getEnginePatchName(sp->engine, sp->patchOrNote);
        if (romName && *romName != 0 && strcmp(romName, "-") != 0) rawName = romName;
        else rawName = "";
    }
    char *safeName = (char*)alloca(24);
    int j = 0;
    for (int i = 0; i < 18 && *(rawName + i) != 0 && j < 16; i++) {
        char c = *(rawName + i);
        if (isalnum((unsigned char)c) || c == '-' || c == '_') *(safeName + j++) = c;
        else if (c == ' ' && j > 0 && *(safeName + j - 1) != '_') *(safeName + j++) = '_';
    }
    while (j > 0 && *(safeName + j - 1) == '_') j--;
    *(safeName + j) = 0;
    if (j == 0) snprintf(safeName, 24, "Synth_%02d", partIdx + 1);
    char *path = (char*)alloca(48);
    snprintf(path, 48, "/AMY/patches/%s.pat", safeName);
    if (activeStorageFS->exists(path)) activeStorageFS->remove(path);
    File f = activeStorageFS->open(path, FILE_WRITE);
    if (!f) return false;
    f.printf("{\n  \"format\": \"MiNi-AMY-Patch\",\n  \"version\": 1,\n");
    f.printf("  \"name\": \"%s\",\n", rawName);
    f.printf("  \"engine\": %u,\n  \"voices\": %u,\n  \"patch\": %u,\n  \"level\": %.4f,\n",
             sp->engine, sp->voices, sp->patchOrNote, sp->level);
    f.print("  \"params\": {\n");
    bool first = true;
    uint8_t targetMode = 255;
    if (sp->engine == 1) targetMode = 0;
    else if (sp->engine == 2) targetMode = 1;
    else if (sp->engine == 3) targetMode = 4;
    else if (sp->engine == 4) targetMode = 2;
    else if (sp->engine == 5) targetMode = 5;
    for (int i = 0; i < paramCount; i++) {
        ParamT *pm = matrix + i;
        if (pm->pType != 0 || !pm->name || *(pm->name) == 0) continue;
        if (targetMode != 255 && (uint8_t)pm->belongsToMode != targetMode) continue;
        float val = partRam ? *(partRam + partIdx * paramCount + i) : pm->currentVal;
        if (!first) f.print(",\n");
        f.printf("    \"id_%d_%s\": %.4f", pm->internalId, pm->name, val);
        first = false;
    }
    f.print("\n  }\n}\n");
    f.close();
    Serial.printf("[Storage] Exporterade JSON-patch: %s\n", path);
    return true;
}

template<typename ParamT, typename PartT>
inline bool loadSinglePatchFromDisk(const char *fileName, int partIdx, ParamT *matrix, int paramCount, PartT *parts, float *partRam) {
    if (!activeStorageFS || !fileName || partIdx < 0 || partIdx >= 16) return false;
    char *path = (char*)alloca(56);
    snprintf(path, 56, "/AMY/patches/%s", fileName);
    File f = activeStorageFS->open(path, FILE_READ);
    if (!f) return false;
    size_t fSize = f.size();
    char *txt = (char*)ps_malloc(fSize + 1);
    if (!txt) { f.close(); return false; }
    size_t got = f.read((uint8_t*)txt, fSize);
    f.close();
    *(txt + got) = 0;

    if (got >= 4 && *((uint32_t*)txt) == 0x50415431) {
        size_t ramBytes = paramCount * sizeof(float);
        if (got == sizeof(uint32_t) + sizeof(StoredPart) + 16 + ramBytes) {
            StoredPart sp;
            memcpy(&sp, txt + 4, sizeof(StoredPart));
            (parts + partIdx)->engine = sp.engine;
            (parts + partIdx)->voices = sp.voices;
            (parts + partIdx)->patchOrNote = sp.patchOrNote;
            (parts + partIdx)->level = sp.level;
            memcpy(((char*)synthPatchNames) + partIdx * 16, txt + 4 + sizeof(StoredPart), 16);
            if (partRam) memcpy(partRam + partIdx * paramCount, txt + 4 + sizeof(StoredPart) + 16, ramBytes);
            free(txt);
            return true;
        }
        free(txt);
        return false;
    }

    if (!strstr(txt, "MiNi-AMY-Patch")) { free(txt); return false; }
    PartT *sp = parts + partIdx;
    char *pName = strstr(txt, "\"name\":");
    if (pName) {
        char *q1 = strchr(pName + 7, '"');
        char *q2 = q1 ? strchr(q1 + 1, '"') : nullptr;
        if (q1 && q2) {
            int len = min((int)(q2 - q1 - 1), 15);
            char *dst = ((char*)synthPatchNames) + partIdx * 16;
            memset(dst, 0, 16);
            memcpy(dst, q1 + 1, len);
        }
    }
    char *pEng = strstr(txt, "\"engine\":");
    if (pEng) sp->engine = (uint8_t)atoi(pEng + 9);
    char *pVoi = strstr(txt, "\"voices\":");
    if (pVoi) sp->voices = (uint8_t)atoi(pVoi + 9);
    char *pPat = strstr(txt, "\"patch\":");
    if (pPat) sp->patchOrNote = (uint8_t)atoi(pPat + 8);
    char *pLvl = strstr(txt, "\"level\":");
    if (pLvl) sp->level = (float)atof(pLvl + 8);

    char *key = (char*)alloca(48);
    for (int i = 0; i < paramCount; i++) {
        ParamT *pm = matrix + i;
        if (pm->pType != 0 || !pm->name || *(pm->name) == 0) continue;
        snprintf(key, 48, "\"id_%d_%s\":", pm->internalId, pm->name);
        char *hit = strstr(txt, key);
        if (hit && partRam) {
            float v = (float)atof(hit + strlen(key));
            *(partRam + partIdx * paramCount + i) = constrain(v, pm->minVal, pm->maxVal);
        }
    }
    free(txt);
    Serial.printf("[Storage] Importerade JSON-patch: %s -> Synth %d\n", path, partIdx + 1);
    return true;
}

inline void factoryResetNvs() {
    storageDirty = false;
    if (usingPhysicalSD && activeStorageFS && activeStorageFS->exists("/AMY/autosave.bin")) {
        activeStorageFS->remove("/AMY/autosave.bin");
    }
    if (synthPrefs.begin("amy_synth", false)) {
        synthPrefs.clear();
        synthPrefs.end();
    }
    Serial.println("[Storage] NVS rensad - startar om...");
    delay(400);
    ESP.restart();
}
