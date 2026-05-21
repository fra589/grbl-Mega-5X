/*
  settings.c - eeprom configuration handling
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

settings_t settings;

const __flash settings_t defaults = {
    .pulse_microseconds = DEFAULT_STEP_PULSE_MICROSECONDS,
    .stepper_idle_lock_time = DEFAULT_STEPPER_IDLE_LOCK_TIME,
    .step_invert_mask = DEFAULT_STEPPING_INVERT_MASK,
    .dir_invert_mask = DEFAULT_DIRECTION_INVERT_MASK,
    .status_report_mask = DEFAULT_STATUS_REPORT_MASK,
    .junction_deviation = DEFAULT_JUNCTION_DEVIATION,
    .arc_tolerance = DEFAULT_ARC_TOLERANCE,
    .rpm_max = DEFAULT_SPINDLE_RPM_MAX,
    .rpm_min = DEFAULT_SPINDLE_RPM_MIN,
    #ifdef SEPARATE_SPINDLE_LASER_PIN
      .laser_max = DEFAULT_LASER_MAX,
      .laser_min = DEFAULT_LASER_MIN,
    #endif
    #ifdef USE_OUTPUT_PWM
      .volts_max = DEFAULT_OUTPUT_MAX,
      .volts_min = DEFAULT_OUTPUT_MIN,
    #endif
    .homing_dir_mask = DEFAULT_HOMING_DIR_MASK,
    .homing_feed_rate = DEFAULT_HOMING_FEED_RATE,
    .homing_seek_rate = DEFAULT_HOMING_SEEK_RATE,
    .homing_debounce_delay = DEFAULT_HOMING_DEBOUNCE_DELAY,
    .homing_pulloff = DEFAULT_HOMING_PULLOFF,
    .flags = (DEFAULT_REPORT_INCHES << BIT_REPORT_INCHES) |
             (DEFAULT_LASER_MODE << BIT_LASER_MODE) |
             (DEFAULT_INVERT_ST_ENABLE << BIT_INVERT_ST_ENABLE) |
             (DEFAULT_HARD_LIMIT_ENABLE << BIT_HARD_LIMIT_ENABLE) |
             (DEFAULT_HOMING_ENABLE << BIT_HOMING_ENABLE) |
             (DEFAULT_SOFT_LIMIT_ENABLE << BIT_SOFT_LIMIT_ENABLE) |
             (DEFAULT_INVERT_LIMIT_PINS << BIT_INVERT_LIMIT_PINS) |
             (DEFAULT_INVERT_PROBE_PIN << BIT_INVERT_PROBE_PIN),
    .steps_per_mm[AXIS_1] = DEFAULT_AXIS1_STEPS_PER_UNIT,
    .steps_per_mm[AXIS_2] = DEFAULT_AXIS2_STEPS_PER_UNIT,
    .steps_per_mm[AXIS_3] = DEFAULT_AXIS3_STEPS_PER_UNIT,
    .max_rate[AXIS_1] = DEFAULT_AXIS1_MAX_RATE,
    .max_rate[AXIS_2] = DEFAULT_AXIS2_MAX_RATE,
    .max_rate[AXIS_3] = DEFAULT_AXIS3_MAX_RATE,
    .acceleration[AXIS_1] = DEFAULT_AXIS1_ACCELERATION,
    .acceleration[AXIS_2] = DEFAULT_AXIS2_ACCELERATION,
    .acceleration[AXIS_3] = DEFAULT_AXIS3_ACCELERATION,
    .max_travel[AXIS_1] = (-DEFAULT_AXIS1_MAX_TRAVEL),
    .max_travel[AXIS_2] = (-DEFAULT_AXIS2_MAX_TRAVEL),
    .max_travel[AXIS_3] = (-DEFAULT_AXIS3_MAX_TRAVEL),
#if N_AXIS > 3
    .steps_per_mm[AXIS_4] = DEFAULT_AXIS4_STEPS_PER_UNIT,
    .max_rate[AXIS_4] = DEFAULT_AXIS4_MAX_RATE,
    .acceleration[AXIS_4] = DEFAULT_AXIS4_ACCELERATION,
    .max_travel[AXIS_4] = (-DEFAULT_AXIS4_MAX_TRAVEL),
#endif
#if N_AXIS > 4
    .steps_per_mm[AXIS_5] = DEFAULT_AXIS5_STEPS_PER_UNIT,
    .max_rate[AXIS_5] = DEFAULT_AXIS5_MAX_RATE,
    .acceleration[AXIS_5] = DEFAULT_AXIS5_ACCELERATION,
    .max_travel[AXIS_5] = (-DEFAULT_AXIS5_MAX_TRAVEL),
#endif
#if N_AXIS > 5
    .steps_per_mm[AXIS_6] = DEFAULT_AXIS6_STEPS_PER_UNIT,
    .max_rate[AXIS_6] = DEFAULT_AXIS6_MAX_RATE,
    .acceleration[AXIS_6] = DEFAULT_AXIS6_ACCELERATION,
    .max_travel[AXIS_6] = (-DEFAULT_AXIS6_MAX_TRAVEL),
#endif
};

// Method to store startup lines into EEPROM
void settings_store_startup_line(uint8_t n, char *line)
{
  #ifdef FORCE_BUFFER_SYNC_DURING_EEPROM_WRITE
    protocol_buffer_synchronize(); // A startup line may contain a motion and be executing.
  #endif
  uint32_t addr = n*(LINE_BUFFER_SIZE+1)+EEPROM_ADDR_STARTUP_BLOCK;
  memcpy_to_eeprom_with_checksum(addr,(char*)line, LINE_BUFFER_SIZE);
}


// Method to store build info into EEPROM
// NOTE: This function can only be called in IDLE state.
void settings_store_build_info(char *line)
{
  // Build info can only be stored when state is IDLE.
  memcpy_to_eeprom_with_checksum(EEPROM_ADDR_BUILD_INFO,(char*)line, LINE_BUFFER_SIZE);
}


// Method to store coord data parameters into EEPROM
void settings_write_coord_data(uint8_t coord_select, float *coord_data)
{
  #ifdef FORCE_BUFFER_SYNC_DURING_EEPROM_WRITE
    protocol_buffer_synchronize();
  #endif
  uint32_t addr = coord_select*(sizeof(float)*N_AXIS+1) + EEPROM_ADDR_PARAMETERS;
  memcpy_to_eeprom_with_checksum(addr,(char*)coord_data, sizeof(float)*N_AXIS);
}


// Method to store Grbl global settings struct and version number into EEPROM
// NOTE: This function can only be called in IDLE state.
void write_global_settings()
{
  eeprom_put_char(0, SETTINGS_VERSION);
  memcpy_to_eeprom_with_checksum(EEPROM_ADDR_GLOBAL, (char*)&settings, sizeof(settings_t));
}


// Method to restore EEPROM-saved Grbl global settings back to defaults.
void settings_restore(uint8_t restore_flag) {
  if (restore_flag & SETTINGS_RESTORE_DEFAULTS) {
    settings = defaults;
    write_global_settings();
  }

  if (restore_flag & SETTINGS_RESTORE_PARAMETERS) {
    uint8_t idx;
    float coord_data[N_AXIS];
    memset(&coord_data, 0, sizeof(coord_data));
    for (idx=0; idx <= SETTING_INDEX_NCOORD; idx++) { settings_write_coord_data(idx, coord_data); }
  }

  if (restore_flag & SETTINGS_RESTORE_STARTUP_LINES) {
    #if N_STARTUP_LINE > 0
      eeprom_put_char(EEPROM_ADDR_STARTUP_BLOCK, 0);
      eeprom_put_char(EEPROM_ADDR_STARTUP_BLOCK+1, 0); // Checksum
    #endif
    #if N_STARTUP_LINE > 1
      eeprom_put_char(EEPROM_ADDR_STARTUP_BLOCK+(LINE_BUFFER_SIZE+1), 0);
      eeprom_put_char(EEPROM_ADDR_STARTUP_BLOCK+(LINE_BUFFER_SIZE+2), 0); // Checksum
    #endif
  }

  if (restore_flag & SETTINGS_RESTORE_BUILD_INFO) {
    eeprom_put_char(EEPROM_ADDR_BUILD_INFO , 0);
    eeprom_put_char(EEPROM_ADDR_BUILD_INFO+1 , 0); // Checksum
  }
}


// Reads startup line from EEPROM. Updated pointed line string data.
uint8_t settings_read_startup_line(uint8_t n, char *line)
{
  uint32_t addr = n*(LINE_BUFFER_SIZE+1)+EEPROM_ADDR_STARTUP_BLOCK;
  if (!(memcpy_from_eeprom_with_checksum((char*)line, addr, LINE_BUFFER_SIZE))) {
    // Reset line with default value
    line[0] = 0; // Empty line
    settings_store_startup_line(n, line);
    return(false);
  }
  return(true);
}


// Reads startup line from EEPROM. Updated pointed line string data.
uint8_t settings_read_build_info(char *line)
{
  if (!(memcpy_from_eeprom_with_checksum((char*)line, EEPROM_ADDR_BUILD_INFO, LINE_BUFFER_SIZE))) {
    // Reset line with default value
    line[0] = 0; // Empty line
    settings_store_build_info(line);
    return(false);
  }
  return(true);
}


// Read selected coordinate data from EEPROM. Updates pointed coord_data value.
uint8_t settings_read_coord_data(uint8_t coord_select, float *coord_data)
{
  uint32_t addr = coord_select*(sizeof(float)*N_AXIS+1) + EEPROM_ADDR_PARAMETERS;
  if (!(memcpy_from_eeprom_with_checksum((char*)coord_data, addr, sizeof(float)*N_AXIS))) {
    // Reset with default zero vector
    clear_vector_float(coord_data);
    settings_write_coord_data(coord_select,coord_data);
    return(false);
  }
  return(true);
}


// Reads Grbl global settings struct from EEPROM.
uint8_t read_global_settings() {
  // Check version-byte of eeprom
  uint8_t version = eeprom_get_char(0);
  if (version == SETTINGS_VERSION) {
    // Read settings-record and check checksum
    if (!(memcpy_from_eeprom_with_checksum((char*)&settings, EEPROM_ADDR_GLOBAL, sizeof(settings_t)))) {
      return(false);
    }
  } else {
    return(false);
  }
  return(true);
}


// A helper method to set settings from command line
uint8_t settings_store_global_setting(uint8_t parameter, float value) {
  if (value < 0.0) { return(STATUS_NEGATIVE_VALUE); }
  if (parameter >= AXIS_SETTINGS_START_VAL) {
    // Store axis configuration. Axis numbering sequence set by AXIS_SETTING defines.
    // NOTE: Ensure the setting index corresponds to the report.c settings printout.
    parameter -= AXIS_SETTINGS_START_VAL;
    uint8_t set_idx = 0;
    while (set_idx < AXIS_N_SETTINGS) {
      if (parameter < N_AXIS) {
        // Valid axis setting found.
        switch (set_idx) {
          case 0:
            #ifdef MAX_STEP_RATE_HZ
              if (value*settings.max_rate[parameter] > (MAX_STEP_RATE_HZ*60.0)) { return(STATUS_MAX_STEP_RATE_EXCEEDED); }
            #endif
            settings.steps_per_mm[parameter] = value;
            break;
          case 1:
            #ifdef MAX_STEP_RATE_HZ
              if (value*settings.steps_per_mm[parameter] > (MAX_STEP_RATE_HZ*60.0)) {  return(STATUS_MAX_STEP_RATE_EXCEEDED); }
            #endif
            settings.max_rate[parameter] = value;
            break;
          case 2: settings.acceleration[parameter] = value*60*60; break; // Convert to mm/min^2 for grbl internal use.
          case 3: settings.max_travel[parameter] = -value; break;  // Store as negative for grbl internal use.
        }
        break; // Exit while-loop after setting has been configured and proceed to the EEPROM write call.
      } else {
        set_idx++;
        // If axis index greater than N_AXIS or setting index greater than number of axis settings, error out.
        if ((parameter < AXIS_SETTINGS_INCREMENT) || (set_idx == AXIS_N_SETTINGS)) { return(STATUS_INVALID_STATEMENT); }
        parameter -= AXIS_SETTINGS_INCREMENT;
      }
    }
  } else {
    // Store non-axis Grbl settings
    uint8_t int_value = trunc(value);
    switch(parameter) {
      case 0:
        if (int_value < 3) { return(STATUS_SETTING_STEP_PULSE_MIN); }
        settings.pulse_microseconds = int_value; break;
      case 1: settings.stepper_idle_lock_time = int_value; break;
      case 2:
        settings.step_invert_mask = int_value;
        st_generate_step_dir_invert_masks(); // Regenerate step and direction port invert masks.
        break;
      case 3:
        settings.dir_invert_mask = int_value;
        st_generate_step_dir_invert_masks(); // Regenerate step and direction port invert masks.
        break;
      case 4: // Reset to ensure change. Immediate re-init may cause problems.
        if (int_value) { settings.flags |= BITFLAG_INVERT_ST_ENABLE; }
        else { settings.flags &= ~BITFLAG_INVERT_ST_ENABLE; }
        break;
      case 5: // Reset to ensure change. Immediate re-init may cause problems.
        if (int_value) { settings.flags |= BITFLAG_INVERT_LIMIT_PINS; }
        else { settings.flags &= ~BITFLAG_INVERT_LIMIT_PINS; }
        break;
      case 6: // Reset to ensure change. Immediate re-init may cause problems.
        if (int_value) { settings.flags |= BITFLAG_INVERT_PROBE_PIN; }
        else { settings.flags &= ~BITFLAG_INVERT_PROBE_PIN; }
        probe_configure_invert_mask(false);
        break;
      case 10: settings.status_report_mask = int_value; break;
      case 11: settings.junction_deviation = value; break;
      case 12: settings.arc_tolerance = value; break;
      case 13:
        if (int_value) { settings.flags |= BITFLAG_REPORT_INCHES; }
        else { settings.flags &= ~BITFLAG_REPORT_INCHES; }
        system_flag_wco_change(); // Make sure WCO is immediately updated.
        break;
      case 20:
        if (int_value) {
          if (bit_isfalse(settings.flags, BITFLAG_HOMING_ENABLE)) { return(STATUS_SOFT_LIMIT_ERROR); }
          settings.flags |= BITFLAG_SOFT_LIMIT_ENABLE;
        } else { settings.flags &= ~BITFLAG_SOFT_LIMIT_ENABLE; }
        break;
      case 21:
        if (int_value) { settings.flags |= BITFLAG_HARD_LIMIT_ENABLE; }
        else { settings.flags &= ~BITFLAG_HARD_LIMIT_ENABLE; }
        limits_init(); // Re-init to immediately change. NOTE: Nice to have but could be problematic later.
        break;
      case 22:
        if (int_value) { settings.flags |= BITFLAG_HOMING_ENABLE; }
        else {
          settings.flags &= ~BITFLAG_HOMING_ENABLE;
          settings.flags &= ~BITFLAG_SOFT_LIMIT_ENABLE; // Force disable soft-limits.
        }
        break;
      case 23: settings.homing_dir_mask = int_value; break;
      case 24: settings.homing_feed_rate = value; break;
      case 25: settings.homing_seek_rate = value; break;
      case 26: settings.homing_debounce_delay = int_value; break;
      case 27: settings.homing_pulloff = value; break;
      case 30: settings.rpm_max = value; spindle_init(); break; // Re-initialize spindle rpm calibration
      case 31: settings.rpm_min = value; spindle_init(); break; // Re-initialize spindle rpm calibration
      case 32:
        if (int_value) { settings.flags |= BITFLAG_LASER_MODE; }
        else { settings.flags &= ~BITFLAG_LASER_MODE; }
        #ifdef SEPARATE_SPINDLE_LASER_PIN
          spindle_init(); // Re-initialize spindle / laser calibration
        #endif
        break;
      #ifdef SEPARATE_SPINDLE_LASER_PIN
        case 33: settings.laser_max = value; spindle_init(); break; // Re-initialize laser calibration
        case 34: settings.laser_min = value; spindle_init(); break; // Re-initialize laser calibration
      #endif
      #ifdef USE_OUTPUT_PWM
        case 35: settings.volts_max = value; output_pwm_init(); break;
        case 36: settings.volts_min = value; output_pwm_init(); break;
      #endif
      default:
        return(STATUS_INVALID_STATEMENT);
    }
  }
  write_global_settings();
  return(STATUS_OK);
}


// Initialize the config subsystem
void settings_init() {
  if(!read_global_settings()) {
    report_status_message(STATUS_SETTING_READ_FAIL);
    settings_restore(SETTINGS_RESTORE_ALL); // Force restore all EEPROM data.
    report_grbl_settings();
    do {} while (is_report_grbl_settings_running()); // Wait for report settings complete
  }
  #ifdef DEBUG
  else
  {
    report_debug_string("settings_init() Ok.");
  }
  #endif
}


// Returns step pin mask according to Grbl internal axis indexing.
uint8_t get_step_pin_mask(uint8_t axis_idx)
{
  if ( axis_idx == AXIS_1 ) { return((1<<STEP_BIT(AXIS_1))); }
  if ( axis_idx == AXIS_2 ) { return((1<<STEP_BIT(AXIS_2))); }
  #if N_AXIS > 3
    if ( axis_idx == AXIS_4 ) { return((1<<STEP_BIT(AXIS_4))); }
  #endif
  #if N_AXIS > 4
    if ( axis_idx == AXIS_5 ) { return((1<<STEP_BIT(AXIS_5))); }
  #endif
  #if N_AXIS > 5
    if ( axis_idx == AXIS_6 ) { return((1<<STEP_BIT(AXIS_6))); }
  #endif
  return((1<<STEP_BIT(AXIS_3)));
}


// Returns direction pin mask according to Grbl internal axis indexing.
uint8_t get_direction_pin_mask(uint8_t axis_idx)
{
  if ( axis_idx == AXIS_1 ) { return((1<<DIRECTION_BIT(AXIS_1))); }
  if ( axis_idx == AXIS_2 ) { return((1<<DIRECTION_BIT(AXIS_2))); }
  #if N_AXIS > 3
    if ( axis_idx == AXIS_4 ) { return((1<<DIRECTION_BIT(AXIS_4))); }
  #endif
  #if N_AXIS > 4
    if ( axis_idx == AXIS_5 ) { return((1<<DIRECTION_BIT(AXIS_5))); }
  #endif
  #if N_AXIS > 5
    if ( axis_idx == AXIS_6 ) { return((1<<DIRECTION_BIT(AXIS_6))); }
  #endif
  return((1<<DIRECTION_BIT(AXIS_3)));
}


// Returns limit pin mask according to Grbl internal axis indexing.

uint8_t get_min_limit_pin_mask(uint8_t axis_idx)
{
  if ( axis_idx == AXIS_1 ) { return((1<<MIN_LIMIT_BIT(AXIS_1))); }
  if ( axis_idx == AXIS_2 ) { return((1<<MIN_LIMIT_BIT(AXIS_2))); }
  #if N_AXIS > 3
    if ( axis_idx == AXIS_4 ) { return((1<<MIN_LIMIT_BIT(AXIS_4))); }
  #endif
  #if N_AXIS > 4
    if ( axis_idx == AXIS_5 ) { return((1<<MIN_LIMIT_BIT(AXIS_5))); }
  #endif
  #if N_AXIS > 5
    if ( axis_idx == AXIS_6 ) { return((1<<MIN_LIMIT_BIT(AXIS_6))); }
  #endif
  return((1<<MIN_LIMIT_BIT(AXIS_3)));
}

uint8_t get_max_limit_pin_mask(uint8_t axis_idx)
{
  if ( axis_idx == AXIS_1 ) { return((1<<MAX_LIMIT_BIT(AXIS_1))); }
  if ( axis_idx == AXIS_2 ) { return((1<<MAX_LIMIT_BIT(AXIS_2))); }
  #if N_AXIS > 3
    if ( axis_idx == AXIS_4 ) { return((1<<MAX_LIMIT_BIT(AXIS_4))); }
  #endif
  #if N_AXIS > 4
    if ( axis_idx == AXIS_5 ) { return((1<<MAX_LIMIT_BIT(AXIS_5))); }
  #endif
  #if N_AXIS > 5
    if ( axis_idx == AXIS_6 ) { return((1<<MAX_LIMIT_BIT(AXIS_6))); }
  #endif
  return((1<<MAX_LIMIT_BIT(AXIS_3)));
}

