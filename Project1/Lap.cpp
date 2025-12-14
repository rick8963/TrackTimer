#include "Lap.h"

#include<cmath>

Lap::Lap(int numSectors, Line2D start, Line2D end)
{
}

void Lap::setLapTime(double time)
{
}

double Lap::getLapTime() const
{
	return 0.0;
}

void Lap::setSectorTime(int index, double time)
{
}

double Lap::getSectorTime(int index) const
{
	return 0.0;
}

bool Lap::checkLapCompleted(const Point2D& currentPos, const Point2D& lastPos) const
{
	return false;
}
