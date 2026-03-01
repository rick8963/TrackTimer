#pragma once
class Point2D
{
public:
    Point2D(float inx = 0.0f, float iny = 0.0f);
    Point2D(const Point2D& p);

    inline float getX() const { return x; }
    inline float getY() const { return y; }

    inline void setX(float inx) { x = inx; }
    inline void setY(float iny) { y = iny; }

    // 距離計算（含 sqrt，較慢）
    float distanceTo(const Point2D& p) const;
    
    // 距離平方（避免 sqrt，用於比較距離時更快）
    inline float distanceSquaredTo(const Point2D& p) const {
        const float dx = x - p.x;
        const float dy = y - p.y;
        return dx * dx + dy * dy;
    }
    
protected:
    float x;  // 改用 float 提升 ESP32 效能
    float y;
};
