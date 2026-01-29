#include "Track.h"
#include<iostream>
Track::Track()
	: sectorCount(0),
	currentSector(0),
	currentPos(Point2D(0, 0)),
	lastPos(Point2D(0, 0)),
	lastLapValid(true),
	sessionStartTime(clock()),
	sessionEndTime(0),
	bestLapTime(0),
	latestLapTime(0),
	currentLapTime(0),
	currentLap(nullptr)
{}

Track::Track(vector<Line2D> nds, bool isCircuit)
	:
	sectorCount(isCircuit ? nds.size() : nds.size() - 1),
	nodes(std::move(nds)),
	passState(sectorCount, false),
	currentSector(sectorCount - 1), // start at last sector
	currentPos(Point2D(0, 0)),
	lastPos(Point2D(0, 0)),
	lastLapValid(true),
	sessionStartTime(clock()),
	sessionEndTime(0),
	bestLapTime(0),
	latestLapTime(0),
	currentLapTime(0),
	currentLap(nullptr)
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
	if (i >= nodes.size()) return false;
	Line2D* sector = &nodes.at(i);
	if (!sector->isPointInInterval(lastPos) && !sector->isPointInInterval(currentPos)) return false; // not in detect interval
	if (sector->crossValue(lastPos) * sector->crossValue(currentPos) > 0) return false; // not pass a sector
	
	if (currentLap != nullptr)
	{
		if (currentLap->setSectorTime(currentSector, clock()))
		{
			cout << "sector " << currentSector + 1 << " recorded succesfully" << endl;
		}
		else {
			cout << "sector " << currentSector + 1 << " recorded falied" << endl;
		}
		if (currentLap->getSectorTime(currentSector).has_value())
			cout << currentLap->getSectorTime(currentSector).value() << " ticks" << endl;
		else
			cout << "N/A" << endl;
	}

	return true;
}

unsigned int Track::getSectorCount() const
{
	return sectorCount;
}

unsigned int Track::getCurrentSectorCount() const
{
	return currentSector;
}

// lap compeleted
void Track::nextLap()
{
	for (auto& sector : sectors) sector.reset();
	std::fill(passState.begin(), passState.end(), false);
	laps.push_back(Lap(sectorCount, clock()));
	currentLap = &laps.back();
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
		for (int i = 0; i < sectorCount - 1; i++)
		{
			if(passState.at(i) ==false) lastLapValid = false; // check last lap is valid
		}
		//for (auto state : passState) if (!state) lastLapValid = false; // check last lap is valid
		cout << "last lap is invalid###########" << lastLapValid << endl;
		nextLap();
		currentSector = 0;
		return;
	}
	if(currentSector == sectorCount - 1) return;
	if (passSector(currentSector + 1))
	{
		passState.at(currentSector) = true;
		currentSector++;
	}
}

std::vector<Lap> Track::getLaps() const
{
	return laps;
}
