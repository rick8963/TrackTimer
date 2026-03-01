#pragma once
#include "Lap.h"
#include "Sector.h"

using namespace std;

using TimeMs = uint32_t;

class Track {
public:
    Track(vector<Line2D> nds, bool isCircuit = true);
    unsigned int getSectorCount() const;
    unsigned int getCurrentSectorCount() const;
    const vector<Sector>& getSectors() const;
    Line2D getNextCheckpoint() const;
    Point2D getCurrentPos() const;
    void updatePos(Point2D& pos, TimeMs timestamp);
    const std::vector<Lap>& getLaps() const;
    TimeMs getBestLapTime() const;
    TimeMs getLatestLapTime() const;
    TimeMs getSessionStartTime() const;
    TimeMs getSessionEndTime() const;

private:
    Track();
    const unsigned int sectorCount;
    unsigned int currentSector;
    bool lastLapValid;

    Point2D currentPos;
    Point2D lastPos;
    vector<Line2D> nodes;
    vector<Sector> sectors;

    bool passSector(unsigned int i, TimeMs timestamp);
    void nextLap(TimeMs timestamp);
    bool isAllSectorsPassed() const;
    // 計算精確的跨線時間（使用距離內插）
    TimeMs interpolateCrossingTime(const Point2D& prevPos, const Point2D& currPos,
        const Line2D& line, TimeMs prevTime, TimeMs currTime) const;

    std::optional<size_t> currentLapIndex;
    std::vector<Lap> laps;
    TimeMs sessionStartTime;
    TimeMs sessionEndTime;
    TimeMs bestLapTime;
    TimeMs latestLapTime;
    TimeMs lastTimestamp;
    TimeMs lastCrossingTime;
};