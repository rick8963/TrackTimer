#include "Track.h"

static TimeMs adjustTimeForMidnight(TimeMs currentTime, TimeMs lastTime) {
    const TimeMs HALF_DAY = 12u * 60u * 60u * 1000u;
    const TimeMs ONE_DAY = 24u * 60u * 60u * 1000u;
    
    if (lastTime > 0 && lastTime > currentTime && (lastTime - currentTime) > HALF_DAY) {
        return currentTime + ONE_DAY;
    }
    return currentTime;
}

Track::Track(std::vector<Line2D> nds, bool isCircuit) 
    : sectorCount(isCircuit ? nds.size() : nds.size() - 1),
      nodes(std::move(nds)),
      currentSector(sectorCount - 1),
      currentPos(Point2D(0, 0)),
      lastPos(Point2D(0, 0)),
      lastLapValid(true),
      currentLapIndex(-1),
      sessionStartTime(0),
      sessionEndTime(0),
      bestLapTime(0),
      latestLapTime(0),
      lastTimestamp(0),
      lastCrossingTime(0),
      firstLapCompleted(false)
{
    // 建立 sectors
    for (unsigned int i = 0; i < sectorCount - 1; i++) {
        sectors.emplace_back(i, i + 1);
    }
    if (isCircuit) {
        sectors.emplace_back(sectorCount - 1, 0);
    }
}

bool Track::passSector(unsigned int i, TimeMs timestamp) {
    if (i >= nodes.size()) return false;
    
    Line2D& sector = nodes[i];
    
    // 快速過濾
    if (!sector.isPointInInterval(lastPos) && !sector.isPointInInterval(currentPos)) {
        return false;
    }
    
    int64_t cross1 = sector.crossValue(lastPos);
    int64_t cross2 = sector.crossValue(currentPos);
    
    // 檢查是否跨線 (符號不同)
    if (cross1 * cross2 > 0) {
        return false;
    }
    
    if (currentLapIndex < 0) return true;  // 無當前lap
    
    TimeMs accurateTime = interpolateCrossingTime(lastPos, currentPos, sector, 
        lastTimestamp, timestamp, cross1, cross2);
    lastCrossingTime = accurateTime;
    
    Lap& lap = laps[currentLapIndex];
    lap.setSectorTime(currentSector, accurateTime);
    return true;
}

TimeMs Track::interpolateCrossingTime(const Point2D& prevPos, const Point2D& currPos,
    const Line2D& line, TimeMs prevTime, TimeMs currTime,
    int64_t cross1, int64_t cross2) const {
    
    uint32_t dist1_cm = line.distanceToLineCm(prevPos);
    uint32_t dist2_cm = line.distanceToLineCm(currPos);
    uint32_t totalDist_cm = dist1_cm + dist2_cm;
    
    if (totalDist_cm < 1) {
        return currTime;
    }
    
    uint32_t timeDiff = currTime - prevTime;
    uint32_t offset = static_cast<uint32_t>((static_cast<uint64_t>(timeDiff) * dist1_cm + (totalDist_cm >> 1)) / totalDist_cm);
    
    return prevTime + offset;
}

void Track::nextLap(TimeMs timestamp) {
    if (currentLapIndex >= 0) {
        Lap& prevLap = laps[currentLapIndex];
        TimeMs lapTime = prevLap.getLapTime();
        
        if (lapTime > 0) {
            latestLapTime = lapTime;
            if (lastLapValid && (bestLapTime == 0 || lapTime < bestLapTime)) {
                bestLapTime = lapTime;
            }
        }
    }
    
    // 重置 sectors
    for (auto& sector : sectors) {
        sector.reset();
    }
    
    laps.emplace_back(sectorCount, timestamp);
    currentLapIndex = laps.size() - 1;
}

void Track::updatePos(Point2D& pos, TimeMs timestamp) {
    timestamp = adjustTimeForMidnight(timestamp, lastTimestamp);
    
    if (sessionStartTime == 0) {
        sessionStartTime = timestamp;
    }
    
    lastPos = currentPos;
    currentPos = pos;
    
    if (passSector(0, timestamp)) {
        // 起點跨線 = 新圈
        bool completedFullLap = (currentLapIndex >= 0 && currentSector == sectorCount - 1);
        
        if (completedFullLap && currentSector < sectors.size()) {
            sectors[currentSector].pass();
        }
        
        lastLapValid = isAllSectorsPassed();
        removeFirstIncompleteLap();
        nextLap(lastCrossingTime);
        currentSector = 0;
        lastTimestamp = timestamp;
        return;
    }
    
    if (currentSector == sectorCount - 1) {
        lastTimestamp = timestamp;
        return;
    }
    
    if (passSector(currentSector + 1, timestamp)) {
        if (currentSector < sectors.size()) {
            sectors[currentSector].pass();
        }
        currentSector++;
    }
    
    lastTimestamp = timestamp;
}

// 其餘 getter 實作保持不變...
unsigned int Track::getSectorCount() const { return sectorCount; }
unsigned int Track::getCurrentSectorCount() const { return currentSector; }
const std::vector<Sector>& Track::getSectors() const { return sectors; }
Line2D Track::getNextCheckpoint() const { 
    return (currentSector + 1 == sectorCount) ? nodes[0] : nodes[currentSector + 1]; 
}
Point2D Track::getCurrentPos() const { return currentPos; }
const std::vector<Lap>& Track::getLaps() const { return laps; }
TimeMs Track::getBestLapTime() const { return bestLapTime; }
TimeMs Track::getLatestLapTime() const { return latestLapTime; }
TimeMs Track::getSessionStartTime() const { return sessionStartTime; }
TimeMs Track::getSessionEndTime() const { return sessionEndTime; }

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
    if (currentLapIndex > 0) {
        currentLapIndex--;
    }
}