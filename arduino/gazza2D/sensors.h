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

/*
 * Untilted MAG is a magnetometer sensor combined with an accelerometer/gyro to compensate for non complanarity with the horizontal plane
 */
bool LIS3MDL_MAG = false;
bool RM3100_MAG = false;
bool MMA_ACCEL = false;
bool LSM_ACCEL = false;
bool MPU6050_ACCEL = false;
bool MMA_UNTILTER = false;
bool MPU6050_UNTILTER = false;

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
SimpleKalmanFilter* kf_mag_x;
SimpleKalmanFilter* kf_mag_y;
SimpleKalmanFilter* kf_mag_z;
SimpleKalmanFilter* kf_acc_x;
SimpleKalmanFilter* kf_acc_y;
SimpleKalmanFilter* kf_acc_z;

bool mag_smooth = false;
bool acc_smooth = false;
bool untilt_smooth = false;


/*
// Kalman filters parameters - defined in magnetometer modules
float K_ERR_AZ = 0.01;  // default value - meaning: 
float K_Q_AZ = 1.5;     // default value - meaning: how much we expect the measurement to vary. Quite small, fastest rotation at 3°/sec when calibrating. Generally below 1"/sec
// Kalman filters parameters - defined in accelerometer modules
float K_ERR_ALT = 0.01;  // default value
float K_Q_ALT = 0.5;     // default value - meaning: how much we expect the measurement to vary. Even smaller than azimuth, the scope rotates at 1°/sec when slewing. Generally below 1"/sec
*/

void zero_untilt();

void findUntilter() {
    float K_ERR_UNTILT;
    float K_Q_UNTILT;
    if((!MMA_ACCEL) & (init_accelerometer_MMA())) {
        Serial.println("MMA8451 Untilter Accelerometer Found");
        untilter = true;
        MMA_UNTILTER = true;
        UNTILTER_DATARATE = ACCEL_DATARATE_MMA;
        K_ERR_UNTILT = K_ERR_MMA;
        K_Q_UNTILT = K_Q_MMA;
        untilt_smooth = true;  // MMA8451 is best if smoothed
    } else if(init_accelerometer_MPU6050()) {
        Serial.println("MPU6050 Untilter Accelerometer Found");
        untilter = true;
        MPU6050_UNTILTER = true;
        UNTILTER_DATARATE = ACCEL_DATARATE_MPU6050;
        K_ERR_UNTILT = K_ERR_MPU_FAST;
        K_Q_UNTILT = K_Q_MPU_FAST;
        untilt_smooth = true;  // MPU6050 is best if smoothed
    }  else {
        Serial.println("No untilter accelerometer found. Make sure the telescope mount is perfectly level");
        return;
    }
    kf_untilt_x = new SimpleKalmanFilter(K_ERR_UNTILT, K_ERR_UNTILT, K_Q_UNTILT);
    kf_untilt_y = new SimpleKalmanFilter(K_ERR_UNTILT, K_ERR_UNTILT, K_Q_UNTILT);
    kf_untilt_z = new SimpleKalmanFilter(K_ERR_UNTILT, K_ERR_UNTILT, K_Q_UNTILT);
}

bool sensors() {
    float K_ERR_ACC;
    float K_Q_ACC;
    float K_ERR_MAG;
    float K_Q_MAG;
    Wire.begin();
    // Search for Altitude Accelerometer - supported LSM and MMA
    if(init_accelerometer_LSM()) {
        Serial.println("LSM6DSV16X Accel/Gyro found");
        LSM_ACCEL = true;
        ACCEL_DATARATE = ACCEL_DATARATE_LSM;
        K_ERR_ACC = K_ERR_A_LSM6;
        K_Q_ACC = K_Q_A_LSM6;
        acc_smooth = false;  // LSM6DSV does not require smoothing
    } else if(init_accelerometer_MMA()) {
        MMA_ACCEL = true;
        ACCEL_DATARATE = ACCEL_DATARATE_MMA;
        K_ERR_ACC = K_ERR_MMA;
        K_Q_ACC = K_Q_MMA;
        acc_smooth = true;  // MMA8451 is best if smoothed
        Serial.println("MMA8451 Accelerometer Found");
    }  else {
        Serial.println("Error, accelerometer not found");
        return false;
    }
    kf_acc_x = new SimpleKalmanFilter(K_ERR_ACC, K_ERR_ACC, K_Q_ACC);
    kf_acc_y = new SimpleKalmanFilter(K_ERR_ACC, K_ERR_ACC, K_Q_ACC);
    kf_acc_z = new SimpleKalmanFilter(K_ERR_ACC, K_ERR_ACC, K_Q_ACC);


    if(init_magnetometer_LIS3MDL()) {
        Serial.println("LIS3MDL Magnetometer Found");
        LIS3MDL_MAG = true;
        MAG_DATARATE = MAGNET_DATARATE_LIS3MDL;
        K_ERR_MAG = K_ERR_M_LIS;
        K_Q_MAG = K_Q_M_LIS;
        mag_smooth = true;  // LIS3MDL is best if smoothed
//        mag_smooth = false;  // LIS3MDL is best if smoothed
        findUntilter();
    } else if(init_magnetometer_R3100()) {
        Serial.println("RM3100 Magnetometer Found");
        RM3100_MAG = true;
        MAG_DATARATE = dataRate_RM3100();
        K_ERR_MAG = K_ERR_M_RM3100;
        K_Q_MAG = K_Q_M_RM3100;
        mag_smooth = true;
        findUntilter();
    } else {
        Serial.println("Error, magnetometer not found");
        return false;
    }
    kf_mag_x = new SimpleKalmanFilter(K_ERR_MAG, K_ERR_MAG, K_Q_MAG);
    kf_mag_y = new SimpleKalmanFilter(K_ERR_MAG, K_ERR_MAG, K_Q_MAG);
    kf_mag_z = new SimpleKalmanFilter(K_ERR_MAG, K_ERR_MAG, K_Q_MAG);
    return true;
}

