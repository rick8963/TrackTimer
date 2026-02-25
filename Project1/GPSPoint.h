#pragma once
#include"Point2D.h"
class GPSPoint : public Point2D
{
public:
	GPSPoint(double nmeaLat, double nmeaLng);
	double getLatitude();
	double getLongitude();

private:
	GPSPoint();
	double latitude;
	double longitude;
	
	double convertNMEAToDecimalDegrees(double nmeaCoordinate);
};

