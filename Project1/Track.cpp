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
    lastTimestamp(0)
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
    lastTimestamp(0)
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
    if (sector->crossValue(lastPos) * sector->crossValue(currentPos) > 0) return false;
    if (!currentLapIndex.has_value()) return true;

    Lap& lap = laps[currentLapIndex.value()];
    lap.setSectorTime(currentSector, timestamp);
    return true;
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
    lastTimestamp = timestamp;

    if (passSector(0, timestamp)) {
        if (currentLapIndex.has_value() && currentSector == sectorCount - 1) {
            Lap& lap = laps[currentLapIndex.value()];
            lap.setSectorTime(sectorCount - 1, timestamp);
            if (currentSector < sectors.size()) {
                sectors[currentSector].pass();
            }
        }

        lastLapValid = isAllSectorsPassed();
        nextLap(timestamp);
        currentSector = 0;
        return;
    }

    if (currentSector == sectorCount - 1) return;

    if (passSector(currentSector + 1, timestamp)) {
        if (currentSector < sectors.size()) {
            sectors[currentSector].pass();
        }
        currentSector++;
    }
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