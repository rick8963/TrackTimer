#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <FastLED.h>
#include "Config.h"

/*
LED 位置  顏色/模式         狀態意義
LED 0     🟢 常亮           電源正常
LED 1     🟢 1Hz 閃爍       SPIFFS 就緒
LED 2     🔵 1Hz 閃爍 / 🔴常亮  GPS 接收中 / 不活躍
LED 3     🟡 1Hz 閃爍 / 🟡常亮  等待 GPS 開始錄 / 正在記錄
LED 4     🟣 常亮           Wi-Fi AP 有連線
LED 5     🟢 1Hz 閃爍       SSE Live View 連線
LED 6     🔴 1Hz 閃爍       錯誤狀態
LED 7     ⚫ 關閉           保留
*/

class StatusLED {
public:
  static void begin();
  static void update(bool spiffsReady, bool gpsFixValid, bool recordArmed, bool logOpen,
                     bool wifiReady, bool sseConnected, bool hasError);

private:
  static CRGB leds[NUM_LEDS];
};

#endif