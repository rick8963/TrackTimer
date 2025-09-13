#pragma once
#include "Point2D.h"

class Line2D
{
public:
	Line2D(const Point2D& p1 = Point2D(), const Point2D& p2 = Point2D());
	Line2D(const Point2D& p, float direction, float width); // direction in degree

	Point2D getPoint1() const;
	Point2D getPoint2() const;

	void setPoint1(const Point2D& p);
	void setPoint2(const Point2D& p);

	float getLength() const; // in meters.
	float distanceToLine(const Point2D& p) const; // in meters.
	float crossValue(const Point2D& p) const;
	bool isPointInInterval(const Point2D& p) const;

private:
	Point2D point1;
	Point2D point2;
	float length;
};