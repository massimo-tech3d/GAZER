/******
 * Sensors initialization and readings
 *   abstracted functions
 *   can handle either two separate sensors or a combined magnetometer/accelerator sensor
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/
 
#include "sensors/LIS3MDL.h"
#include "sensors/MMA8451.h"
#include "sensors/FXOS_8700.h"
#include "sensors/RM3100.h"

#define FLAT_MAG true // is the magnetometer flat or tilting together with accelerometer

bool LIS3MDL_MAG = false;
bool FXOS_MAG = false;
bool RM3100_MAG = false;
bool MMA_ACCEL = false;
bool FXOS_ACCEL = false;

int ACCEL_DATARATE = 0;
int MAG_DATARATE = 0;

// TODO
// Se trovo due FXOS devo richiuderli e riaprili in combined mode
bool sensors() {
  if(init_magnetometer_LIS3MDL()) {
      Serial.println("LIS3MDL Magnetometer Found");
      LIS3MDL_MAG = true;
      MAG_DATARATE = MAGNET_DATARATE_LIS3MDL;
  } else if(init_magnetometer_FXOS()) {
      Serial.println("FXOS Magnetometer Found");
      FXOS_MAG = true;
      MAG_DATARATE = MAGNET_DATARATE_FXOS;
  } else if(init_magnetometer_R3100()) {
      Serial.println("RM3100 Magnetometer Found");
      RM3100_MAG = true;
      MAG_DATARATE = dataRate_RM3100();
  } else {
      Serial.println("Error, magnetometer not found");
      return false;
  }
  if(init_accelerometer_MMA()) {
      Serial.println("MMA8421 Accelerometer Found");
      MMA_ACCEL = true;
      ACCEL_DATARATE = ACCEL_DATARATE_MMA;
  }
  else if (init_accelerometer_FXOS()) {
      Serial.println("FXOS Accelerometer Found");
      FXOS_ACCEL = true;
      ACCEL_DATARATE = ACCEL_DATARATE_FXOS;
  } else {
      Serial.println("Error, magnetometer not found");
      return false;
  }
  if(FXOS_MAG & FXOS_ACCEL) {
    Serial.println("need to close magnetometer and accelerometer and reopen the FXOS as common mode");
    return false;
  }
  return true;
}


void mag_readings(float mag_raw[3]) {
  if(LIS3MDL_MAG){
    mag_readings_LIS(mag_raw, FLAT_MAG);
  } else if(FXOS_MAG) {
    mag_readings_FXOS(mag_raw, FLAT_MAG);
  } else if(RM3100_MAG) {
    mag_readings_RM3100(mag_raw, FLAT_MAG);
  }
  // Serial.print("MX,");Serial.print(mag_raw[0]);Serial.print(",MY,");Serial.print(mag_raw[1]);Serial.print(",MZ,");Serial.println(mag_raw[2]);
}

void accel_readings(float accel_raw[3]) {
  if(MMA_ACCEL){
    accel_readings_MMA(accel_raw);
  } else if(FXOS_ACCEL) {
    accel_readings_FXOS(accel_raw);
  }
}
