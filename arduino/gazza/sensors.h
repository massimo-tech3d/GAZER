/******
 * Sensors initialization and readings
 *   abstracted functions
 *   can handle either two separate sensors or a combined magnetometer/accelerator sensor
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/
 
#include "sensors/LIS3MDL.h"
#include "sensors/RM3100.h"
#include "sensors/MMA8451.h"
#include "sensors/LSM6DSV.h"
#include "sensors/MPU6050.h"
#include "defines.h"
#include <Wire.h>
#include <SimpleKalmanFilter.h>
//#include "SimpleKalmanFilter.h"

/*
 * Untilted MAG is a magnetometer sensor combined with an accelerometer/gyro to compensate for non complanarity with the horizontal plane
 */
bool LIS3MDL_MAG = false;
bool RM3100_MAG = false;
bool MMA_ACCEL = false;
bool LSM_ACCEL = false;
bool FXOS_ACCEL = false;
bool MPU6050_ACCEL = false;

int ACCEL_DATARATE = 0;
int MAG_DATARATE = 0;
int UNTILTER_DATARATE = 0;
bool untilter = false;
double aRMS;

bool sensor_block_vertical = false;

float B = 46.8;  // magnitude of earth mag vector - to be received from raspberry

// the untilting functions require as smooth as possible accelerometer readings
SimpleKalmanFilter* kf_untilt_x;
SimpleKalmanFilter* kf_untilt_y;
SimpleKalmanFilter* kf_untilt_z;

/*
// Kalman filters parameters - defined in magnetometer modules
float K_ERR_AZ = 0.01;  // default value - meaning: 
float K_Q_AZ = 1.5;     // default value - meaning: how much we expect the measurement to vary. Quite small, fastest rotation at 3°/sec when calibrating. Generally below 1"/sec
// Kalman filters parameters - defined in accelerometer modules
float K_ERR_ALT = 0.01;  // default value
float K_Q_ALT = 0.5;     // default value - meaning: how much we expect the measurement to vary. Even smaller than azimuth, the scope rotates at 1°/sec when slewing. Generally below 1"/sec
*/

void zero_untilt();

bool sensors() {
  Wire.begin();

  if(init_magnetometer_LIS3MDL()) {
      Serial.println("LIS3MDL Magnetometer Found");
      LIS3MDL_MAG = true;
      MAG_DATARATE = MAGNET_DATARATE_LIS3MDL;
  } else if(init_magnetometer_R3100()) {
      Serial.println("RM3100 Magnetometer Found");
      RM3100_MAG = true;
      MAG_DATARATE = dataRate_RM3100();
  } else {
      Serial.println("Error, magnetometer not found");
      return false;
  }
  if(init_accelerometer_MMA()) {
      untilter = true;
      MMA_ACCEL = true;
      float K_ERR_U_UNTILT = K_ERR_MMA;
      float K_Q_U_UNTILT = K_Q_MMA;
      kf_untilt_x = new SimpleKalmanFilter(K_ERR_U_UNTILT, K_ERR_U_UNTILT, K_Q_U_UNTILT);
      kf_untilt_y = new SimpleKalmanFilter(K_ERR_U_UNTILT, K_ERR_U_UNTILT, K_Q_U_UNTILT);
      kf_untilt_z = new SimpleKalmanFilter(K_ERR_U_UNTILT, K_ERR_U_UNTILT, K_Q_U_UNTILT);
      UNTILTER_DATARATE = ACCEL_DATARATE_MMA;
      aRMS = ARMS_MMA8451;
      Serial.println("MMA8451 Untilter Found");
      Serial.print("ARMS ");Serial.print(aRMS, 4);Serial.print(" Datarate ");Serial.println(UNTILTER_DATARATE);
  } else if(init_accelerometer_MPU6050()) {
      untilter = true;
      MPU6050_ACCEL = true;
      float K_ERR_U_UNTILT = K_ERR_MPU_FAST;
      float K_Q_U_UNTILT = K_Q_MPU_FAST;
      kf_untilt_x = new SimpleKalmanFilter(K_ERR_U_UNTILT, K_ERR_U_UNTILT, K_Q_U_UNTILT);
      kf_untilt_y = new SimpleKalmanFilter(K_ERR_U_UNTILT, K_ERR_U_UNTILT, K_Q_U_UNTILT);
      kf_untilt_z = new SimpleKalmanFilter(K_ERR_U_UNTILT, K_ERR_U_UNTILT, K_Q_U_UNTILT);
      UNTILTER_DATARATE = ACCEL_DATARATE_MPU6050;
      aRMS = ARMS_MPU6050;
      Serial.println("MPU6050 Untilter Found");
      Serial.print("ARMS ");Serial.print(aRMS, 4);Serial.print(" Datarate ");Serial.println(UNTILTER_DATARATE);
  } else {
    Serial.println("untilting not available - ensure magnetometer block is flat");
    Serial1.println("untilting not available - ensure magnetometer block is flat");
  }
  if(init_accelerometer_LSM()) {
      Serial.println("LSM8DSV16X Accelerometer Found");
      LSM_ACCEL = true;
      ACCEL_DATARATE = ACCEL_DATARATE_LSM;
  } else {
      Serial.println("Error, Accelerometer not found");
      return false;
  }
  return true;
}

