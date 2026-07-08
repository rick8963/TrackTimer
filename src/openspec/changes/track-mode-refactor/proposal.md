# Proposal: Track Mode Refactor
**Change ID:** `track-mode-refactor`
**Status:** Draft
**Date:** 2026-07-08
**Author:** rick8963

## Executive Summary
This change refactors the existing track mode into a clearly specified, testable, and maintainable capability for the ESP32-based lap timer system.

The current project already supports track mode display and core lap timing logic. This change formalizes the expected behavior of track timing, lap comparison, and display update rules so the system can be extended safely in future iterations.

## Background
The project is intended to become a full replacement for a Qstarz-style track timing device on ESP32. Current functionality already includes GPS logging, web access, OLED display modes, RGB LED status indication, and physical button controls.

Track mode is the core user-facing feature for racing use. It must reliably present current lap time, delta time, previous lap time, and best lap time while staying synchronized with the timing engine.

## Goals
- Define track mode behavior as a stable, testable specification.
- Clarify how lap timing values are computed and displayed.
- Ensure the OLED and RGB LED reflect track mode status consistently.
- Preserve existing behavior where it already works.

## Scope / Non-Goals
### In Scope
- Track mode timing display behavior.
- Lap time, delta, last lap, and best lap display rules.
- Track-mode status indicators on OLED and RGB LED.
- Interaction between track mode and start/stop recording control.
- Error and fallback behavior related to invalid or unavailable timing data.

### Non-Goals
- Redesigning the GPS hardware stack.
- Adding new sensors.
- Changing the web UI layout.
- Reworking file storage format for GPS logs.
- Replacing the existing core timing algorithm unless required for specification compliance.

## Approach
1. Document the current track mode as the source of truth for existing behavior.
2. Define a formal state model for timing and display updates.
3. Specify how lap values are derived, rounded, and reset.
4. Add explicit scenarios for normal operation and edge cases.
5. Break implementation work into small tasks that can be verified independently.

## Risks & Mitigations
- **Risk:** Timing values may differ from current output after refactoring.
  - **Mitigation:** Preserve existing calculations unless a defect is confirmed.
- **Risk:** Display refresh could introduce flicker or latency.
  - **Mitigation:** Define update intervals and minimal redraw rules.
- **Risk:** GPS loss may disrupt lap computation.
  - **Mitigation:** Specify fallback behavior for signal loss and invalid fixes.

## Validation & Metrics
- Track mode must display the correct timing fields consistently.
- Lap transitions must be reproducible from the same input sequence.
- UI state must match timing state within a defined update delay.
- No regression in existing GPS logging or web access behavior.

## Open Questions
- What exact lap trigger logic is currently used?
- Is delta time computed against best lap, previous lap, or a target lap?
- Should the system freeze the last valid values when GPS quality drops?
- What is the acceptable UI refresh rate for the OLED display?