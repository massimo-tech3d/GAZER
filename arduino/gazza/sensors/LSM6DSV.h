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

#define ACCEL_DATARATE_LSM 100  // should be SLOWER than the datarate e.g. 120 vs 100

SparkFun_LSM6DSV16X LSM6D;

bool init_accelerometer_LSM(void) {
    if (!LSM6D.begin()) {
        return false;
    }
    LSM6D.enableBlockDataUpdate();   // Accelerometer and Gyroscope registers will not be updated until read
 
    LSM6D.setAccelDataRate(LSM6DSV16X_ODR_AT_120Hz);  // see doc for all options choose the fastest compatible with cycle
    LSM6D.setAccelFullScale(LSM6DSV16X_2g);
    LSM6D.setAccelMode(LSM6DSV16X_XL_HIGH_ACCURACY_ODR_MD);

    LSM6D.setGyroDataRate(LSM6DSV16X_ODR_AT_120Hz);   // see doc for all options
    LSM6D.setGyroFullScale(LSM6DSV16X_125dps);
    LSM6D.setGyroMode(LSM6DSV16X_GY_HIGH_ACCURACY_ODR_MD);

    LSM6D.enableFilterSettling(true); // accel slope filter
    LSM6D.enableAccelLP2Filter(true);  // accel low pass filter 2
    LSM6D.setAccelLP2Bandwidth(LSM6DSV16X_XL_STRONG);  // set bandwidth for accel LP2
    // LSM6D.enableAccelHpFilter(true);  // accel high pass filter
    // LSM6D.enableAccelLPS2(true);      // accel low pass filter
    LSM6D.enableGyroLP1Filter(true);  // gyro low pass filter 1
    LSM6D.setGyroLP1Bandwidth(LSM6DSV16X_GY_ULTRA_LIGHT);  // set bandwidth for gyro LP1
    return true;
}

void zero_LSM() {
    // Altitude Acc is zeroed in another place
}

// Rotates axes according to NED reference frame
void axes_LSM(float raw[3]) {
    float x = raw[0];  // front direction
    float y = raw[1];  // left direction
    float z = raw[2];  // up direction
    raw[0] = x;        // x and y axes are swapped because the chip is rotated 90°
    raw[1] = z;        // with respect to the sensor block
    raw[2] = y;        // signs are defined accordingly to NED reference frame
}

void accel_readings_LSM(float raw[3]) {
    sfe_lsm_data_t accelData;
    
    if(LSM6D.checkAccelStatus()) {          // what should I do if data not available ?
        LSM6D.getAccel(&accelData);         // should I cycle until available ?
        raw[0] = accelData.xData / 1000;    // the lib returns values in milligravity
        raw[1] = accelData.yData / 1000;
        raw[2] = accelData.zData / 1000;
        axes_LSM(raw);
    }
}

void gyro_readings_LSM(float raw[3]) {
    sfe_lsm_data_t gyroData;
    
    if(LSM6D.checkGyroStatus()) {           // what should I do if data not available ?
        LSM6D.getGyro(&gyroData);           // should I cycle until available ?
        raw[0] = gyroData.xData / 1000;     // the lib returns values in millidegrees
        raw[1] = gyroData.yData / 1000;
        raw[2] = gyroData.zData / 1000;
        axes_LSM(raw);
    }
}

