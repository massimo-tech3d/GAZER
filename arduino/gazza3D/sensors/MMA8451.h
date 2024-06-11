/******
 * Initialization and reading
 * of NXP MMA8451 accelerometer Adafruit breakout board
 * 
 * This accelerometer is used to untilt the compass
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/

#include <Adafruit_MMA8451.h>

#define K_ERR_MMA 0.01  // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_Q_MMA   0.05  // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed

#define SENSORS_GRAVITY_EARTH    (9.80665F)  //< Earth's gravity in m/s^2

// #define ACCEL_DATARATE_MMA 200
// #define ACC_DATARATE_MMA MMA8451_DATARATE_200_HZ
#define ACCEL_DATARATE_MMA 500  // should not be faster than the datarate e.g. 500 vs 800
#define ACC_DATARATE_MMA MMA8451_DATARATE_800_HZ
// possible values MMA8451_DATARATE_800_HZ MMA8451_DATARATE_400_HZ MMA8451_DATARATE_200_HZ MMA8451_DATARATE_100_HZ
//                 MMA8451_DATARATE_50_HZ MMA8451_DATARATE_12_5_HZ MMA8451_DATARATE_6_25HZ MMA8451_DATARATE_1_56_HZ
int CALIB_OFFSET_SAMPLES = 100;  // adjust according to ACCEL_DATARATE_MMA -- sample for 5 seconds 12.5*5 = 62.5
double ARMS_MMA8451 = 0.01;  // 0.001;

// #define K_ERR_ALT_MMA8451 0.01   // kalman filter error estimate - sensor specific
// #define K_Q_ALT_MMA8451    0.5   // kalman filter process variance - sensor specific

float accXoffset = 0;
float accYoffset = 0;
float accZoffset = 0;

Adafruit_MMA8451 accel_MMA;  //  = Adafruit_MMA8451(int32_t id = -1);


// The MMA8452 sensor returns the most accurate and noiseless data when
// used at the slowest datarate. The slowest would be 3.125 Hz, but that's
// too slow for the main loop --> ACC_DATARATE_MMA defined 12.5 Hz
bool init_accelerometer_MMA(void) {
  accel_MMA = Adafruit_MMA8451();

  if (!accel_MMA.begin()) {
    return false;
  }
  accel_MMA.setRange(MMA8451_RANGE_2_G);
  accel_MMA.setDataRate(ACC_DATARATE_MMA);     // slowest possible data rate for stable ACC readings is 3.125 but too slow for the loop
  return true;
}

/*******
 * Mandatory NED convention
 * X axis pointing forward
 * Y axis pointing right
 * Z axis pointing down
********/
void axes_MMA(float raw[3]) {
  float north =  raw[0];  // front direction
  float east  = -raw[1];  // right direction
  float down  = -raw[2];  // down direction

  raw[0] = north; // -x;
  raw[1] = east;  // y;
  raw[2] = down;  // -z;
}

/**************************************************************
 *                        IMPORTANT
 *       The untilter accelerometer MUST return readings
 *  expressed in gravity units, not in m/s^2 or whatever else
 * 
 *         Compliance to NED convention is necessary
 **************************************************************/
void accel_readings_MMA(float accel_raw[3]) {
  sensors_event_t aevent;
  accel_MMA.getEvent(&aevent);
  accel_raw[0] = aevent.acceleration.x / SENSORS_GRAVITY_EARTH - accXoffset;
  accel_raw[1] = aevent.acceleration.y / SENSORS_GRAVITY_EARTH + accYoffset;  // why + ? Mistery... just works
  accel_raw[2] = aevent.acceleration.z / SENSORS_GRAVITY_EARTH - accZoffset;
  axes_MMA(accel_raw);
}

void zero_MMA() {
    Serial.print("Zeroing MMA");
    accXoffset = 0;
    accYoffset = 0;
    accZoffset = 0;
    float ag[3] = {0,0,0};
    float uraw[3];
    for(int i = 0; i < 3; i++){                 // the first readings after a cold start are always way off let's just skip them
      accel_readings_MMA(uraw);                 // don't rotate axes in this phase
      delay(1000/ACCEL_DATARATE_MMA);           // wait a little bit between 2 measurements
    }

    for(int i = 0; i < CALIB_OFFSET_SAMPLES; i++) {
      accel_readings_MMA(uraw);
      ag[0] += uraw[0];
      ag[1] += uraw[1];
      ag[2] += (uraw[2]-1.0);
      delay(1000/ACCEL_DATARATE_MMA); // wait a little bit between 2 measurements
    }

    accXoffset = ag[0] / CALIB_OFFSET_SAMPLES;
    accYoffset = ag[1] / CALIB_OFFSET_SAMPLES;
    accZoffset = ag[2] / CALIB_OFFSET_SAMPLES;
    Serial.print("Number of samples ");Serial.print(CALIB_OFFSET_SAMPLES);Serial.print(" ag[2] ");Serial.println(ag[2]);
    Serial.print("XO: ");Serial.print(accXoffset,4);Serial.print(" YO: ");Serial.print(accYoffset,4);Serial.print(" ZO: ");Serial.println(accZoffset,4);
}