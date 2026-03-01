#pragma once
class Point2D
{
public:
    Point2D(double inx = 0.0, double iny = 0.0);
    Point2D(const Point2D& p);

    virtual double getX() const;
    virtual double getY() const;

    void setX(double inx);
    void setY(double iny);

    double distanceTo(const Point2D& p) const;  // in meters.
    
protected:
    double x;
    double y;
};
