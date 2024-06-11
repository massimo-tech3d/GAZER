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

bool init_accelerometer_MPU6050(void) {
  delay(100);
  // sets gyro to 250dps / acc to 2G
  if(!(mpu.begin(0, 0) == 0)){
    return false;
  }
  return true;
}

/*******
 * Mandatory NED convention
 * X axis pointing forward
 * Y axis pointing right
 * Z axis pointing down
********/
void axes_MPU6050(float raw[3]) {
  float north = raw[1];   // right direction becomes front
  float east =  raw[0];   // front direction becomes left
  float down = -raw[2];   // down direction

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
void accel_readings_MPU6050(float raw[3]) {
    mpu.update();
    raw[0] = mpu.getAccY();
    raw[1] = mpu.getAccX();
    raw[2] = mpu.getAccZ();
    axes_MPU6050(raw);
}

void gyro_readings_MPU6050(float raw[3]) {
    mpu.update();
    raw[0] = mpu.getGyroY();
    raw[1] = mpu.getGyroX();
    raw[2] = mpu.getGyroZ();
    axes_MPU6050(raw);
}

void zero_MPU6050() {
    mpu.calcOffsets(true,true); // gyro and accelero
    Serial.println("Gyro/Accel offset calculated");
}