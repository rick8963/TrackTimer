#include <Arduino.h>
#include <SPIFFS.h>
#include "Track.h"
#include "GPSPoint.h"
#include "../lib/StorageManager.h"
#include "../lib/GpsTimeParser.h"
#include "../lib/GpsReceiver.h"
#include "../lib/WebInterface.h"
#include "../lib/DisplayManager.h"
#include "../lib/StatusLED.h"
#include "Config.h"

// 全域物件
StorageManager g_storage(STORAGE_FS);
GpsTimeParser g_timeParser;
GpsReceiver g_gpsReceiver;
WebInterface g_web(g_storage);
DisplayManager g_display;
StatusLED g_statusLED;
Track track(buildTKS(), true);

// 狀態變數
int lineCount = 0;
uint32_t firstTimestamp = 0;
bool g_storageReady = false;
bool g_logFileOpened = false;
bool g_replayMode = true;  // true=檔案回放, false=即時GPS
File nmeaFile;

LapInfo trackToLapInfo(const Track& track) {
    LapInfo lap;
    if (track.getLaps().empty()) {
        lap.currentLap = "00:00.0";
        lap.bestLap = "--:--.-";
    } else {
        const auto& latestLap = track.getLaps().back();
        lap.currentLapNum = track.getLaps().size();
        lap.totalLaps = track.getLaps().size();
        
        // Current lap (未完成)
        uint32_t currLapTime = track.getCurrentSectorCount() > 0 ? 
            (millis() - track.getSessionStartTime()) : 0;
        lap.currentLap = msToLapTime(currLapTime);
        
        // Best lap
        lap.bestLap = msToLapTime(track.getBestLapTime());
        
        // Delta (未完成)
        lap.deltaStr = "+0.123";
        lap.deltaSeconds = 0.123f;
    }
    return lap;
}

String msToLapTime(TimeMs ms) {
    if (ms == 0) return "00:00.0";
    TimeMs minutes = ms / 60000;
    TimeMs seconds = (ms % 60000) / 1000;
    TimeMs millis = ms % 1000 / 10;
    char buf[9];
    snprintf(buf, sizeof(buf), "%02u:%02u.%u", minutes, seconds, (unsigned)millis);
    return String(buf);
}

void handleNmeaLine(const String &line);  // 前置聲明
void setupWiFiAP();
void printResults();

std::vector<Line2D> buildTKS() {
    std::vector<Line2D> sectors;
    sectors.push_back(Line2D(GPSPoint(22.742248, 120.322181, true), 0, 20));
    sectors.push_back(Line2D(GPSPoint(22.742798, 120.321496, true), 180, 20));
    sectors.push_back(Line2D(GPSPoint(22.742724, 120.322010, true), 180, 20));
    sectors.push_back(Line2D(GPSPoint(22.742285, 120.321387, true), 60, 20));
    sectors.push_back(Line2D(GPSPoint(22.742540, 120.321959, true), 88, 20));
    sectors.push_back(Line2D(GPSPoint(22.741863, 120.321912, true), 262, 20));
    sectors.push_back(Line2D(GPSPoint(22.741763, 120.321930, true), 81, 20));
    return sectors;
}
void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=== ESP32 TrackTimer v2.0 ===");
    Serial.println("完全取代 Qstarz 賽道計時器");
    
    // 1. SPIFFS 初始化
    if (!SPIFFS.begin(true)) {
        Serial.println("❌ SPIFFS mount failed");
        return;
    }
    Serial.printf("✅ SPIFFS: %d total, %d used\n", 
        SPIFFS.totalBytes(), SPIFFS.usedBytes());
    
    // 檢查 TKS.nmea
    nmeaFile = SPIFFS.open("/TKS.nmea", "r");
    if (!nmeaFile || nmeaFile.size() == 0) {
        Serial.println("⚠️ TKS.nmea not found, GPS live mode");
        g_replayMode = false;
    } else {
        Serial.printf("✅ TKS.nmea: %d bytes\n", nmeaFile.size());
        g_replayMode = true;
    }
    
    // 2. StorageManager
    g_storageReady = g_storage.begin();
    // if (g_storageReady) {
    //     g_storage.listFiles();  // 顯示現有 logs
    // }
    
    // 3. GPS Receiver（即時模式）
    if (!g_replayMode) {
        g_gpsReceiver.begin(GPS_BAUD_RATE);
        g_gpsReceiver.onLine(handleNmeaLine);
    }
    
    // 4. WiFi AP + Web
    setupWiFiAP();
    g_web.begin();
    
    // 5. Display + LED
    g_display.begin();
    g_statusLED.begin();
    
    Serial.println("🚀 TrackTimer ready!");
}

