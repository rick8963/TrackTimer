#include "Line2D.h"
#define _USE_MATH_DEFINES
#include <cmath>

constexpr double DEG_TO_RAD = M_PI / 180.0;
constexpr double MILLIDEG_TO_RAD = M_PI / 180000.0;  // 千分之一度轉弧度

Line2D::Line2D(const Point2D& p1, const Point2D& p2)
	: point1(p1), point2(p2) {
    updateCachedValues();
}

Line2D::Line2D(const Point2D& p, int32_t direction_millideg, int32_t width_mm) {
	// direction_millideg: 方向角（千分之一度）
	// width_mm: 寬度（毫米）
	const double direction_rad = direction_millideg * MILLIDEG_TO_RAD;
	const int32_t halfWidth = width_mm / 2;
	const double cosDir = cos(direction_rad);
	const double sinDir = sin(direction_rad);
	
	point1 = Point2D(
		p.getX() - static_cast<int32_t>(halfWidth * cosDir),
		p.getY() - static_cast<int32_t>(halfWidth * sinDir)
	);
	point2 = Point2D(
		p.getX() + static_cast<int32_t>(halfWidth * cosDir),
		p.getY() + static_cast<int32_t>(halfWidth * sinDir)
	);
	updateCachedValues();
}

void Line2D::updateCachedValues() {
    dx_mm = static_cast<int64_t>(point2.getX()) - point1.getX();
    dy_mm = static_cast<int64_t>(point2.getY()) - point1.getY();
    lengthSq_mm2 = dx_mm * dx_mm + dy_mm * dy_mm;
    length_mm = static_cast<uint32_t>(sqrt(static_cast<double>(lengthSq_mm2)));
}

Point2D Line2D::getPoint1() const { return point1; }
Point2D Line2D::getPoint2() const { return point2; }
uint32_t Line2D::getLength() const { return length_mm; }

void Line2D::setPoint1(const Point2D& p) { 
    point1 = p;
    updateCachedValues();
}

void Line2D::setPoint2(const Point2D& p) { 
    point2 = p;
    updateCachedValues();
}

uint32_t Line2D::distanceToLine(const Point2D& p) const {
	const int64_t x0 = p.getX();
	const int64_t y0 = p.getY();
	const int64_t x1 = point1.getX();
	const int64_t y1 = point1.getY();
	const int64_t x2 = point2.getX();
	const int64_t y2 = point2.getY();

	// 計算交叉乘積的絕對值
	const int64_t cross = dy_mm * x0 - dx_mm * y0 + x2 * y1 - y2 * x1;
	const int64_t absCross = (cross < 0) ? -cross : cross;

	if (lengthSq_mm2 < 1) {
		// 線段實際上是一個點
		return point1.distanceTo(p);
	}
	
	// |cross| / length
	return static_cast<uint32_t>(absCross / length_mm);
}

bool Line2D::isPointInInterval(const Point2D& p) const {
	const int64_t px = static_cast<int64_t>(p.getX()) - point1.getX();
	const int64_t py = static_cast<int64_t>(p.getY()) - point1.getY();

	if (lengthSq_mm2 < 1) {
		// 線段是一個點
		return (px == 0 && py == 0);
	}
	
	// 將向量 p 投影到線段向量上（點積）
	const int64_t dot = px * dx_mm + py * dy_mm;

	// 檢查投影是否在 0 和 lengthSq 之間
	return (dot >= 0 && dot <= lengthSq_mm2);
}
