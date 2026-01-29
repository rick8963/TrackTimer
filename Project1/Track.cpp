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
	latestLapTime(0)
{}

Track::Track(vector<Line2D> nds, bool isCircuit)
	:
	sectorCount(isCircuit ? nds.size() : nds.size() - 1),
	nodes(std::move(nds)),
	currentSector(sectorCount - 1), // start at last sector
	currentPos(Point2D(0, 0)),
	lastPos(Point2D(0, 0)),
	lastLapValid(true),
	sessionStartTime(clock()),
	sessionEndTime(0),
	bestLapTime(0),
	latestLapTime(0)
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
	if (!sector->isPointInInterval(lastPos) && !sector->isPointInInterval(currentPos)) return false;
	if (sector->crossValue(lastPos) * sector->crossValue(currentPos) > 0) return false;
	if (!currentLapIndex.has_value()) return true;

	Lap& lap = laps[currentLapIndex.value()];
	if (lap.setSectorTime(currentSector, clock()))
	{
		cout << "sector " << currentSector + 1 << " recorded successfully" << endl;
	}
	else {
		cout << "sector " << currentSector + 1 << " recorded failed" << endl;
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

bool Track::isAllSectorsPassed() const
{
	for (const auto& sector : sectors)
	{
		if (!sector.isPassed()) return false;
	}
	return true;
}

// lap completed
void Track::nextLap()
{
	// Update lap statistics if there was a previous lap
	if (currentLapIndex.has_value())
	{
		clock_t lapTime = laps[currentLapIndex.value()].getLapTime();
		latestLapTime = lapTime;
		
		// Update best lap time (only for valid laps)
		if (lastLapValid && (bestLapTime == 0 || lapTime < bestLapTime))
		{
			bestLapTime = lapTime;
		}
	}
	
	// Reset all sectors
	for (auto& sector : sectors) sector.reset();
	
	// Create new lap
	laps.push_back(Lap(sectorCount, clock()));
	currentLapIndex = laps.size() - 1;
}

const vector<Sector>& Track::getSectors() const
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
		// Check if last lap was valid using Sector states
		lastLapValid = isAllSectorsPassed();
		
		cout << "last lap is valid: " << lastLapValid << endl;
		nextLap();
		currentSector = 0;
		return;
	}
	if(currentSector == sectorCount - 1) return;
	if (passSector(currentSector + 1))
	{
		// Mark current sector as passed
		if (currentSector < sectors.size())
		{
			sectors[currentSector].pass();
		}
		currentSector++;
	}
}

const std::vector<Lap>& Track::getLaps() const
{
	return laps;
}

clock_t Track::getBestLapTime() const
{
	return bestLapTime;
}

clock_t Track::getLatestLapTime() const
{
	return latestLapTime;
}

clock_t Track::getSessionStartTime() const
{
	return sessionStartTime;
}

clock_t Track::getSessionEndTime() const
{
	return sessionEndTime;
}
