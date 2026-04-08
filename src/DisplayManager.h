#pragma once

#include "Config.h"
#include "GpsTimeParser.h"
#include "WebInterface.h"


class DisplayManager {
public:
    void begin();
    void update(
        bool storageReady,
        bool gpsActive,
        bool logFileOpened,
        bool wifiActive,
        bool sseActive,
        const DateTimeInfo &timeInfo,
        const GpsData &gps,
        const LapInfo &lap,
        uint32_t loopCount
    );

private:
    enum DisplayMode {
        DEBUG_MODE,
        DRIVE_MODE,
        TRACK_MODE
    };

    DisplayMode mode = DEBUG_MODE;
    DisplayMode _lastMode = DEBUG_MODE;

    static const int BUTTON_PIN = 11;
    uint32_t lastButtonPress = 0;
    uint32_t _lastDisplayMs = 0;

    bool _storageReady = false;
    bool _gpsActive = false;
    bool _logFileOpened = false;
    bool _wifiActive = false;
    bool _sseActive = false;

    DateTimeInfo _timeInfo;
    GpsData _gpsData;
    LapInfo _lapInfo;

    DateTimeInfo _lastTimeInfo;
    GpsData _lastGpsData;
    LapInfo _lastLapInfo;
    
    void drawDebugMode();
    void drawDriveMode(const GpsData &gps, const DateTimeInfo &timeInfo);
    void drawTrackMode(const GpsData &gps, const LapInfo &lap);

    bool needsRedraw() const;
    bool buttonPressed();
};