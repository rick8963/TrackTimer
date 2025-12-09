#include "GPSPoint.h"
#define _USE_MATH_DEFINES
#include <cmath>

constexpr float DEG_TO_RAD = M_PI / 180.0;
constexpr float kCentralMeridian = 121.0 * DEG_TO_RAD; // 以台灣中央經線為例
constexpr float kScaleFactor = 0.9999; // 橫麥卡托投影尺度因子
constexpr float kFalseEasting = 250000.0; // 偏移量
constexpr float earthEquRadius = 6378137.0; // 地球赤道半徑

GPSPoint::GPSPoint(float nmeaLat, float nmeaLng)
{
	latitude = convertNMEAToDecimalDegrees(nmeaLat);
	longitude = convertNMEAToDecimalDegrees(nmeaLng);

	float latRad = latitude * DEG_TO_RAD;
	float lonRad = longitude * DEG_TO_RAD;
	float deltaLon = lonRad - kCentralMeridian;
	// 計算TM投影的公式示意 (需完整橢球體和參數公式)
	x = kScaleFactor * deltaLon * earthEquRadius + kFalseEasting; // 東偏移
	y = kScaleFactor * latRad * earthEquRadius; // 北向近似
}

float GPSPoint::getLatitude()
{
	return latitude;
}

float GPSPoint::getLongitude()
{
	return longitude;
}	

float GPSPoint::convertNMEAToDecimalDegrees(float nmeaCoordinate)
{
	int degrees = static_cast<int>(nmeaCoordinate / 100);
	float minutes = nmeaCoordinate - degrees * 100;
	return degrees + minutes / 60.0;
}
