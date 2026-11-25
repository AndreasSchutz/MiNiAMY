#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <usb/usb_host.h>

extern void processRawMidiByte(uint8_t b);
inline volatile bool g_usbMidiConnected = false;

static usb_host_client_handle_t g_usbClientHdl = nullptr;
static usb_device_handle_t g_usbDevHdl = nullptr;
static usb_transfer_t *g_usbMidiInXfer = nullptr;
static uint8_t g_usbMidiIntfNum = 0;
static uint8_t g_usbMidiInEp = 0;
static uint16_t g_usbMidiInMps = 64;
static volatile bool g_usbDevGone = false;

inline void usbMidiInTransferCb(usb_transfer_t *xfer) {
    if (!xfer) return;
    if (xfer->status == USB_TRANSFER_STATUS_COMPLETED && xfer->actual_num_bytes >= 4) {
        const uint8_t *d = xfer->data_buffer;
        for (int i = 0; i + 3 < xfer->actual_num_bytes; i += 4) {
            uint8_t cin = *(d + i) & 0x0F;
            if (cin == 0) continue;
            uint8_t len = 3;
            if (cin == 0x05 || cin == 0x0F) len = 1;
            else if (cin == 0x02 || cin == 0x06 || cin == 0x0C || cin == 0x0D) len = 2;
            for (uint8_t k = 0; k < len; k++) processRawMidiByte(*(d + i + 1 + k));
        }
    }
    if (g_usbMidiConnected && !g_usbDevGone && g_usbDevHdl) {
        usb_host_transfer_submit(xfer);
    }
}

inline bool findMidiInEndpoint(const usb_config_desc_t *cfg, uint8_t *outIntf, uint8_t *outEp, uint16_t *outMps) {
    if (!cfg) return false;
    const uint8_t *p = (const uint8_t *)cfg;
    uint16_t tot = cfg->wTotalLength, off = 0;
    bool inMidiIntf = false;
    uint8_t curIntf = 0;
    while (off + 2 <= tot) {
        uint8_t bLen = *(p + off), bType = *(p + off + 1);
        if (bLen == 0) break;
        if (bType == USB_B_DESCRIPTOR_TYPE_INTERFACE && off + 9 <= tot) {
            curIntf = *(p + off + 2);
            uint8_t cls = *(p + off + 5), sub = *(p + off + 6);
            inMidiIntf = (cls == 0x01 && sub == 0x03);
        } else if (inMidiIntf && bType == USB_B_DESCRIPTOR_TYPE_ENDPOINT && off + 7 <= tot) {
            uint8_t epAddr = *(p + off + 2), attr = *(p + off + 3);
            if ((epAddr & 0x80) && ((attr & 0x03) == 0x02 || (attr & 0x03) == 0x03)) {
                *outIntf = curIntf;
                *outEp = epAddr;
                *outMps = (uint16_t)(*(p + off + 4) | (*(p + off + 5) << 8));
                if (*outMps == 0 || *outMps > 64) *outMps = 64;
                return true;
            }
        }
        off += bLen;
    }
    return false;
}

inline void usbClientEventCb(const usb_host_client_event_msg_t *eventMsg, void *arg) {
    if (!eventMsg) return;
    if (eventMsg->event == USB_HOST_CLIENT_EVENT_NEW_DEV && !g_usbDevHdl) {
        if (usb_host_device_open(g_usbClientHdl, eventMsg->new_dev.address, &g_usbDevHdl) == ESP_OK) {
            const usb_config_desc_t *cfg = nullptr;
            if (usb_host_get_active_config_descriptor(g_usbDevHdl, &cfg) == ESP_OK &&
                findMidiInEndpoint(cfg, &g_usbMidiIntfNum, &g_usbMidiInEp, &g_usbMidiInMps)) {
                if (usb_host_interface_claim(g_usbClientHdl, g_usbDevHdl, g_usbMidiIntfNum, 0) == ESP_OK) {
                    if (usb_host_transfer_alloc(g_usbMidiInMps, 0, &g_usbMidiInXfer) == ESP_OK) {
                        g_usbMidiInXfer->device_handle = g_usbDevHdl;
                        g_usbMidiInXfer->bEndpointAddress = g_usbMidiInEp;
                        g_usbMidiInXfer->callback = usbMidiInTransferCb;
                        g_usbMidiInXfer->num_bytes = g_usbMidiInMps;
                        g_usbDevGone = false;
                        g_usbMidiConnected = true;
                        usb_host_transfer_submit(g_usbMidiInXfer);
                        Serial.printf("[USB MIDI] Keyboard anslutet (EP 0x%02X)\n", g_usbMidiInEp);
                        return;
                    }
                    usb_host_interface_release(g_usbClientHdl, g_usbDevHdl, g_usbMidiIntfNum);
                }
            }
            usb_host_device_close(g_usbClientHdl, g_usbDevHdl);
            g_usbDevHdl = nullptr;
        }
    } else if (eventMsg->event == USB_HOST_CLIENT_EVENT_DEV_GONE) {
        g_usbDevGone = true;
        g_usbMidiConnected = false;
    }
}

inline void usbHostLibTask(void *arg) {
    usb_host_config_t hCfg = {};
    hCfg.skip_phy_setup = false;
    hCfg.intr_flags = ESP_INTR_FLAG_LEVEL1;
    if (usb_host_install(&hCfg) != ESP_OK) { vTaskDelete(nullptr); return; }
    for (;;) {
        uint32_t flags = 0;
        usb_host_lib_handle_events(portMAX_DELAY, &flags);
        if (flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) usb_host_device_free_all();
    }
}

inline void usbMidiClientTask(void *arg) {
    vTaskDelay(pdMS_TO_TICKS(150));
    usb_host_client_config_t cCfg = {};
    cCfg.is_synchronous = false;
    cCfg.max_num_event_msg = 5;
    cCfg.async.client_event_callback = usbClientEventCb;
    if (usb_host_client_register(&cCfg, &g_usbClientHdl) != ESP_OK) { vTaskDelete(nullptr); return; }
    for (;;) {
        usb_host_client_handle_events(g_usbClientHdl, pdMS_TO_TICKS(20));
        if (g_usbDevGone && g_usbDevHdl) {
            g_usbDevGone = false;
            if (g_usbMidiInXfer) { usb_host_transfer_free(g_usbMidiInXfer); g_usbMidiInXfer = nullptr; }
            usb_host_interface_release(g_usbClientHdl, g_usbDevHdl, g_usbMidiIntfNum);
            usb_host_device_close(g_usbClientHdl, g_usbDevHdl);
            g_usbDevHdl = nullptr;
            Serial.println("[USB MIDI] Keyboard urkopplat");
        }
    }
}

inline void startUsbMidiHost() {
    xTaskCreatePinnedToCore(usbHostLibTask, "USB_HostLib", 4096, nullptr, 2, nullptr, 0);
    xTaskCreatePinnedToCore(usbMidiClientTask, "USB_MidiCli", 4096, nullptr, 2, nullptr, 0);
}
