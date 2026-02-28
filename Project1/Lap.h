#pragma once
#include <vector>
#include <optional>
#include "Sector.h"

using TimeMs = uint32_t; // 或用 uint64_t 若擔心 session >49 天

class Lap {
public:
    Lap(int numSectors);
    Lap(int numSectors, TimeMs startTime);
    bool start(TimeMs t);
    bool stop(TimeMs t);
    bool setSectorTime(unsigned int index, TimeMs t);
    TimeMs getLapTime() const;
    std::optional<TimeMs> getSectorTime(unsigned int index) const;

private:
    TimeMs lapStartTime;
    TimeMs lapEndTime;
    unsigned short sectorCount;
    std::vector<std::optional<TimeMs>> sectorTimes;
};