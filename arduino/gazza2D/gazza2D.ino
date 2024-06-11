/**************************************************************************************
 * This is the final version of gazza. refactored to isolate the sensors
 * initialization and reading functions in semsors.h file. The sensors() function call
 * tries to initialize separatetly mag and the two accel sensors.
 * 
 * Sensors initialization parameters are defined in the respective header files in the sensors
 * subdirectory
 * 
 * The sensors supported are:
 * - MAGNETOMETERS: ST LIS3MDL, PNI RM3100 (only chinese breakout board, not reccommended, get a PNI BB instead and use it's i2c interface)
 * - ACCELEROMETERS: ST LSM6DSV for Altitude detection. For untilt either NXP MMA8451 or InvenSense MPU6050 (but this is very noisy)
 * 
 * Serial1 IO is to/from the Raspberry
 * Serial  IO is to/from the optional usb cable, typically used for testing purposes
 * 
 * Created by Massimo Tasso, January, 1, 2023 except where otherwise stated.
 * Released under GPLv3 License - see LICENSE file for details.
 **************************************************************************************/

#include "sensors.h"
#include "joystick.h"
#include "compass.h"
#include "m_calibration.h"
#include "m_compensation.h"
#include "a_calibration.h"
#include "defines.h"

unsigned long magLoopIntervalMs;
unsigned long untiltLoopIntervalMs;
unsigned long accelLoopIntervalMs;
unsigned long magCalLoopIntervalMs;
unsigned long magCompLoopIntervalMS = 100;
unsigned long headingLoopIntervalMs;
const unsigned long outputLoopIntervalMs = 100;  // sensors sent to Raspberry at 10 Hz - sounds enough
const unsigned long devoutputLoopIntervalMs = 200;  // outputs mag and untiltacc raw data - temporary, can be removed after development tests

unsigned long last_mag_time = millis();
unsigned long last_untilt_time = millis();
unsigned long last_accel_time = millis();
unsigned long last_mag_cal_time = millis();
unsigned long last_compensation_time = millis();
unsigned long last_heading_time = millis();
unsigned long last_print_time = millis();

bool m_calib_on = false;        // true if calibration data collection is in progress
bool m_calib_complete = false;

bool compensation_on = false;        // true if compensation data collection is in progress
bool compensation_complete = false;
float compensation_reference = 180.0;

uint8_t debug;

// Buffer for holding general text output
#define MAX_LEN_OUT_BUF 130
char output_str[MAX_LEN_OUT_BUF];

/*
 * Latest orientation data
 * magnetometer values are kalman smoothed, calibrated and untilted
 * accelerometer values are kalman smoothed
 */
float azimuth = 0;
// float azimuth_ALT = 0;
float altitude = 0;
float acc[3];
float mag[3];
float unt_acc[3];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);   // Serial: Console
  Serial1.begin(115200);  // Serial1: raspberry
  while(!Serial1) {
    delay(1);
  }

  initJoystick();
  Serial.println("STARTING");
  /* Initialise the sensors */
  while(!(sensors())) {
    Serial.println("Sensor error");
    Serial1.println("Sensor error");
    delay(1000);
  }

  // need to initialize the values here because they depend on sensors() result
  magLoopIntervalMs = 1000 / MAG_DATARATE;
  untiltLoopIntervalMs = 1000 / UNTILTER_DATARATE;  // 2;  // 500 Hz;
  accelLoopIntervalMs = 1000 / ACCEL_DATARATE;
  headingLoopIntervalMs = max(magLoopIntervalMs, accelLoopIntervalMs);  // used only if heading is kalman smoothed -- to be decided
  
  Serial.print("MAG Datarate: ");Serial.println(MAG_DATARATE);
  Serial.print("ACC Datarate: ");Serial.println(ACCEL_DATARATE);
  Serial.print("UNT Datarate: ");Serial.println(UNTILTER_DATARATE);
  Serial.print("MAG interval ms: ");Serial.println(magLoopIntervalMs);
  Serial.print("UNTILT interval ms: ");Serial.println(untiltLoopIntervalMs);
  Serial.print("ACC interval ms: ");Serial.println(accelLoopIntervalMs);
}

