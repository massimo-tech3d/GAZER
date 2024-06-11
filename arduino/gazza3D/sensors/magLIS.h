#include "sensor.h"
#include <Adafruit_LIS3MDL.h>


class MagLIS: private Sensor {

    public:
        int MAG_DATARATE_STRING = LIS3MDL_DATARATE_155_HZ;  // adafruit dice essere il top per la precisione...
        int MAGNET_DATARATE_LIS3MDL = 155;

        bool init_magnetometer() {
            magnet_LIS3MDL = Adafruit_LIS3MDL();
            if (!magnet_LIS3MDL.begin_I2C()) {
                return false;
            }
            magnet_LIS3MDL.setDataRate(MAG_DATARATE_STRING);
            magnet_LIS3MDL.setPerformanceMode(LIS3MDL_ULTRAHIGHMODE);
            magnet_LIS3MDL.setOperationMode(LIS3MDL_CONTINUOUSMODE);
            magnet_LIS3MDL.setRange(LIS3MDL_RANGE_4_GAUSS);
            magnet_LIS3MDL.setIntThreshold(500);
            magnet_LIS3MDL.configInterrupt(true, true, true,
                                           true,
                                           false, // don't latch  // ??? benefit of latching ?
                                           true);
            initSensor(K_ERR_LIS, K_Q_LIS);
            return true;
        }

        void readings(float *mag_raw) {
            sensors_event_t mevent;
            magnet_LIS3MDL.getEvent(&mevent);
            mag_raw[0] = mevent.magnetic.x;  // gauss
            mag_raw[1] = mevent.magnetic.y;  // gauss
            mag_raw[2] = mevent.magnetic.z;  // gauss
            smooth_readings(mag_raw);
            mag_axes(mag_raw);
        }

        int dataRate(void) {
            return MAGNET_DATARATE_LIS3MDL;
        }

    private:
        float K_ERR_LIS = 1.0;   //0.08   // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
        float K_Q_LIS   = 0.01;  //0.01   // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed
        Adafruit_LIS3MDL magnet_LIS3MDL;

        /*
        * Look at the sensor block and call north the axis pointing forward
        * east and down consequently.
        * This method then swaps the axes in the correct way.
        * 
        * NED REFERENCE FRAME IS REQUIRED
        */
        void axes(float *raw) {
            float north = raw[1];   // the axis pointing forward is y, [1]
            float east  = raw[0];   // the axis pointing right is x, [0]
            float down  = raw[2];   // the axis pointing down is z, [2]
            swap_axes(north, east, down, raw);
        }
}