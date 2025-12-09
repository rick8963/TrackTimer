#pragma once
#include<vector>
#include"Sector.h"

class Lap {
private:
    double lapTime;
    std::vector<Sector> sectors;
    Line2D lapStartPoint;
    Line2D lapEndPoint;

public:
    Lap(int numSectors, Line2D start, Line2D end);
    void setLapTime(double time);
    double getLapTime() const;

    void setSectorTime(int index, double time);
    double getSectorTime(int index) const;

    // §PÂ_¬O§_§¹¦¨°é
    bool checkLapCompleted(const Point2D& currentPos, const Point2D& lastPos) const;
};