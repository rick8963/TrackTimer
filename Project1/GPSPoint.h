#pragma once
#include"Point2D.h"
class GPSPoint : public Point2D
{
public:
	GPSPoint(float nmeaLat, float nmeaLng);
	GPSPoint(float lat, float lng, bool isDecimalDegrees);
	float getLatitude() const;
	float getLongitude() const;

private:
	GPSPoint();
	float latitude;
	float longitude;
	
	float convertNMEAToDecimalDegrees(float nmeaCoordinate);
	void initFromDecimalDegrees(float lat, float lng);
};

