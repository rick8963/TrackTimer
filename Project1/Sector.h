#pragma once

#include"Line2D.h"

class Sector
{
public:
	Sector(unsigned int startIndex, unsigned int endIndex);

	void setSectorTime(float time);
	float getSectorTime() const;

	void reset();
	void enter();
	void pass();
	bool isEntered() const;
	bool isPassed()const ;
	unsigned int getStartNodeIndex() const;
	unsigned int getEndNodeIndex() const;

private:
	float sectorTime;
	bool entered;
	bool passed;
	unsigned int startNodeIndex;
	unsigned int endNodeIndex;
};

