#include "Line2D.h"
#define _USE_MATH_DEFINES
#include <cmath>

constexpr double DEG_TO_RAD = M_PI / 180.0;

Line2D::Line2D(const Point2D& p1, const Point2D& p2)
	: point1(p1), point2(p2), length(p1.distanceTo(p2)) {}

Line2D::Line2D(const Point2D& p, double direction, double width) {
	const double halfWidth = width / 2.0f;
	point1 = Point2D(p.getX() - halfWidth * std::cos(direction * DEG_TO_RAD), p.getY() - halfWidth * std::sin(direction * DEG_TO_RAD));
	point2 = Point2D(p.getX() + halfWidth * std::cos(direction * DEG_TO_RAD), p.getY() + halfWidth * std::sin(direction * DEG_TO_RAD));
	length = point1.distanceTo(point2);
}

Point2D Line2D::getPoint1() const { return point1; }
Point2D Line2D::getPoint2() const { return point2; }
double Line2D::getLength() const { return length; }

void Line2D::setPoint1(const Point2D& p) { point1 = p; }
void Line2D::setPoint2(const Point2D& p) { point2 = p; }

double Line2D::distanceToLine(const Point2D& p) const {
	double x1 = point1.getX();
	double y1 = point1.getY();
	double x2 = point2.getX();
	double y2 = point2.getY();
	double x0 = p.getX();
	double y0 = p.getY();

	double numerator = std::abs((y2 - y1) * x0 - (x2 - x1) * y0 + x2 * y1 - y2 * x1);
	double denominator = std::sqrt((y2 - y1) * (y2 - y1) + (x2 - x1) * (x2 - x1));

	if (denominator == 0) {
		// The line segment is actually a single point
		// Return the distance from p to that point
		double dx = x0 - x1;
		double dy = y0 - y1;
		return std::sqrt(dx * dx + dy * dy);
	}
	return numerator / denominator;
}

double Line2D::crossValue(const Point2D& p) const {
	const double dx = point2.getX() - point1.getX();
	const double dy = point2.getY() - point1.getY();

	const double dxp = p.getX() - point1.getX();
	const double dyp = p.getY() - point1.getY();

	const double cross = dx * dyp - dy * dxp;
	return cross;
}


bool Line2D::isPointInInterval(const Point2D& p) const {
	double dx = point2.getX() - point1.getX();
	double dy = point2.getY() - point1.getY();

	double px = p.getX() - point1.getX();
	double py = p.getY() - point1.getY();

	double lenSq = dx * dx + dy * dy;

	if (lenSq == 0) {
		// Segment is a point, check if p coincides with point1
		return (px == 0 && py == 0);
	}
	// Project vector p onto the line vector (dot product)
	double dot = px * dx + py * dy;

	// Check if projection lies between 0 and lenSq (along the segment)
	if (dot < 0 || dot > lenSq) {
		return false; // Outside segment bounds
	}
	//// Calculate cross product for perpendicular distance squared to line
	//double cross = px * dy - py * dx;
	//double distSq = (cross * cross) / lenSq; // Distance squared from point to line

	//// You may define a tolerance for perpendicular distance (e.g., zero or small epsilon)
	//// Here we assume point must lie exactly on or very close to the line (distance ~ 0)
	//const double epsilon = 1e-6f;
	//// Point is inside the interval if it's within tolerance perpendicular distance
	//return distSq <= epsilon;
	return true;
}