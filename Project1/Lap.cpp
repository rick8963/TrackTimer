#include "Lap.h"

Lap::Lap(int numSectors)
	: lapStartTime(0),
	lapEndTime(0),
	sectorCount(numSectors),
	sectorTimes(numSectors) // initialize as nullopt
{}

Lap::Lap(int numSectors, clock_t startTime)
	: lapStartTime(startTime),
	lapEndTime(0),
	sectorCount(numSectors),
	sectorTimes(numSectors) // initialize as nullopt
{}

bool Lap::start(clock_t t)
{
	if (lapStartTime != 0) return false;
	lapStartTime = t;
	return true;
}

bool Lap::setSectorTime(unsigned int index, clock_t t)
{
	if (index >= sectorCount) return false;
	if (lapStartTime == 0) return false; // lap not started

	// Ensure all previous sectors have values
	for (unsigned int i = 0; i < index; ++i)
		if (!sectorTimes[i].has_value()) return false;

	clock_t cumulativeBefore = 0;
	for (unsigned int i = 0; i < index; ++i)
		cumulativeBefore += sectorTimes[i].value_or(0);

	clock_t duration = (t - lapStartTime) - cumulativeBefore;
	if (duration < 0) return false; // guard

	sectorTimes[index] = duration;

	if (index == sectorCount - 1)
		lapEndTime = t;

	return true;
}

bool Lap::stop(clock_t t)
{
	if (lapStartTime == 0 || lapEndTime != 0) return false;
	lapEndTime = t;
	return true;
}

clock_t Lap::getLapTime() const
{
	if (lapStartTime == 0 || lapEndTime <= lapStartTime) return 0;
	return lapEndTime - lapStartTime;
}

std::optional<clock_t> Lap::getSectorTime(unsigned int index) const
{
	if (index >= sectorCount) std::nullopt;
	return sectorTimes[index];
}
