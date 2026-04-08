#ifndef GPS_TIME_PARSER_H
#define GPS_TIME_PARSER_H

#include "Config.h"

class GpsTimeParser {
public:
  void processLine(const String &line);
  bool hasValidTime() const;
  DateTimeInfo current() const;
  GpsData currentGps() const { return _gpsData; }

private:
  DateTimeInfo _current;
  bool _hasValidTime = false;
  GpsData _gpsData;
};

#endif