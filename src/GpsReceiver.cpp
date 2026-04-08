#include "GpsReceiver.h"

void GpsReceiver::begin(uint32_t baud) {
  GPS_SERIAL.begin(baud, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("[GPS] Serial1 started");
}

void GpsReceiver::onLine(LineHandler handler) {
  _handler = handler;
}

bool GpsReceiver::loop() {
  static uint32_t lastData = 0;
  bool activity = false;
  while (GPS_SERIAL.available() > 0) {
    char c = static_cast<char>(GPS_SERIAL.read());
    Serial.write(c);
    activity = true;
    lastData = millis();

    _buffer += c;
    if (_buffer.length() > 256) {
      _buffer.remove(0, _buffer.length() - 256);
    }
    if (c == '\n') {
      String line = _buffer;
      line.trim();
      if (line.length() > 0 && _handler) {
        _handler(line);
      }
      _buffer = "";
    }
  }
  return (millis() - lastData < 100);
}