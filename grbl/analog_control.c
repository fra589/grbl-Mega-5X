/*
  analog_control.c - Read analog input
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


#ifdef USE_OUTPUT_PWM

static float output_pwm_gradient; // Precalulated value to speed up volts to PWM conversions.

float o_pwm_gradient()
{
  return output_pwm_gradient;
}

void output_pwm_init()
{
  // Force PWM output zero to avoid PWM output flash on laser output
  OUTPUT_TCCRA_REGISTER &= ~(1<<OUTPUT_COMB_BIT); // Disable PWM. Output voltage is zero.
  // Configure variable spindle PWM and enable pin, if required.
  OUTPUT_PWM_DDR |= (1<<OUTPUT_PWM_BIT); // Configure as PWM output pin.
  OUTPUT_TCCRA_REGISTER = OUTPUT_TCCRA_INIT_MASK; // Configure PWM output compare timer
  OUTPUT_TCCRB_REGISTER = OUTPUT_TCCRB_INIT_MASK;
  OUTPUT_OCRA_REGISTER = OUTPUT_OCRA_TOP_VALUE; // Set the top value for 16-bit fast PWM mode

  output_pwm_gradient = OUTPUT_PWM_RANGE/(settings.volts_max-settings.volts_min);
  output_pwm_stop();
}

uint8_t output_pwm_get_state()
{
  if ((OUTPUT_TCCRA_REGISTER & (1<<OUTPUT_COMB_BIT))) {
    return OUTPUT_PWM_STATE_ON;
  } else {
    return OUTPUT_PWM_STATE_OFF;
  }
}

void output_pwm_stop()
{
  OUTPUT_TCCRA_REGISTER &= ~(1<<OUTPUT_COMB_BIT); // Disable PWM. Output voltage is zero.
}

void output_pwm_set_value(uint16_t pwm_value)
{
  OUTPUT_OCR_REGISTER = pwm_value; // Set PWM output level.
  if (pwm_value == OUTPUT_PWM_OFF_VALUE) {
    OUTPUT_TCCRA_REGISTER &= ~(1<<OUTPUT_COMB_BIT); // Disable PWM. Output voltage is zero.
  } else {
    OUTPUT_TCCRA_REGISTER |= (1<<OUTPUT_COMB_BIT); // Ensure PWM output is enabled.
  }
}

uint16_t output_compute_pwm_value(float volts)
{
  uint16_t pwm_value;
	if ((settings.volts_min >= settings.volts_max) || (volts >= settings.volts_max)) {
	  // No PWM range possible. Set simple on/off output control pin state.
	  sys.output_volts = settings.volts_max;
	  pwm_value = OUTPUT_PWM_MAX_VALUE;
	} else if (volts <= settings.volts_min) {
	  if (volts == 0.0) { // S0 disables output
		sys.output_volts = 0.0;
		pwm_value = OUTPUT_PWM_OFF_VALUE;
	  } else { // Set minimum PWM output
		sys.output_volts = settings.volts_min;
		pwm_value = OUTPUT_PWM_MIN_VALUE;
	  }
	} else { 
	  // Compute intermediate PWM value with linear output volts model.
	  // NOTE: A nonlinear model could be installed here, if required, but keep it VERY light-weight.
	  sys.output_volts = volts;
	  pwm_value = floor((volts - settings.volts_min)*output_pwm_gradient) + OUTPUT_PWM_MIN_VALUE;
	}
  return(pwm_value);
}

void output_pwm_set_state(uint8_t state, float volts)
{
  if (sys.abort) { return; } // Block during abort.
  if (state == OUTPUT_PWM_STATE_OFF) { // Stop output PWM.
    sys.output_volts = 0.0;
    output_pwm_stop();
  } else {
    // NOTE: In laser mode, output will be active only when moving (STATE_CYCLE) or sleeping (STATE_SLEEP)
    if (settings.flags & BITFLAG_LASER_MODE) {
      if ((sys.state == STATE_IDLE) || (sys.state & (STATE_ALARM | STATE_CHECK_MODE | STATE_HOMING | STATE_HOLD | STATE_JOG | STATE_SAFETY_DOOR ))) {
        volts = 0.0;
      }
    }
    output_pwm_set_value(output_compute_pwm_value(volts));
  }
  // Set to report change immediately
  sys.report_ovr_counter = 0; 
}

// G-code parser entry-point for setting output PWM state. Forces a planner buffer sync and bails 
// if an abort or check-mode is active.
void output_pwm_sync(uint8_t state, float volts)
{
  if (sys.state == STATE_CHECK_MODE) { return; }
  protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
  output_pwm_set_state(state, volts);
}

#endif // USE_OUTPUT_PWM

