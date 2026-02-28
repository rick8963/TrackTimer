#include "Lap.h"

Lap::Lap(int numSectors)
    : lapStartTime(0),
    lapEndTime(0),
    sectorCount(numSectors),
    sectorTimes(numSectors) // initialize as nullopt
{}

Lap::Lap(int numSectors, TimeMs startTime)
    : lapStartTime(startTime),
    lapEndTime(0),
    sectorCount(numSectors),
    sectorTimes(numSectors) // initialize as nullopt
{}

bool Lap::start(TimeMs t) {
    if (lapStartTime != 0) return false;
    lapStartTime = t;
    return true;
}

bool Lap::setSectorTime(unsigned int index, TimeMs t) {
    if (index >= sectorCount) return false;
    if (lapStartTime == 0) return false; // lap not started

    TimeMs cumulativeBefore = 0;
    for (unsigned int i = 0; i < index; ++i) {
        if (sectorTimes[i].has_value()) {
            cumulativeBefore += sectorTimes[i].value();
        }
    }

    TimeMs lastSectorEndTime = lapStartTime + cumulativeBefore;
    if (t < lastSectorEndTime) return false; // guard

    TimeMs duration = t - lastSectorEndTime;
    sectorTimes[index] = duration;

    if (index == sectorCount - 1) {
        lapEndTime = t;
    }

    return true;
}

bool Lap::stop(TimeMs t) {
    if (lapStartTime == 0 || lapEndTime != 0) return false;
    lapEndTime = t;
    return true;
}

TimeMs Lap::getLapTime() const {
    if (lapEndTime == 0) return 0;  // Lap not finished
    if (lapStartTime == 0) return 0;  // Invalid lap
    return lapEndTime - lapStartTime;
}

std::optional<TimeMs> Lap::getSectorTime(unsigned int index) const {
    if (index >= sectorCount) return std::nullopt;
    return sectorTimes[index];
}