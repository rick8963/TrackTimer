#pragma once

#include"Line2D.h"

class Sector
{
public:
	Sector(Line2D start, Line2D end);

	void setSectorTime(float time);
	float getSectorTime() const;

	void reset();
	void enter();
	void pass();
	bool isEntered() const;
	bool isPassed()const ;
	bool checkIfPassed(const Point2D& currentPos, const Point2D& lastPos);

private:
	float sectorTime;
	bool entered;
	bool passed;
	Line2D startPos;
	Line2D endPos;
};

