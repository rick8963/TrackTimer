#pragma once
#include<vector>
#include <optional>
#include"Sector.h"

class Lap {
public:
    Lap(int numSectors);
    Lap(int numSectors, double startTime);
    bool start(double t);
    bool stop(double t);
    bool setSectorTime(unsigned int index, double t);
    double getLapTime() const;
    std::optional<double> getSectorTime(unsigned int index) const;

private:
    double lapStartTime;
    double lapEndTime;
    unsigned short sectorCount;
    std::vector<std::optional<double>> sectorTimes;
};