#pragma once
#include <vector>
#include <cstdint>
#include "Lap.h"
#include "Sector.h"
#include "Point2D.h"
#include "Line2D.h"

using TimeMs = uint32_t;

class Track {
public:
    Track(std::vector<Line2D> nds, bool isCircuit = true);
    
    unsigned int getSectorCount() const;
    unsigned int getCurrentSectorCount() const;
    const std::vector<Sector>& getSectors() const;
    Line2D getNextCheckpoint() const;
    Point2D getCurrentPos() const;
    
    void updatePos(Point2D& pos, TimeMs timestamp);
    const std::vector<Lap>& getLaps() const;
    
    TimeMs getBestLapTime() const;
    TimeMs getLatestLapTime() const;
    TimeMs getSessionStartTime() const;
    TimeMs getSessionEndTime() const;

private:
    unsigned int sectorCount;
    unsigned int currentSector;
    bool lastLapValid;
    
    Point2D currentPos;
    Point2D lastPos;
    std::vector<Line2D> nodes;
    std::vector<Sector> sectors;
    
    bool passSector(unsigned int i, TimeMs timestamp);
    void nextLap(TimeMs timestamp);
    bool isAllSectorsPassed() const;
    bool firstLapCompleted;
    void removeFirstIncompleteLap();
    
    // 計算精確的跨線時間（全整數內插）
    TimeMs interpolateCrossingTime(const Point2D& prevPos, const Point2D& currPos,
        const Line2D& line, TimeMs prevTime, TimeMs currTime,
        int64_t cross1, int64_t cross2) const;

    int currentLapIndex;  // -1 = no current lap
    std::vector<Lap> laps;
    TimeMs sessionStartTime;
    TimeMs sessionEndTime;
    TimeMs bestLapTime;
    TimeMs latestLapTime;
    TimeMs lastTimestamp;
    TimeMs lastCrossingTime;
};