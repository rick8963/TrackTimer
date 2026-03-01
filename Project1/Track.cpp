#include "Track.h"

static TimeMs adjustTimeForMidnight(TimeMs currentTime, TimeMs lastTime) {
    const TimeMs HALF_DAY = 12u * 60u * 60u * 1000u;
    const TimeMs ONE_DAY = 24u * 60u * 60u * 1000u;

    if (lastTime > 0 && lastTime > currentTime && (lastTime - currentTime) > HALF_DAY) {
        return currentTime + ONE_DAY; // 加法效能較高
    }
    return currentTime;
}

Track::Track() :
    sectorCount(0),
    currentSector(0),
    currentPos(Point2D(0, 0)),
    lastPos(Point2D(0, 0)),
    lastLapValid(true),
    sessionStartTime(0),
    sessionEndTime(0),
    bestLapTime(0),
    latestLapTime(0),
    lastTimestamp(0),
    lastCrossingTime(0),
    firstLapCompleted(false)
{}

Track::Track(vector<Line2D> nds, bool isCircuit) :
    sectorCount(isCircuit ? nds.size() : nds.size() - 1),
    nodes(std::move(nds)),
    currentSector(sectorCount - 1),
    currentPos(Point2D(0, 0)),
    lastPos(Point2D(0, 0)),
    lastLapValid(true),
    sessionStartTime(0),
    sessionEndTime(0),
    bestLapTime(0),
    latestLapTime(0),
    lastTimestamp(0),
    lastCrossingTime(0),
    firstLapCompleted(false)
{
    for (int i = 0; i < sectorCount - 1; i++) {
        sectors.push_back(Sector(i, i + 1));
    }
    if (isCircuit) {
        sectors.push_back(Sector(sectorCount - 1, 0));
    }
}

bool Track::passSector(unsigned int i, TimeMs timestamp) {
    if (i >= nodes.size()) return false;
    Line2D* sector = &nodes.at(i);
    
    // 快速過濾：如果上一個點和當前點都不在區間內，直接放棄
    if (!sector->isPointInInterval(lastPos) && !sector->isPointInInterval(currentPos)) {
        return false;
    }
    
    const int64_t cross1 = sector->crossValue(lastPos);
    const int64_t cross2 = sector->crossValue(currentPos);
    
    // 使用 XOR 檢查符號是否不同 (最高位是否不同)
    // 由於 0 乘以任何數都是 0，這裡用大於 0 來確保兩者同號（都在同一側）
    if (cross1 * cross2 > 0) {
        return false;
    }
    
    if (!currentLapIndex.has_value()) return true;

    TimeMs accurateTime = interpolateCrossingTime(lastPos, currentPos, *sector, 
                                                   lastTimestamp, timestamp, cross1, cross2);
    lastCrossingTime = accurateTime;

    Lap& lap = laps[currentLapIndex.value()];
    lap.setSectorTime(currentSector, accurateTime);
    return true;
}

TimeMs Track::interpolateCrossingTime(const Point2D& prevPos, const Point2D& currPos,
    const Line2D& line, TimeMs prevTime, TimeMs currTime,
    int64_t cross1, int64_t cross2) const {
    
    // 計算到線段的距離（公分）
    const uint32_t dist1_cm = line.distanceToLineCm(prevPos);
    const uint32_t dist2_cm = line.distanceToLineCm(currPos);
    const uint32_t totalDist_cm = dist1_cm + dist2_cm;
    
    if (totalDist_cm < 1) {
        return currTime;
    }

    // 全整數計算內插，避免 float 轉換與除法
    // offset = timeDiff * dist1 / totalDist
    const uint32_t timeDiff = currTime - prevTime;
    // 使用 uint64_t 避免 timeDiff * dist1_cm 溢位
    const uint32_t offset = static_cast<uint32_t>(((static_cast<uint64_t>(timeDiff) * dist1_cm) + (totalDist_cm >> 1)) / totalDist_cm);

    return prevTime + offset;
}

unsigned int Track::getSectorCount() const {
    return sectorCount;
}

unsigned int Track::getCurrentSectorCount() const {
    return currentSector;
}

bool Track::isAllSectorsPassed() const {
    for (const auto& sector : sectors) {
        if (!sector.isPassed()) return false;
    }
    return true;
}

void Track::removeFirstIncompleteLap() {
    if (firstLapCompleted) return;

    firstLapCompleted = true;
    if (laps.empty()) return;

    laps.erase(laps.begin());
    if (currentLapIndex.has_value() && currentLapIndex.value() > 0) {
        currentLapIndex = currentLapIndex.value() - 1;
    }
}

void Track::nextLap(TimeMs timestamp) {
    if (currentLapIndex.has_value()) {
        Lap& prevLap = laps[currentLapIndex.value()];
        TimeMs lapTime = prevLap.getLapTime();

        if (lapTime > 0) {
            latestLapTime = lapTime;
            if (lastLapValid && (bestLapTime == 0 || lapTime < bestLapTime)) {
                bestLapTime = lapTime;
            }
        }
    }

    for (auto& sector : sectors) {
        sector.reset();
    }

    laps.push_back(Lap(sectorCount, timestamp));
    currentLapIndex = laps.size() - 1;
}

const vector<Sector>& Track::getSectors() const {
    return sectors;
}

Line2D Track::getNextCheckpoint() const {
    return (currentSector + 1 == sectorCount) ? nodes.at(0) : nodes.at(currentSector + 1);
}

Point2D Track::getCurrentPos() const {
    return Point2D(currentPos);
}

void Track::updatePos(Point2D& pos, TimeMs timestamp) {
    timestamp = adjustTimeForMidnight(timestamp, lastTimestamp);

    if (sessionStartTime == 0) {
        sessionStartTime = timestamp;
    }

    lastPos = currentPos;
    currentPos = pos;

    // 緩存常用變數避免重複呼叫
    const TimeMs currentTimestamp = timestamp;

    if (passSector(0, currentTimestamp)) {
        bool completedFullLap = (currentLapIndex.has_value() && currentSector == sectorCount - 1);

        if (completedFullLap) {
            if (currentSector < sectors.size()) {
                sectors[currentSector].pass();
            }
            lastLapValid = isAllSectorsPassed();
            removeFirstIncompleteLap();
        }

        nextLap(lastCrossingTime);
        currentSector = 0;
        lastTimestamp = timestamp;
        return;
    }

    if (currentSector == sectorCount - 1) {
        lastTimestamp = timestamp;
        return;
    }

    if (passSector(currentSector + 1, currentTimestamp)) {
        if (currentSector < sectors.size()) {
            sectors[currentSector].pass();
        }
        currentSector++;
    }

    lastTimestamp = timestamp;
}

const std::vector<Lap>& Track::getLaps() const {
    return laps;
}

TimeMs Track::getBestLapTime() const {
    return bestLapTime;
}

TimeMs Track::getLatestLapTime() const {
    return latestLapTime;
}

TimeMs Track::getSessionStartTime() const {
    return sessionStartTime;
}

TimeMs Track::getSessionEndTime() const {
    return sessionEndTime;
}
