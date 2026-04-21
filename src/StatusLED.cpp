#include "StatusLED.h"

CRGB StatusLED::leds[NUM_LEDS];

void StatusLED::begin() {
  FastLED.addLeds<WS2812, STATUS_LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  FastLED.clear();
  FastLED.show();
}

void StatusLED::update(bool spiffsReady, bool gpsFixValid, bool recordArmed, bool logOpen,
                       bool wifiReady, bool sseConnected, bool hasError) {
  uint32_t now = millis();

  // 先全部清掉
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // LED 0: 電源 (永遠綠)
  leds[0] = CRGB::Green;

  // LED 1: SPIFFS (綠閃)
  if (spiffsReady) {
    leds[1] = (now % 1000 < 500) ? CRGB::Green : CRGB::Black;
  }

  // LED 2: GPS (藍閃 / 紅常亮)
  if (gpsFixValid) {
    leds[2] = (now % 1000 < 500) ? CRGB::Blue : CRGB::Black;
  } else {
    leds[2] = CRGB::Red;
  }

  // LED 3: 錄製狀態
  // 未 armed: 關
  // armed 但未開檔: 黃閃（等待 GPS 有效時間）
  // 已開檔: 黃常亮（正在記錄）
  if (logOpen) {
    leds[3] = CRGB::Yellow;
  } else if (recordArmed) {
    leds[3] = (now % 1000 < 500) ? CRGB::Yellow : CRGB::Black;
  } else {
    leds[3] = CRGB::Black;
  }

  // LED 4: WiFi AP / client connected
  leds[4] = wifiReady ? CRGB::Magenta : CRGB::Black;

  // LED 5: SSE 連線 (青閃)
  if (sseConnected) {
    leds[5] = (now % 1000 < 500) ? CRGB::Cyan : CRGB::Black;
  }

  // LED 6: 錯誤 (紅閃)
  if (hasError) {
    leds[6] = (now % 1000 < 500) ? CRGB::Red : CRGB::Black;
  }

  // LED 7: 保留
  leds[7] = CRGB::Black;

  FastLED.show();
}