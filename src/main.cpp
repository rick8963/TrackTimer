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

std::vector<Line2D> buildARK() {
    std::vector<Line2D> sectors;
    sectors.push_back(Line2D(GPSPoint(23.104046, 120.222489, true), 184,15));
    sectors.push_back(Line2D(GPSPoint(23.103841, 120.223270, true), 2, 30));
    return sectors;
}

std::vector<Line2D> buildLC_PARK() {
    std::vector<Line2D> sectors;
    sectors.push_back(Line2D(GPSPoint(22.854111, 120.254947, true), 185, 50));
    sectors.push_back(Line2D(GPSPoint(22.853006, 120.253759, true), 277, 50));
    sectors.push_back(Line2D(GPSPoint(22.854177, 120.253018, true), 27, 50));
    sectors.push_back(Line2D(GPSPoint(22.854996, 120.254341, true), 137, 50));
    return sectors;
}

std::vector<Line2D> buildLihPaoFull() {
    std::vector<Line2D> sectors;
sectors.push_back(Line2D(GPSPoint(24.319902, 120.688045, true), 139, 75));

    // sectors.push_back(Line2D(GPSPoint(24.318767, 120.686510, true), 139, 75));
    // sectors.push_back(Line2D(GPSPoint(24.317924, 120.682802, true), 283, 50));
    // sectors.push_back(Line2D(GPSPoint(24.318467, 120.680843, true), 56, 50));
    // sectors.push_back(Line2D(GPSPoint(24.319502, 120.683055, true), 66, 50));
    // sectors.push_back(Line2D(GPSPoint(24.320304, 120.685262, true), 72, 50));
    // sectors.push_back(Line2D(GPSPoint(24.320571, 120.689632, true), 136, 50));
    // sectors.push_back(Line2D(GPSPoint(24.318296, 120.689681, true), 272, 50));
    // sectors.push_back(Line2D(GPSPoint(24.318886, 120.688060, true), 86, 50));
    // sectors.push_back(Line2D(GPSPoint(24.319951, 120.689036, true), 0, 50));
    // sectors.push_back(Line2D(GPSPoint(24.319841, 120.687904, true), 230, 50));
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
bool g_recordArmed = false;           // 按鈕是否已啟用「錄製」
bool g_recordButtonState = false;   // 按鈕上一次讀到的狀態
uint32_t g_lastButtonChange = 0;      // 上一次按鈕狀態變化的時間
uint8_t g_buttonPin = RECORD_BUTTON_PIN;

bool g_wifiOn = false;                     // 一開始 WiFi 關閉
bool g_wifiArmed = true;                  // 是否允許使用 WiFi（按鈕控制）
bool g_wifiButtonState = false;           // WiFi 按鈕上次的狀態
uint32_t g_lastWifiButtonChange = 0;      // 上次 WiFi 按鈕變化時間
uint8_t g_wifiButtonPin = WIFI_BUTTON_PIN;

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

    const int completedLaps = static_cast<int>(track.getLaps().size());

    lap.totalLaps = completedLaps;
    lap.lastLapNum = track.hasCurrentLap() ? completedLaps - 1 : 0;
    lap.currentLapNum = track.hasCurrentLap() ? completedLaps : 0;

    lap.distToNextSector = track.getNextCheckpoint().distanceToLine(track.getCurrentPos());
    lap.distToNextSector = lap.distToNextSector < 999 ? lap.distToNextSector : 999;

    TimeMs currLapTime = 0;
    if (track.hasCurrentLap()) {
        currLapTime = g_timeParser.currentTimestamp() - track.getCurrentLapStartTime();
    }
    lap.currentLap = msToLapTime(currLapTime);

    lap.lastLap = msToLapTime(track.getLatestLapTime());
    lap.bestLap = msToLapTime(track.getBestLapTime());
    lap.bestLapNum = track.getBestLapNum();

    lap.deltaStr = "+0.123";
    lap.deltaSeconds = 0.123f;
    return lap;
}


void handleNmeaLine(const String &line);
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
    WiFi.mode(g_wifiOn ? WIFI_AP : WIFI_OFF);

    pinMode(g_buttonPin, INPUT_PULLUP);
    g_recordButtonState = digitalRead(g_buttonPin);
    g_lastButtonChange = millis();
    g_recordArmed = false;

    pinMode(g_wifiButtonPin, INPUT_PULLUP);
    g_wifiButtonState = digitalRead(g_wifiButtonPin);
    g_lastWifiButtonChange = millis();

    Serial.println("[Setup] Completed");
    g_statusLED.begin();
    g_display.begin();
}

