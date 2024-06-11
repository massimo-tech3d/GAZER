/******
 * Initialization and reading
 * of ST LIS3MDL magnetometer Adafruit breakout board
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/

/*************
 * 
 * STEPPER HZ, trying to minimize interferences
 * 
 * 0.25°/sec : 3840 Hz
 * 0.5°/sec  : 7680 Hz
 * 1°/sec    : 15.360 Hz
 * 2°/sec    : 30.720
 * 3°/sec    : 46.080
 *  
 *  
 *  methods: avg 5 readings
 *  
 *  or
 *  
 *  https://robotics.stackexchange.com/questions/16698/noisy-magnetometer-data
 *  prima risposta
 *  
 *  lag filter :  filterValue = alpha*newReading + (1-alpha)*filterValue)
 *                suggested alpha = 0.05
 *                
 *  DRIFT
 *  https://electronics.stackexchange.com/questions/477794/magnetometer-offset-drift
 *************/

 
#include <Wire.h>
#include <Adafruit_LIS3MDL.h>
#include <SimpleKalmanFilter.h>

#define MAG_DATARATE_STRING LIS3MDL_DATARATE_155_HZ  // could be faster but not Ultra High Performance Mode
#define MAGNET_DATARATE_LIS3MDL 155

// this magnetometer is rather noisy. Hence a kalman smooth also on the raw readings.
#define K_ERR_M_LIS 1.0 //0.08   // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_Q_M_LIS   0.01 //0.01   // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed

// SimpleKalmanFilter kf_mx = SimpleKalmanFilter(K_ERR_M_L, K_ERR_M_L, K_Q_M_L);
// SimpleKalmanFilter kf_my = SimpleKalmanFilter(K_ERR_M_L, K_ERR_M_L, K_Q_M_L);
// SimpleKalmanFilter kf_mz = SimpleKalmanFilter(K_ERR_M_L, K_ERR_M_L, K_Q_M_L);

Adafruit_LIS3MDL magnet_LIS3MDL;

bool init_magnetometer_LIS3MDL() {
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
  return true;
}

/*******
 * Mandatory NED convention
 * X axis pointing forward
 * Y axis pointing right
 * Z axis pointing down
********/
void mag_axes_LIS(float raw[3]) {
  float north = raw[1];   // right direction becomes front
  float east =  raw[0];   // front direction becomes left
  float down = -raw[2];   // down direction

  raw[0] = north; // -x;
  raw[1] = east;  // y;
  raw[2] = down;  // -z;
}

void mag_readings_LIS(float mag_raw[3]) {
   sensors_event_t mevent;
   magnet_LIS3MDL.getEvent(&mevent);
  //  mag_raw[0] = kf_mx.updateEstimate(mevent.magnetic.x);  // gauss
  //  mag_raw[1] = kf_my.updateEstimate(mevent.magnetic.y);  // gauss
  //  mag_raw[2] = kf_mz.updateEstimate(mevent.magnetic.z);  // gauss
   mag_raw[0] = mevent.magnetic.x;  // gauss
   mag_raw[1] = mevent.magnetic.y;  // gauss
   mag_raw[2] = mevent.magnetic.z;  // gauss

   mag_axes_LIS(mag_raw);
}
