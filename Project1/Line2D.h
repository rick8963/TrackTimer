#pragma once
#include "Point2D.h"

class Line2D
{
public:
    Line2D(const Point2D& p1, const Point2D& p2);
    Line2D(const Point2D& p, float direction, float width);

    Point2D getPoint1() const;
    Point2D getPoint2() const;
    float getLength() const;

    void setPoint1(const Point2D& p);
    void setPoint2(const Point2D& p);

    float distanceToLine(const Point2D& p) const;
    
    // 交叉乘積值（判斷點在線的哪一側）
    inline float crossValue(const Point2D& p) const {
        const float dxp = p.getX() - point1.getX();
        const float dyp = p.getY() - point1.getY();
        return dx * dyp - dy * dxp;  // 使用預計算的 dx, dy
    }
    
    bool isPointInInterval(const Point2D& p) const;

private:
    Point2D point1;
    Point2D point2;
    float length;
    
    // 預先計算的常數，避免重複運算
    float dx;       // point2.x - point1.x
    float dy;       // point2.y - point1.y
    float lengthSq; // dx*dx + dy*dy
    
    void updateCachedValues();
};
