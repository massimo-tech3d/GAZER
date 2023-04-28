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

#define K_ERR_AZ 0.05   // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed - sensor specific
#define K_Q_AZ   0.1   // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed - sensor specific

#define K_ERR_ALT 0.1   // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed - sensor specific
#define K_Q_ALT    2.0   // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed - sensor specific

SimpleKalmanFilter kf_azimuth = SimpleKalmanFilter(K_ERR_AZ, K_ERR_AZ, K_Q_AZ);
SimpleKalmanFilter kf_altitude = SimpleKalmanFilter(K_ERR_ALT, K_ERR_ALT, K_Q_ALT);

float norm_2PI(float angle);

void flatCompass(float Bx, float By, float Bz, float &yaw) {
    yaw = norm_2PI(atan2(By, Bx));
    yaw = kf_azimuth.updateEstimate(yaw);
}


float elevation(float Gx, float Gz) {
  float pitch = atan2(Gx, Gz);
  pitch = kf_altitude.updateEstimate(pitch);
  return pitch;
}

float norm_2PI(float angle) {
  float PI_2 = 2*PI;
  return fmod(PI_2 + fmod(angle, PI_2), PI_2);
}
