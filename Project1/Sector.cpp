#include "Sector.h"

Sector::Sector(unsigned int startIndex, unsigned int endIndex):
	startNodeIndex(startIndex), endNodeIndex(endIndex)
{
	reset();
}

void Sector::enter() { entered = true; }
void Sector::pass() { passed = true; }
bool Sector::isEntered() const{	return entered; }
bool Sector::isPassed() const { return passed; }
unsigned int Sector::getStartNodeIndex() const{	return startNodeIndex; }
unsigned int Sector::getEndNodeIndex() const{ return endNodeIndex; }

void Sector::reset()
{
	entered = false;
	passed = false;
}