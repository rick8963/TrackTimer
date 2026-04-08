#ifndef GPS_RECEIVER_H
#define GPS_RECEIVER_H

#include "Config.h"

class GpsReceiver {
public:
  using LineHandler = void (*)(const String &line);

  void begin(uint32_t baud);
  void onLine(LineHandler handler);
  bool loop();

private:
  String _buffer;
  LineHandler _handler = nullptr;
};

#endif