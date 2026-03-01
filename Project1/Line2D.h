#pragma once
#include "Point2D.h"

class Line2D
{
public:
    Line2D(const Point2D& p1, const Point2D& p2);
    // 恢復原始 API：角度(度)與寬度(公尺)
    Line2D(const Point2D& p, float direction_deg, float width_meters);

    Point2D getPoint1() const;
    Point2D getPoint2() const;
    
    uint32_t getLengthCm() const;
    float getLength() const;  // 返回長度（公尺）

    void setPoint1(const Point2D& p);
    void setPoint2(const Point2D& p);

    uint32_t distanceToLineCm(const Point2D& p) const; // 內部使用(公分)
    float distanceToLine(const Point2D& p) const;      // 外部顯示(公尺)
    
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
    
    // 預先計算的常數
    int64_t dx_cm;       // point2.x - point1.x
    int64_t dy_cm;       // point2.y - point1.y
    int64_t lengthSq_cm2; // dx*dx + dy*dy (公分平方)
    
    void updateCachedValues();
};
