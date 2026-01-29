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
	currentLapTime(0)
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
	currentLapTime(0)
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
	if (!currentLapIndex.has_value()) return true; // no current lap, just return

	Lap& lap = laps[currentLapIndex.value()];
	if (lap.setSectorTime(currentSector, clock()))
	{
		cout << "sector " << currentSector + 1 << " recorded succesfully" << endl;
	}
	else {
		cout << "sector " << currentSector + 1 << " recorded falied" << endl;
	}
	if (lap.getSectorTime(currentSector).has_value())
		cout << lap.getSectorTime(currentSector).value() << " ticks" << endl;
	else
		cout << "N/A" << endl;

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
	currentLapIndex = laps.size() - 1;
}

vector<bool>& const Track::getPassState()
{
	return passState;
}

vector<Sector>& const Track::getSectors()
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

std::vector<Lap>& const Track::getLaps()
{
	return laps;
}
