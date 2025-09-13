#include "Sector.h"

Sector::Sector(Line2D start, Line2D end):
 startPos(start), endPos(end)
{
	reset();
}

void Sector::setSectorTime(float time) { sectorTime = time; }
float Sector::getSectorTime() const { return sectorTime; }

void Sector::reset()
{
	sectorTime = 0;
	entered = false;
	passed = false;
}

void Sector::enter() { entered = true; }
void Sector::pass() { passed = true; }
bool Sector::isEntered() const{	return entered; }
bool Sector::isPassed() const { return passed; }

bool Sector::checkIfPassed(const Point2D& currentPos, const Point2D& lastPos)
{
	return false;
}




