/******
 * Initialization and reading
 * of NXP MMA8451 accelerometer Adafruit breakout board
 * 
 * This accelerometer is used to untilt the compass
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/

#include <MPU6050_light.h>
#include <Wire.h>

#define ACCEL_DATARATE_MPU6050 500
double ARMS_MPU6050 = 0.01;

#define K_ERR_MPU_FAST 0.1 // 0.01   // kalman filter error estimate - sensor specific
#define K_Q_MPU_FAST   0.5  // 1.5   // kalman filter process variance - sensor specific
//SLOW
#define K_ERR_MPU_SLOW 1.0 // 1.0   // kalman filter error estimate - sensor specific
#define K_Q_MPU_SLOW   0.01 // 0.01   // kalman filter process variance - sensor specific

MPU6050 mpu(Wire);

// The MMA8452 sensor returns the most accurate and noiseless data when
// used at the slowest datarate. The slowest would be 3.125 Hz, but that's
// too slow for the main loop --> ACC_DATARATE_MMA defined 12.5 Hz
bool init_accelerometer_MPU6050(void) {
  Serial.print("MPU-6050");
  delay(100);
  if(!(mpu.begin() == 0)){
    return false;
  }
  Serial.println(" started!");
  return true;
}

/*******
 * Mandatory NED convention
********/
void axes_MPU6050(float raw[3]) {
  float north = raw[0];
  float east = raw[1];
  float down = raw[2];

  raw[0] = north; // -x;
  raw[1] = east; // y;
  raw[2] = down; // -z;
}

/**************************************************************
 *                        IMPORTANT
 *       The untilter accelerometer MUST return readings
 *  expressed in gravity units, not in m/s^2 or whatever else
 * 
 *         Compliance to NED convention is necessary
 **************************************************************/
void accel_readings_MPU6050(float untilt_raw[3], bool axes=true) {
    mpu.update();
    untilt_raw[0] = mpu.getAccY();
    untilt_raw[1] = mpu.getAccX();
    untilt_raw[2] = mpu.getAccZ();
    axes_MPU6050(untilt_raw);
}

void zero_MPU6050() {
    mpu.calcOffsets(true,true); // gyro and accelero
    Serial.println("Gyro/Accel offset calculated");
}