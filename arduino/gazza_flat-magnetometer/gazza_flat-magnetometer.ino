/**************************************************************************************
 * This is the final version of gazza. refactored to isolate the sensors
 * initialization and reading functions in semsors.h file. The sensors() function call
 * tries to initialize separate mag and accel sensors (which work best) and falls back
 * to FXOS_8700 sensor for both accelerator and magnetometer.
 * 
 * Sensors initialization parameters are ddefined in L)S3MDL.h (mag only) and FXOS_8700
 * (both accel only and combined initializations) respectively.
 * 
 * Combined sensors are supported for testing purposes only, because the calibration
 * procedure requires separate sensors and the magnetomer permanently flat-horizontal
 * therefore separate sensors are required together with this sketch.
 * 
 * Serial1 IO is to/from the Raspberry
 * Serial IO is to/from the optional usb cable, typically used for testing purposes
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

//int counter = 0;

int LOOP_RATE_HZ;
unsigned long kLoopIntervalMs;
unsigned long magLoopIntervalMs;
unsigned long accelLoopIntervalMs;

unsigned long last_print_time = millis();
const unsigned long kPrintIntervalMs = 100;  // sensors sent to Raspberry at 10 Hz - sounds enough

bool calib_complete = false;
bool calib_on = false;        // true if calibration data collection is in progress
unsigned long last_calibration_time = millis();
unsigned long calibration_interval = 100;

bool compensation_on = false;        // true if compensation data collection is in progress
unsigned long last_compensation_time = millis();
unsigned long compensation_interval = 100;

// Buffer for holding general text output
#define MAX_LEN_OUT_BUF 130
char output_str[MAX_LEN_OUT_BUF];

float azimuth = 0;
float altitude = 0;
float acc[3];
float mag[3];

float compensation_reference = 180.0;

bool live = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);   // Serial: Console
  Serial1.begin(115200);  // Serial1: raspberry
  while(!Serial1) {
    delay(1);
  }

  initJoystick();
  
  /* Initialise the sensors */
  while(!(sensors())) {
    Serial.println("Sensor error");
    Serial1.println("Sensor error");
    delay(1000);
  }
  // need to initialize the values here because they depend on sensors() result
  LOOP_RATE_HZ = min(MAG_DATARATE, ACCEL_DATARATE);
  magLoopIntervalMs = 1000 / MAG_DATARATE;
  accelLoopIntervalMs = 1000 / ACCEL_DATARATE;
  kLoopIntervalMs = max(magLoopIntervalMs, accelLoopIntervalMs);
  Serial.print("MAG interval ms: ");Serial.println(magLoopIntervalMs);
  Serial.print("ACC interval ms: ");Serial.println(accelLoopIntervalMs);
  Serial.print("kLoopIntervalMs: ");Serial.println(kLoopIntervalMs);
}

void checkSerial();
void checkSerial1();

