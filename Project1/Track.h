#pragma once
#include "Line2D.h"
#include <vector>

using namespace std;

class Track{
public:
	/*void startLap();
	void finishLap();
	int getTotalLaps() const;
	Lap getLap(int index) const;*/
	Track(vector<Line2D> scts);
	unsigned int getSectorNumber() const;
	unsigned int getCurrentSectorNumber() const;
	vector<bool> getPassState() const;
	vector<Line2D> getSectors() const;
	Line2D getNextCheckpoint() const;
	Point2D getCurrentPos() const;
	void updatePos(Point2D& pos);

private:
	Track();
	const unsigned int sectorNumber;
	unsigned int currentSector;
	bool lastLapValid;

	Point2D currentPos;
	Point2D lastPos;
	vector<Line2D> sectors;
	vector<bool> passState; // a bool array to record sectors has been passed
	bool passSector(unsigned int i);
	/*std::vector<Lap> laps;
	Point2D lastPos;
	bool lapStarted = false;*/
	
	void nextLap();
};