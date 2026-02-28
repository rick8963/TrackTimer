#include "Lap.h"

Lap::Lap(int numSectors)
	: lapStartTime(0),
	lapEndTime(0),
	sectorCount(numSectors),
	sectorTimes(numSectors) // initialize as nullopt
{}

Lap::Lap(int numSectors, double startTime)
	: lapStartTime(startTime),
	lapEndTime(0),
	sectorCount(numSectors),
	sectorTimes(numSectors) // initialize as nullopt
{}

bool Lap::start(double t)
{
	if (lapStartTime != 0) return false;
	lapStartTime = t;
	return true;
}

bool Lap::setSectorTime(unsigned int index, double t)
{
	if (index >= sectorCount) return false;
	if (lapStartTime == 0) return false; // lap not started

	double cumulativeBefore = 0;
	for (unsigned int i = 0; i < index; ++i)
		cumulativeBefore += sectorTimes[i].value_or(0);

	double lastSectorEndTime = lapStartTime + cumulativeBefore;
	double duration = t - lastSectorEndTime;
	if (duration < 0) return false; // guard

	sectorTimes[index] = duration;

	if (index == sectorCount - 1)
		lapEndTime = t;

	return true;
}

bool Lap::stop(double t)
{
	if (lapStartTime == 0 || lapEndTime != 0) return false;
	lapEndTime = t;
	return true;
}

double Lap::getLapTime() const
{
	if (lapEndTime == 0) return 0;  // Lap not finished
	if (lapStartTime == 0) return 0;  // Invalid lap
	return lapEndTime - lapStartTime;
}

std::optional<double> Lap::getSectorTime(unsigned int index) const
{
	if (index >= sectorCount) return std::nullopt;
	return sectorTimes[index];
}
