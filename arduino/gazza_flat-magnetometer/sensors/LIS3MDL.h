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
#include "../SimpleKalmanFilter.h"

#define MAG_DATARATE_STRING LIS3MDL_DATARATE_155_HZ  // adafruit dice essere il top per la precisione...
#define MAGNET_DATARATE_LIS3MDL 155

#define K_ERR_M_L 0.08   // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_Q_M_L   0.01   // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed

SimpleKalmanFilter kf_l_mx = SimpleKalmanFilter(K_ERR_M_L, K_ERR_M_L, K_Q_M_L);
SimpleKalmanFilter kf_l_my = SimpleKalmanFilter(K_ERR_M_L, K_ERR_M_L, K_Q_M_L);
SimpleKalmanFilter kf_l_mz = SimpleKalmanFilter(K_ERR_M_L, K_ERR_M_L, K_Q_M_L);

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

void mag_axes_LIS(float raw[3], bool flat=true) {
  float x = raw[0];  // right direction
  float y = raw[1];  // front direction
  float z = raw[2];  // up direction

  if(flat){
    // sensor horizontal/flat
    raw[0] = y;  // front
    raw[1] = -x; // left
    raw[2] = z;  // up
  } else {
    // sensor vertical
    raw[0] = y;  // front is y
    raw[1] = -z; // left is z
    raw[2] = -x; // up is -x
  }
}

void mag_readings_avg_LIS(float mag_raw[3]) {
    sensors_event_t mevent;
    int iter = 5;
    float x = 0, y = 0, z = 0;

    for(int i=0; i<iter; i++){
      magnet_LIS3MDL.getEvent(&mevent);
      x += mevent.magnetic.x;
      y += mevent.magnetic.y;
      z += mevent.magnetic.z;
      if(i != iter-1)
        delay(1000/MAGNET_DATARATE_LIS3MDL);
    }
    x = x/iter;
    y = y/iter;
    z = z/iter;
  
    mag_raw[0] = kf_l_mx.updateEstimate(x);
    mag_raw[1] = kf_l_my.updateEstimate(y);
    mag_raw[2] = kf_l_mz.updateEstimate(z);
    // mag_axes(mag_raw);
}

void mag_readings_LIS(float mag_raw[3], bool flat=true) {
//    sensors_event_t mevent;
//    magnet_LIS3MDL.getEvent(&mevent);
//    mag_raw[0] = kf_lx.updateEstimate(mevent.magnetic.x);
//    mag_raw[1] = kf_ly.updateEstimate(mevent.magnetic.y);
//    mag_raw[2] = kf_lz.updateEstimate(mevent.magnetic.z);
/* toggle comments 5 lines above OR line below to toggle average readings */
    mag_readings_avg_LIS(mag_raw);

    mag_axes_LIS(mag_raw, flat);
}
