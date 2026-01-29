#include<iostream>
#include<vector>
#include<Windows.h>
#include"Track.h"
#include"GPSPoint.h"

using namespace std;

void updatePos(Point2D& pos)
{
	const int x = pos.getX();
	const int y = pos.getY();

	int dx = 0;
	int dy = 0;
	if (x == 0 && y < 10)
	{
		dx = 0;
		dy = 1;
	}
	else if (x < 10 && y == 10.0)
	{
		dx = 1;
		dy = 0;
	}
	else if (x == 10 && y > 0)
	{
		dx = 0;
		dy = -1;
	}
	else
	{
		dx = -1;
		dy = 0;
	}
	pos.setX(pos.getX() + dx);
	pos.setY(pos.getY() + dy);
}

int main()
{
	vector<Line2D> sectors;
	sectors.push_back(Line2D(Point2D(0, 5), 0, 2));
	sectors.push_back(Line2D(Point2D(5, 10), 90, 2));
	sectors.push_back(Line2D(Point2D(10, 5), 180, 2));
	sectors.push_back(Line2D(Point2D(5, 0), 270, 2));
	Track SQR(sectors);

	Point2D currentPos(0.5, 0.5);
	for (int i = 0; i < 200; i++)
	{
		updatePos(currentPos);
		SQR.updatePos(currentPos);


		Line2D curSector = SQR.getNextCheckpoint();
		vector<bool> passState = SQR.getPassState();

		cout << "current pos (" << currentPos.getX() << ", " << currentPos.getY() << ")\tat sector " << SQR.getCurrentSectorCount() + 1 << endl
			<< curSector.distanceToLine(currentPos) << " meters to next checkpoint (" << curSector.getPoint1().getX() << ", " << curSector.getPoint1().getY() << "), "
			<< "(" << curSector.getPoint2().getX() << ", " << curSector.getPoint2().getY() << ")" << endl;
		
		cout << "sectors state : \n";
		for (int i = 0; i < SQR.getSectorCount(); i++)
		{
			cout << "Sector " << i + 1 << "\t";
		}
		cout << endl;
		for (auto state : SQR.getPassState())
		{
			cout << (state ? "True" : "False") << "\t\t";
		}
		cout << endl << endl;
		//Sleep(500);
	}
	cout << SQR.getLaps().size() << " laps in total" << endl;
	int i = 1;
	for (auto& lap : SQR.getLaps())
	{
		cout << "Lap " << i++ << " : " << lap.getLapTime() << " ticks" << endl;
		for (int j = 0; j < SQR.getSectorCount() ; j++)
		{
			auto sectorTime = lap.getSectorTime(j);
			if (sectorTime.has_value())
				cout << "\tSector " << j + 1 << " : " << sectorTime.value() << " ticks" << endl;
			else
				cout << "\tSector " << j + 1 << " : " << "N/A" << endl;
		}
			
	}

	 //22.857318, 120.289120
	 //22.856466, 120.289463

	//GPSPoint loc(2251.4390, 12017.3472);
	//cout << loc.getX() << " " << loc.getY() << endl;
	//GPSPoint loca(2251.3879, 12017.3677);
	//cout << loca.getX() << " " << loca.getY() << endl;
	//cout << "distance " << loc.distanceTo(loca) << " meters" << endl;
	return 0;
}