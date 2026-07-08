#pragma once

#include "Config.h"
#include "Track.h"
#include "Point2D.h"

enum class TrackState {
    Idle,
    Running,
    InvalidGps
};

struct TrackTimingModel {
    TimeMs currentLapTime = 0;
    TimeMs bestLapTime = 0;
    TimeMs lastValidLapTime = 0;
    TimeMs deltaToBestLap = 0;
    bool hasBestLap = false;
    bool hasDelta = false;
    bool gpsValidity = false;
    TrackState trackState = TrackState::Idle;
};

class TrackTimingEngine {
public:
    explicit TrackTimingEngine(Track& track);

    void feedGpsSample(const Point2D& pos, TimeMs timestamp, bool gpsValid);
    const TrackTimingModel& getModel() const { return _model; }
    LapInfo buildLapInfo(TimeMs nowTimestamp) const;
    LapInfo getDisplayLapInfo(TimeMs nowTimestamp, bool gpsValid);

    static String formatLapTime(TimeMs ms);
    static String formatDelta(TimeMs currentMs, TimeMs bestMs, bool hasBestLap, float* outSeconds);

private:
    Track& _track;
    TrackTimingModel _model;
    LapInfo _lastValidDisplay;
    bool _hasLastValidDisplay = false;

    void refreshModel(TimeMs nowTimestamp, bool gpsValid);
};
