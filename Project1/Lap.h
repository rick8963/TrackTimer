#pragma once
#include<vector>
#include<ctime>
#include <optional>
#include"Sector.h"

class Lap {
public:
    Lap(int numSectors);
    Lap(int numSectors, clock_t startTime);
    bool start(clock_t t);
    bool stop(clock_t t);
    bool setSectorTime(unsigned int index, clock_t t);
    clock_t getLapTime() const;
    std::optional<clock_t> getSectorTime(unsigned int index) const;

private:
    clock_t lapStartTime;
    clock_t lapEndTime;
    unsigned short sectorCount;
    std::vector<std::optional<clock_t>> sectorTimes;
};