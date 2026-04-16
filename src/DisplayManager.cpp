#include "DisplayManager.h"
#include <U8G2lib.h>
#include <stdio.h>

char displayBuf[32];

static void drawInvertedLabel(U8G2 &u8g2, int x, int y, int w, int h, const char *label);
const char* headingToDir(float deg);

U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8g2(
    U8G2_R0,          // 旋轉 0°   
    OLED_SCLK_PIN,
    OLED_MOSI_PIN,
    OLED_CS_PIN,      // cs
    OLED_DC_PIN,      // dc
    OLED_RES_PIN      // reset
);


void DisplayManager::begin() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    u8g2.begin();

    // 可選：有些面板調整 clock 後體感會更穩，可先保守不開
    // u8g2.sendF("ca", 0xD5, 0x80);

    u8g2.setFont(u8g2_font_6x10_tf);
}

bool DisplayManager::buttonPressed() {
    static bool lastState = HIGH;
    bool currentState = digitalRead(BUTTON_PIN);
    bool pressed = (lastState == HIGH && currentState == LOW);
    lastState = currentState;
    return pressed;
}

bool DisplayManager::needsRedraw() const {
    if (_lastMode != mode) return true;

    static bool lastStorageReady = false;
    static bool lastGpsActive = false;
    static bool lastLogFileOpened = false;
    static bool lastWifiActive = false;
    static bool lastSseActive = false;

    if (_storageReady != lastStorageReady) return true;
    if (_gpsActive != lastGpsActive) return true;
    if (_logFileOpened != lastLogFileOpened) return true;
    if (_wifiActive != lastWifiActive) return true;
    if (_sseActive != lastSseActive) return true;

    if (_timeInfo.second != _lastTimeInfo.second) return true;
    if (_timeInfo.minute != _lastTimeInfo.minute) return true;
    if (_timeInfo.hour != _lastTimeInfo.hour) return true;

    if ((int)_gpsData.speedKmh != (int)_lastGpsData.speedKmh) return true;
    if (_gpsData.satCount != _lastGpsData.satCount) return true;
    if (_gpsData.fixType != _lastGpsData.fixType) return true;
    if ((int)_gpsData.headingDeg != (int)_lastGpsData.headingDeg) return true;

    if ((int)(_gpsData.latitude * 10000) != (int)(_lastGpsData.latitude * 10000)) return true;
    if ((int)(_gpsData.longitude * 10000) != (int)(_lastGpsData.longitude * 10000)) return true;

    if (_lapInfo.currentLap != _lastLapInfo.currentLap) return true;
    if (_lapInfo.bestLap != _lastLapInfo.bestLap) return true;
    if (_lapInfo.deltaStr != _lastLapInfo.deltaStr) return true;
    if (_lapInfo.currentLapNum != _lastLapInfo.currentLapNum) return true;

    return false;
}

void DisplayManager::update(
    bool storageReady,
    bool gpsActive,
    bool logFileOpened,
    bool wifiActive,
    bool sseActive,
    const DateTimeInfo &timeInfo,
    const GpsData &gps,
    const LapInfo &lap,
    uint32_t loopCount
) {
    (void)loopCount;

    if (buttonPressed() && millis() - lastButtonPress > 250) {
        if (mode == DEBUG_MODE) mode = DRIVE_MODE;
        else if (mode == DRIVE_MODE) mode = TRACK_MODE;
        else mode = DEBUG_MODE;

        lastButtonPress = millis();
    }

    _storageReady = storageReady;
    _gpsActive = gpsActive;
    _logFileOpened = logFileOpened;
    _wifiActive = wifiActive;
    _sseActive = sseActive;
    _timeInfo = timeInfo;
    _gpsData = gps;
    _lapInfo = lap;

    uint32_t interval = 200;
    if (mode == DEBUG_MODE) interval = 300;
    else if (mode == DRIVE_MODE) interval = 200;
    else if (mode == TRACK_MODE) interval = 100;

    if (millis() - _lastDisplayMs < interval) return;
    _lastDisplayMs = millis();

    if (!needsRedraw()) return;

    u8g2.clearBuffer();

    if (mode == DEBUG_MODE) {
        drawDebugMode();
    } else if (mode == DRIVE_MODE) {
        drawDriveMode(_gpsData, _timeInfo);
    } else {
        drawTrackMode(_gpsData, _lapInfo);
    }

    u8g2.sendBuffer();

    _lastMode = mode;
    _lastTimeInfo = _timeInfo;
    _lastGpsData = _gpsData;
    _lastLapInfo = _lapInfo;
}

