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
#include "MMA8451.h"

Adafruit_LIS3MDL magnet;
//Adafruit_FXOS8700 accel; // = Adafruit_FXOS8700(0x8700A, -1);  // Accelerometer
Adafruit_MMA8451 accel; // = Adafruit_FXOS8700(0x8700A, -1);  // Accelerometer

float ACCEL_DATARATE = ACCEL_DATARATE_MMA;
float MAG_DATARATE = MAG_DATARATE_LIS3MDL;
bool FX_ACCEL = false;
bool MMA_ACCEL = false;

bool sensors() {
  if(init_magnetometer()) {       // tries to initialize the single magnetometer
    Serial.println("LIS3MDL Found");
    if(init_accelerometer_MMA()) {   // initializes the single accelerometer
      MMA_ACCEL = true;
      FX_ACCEL = false;
      Serial.println("MMA8451 Accelerometer Found");
      return true;
    }
    if(init_accelerometer_FX()) {   // initializes the single accelerometer
      MMA_ACCEL = false;
      FX_ACCEL = true;
      Serial.println("FXOS Accelerometer Found");
      return true;
    }
  }
  return false;
}

void mag_readings(float mag_raw[3]) {
  mag_readings_LIS(mag_raw);
}

void accel_readings(float accel_raw[3]) {
  if(MMA_ACCEL)
    accel_readings_MMA(accel_raw);
  else
    accel_readings_FX(accel_raw);
}
