#include "Point2D.h"
#include <cmath>

Point2D::Point2D(float inx, float iny) : x(inx), y(iny) {}
Point2D::Point2D(const Point2D& p) : x(p.x), y(p.y) {}

float Point2D::getX() const { return x; }
float Point2D::getY() const { return y; }

void Point2D::setX(float inx) { x = inx; }
void Point2D::setY(float iny) { y = iny; }


float Point2D::distanceTo(const Point2D& p) const
{
    const float dx = x - p.x;
    const float dy = y - p.y;
    return std::sqrt(dx * dx + dy * dy);
}