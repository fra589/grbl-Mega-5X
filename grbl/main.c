/*
  main.c - An embedded CNC Controller with rs274/ngc (g-code) support
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

#include "grbl.h"


// Declare system global variable structure
system_t sys;
int32_t sys_position[N_AXIS];      // Real-time machine (aka home) position vector in steps.
int32_t sys_probe_position[N_AXIS]; // Last probe position in machine coordinates and steps.
volatile uint8_t sys_probe_state;   // Probing state value.  Used to coordinate the probing cycle with stepper ISR.
volatile uint8_t sys_rt_exec_state;   // Global realtime executor bitflag variable for state management. See EXEC bitmasks.
volatile uint8_t sys_rt_exec_alarm;   // Global realtime executor bitflag variable for setting various alarms.
volatile uint8_t sys_rt_exec_motion_override; // Global realtime executor bitflag variable for motion-based overrides.
volatile uint8_t sys_rt_exec_accessory_override; // Global realtime executor bitflag variable for spindle/coolant overrides.
uint8_t axis_X_mask = 0; // Global mask for axis X bits
uint8_t axis_Y_mask = 0; // Global mask for axis Y bits
uint8_t axis_Z_mask = 0; // Global mask for axis Z bits
uint8_t axis_A_mask = 0; // Global mask for axis A bits
uint8_t axis_B_mask = 0; // Global mask for axis B bits
uint8_t axis_C_mask = 0; // Global mask for axis C bits
uint8_t axis_U_mask = 0; // Global mask for axis U bits
uint8_t axis_V_mask = 0; // Global mask for axis V bits
uint8_t axis_W_mask = 0; // Global mask for axis W bits
uint8_t axis_D_mask = 0; // Global mask for axis U bits
uint8_t axis_E_mask = 0; // Global mask for axis V bits
uint8_t axis_H_mask = 0; // Global mask for axis W bits
unsigned char axis_name[N_AXIS]; // Global table of axis names
bool is_clone[N_AXIS];  // Global table if axes are cloned

#ifdef DEBUG
  volatile uint8_t sys_rt_exec_debug;
#endif
#ifdef SORT_REPORT_BY_AXIS_NAME
  uint8_t n_axis_report;
#endif

int main(void)
{
  // Initialize system upon power-up.
  serial_init();   // Setup serial baud rate and interrupts
  settings_init(); // Load Grbl settings from EEPROM
  stepper_init();  // Configure stepper pins and interrupt timers
  system_init();   // Configure pinout pins and pin-change interrupt

  // Initialize axis mask bits (ability to axis renaming and cloning)
  // and global table of axis names.
  if (AXIS_1_NAME == 'X') {
    axis_X_mask |= (1<<AXIS_1);
    axis_name[0] = 'X';
  }
  if (AXIS_2_NAME == 'X') {
    axis_X_mask |= (1<<AXIS_2);
    axis_name[1] = 'X';
  }
  if (AXIS_3_NAME == 'X') {
    axis_X_mask |= (1<<AXIS_3);
    axis_name[2] = 'X';
  }
  #ifdef AXIS_4
    if (AXIS_4_NAME == 'X') {
      axis_X_mask |= (1<<AXIS_4);
    axis_name[3] = 'X';
    }
  #endif
  #ifdef AXIS_5
    if (AXIS_5_NAME == 'X') {
      axis_X_mask |= (1<<AXIS_5);
    axis_name[4] = 'X';
    }
  #endif
  #ifdef AXIS_6
    if (AXIS_6_NAME == 'X') {
      axis_X_mask |= (1<<AXIS_6);
    axis_name[5] = 'X';
    }
  #endif

  if (AXIS_1_NAME == 'Y') {
    axis_Y_mask |= (1<<AXIS_1);
    axis_name[0] = 'Y';
  }
  if (AXIS_2_NAME == 'Y') {
    axis_Y_mask |= (1<<AXIS_2);
    axis_name[1] = 'Y';
  }
  if (AXIS_3_NAME == 'Y') {
    axis_Y_mask |= (1<<AXIS_3);
    axis_name[2] = 'Y';
  }
  #ifdef AXIS_4
    if (AXIS_4_NAME == 'Y') {
      axis_Y_mask |= (1<<AXIS_4);
      axis_name[3] = 'Y';
    }
  #endif
  #ifdef AXIS_5
    if (AXIS_5_NAME == 'Y') {
      axis_Y_mask |= (1<<AXIS_5);
      axis_name[4] = 'Y';
    }
  #endif
  #ifdef AXIS_6
    if (AXIS_6_NAME == 'Y') {
      axis_Y_mask |= (1<<AXIS_6);
      axis_name[5] = 'Y';
    }
  #endif

  if (AXIS_1_NAME == 'Z') {
    axis_Z_mask |= (1<<AXIS_1);
    axis_name[0] = 'Z';
  }
  if (AXIS_2_NAME == 'Z') {
    axis_Z_mask |= (1<<AXIS_2);
    axis_name[1] = 'Z';
  }
  if (AXIS_3_NAME == 'Z') {
    axis_Z_mask |= (1<<AXIS_3);
    axis_name[2] = 'Z';
  }
  #ifdef AXIS_4
    if (AXIS_4_NAME == 'Z') {
      axis_Z_mask |= (1<<AXIS_4);
      axis_name[3] = 'Z';
    }
  #endif
  #ifdef AXIS_5
    if (AXIS_5_NAME == 'Z') {
      axis_Z_mask |= (1<<AXIS_5);
      axis_name[4] = 'Z';
    }
  #endif
  #ifdef AXIS_6
    if (AXIS_6_NAME == 'Z') {
      axis_Z_mask |= (1<<AXIS_6);
      axis_name[5] = 'Z';
    }
  #endif

  if (AXIS_1_NAME == 'A') {
    axis_A_mask |= (1<<AXIS_1);
    axis_name[0] = 'A';
  }
  if (AXIS_2_NAME == 'A') {
    axis_A_mask |= (1<<AXIS_2);
    axis_name[1] = 'A';
  }
  if (AXIS_3_NAME == 'A') {
    axis_A_mask |= (1<<AXIS_3);
    axis_name[2] = 'A';
  }
  #ifdef AXIS_4
    if (AXIS_4_NAME == 'A') {
      axis_A_mask |= (1<<AXIS_4);
      axis_name[3] = 'A';
    }
  #endif
  #ifdef AXIS_5
    if (AXIS_5_NAME == 'A') {
      axis_A_mask |= (1<<AXIS_5);
      axis_name[4] = 'A';
    }
  #endif
  #ifdef AXIS_6
    if (AXIS_6_NAME == 'A') {
      axis_A_mask |= (1<<AXIS_6);
      axis_name[5] = 'A';
    }
  #endif

  if (AXIS_1_NAME == 'B') {
    axis_B_mask |= (1<<AXIS_1);
    axis_name[0] = 'B';
  }
  if (AXIS_2_NAME == 'B') {
    axis_B_mask |= (1<<AXIS_2);
    axis_name[1] = 'B';
  }
  if (AXIS_3_NAME == 'B') {
    axis_B_mask |= (1<<AXIS_3);
    axis_name[2] = 'B';
  }
  #ifdef AXIS_4
    if (AXIS_4_NAME == 'B') {
      axis_B_mask |= (1<<AXIS_4);
      axis_name[3] = 'B';
    }
  #endif
  #ifdef AXIS_5
    if (AXIS_5_NAME == 'B') {
      axis_B_mask |= (1<<AXIS_5);
      axis_name[4] = 'B';
    }
  #endif
  #ifdef AXIS_6
    if (AXIS_6_NAME == 'B') {
      axis_B_mask |= (1<<AXIS_6);
      axis_name[5] = 'B';
    }
  #endif

  if (AXIS_1_NAME == 'C') {
    axis_C_mask |= (1<<AXIS_1);
    axis_name[0] = 'C';
  }
  if (AXIS_2_NAME == 'C') {
    axis_C_mask |= (1<<AXIS_2);
    axis_name[1] = 'C';
  }
  if (AXIS_3_NAME == 'C') {
    axis_C_mask |= (1<<AXIS_3);
    axis_name[2] = 'C';
  }
  #ifdef AXIS_4
    if (AXIS_4_NAME == 'C') {
      axis_C_mask |= (1<<AXIS_4);
      axis_name[3] = 'C';
    }
  #endif
  #ifdef AXIS_5
    if (AXIS_5_NAME == 'C') {
      axis_C_mask |= (1<<AXIS_5);
      axis_name[4] = 'C';
    }
  #endif
  #ifdef AXIS_6
    if (AXIS_6_NAME == 'C') {
      axis_C_mask |= (1<<AXIS_6);
      axis_name[5] = 'C';
    }
  #endif

  if (AXIS_1_NAME == 'U') {
    axis_U_mask |= (1<<AXIS_1);
    axis_name[0] = 'U';
  }
  if (AXIS_2_NAME == 'U') {
    axis_U_mask |= (1<<AXIS_2);
    axis_name[1] = 'U';
  }
  if (AXIS_3_NAME == 'U') {
    axis_U_mask |= (1<<AXIS_3);
    axis_name[2] = 'U';
  }
  #ifdef AXIS_4
    if (AXIS_4_NAME == 'U') {
      axis_U_mask |= (1<<AXIS_4);
      axis_name[3] = 'U';
    }
  #endif
  #ifdef AXIS_5
    if (AXIS_5_NAME == 'U') {
      axis_U_mask |= (1<<AXIS_5);
      axis_name[4] = 'U';
    }
  #endif
  #ifdef AXIS_6
    if (AXIS_6_NAME == 'U') {
      axis_U_mask |= (1<<AXIS_6);
      axis_name[5] = 'U';
    }
  #endif

  if (AXIS_1_NAME == 'V') {
    axis_V_mask |= (1<<AXIS_1);
    axis_name[0] = 'V';
  }
  if (AXIS_2_NAME == 'V') {
    axis_V_mask |= (1<<AXIS_2);
    axis_name[1] = 'V';
  }
  if (AXIS_3_NAME == 'V') {
    axis_V_mask |= (1<<AXIS_3);
    axis_name[2] = 'V';
  }
  #ifdef AXIS_4
    if (AXIS_4_NAME == 'V') {
      axis_V_mask |= (1<<AXIS_4);
      axis_name[3] = 'V';
    }
  #endif
  #ifdef AXIS_5
    if (AXIS_5_NAME == 'V') {
      axis_V_mask |= (1<<AXIS_5);
      axis_name[4] = 'V';
    }
  #endif
  #ifdef AXIS_6
    if (AXIS_6_NAME == 'V') {
      axis_V_mask |= (1<<AXIS_6);
      axis_name[5] = 'V';
    }
  #endif

  if (AXIS_1_NAME == 'W') {
    axis_W_mask |= (1<<AXIS_1);
    axis_name[0] = 'W';
  }
  if (AXIS_2_NAME == 'W') {
    axis_W_mask |= (1<<AXIS_2);
    axis_name[1] = 'W';
  }
  if (AXIS_3_NAME == 'W') {
    axis_W_mask |= (1<<AXIS_3);
    axis_name[2] = 'W';
  }
  #ifdef AXIS_4
    if (AXIS_4_NAME == 'W') {
      axis_W_mask |= (1<<AXIS_4);
      axis_name[3] = 'W';
    }
  #endif
  #ifdef AXIS_5
    if (AXIS_5_NAME == 'W') {
      axis_W_mask |= (1<<AXIS_5);
      axis_name[4] = 'W';
    }
  #endif
  #ifdef AXIS_6
    if (AXIS_6_NAME == 'W') {
      axis_W_mask |= (1<<AXIS_6);
      axis_name[5] = 'W';
    }
  #endif
  
  if (AXIS_1_NAME == 'D') {
    axis_D_mask |= (1<<AXIS_1);
    axis_name[0] = 'D';
  }
  if (AXIS_2_NAME == 'D') {
    axis_D_mask |= (1<<AXIS_2);
    axis_name[1] = 'D';
  }
  if (AXIS_3_NAME == 'D') {
    axis_D_mask |= (1<<AXIS_3);
    axis_name[2] = 'D';
  }
  #ifdef AXIS_4
    if (AXIS_4_NAME == 'D') {
      axis_D_mask |= (1<<AXIS_4);
      axis_name[3] = 'D';
    }
  #endif
  #ifdef AXIS_5
    if (AXIS_5_NAME == 'D') {
      axis_D_mask |= (1<<AXIS_5);
      axis_name[4] = 'D';
    }
  #endif
  #ifdef AXIS_6
    if (AXIS_6_NAME == 'D') {
      axis_D_mask |= (1<<AXIS_6);
      axis_name[5] = 'D';
    }
  #endif
  
  if (AXIS_1_NAME == 'E') {
    axis_E_mask |= (1<<AXIS_1);
    axis_name[0] = 'E';
  }
  if (AXIS_2_NAME == 'E') {
    axis_E_mask |= (1<<AXIS_2);
    axis_name[1] = 'E';
  }
  if (AXIS_3_NAME == 'E') {
    axis_E_mask |= (1<<AXIS_3);
    axis_name[2] = 'E';
  }
  #ifdef AXIS_4
    if (AXIS_4_NAME == 'E') {
      axis_E_mask |= (1<<AXIS_4);
      axis_name[3] = 'E';
    }
  #endif
  #ifdef AXIS_5
    if (AXIS_5_NAME == 'E') {
      axis_E_mask |= (1<<AXIS_5);
      axis_name[4] = 'E';
    }
  #endif
  #ifdef AXIS_6
    if (AXIS_6_NAME == 'E') {
      axis_E_mask |= (1<<AXIS_6);
      axis_name[5] = 'E';
    }
  #endif

  if (AXIS_1_NAME == 'H') {
    axis_H_mask |= (1<<AXIS_1);
    axis_name[0] = 'H';
  }
  if (AXIS_2_NAME == 'H') {
    axis_H_mask |= (1<<AXIS_2);
    axis_name[1] = 'H';
  }
  if (AXIS_3_NAME == 'H') {
    axis_H_mask |= (1<<AXIS_3);
    axis_name[2] = 'H';
  }
  #ifdef AXIS_4
    if (AXIS_4_NAME == 'H') {
      axis_H_mask |= (1<<AXIS_4);
      axis_name[3] = 'H';
    }
  #endif
  #ifdef AXIS_5
    if (AXIS_5_NAME == 'H') {
      axis_H_mask |= (1<<AXIS_5);
      axis_name[4] = 'H';
    }
  #endif
  #ifdef AXIS_6
    if (AXIS_6_NAME == 'H') {
      axis_H_mask |= (1<<AXIS_6);
      axis_name[5] = 'H';
    }
  #endif
  
  for (int i=0; i<N_AXIS; i++) {
    for (int j=i+1; j<N_AXIS; j++) {
      if (axis_name[i] == axis_name[j]) {
        is_clone[j] = true;
      }
    }
  }

  #ifdef SORT_REPORT_BY_AXIS_NAME
    #ifdef REPORT_VALUE_FOR_AXIS_NAME_ONCE
      // Calcule le nombre de nom d'axes différents à utiliser dans report.c
      n_axis_report = 1; // Au moins le nom du premier axe
      if (AXIS_2_NAME != AXIS_1_NAME) {
        n_axis_report++;
      }
      if ((AXIS_3_NAME != AXIS_2_NAME) && (AXIS_3_NAME != AXIS_1_NAME)) {
        n_axis_report++;
      }
      #if N_AXIS > 3
        if ((AXIS_4_NAME != AXIS_3_NAME) && (AXIS_4_NAME != AXIS_2_NAME) && (AXIS_4_NAME != AXIS_1_NAME)) {
          n_axis_report++;
        }
      #endif
      #if N_AXIS > 4
        if ((AXIS_5_NAME != AXIS_4_NAME) && (AXIS_5_NAME != AXIS_3_NAME) && (AXIS_5_NAME != AXIS_2_NAME) && (AXIS_5_NAME != AXIS_1_NAME)) {
          n_axis_report++;
        }
      #endif
      #if N_AXIS > 5
        if ((AXIS_6_NAME != AXIS_5_NAME) && (AXIS_6_NAME != AXIS_4_NAME) && (AXIS_6_NAME != AXIS_3_NAME) && (AXIS_6_NAME != AXIS_2_NAME) && (AXIS_6_NAME != AXIS_1_NAME)) {
          n_axis_report++;
        }
      #endif
    #else
      n_axis_report = N_AXIS;
    #endif
  #endif

  memset(sys_position,0,sizeof(sys_position)); // Clear machine position.
  sei(); // Enable interrupts

  // Initialize system state.
  #ifdef FORCE_INITIALIZATION_ALARM
    // Force Grbl into an ALARM state upon a power-cycle or hard reset.
    sys.state = STATE_ALARM;
  #else
    sys.state = STATE_IDLE;
  #endif

  // Check for power-up and set system alarm if homing is enabled to force homing cycle
  // by setting Grbl's alarm state. Alarm locks out all g-code commands, including the
  // startup scripts, but allows access to settings and internal commands. Only a homing
  // cycle '$H' or kill alarm locks '$X' will disable the alarm.
  // NOTE: The startup script will run after successful completion of the homing cycle, but
  // not after disabling the alarm locks. Prevents motion startup blocks from crashing into
  // things uncontrollably. Very bad.
  #ifdef HOMING_INIT_LOCK
    if (bit_istrue(settings.flags,BITFLAG_HOMING_ENABLE)) { sys.state = STATE_ALARM; }
  #endif

  // Grbl initialization loop upon power-up or a system abort. For the latter, all processes
  // will return to this loop to be cleanly re-initialized.
  for(;;) {

    // Reset system variables.
    uint8_t prior_state = sys.state;
    memset(&sys, 0, sizeof(system_t)); // Clear system struct variable.
    sys.state = prior_state;
    sys.f_override = DEFAULT_FEED_OVERRIDE;  // Set to 100%
    sys.r_override = DEFAULT_RAPID_OVERRIDE; // Set to 100%
    sys.spindle_speed_ovr = DEFAULT_SPINDLE_SPEED_OVERRIDE; // Set to 100%
    memset(sys_probe_position,0,sizeof(sys_probe_position)); // Clear probe position.
    sys_probe_state = 0;
    sys_rt_exec_state = 0;
    sys_rt_exec_alarm = 0;
    sys_rt_exec_motion_override = 0;
    sys_rt_exec_accessory_override = 0;

    // Reset Grbl primary systems.
    serial_reset_read_buffer(); // Clear serial read buffer
    gc_init(); // Set g-code parser to default state
    spindle_init();
    #ifdef USE_OUTPUT_PWM
      output_pwm_init();
    #endif
    coolant_init();
    digital_init();
    limits_init();
    probe_init();
    sleep_init();
    plan_reset(); // Clear block buffer and planner variables
    st_reset(); // Clear stepper subsystem variables.

    // Sync cleared gcode and planner positions to current system position.
    plan_sync_position();
    gc_sync_position();

    // Print welcome message. Indicates an initialization has occured at power-up or with a reset.
    report_init_message();

    // Start Grbl main loop. Processes program inputs and executes them.
    protocol_main_loop();

  }
  return 0;   /* Never reached */
}