void checkSerial();
void checkSerial1();

/**
 * main cycle.
 * actions to perform, with different intervals
 * 
 * - accelererometer readings - accelLoopIntervalMS
 * - untilt accelerometer readings - 500 Hz, 2ms
 * - magnetometer readings - magLoopIntervalMS
 * - samples acquisition for magnetometer calibration - interval calculated basing on rotation time and number of samples
 * - samples acquisition for magnetometer compensation (mag data and accel data)
 * - ALT-AZ computation
 * - ALT-AZ sendout to raspberry (and serial console)
 **/
void loop() {
    unsigned long timestamp = millis();
    // ALTITUDE ACC
    if ((timestamp - last_accel_time) >= accelLoopIntervalMs) {                       // time to read accelerometer
        last_accel_time = timestamp;
        accel_readings(acc, debug);
        a_calibrate(acc);                                                             // call a_calibrate even if calibration is not done, won't harm
    }

    // UNTILT ACC
    if ((timestamp - last_untilt_time) >= untiltLoopIntervalMs) {                     // time to read untilt accelerometer
        last_untilt_time = timestamp;
//        Serial.println("LOOP Reading untilter");
        get_untilt_raw(unt_acc, debug);
//        Serial.print("LOOP untilter READ ");Serial.print(unt_acc[0]);Serial.print(" ");Serial.print(unt_acc[1]);Serial.print(" ");Serial.println(unt_acc[2]);
    }

    // MAGNETOMETER
    if ((timestamp - last_mag_time) >= magLoopIntervalMs) {                           // time to read magnetometer
        last_mag_time = timestamp;
        mag_readings(mag, debug);
        if (m_calib_on && (timestamp - last_mag_cal_time) >= magCalLoopIntervalMs) {        // time to read mag calibration sample
            last_mag_cal_time = timestamp;
    
            float unt_acc[3];
            get_untilt_raw(unt_acc, debug);
    
            int to_go = add_sample(mag, unt_acc, debug);
            if(!to_go) {
                m_calib_on = false;
                m_calib_complete = true;
                Serial.print("AZ_CORR ");Serial.println(AZ_CORR*RadToDeg);
                Serial.println("M_CAL, points collected");
                Serial1.println("M_CAL, points collected");  // tell Raspberry that magnetometer calibration is completed
            } else {
                Serial.print(millis());Serial.print(" \tM Calibration togo ");Serial.println(to_go);
            }
        }
        m_calibrate(mag, unt_acc, debug);                    // can call m_calibrate even if calibration is not completed, won't harm
    }

    // MAGNETOMETER COMPENSATION
    if (compensation_on && (timestamp - last_compensation_time) >= magCompLoopIntervalMS) {  // time to read mag compensation sample
        last_compensation_time = timestamp;
        int to_go = add_compensation_sample(compensation_reference, altitude, azimuth, debug);
        if(!to_go) {
          compensation_on = false;
          compensation_complete = true;
          Serial.println("M_COMP, compensation map completed");
          Serial1.println("M_COMP, compensation map completed");  // tell Raspberry that magnetometer compensation is completed
        } else {
          Serial.print(millis());Serial.print(" \tM Compensation togo ");Serial.println(to_go);
        }
    }

    // AZIMUTH
    if ((timestamp - last_heading_time) >= headingLoopIntervalMs) {       // time to calculate ALT/AZ using most recent readings
        last_heading_time = timestamp;
//        altitude = elevation(acc[0], acc[2], debug);
        altitude = elevation(acc, debug);
//        azimuth = flatCompass(unt_acc, mag, debug);
        azimuth = compass3D(unt_acc, mag, debug);
        // azimuth_ALT = flatCompassALT(unt_acc, mag, debug);
    }

    // OUTPUTS
    if ((timestamp - last_print_time) >= outputLoopIntervalMs) {          // time to send to serial ALT/AZ
        last_print_time = timestamp;
        snprintf(output_str, MAX_LEN_OUT_BUF, "SENSORS, AZ, %+3.3f, ALT, %+3.3f,", azimuth, altitude);
        if(!m_calib_on) {
            if((uint8_t)(debug | ~DEBUG_UNTILT_ACC)==255) {
                Serial.print("UNT ");Serial.print(unt_acc[0], 4);Serial.print(" ");Serial.print(unt_acc[1], 4);Serial.print(" ");Serial.print(unt_acc[2], 4);
                Serial.print(" ");Serial.print(mag[0], 4);Serial.print(" ");Serial.print(mag[1], 4);Serial.print(" ");Serial.println(mag[2], 4);
            }
            if(!debug){  // don't print if debug is active (!= 0)
                Serial.println(output_str);
                // Serial.print("Alternative AZ ");Serial.println(azimuth_ALT);
            }
        }
        Serial1.println(output_str); // send to Raspberry
    }

    checkSerial();        // manual commands
    checkSerial1();       // commands from raspberry
    checkJoystick(debug);
}


