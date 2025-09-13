#pragma once
#include<time.h>
#include<vector>

using namespace std;

class LapTimer
{

private:
	clock_t currentLapTime;
	clock_t bestLapTime;
	clock_t latestLapTime;
	unsigned int lapCount;
	vector<clock_t> lapTimeLog;
};

