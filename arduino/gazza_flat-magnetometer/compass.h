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

SimpleKalmanFilter kf_azimuth = SimpleKalmanFilter(1, 1, 0.1);  // best parameters to be fine tuned

/*
 * for some reason the kalman filter may not be used for acceletometer readings (altitude)
 * it simply does not work. It adds some 5/6° to 0° altitude and reduces by even 10/15° the 90° altitude.
 * Don't know why, but this is it
 */
//SimpleKalmanFilter kf_altitude = SimpleKalmanFilter(10, 1, 0.001);  // best parameters to be fine tuned

/*
 * The values Bx and By are tilt compensated after the execution
 * Bz is just ignored because the scope cannot Roll, any Bz value other than 0 is just noise
 */
void flatCompass(float Bx, float By, float Bz, float &yaw) {
    yaw = atan2(By, Bx) * RadToDeg + 180;
    yaw = kf_azimuth.updateEstimate(yaw);
}

void elevation(float Gx, float Gz, float &pitch) {
//  pitch = kf_azimuth.updateEstimate(atan2(Gx, Gz) * RadToDeg);
  pitch = atan2(Gx, Gz) * RadToDeg;
}
