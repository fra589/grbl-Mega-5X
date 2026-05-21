/*
  digital_control.c - digital output M62-M65 control methods
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

#include "grbl.h"


void digital_init()
{
  // Outputs
  DIGITAL_OUTPUT_DDR_0 |= (1 << DIGITAL_OUTPUT_BIT_0); // Configure as output pin.
  DIGITAL_OUTPUT_DDR_1 |= (1 << DIGITAL_OUTPUT_BIT_1); // Configure as output pin.
  DIGITAL_OUTPUT_DDR_2 |= (1 << DIGITAL_OUTPUT_BIT_2); // Configure as output pin.
  DIGITAL_OUTPUT_DDR_3 |= (1 << DIGITAL_OUTPUT_BIT_3); // Configure as output pin.
  digital_stop(0x0F);
  #ifdef USE_DIGITAL_INPUT
    // Input
    DIGITAL_INPUT_DDR_0 &= ~(DIGITAL_INPUT_MASK_0); // Configure as input pin
    DIGITAL_INPUT_DDR_1 &= ~(DIGITAL_INPUT_MASK_1); // Configure as input pin
    DIGITAL_INPUT_DDR_2 &= ~(DIGITAL_INPUT_MASK_2); // Configure as input pin
    DIGITAL_INPUT_DDR_3 &= ~(DIGITAL_INPUT_MASK_3); // Configure as input pin
    #ifdef DISABLE_DIGITAL_INPUT_PIN_PULL_UP
      DIGITAL_INPUT_PORT_0 &= ~(DIGITAL_INPUT_MASK_0); // Normal low operation. Requires external pull-down.
      DIGITAL_INPUT_PORT_1 &= ~(DIGITAL_INPUT_MASK_1); // Normal low operation. Requires external pull-down.
      DIGITAL_INPUT_PORT_2 &= ~(DIGITAL_INPUT_MASK_2); // Normal low operation. Requires external pull-down.
      DIGITAL_INPUT_PORT_3 &= ~(DIGITAL_INPUT_MASK_3); // Normal low operation. Requires external pull-down.
    #else
      DIGITAL_INPUT_PORT_0 |= DIGITAL_INPUT_MASK_0;    // Enable internal pull-up resistors. Normal high operation.
      DIGITAL_INPUT_PORT_1 |= DIGITAL_INPUT_MASK_1;    // Enable internal pull-up resistors. Normal high operation.
      DIGITAL_INPUT_PORT_2 |= DIGITAL_INPUT_MASK_2;    // Enable internal pull-up resistors. Normal high operation.
      DIGITAL_INPUT_PORT_3 |= DIGITAL_INPUT_MASK_3;    // Enable internal pull-up resistors. Normal high operation.
    #endif
  #endif
}


// Returns current digital output state. Overrides may alter it from programmed state.
uint8_t digital_get_state()
{
  uint8_t digital_state = DIGITAL_OUTPUT_STATE_OFF;
  // Output status
  #ifdef INVERT_DIGITAL_OUTPUT_PIN_0
    if (bit_isfalse(DIGITAL_OUTPUT_PORT_0,(1 << DIGITAL_OUTPUT_BIT_0))) {
  #else
    if (bit_istrue(DIGITAL_OUTPUT_PORT_0,(1 << DIGITAL_OUTPUT_BIT_0))) {
  #endif
    digital_state |= DIGITAL_OUTPUT_STATE_P0;
  }
  #ifdef INVERT_DIGITAL_OUTPUT_PIN_1
    if (bit_isfalse(DIGITAL_OUTPUT_PORT_1,(1 << DIGITAL_OUTPUT_BIT_1))) {
  #else
    if (bit_istrue(DIGITAL_OUTPUT_PORT_1,(1 << DIGITAL_OUTPUT_BIT_1))) {
  #endif
    digital_state |= DIGITAL_OUTPUT_STATE_P1;
  }
  #ifdef INVERT_DIGITAL_OUTPUT_PIN_2
    if (bit_isfalse(DIGITAL_OUTPUT_PORT_2,(1 << DIGITAL_OUTPUT_BIT_2))) {
  #else
    if (bit_istrue(DIGITAL_OUTPUT_PORT_2,(1 << DIGITAL_OUTPUT_BIT_2))) {
  #endif
    digital_state |= DIGITAL_OUTPUT_STATE_P2;
  }
  #ifdef INVERT_DIGITAL_OUTPUT_PIN_3
    if (bit_isfalse(DIGITAL_OUTPUT_PORT_3,(1 << DIGITAL_OUTPUT_BIT_3))) {
  #else
    if (bit_istrue(DIGITAL_OUTPUT_PORT_3,(1 << DIGITAL_OUTPUT_BIT_3))) {
  #endif
    digital_state |= DIGITAL_OUTPUT_STATE_P3;
  }
  // Input status
  #ifdef USE_DIGITAL_INPUT
    #ifdef INVERT_DIGITAL_INPUT_PIN_0
      if (DIGITAL_INPUT_PIN_0 & DIGITAL_INPUT_MASK_0) {
    #else
      if (!(DIGITAL_INPUT_PIN_0 & DIGITAL_INPUT_MASK_0)) {
    #endif
      digital_state |= DIGITAL_INPUT_STATE_P0;
    }
    #ifdef INVERT_DIGITAL_INPUT_PIN_1
      if (DIGITAL_INPUT_PIN_1 & DIGITAL_INPUT_MASK_1) {
    #else
      if (!(DIGITAL_INPUT_PIN_1 & DIGITAL_INPUT_MASK_1)) {
    #endif
      digital_state |= DIGITAL_INPUT_STATE_P1;
    }
    #ifdef INVERT_DIGITAL_INPUT_PIN_2
      if (DIGITAL_INPUT_PIN_2 & DIGITAL_INPUT_MASK_2) {
    #else
      if (!(DIGITAL_INPUT_PIN_2 & DIGITAL_INPUT_MASK_2)) {
    #endif
      digital_state |= DIGITAL_INPUT_STATE_P2;
    }
    #ifdef INVERT_DIGITAL_INPUT_PIN_3
      if (DIGITAL_INPUT_PIN_3 & DIGITAL_INPUT_MASK_3) {
    #else
      if (!(DIGITAL_INPUT_PIN_3 & DIGITAL_INPUT_MASK_3)) {
    #endif
      digital_state |= DIGITAL_INPUT_STATE_P3;
    }
  #endif
  return(digital_state);
}


// Directly called by digital_init(), digital_set_state(), and mc_reset(), which can be at
// an interrupt-level. No report flag set, but only called by routines that don't need it.
// The mode argument is a bit flag wich define which output must be stopped.
void digital_stop(const uint8_t mode)
{
  if (mode & DIGITAL_OUTPUT_STATE_P0) {
    #ifdef INVERT_DIGITAL_OUTPUT_PIN_0
      DIGITAL_OUTPUT_PORT_0 |= (1 << DIGITAL_OUTPUT_BIT_0);
    #else
      DIGITAL_OUTPUT_PORT_0 &= ~(1 << DIGITAL_OUTPUT_BIT_0);
    #endif
  }
  if (mode & DIGITAL_OUTPUT_STATE_P1) {
    #ifdef INVERT_DIGITAL_OUTPUT_PIN_1
      DIGITAL_OUTPUT_PORT_1 |= (1 << DIGITAL_OUTPUT_BIT_1);
    #else
      DIGITAL_OUTPUT_PORT_1 &= ~(1 << DIGITAL_OUTPUT_BIT_1);
    #endif
  }
  if (mode & DIGITAL_OUTPUT_STATE_P2) {
    #ifdef INVERT_DIGITAL_OUTPUT_PIN_2
      DIGITAL_OUTPUT_PORT_2 |= (1 << DIGITAL_OUTPUT_BIT_2);
    #else
      DIGITAL_OUTPUT_PORT_2 &= ~(1 << DIGITAL_OUTPUT_BIT_2);
    #endif
  }
  if (mode & DIGITAL_OUTPUT_STATE_P3) {
    #ifdef INVERT_DIGITAL_OUTPUT_PIN_3
      DIGITAL_OUTPUT_PORT_3 |= (1 << DIGITAL_OUTPUT_BIT_3);
    #else
      DIGITAL_OUTPUT_PORT_3 &= ~(1 << DIGITAL_OUTPUT_BIT_3);
    #endif
  }
}


// Immediately sets digital outputs
// Also sets a flag to report an update to a digital state.
void digital_set_state(uint8_t mode)
{
  if (sys.abort) { return; } // Block during abort.  
  
  if (mode & DIGITAL_OUTPUT_STATE_P0) {
    #ifdef INVERT_DIGITAL_OUTPUT_PIN_0
      DIGITAL_OUTPUT_PORT_0 &= ~(1 << DIGITAL_OUTPUT_BIT_0);
    #else
      DIGITAL_OUTPUT_PORT_0 |= (1 << DIGITAL_OUTPUT_BIT_0);
    #endif
  } else {
    #ifdef INVERT_DIGITAL_OUTPUT_PIN_0
      DIGITAL_OUTPUT_PORT_0 |= (1 << DIGITAL_OUTPUT_BIT_0);
    #else
      DIGITAL_OUTPUT_PORT_0 &= ~(1 << DIGITAL_OUTPUT_BIT_0);
    #endif
  }
  if (mode & DIGITAL_OUTPUT_STATE_P1) {
    #ifdef INVERT_DIGITAL_OUTPUT_PIN_1
      DIGITAL_OUTPUT_PORT_1 &= ~(1 << DIGITAL_OUTPUT_BIT_1);
    #else
      DIGITAL_OUTPUT_PORT_1 |= (1 << DIGITAL_OUTPUT_BIT_1);
    #endif
  } else {
    #ifdef INVERT_DIGITAL_OUTPUT_PIN_1
      DIGITAL_OUTPUT_PORT_1 |= (1 << DIGITAL_OUTPUT_BIT_1);
    #else
      DIGITAL_OUTPUT_PORT_1 &= ~(1 << DIGITAL_OUTPUT_BIT_1);
    #endif
  }
  if (mode & DIGITAL_OUTPUT_STATE_P2) {
    #ifdef INVERT_DIGITAL_OUTPUT_PIN_2
      DIGITAL_OUTPUT_PORT_2 &= ~(1 << DIGITAL_OUTPUT_BIT_2);
    #else
      DIGITAL_OUTPUT_PORT_2 |= (1 << DIGITAL_OUTPUT_BIT_2);
    #endif
  } else {
    #ifdef INVERT_DIGITAL_OUTPUT_PIN_2
      DIGITAL_OUTPUT_PORT_2 |= (1 << DIGITAL_OUTPUT_BIT_2);
    #else
      DIGITAL_OUTPUT_PORT_2 &= ~(1 << DIGITAL_OUTPUT_BIT_2);
    #endif
  }
  if (mode & DIGITAL_OUTPUT_STATE_P3) {
    #ifdef INVERT_DIGITAL_OUTPUT_PIN_3
      DIGITAL_OUTPUT_PORT_3 &= ~(1 << DIGITAL_OUTPUT_BIT_3);
    #else
      DIGITAL_OUTPUT_PORT_3 |= (1 << DIGITAL_OUTPUT_BIT_3);
    #endif
  } else {
    #ifdef INVERT_DIGITAL_OUTPUT_PIN_3
      DIGITAL_OUTPUT_PORT_3 |= (1 << DIGITAL_OUTPUT_BIT_3);
    #else
      DIGITAL_OUTPUT_PORT_3 &= ~(1 << DIGITAL_OUTPUT_BIT_3);
    #endif
  }
  sys.report_ovr_counter = 0; // Set to report change immediately
}


// G-code parser entry-point for setting digital state. Forces a planner buffer sync and bails 
// if an abort or check-mode is active.
void digital_sync(uint8_t mode)
{
  if (sys.state == STATE_CHECK_MODE) { return; }
  protocol_buffer_synchronize(); // Ensure digital turns on when specified in program.
  digital_set_state(mode);
}
