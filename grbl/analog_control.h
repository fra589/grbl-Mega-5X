/*
  analog_control.h - Read analog input
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

#ifndef analog_control_h
#define analog_control_h

#define OUTPUT_PWM_STATE_OFF  0 // Must be 0
#define OUTPUT_PWM_STATE_ON       bit(0)

#ifndef cbi
  #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
  #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#ifdef USE_OUTPUT_PWM

float o_pwm_gradient();

  void output_pwm_init();
  uint8_t output_pwm_get_state();
  void output_pwm_stop();
  void output_pwm_set_value(uint16_t pwm_value);
  uint16_t output_compute_pwm_value(float volts);
  void output_pwm_set_state(uint8_t state, float volts);
  void output_pwm_sync(uint8_t state, float volts);
#endif // USE_OUTPUT_PWM

#endif
