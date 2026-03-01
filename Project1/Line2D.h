#pragma once
#include "Point2D.h"

class Line2D
{
public:
    Line2D(const Point2D& p1, const Point2D& p2);
    Line2D(const Point2D& p, float direction_deg, float width_meters);

    inline Point2D getPoint1() const { return point1; }
    inline Point2D getPoint2() const { return point2; }
    
    inline uint32_t getLengthCm() const { return length_cm; }
    inline float getLength() const { return length_cm * 0.01f; } // 使用乘法代替除法

    void setPoint1(const Point2D& p);
    void setPoint2(const Point2D& p);

    uint32_t distanceToLineCm(const Point2D& p) const; // 內部使用(公分)
    inline float distanceToLine(const Point2D& p) const { return distanceToLineCm(p) * 0.01f; }
    
    // 交叉乘積值（判斷點在線的哪一側）
    inline int64_t crossValue(const Point2D& p) const {
        int64_t dxp = static_cast<int64_t>(p.getX()) - point1.getX();
        int64_t dyp = static_cast<int64_t>(p.getY()) - point1.getY();
        return dx_cm * dyp - dy_cm * dxp;
    }
    
    bool isPointInInterval(const Point2D& p) const;

private:
    Point2D point1;
    Point2D point2;
    uint32_t length_cm;  // 長度（公分）
    
    // 預先計算的常數，加速運算
    int64_t dx_cm;       // point2.x - point1.x
    int64_t dy_cm;       // point2.y - point1.y
    int64_t lengthSq_cm2; // dx*dx + dy*dy (公分平方)
    
    void updateCachedValues();
};
