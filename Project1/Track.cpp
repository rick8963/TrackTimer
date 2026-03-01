#include "Track.h"

static TimeMs adjustTimeForMidnight(TimeMs currentTime, TimeMs lastTime) {
    const TimeMs HALF_DAY = 12u * 60u * 60u * 1000u;
    const TimeMs ONE_DAY = 24u * 60u * 60u * 1000u;

    if (lastTime > 0 && lastTime > currentTime && (lastTime - currentTime) > HALF_DAY) {
        return currentTime + ONE_DAY;
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
    if (!sector->isPointInInterval(lastPos) && !sector->isPointInInterval(currentPos)) return false;
    
    // 計算交叉乘積，檢查是否穿越線段
    const float cross1 = sector->crossValue(lastPos);
    const float cross2 = sector->crossValue(currentPos);
    
    if (cross1 * cross2 > 0) return false;  // 同側，未穿越
    
    if (!currentLapIndex.has_value()) return true;

    // 使用內插計算精確通過時間（重用 cross1, cross2 的結果）
    TimeMs accurateTime = interpolateCrossingTime(lastPos, currentPos, *sector, 
                                                   lastTimestamp, timestamp, cross1, cross2);
    lastCrossingTime = accurateTime;

    Lap& lap = laps[currentLapIndex.value()];
    lap.setSectorTime(currentSector, accurateTime);
    return true;
}

TimeMs Track::interpolateCrossingTime(const Point2D& prevPos, const Point2D& currPos,
    const Line2D& line, TimeMs prevTime, TimeMs currTime,
    float cross1, float cross2) const {
    
    // 已知 cross1 * cross2 <= 0，表示穿越線段
    if (cross1 * cross2 > 0) {
        return currTime;  // 不應發生
    }

    // 計算到線段的距離
    const float dist1 = line.distanceToLine(prevPos);
    const float dist2 = line.distanceToLine(currPos);
    const float totalDist = dist1 + dist2;
    
    if (totalDist < 1e-6f) {
        return currTime;  // 距離太小
    }

    // 計算距離比例（用 float）
    const float ratio = dist1 / totalDist;
    
    // 時間計算用整數運算，避免精度損失
    const uint32_t timeDiff = currTime - prevTime;
    const uint32_t offset = static_cast<uint32_t>(timeDiff * ratio + 0.5f);
    const TimeMs interpolatedTime = prevTime + offset;

    return interpolatedTime;
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

    TimeMs currentTimestamp = timestamp;

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
