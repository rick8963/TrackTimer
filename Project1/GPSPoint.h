#pragma once
#include"Point2D.h"
class GPSPoint : public Point2D
{
public:
	GPSPoint(float nmeaLat, float nmeaLng);
	float getLatitude();
	float getLongitude();

private:
	GPSPoint();
	float latitude;
	float longitude;
	
	float convertNMEAToDecimalDegrees(float nmeaCoordinate);
};