void loop() {
    unsigned long timestamp = millis();
    accel_readings(acc);
    mag_readings(mag);
    a_calibrate(acc);      // can call a_calibrate cause it does not change acc values if they are still uncalibrated
    m_calibrate(mag);      // can call m_calibrate even if calibration is not completed
    altitude = elevation(acc[0], acc[2]);
    azimuth = flatCompass(mag[0], mag[1], mag[2]);
    azimuth = m_compensate(azimuth, altitude);

    if ((timestamp - last_print_time) >= kPrintIntervalMs) {  // time to send compass to Raspberry
        last_print_time = timestamp;

        snprintf(output_str, MAX_LEN_OUT_BUF, "SENSORS, AZ, %+3.3f, ALT, %+3.3f,", azimuth, altitude);
        if(!calib_on) {
            Serial.println( output_str );  //simplest way to see library output
        }
        Serial1.println( output_str ); //simplest way to see library output
    }

    if(compensation_on & ((timestamp - last_compensation_time) >= compensation_interval)) {
        last_compensation_time = timestamp;
        int to_go = add_compensation_sample(compensation_reference, altitude, azimuth);
        if(!to_go) {
          compensation_on = false;
          Serial.println("M_CAL, compensation map completed");
          Serial1.println("M_CAL, compensation map completed");
        } else {
          Serial.print(millis());Serial.print(" \tM Compensation togo ");Serial.println(to_go);
        }
    }
    
    if(calib_on & ((timestamp - last_calibration_time) >= calibration_interval)) {
        last_calibration_time = timestamp;
        int to_go = add_sample(mag, acc);
        if(!to_go) {
            calib_on = false;
            calib_complete = true;
            Serial.println("M_CAL, points collected");
            Serial1.println("M_CAL, points collected");
        } else {
            Serial.print(millis());Serial.print(" \tM Calibration togo ");Serial.println(to_go);
        }
    }
    checkSerial();
    checkSerial1();
    checkJoystick();
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
void processCommand(String command){
  if(command.startsWith("m_calibration")) {  // start accelerometer calibration
    if(!calib_complete) {
      calib_on = true;
      // NOTE: substring below from X, where X is the length (base 1) of the 1st word of command: m_calibration
      String seconds = command.substring(13, command.length());    // extract second word, which is duration in seconds, from command string
      int duration_in_seconds = seconds.toInt();
      calibration_interval = init_cal(duration_in_seconds * 1000); // result in milliseconds for 1000 calibration readings
    }
  }  else if(command.startsWith("m_cal_to_horizontal")) {
    compensation_on = true;
    String seconds = command.substring(19, command.length());    // extract second word, which is duration in seconds, from command string
    int duration_in_seconds = seconds.toInt();
    compensation_interval = init_compensation(duration_in_seconds * 1000); // result in milliseconds for 1000 calibration readings

    // calculates the azimuth at starting time (ALT=90°) which shall stay fixed
    // note: azimut oscillates randomnly within an range hopefully less than 1°
    // this is the precision limit of whole system. Compensation won't be more accurate.
    mag_readings(mag);
    m_calibrate(mag);      // can call m_calibrate even if calibration is not completed
    float az = flatCompass(mag[0], mag[1], mag[2]);

    compensation_reference = az;
  } else if(command=="a_calibration_h") {    // read accel calibration horizontal
    read_horizontal_accel();
    Serial.println("Acc cal horizontal reading acquired");
    Serial1.println("Acc cal horizontal reading acquired");
  } else if(command=="a_calibration_v") {    // read accel calibration vertical
    read_vertical_accel();
    Serial.println("Acc cal horizontal reading acquired");
    Serial1.println("Acc cal horizontal reading acquired");
  } else if(command.startsWith("set_a_calibration")) {
    float ox = 0;
    float oz = 0;
    float gx = 1;
    float gz = 1;
    int ParCount = 0;
    String pars[4];
    String a_cal_pars = command.substring(18, command.length());
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
  } else if(command=="export_a_calibration") {
    float ox;
    float oz;
    float gx;
    float gz;
    export_a_calibration(ox, gx, oz, gz);
    Serial.print("A_CAL,OX,");Serial.print(ox);Serial.print(",GX,");Serial.print(gx);Serial.print(",OZ,");Serial.print(oz);Serial.print(",GZ,");Serial.println(gz);
    Serial1.print("A_CAL, ");Serial1.print(ox);Serial1.print(", ");Serial1.print(gx);Serial1.print(", ");Serial1.print(oz);Serial1.print(", ");Serial1.println(gz);
  } else if(command=="print_a_cal") {
    print_a_cal();
  } else if(command=="samples") {
    SendSamples();
  } else if(command=="ellipse") {
    SendEllipse();    
  } else if(command=="show_m_cal") {
    Serial.print("Offset X: ");Serial.print(getAlfa());Serial.print(" Offset Y: ");Serial.println(getBeta());
    printIrons();
  } else if(command=="live") {
    live = true;
  } else {
    Serial.println(command);Serial.println("To Serial: UNRECOGNIZED COMMAND");
    Serial1.println("To Serial1: UNRECOGNIZED COMMAND");
  }
}

// RECEIVES COMMANDS FROM CONSOLE - used during develoment and testing
// Tipically commands to start Mag calibration
void checkSerial(){
  if(!Serial.available()){
    return;
  } else {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }
}

// RECEIVES COMMANDS FROM RASPBERRY
// Tipically commands to start Mag calibrations
void checkSerial1(){
  if(!Serial1.available()){
    return;
  } else {
    String command = Serial1.readStringUntil('\n');
    processCommand(command);
  }
}
