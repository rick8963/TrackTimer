#include "GPSPoint.h"
#define _USE_MATH_DEFINES
#include <cmath>

constexpr double DEG_TO_RAD = M_PI / 180.0;
constexpr double kCentralMeridian = 121.0 * DEG_TO_RAD; // 以台灣中央經線為例
constexpr double kScaleFactor = 0.9999; // 橫麥卡托投影尺度因子
constexpr double kFalseEasting = 250000.0; // 偏移量
constexpr double earthEquRadius = 6378137.0; // 地球赤道半徑

GPSPoint::GPSPoint(double nmeaLat, double nmeaLng)
{
	latitude = convertNMEAToDecimalDegrees(nmeaLat);
	longitude = convertNMEAToDecimalDegrees(nmeaLng);

	double latRad = latitude * DEG_TO_RAD;
	double lonRad = longitude * DEG_TO_RAD;
	double deltaLon = lonRad - kCentralMeridian;
	
	double x_meters = kScaleFactor * deltaLon * earthEquRadius + kFalseEasting;
	double y_meters = kScaleFactor * latRad * earthEquRadius;
	
	// 轉換成公分並儲存為 int32_t (避免毫米溢位)
	x_cm = static_cast<int32_t>(x_meters * 100.0 + 0.5);
	y_cm = static_cast<int32_t>(y_meters * 100.0 + 0.5);
}

GPSPoint::GPSPoint(double lat, double lng, bool isDecimalDegrees)
{
	if (isDecimalDegrees) {
		initFromDecimalDegrees(lat, lng);
	}
	else {
		latitude = convertNMEAToDecimalDegrees(lat);
		longitude = convertNMEAToDecimalDegrees(lng);

		double latRad = latitude * DEG_TO_RAD;
		double lonRad = longitude * DEG_TO_RAD;
		double deltaLon = lonRad - kCentralMeridian;
		
		double x_meters = kScaleFactor * deltaLon * earthEquRadius + kFalseEasting;
		double y_meters = kScaleFactor * latRad * earthEquRadius;
		
		x_cm = static_cast<int32_t>(x_meters * 100.0 + 0.5);
		y_cm = static_cast<int32_t>(y_meters * 100.0 + 0.5);
	}
}

double GPSPoint::getLatitude() const
{
	return latitude;
}

double GPSPoint::getLongitude() const
{
	return longitude;
}	

double GPSPoint::convertNMEAToDecimalDegrees(double nmeaCoordinate)
{
	int degrees = static_cast<int>(nmeaCoordinate / 100.0);
	double minutes = nmeaCoordinate - degrees * 100.0;
	return degrees + minutes / 60.0;
}

void GPSPoint::initFromDecimalDegrees(double lat, double lng)
{
	latitude = lat;
	longitude = lng;

	double latRad = latitude * DEG_TO_RAD;
	double lonRad = longitude * DEG_TO_RAD;
	double deltaLon = lonRad - kCentralMeridian;
	
	double x_meters = kScaleFactor * deltaLon * earthEquRadius + kFalseEasting;
	double y_meters = kScaleFactor * latRad * earthEquRadius;
	
	x_cm = static_cast<int32_t>(x_meters * 100.0 + 0.5);
	y_cm = static_cast<int32_t>(y_meters * 100.0 + 0.5);
}
