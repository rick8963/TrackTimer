#include "Line2D.h"
#define _USE_MATH_DEFINES
#include <cmath>

constexpr double DEG_TO_RAD = M_PI / 180.0;

Line2D::Line2D(const Point2D& p1, const Point2D& p2)
	: point1(p1), point2(p2) {
    updateCachedValues();
}

Line2D::Line2D(const Point2D& p, float direction_deg, float width_meters) {
	// 將外部傳入的度數和公尺，轉換為弧度和公分
	const double direction_rad = direction_deg * DEG_TO_RAD;
	const float halfWidthCm = (width_meters / 2.0f) * 100.0f;
	const double cosDir = cos(direction_rad);
	const double sinDir = sin(direction_rad);
	
	point1 = Point2D(
		p.getX() - static_cast<int32_t>(halfWidthCm * cosDir + 0.5f),
		p.getY() - static_cast<int32_t>(halfWidthCm * sinDir + 0.5f)
	);
	point2 = Point2D(
		p.getX() + static_cast<int32_t>(halfWidthCm * cosDir + 0.5f),
		p.getY() + static_cast<int32_t>(halfWidthCm * sinDir + 0.5f)
	);
	updateCachedValues();
}

void Line2D::updateCachedValues() {
    dx_cm = static_cast<int64_t>(point2.getX()) - point1.getX();
    dy_cm = static_cast<int64_t>(point2.getY()) - point1.getY();
    lengthSq_cm2 = dx_cm * dx_cm + dy_cm * dy_cm;
    length_cm = static_cast<uint32_t>(sqrt(static_cast<double>(lengthSq_cm2)));
}

Point2D Line2D::getPoint1() const { return point1; }
Point2D Line2D::getPoint2() const { return point2; }
uint32_t Line2D::getLengthCm() const { return length_cm; }
float Line2D::getLength() const { return length_cm / 100.0f; }

void Line2D::setPoint1(const Point2D& p) { 
    point1 = p;
    updateCachedValues();
}

void Line2D::setPoint2(const Point2D& p) { 
    point2 = p;
    updateCachedValues();
}

uint32_t Line2D::distanceToLineCm(const Point2D& p) const {
    const int64_t cross = crossValue(p);
    const int64_t absCross = (cross < 0) ? -cross : cross;

	if (lengthSq_cm2 < 1) {
		// 線段實際上是一個點
		return point1.distanceToCm(p);
	}
	
	// |cross| / length
	return static_cast<uint32_t>(absCross / length_cm);
}

float Line2D::distanceToLine(const Point2D& p) const {
    return distanceToLineCm(p) / 100.0f;
}

bool Line2D::isPointInInterval(const Point2D& p) const {
	const int64_t px = static_cast<int64_t>(p.getX()) - point1.getX();
	const int64_t py = static_cast<int64_t>(p.getY()) - point1.getY();

	if (lengthSq_cm2 < 1) {
		// 線段是一個點
		return (px == 0 && py == 0);
	}
	
	// 將向量 p 投影到線段向量上（點積）
	const int64_t dot = px * dx_cm + py * dy_cm;

	// 檢查投影是否在 0 和 lengthSq 之間
	return (dot >= 0 && dot <= lengthSq_cm2);
}
