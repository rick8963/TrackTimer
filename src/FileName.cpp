#include <Arduino.h>
#include "Track.h"
#include "GPSPoint.h"
#include "StorageManager.h"
#include "GpsTimeParser.h"
#include "GpsReceiver.h"
#include "WebInterface.h"
#include "DisplayManager.h"
#include "StatusLED.h"
#include "Config.h"

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
bool g_storageReady = false;
bool g_logFileOpened = false;

String msToLapTime(TimeMs ms) {
    if (ms == 0) return "00:00.0";
    TimeMs minutes = ms / 60000;
    TimeMs seconds = (ms % 60000) / 1000;
    TimeMs millis = ms % 1000 / 10;
    char buf[9];
    snprintf(buf, sizeof(buf), "%02u:%02u.%u", minutes, seconds, (unsigned)millis);
    return String(buf);
}
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


void handleNmeaLine(const String &line);  // 前置聲明
void setupWiFiAP();

void setup() {
    Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== ESP32 GPS NMEA Logger ===");

  if (!STORAGE_FS.begin(true)) {
    Serial.println("[Storage] SPIFFS mount failed");
    g_storageReady = false;
  } else {
    g_storageReady = g_storage.begin();
  }

  g_gpsReceiver.begin(GPS_BAUD_RATE);
  g_gpsReceiver.onLine(handleNmeaLine);

  setupWiFiAP();
  g_web.begin();

  Serial.println("[Setup] Completed");
  g_statusLED.begin();
  g_display.begin();
}

void loop() {
    uint32_t loopStart = millis();
    
    bool gpsActive = g_gpsReceiver.loop();
    
    // Web 服務
    g_web.handleClient();
    
    // 狀態更新（每100ms）
    static uint32_t lastStatus = 0;
    if (millis() - lastStatus > 100) {
        bool wifiActive = WiFi.softAPgetStationNum() > 0;
        bool sseActive = g_web.isSSEConnected();
        
        g_statusLED.update(
            g_storageReady, gpsActive, g_logFileOpened,
            wifiActive, sseActive, false
        );
        
        LapInfo lap = trackToLapInfo(track);
        GpsData gps = g_timeParser.currentGps();
        DateTimeInfo time = g_timeParser.current();
        
        g_display.update(
            g_storageReady, gpsActive, g_logFileOpened,
            wifiActive, sseActive,
            time, gps, lap, lineCount
        );
        
        lastStatus = millis();
    }   
    
    // 避免 WDT
    if (millis() - loopStart > 50) {
        Serial.printf("⚠️ Loop overrun: %dms\n", millis() - loopStart);
    }
}

void handleNmeaLine(const String &line) {
  g_timeParser.processLine(line);

  // Push to live view buffer (always, regardless of storage state)
  g_web.pushNmeaLine(line);

  if (!g_logFileOpened && g_timeParser.hasValidTime() && g_storageReady) {
    DateTimeInfo t = g_timeParser.current();
    String path = g_storage.createNewLogFile(t);
    if (path.length() > 0) g_logFileOpened = true;
  }

  if (!g_logFileOpened && g_storageReady && !g_timeParser.hasValidTime()) {
    DateTimeInfo dummy;
    String path = g_storage.createNewLogFile(dummy);
    if (path.length() > 0) g_logFileOpened = true;
  }

  if (g_logFileOpened) g_storage.appendLine(line);
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