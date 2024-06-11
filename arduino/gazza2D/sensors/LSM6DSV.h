/******
 * Initialization and reading
 * of ST LSM6DSV16X gyro/accelerometer Sparkfun breakout board
 * 
 * This accelerometer is used to measure the OTA Altitude
 * 
 * Created by Massimo Tasso, February, 6, 2024
 * Released under GPLv3 License - see LICENSE file for details.
 ******/

#include "SparkFun_LSM6DSV16X.h"

#define SENSORS_GRAVITY_EARTH    (9.80665F)  //< Earth's gravity in m/s^2
#define SENSORS_DPS_TO_RADS  (0.017453293F)  //< Degrees/s to rad/s multiplier
#define SENSORS_RADS_TO_DPS  (57.29577793F)  //< Rad/s to degrees/s  multiplier

// this accelerometer is not very noisy, and internally smoothed. Kalman smoothing may not even be required.
#define K_ERR_A_LSM6 0.08   // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_Q_A_LSM6   0.01   // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed

#define ACCEL_DATARATE_LSM 500  // should not be faster than the datarate e.g. 500 vs 1000
#define GYRO_DATARATE_LSM  500  // should not be faster than the datarate e.g. 500 vs 1000

SparkFun_LSM6DSV16X LSM6D;

bool init_accelerometer_LSM(void) {
    if (!LSM6D.begin()) {
        return false;
    }
    LSM6D.enableBlockDataUpdate();   // Accelerometer and Gyroscope registers will not be updated until read
 
    LSM6D.setAccelFullScale(LSM6DSV16X_2g);
    LSM6D.setAccelMode(LSM6DSV16X_XL_HIGH_ACCURACY_ODR_MD);
    LSM6D.setAccelDataRate(LSM6DSV16X_ODR_HA01_AT_1000Hz);  // see doc for all options choose the fastest compatible with cycle
    //LSM6D.setAccelDataRate(LSM6DSV16X_ODR_HA01_AT_125Hz);  // see doc for all options choose the fastest compatible with cycle

    LSM6D.setGyroFullScale(LSM6DSV16X_125dps);
    LSM6D.setGyroMode(LSM6DSV16X_GY_HIGH_ACCURACY_ODR_MD);
    LSM6D.setGyroDataRate(LSM6DSV16X_ODR_HA01_AT_1000Hz);   // see doc for all options

    LSM6D.enableFilterSettling(true); // accel slope filter
    LSM6D.enableAccelLP2Filter(true);  // accel low pass filter 2
    LSM6D.setAccelLP2Bandwidth(LSM6DSV16X_XL_STRONG);  // set bandwidth for accel LP2
    // LSM6D.enableAccelHpFilter(true);  // accel high pass filter
    // LSM6D.enableAccelLPS2(true);      // accel low pass filter
    // LSM6D.enableGyroLP1Filter(true);  // gyro low pass filter 1
    LSM6D.enableGyroLP1Filter(false);    // gyro low pass filter 1
    // LSM6D.setGyroLP1Bandwidth(LSM6DSV16X_GY_ULTRA_LIGHT);  // set bandwidth for gyro LP1
    return true;
}

void zero_LSM() {
    // Altitude Acc is zeroed in another place
}

/*******
 * Mandatory NED convention
 * X axis pointing forward
 * Y axis pointing right
 * Z axis pointing down
********/
void axes_LSM(float raw[3]) {
    float north =  raw[0];  // front direction
    float east  = -raw[1];  // right direction
    float down  = -raw[2];  // down direction
    raw[0] = north;        // x and y axes are swapped because the chip is rotated 90°
    raw[1] = east;        // with respect to the sensor block
    raw[2] = down;        // signs are defined accordingly to NED reference frame
}

/* returns values in gravity */
void accel_readings_LSM(float raw[3]) {
    sfe_lsm_data_t accelData;
    while(!LSM6D.checkAccelStatus()){
    }
    LSM6D.getAccel(&accelData);
    raw[0] = accelData.xData / 1000;    // the lib returns values in milligravity
    raw[1] = accelData.yData / 1000;
    raw[2] = accelData.zData / 1000;
    axes_LSM(raw);
}

/* returns values in deg per second */
void gyro_readings_LSM(float raw[3]) {
    sfe_lsm_data_t gyroData;
    
    while(!LSM6D.checkGyroStatus()){
    }
    LSM6D.getGyro(&gyroData);           // should I cycle until available ?
    raw[0] = gyroData.xData / 1000;     // the lib returns values in millidegrees
    raw[1] = gyroData.yData / 1000;
    raw[2] = gyroData.zData / 1000;
    axes_LSM(raw);
}

