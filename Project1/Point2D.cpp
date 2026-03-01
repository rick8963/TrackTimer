#include "Point2D.h"
#include <cmath>

Point2D::Point2D(int32_t inx, int32_t iny) : x_mm(inx), y_mm(iny) {}
Point2D::Point2D(const Point2D& p) : x_mm(p.x_mm), y_mm(p.y_mm) {}

uint32_t Point2D::distanceTo(const Point2D& p) const
{
    int64_t distSq = distanceSquaredTo(p);
    return static_cast<uint32_t>(sqrt(static_cast<double>(distSq)));
}
