/******
 * Calculates azimuth and altitude from sensors readings
 * Azimuth is smoothed by a kalman filter
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/
 
/* Conversion constants */
#include <SimpleKalmanFilter.h>
#include "defines.h"

float K_ERR_AZ = 0.01;   // expected magnitude of error - small, readings are already smoothed 
float K_Q_AZ = 1.0; //1.5;      // meaning: how much we expect the measurement to vary. Quite small, fastest rotation at 3°/sec when calibrating. Generally below 1"/sec
float K_ERR_ALT = 0.01;  // expected magnitude of error - small, the sensor is not particularly noisy
float K_Q_ALT = 1.0;     // meaning: how much we expect the measurement to vary. Even smaller than azimuth, the scope rotates at 1°/sec when slewing. Generally below 1"/sec

SimpleKalmanFilter kf_azimuth = SimpleKalmanFilter(K_ERR_AZ, K_ERR_AZ, K_Q_AZ);  // TODO NON POSSO USARE KF per l'Azimuth perchè da 359 salta a 0
SimpleKalmanFilter kf_altitude = SimpleKalmanFilter(K_ERR_ALT, K_ERR_ALT, K_Q_ALT);

float latest_yaw = 0;

float norm_2PI(float angle);

void vector_cross(float a[3], float b[3], float out[3]) {
  out[0] = a[1] * b[2] - a[2] * b[1];
  out[1] = a[2] * b[0] - a[0] * b[2];
  out[2] = a[0] * b[1] - a[1] * b[0];
}

float vector_dot(float a[3], float b[3]) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void vector_normalize(float a[3]) {
  float mag = sqrt(vector_dot(a, a));
  a[0] /= mag;
  a[1] /= mag;
  a[2] /= mag;
}

// https://github.com/jremington/AltIMU-AHRS/blob/master/altimu10v3_tiltcomp/altimu10v3_tiltcomp.ino
// brilliant method that replaces lots of trig functions with simple vector products
// however might be inaccurate because we can't do 3D calibration and Z-axis is only removed the
// hard iron but not the soft iron. Same problem also with the trigonometric method.
//
// For this issue to be negligible, the tilt must be small, few degrees.
// Z is multiplied times sin(tilt), sin(6°) = 0.1 sin(10°) = 0.17, this would be the weight of Z axis soft iron
// if it is significant. Bigger angles than this, if soft iron is significant, the error might become apparent
//
// The measeured is sent to the UI which will post a warning to level up if necessary
//
float flatCompass(float acc[3], float mag[3], uint8_t debug) {
    float E[3], N[3]; //direction vectors
    float P[] = {1, 0, 0};      // X ahead, to North
    vector_cross(acc, mag, E);  // cross "down" (acceleration vector) with magnetic vector (magnetic north + inclination) with  to produce "east"
    vector_normalize(E);
    vector_cross(E, acc, N);    // cross "east" with "down" to produce "north" (parallel to the ground)
    vector_normalize(N);

    float yaw = atan2(vector_dot(E, P), vector_dot(N, P));
    if (yaw > 2*M_PI) yaw -= 2*M_PI;
    if (yaw < 0.0)  yaw += 2*M_PI;
    if((uint8_t)(debug | ~DEBUG_AZ)==255) {
        Serial.print("debug Az,");Serial.print(yaw*RadToDeg);
    }
    yaw = kf_azimuth.updateEstimate(yaw);
    if((uint8_t)(debug | ~DEBUG_AZ)==255) {
        Serial.print(",KAZ,");Serial.println(yaw*RadToDeg);
    }
    return yaw * RadToDeg;
}

// Trigonometric method. Works very well but is full of trigonometrics functions
// which may introduce too much rounding error
//
//float flatCompass(float acc[3], float mag[3], uint8_t debug) {
//  float X = mag[0];
//  float Y = mag[1];
//  float Z = mag[2];
//  float Pitch = atan2((-acc[0]) , sqrt(acc[1] * acc[1] + acc[2] * acc[2]));
//  float Roll = atan2(acc[1], acc[2]);
//  float Xh = X * cos(Pitch) + Z * sin(Pitch);
//  float Yh = X * sin(Roll) * sin(Pitch) + Y * cos(Roll) - Z * sin(Roll) * cos(Pitch);
//  
//  float yaw = norm_2PI(atan2(Yh, -Xh) - M_PI);  // subtracting Pi because we want north 0° and south 180°
//  if((uint8_t)(debug | ~DEBUG_AZ)==255) {
//      Serial.print("debug Az,");Serial.print(yaw*RadToDeg);
//  }
//
//  yaw = kf_azimuth.updateEstimate(yaw);
//  latest_yaw = yaw;
//  if((uint8_t)(debug | ~DEBUG_AZ)==255) {
//    Serial.print(",KAZ,");Serial.println(yaw*RadToDeg);
//  }
//  return yaw*RadToDeg;
//}

float elevation(float Gx, float Gz, uint8_t debug) {
  float pitch = atan2(Gx, Gz);
  if((uint8_t)(debug | ~DEBUG_ALT)==255) {
      Serial.print("debug alt Alt,");Serial.print(pitch*RadToDeg);
  }
  pitch = kf_altitude.updateEstimate(pitch);
  if((uint8_t)(debug | ~DEBUG_ALT)==255) {
      Serial.print(",KAlt,");Serial.println(pitch*RadToDeg);
  }
  return pitch*RadToDeg;
}

float norm_2PI(float angle) {
  float PI_2 = 2*PI;
  return fmod(PI_2 + fmod(angle, PI_2), PI_2);
}