void rotate(float raw[3]) {
  if(sensor_block_vertical) {
    float x = -raw[2];
    float y = raw[1];
    float z = raw[0];
    raw[0] = x;
    raw[1] = y;
    raw[2] = z;
  }
}

void mag_readings(float mag_raw[3], uint8_t debug, bool smooth=true) {
  if(LIS3MDL_MAG){
    mag_readings_LIS(mag_raw);  // TODO ADD SMOOTH TO mag_reading_LIS
  } else if(RM3100_MAG) {
    mag_readings_RM3100(mag_raw, smooth);
  }
  rotate(mag_raw);
  mag_raw[0] /= B;  // non ho ancora tolto HI/SI ma va bene lo stesso
  mag_raw[1] /= B;  // perchè sto solo riscalando, non normalizzando
  mag_raw[2] /= B;
  if((uint8_t)(debug | ~DEBUG_MAG_RAW)==255) {
    Serial.print("MAG ");Serial.print(mag_raw[0], 4);Serial.print(" ");Serial.print(mag_raw[1], 4);Serial.print(" ");Serial.println(mag_raw[2], 4);
  }
}

void accel_readings(float accel_raw[3], uint8_t debug) {
  if(LSM_ACCEL){
    accel_readings_LSM(accel_raw);
  }
  if((uint8_t)(debug | ~DEBUG_ALT_ACC)==255) {
    Serial.print("ACC ");Serial.print(accel_raw[0]);Serial.print(" ");Serial.print(accel_raw[1]);Serial.print(" ");Serial.println(accel_raw[2]);
  }
}

void zero_untilt() {
  if(MMA_ACCEL) {
    zero_MMA();
  } else if(MPU6050_ACCEL) {
    zero_MPU6050();
  }
  Serial.println("Gyro/Accel offset calculated");
}

void get_untilt_raw(float raw[3]) {
  if(untilter) {
    if(MMA_ACCEL) {
      accel_readings_MMA(raw);
    } else if(MPU6050_ACCEL) {
      accel_readings_MPU6050(raw);
    }      // else returns raw[3] untouched (and zeroed)
    raw[0] = kf_untilt_x->updateEstimate(raw[0]);
    raw[1] = kf_untilt_y->updateEstimate(raw[1]);
    raw[2] = kf_untilt_z->updateEstimate(raw[2]);
    rotate(raw);
  }
}

void set_bmag(String msg) {
    // raspberry sends something like 46781.68 nT. Has to be divided by 1000 because we need uTesla
    String bs = msg.substring(5, msg.length());    // extract second word, which is duration in seconds, from command string
    B = bs.toFloat()/1000;
}
