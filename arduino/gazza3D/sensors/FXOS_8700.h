/******
 * Initialization and reading
 * of NXP FXOS8700 combined magnetometer accelerometer Adafruit breakout board
 * 
 * NOTE: the same breakout board also includes an NXP Gyroscope which is not use
 *       in this application
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/

#include <Adafruit_FXOS8700.h>
#include <SimpleKalmanFilter.h>
//#include "SimpleKalmanFilter.h"

#define K_ERR_A_F 0.01  // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_Q_A_F   0.05  // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_ERR_M_F 0.08  // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_Q_M_F   0.01  // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed

#define ACCEL  0
#define MAGNET 2

#define SENSORS_GRAVITY_EARTH    (9.80665F)  //< Earth's gravity in m/s^2
#define SENSORS_MAGFIELD_EARTH_MAX  (60.0F)  //< Maximum magnetic field on Earth's surface
#define SENSORS_MAGFIELD_EARTH_MIN  (30.0F)  //< Minimum magnetic field on Earth's surface
#define SENSORS_DPS_TO_RADS  (0.017453293F)  //< Degrees/s to rad/s multiplier
#define SENSORS_RADS_TO_DPS  (57.29577793F)  //< Rad/s to degrees/s  multiplier
#define SENSORS_GAUSS_TO_MICROTESLA   (100)  //< Gauss to micro-Tesla multiplier

#define FXOS8700_COUNTSPERG 16384  // +/-2 g range on accelerometer

//#define ACCEL_MG_LSB_2G (0.000244F)  // Macro for mg per LSB at +/- 2g sensitivity (1 LSB = 0.000244mg)
//#define MAG_UT_LSB (0.1F)            // Macro for micro tesla (uT) per LSB (1 LSB = 0.1uT)
//#define AF_ACCEL_MG_LSB_2G (0.000244F)  // Adafruit approximated number
//#define ACCURACY_RATIO AF_ACCEL_MG_LSB_2G/ACCEL_MG_LSB_2G

#define ACC_DATARATE_FXOS ODR_12_5HZ  // adafruit dice essere il top per la precisione...
#define MAG_DATARATE_FXOS ODR_400HZ  // adafruit dice essere il top per la precisione...

// possible values ODR_6_25HZ ODR_12_5HZ 200HZ ODR_25HZ ODR_50HZ ODR_100HZ ODR_200HZ ODR_400HZ

#define ACCEL_DATARATE_FXOS 12.5
#define MAGNET_DATARATE_FXOS  100

Adafruit_FXOS8700 accel_FXOS;  //  = Adafruit_FXOS8700(0x8700A, -1);
Adafruit_FXOS8700 magnet_FXOS;  //  = Adafruit_FXOS8700(-1, 0x8700B);

SimpleKalmanFilter kf_f_ax = SimpleKalmanFilter(K_ERR_A_F, K_ERR_A_F, K_Q_A_F);
SimpleKalmanFilter kf_f_ay = SimpleKalmanFilter(K_ERR_A_F, K_ERR_A_F, K_Q_A_F);
SimpleKalmanFilter kf_f_az = SimpleKalmanFilter(K_ERR_A_F, K_ERR_A_F, K_Q_A_F);

SimpleKalmanFilter kf_f_mx = SimpleKalmanFilter(K_ERR_M_F, K_ERR_M_F, K_Q_M_F);
SimpleKalmanFilter kf_f_my = SimpleKalmanFilter(K_ERR_M_F, K_ERR_M_F, K_Q_M_F);
SimpleKalmanFilter kf_f_mz = SimpleKalmanFilter(K_ERR_M_F, K_ERR_M_F, K_Q_M_F);


bool init_magnetometer_FXOS(void) {
//  magnet_FXOS = Adafruit_FXOS8700(-1, 0x8700B);
  magnet_FXOS = Adafruit_FXOS8700(0x8700A, 0x8700B);
  if (!magnet_FXOS.begin()) {
    return false;
  }
  //magnet_FXOS.setSensorMode(MAG_ONLY_MODE);
  magnet_FXOS.setOutputDataRate(MAG_DATARATE_FXOS);
  //magnet_FXOS.setMagOversamplingRatio(MAG_OSR_7);
  return true;
}

void update_tilt_FXOS(void) {
    // TO BE IMPLEMENTED
}

bool init_accelerometer_FXOS(void) {
  accel_FXOS = Adafruit_FXOS8700(0x8700A, -1);
  //  if (!accel_FXOS->begin(0x1F, Wire1)) {
  if (!accel_FXOS.begin()) {
    return false;
  }
  accel_FXOS.setSensorMode(ACCEL_ONLY_MODE);
  accel_FXOS.setAccelRange(ACCEL_RANGE_2G);
  accel_FXOS.setOutputDataRate(ACC_DATARATE_FXOS);         // slowest possible data rate for stable ACC readings is 3.125 but too slow for the loop
//  accelerometer.setLPFilter(true);
/*  set HP_FILTER_CUTOFF register pulse_lpf_en bit to 1
    è il bit 4 (pag 56 data sheet)
*/
  return true;
}

void axes_FXOS(float raw[3]) {
  float x = raw[0];  // front direction
  float y = raw[1];  // left direction
  float z = raw[2];  // up direction

  raw[0] = x;
  raw[1] = y;
  raw[2] = z;
}

void accel_readings_FXOS(float accel_raw[3]) {
  sensors_event_t aevent;

  accel_FXOS.getEvent(&aevent);
  accel_raw[0] = aevent.acceleration.x;
  accel_raw[1] = aevent.acceleration.y;
  accel_raw[2] = aevent.acceleration.z;

  axes_FXOS(accel_raw);
}

void mag_readings_avg_FXOS(float mag_raw[3]) {
    sensors_event_t aevent, mevent;
    int iter = 5;
    float x = 0, y = 0, z = 0;

    for(int i=0; i<iter; i++){
      magnet_FXOS.getEvent(&aevent, &mevent);
      x += mevent.magnetic.x;
      y += mevent.magnetic.y;
      z += mevent.magnetic.z;
      if(i != iter-1)
        delay(1000/MAGNET_DATARATE_FXOS);
    }
    x = x/iter;
    y = y/iter;
    z = z/iter;
  
    mag_raw[0] = kf_f_mx.updateEstimate(x);
    mag_raw[1] = kf_f_my.updateEstimate(y);
    mag_raw[2] = kf_f_mz.updateEstimate(z);
    // mag_axes(mag_raw);
}

void mag_readings_FXOS(float mag_raw[3]) {
    mag_readings_avg_FXOS(mag_raw);

    axes_FXOS(mag_raw); // no need to swap axes (perchè ho messo questa nota ?
}
