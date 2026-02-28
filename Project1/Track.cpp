#include "Track.h"

// Helper function to handle midnight rollover
static double adjustTimeForMidnight(double currentTime, double lastTime) {
	// If time goes backward by more than 12 hours, assume midnight rollover
	if (lastTime - currentTime > 43200000.0) { // 12 hours in milliseconds
		return currentTime + 86400000.0; // Add 24 hours
	}
	return currentTime;
}

Track::Track()
	: sectorCount(0),
	currentSector(0),
	currentPos(Point2D(0, 0)),
	lastPos(Point2D(0, 0)),
	lastLapValid(true),
	sessionStartTime(0),
	sessionEndTime(0),
	bestLapTime(0),
	latestLapTime(0),
	lastTimestamp(0)
{}

Track::Track(vector<Line2D> nds, bool isCircuit)
	:
	sectorCount(isCircuit ? nds.size() : nds.size() - 1),
	nodes(std::move(nds)),
	currentSector(sectorCount - 1), // start at last sector
	currentPos(Point2D(0, 0)),
	lastPos(Point2D(0, 0)),
	lastLapValid(true),
	sessionStartTime(0),
	sessionEndTime(0),
	bestLapTime(0),
	latestLapTime(0),
	lastTimestamp(0)
{
	for (int i = 0; i < sectorCount - 1; i++)
	{
		sectors.push_back(Sector(i, i + 1));
	}
	// insert last sector
	if(isCircuit) sectors.push_back(Sector(sectorCount - 1, 0));
}

bool Track::passSector(unsigned int i, double timestamp)
{
	if (i >= nodes.size()) return false;
	Line2D* sector = &nodes.at(i);
	if (!sector->isPointInInterval(lastPos) && !sector->isPointInInterval(currentPos)) return false;
	if (sector->crossValue(lastPos) * sector->crossValue(currentPos) > 0) return false;
	if (!currentLapIndex.has_value()) return true;

	Lap& lap = laps[currentLapIndex.value()];
	lap.setSectorTime(currentSector, timestamp);

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
void Track::nextLap(double timestamp)
{
	// Update lap statistics if there was a previous lap
	if (currentLapIndex.has_value())
	{
		Lap& prevLap = laps[currentLapIndex.value()];
		double lapTime = prevLap.getLapTime();

		// Only update statistics if lap is completed (lapTime > 0)
		if (lapTime > 0)
		{
			latestLapTime = lapTime;

			// Update best lap time (only for valid and completed laps)
			if (lastLapValid && (bestLapTime == 0 || lapTime < bestLapTime))
			{
				bestLapTime = lapTime;
			}
		}
	}

	// Reset all sectors
	for (auto& sector : sectors) sector.reset();

	// Create new lap
	laps.push_back(Lap(sectorCount, timestamp));
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

void Track::updatePos(Point2D& pos, double timestamp)
{
	// Handle midnight rollover
	if (lastTimestamp > 0) {
		timestamp = adjustTimeForMidnight(timestamp, lastTimestamp);
	}
	
	// Initialize session start time on first update
	if (sessionStartTime == 0) {
		sessionStartTime = timestamp;
	}
	
	lastPos = currentPos;
	currentPos = pos;
	lastTimestamp = timestamp;

	// pass start line
	if (passSector(0, timestamp))
	{
		// Record the last sector time BEFORE creating new lap
		if (currentLapIndex.has_value() && currentSector == sectorCount - 1)
		{
			Lap& lap = laps[currentLapIndex.value()];
			lap.setSectorTime(sectorCount - 1, timestamp);
			
			// Mark the last sector as passed before validation
			if (currentSector < sectors.size())
			{
				sectors[currentSector].pass();
			}
		}

		// Check if last lap was valid using Sector states
		lastLapValid = isAllSectorsPassed();
		nextLap(timestamp);
		currentSector = 0;
		return;
	}
	if (currentSector == sectorCount - 1) return;
	if (passSector(currentSector + 1, timestamp))
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

double Track::getBestLapTime() const
{
	return bestLapTime;
}

double Track::getLatestLapTime() const
{
	return latestLapTime;
}

double Track::getSessionStartTime() const
{
	return sessionStartTime;
}

double Track::getSessionEndTime() const
{
	return sessionEndTime;
}
