#pragma once
#include "Point2D.h"

class Line2D
{
public:
    Line2D(const Point2D& p1, const Point2D& p2);
    Line2D(const Point2D& p, int32_t direction_millideg, int32_t width_mm);

    Point2D getPoint1() const;
    Point2D getPoint2() const;
    uint32_t getLength() const;  // 返回長度（毫米）

    void setPoint1(const Point2D& p);
    void setPoint2(const Point2D& p);

    uint32_t distanceToLine(const Point2D& p) const;  // 返回距離（毫米）
    
    // 交叉乘積值（判斷點在線的哪一側）
    inline int64_t crossValue(const Point2D& p) const {
        int64_t dxp = static_cast<int64_t>(p.getX()) - point1.getX();
        int64_t dyp = static_cast<int64_t>(p.getY()) - point1.getY();
        return dx_mm * dyp - dy_mm * dxp;
    }
    
    bool isPointInInterval(const Point2D& p) const;

private:
    Point2D point1;
    Point2D point2;
    uint32_t length_mm;  // 長度（毫米）
    
    // 預先計算的常數
    int64_t dx_mm;       // point2.x - point1.x
    int64_t dy_mm;       // point2.y - point1.y
    int64_t lengthSq_mm2; // dx*dx + dy*dy (毫米平方)
    
    void updateCachedValues();
};