void rotate(float raw[3], bool force=false) {
  if(sensor_block_vertical | force) {
    float x = raw[0];
    float y = raw[2];
    float z = -raw[1];
    raw[0] = x;
    raw[1] = y;
    raw[2] = z;
  }
}

void mag_readings(float mag_raw[3], uint8_t debug) {
  if(LIS3MDL_MAG){
    mag_readings_LIS(mag_raw);
  } else if(RM3100_MAG) {
    mag_readings_RM3100(mag_raw);
  }
  rotate(mag_raw);
  if(mag_smooth) {
      mag_raw[0] = kf_mag_x->updateEstimate(mag_raw[0]);
      mag_raw[1] = kf_mag_y->updateEstimate(mag_raw[1]);
      mag_raw[2] = kf_mag_z->updateEstimate(mag_raw[2]);
  }
  mag_raw[0] /= B;  // non ho ancora tolto HI/SI ma va bene lo stesso
  mag_raw[1] /= B;  // perchè sto solo riscalando, non normalizzando
  mag_raw[2] /= B;
  if((uint8_t)(debug | ~DEBUG_MAG_RAW)==255) {
      Serial.print("MAG ");Serial.print(mag_raw[0], 4);Serial.print(" ");Serial.print(mag_raw[1], 4);Serial.print(" ");Serial.println(mag_raw[2], 4);
  }
}

void accel_readings(float accel_raw[3], uint8_t debug, bool smooth=true) {
  if(LSM_ACCEL){
    accel_readings_LSM(accel_raw);
  } else if(MMA_ACCEL) {
    accel_readings_MMA(accel_raw);
  }
  rotate(accel_raw, true);
  if(acc_smooth) {
      accel_raw[0] = kf_acc_x->updateEstimate(accel_raw[0]);
      accel_raw[1] = kf_acc_y->updateEstimate(accel_raw[1]);
      accel_raw[2] = kf_acc_z->updateEstimate(accel_raw[2]);
  }

  if((uint8_t)(debug | ~DEBUG_ALT_ACC)==255) {
    Serial.print("ACC ");Serial.print(accel_raw[0]);Serial.print(" ");Serial.print(accel_raw[1]);Serial.print(" ");Serial.println(accel_raw[2]);
  }
}

void zero_untilt() {
  if(MMA_UNTILTER) {
    zero_MMA();
  } else if(MPU6050_UNTILTER) {
    zero_MPU6050();
  }
  Serial.println("Gyro/Accel offset calculated");
}

void get_untilt_raw(float raw[3], uint8_t debug, bool smooth=true) {
  if(untilter) {
    if(MMA_UNTILTER) {
      accel_readings_MMA(raw);
    } else if(MPU6050_UNTILTER) {
      accel_readings_MPU6050(raw);
    }      // else returns raw[3] untouched (and zeroed)
    rotate(raw);
    if(untilt_smooth) {
        raw[0] = kf_untilt_x->updateEstimate(raw[0]);
        raw[1] = kf_untilt_y->updateEstimate(raw[1]);
        raw[2] = kf_untilt_z->updateEstimate(raw[2]);
    }
    if((uint8_t)(debug | ~DEBUG_UNTILT_ACC)==255) {
        Serial.print("UNT ");Serial.print(raw[0]);Serial.print(" ");Serial.print(raw[1]);Serial.print(" ");Serial.println(raw[2]);
    }
  }
}

void set_bmag(String msg) {
    // raspberry sends something like 46781.68 nT. Has to be divided by 1000 because we need uTesla
    String bs = msg.substring(5, msg.length());    // extract second word, which is duration in seconds, from command string
    B = bs.toFloat()/1000;
}