void loop() {
    uint32_t loopStart = millis();
    
    bool gpsSerialActive = g_gpsReceiver.loop();
    GpsData gps = g_timeParser.currentGps();
    DateTimeInfo time = g_timeParser.current();
    if (gpsSerialActive) {
        if (gps.hasValidFix && gps.hasValidSpeed && g_timeParser.hasValidTime()) {
            Point2D pos = GPSPoint(g_timeParser.currentGps().latitude, g_timeParser.currentGps().longitude, true);
            track.updatePos(pos, g_timeParser.currentTimestamp());
        }
    }
    
    // Web 服務
    g_web.handleClient();

    uint32_t now = millis();
    int reading = digitalRead(g_buttonPin);
    if (reading != g_recordButtonState)g_lastButtonChange = now;
    if ((now - g_lastButtonChange) < 500) {
        if (reading == LOW && g_recordButtonState == HIGH) {
            Serial.println("DEBUG: REAL BUTTON PRESSED!");

            g_recordArmed = !g_recordArmed;
            if (g_recordArmed) {
                Serial.println("[Record Button] Armed, waiting for GPS valid time...");
            } else {
                if (g_logFileOpened) {
                    g_storage.closeCurrentFile();
                    g_logFileOpened = false;
                    Serial.println("[Record Button] Recording stopped and file closed.");
                } else {
                    Serial.println("[Record Button] Recording disabled.");
                }
            }
        }
        g_recordButtonState = reading;
    }

    int wifiReading = digitalRead(g_wifiButtonPin);
    if (wifiReading != g_wifiButtonState) {
        g_lastWifiButtonChange = now;
    }
    if ((now - g_lastWifiButtonChange) < 500) {
        if (wifiReading == LOW && g_wifiButtonState == HIGH) {
            Serial.println("DEBUG: WIFI BUTTON PRESSED!");
            g_wifiArmed = !g_wifiArmed;

            if (g_wifiArmed) {
                // 重新開啟 WiFi AP
                WiFi.mode(WIFI_AP);
                bool ok = WiFi.softAP(AP_SSID, AP_PASSWORD);
                if (!ok) {
                    Serial.println("❌ WiFi AP restart failed");
                } else {
                    IPAddress ip = WiFi.softAPIP();
                    Serial.printf("📶 WiFi AP restarted: %s @ %s\n", AP_SSID, ip.toString().c_str());
                }
            } else {
                // 關閉 WiFi
                WiFi.mode(WIFI_OFF);
                Serial.println("📴 WiFi turned OFF");
            }
        }
            g_wifiButtonState = wifiReading;
            g_lastWifiButtonChange = now; // 這邊寫死為 now，避免一直觸發
    }
    
    uint32_t dis_proc_interval;
    // 狀態更新（每100ms）
    static uint32_t lastStatus = 0;
    if (millis() - lastStatus > 100) {
        bool wifiActive = g_wifiArmed && (WiFi.getMode() != WIFI_OFF);
        bool sseActive = g_web.isSSEConnected();

        
        bool gpsFixValid = gps.hasValidFix;
        LapInfo lap;
        if(gpsFixValid) lap = trackToLapInfo(track);

        g_statusLED.update(
            g_storageReady, gpsFixValid, g_recordArmed, g_logFileOpened,
            wifiActive, sseActive, false);

        dis_proc_interval = millis();
        g_display.update(
            g_storageReady, gpsSerialActive, g_logFileOpened,
            wifiActive, sseActive,
            time, gps, lap, lineCount
        );
        dis_proc_interval = millis() - dis_proc_interval;
        
        lastStatus = millis();
    }   
    
    // 避免 WDT
    if (millis() - loopStart > 50) {
        Serial.printf("⚠️ Loop overrun: %dms, Display: %dms\n", millis() - loopStart, dis_proc_interval);
    }
}

void handleNmeaLine(const String &line) {
    g_timeParser.processLine(line);

    // Push to live view buffer (always, regardless of storage state)
    g_web.pushNmeaLine(line);

    // 只有在「按鈕已啟用錄製」、且「有有效時間」且「目前沒有開啟檔」時才建立新檔
    if (!g_logFileOpened && g_recordArmed && g_timeParser.hasValidTime() && g_storageReady) {
        DateTimeInfo t = g_timeParser.current();
        String path = g_storage.createNewLogFile(t);
        if (path.length() > 0) g_logFileOpened = true;
    }
    // 只有在檔案已開啟時才寫入
    if (g_logFileOpened) {
        g_storage.appendLine(line);
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