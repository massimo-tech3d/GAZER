/******
 * Initialization and reading
 * of ST LSM6DSV16X gyro/accelerometer Sparkfun breakout board
 * 
 * This accelerometer is used to measure the OTA Altitude
 * 
 * Created by Massimo Tasso, February, 6, 2024
 * Released under GPLv3 License - see LICENSE file for details.
 ******/
#include "sensor.h"
#include "SparkFun_LSM6DSV16X.h"

class AccLSM: private Sensor {

    public:
        int MAG_DATARATE_STRING = LIS3MDL_DATARATE_155_HZ;  // adafruit dice essere il top per la precisione...
        int MAGNET_DATARATE_LIS3MDL = 155;

        bool init_accelerometer(void) {
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
            // initSensor(K_ERR_LIS, K_Q_LIS);  // kalman smooth is not required with this sensor
            return true;
        }

        void zero() {
            // Altitude Acc is zeroed in another place
        }

        // values required to be expressed in gravity units
        void readings(float raw[3]) {
            sfe_lsm_data_t accelData;
            
            if(LSM6D.checkAccelStatus()) {          // what should I do if data not available ?
                LSM6D.getAccel(&accelData);         // should I cycle until available ?
                raw[0] = accelData.xData / 1000;    // the lib returns values in milligravity
                raw[1] = accelData.yData / 1000;
                raw[2] = accelData.zData / 1000;
                axes(raw);
            }
        }

    private:
        SparkFun_LSM6DSV16X LSM6D;

        /*
        * Look at the sensor block and call north the axis pointing forward
        * east and down consequently.
        * This method then swaps the axes in the correct way.
        * 
        * NED REFERENCE FRAME IS REQUIRED
        */
        void axes(float raw[3]) {
            // commented rotations are OK. Check the uncommented ones before deleting
            // float x = raw[0];  // front direction
            // float y = raw[1];  // left direction
            // float z = raw[2];  // up direction
            // raw[0] = x;        // x and y axes are swapped because the chip is rotated 90°
            // raw[1] = z;        // with respect to the sensor block
            // raw[2] = y;        // signs are defined accordingly to NED reference frame
            float north =  raw[0];   // the axis pointing forward is y, [1]
            float east  = -raw[1];   // the axis pointing right is x, [0]
            float down  = -raw[2];   // the axis pointing down is z, [2]
            swap_axes(north, east, down, raw);
        }
}