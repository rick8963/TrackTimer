#include "Point2D.h"
#include <cmath>

Point2D::Point2D(float inx, float iny) : x(inx), y(iny) {}
Point2D::Point2D(const Point2D& p) : x(p.x), y(p.y) {}

float Point2D::distanceTo(const Point2D& p) const
{
    const float dx = x - p.x;
    const float dy = y - p.y;
    return sqrtf(dx * dx + dy * dy);  // 使用 sqrtf (float版本)
}
