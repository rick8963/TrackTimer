# Task 2: GPS Sample Flow (Input → Timing Engine)

**Change ID:** `track-mode-refactor`  
**Date:** 2026-07-08

## Flow Diagram

```
GPS UART (115200)
    │
    ▼
GpsReceiver::loop()          ← 讀取 serial，逐行 callback
    │
    ▼
handleNmeaLine()             ← main.cpp
    │
    ├──► GpsTimeParser::processLine()   ← 解析 RMC/GGA/ZDA
    │         │
    │         ├── GpsData (fix, speed, lat/lon, sats)
    │         └── DateTimeInfo + TimeMs timestamp
    │
    └──► StorageManager (若錄製中)

main loop (每圈):
    │
    ├── g_gpsReceiver.loop()
    ├── gps = g_timeParser.currentGps()
    │
    └── if (gpsSerialActive && hasValidFix && hasValidSpeed && hasValidTime)
              Point2D pos = GPSPoint(lat, lon)
              track.updatePos(pos, timestamp)    ← Timing Engine 邊界
```

## Layer Boundaries (重構後)

| Layer | Component | Input | Output |
|-------|-----------|-------|--------|
| Input | `GpsReceiver`, `GpsTimeParser` | NMEA bytes | `GpsData`, `TimeMs` |
| Timing Engine | `TrackTimingEngine` + `Track` | `Point2D`, timestamp, validity | `TrackTimingModel`, `LapInfo` |
| Display | `DisplayManager` | `LapInfo`, `GpsData` | OLED pixels |

## Valid Sample Gate

進入 timing engine 需同時滿足（`main.cpp` loop）：

1. `gpsSerialActive` — GPS UART 有資料
2. `gps.hasValidFix` — fix quality > 0
3. `gps.hasValidSpeed` — RMC 有效
4. `g_timeParser.hasValidTime()` — 時間已解析

任一不成立 → 不呼叫 `updatePos()`；display 應保留 `lastValidDisplay` 快照。

## Coordinate Conversion

`GPSPoint(lat, lon, true)` → 內部 `Point2D`（公分平面座標），供 `Line2D` 幾何運算。

## Display Refresh

- Status / display 更新週期：100ms（`main.cpp` `lastStatus`）
- `DisplayManager` Track 模式重繪間隔：100ms
- `trackToLapInfo()` / `TrackTimingEngine::toLapInfo()` 在 display 更新路徑被呼叫

## Invalid GPS Behavior (規格要求)

當 validity 喪失：

- Timing engine **不覆寫** last valid lap 顯示值
- `TrackTimingEngine` 維護 `_lastValidDisplay` 快照
- `trackState` → `invalid_gps`（若曾有 running 狀態）
