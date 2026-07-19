#pragma once

#include <vector>
#include <Arduino.h>
#include "StorageManager.h"
#include "Line2D.h"
#include "GPSPoint.h"

struct SectorConfig {
    double lat;     // 中心緯度 (decimal degrees)
    double lon;     // 中心經度 (decimal degrees)
    float heading;  // 線段方向 (degrees)
    float width;    // 檢測寬度/線長度 (meters)
};

struct TrackConfig {
    String name;
    bool isCircuit;
    std::vector<SectorConfig> sectors;
};

/**
 * 從 SPIFFS 的 /track.json 讀取設定到 cfg。
 * 若檔案不存在或解析失敗，會用「內建 TKS」建立預設 cfg 並寫入 track.json。
 */
bool loadTrackConfig(StorageManager &storage, TrackConfig &cfg);

/**
 * 把 cfg 寫回 /track.json（覆蓋原檔）。
 */
bool saveTrackConfig(StorageManager &storage, const TrackConfig &cfg);

/**
 * 依 TrackConfig 的 sectors 建立 Track 用的 nodes (std::vector<Line2D>)。
 */
std::vector<Line2D> makeTrackNodes(const TrackConfig &cfg);