# Task 1: Current Track Timing Implementation

**Change ID:** `track-mode-refactor`  
**Date:** 2026-07-08

## Lap Trigger (圈數觸發)

圈數完成由 `Track::updatePos()` 驅動，核心邏輯在 `Track::passSector()`：

1. 每次有效 GPS 樣本更新 `lastPos` → `currentPos`。
2. 先檢查 sector 0（起點/終點線，`nodes[0]`）是否被連續兩點路徑穿越：
   - 至少一點在線段區間內（`Line2D::isPointInInterval`）。
   - 兩點在線兩側（`crossValue(prev) * crossValue(curr) <= 0`）。
3. 若 sector 0 被穿越且 `currentSector == sectorCount - 1`（所有 sector 已通過），視為完成一整圈。
4. 跨線時間以距離比例內插（`Track::interpolateCrossingTime`）提高精度。
5. 完成圈數時呼叫 `Track::nextLap()` 建立新圈並更新 best/latest lap。

Sector 中間點由 `passSector(currentSector + 1)` 遞增 `currentSector`。

首圈不完整資料由 `removeFirstIncompleteLap()` 在第一次完整圈後移除。

## Best-Lap Update (最佳圈更新)

在 `Track::nextLap()`：

```cpp
if (lapTime > 0) {
    latestLapTime = lapTime;
    if (lastLapValid && (bestLapTime == 0 || lapTime < bestLapTime)) {
        bestLapTime = lapTime;
        bestLapNum = currentLapIndex;
    }
}
```

- `lastLapValid` 由 `isAllSectorsPassed()` 決定（所有 sector 是否都有通過紀錄）。
- 僅 valid lap 可更新 best lap。
- 第一圈若 sector 未全通過則 `lastLapValid == false`，不計入 best。

## Delta Calculation (差距計算)

**目前尚未實作。** `main.cpp` 的 `trackToLapInfo()` 硬編碼：

```cpp
lap.deltaStr = "+0.123";
lap.deltaSeconds = 0.123f;
```

設計規格要求：`delta = currentLapTime - bestLapTime`；無 best lap 時顯示中性 placeholder。

## Display Path (顯示路徑)

```
Track (timing) → trackToLapInfo() → LapInfo → DisplayManager::drawTrackMode()
```

- `main.cpp` loop 每 100ms：若 `gps.hasValidFix`，才呼叫 `trackToLapInfo()`。
- GPS 無效時 `LapInfo` 為預設零值，**未保留上次有效顯示**（與規格不符，待修正）。

## RGB LED

`StatusLED::update()` 以 `gpsFixValid` 控制 LED 5（藍閃=有效、紅=無效），與 track timing 狀態間接相關，但未反映 running/idle 細分狀態。

## Key Files

| 檔案 | 職責 |
|------|------|
| `Track.cpp` / `Track.h` | 圈數偵測、sector、best/latest lap |
| `Line2D.cpp` | 線段幾何、crossValue、距離 |
| `Lap.cpp` | 單圈 sector 時間累計 |
| `main.cpp` | GPS 餵入、LapInfo 組裝、顯示更新 |
| `DisplayManager.cpp` | OLED Track 模式版面（不應修改） |
