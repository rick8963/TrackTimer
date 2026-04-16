#include "GpsTimeParser.h"

static double nmeaToDecimalDegrees(const String &raw, bool isLatitude) {
    if (raw.length() < 4) return 0.0;

    double value = raw.toDouble();
    int degrees = 0;
    double minutes = 0.0;

    if (isLatitude) {
        degrees = (int)(value / 100);
        minutes = value - (degrees * 100);
    } else {
        degrees = (int)(value / 100);
        minutes = value - (degrees * 100);
    }

    return degrees + (minutes / 60.0);
}

void GpsTimeParser::processLine(const String &line) {
  if (line.startsWith("$GPRMC") || line.startsWith("$GNRMC")) {
        // split by comma
        String fields[16];
        int fieldCount = 0;
        int start = 0;
        for (int i = 0; i <= line.length() && fieldCount < 16; i++) {
            if (i == line.length() || line[i] == ',') {
                fields[fieldCount++] = line.substring(start, i);
                start = i + 1;
            }
        }

        // fields:
        // 1 time, 2 status, 3 lat, 4 N/S, 5 lon, 6 E/W, 7 speed(knots), 8 course, 9 date
        if (fieldCount > 9 && fields[2] == "A") {
            _gpsData.latitude = nmeaToDecimalDegrees(fields[3], true);
            _gpsData.longitude = nmeaToDecimalDegrees(fields[5], false);
            _gpsData.latNorth = (fields[4] != "S");
            _gpsData.lonEast = (fields[6] != "W");

            if (!_gpsData.latNorth) _gpsData.latitude = -_gpsData.latitude;
            if (!_gpsData.lonEast) _gpsData.longitude = -_gpsData.longitude;

            _gpsData.speedKmh = fields[7].toFloat() * 1.852f;   // knots -> km/h
            _gpsData.headingDeg = fields[8].toFloat();
            _gpsData.hasValidSpeed = true;
            _gpsData.hasValidHeading = true;
            _gpsData.hasValidFix = true;
        }
    }

    if (line.startsWith("$GPGGA") || line.startsWith("$GNGGA")) {
        String fields[16];
        int fieldCount = 0;
        int start = 0;
        for (int i = 0; i <= line.length() && fieldCount < 16; i++) {
            if (i == line.length() || line[i] == ',') {
                fields[fieldCount++] = line.substring(start, i);
                start = i + 1;
            }
        }

        // fields:
        // 6 fix quality, 7 satellites
        if (fieldCount > 7) {
            _gpsData.fixType = fields[6].toInt();
            _gpsData.satCount = fields[7].toInt();
            _gpsData.hasValidFix = (_gpsData.fixType > 0);
        }
  }

  const int MAX_FIELDS = 20;
  String fields[MAX_FIELDS];
  int fieldIndex = 0;
  int start = 0;
  int len = line.length();

  for (int i = 0; i <= len && fieldIndex < MAX_FIELDS; ++i) {
    if (i == len || line[i] == ',') {
      fields[fieldIndex++] = line.substring(start, i);
      start = i + 1;
    }
  }

  if (fieldIndex < 10) {
    return;
  }

  String timeStr = fields[1];
  String dateStr = fields[9];

  if (timeStr.length() < 6 || dateStr.length() != 6) {
    return;
  }

  int hh = timeStr.substring(0, 2).toInt();
  int mm = timeStr.substring(2, 4).toInt();
  int ss = timeStr.substring(4, 6).toInt();

  int dd = dateStr.substring(0, 2).toInt();
  int mo = dateStr.substring(2, 4).toInt();
  int yy = dateStr.substring(4, 6).toInt();

  int fullYear = 2000 + yy;

  if (hh < 0 || hh > 23 || mm < 0 || mm > 59 || ss < 0 || ss > 59) {
    return;
  }
  if (dd <= 0 || dd > 31 || mo <= 0 || mo > 12) {
    return;
  }

  _current.year = fullYear;
  _current.month = mo;
  _current.day = dd;
  _current.hour = hh;
  _current.minute = mm;
  _current.second = ss;
  _hasValidTime = true;
}

bool GpsTimeParser::hasValidTime() const {
  return _hasValidTime && _current.isValid();
}

DateTimeInfo GpsTimeParser::current() const {
  return _current;
}