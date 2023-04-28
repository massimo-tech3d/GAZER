/******
 * Initialization and reading
 * of NXP FXOS_8700 combined magnetometer/accelerometer Adafruit breakout board
 * 
 * NOTE: the same breakout board also includes an NXP Gyroscope which is not use
 *       in this application
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/

#include <Adafruit_FXOS8700.h>
#include "SimpleKalmanFilter.h"

#define K_ERR_A 0.01  // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_Q_A   0.05  // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_ERR_M 0.08  // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_Q_M   0.01  // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed

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

#define ACC_DATARATE ODR_12_5HZ  // adafruit dice essere il top per la precisione...
#define COMBO_DATARATE ODR_12_5HZ
// possible values ODR_6_25HZ ODR_12_5HZ 200HZ ODR_25HZ ODR_50HZ ODR_100HZ ODR_200HZ ODR_400HZ

#define ACCEL_DATARATE_FXOS_SINGLE 12.5
#define ACCEL_DATARATE_FXOS_COMBO  12.5
#define MAG_DATARATE_FXOS_COMBO  200

Adafruit_FXOS8700 fxos;           //  = Adafruit_FXOS8700(0x8700A, 0x8700B);
Adafruit_FXOS8700 accelerometer;  //  = Adafruit_FXOS8700(0x8700A, -1);

SimpleKalmanFilter kf_ax = SimpleKalmanFilter(K_ERR_A, K_ERR_A, K_Q_A);
SimpleKalmanFilter kf_ay = SimpleKalmanFilter(K_ERR_A, K_ERR_A, K_Q_A);
SimpleKalmanFilter kf_az = SimpleKalmanFilter(K_ERR_A, K_ERR_A, K_Q_A);

SimpleKalmanFilter kf_mx = SimpleKalmanFilter(K_ERR_M, K_ERR_M, K_Q_M);
SimpleKalmanFilter kf_my = SimpleKalmanFilter(K_ERR_M, K_ERR_M, K_Q_M);
SimpleKalmanFilter kf_mz = SimpleKalmanFilter(K_ERR_M, K_ERR_M, K_Q_M);

bool init_sensors(void) {
  fxos = Adafruit_FXOS8700(0x8700A, 0x8700B);
  if (!fxos.begin()) {
    return false;
  }
  fxos.setSensorMode(HYBRID_MODE);
  fxos.setAccelRange(ACCEL_RANGE_2G);
  fxos.setMagOversamplingRatio(MAG_OSR_7);
  fxos.setOutputDataRate(COMBO_DATARATE);  // 400HZ
  return true; 
}

bool init_accelerometrer(void) {
  accelerometer = Adafruit_FXOS8700(0x8700A, -1);
  if (!accelerometer.begin()) {
    return false;
  }
  accelerometer.setSensorMode(ACCEL_ONLY_MODE);
  accelerometer.setAccelRange(ACCEL_RANGE_2G);
  accelerometer.setOutputDataRate(ACC_DATARATE);         // slowest possible data rate for stable ACC readings is 3.125 but too slow for the loop
  return true;
}

void axes(float raw[3], bool flat) {
  float x = raw[0];  // front direction
  float y = raw[1];  // left direction
  float z = raw[2];  // up direction

  if(flat){
    // sensor horizontal/flat
    raw[0] = x;
    raw[1] = y;
    raw[2] = z;
  } else {
    // sensor vertical
    raw[0] = x;
    raw[1] = -z;
    raw[2] = y;
  }
}

void accel_readings(bool combo, float accel_raw[3], bool flat=false) {
  sensors_event_t aevent, mevent;

  if(combo){
    fxos.getEvent(&aevent, &mevent);
  } else {
    accelerometer.getEvent(&aevent, &mevent);
  }
  accel_raw[0] = aevent.acceleration.x;
  accel_raw[1] = aevent.acceleration.y;
  accel_raw[2] = aevent.acceleration.z;

  axes(accel_raw, flat);
}

void mag_readings_FX(float mag_raw[3], bool flat=false) {
  sensors_event_t aevent, mevent;

  fxos.getEvent(&aevent, &mevent);
  mag_raw[0] = mevent.magnetic.x;
  mag_raw[1] = mevent.magnetic.y;
  mag_raw[2] = mevent.magnetic.z;

  axes(mag_raw, flat); // no need to swap axes (perchè ho messo questa nota ?
}
