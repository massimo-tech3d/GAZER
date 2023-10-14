/******
 * Calculates azimuth and altitude from sensors readings
 * Azimuth is smoothed by a kalman filter
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/
 
/* Conversion constants */
#define DegToRad 0.017453292F
#define RadToDeg 57.295779F
#include "SimpleKalmanFilter.h"

//#define K_ERR_AZ 0.05   // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed - sensor specific
//#define K_Q_AZ   0.1   // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed - sensor specific

#define K_ERR_AZ 0.1   // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed - sensor specific
#define K_Q_AZ   1.5   // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed - sensor specific

#define K_ERR_ALT 0.1   // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed - sensor specific
#define K_Q_ALT    2.0   // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed - sensor specific

SimpleKalmanFilter kf_azimuth = SimpleKalmanFilter(K_ERR_AZ, K_ERR_AZ, K_Q_AZ);
SimpleKalmanFilter kf_altitude = SimpleKalmanFilter(K_ERR_ALT, K_ERR_ALT, K_Q_ALT);

float norm_2PI(float angle);

float flatCompass(float Bx, float By, float Bz) {
    float yaw = norm_2PI(atan2(By, Bx));
//dbg    Serial.print("Az,");Serial.print(yaw*RadToDeg);
    yaw = kf_azimuth.updateEstimate(yaw);
//dbg    Serial.print(", kAz,");Serial.println(yaw*RadToDeg);
    return yaw*RadToDeg;
}

float elevation(float Gx, float Gz) {
  float pitch = atan2(Gx, Gz);
//dbg  Serial.print("Alt,");Serial.print(pitch*RadToDeg);
  pitch = kf_altitude.updateEstimate(pitch);
//dbg  Serial.print(", kAlt,");Serial.println(pitch*RadToDeg);
  if(isnan(pitch)){
    Serial.print("Alt is NAN - Ax ");Serial.print(Gx);Serial.print( " Az ");Serial.print(Gz);
  }
  return pitch*RadToDeg;
}

float norm_2PI(float angle) {
  float PI_2 = 2*PI;
  return fmod(PI_2 + fmod(angle, PI_2), PI_2);
}
