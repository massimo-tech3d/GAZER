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
#include "a_calibration.h"

//int counter = 0;

int LOOP_RATE_HZ;
unsigned long last_loop_time = millis();
unsigned long last_mag_read = millis();
unsigned long last_accel_read = millis();
unsigned long last_print_time = millis();
unsigned long last_calibration_time = millis();

unsigned long kLoopIntervalMs;
unsigned long magLoopIntervalMs;
unsigned long accelLoopIntervalMs;
const unsigned long kPrintIntervalMs = 100;  // sensors sent to Raspberry at 10 Hz - sounds enough

bool calib_complete = false;
bool calib_on = false;        // true if calibration data collection is in progress
unsigned long calibration_interval = 100;

// Buffer for holding general text output
#define MAX_LEN_OUT_BUF 130
char output_str[MAX_LEN_OUT_BUF];

float azimuth = 0;
float altitude = 0;
float acc[3];
float mag[3];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
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
  Serial.print("kLoopIntervalMs: ");Serial.println(kLoopIntervalMs);
}

void checkSerial();
void checkSerial1();

void loop() {
    unsigned long timestamp = millis();
    accel_readings(acc);
    mag_readings(mag);
    if ((timestamp - last_print_time) >= kPrintIntervalMs) {  // time to send compass to Raspberry
        last_print_time = timestamp;
        a_calibrate(acc);      // a_calibrate does not change acc values if they are still required to calibrate
        if(calib_complete)
            m_calibrate(mag);  // don't change mag values if they are still required to calibrate
    
        altitude = elevation(acc[0], acc[2]);
        flatCompass(mag[0], mag[1], mag[2], azimuth);
        
        snprintf(output_str, MAX_LEN_OUT_BUF, "SENSORS, AZ, %+3.3f, ALT, %+3.3f,", azimuth*RadToDeg, altitude*RadToDeg);
        Serial.println( output_str );  //simplest way to see library output
        Serial1.println( output_str ); //simplest way to see library output
    }
    if(calib_on & ((timestamp - last_calibration_time) >= calibration_interval)) {
        last_calibration_time = timestamp;
        int to_go = add_sample(mag, acc);
        if(!to_go) {
            calib_on = false;
            calib_complete = true;
            Serial.println("calibration completed");
        } else {
//dbg            Serial.print("PROCESSED MX\t");Serial.print(mag[0]);Serial.print(" \tMY ");Serial.print(mag[1]);Serial.print(" \tMZ ");Serial.print(mag[2]);
//dbg            Serial.print(" \tAX ");Serial.print(acc[0]);Serial.print(" \tAY ");Serial.print(acc[1]);Serial.print(" \tAZ ");Serial.print(acc[2]);
//dbg            Serial.print(" \tI ");Serial.print(counter);Serial.print(" \ttogo ");Serial.println(to_go);
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
  Serial.print("**** ");Serial.println(command);
  if(command.startsWith("m_calibration")) {  // start accelerometer calibration
    if(!calib_complete) {
      calib_on = true;
      // NOTE: substring below from X, where X is the length (base 1) of the 1st word of command: m_calibration
      String seconds = command.substring(13, command.length());    // extract second word, which is duration in seconds, from command string
      int duration_in_seconds = seconds.toInt();
      calibration_interval = init_cal(duration_in_seconds * 1000); // result in milliseconds for 1000 calibration readings
    }
  } else if(command=="a_calibration_h") {    // read accel calibration horizontal
    read_horizontal_accel();
    calc_a_calibration();
    Serial.println("Acc cal horizontal reading acquired");
    Serial1.println("Acc cal horizontal reading acquired");
  } else if(command=="a_calibration_v") {    // read accel calibration vertical
    read_vertical_accel();
    calc_a_calibration();
    Serial.println("Acc cal horizontal reading acquired");
    Serial1.println("Acc cal horizontal reading acquired");
  } else if(command=="set_a_calibration") {
    // String seconds = command.substring(10, command.length())
    float ox = 0;
    float oz = 0;
    float gx = 0;
    float gz = 0;
    set_a_calibration(ox, oz, gx, gz);
    Serial.println("Acc calibration saved");
  } else if(command=="export_a_calibration") {
    float ox;
    float oz;
    float gx;
    float gz;
    export_a_calibration(ox, gx, oz, gz);
    Serial.print("A_CAL, OX");Serial.print(ox);Serial.print(", OZ");Serial.print(oz);Serial.print(", GX");Serial.print(gx);Serial.print(", GZ");Serial.println(gz);
    Serial1.print("A_CAL, OX");Serial.print(ox);Serial.print(", OZ");Serial.print(oz);Serial.print(", GX");Serial.print(gx);Serial.print(", GZ");Serial.println(gz);
  } else if(command=="print_a_cal") {
    print_a_cal();
  } else if(command=="samples") {
    SendSamples();
  } else if(command=="ellipse") {
    SendEllipse();    
  } else if(command=="show_m_cal") {
    Serial.print("Offset X: ");Serial.print(getAlfa());Serial.print(" Offset Y: ");Serial.println(getBeta());
    printIrons();
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