/*
 * calibration command must be paired with the duration to execute one full round and back
 * to be parsed, e.g.:
 * 
 *    calibstart 300
 * 
 * will split SAMPLES (1000, or change in calibration.h) readings over 300 seconds.
 * The Telescope controlled by the Raspberry must be able to complete the required movements
 * in the specified time
 */

void debug_on(String msg);
void debug_off(String msg);
void start_mag_calibration(String msg);
void start_mag_compensation(String msg);
void parse_acc_calibration(String msg);

void processCommand(String command) {
  Serial.println(command);
  if(command.startsWith("m_calibration")) {
    if(!m_calib_complete) {
      start_mag_calibration(command);
    }
  } else if(command.startsWith("m_compensation")) {
    start_mag_compensation(command);
  } else if(command=="untilt_zero") {
    zero_untilt();
  } else if(command=="cal_type") {
    Serial1.println("2D");
  } else if(command.startsWith("B_mag")) {
    set_bmag(command);
  } else if(command=="block_v") {
    sensor_block_vertical = true;
  } else if(command=="block_h") {
    sensor_block_vertical = false;
  } else if(command=="print_compensation") {
    print_compensation_values();
  } else if(command=="a_calibration_h") {
    read_horizontal_accel();
  } else if(command=="a_calibration_v") {
    read_vertical_accel();
  } else if(command.startsWith("set_a_calibration")) {
    parse_acc_calibration(command);
  } else if(command=="export_a_calibration") {
    export_a_calibration();
  } else if(command=="print_a_cal") {
    print_a_cal();
  } else if(command=="analysis_carousel") {
    print_carousel_data();
  } else if(command=="samples") {
    SendSamples();
  } else if(command=="ellipse") {
    SendEllipse();    
  } else if(command=="show_m_cal") {
    printIrons();
  } else if(command.startsWith("debug")) {  // valid debugs: MAG_CAL, MAG_RAW, UNTILT, ALT_ACC, ALTAZ, JOYSTICK, ALL
    debug_on(command);
  } else if(command.startsWith("nodebug")) {
    debug_off(command);
  } else {
    Serial.println(command);Serial.println("To Serial: UNRECOGNIZED COMMAND");
    Serial1.print("To Serial1: UNRECOGNIZED COMMAND ");Serial.println(command);
  }
}

// RECEIVES COMMANDS FROM CONSOLE - used during develoment and testing
// Tipically commands to start calibrations or to turn on/off debugging
void checkSerial(){
  if(!Serial.available()){
    return;
  } else {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }
}

// RECEIVES COMMANDS FROM RASPBERRY
// Tipically commands to start calibrations
void checkSerial1(){
  if(!Serial1.available()){
    return;
  } else {
    String command = Serial1.readStringUntil('\n');
    processCommand(command);
  }
}

