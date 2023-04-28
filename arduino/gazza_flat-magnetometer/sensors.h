/******
 * Sensors initialization and readings
 *   abstracted functions
 *   can handle either two separate sensors or a combined magnetometer/accelerator sensor
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/
 
#include "LIS3MDL.h"
#include "FXOS_8700.h"

Adafruit_LIS3MDL magnet;
Adafruit_FXOS8700 accel; // = Adafruit_FXOS8700(0x8700A, -1);  // Accelerometer
Adafruit_FXOS8700 comb_mag; // = Adafruit_FXOS8700(0x8700A, 0x8700B);

bool combo = false;
float ACCEL_DATARATE = ACCEL_DATARATE_FXOS_SINGLE;
float MAG_DATARATE = MAG_DATARATE_LIS3MDL;

bool sensors() {
  if(init_magnetometer()) {       // tries to initialize the single magnetometer
    Serial.println("LIS3MDL Found");
    if(init_accelerometrer()) {   // initializes the single accelerometer
      Serial.println("FXOS Accelerometer Found");
      return true;
    }
  } else {                        // failed to init the single magnetometer
    if(init_sensors()) {          // tries to initialize the combined sensors
      Serial.println("FXOS Combo Found");
      combo = true;
      ACCEL_DATARATE = ACCEL_DATARATE_FXOS_COMBO;
      MAG_DATARATE = MAG_DATARATE_FXOS_COMBO;
      return true;
    }
  }
  return false;
}

void mag_readings(float mag_raw[3]) {
  if(combo) {
    mag_readings_FX(mag_raw);
  } else {
    mag_readings_LIS(mag_raw);
  }
}

void accel_readings(float accel_raw[3]) {
    accel_readings(combo, accel_raw);
}
