/*
  motion_control.h - high level interface for issuing motion commands
  Part of Grbl

  Copyright (c) 2017-2026 Gauthier Briere
  Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

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

#ifndef motion_control_h
#define motion_control_h


// System motion commands must have a line number of zero.
#define HOMING_CYCLE_LINE_NUMBER 0
#define PARKING_MOTION_LINE_NUMBER 0

#define HOMING_CYCLE_ALL  0  // Must be zero.

// Execute linear motion in absolute millimeter coordinates. Feed rate given in millimeters/second
// unless invert_feed_rate is true. Then the feed_rate means that the motion should be completed in
// (1 minute)/feed_rate time.
void mc_line(float *target, plan_line_data_t *pl_data);

// Execute an arc in offset mode format. position == current xyz, target == target xyz,
// offset == offset from current xyz, axis_XXX defines circle plane in tool space, axis_linear is
// the direction of helical travel, radius == circle radius, is_clockwise_arc boolean. Used
// for vector transformation direction.
void mc_arc(float *target, plan_line_data_t *pl_data, float *position, float *offset, float radius,
  uint8_t axis_0, uint8_t axis_1, uint8_t axis_linear, uint8_t axis_0_mask, uint8_t axis_1_mask, uint8_t axis_linear_mask,
  uint8_t axis_a, uint8_t axis_b, uint8_t axis_c, uint8_t axis_a_mask, uint8_t axis_b_mask, uint8_t axis_c_mask,
  uint8_t axis_u, uint8_t axis_v, uint8_t axis_w, uint8_t axis_u_mask, uint8_t axis_v_mask, uint8_t axis_w_mask,
  uint8_t axis_d, uint8_t axis_e, uint8_t axis_h, uint8_t axis_d_mask, uint8_t axis_e_mask, uint8_t axis_h_mask,
  uint8_t is_clockwise_arc);

// Dwell for a specific number of seconds
void mc_dwell(float seconds);

// Perform homing cycle to locate machine zero. Requires limit switches.
void mc_homing_cycle(uint8_t cycle_mask);

// Perform tool length probe cycle. Requires probe switch.
uint8_t mc_probe_cycle(float *target, plan_line_data_t *pl_data, uint8_t parser_flags);

// Handles updating the override control state.
void mc_override_ctrl_update(uint8_t override_state);

// Plans and executes the single special motion case for parking. Independent of main planner buffer.
void mc_parking_motion(float *parking_target, plan_line_data_t *pl_data);

// Performs system reset. If in motion state, kills all motion and sets system alarm.
void mc_reset();

#endif
