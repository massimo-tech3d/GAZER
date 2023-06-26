/******
 * Initialization and reading
 * of ST LIS3MDL magnetometer Adafruit breakout board
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/
 
#include <Wire.h>
#include <Adafruit_LIS3MDL.h>
#include "SimpleKalmanFilter.h"

#define MAG_DATARATE_STRING LIS3MDL_DATARATE_155_HZ  // adafruit dice essere il top per la precisione...
#define MAG_DATARATE_LIS3MDL 155

#define K_ERR_M 0.08   // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_Q_M   0.01   // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed

SimpleKalmanFilter kf_x = SimpleKalmanFilter(K_ERR_M, K_ERR_M, K_Q_M);
SimpleKalmanFilter kf_y = SimpleKalmanFilter(K_ERR_M, K_ERR_M, K_Q_M);
SimpleKalmanFilter kf_z = SimpleKalmanFilter(K_ERR_M, K_ERR_M, K_Q_M);

Adafruit_LIS3MDL magnetometer;

bool init_magnetometer(void) {
  magnetometer = Adafruit_LIS3MDL();
  if (!magnetometer.begin_I2C()) {
    return false;
  }
  magnetometer.setDataRate(MAG_DATARATE_STRING);
  magnetometer.setPerformanceMode(LIS3MDL_ULTRAHIGHMODE);
  magnetometer.setOperationMode(LIS3MDL_CONTINUOUSMODE);
  magnetometer.setRange(LIS3MDL_RANGE_4_GAUSS);
  magnetometer.setIntThreshold(500);
  magnetometer.configInterrupt(true, true, true,
                               true,
                               false, // don't latch  // ??? benefit of latching ?
                               true);
  return true;
}

void mag_axes(float raw[3], bool flat=true) {
  float x = raw[0];  // right direction
  float y = raw[1];  // front direction
  float z = raw[2];  // up direction

  if(flat){
    // sensor horizontal/flat
    raw[0] = y;
    raw[1] = -x;
    raw[2] = z;
  } else {
    // sensor vertical
    raw[0] = y;  // front is y
    raw[1] = -z; // left is z
    raw[2] = -x; // up is -x
  }
}

void mag_readings_LIS(float mag_raw[3]) {
    sensors_event_t mevent;
  
    magnetometer.getEvent(&mevent);
    mag_raw[0] = kf_x.updateEstimate(mevent.magnetic.x);
    mag_raw[1] = kf_y.updateEstimate(mevent.magnetic.y);
    mag_raw[2] = kf_z.updateEstimate(mevent.magnetic.z);
//    mag_raw[0] = mevent.magnetic.x;
//    mag_raw[1] = mevent.magnetic.y;
//    mag_raw[2] = mevent.magnetic.z;
//    Serial.print("X,");Serial.print(mevent.magnetic.x);Serial.print(", Y,");Serial.print(mevent.magnetic.y);Serial.print(", Z,");Serial.print(mevent.magnetic.z);
//    Serial.print(", kX,");Serial.print(mag_raw[0]);Serial.print(", kY,");Serial.print(mag_raw[1]);Serial.print(", kZ,");Serial.println(mag_raw[2]);
    mag_axes(mag_raw);

}
