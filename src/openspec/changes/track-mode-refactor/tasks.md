# Implementation Tasks
**Change ID:** `track-mode-refactor`
**Status:** Draft

1. Inspect the current track timing implementation and identify the existing lap trigger, best-lap update logic, and delta calculation path.
2. Document the current GPS sample flow from input to timing engine so the implementation boundaries are clear.
3. Implement or refactor the internal track timing state model to hold current lap time, best lap time, last valid lap time, delta to best lap, GPS validity, and track state.
4. Implement lap completion detection based on GPS position crossing the configured start/finish line segment.
5. Add protection against duplicate lap events caused by noisy or repeated GPS samples.
6. Update lap record handling so that each valid lap updates current lap, best lap, and last valid display values correctly.
7. Implement delta calculation against the best lap time.
8. Ensure invalid GPS input does not overwrite the last valid displayed timing values.
9. Keep the existing OLED layout unchanged while feeding it from the updated track timing model.
10. Verify that the RGB LED status output still reflects the correct track timing state.
11. Add unit tests for line-segment crossing detection.
12. Add unit tests for lap completion updates and best-lap selection.
13. Add unit tests for delta-to-best calculation.
14. Add integration tests for invalid GPS fallback behavior and preservation of last valid values.
15. Run the project’s documented build and validation checks, and fix any failures.