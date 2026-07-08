# Capability: track-timing
**Status:** Active

## Overview
Track timing is the race-oriented operating mode of the ESP32-based timer system. It presents lap-related timing values on the OLED display and maintains the lap timing state used by the rest of the system.

## Requirements

### Requirement: Display track timing values
The system MUST display current lap time, delta time, previous lap time, and best lap time in track mode.

The system MUST use the GPS location as the starting and ending line segment for judgment.

The system MUST use the optimal loop as the second difference reference.

The system MUST retain the last valid display value when GPS is invalid.

The system MUST not modify the existing OLED display.

The system MUST not define reset/pause/resume behaviors that do not yet exist.

#### Scenario: Valid lap data is available
**And** at least one valid lap has been recorded
**When** the display refreshes
**Then** the OLED shows the current lap time
**And** the OLED shows the delta time
**And** the OLED shows the previous lap time
**And** the OLED shows the best lap time

### Requirement: Update lap records on lap completion
The system MUST update lap timing records when a lap completion event is detected.

#### Scenario: A new lap is completed
**When** a lap completion event occurs
**Then** the previous lap value is updated
**And** the current lap value is reset for the new lap
**And** the best lap value is updated if the new lap is faster

### Requirement: Preserve valid lap data on invalid input
The system MUST preserve the most recent valid lap timing values when GPS or timing input becomes invalid.

#### Scenario: GPS signal becomes invalid
**Given** valid lap timing values already exist
**When** GPS validity is lost
**Then** the last valid lap values remain visible
**And** the system does not overwrite them with invalid data

### Requirement: Show state-dependent status
The system MUST present track timing state through the OLED and RGB LED status indicators.

#### Scenario: Timing is running
**And** timing is active
**When** the status indicators update
**Then** the OLED reflects running status
**And** the RGB LED reflects active track timing
