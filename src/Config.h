#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <FS.h>
#include <vector>

static const int GPS_RX_PIN = 9;
static const int GPS_TX_PIN = 10;
static const uint32_t GPS_BAUD_RATE = 115200;


static const int NUM_LEDS = 8;
static const int STATUS_LED_PIN = 7;

static const char *AP_SSID = "ESP32_GPS_LOGGER";  
static const char *AP_PASSWORD = "12345678";

static const char *LOG_FOLDER = "/";
static const size_t MAX_LOG_FILE_SIZE = 512 * 1024;

// === OLED Display (SSD1309 128x64, SPI) ===
static const int OLED_CS_PIN   = 5;
static const int OLED_DC_PIN   = 21;
static const int OLED_RES_PIN  = 4;
static const int OLED_SCLK_PIN = 18;
static const int OLED_MOSI_PIN = 16;

struct DateTimeInfo {
  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;
  int second = 0;

  bool isValid() const {
    return year > 0 && month > 0 && day > 0;
  }
};

struct FileInfo {
  String name;
  size_t size;
};

struct GpsData {
  float speedKmh = 0.0f;   // 速度 km/h
  float altitude = 0.0f;   // 海拔 m
  int satCount = 0;        // 衛星數
  int fixType = 0;         // Fix 品質 (0=無, 2=2D, 3=3D)
  float headingDeg = 0.0f;  // 0~359.9
  double latitude = 0.0;
  double longitude = 0.0;
  bool latNorth = true;
  bool lonEast = true;
  bool hasValidFix = false;
  bool hasValidSpeed = false;
  bool hasValidHeading = false;
};

struct LapInfo {
  String currentLap = "00:00.0";    // "01:23.456"
  String lastLap = "00:00.0";
  String bestLap = "00:00.0";
  String deltaStr = "+0.000";       // 差距
  float deltaSeconds = 0;           // 正=快，負=慢
  int currentLapNum = 0;
  int lastLapNum = 0;
  int bestLapNum = 0;
  int totalLaps = 0;
};

#define STORAGE_FS SPIFFS

extern HardwareSerial GPS_SERIAL;
extern WebServer server;

#endif