#include <ArduinoJson.h>
#include "TrackConfig.h"

static const char *TRACK_CONFIG_PATH = "/track.json";

// -----------------------------------------------------------------------------
// 共用 JSON <-> SPIFFS 工具
// -----------------------------------------------------------------------------
static bool readJsonFromFile(fs::FS &fs, const char *path, StaticJsonDocument<2048> &doc) {
    File f = fs.open(path, FILE_READ);
    if (!f || f.isDirectory()) {
        Serial.println("[TrackConfig] Config file not found or is a directory");
        return false;
    }

    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) {
        Serial.printf("[TrackConfig] JSON parse error: %s\n", err.c_str());
        return false;
    }
    return true;
}

static bool writeJsonToFile(fs::FS &fs, const char *path, const StaticJsonDocument<2048> &doc) {
    File f = fs.open(path, FILE_WRITE);
    if (!f) {
        Serial.println("[TrackConfig] Failed to open config file for writing");
        return false;
    }

    size_t n = serializeJson(doc, f);
    f.close();

    if (n == 0) {
        Serial.println("[TrackConfig] Failed to write JSON to file");
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
// 內建 TKS 預設設定（只有在 track.json 不存在或壞掉時用）
// -----------------------------------------------------------------------------
static void fillDefaultTKS(TrackConfig &cfg) {
    cfg.name = "TKS";
    cfg.isCircuit = true;
    cfg.sectors.clear();

    // 對應你原本的 buildTKS()：Line2D(GPSPoint(lat, lon, true), heading, 20)
    cfg.sectors.push_back({22.742248, 120.322181,   0.0f, 20.0f});
    cfg.sectors.push_back({22.742798, 120.321496, 180.0f, 20.0f});
    cfg.sectors.push_back({22.742724, 120.322010, 180.0f, 20.0f});
    cfg.sectors.push_back({22.742285, 120.321387,  60.0f, 20.0f});
    cfg.sectors.push_back({22.742540, 120.321959,  88.0f, 20.0f});
    cfg.sectors.push_back({22.741863, 120.321912, 262.0f, 20.0f});
    cfg.sectors.push_back({22.741763, 120.321930,  81.0f, 20.0f});
}

// -----------------------------------------------------------------------------
// 將 TrackConfig 寫入 JSON 檔
// -----------------------------------------------------------------------------
bool saveTrackConfig(StorageManager &storage, const TrackConfig &cfg) {
    fs::FS &fs = storage.fs();
    StaticJsonDocument<2048> doc;

    doc["version"] = 1;
    doc["name"] = cfg.name;
    doc["isCircuit"] = cfg.isCircuit;

    JsonArray arr = doc.createNestedArray("sectors");
    for (const auto &s : cfg.sectors) {
        JsonObject o = arr.createNestedObject();
        o["lat"] = s.lat;
        o["lon"] = s.lon;
        o["heading"] = s.heading;
        o["width"] = s.width;
    }

    bool ok = writeJsonToFile(fs, TRACK_CONFIG_PATH, doc);
    if (!ok) {
        Serial.println("[TrackConfig] Failed to save track.json");
    } else {
        Serial.printf("[TrackConfig] Saved '%s' (%d sectors, isCircuit=%s)\n",
                      cfg.name.c_str(),
                      (int)cfg.sectors.size(),
                      cfg.isCircuit ? "true" : "false");
    }
    return ok;
}

// -----------------------------------------------------------------------------
// 從 JSON 檔載入 TrackConfig（無檔案時建立預設 TKS 並存檔）
// -----------------------------------------------------------------------------
bool loadTrackConfig(StorageManager &storage, TrackConfig &cfg) {
    fs::FS &fs = storage.fs();
    StaticJsonDocument<2048> doc;

    // 試著讀取既有 track.json
    if (!readJsonFromFile(fs, TRACK_CONFIG_PATH, doc)) {
        Serial.println("[TrackConfig] track.json missing or invalid, using built-in TKS");
        fillDefaultTKS(cfg);
        // 順便寫一份預設檔案到 SPIFFS，方便之後 Web 編輯
        saveTrackConfig(storage, cfg);
        return true;
    }

    // 解析 JSON 到 cfg
    cfg.name = doc["name"] | String("Track");
    cfg.isCircuit = doc["isCircuit"] | true;

    cfg.sectors.clear();
    JsonArray arr = doc["sectors"].as<JsonArray>();
    for (JsonVariant v : arr) {
        SectorConfig s{};
        s.lat     = v["lat"]     | 0.0;
        s.lon     = v["lon"]     | 0.0;
        s.heading = v["heading"] | 0.0f;
        s.width   = v["width"]   | 20.0f;
        cfg.sectors.push_back(s);
    }

    Serial.printf("[TrackConfig] Loaded track '%s' (%d sectors, isCircuit=%s)\n",
                  cfg.name.c_str(),
                  (int)cfg.sectors.size(),
                  cfg.isCircuit ? "true" : "false");
    return true;
}

// -----------------------------------------------------------------------------
// 將 SectorConfig 陣列轉成 Track 使用的 nodes (Line2D)
// -----------------------------------------------------------------------------
std::vector<Line2D> makeTrackNodes(const TrackConfig &cfg) {
    std::vector<Line2D> nodes;
    nodes.reserve(cfg.sectors.size());
    for (const auto &s : cfg.sectors) {
        GPSPoint p(s.lat, s.lon, true);            // lat/lon (decimal degrees)
        nodes.emplace_back(p, s.heading, s.width); // Line2D(Point2D, direction_deg, width_meters)
    }
    return nodes;
}