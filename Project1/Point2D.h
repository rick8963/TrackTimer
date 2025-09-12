#pragma once
class Point2D
{
public:
    Point2D(float inx = 0.0, float iny = 0.0);
    Point2D(const Point2D& p);

    float getX() const;
    float getY() const;

    void setX(float inx);
    void setY(float iny);

    float distanceTo(const Point2D& p) const;  // warning, unit is not meters.
    
private:
    float x;
    float y;
};
