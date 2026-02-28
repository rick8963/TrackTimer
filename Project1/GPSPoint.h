#pragma once
#include"Point2D.h"
class GPSPoint : public Point2D
{
public:
	GPSPoint(double nmeaLat, double nmeaLng);
	GPSPoint(double lat, double lng, bool isDecimalDegrees);
	double getLatitude();
	double getLongitude();

private:
	GPSPoint();
	double latitude;
	double longitude;
	
	double convertNMEAToDecimalDegrees(double nmeaCoordinate);
	void initFromDecimalDegrees(double lat, double lng);
};

