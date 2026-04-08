#pragma once
#include <cstdint>

class Point2D
{
public:
    Point2D(int32_t x_cm = 0, int32_t y_cm = 0);
    Point2D(const Point2D& p);

    inline int32_t getX() const { return x_cm; }
    inline int32_t getY() const { return y_cm; }

    inline void setX(int32_t inx) { x_cm = inx; }
    inline void setY(int32_t iny) { y_cm = iny; }

    // 距離平方（公分平方）- 用於比較距離，避免開根號
    inline int64_t distanceSquaredTo(const Point2D& p) const {
        int64_t dx = static_cast<int64_t>(x_cm) - p.x_cm;
        int64_t dy = static_cast<int64_t>(y_cm) - p.y_cm;
        return dx * dx + dy * dy;
    }
    
    // 實際距離（公分）- 內部使用
    uint32_t distanceToCm(const Point2D& p) const;
    
    // 實際距離（公尺）- 提供給外部顯示用
    float distanceTo(const Point2D& p) const;
    
protected:
    int32_t x_cm;  // X 座標（公分）
    int32_t y_cm;  // Y 座標（公分）
};
