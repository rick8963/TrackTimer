#include "StatusLED.h"

CRGB StatusLED::leds[NUM_LEDS];
uint32_t StatusLED::lastGPSTick = 0;
uint32_t StatusLED::lastSSETick = 0;

void StatusLED::begin() {
  FastLED.addLeds<WS2812, STATUS_LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);  // 低亮度省電
  FastLED.clear();
  FastLED.show();
}

void StatusLED::update(bool spiffsReady, bool gpsActive, bool logOpen, 
                      bool wifiReady, bool sseConnected, bool hasError) {
  uint32_t now = millis();
  
  // LED 0: 電源 (永遠綠)
  leds[0] = CRGB::Green;
  
  // LED 1: SPIFFS (綠閃)
  leds[1] = spiffsReady ? CRGB::Green : CRGB::Black;
  if (spiffsReady && (now % 2000 < 1000)) leds[1].fadeToBlackBy(128);
  
  // LED 2: GPS 10Hz (藍)
  static bool gpsBlink = false;
  if (gpsActive) {
    if (now % 500 < 250) {  // 500ms 週期，前 250ms 亮
      leds[2] = CRGB::Blue;
    } else {
      leds[2] = CRGB::DarkBlue;  // 暗藍，視覺對比
    }
  } else {
    leds[2] = CRGB::Black;
  }
  
  // LED 3: Log 檔案 (黃)
  leds[3] = logOpen ? CRGB::Yellow : CRGB::Black;
  
  // LED 4: WiFi AP (紫)
  leds[4] = wifiReady ? CRGB::Magenta : CRGB::Black;
  
  // LED 5: SSE 連線 (青 1Hz)
  if (sseConnected && ((now - lastSSETick) % 1000 < 500)) {
    leds[5] = CRGB::Cyan;
  } else {
    leds[5] = CRGB::Black;
  }
  
  // LED 6: 錯誤 (紅閃)
  leds[6] = hasError ? CRGB::Red : CRGB::Black;
  if (hasError && (now % 1000 < 500)) leds[6].fadeToBlackBy(128);
  
  // LED 7: 保留 (黑)
  leds[7] = CRGB::Black;
  
  FastLED.show();
  
  if (gpsActive) lastGPSTick = now;
}