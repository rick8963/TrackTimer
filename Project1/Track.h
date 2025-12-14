#pragma once
#include "Sector.h"
#include <vector>

using namespace std;

class Track{
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
	/*std::vector<Lap> laps;
	Point2D lastPos;
	bool lapStarted = false;*/
	
	void nextLap();
};