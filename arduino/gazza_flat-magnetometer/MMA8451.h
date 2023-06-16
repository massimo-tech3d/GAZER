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

#include <Adafruit_MMA8451.h>
#include "SimpleKalmanFilter.h"

#define K_ERR_A 0.01  // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_Q_A   0.05  // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_ERR_M 0.08  // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_Q_M   0.01  // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed

#define ACCEL  0

#define SENSORS_GRAVITY_EARTH    (9.80665F)  //< Earth's gravity in m/s^2
#define SENSORS_DPS_TO_RADS  (0.017453293F)  //< Degrees/s to rad/s multiplier
#define SENSORS_RADS_TO_DPS  (57.29577793F)  //< Rad/s to degrees/s  multiplier

#define ACCEL_DATARATE_MMA 12.5
//#define ACCEL_DATARATE_MMA 50
#define ACC_DATARATE_MMA MMA8451_DATARATE_12_5_HZ  // adafruit dice essere il top per la precisione...
//#define ACC_DATARATE_MMA MMA8451_DATARATE_50_HZ  // adafruit dice essere il top per la precisione...
// possible values MMA8451_DATARATE_800_HZ MMA8451_DATARATE_400_HZ MMA8451_DATARATE_200_HZ MMA8451_DATARATE_100_HZ
//                 MMA8451_DATARATE_50_HZ MMA8451_DATARATE_12_5_HZ MMA8451_DATARATE_6_25HZ MMA8451_DATARATE_1_56_HZ

Adafruit_MMA8451 accelerometer_MMA;  //  = Adafruit_MMA8451(int32_t id = -1);


bool init_accelerometer_MMA(void) {
//  int32_t id = 1;
//  accelerometer_MMA = Adafruit_MMA8451(id);
  accelerometer_MMA = Adafruit_MMA8451();

  if (!accelerometer_MMA.begin()) {
    return false;
  }
  accelerometer_MMA.setRange(MMA8451_RANGE_2_G);
  accelerometer_MMA.setDataRate(ACC_DATARATE_MMA);         // slowest possible data rate for stable ACC readings is 3.125 but too slow for the loop
  return true;
}

void axes_MMA(float raw[3], bool flat) {
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

void accel_readings_MMA(float accel_raw[3], bool flat=false) {
  sensors_event_t aevent;

  accelerometer_MMA.getEvent(&aevent);
  accel_raw[0] = aevent.acceleration.x;
  accel_raw[1] = aevent.acceleration.y;
  accel_raw[2] = aevent.acceleration.z;

  axes_MMA(accel_raw, flat);
}