void DisplayManager::drawTrackMode(const GpsData &gps, const LapInfo &lap) {
    char buf[32];

    u8g2.drawFrame(0, 0, 128, 64);

    // =========================
    // Main current lap block
    // =========================
    u8g2.drawFrame(0, 0, 128, 28);
    drawInvertedLabel(u8g2, 1, 1, 20, 8, "CUR");

    u8g2.setFont(u8g2_font_logisoso16_tn);
    u8g2.drawStr(24, 18, lap.currentLap.c_str());

    // Current lap number + DEL under CUR
    u8g2.setFont(u8g2_font_5x7_tf);
    snprintf(buf, sizeof(buf), "#%d  DEL %s", lap.currentLapNum, lap.deltaStr.c_str());
    u8g2.drawStr(4, 26, buf);

    // =========================
    // LAST block
    // =========================
    u8g2.drawFrame(0, 28, 64, 28);
    drawInvertedLabel(u8g2, 1, 29, 24, 8, "LAST");

    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(6, 45, lap.lastLap.c_str());

    u8g2.setFont(u8g2_font_5x7_tf);
    snprintf(buf, sizeof(buf), "#%d", lap.lastLapNum);
    u8g2.drawStr(46, 54, buf);

    // =========================
    // BEST block
    // =========================
    u8g2.drawFrame(64, 28, 64, 28);
    drawInvertedLabel(u8g2, 65, 29, 24, 8, "BEST");

    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(70, 45, lap.bestLap.c_str());

    u8g2.setFont(u8g2_font_5x7_tf);
    snprintf(buf, sizeof(buf), "#%d", lap.bestLapNum);
    u8g2.drawStr(110, 54, buf);

    // =========================
    // Bottom status bar
    // =========================
    u8g2.drawFrame(0, 56, 128, 8);
    u8g2.setFont(u8g2_font_4x6_tf);

    const char *fixStr = "NO";
    if (gps.fixType == 2) fixStr = "2D";
    else if (gps.fixType == 3) fixStr = "3D";
    else if (gps.fixType > 0) fixStr = "FX";

    snprintf(buf, sizeof(buf), "SPD%3d SAT%02d %s LAP%d/%d",
         (int)gps.speedKmh,
         gps.satCount,
         fixStr,
         lap.currentLapNum,
         lap.totalLaps);
    u8g2.drawStr(2, 63, buf);
}

void DisplayManager::drawDebugMode() {
    u8g2.setFont(u8g2_font_6x10_tf);

    u8g2.drawStr(0, 12, _gpsActive ? "GPS:ON" : "GPS:OFF");
    u8g2.drawStr(64, 12, _logFileOpened ? "LOG:ON" : "LOG:OFF");

    u8g2.drawStr(0, 28, _wifiActive ? "WIFI:ON" : "WIFI:OFF");
    u8g2.drawStr(64, 28, _sseActive ? "SSE:ON" : "SSE:OFF");

    if (_timeInfo.isValid()) {
        snprintf(displayBuf, sizeof(displayBuf), "%04d/%02d/%02d",
                 _timeInfo.year, _timeInfo.month, _timeInfo.day);
        u8g2.drawStr(0, 44, displayBuf);

        snprintf(displayBuf, sizeof(displayBuf), "%02d:%02d:%02d",
                 _timeInfo.hour, _timeInfo.minute, _timeInfo.second);
        u8g2.drawStr(0, 60, displayBuf);
    } else {
        u8g2.drawStr(0, 44, "TIME:NO FIX");
    }
}

static void drawInvertedLabel(U8G2 &u8g2, int x, int y, int w, int h, const char *label) {
    u8g2.setDrawColor(1);
    u8g2.drawBox(x, y, w, h);
    u8g2.setDrawColor(0);
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(x + 2, y + h - 2, label);
    u8g2.setDrawColor(1);
}

