#pragma once
#include <vector>
#include <optional>
#include "Sector.h"

using TimeMs = uint32_t;

struct SectorTime {
    TimeMs value = 0;
    bool valid = false;
};

class Lap {
public:
    Lap(int numSectors);
    Lap(int numSectors, TimeMs startTime);

    bool start(TimeMs t);
    bool stop(TimeMs t);
    bool setSectorTime(unsigned int index, TimeMs t);

    TimeMs getLapTime() const;
    bool hasSectorTime(unsigned int index) const;
    TimeMs getSectorTime(unsigned int index) const;

private:
    bool started;
    bool finished;
    TimeMs lapStartTime;
    TimeMs lapEndTime;
    unsigned short sectorCount;
    std::vector<SectorTime> sectorTimes;
};