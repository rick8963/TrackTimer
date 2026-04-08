#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include "Config.h"
#include "StorageManager.h"

// Circular buffer: 20 lines x 100 chars max = ~2 KB RAM
static const size_t LIVE_BUF_LINES = 20;
static const size_t LIVE_LINE_MAX  = 100;

class WebInterface {
public:
  explicit WebInterface(StorageManager &storage);
  void begin();
  void handleClient();

  // Called from main loop for every incoming NMEA line
  void pushNmeaLine(const String &line);
  bool isSSEConnected() const;
  void handleDelete();

private:
  void handleRoot();
  void handleDownload();
  void handleLiveSSE();
  String urlEncode(const String &value);
  void registerExtraRoutes();
  void handleStorageInfo();
  void handleFileList();

  StorageManager &_storage;

  // Fixed-size circular buffer (no heap alloc after init)
  char     _liveBuf[LIVE_BUF_LINES][LIVE_LINE_MAX];
  uint8_t  _liveHead = 0;   // next write position
  uint8_t  _liveCount = 0;  // how many lines are valid

  // SSE client tracking
  WiFiClient _sseClient;
  mutable bool _sseConnected = false;  // 避免直接存取 client
  bool       _sseActive = false;
};

#endif
