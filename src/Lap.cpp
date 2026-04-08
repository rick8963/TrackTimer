#include "Lap.h"

Lap::Lap(int numSectors)
    : lapStartTime(0),
      lapEndTime(0),
      sectorCount(numSectors),
      sectorTimes(numSectors),
      started(false),
      finished(false) {}

Lap::Lap(int numSectors, TimeMs startTime)
    : lapStartTime(startTime),
      lapEndTime(0),
      sectorCount(numSectors),
      sectorTimes(numSectors),
      started(true),
      finished(false) {}

bool Lap::start(TimeMs t) {
    if (started) return false;
    lapStartTime = t;
    lapEndTime = 0;
    started = true;
    finished = false;
    return true;
}

bool Lap::setSectorTime(unsigned int index, TimeMs t) {
    if (index >= sectorCount) return false;
    if (!started || finished) return false;

    TimeMs cumulativeBefore = 0;
    for (unsigned int i = 0; i < index; ++i) {
        if (!sectorTimes[i].valid) return false;
        cumulativeBefore += sectorTimes[i].value;
    }

    TimeMs lastSectorEndTime = lapStartTime + cumulativeBefore;
    if (t < lastSectorEndTime) return false;

    TimeMs duration = t - lastSectorEndTime;
    sectorTimes[index].value = duration;
    sectorTimes[index].valid = true;

    if (index == sectorCount - 1) {
        lapEndTime = t;
        finished = true;
    }

    return true;
}

bool Lap::stop(TimeMs t) {
    if (!started || finished) return false;
    if (t < lapStartTime) return false;

    lapEndTime = t;
    finished = true;
    return true;
}

TimeMs Lap::getLapTime() const {
    if (!started || !finished) return 0;
    return lapEndTime - lapStartTime;
}

bool Lap::hasSectorTime(unsigned int index) const {
    if (index >= sectorCount) return false;
    return sectorTimes[index].valid;
}

TimeMs Lap::getSectorTime(unsigned int index) const {
    if (index >= sectorCount) return 0;
    if (!sectorTimes[index].valid) return 0;
    return sectorTimes[index].value;
}