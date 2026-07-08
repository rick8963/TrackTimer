# Implementation Tasks
**Change ID:** `track-mode-refactor`
**Status:** Complete

1. [x] Inspect the current track timing implementation and identify the existing lap trigger, best-lap update logic, and delta calculation path. → `current-implementation.md`
2. [x] Document the current GPS sample flow from input to timing engine so the implementation boundaries are clear. → `gps-sample-flow.md`
3. [x] Implement or refactor the internal track timing state model to hold current lap time, best lap time, last valid lap time, delta to best lap, GPS validity, and track state. → `TrackTimingEngine`
4. [x] Implement lap completion detection based on GPS position crossing the configured start/finish line segment. → existing `Track` + `Line2D::pathCrossesSegment`
5. [x] Add protection against duplicate lap events caused by noisy or repeated GPS samples. → `Track::MIN_LAP_INTERVAL_MS` debounce
6. [x] Update lap record handling so that each valid lap updates current lap, best lap, and last valid display values correctly.
7. [x] Implement delta calculation against the best lap time. → `TrackTimingEngine::formatDelta`
8. [x] Ensure invalid GPS input does not overwrite the last valid displayed timing values. → `getDisplayLapInfo`
9. [x] Keep the existing OLED layout unchanged while feeding it from the updated track timing model.
10. [x] Verify that the RGB LED status output still reflects the correct track timing state. → unchanged `StatusLED`, GPS fix drives LED 5
11. [x] Add unit tests for line-segment crossing detection. → `test/test_line_crossing`
12. [x] Add unit tests for lap completion updates and best-lap selection. → `test/test_lap_updates`
13. [x] Add unit tests for delta-to-best calculation. → `test/test_delta`
14. [x] Add integration tests for invalid GPS fallback behavior and preservation of last valid values. → `test/test_invalid_gps`
15. [x] Run the project's documented build and validation checks, and fix any failures. → ESP32 build PASS; native tests require host `g++`
