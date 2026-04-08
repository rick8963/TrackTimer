#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <FastLED.h>
#include "Config.h"

/*
LED 位置	顏色/模式	    狀態意義	        觸發條件
LED 0	    🟢 常亮	        電源正常	        程式啟動後永遠亮
LED 1	    🟢 1Hz 閃爍	    SPIFFS 就緒         g_storageReady == true
LED 2	    🔵 10Hz 快速閃	GPS 接收中	        g_gpsReceiver.loop() 有資料
LED 3	    🟡 常亮	        Log 檔案開啟	    g_logFileOpened == true
LED 4	    🟣 常亮	        Wi-Fi AP 就緒	    WiFi.softAPgetStationNum() > 0
LED 5	    🟢 1Hz 閃爍	    SSE Live View 連線	g_web.isSSEConnected() == true
LED 6	    🔴 1Hz 閃爍	    錯誤狀態	        SPIFFS/GPS/WiFi 失敗
LED 7	    ⚫ 關閉	       保留（計時器用）	    未來圈速/狀態顯示
*/


class StatusLED {
public:
  static void begin();
  static void update(bool spiffsReady, bool gpsActive, bool logOpen, 
                    bool wifiReady, bool sseConnected, bool hasError);
  
private:
  static CRGB leds[NUM_LEDS];
  static uint32_t lastGPSTick;
  static uint32_t lastSSETick;
};

#endif