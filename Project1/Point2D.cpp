#include "Point2D.h"
#include <cmath>

Point2D::Point2D(int32_t inx, int32_t iny) : x_cm(inx), y_cm(iny) {}
Point2D::Point2D(const Point2D& p) : x_cm(p.x_cm), y_cm(p.y_cm) {}

uint32_t Point2D::distanceToCm(const Point2D& p) const
{
    int64_t distSq = distanceSquaredTo(p);
    return static_cast<uint32_t>(sqrt(static_cast<double>(distSq)));
}

float Point2D::distanceTo(const Point2D& p) const
{
    return distanceToCm(p) / 100.0f;
}
