#include <Arduino.h>
#include <SPIFFS.h>
#include "Track.h"
#include "GPSPoint.h"

void printResults();

std::vector<Line2D> buildTKS() {
    // TKS 7 sectors（保持不變）
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

Track track(buildTKS(), true);
File nmeaFile;
int lineCount = 0;
uint32_t firstTimestamp = 0;  // ← 新增：記錄第一個時間戳

bool parseGPRMC(const String& line, double& lat, double& lon, uint32_t& relTimestamp) {
    if (!line.startsWith("$GPRMC")) return false;
    
    int c1 = line.indexOf(',');
    int c2 = line.indexOf(',', c1 + 1);
    int c3 = line.indexOf(',', c2 + 1);
    int c4 = line.indexOf(',', c3 + 1);
    int c5 = line.indexOf(',', c4 + 1);
    int c6 = line.indexOf(',', c5 + 1);
    
    String status = line.substring(c2 + 1, c3);
    if (status != "A") return false;
    
    // 解析絕對時間 hhmmss.sss → ms
    String timeStr = line.substring(c1 + 1, c2);
    if (timeStr.length() < 6) return false;
    
    uint32_t h = timeStr.substring(0,2).toInt();
    uint32_t m = timeStr.substring(2,4).toInt();
    float s = timeStr.substring(4).toFloat();
    uint32_t absTime = h*3600000u + m*60000u + (uint32_t)(s*1000);
    
    // 轉換成相對時間（第一個時間戳為 0）
    if (firstTimestamp == 0) {
        firstTimestamp = absTime;
    }
    relTimestamp = absTime - firstTimestamp;
    
    // 經緯度解析（修正你的格式）
    String latStr = line.substring(c3 + 1, c4);
    double lat_raw = latStr.toDouble();
    lat = (int)(lat_raw / 100) + (lat_raw / 100.0 - (int)(lat_raw / 100)) * 100 / 60.0;
    if (line.charAt(c4 + 1) == 'S') lat = -lat;
    
    String lonStr = line.substring(c5 + 1, c6);
    double lon_raw = lonStr.toDouble();
    lon = (int)(lon_raw / 100) + (lon_raw / 100.0 - (int)(lon_raw / 100)) * 100 / 60.0;
    if (line.charAt(c6 + 1) == 'W') lon = -lon;
    
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS failed");
        return;
    }
    
    nmeaFile = SPIFFS.open("/TKS.nmea", "r");
    if (!nmeaFile || nmeaFile.size() == 0) {
        Serial.println("TKS.nmea failed");
        return;
    }
    
    Serial.printf("=== TKS.nmea SPIFFS START ===\n");
    Serial.printf("File: %d bytes\n", nmeaFile.size());
}

void loop() {
    if (nmeaFile.available()) {
        String line = nmeaFile.readStringUntil('\n');
        line.trim();
        lineCount++;
        
        // Serial.printf("\n=== Line %d ===\n", lineCount);
        // Serial.println(line);
        
        double lat, lon;
        uint32_t timestamp;
        
        if (parseGPRMC(line, lat, lon, timestamp)) {
            GPSPoint pos(lat, lon, true);
            track.updatePos(pos, timestamp);
            // Serial.printf("TRACK: S%d/%d Laps=%d\n", 
            //     track.getCurrentSectorCount() + 1,
            //     track.getSectorCount(),
            //     track.getLaps().size());
        }
        
        if (nmeaFile.available() == 0) {
            printResults();
            while(1) delay(1000);
        }
    }
    // delay(100);
}

void printResults() {
    Serial.println("\n=== FINAL RESULTS ===");
    Serial.printf("Lines: %d\n", lineCount);
    Serial.printf("Total Laps: %d\n", track.getLaps().size());
    if (track.getLaps().size() > 0) {
        Serial.printf("Best: %.3f s\n", track.getBestLapTime() / 1000.0);
        Serial.printf("Latest: %.3f s\n", track.getLatestLapTime() / 1000.0);
        
        Serial.println("\nLAP DETAILS:");
        for (size_t i = 0; i < track.getLaps().size(); i++) {
            const auto& lap = track.getLaps()[i];
            Serial.printf("Lap %zu: %.3f s\n", i+1, lap.getLapTime() / 1000.0);
            for (uint8_t s = 0; s < track.getSectorCount(); s++) {
                if (lap.hasSectorTime(s)) {
                    Serial.printf("  S%d: %.3f s\n", s+1, lap.getSectorTime(s) / 1000.0);
                }
            }
        }
    }
}