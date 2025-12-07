#include "Track.h"
#include<iostream>
Track::Track()
	: sectorCount(0),
	currentSector(0),
	currentPos(Point2D(0, 0)),
	lastPos(Point2D(0, 0)),
	lastLapValid(true)
{}

Track::Track(vector<Line2D> nds, bool isCircuit)
	:
	sectorCount(isCircuit ? nds.size() : nds.size() - 1),
	nodes(std::move(nds)),
	passState(sectorCount, false),
	currentSector(sectorCount - 1), // start at last sector
	currentPos(Point2D(0, 0)),
	lastPos(Point2D(0, 0)),
	lastLapValid(true)
{

	for (int i = 0; i < sectorCount - 1; i++)
	{
		sectors.push_back(Sector(i, i + 1));
	}
	// insert last sector
	if(isCircuit) sectors.push_back(Sector(sectorCount - 1, 0));

}

bool Track::passSector(unsigned int i)
{
	
	Line2D* sector = &nodes.at(i);
	//std::cout << "check pass " << i  << " "
		//<< sector->isPointInInterval(lastPos) << " " << sector->isPointInInterval(currentPos) << std::endl;
	if (!sector->isPointInInterval(lastPos) && !sector->isPointInInterval(currentPos)) return false; // not in detect interval
	//std::cout << sector->crossValue(lastPos) * sector->crossValue(currentPos) << std::endl;
	if (sector->crossValue(lastPos) * sector->crossValue(currentPos) > 0) return false; // not pass a sector
	return true;
}

unsigned int Track::getsectorCount() const
{
	return sectorCount;
}

unsigned int Track::getCurrentsectorCount() const
{
	return currentSector;
}

// lap compeleted
void Track::nextLap()
{
	//std::fill(passState.begin(), passState.end(), false);
	for (auto& sector : sectors) sector.reset();
	currentSector = 0;
}

vector<bool> Track::getPassState() const
{
	return passState;
}

vector<Sector> Track::getSectors() const
{
	return sectors;
}

Line2D Track::getNextCheckpoint() const
{
	return currentSector + 1 == sectorCount ? nodes.at(0) : nodes.at(currentSector + 1);
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
	if(currentSector == sectorCount - 1) return;
	if (passSector(currentSector + 1))
	{
		passState.at(currentSector) = true;
		currentSector++;
	}
}
