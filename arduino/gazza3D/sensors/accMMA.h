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
#include <Adafruit_MMA8451.h>

class AccMMA: private Sensor {

    public:
        bool init_accelerometer(void) {
            accel_MMA = Adafruit_MMA8451();

            if (!accel_MMA.begin()) {
                return false;
            }
            accel_MMA.setRange(MMA8451_RANGE_2_G);
            accel_MMA.setDataRate(ACC_DATARATE_MMA);     // slowest possible data rate for stable ACC readings is 3.125 but too slow for the loop
            return true;
        }

        void zero() {
            // Altitude Acc is zeroed in another place
            Serial.print("Zeroing MMA");
            accXoffset = 0;
            accYoffset = 0;
            accZoffset = 0;
            float ag[3] = {0,0,0};
            float uraw[3];
            for(int i = 0; i < 3; i++){               // the first readings after a cold start are always way off let's just skip them
                accel_readings_MMA(uraw);             // don't rotate axes in this phase
                delay(1000/ACCEL_DATARATE_MMA);       // wait a little bit between 2 measurements
            }

            for(int i = 0; i < CALIB_OFFSET_SAMPLES; i++) {
                accel_readings_MMA(uraw);
                ag[0] += uraw[0];
                ag[1] += uraw[1];
                ag[2] += (uraw[2]-1.0);
                delay(1000/ACCEL_DATARATE_MMA);       // wait a little bit between 2 measurements
            }

            accXoffset = ag[0] / CALIB_OFFSET_SAMPLES;
            accYoffset = ag[1] / CALIB_OFFSET_SAMPLES;
            accZoffset = ag[2] / CALIB_OFFSET_SAMPLES;
            Serial.print("Number of samples ");Serial.print(CALIB_OFFSET_SAMPLES);
            Serial.print(" ag[2] ");Serial.println(ag[2]);
            Serial.print("XO: ");Serial.print(accXoffset,4);
            Serial.print(" YO: ");Serial.print(accYoffset,4);
            Serial.print(" ZO: ");Serial.println(accZoffset,4);
        }

        // values required to be expressed in gravity units
        void readings(float raw[3]) {
            sensors_event_t aevent;
            accel_MMA.getEvent(&aevent);
            raw[0] = aevent.acceleration.x / SENSORS_GRAVITY_EARTH - accXoffset;
            raw[1] = aevent.acceleration.y / SENSORS_GRAVITY_EARTH + accYoffset;  // why + ? Mistery... just works
            raw[2] = aevent.acceleration.z / SENSORS_GRAVITY_EARTH - accZoffset;
            axes(accel_raw);
        }


    private:
        int ACCEL_DATARATE_MMA = 200;
        int ACC_DATARATE_MMA = MMA8451_DATARATE_200_HZ;
        float K_ERR_MMA = 0.01;  // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
        float K_Q_MMA   = 0.05;  // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed
        float SENSORS_GRAVITY_EARTH    (9.80665F)  //< Earth's gravity in m/s^2

        Adafruit_MMA8451 accel_MMA;  //  = Adafruit_MMA8451(int32_t id = -1);
        float accXoffset = 0;
        float accYoffset = 0;
        float accZoffset = 0;
        int CALIB_OFFSET_SAMPLES = 100;  // adjust according to ACCEL_DATARATE_MMA -- sample for 5 seconds 12.5*5 = 62.5


        /*
        * Look at the sensor block and call north the axis pointing forward
        * east and down consequently.
        * This method then swaps the axes in the correct way.
        * 
        * NED REFERENCE FRAME IS REQUIRED
        */
        void axes(float raw[3]) {
            float north =  raw[0];   // the axis pointing forward is y, [1]
            float east  = -raw[1];   // the axis pointing right is x, [0]
            float down  =  raw[2];   // the axis pointing down is z, [2]
            swap_axes(north, east, down, raw);
        }
}