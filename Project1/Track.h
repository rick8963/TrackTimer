#pragma once
#include "Lap.h"
#include "Sector.h"

using namespace std;

class Track{
	/*
	* Concept of Track:
	* A track is made up of several sectors.
	* Each sector is defined by a Line2D node.
	* The sectors are connected in a loop (circuit) or a line (non-circuit).
	* 
	* Timer should be started when the first sector is passed.
	* When all sectors are passed, a lap is completed.
	* The track should be able to tell which sector is next based on current position.(???)
	* 
	* Laps should be recorded with time, and sector times should also be recorded.
	* Laps is a vector of Lap objects, each Lap contains sector times and total lap time.
	* 
	*/
public:
	Track(vector<Line2D> nds, bool isCircuit = true);
	unsigned int getSectorCount() const;
	unsigned int getCurrentSectorCount() const;
	const vector<Sector>& getSectors() const;
	Line2D getNextCheckpoint() const;
	Point2D getCurrentPos() const;
	void updatePos(Point2D& pos, double timestamp);
	const std::vector<Lap>& getLaps() const;
	double getBestLapTime() const;
	double getLatestLapTime() const;
	double getSessionStartTime() const;
	double getSessionEndTime() const;
	
private:
	Track();
	const unsigned int sectorCount;
	unsigned int currentSector;
	bool lastLapValid;

	Point2D currentPos;
	Point2D lastPos;
	vector<Line2D> nodes;
	vector<Sector> sectors;
	// Removed passState - now using Sector::isPassed()
	bool passSector(unsigned int i, double timestamp);
	void nextLap(double timestamp);
	bool isAllSectorsPassed() const;

	std::optional<size_t> currentLapIndex;
	std::vector<Lap> laps;
	double sessionStartTime;
	double sessionEndTime;
	double bestLapTime;
	double latestLapTime;
	double lastTimestamp;
	
};