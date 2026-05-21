/*
  digital_control.h - digital output M62-M65 control methods
  Part of Grbl

  Copyright (c) 2017-2026 Gauthier Briere

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef digital_control_h
#define digital_control_h

#define DIGITAL_OUTPUT_STATE_OFF   0  // Must be zero
#define DIGITAL_OUTPUT_STATE_P0    bit(0)
#define DIGITAL_OUTPUT_STATE_P1    bit(1)
#define DIGITAL_OUTPUT_STATE_P2    bit(2)
#define DIGITAL_OUTPUT_STATE_P3    bit(3)
#ifdef USE_DIGITAL_INPUT
  #define DIGITAL_INPUT_STATE_P0     bit(4)
  #define DIGITAL_INPUT_STATE_P1     bit(5)
  #define DIGITAL_INPUT_STATE_P2     bit(6)
  #define DIGITAL_INPUT_STATE_P3     bit(7)
#endif

// Initializes digital output control pins.
void digital_init();

// Returns current digital output state. Overrides may alter it from programmed state.
uint8_t digital_get_state();

// Immediately disables digital output pins.
void digital_stop(const uint8_t n);

// Sets the digital output pins according to state specified.
void digital_set_state(uint8_t mode);

// G-code parser entry-point for setting digital output states. Checks for and executes additional conditions.
void digital_sync(uint8_t mode);

#endif
