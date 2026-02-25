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
	// 計算TM投影的公式示意 (需完整橢球體和參數公式)
	x = kScaleFactor * deltaLon * earthEquRadius + kFalseEasting; // 東偏移
	y = kScaleFactor * latRad * earthEquRadius; // 北向近似
}

double GPSPoint::getLatitude()
{
	return latitude;
}

double GPSPoint::getLongitude()
{
	return longitude;
}	

double GPSPoint::convertNMEAToDecimalDegrees(double nmeaCoordinate)
{
	int degrees = static_cast<int>(nmeaCoordinate / 100);
	double minutes = nmeaCoordinate - degrees * 100;
	return degrees + minutes / 60.0;
}