void DisplayManager::drawDriveMode(const GpsData &gps, const DateTimeInfo &timeInfo) {
    char buf[32];
    const char *dir = headingToDir(gps.headingDeg);
    u8g2.drawFrame(0, 0, 128, 64);

    // speed tile area
    u8g2.drawFrame(0, 0, 84, 18);
    drawInvertedLabel(u8g2, 1, 1, 18, 8, "SPD");

    u8g2.setFont(u8g2_font_logisoso16_tn);
    snprintf(buf, sizeof(buf), "%2d", (int)gps.speedKmh);
    u8g2.drawStr(22, 16, buf);

    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(56, 15, "km/h");

    // SAT tile
    u8g2.drawFrame(84, 0, 22, 18);
    drawInvertedLabel(u8g2, 85, 1, 20, 8, "SAT");
    u8g2.setFont(u8g2_font_6x10_tf);
    snprintf(buf, sizeof(buf), "%02d", gps.satCount);
    u8g2.drawStr(89, 16, buf);

    // FIX tile
    u8g2.drawFrame(106, 0, 22, 18);
    drawInvertedLabel(u8g2, 107, 1, 20, 8, "FIX");
    const char *fixStr = "NO";
    if (gps.fixType == 2) fixStr = "2D";
    else if (gps.fixType == 3) fixStr = "3D";
    else if (gps.fixType > 0) fixStr = "FX";
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(110, 16, fixStr);

    // HDG tile
    u8g2.drawFrame(0, 18, 64, 15);
    drawInvertedLabel(u8g2, 1, 19, 20, 8, "HDG");
    u8g2.setFont(u8g2_font_6x10_tf);
    snprintf(buf, sizeof(buf), "%03d", (int)gps.headingDeg);
    u8g2.drawStr(25, 30, buf);

    // DIR tile
    u8g2.drawFrame(64, 18, 64, 15);
    drawInvertedLabel(u8g2, 65, 19, 18, 8, "DIR");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(89, 30, dir);

    // LAT tile
    u8g2.drawFrame(0, 33, 64, 15);
    drawInvertedLabel(u8g2, 1, 34, 18, 8, "LAT");
    u8g2.setFont(u8g2_font_5x7_tf);
    snprintf(buf, sizeof(buf), "%.4f%c", gps.latitude, gps.latNorth ? 'N' : 'S');
    u8g2.drawStr(22, 45, buf);

    // LON tile
    u8g2.drawFrame(64, 33, 64, 15);
    drawInvertedLabel(u8g2, 65, 34, 18, 8, "LON");
    snprintf(buf, sizeof(buf), "%.4f%c", gps.longitude, gps.lonEast ? 'E' : 'W');
    u8g2.drawStr(86, 45, buf);

    // UTC tile
    u8g2.drawFrame(0, 48, 80, 16);
    drawInvertedLabel(u8g2, 1, 49, 18, 8, "UTC");
    u8g2.setFont(u8g2_font_6x10_tf);
    if (timeInfo.isValid()) {
        int localHour = timeInfo.hour + 8;
        if (localHour >= 24) localHour -= 24;

        snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
                 localHour, timeInfo.minute, timeInfo.second);
    } else {
        snprintf(buf, sizeof(buf), "--:--:--");
    }
    u8g2.drawStr(23, 61, buf);

    // CRS tile
    u8g2.drawFrame(80, 48, 48, 16);
    drawInvertedLabel(u8g2, 81, 49, 18, 8, "CRS");
    snprintf(buf, sizeof(buf), "%03d", (int)gps.headingDeg);
    u8g2.drawStr(103, 61, buf);
}

const char* headingToDir(float deg) {
    if (deg >= 337.5 || deg < 22.5) return "N";
    if (deg < 67.5)  return "NE";
    if (deg < 112.5) return "E";
    if (deg < 157.5) return "SE";
    if (deg < 202.5) return "S";
    if (deg < 247.5) return "SW";
    if (deg < 292.5) return "W";
    return "NW";
}
