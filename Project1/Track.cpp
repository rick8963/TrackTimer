#include "Track.h"
#include<iostream>
Track::Track()
	: sectorNumber(0),
	currentSector(0),
	currentPos(Point2D(0, 0)),
	lastPos(Point2D(0, 0)),
	lastLapValid(true)
{}

Track::Track(vector<Line2D> scts)
	: sectorNumber(scts.size()),
	sectors(std::move(scts)),
	passState(sectorNumber, false),
	currentSector(sectorNumber - 1), // start at last sector
	currentPos(Point2D(0, 0)),
	lastPos(Point2D(0, 0)),
	lastLapValid(true)
{}

bool Track::passSector(unsigned int i)
{
	
	Line2D* sector = &sectors.at(i);
	//std::cout << "check pass " << i  << " "
		//<< sector->isPointInInterval(lastPos) << " " << sector->isPointInInterval(currentPos) << std::endl;
	if (!sector->isPointInInterval(lastPos) && !sector->isPointInInterval(currentPos)) return false; // not in detect interval
	//std::cout << sector->crossValue(lastPos) * sector->crossValue(currentPos) << std::endl;
	if (sector->crossValue(lastPos) * sector->crossValue(currentPos) > 0) return false; // not pass a sector
	return true;
}

unsigned int Track::getSectorNumber() const
{
	return sectorNumber;
}

unsigned int Track::getCurrentSectorNumber() const
{
	return currentSector;
}

// lap compeleted
void Track::nextLap()
{
	std::fill(passState.begin(), passState.end(), false);
	currentSector = 0;
}

vector<bool> Track::getPassState() const
{
	return passState;
}

vector<Line2D> Track::getSectors() const
{
	return sectors;
}

Line2D Track::getNextCheckpoint() const
{
	return currentSector + 1 == sectorNumber ? sectors.at(0) : sectors.at(currentSector + 1);
}

Point2D Track::getCurrentPos() const
{
	return Point2D(currentPos);
}

void Track::updatePos(Point2D& pos)
{
	lastPos = currentPos;
	currentPos = pos;

	// pass start line
	if (passSector(0))
	{
		lastLapValid = true;
		for (auto state : passState) if (!state) lastLapValid = false; // check last lap is valid
		nextLap();
		return;
	}
	if(currentSector == sectorNumber - 1) return;
	if (passSector(currentSector + 1))
	{
		passState.at(currentSector) = true;
		currentSector++;
	}
}