void debug_on(String msg) {
    String values = msg.substring(6, msg.length());    // extract second word, which is values to turn debugging on, from command string
    if(values == "mag_raw")
        debug |= DEBUG_MAG_RAW;
    if(values == "mag_cal")
        debug |= DEBUG_MAG_CAL;
    if(values == "untilt")
        debug |= DEBUG_UNTILT_ACC;
    if(values == "alt_acc")
        debug |= DEBUG_ALT_ACC;
    if(values == "alt")
        debug |= DEBUG_ALT;
    if(values == "az")
        debug |= DEBUG_AZ;
    if(values == "joystick")
        debug |= DEBUG_JOYSTICK;
    if(values == "all")
        debug = 255;
    Serial.print("Debug: ");Serial.println(debug);
}

void debug_off(String msg) {
    String values = msg.substring(8, msg.length());    // extract second word, which is values to turn debugging on, from command string
    if(values == "mag_raw")
        debug &= ~DEBUG_MAG_RAW;
    if(values == "mag_cal")
        debug &= ~DEBUG_MAG_CAL;
    if(values == "untilt")
        debug &= ~DEBUG_UNTILT_ACC;
    if(values == "alt_acc")
        debug &= ~DEBUG_ALT_ACC;
    if(values == "alt")
        debug &= ~DEBUG_ALT;
    if(values == "az")
        debug &= ~DEBUG_AZ;
    if(values == "joystick")
        debug &= ~DEBUG_JOYSTICK;
    if(values == "all")
        debug = 0;
    Serial.print("Debug: ");Serial.println(debug);  
}

void start_mag_calibration(String msg) {
  m_calib_on = true;
  // NOTE: substring below from X, where X is the length (base 1) of the 1st word of command: m_calibration
  String seconds = msg.substring(13, msg.length());    // extract second word, which is duration in seconds, from command string
  int duration_in_seconds = seconds.toInt();
  magCalLoopIntervalMs = init_cal(duration_in_seconds * 1000); // result in milliseconds for 1000 calibration readings
}

void start_mag_compensation(String msg) {
  compensation_on = true;
  Serial.print("Seconds: ");
  String seconds = msg.substring(15, msg.length());    // extract second word, which is duration in seconds, from command string
  int duration_in_seconds = seconds.toInt();
  Serial.println(duration_in_seconds);
  /***** TODO BAD CALCULATIONS *****/
  magCompLoopIntervalMS = init_compensation(duration_in_seconds * 1000); // result in milliseconds for 1000 calibration readings
  Serial.print("Interval: ");
  Serial.println(magCompLoopIntervalMS);
  compensation_reference = azimuth;
  Serial.print("Comp Reference: ");Serial.println(azimuth);
}

/*
 * Receives from Raspberry the last accel calibration saved
 * parses it and saves it making it unnecessary to calibrate
 * again the accelerometer.
 */
void parse_acc_calibration(String msg) {
  float ox = 0;
  float oz = 0;
  float gx = 1;
  float gz = 1;
  int ParCount = 0;
  String pars[4];
  String a_cal_pars = msg.substring(18, msg.length());
  while(ParCount < 4){
    int idx = a_cal_pars.indexOf(',');
    if(idx == -1) {
      pars[ParCount] = a_cal_pars;
      break;
    } else {
      pars[ParCount++] = a_cal_pars.substring(0, idx);
      a_cal_pars = a_cal_pars.substring(idx+1);
    }
  }
  ox = pars[0].toFloat();
  gx = pars[1].toFloat();
  oz = pars[2].toFloat();
  gz = pars[3].toFloat();
  Serial.print("OX: ");Serial.print(ox);Serial.print("\tGX: ");Serial.print(gx);Serial.print("\tOZ: ");Serial.print(oz);Serial.print("\tGZ: ");Serial.println(gz);
  set_a_calibration(ox, gx, oz, gz);
  Serial.println("Acc calibration saved");
}
