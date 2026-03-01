#pragma once
#include <cstdint>

class Point2D
{
public:
    Point2D(int32_t inx = 0, int32_t iny = 0);
    Point2D(const Point2D& p);

    inline int32_t getX() const { return x_mm; }
    inline int32_t getY() const { return y_mm; }

    inline void setX(int32_t inx) { x_mm = inx; }
    inline void setY(int32_t iny) { y_mm = iny; }

    // 距離平方（毫米平方）- 用於比較距離，避免開根號
    inline int64_t distanceSquaredTo(const Point2D& p) const {
        int64_t dx = static_cast<int64_t>(x_mm) - p.x_mm;
        int64_t dy = static_cast<int64_t>(y_mm) - p.y_mm;
        return dx * dx + dy * dy;
    }
    
    // 實際距離（毫米）- 較少使用
    uint32_t distanceTo(const Point2D& p) const;
    
protected:
    int32_t x_mm;  // X 座標（毫米）
    int32_t y_mm;  // Y 座標（毫米）
};
