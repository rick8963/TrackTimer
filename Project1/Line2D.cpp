#include "Line2D.h"
#define _USE_MATH_DEFINES
#include <cmath>

constexpr float DEG_TO_RAD_F = static_cast<float>(M_PI / 180.0);

Line2D::Line2D(const Point2D& p1, const Point2D& p2)
	: point1(p1), point2(p2) {
    updateCachedValues();
}

Line2D::Line2D(const Point2D& p, float direction, float width) {
	const float halfWidth = width / 2.0f;
	const float cosDir = cosf(direction * DEG_TO_RAD_F);
	const float sinDir = sinf(direction * DEG_TO_RAD_F);
	
	point1 = Point2D(p.getX() - halfWidth * cosDir, p.getY() - halfWidth * sinDir);
	point2 = Point2D(p.getX() + halfWidth * cosDir, p.getY() + halfWidth * sinDir);
	updateCachedValues();
}

void Line2D::updateCachedValues() {
    dx = point2.getX() - point1.getX();
    dy = point2.getY() - point1.getY();
    lengthSq = dx * dx + dy * dy;
    length = sqrtf(lengthSq);
}

Point2D Line2D::getPoint1() const { return point1; }
Point2D Line2D::getPoint2() const { return point2; }
float Line2D::getLength() const { return length; }

void Line2D::setPoint1(const Point2D& p) { 
    point1 = p;
    updateCachedValues();
}

void Line2D::setPoint2(const Point2D& p) { 
    point2 = p;
    updateCachedValues();
}

float Line2D::distanceToLine(const Point2D& p) const {
	const float x0 = p.getX();
	const float y0 = p.getY();
	const float x1 = point1.getX();
	const float y1 = point1.getY();

	const float numerator = fabsf(dy * x0 - dx * y0 + point2.getX() * y1 - point2.getY() * x1);

	if (lengthSq < 1e-6f) {
		// 線段實際上是一個點
		return point1.distanceTo(p);
	}
	return numerator / length;
}

bool Line2D::isPointInInterval(const Point2D& p) const {
	const float px = p.getX() - point1.getX();
	const float py = p.getY() - point1.getY();

	if (lengthSq < 1e-6f) {
		// 線段是一個點，檢查 p 是否與 point1 重合
		return (px == 0 && py == 0);
	}
	
	// 將向量 p 投影到線段向量上（點積）
	const float dot = px * dx + py * dy;

	// 檢查投影是否在 0 和 lengthSq 之間（沿線段）
	return (dot >= 0 && dot <= lengthSq);
}