void loop() {
    uint32_t loopStart = millis();
    
    // 1. 處理 NMEA 輸入（統一入口）
    bool gpsActive = false;
    if (g_replayMode) {
        if (nmeaFile.available()) {
            String line = nmeaFile.readStringUntil('\n');
            line.trim();
            lineCount++;
            handleNmeaLine(line);
            gpsActive = true;
        }
    } else {
        gpsActive = g_gpsReceiver.loop();
    }
    
    // 2. Web 服務
    g_web.handleClient();
    
    // 3. 狀態更新（每100ms）
    static uint32_t lastStatus = 0;
    if (millis() - lastStatus > 100) {
        bool wifiActive = WiFi.softAPgetStationNum() > 0;
        bool sseActive = g_web.isSSEConnected();
        
        g_statusLED.update(
            g_storageReady, gpsActive, g_logFileOpened,
            wifiActive, sseActive, false
        );
        
        // Display 更新（Track → LapInfo）
        LapInfo lap = trackToLapInfo(track);
        GpsData gps;  // 簡化，之後從 parser 取
        DateTimeInfo time = g_timeParser.current();
        
        g_display.update(
            g_storageReady, gpsActive, g_logFileOpened,
            wifiActive, sseActive,
            time, gps, lap, lineCount
        );
        
        lastStatus = millis();
    }
    
    // 4. Replay 結束檢查
    if (g_replayMode && nmeaFile.available() == 0) {
        printResults();
        g_storage.closeCurrentFile();
        while(1) { 
            g_web.handleClient();
            delay(100);
        }
    }
    
    // 5. 避免 WDT
    if (millis() - loopStart > 50) {
        Serial.printf("⚠️ Loop overrun: %dms\n", millis() - loopStart);
    }
}

void handleNmeaLine(const String &line) {
    // 1. Time parser
    g_timeParser.processLine(line);
    
    // 2. GPRMC 解析 → Track 更新（核心！）
    double lat, lon;
    uint32_t timestamp;
    if (parseGPRMC(line.c_str(), lat, lon, timestamp)) {
        GPSPoint pos(lat, lon, true);
        track.updatePos(pos, timestamp);
        
        // 3. Web live view
        g_web.pushNmeaLine(line);
        
        // 4. 自動開 log
        if (!g_logFileOpened && g_timeParser.hasValidTime() && g_storageReady) {
            DateTimeInfo t = g_timeParser.current();
            String path = g_storage.createNewLogFile(t);
            if (path.length() > 0) g_logFileOpened = true;
        }
        
        // 5. Log append
        if (g_logFileOpened) {
            String logLine = String(timestamp) + "," + 
                           String(lat,6) + "," + String(lon,6) + "," +
                           String(track.getCurrentSectorCount()+1) + "," +
                           String(track.getLaps().size());
            g_storage.appendLine(logLine);
        }
    }
}

void printResults() {
    Serial.println("\n=== FINAL RESULTS ===");
    Serial.printf("Lines: %d\n", lineCount);
    Serial.printf("Total Laps: %d\n", track.getLaps().size());
    if (!track.getLaps().empty()) {
        Serial.printf("Best: %.3f s\n", track.getBestLapTime() / 1000.0f);
        Serial.printf("Latest: %.3f s\n", track.getLatestLapTime() / 1000.0f);
        
        Serial.println("\nLAP DETAILS:");
        for (size_t i = 0; i < track.getLaps().size(); i++) {
            const auto& lap = track.getLaps()[i];
            Serial.printf("Lap %zu: %.3f s\n", i+1, lap.getLapTime() / 1000.0f);
            for (uint8_t s = 0; s < track.getSectorCount(); s++) {
                if (lap.hasSectorTime(s)) {
                    Serial.printf("  S%d: %.3f s\n", s+1, lap.getSectorTime(s) / 1000.0f);
                }
            }
        }
    }
}

void setupWiFiAP() {
    WiFi.mode(WIFI_AP);
    bool ok = WiFi.softAP(AP_SSID, AP_PASSWORD);
    if (!ok) {
        Serial.println("❌ WiFi AP failed");
        return;
    }
    IPAddress ip = WiFi.softAPIP();
    Serial.printf("📶 WiFi AP: %s @ %s\n", AP_SSID, ip.toString().c_str());
}