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
	unsigned int getsectorCount() const;
	unsigned int getCurrentsectorCount() const;
	vector<bool> getPassState() const;
	vector<Sector> getSectors() const;
	Line2D getNextCheckpoint() const;
	Point2D getCurrentPos() const;
	void updatePos(Point2D& pos);
	
private:
	Track();
	const unsigned int sectorCount;
	unsigned int currentSector;
	bool lastLapValid;

	Point2D currentPos;
	Point2D lastPos;
	vector<Line2D> nodes;
	vector<Sector> sectors;
	vector<bool> passState; // a bool array to record sectors has been passed
	bool passSector(unsigned int i);
	void nextLap();

	std::vector<Lap> laps;
	clock_t sessionStartTime;
	clock_t sessionEndTime;
	clock_t bestLapTime;
	clock_t latestLapTime;
	clock_t currentLapTime;
	
};