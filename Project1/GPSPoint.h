#pragma once
#include"Point2D.h"

class GPSPoint : public Point2D
{
public:
	GPSPoint(double nmeaLat, double nmeaLng);
	GPSPoint(double lat, double lng, bool isDecimalDegrees);
	double getLatitude() const;
	double getLongitude() const;

private:
	GPSPoint();
	double latitude;   // 保留 double 儲存原始 GPS 精度
	double longitude;
	
	double convertNMEAToDecimalDegrees(double nmeaCoordinate);
	void initFromDecimalDegrees(double lat, double lng);
};
