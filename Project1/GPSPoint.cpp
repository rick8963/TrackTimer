#include "GPSPoint.h"
#define _USE_MATH_DEFINES
#include <cmath>

constexpr float DEG_TO_RAD_F = static_cast<float>(M_PI / 180.0);
constexpr float kCentralMeridianF = 121.0f * DEG_TO_RAD_F; // 以台灣中央經線為例
constexpr float kScaleFactorF = 0.9999f; // 橫麥卡托投影尺度因子
constexpr float kFalseEastingF = 250000.0f; // 偏移量
constexpr float earthEquRadiusF = 6378137.0f; // 地球赤道半徑

GPSPoint::GPSPoint(float nmeaLat, float nmeaLng)
{
	latitude = convertNMEAToDecimalDegrees(nmeaLat);
	longitude = convertNMEAToDecimalDegrees(nmeaLng);

	float latRad = latitude * DEG_TO_RAD_F;
	float lonRad = longitude * DEG_TO_RAD_F;
	float deltaLon = lonRad - kCentralMeridianF;
	// 計算TM投影的公式示意 (需完整橢球體和參數公式)
	x = kScaleFactorF * deltaLon * earthEquRadiusF + kFalseEastingF; // 東偏移
	y = kScaleFactorF * latRad * earthEquRadiusF; // 北向近似
}

GPSPoint::GPSPoint(float lat, float lng, bool isDecimalDegrees)
{
	if (isDecimalDegrees) {
		initFromDecimalDegrees(lat, lng);
	}
	else {
		// 若需要也可以處理 NMEA 格式
		latitude = convertNMEAToDecimalDegrees(lat);
		longitude = convertNMEAToDecimalDegrees(lng);

		float latRad = latitude * DEG_TO_RAD_F;
		float lonRad = longitude * DEG_TO_RAD_F;
		float deltaLon = lonRad - kCentralMeridianF;
		x = kScaleFactorF * deltaLon * earthEquRadiusF + kFalseEastingF;
		y = kScaleFactorF * latRad * earthEquRadiusF;
	}
}

float GPSPoint::getLatitude() const
{
	return latitude;
}

float GPSPoint::getLongitude() const
{
	return longitude;
}	

float GPSPoint::convertNMEAToDecimalDegrees(float nmeaCoordinate)
{
	int degrees = static_cast<int>(nmeaCoordinate / 100.0f);
	float minutes = nmeaCoordinate - degrees * 100.0f;
	return degrees + minutes / 60.0f;
}

void GPSPoint::initFromDecimalDegrees(float lat, float lng)
{
	latitude = lat;
	longitude = lng;

	float latRad = latitude * DEG_TO_RAD_F;
	float lonRad = longitude * DEG_TO_RAD_F;
	float deltaLon = lonRad - kCentralMeridianF;
	x = kScaleFactorF * deltaLon * earthEquRadiusF + kFalseEastingF;
	y = kScaleFactorF * latRad * earthEquRadiusF;
}
