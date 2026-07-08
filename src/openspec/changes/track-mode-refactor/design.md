# Design: Track Mode Refactor
**Change ID:** `track-mode-refactor`
**Status:** Draft

## Overview
This change defines the technical approach for track mode timing in the ESP32-based timer system.

Track mode uses GPS as the only timing source. A lap is completed when the GPS position crosses the start/finish line segment. Lap time is treated as single-lap time, and delta time is calculated against the best lap. If GPS becomes invalid, the system preserves the last valid displayed values. The OLED layout is not changed by this change.

## Goals
- Keep lap timing logic deterministic and testable.
- Separate timing computation from display rendering.
- Preserve existing OLED layout.
- Preserve last valid displayed values when GPS is invalid.
- Support best-lap-based delta calculation.

## Non-Goals
- Change the existing OLED screen layout.
- Introduce reset, pause, or resume behavior.
- Change the timing source away from GPS.
- Add new hardware inputs for lap timing.
- Redesign the web UI.

## Architecture
The track mode flow is split into three layers:

1. Timing input layer.
2. Track timing engine.
3. Display state layer.

The input layer consumes GPS fixes and position samples. The timing engine decides whether a lap crossing occurred, updates lap records, and computes delta. The display state layer exposes values for OLED rendering without performing timing logic itself.

## Timing Model
The system maintains these internal values:
- `currentLapTime`.
- `bestLapTime`.
- `lastValidLapTime`.
- `deltaToBestLap`.
- `gpsValidity`.
- `trackState`.

The timing engine updates these values only when a valid GPS sample is available. If GPS is invalid, the engine does not overwrite the last valid display values.

## Lap Detection
A lap is completed when the GPS position crosses the configured start/finish line segment.

The lap detection logic SHOULD:
- Evaluate movement between consecutive GPS samples.
- Determine whether the path intersects the start/finish line segment.
- Ignore duplicate crossings caused by the same sample pair.
- Use a debounce or cooldown mechanism if required by the current implementation.

The exact geometric method can be implemented in code as long as it is consistent and testable.

## Best Lap and Delta
Best lap is the fastest valid lap time observed so far.

Delta time is calculated as:
- current lap time minus best lap time.

If no best lap exists yet, the display SHOULD show a neutral placeholder rather than a computed delta.

Best lap update rules:
- A valid lap can become the new best lap if it is faster.
- No lap category is excluded from best-lap comparison unless the input is explicitly marked invalid by the engine.
- First lap, warm-up lap, and other valid laps are all eligible for best-lap comparison.

## GPS Validity Handling
GPS is the only timing source. If GPS validity is lost:
- The system MUST keep the last valid displayed values.
- The system MUST not replace them with invalid or blank timing values.
- The timing engine may continue to monitor incoming samples, but display values should remain stable until a valid fix returns.

## Display Behavior
The OLED layout is unchanged by this change.

The display layer only consumes track timing data and renders the existing fields. It does not decide lap completion or best-lap logic.

If timing data is unavailable or invalid:
- Existing displayed values remain visible.
- No layout changes are introduced.

## State Model
The system uses a minimal track timing state model:
- `idle`
- `running`
- `invalid_gps`

No reset, pause, or resume behavior is defined in this change.

## Interfaces
The timing engine SHOULD expose functions or methods that allow:
- Feeding GPS samples.
- Detecting lap crossings.
- Retrieving current lap data.
- Retrieving best lap data.
- Retrieving display-ready values.

The display layer SHOULD consume a read-only model produced by the timing engine.

## Error Handling
- Invalid GPS samples MUST not overwrite valid timing output.
- Missing best-lap data SHOULD produce a placeholder delta display.
- Repeated crossings from noisy samples SHOULD be handled by the lap detection logic to avoid false lap events.

## Test Strategy
- Unit test line-segment crossing detection.
- Unit test lap completion updates.
- Unit test best-lap selection.
- Unit test delta-to-best calculation.
- Unit test invalid GPS fallback behavior.
- Integration test GPS sample sequences against expected lap events.