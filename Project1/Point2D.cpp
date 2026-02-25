#include "Point2D.h"
#include <cmath>

Point2D::Point2D(double inx, double iny) : x(inx), y(iny) {}
Point2D::Point2D(const Point2D& p) : x(p.x), y(p.y) {}

double Point2D::getX() const { return x; }
double Point2D::getY() const { return y; }

void Point2D::setX(double inx) { x = inx; }
void Point2D::setY(double iny) { y = iny; }


double Point2D::distanceTo(const Point2D& p) const
{
    const double dx = x - p.x;
    const double dy = y - p.y;
    return std::sqrt(dx * dx + dy * dy);
}