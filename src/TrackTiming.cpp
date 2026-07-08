#include "TrackTiming.h"
#include <stdio.h>

TrackTimingEngine::TrackTimingEngine(Track& track)
    : _track(track) {}

String TrackTimingEngine::formatLapTime(TimeMs ms) {
    if (ms == 0) return "00:00.0";
    const TimeMs minutes = ms / 60000;
    const TimeMs seconds = (ms % 60000) / 1000;
    const TimeMs fraction = ms % 1000 / 10;
    char buf[12];
    snprintf(buf, sizeof(buf), "%02u:%02u.%u", minutes, seconds, (unsigned)fraction);
    return String(buf);
}

String TrackTimingEngine::formatDelta(TimeMs currentMs, TimeMs bestMs, bool hasBestLap, float* outSeconds) {
    if (!hasBestLap || bestMs == 0) {
        if (outSeconds) *outSeconds = 0.0f;
        return "--.---";
    }

    const int32_t deltaMs = static_cast<int32_t>(currentMs) - static_cast<int32_t>(bestMs);
    const float deltaSec = deltaMs / 1000.0f;
    if (outSeconds) *outSeconds = deltaSec;

    char buf[12];
    snprintf(buf, sizeof(buf), "%+.3f", deltaSec);
    return String(buf);
}

void TrackTimingEngine::feedGpsSample(const Point2D& pos, TimeMs timestamp, bool gpsValid) {
    if (!gpsValid) {
        if (_hasLastValidDisplay) {
            _model.trackState = TrackState::InvalidGps;
        }
        _model.gpsValidity = false;
        return;
    }

    Point2D posCopy = pos;
    _track.updatePos(posCopy, timestamp);
    refreshModel(timestamp, true);
}

void TrackTimingEngine::refreshModel(TimeMs nowTimestamp, bool gpsValid) {
    _model.gpsValidity = gpsValid;

    const bool hasCurrentLap = _track.hasCurrentLap();
    _model.currentLapTime = 0;
    if (hasCurrentLap) {
        _model.currentLapTime = nowTimestamp - _track.getCurrentLapStartTime();
    }

    _model.bestLapTime = _track.getBestLapTime();
    _model.hasBestLap = _model.bestLapTime > 0;
    _model.lastValidLapTime = _track.getLatestLapTime();

    if (_model.hasBestLap && hasCurrentLap) {
        _model.deltaToBestLap = _model.currentLapTime - _model.bestLapTime;
        _model.hasDelta = true;
    } else {
        _model.deltaToBestLap = 0;
        _model.hasDelta = false;
    }

    if (_track.getSessionStartTime() == 0 && !hasCurrentLap && _model.lastValidLapTime == 0) {
        _model.trackState = TrackState::Idle;
    } else if (gpsValid) {
        _model.trackState = TrackState::Running;
    }
}

LapInfo TrackTimingEngine::buildLapInfo(TimeMs nowTimestamp) const {
    LapInfo lap;
    const int completedLaps = static_cast<int>(_track.getLaps().size());

    lap.totalLaps = completedLaps;
    lap.lastLapNum = _track.hasCurrentLap() ? completedLaps - 1 : 0;
    lap.currentLapNum = _track.hasCurrentLap() ? completedLaps : 0;

    float dist = _track.getNextCheckpoint().distanceToLine(_track.getCurrentPos());
    lap.distToNextSector = dist < 999.0f ? static_cast<int>(dist) : 999;

    TimeMs currentLapTime = 0;
    if (_track.hasCurrentLap()) {
        currentLapTime = nowTimestamp - _track.getCurrentLapStartTime();
    }

    lap.currentLap = formatLapTime(currentLapTime);
    lap.lastLap = formatLapTime(_track.getLatestLapTime());
    lap.bestLap = formatLapTime(_track.getBestLapTime());
    lap.bestLapNum = _track.getBestLapNum();
    lap.deltaStr = formatDelta(
        currentLapTime,
        _track.getBestLapTime(),
        _track.getBestLapTime() > 0,
        &lap.deltaSeconds);

    return lap;
}

LapInfo TrackTimingEngine::getDisplayLapInfo(TimeMs nowTimestamp, bool gpsValid) {
    if (gpsValid) {
        refreshModel(nowTimestamp, true);
        _lastValidDisplay = buildLapInfo(nowTimestamp);
        _hasLastValidDisplay = true;
        return _lastValidDisplay;
    }

    if (_hasLastValidDisplay) {
        _model.gpsValidity = false;
        _model.trackState = TrackState::InvalidGps;
        return _lastValidDisplay;
    }

    refreshModel(nowTimestamp, false);
    return buildLapInfo(nowTimestamp);
}
