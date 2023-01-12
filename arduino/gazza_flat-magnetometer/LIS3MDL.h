/******
 * Initialization and reading
 * of ST LIS3MDL magnetometer Adafruit breakout board
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/
 
#include <Wire.h>
#include <Adafruit_LIS3MDL.h>

#define MAG_DATARATE_STRING LIS3MDL_DATARATE_155_HZ  // this is the top precision setting according to adafruit
#define MAG_DATARATE_LIS3MDL 155


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
  magnetometer.configInterrupt(true, true, true,      // benefit of turning interrupts on or off ?
                               true, // polarity      // ???
                               false, // don't latch  // ??? benefit of latching ?
                               true); // enabled!
  return true;
}

//// https://robotics.stackexchange.com/questions/16698/noisy-magnetometer-data
////   Regarding techniques to mitigate, you can do anything from a simple lag filter (choose an alpha value like 0.05 and
////   then do filterValue = alpha*newReading + (1-alpha)*filterValue) to a moving average like you mentioned,
////   to a Kalman filter, to the Madgwick filter.
////
//// we prefer to kalman smooth the azimuth reading rather than the magnetometer raw reading

// rotates the sensor axes to ensure they are properly oriented
// x forward
// y right
// z up
// real orientation depends on how the sensor is mounted on the telescope
void mag_axes(float raw[3]) {
  float x = raw[0];
  float y = raw[1];
  float z = raw[2];
  raw[0] = -y;
  raw[1] = z;
  raw[2] = x;
}

void mag_readings_LIS(float mag_raw[3]) {
    sensors_event_t mevent;
  
    magnetometer.getEvent(&mevent);
    mag_raw[0] = mevent.magnetic.x;
    mag_raw[1] = mevent.magnetic.y;
    mag_raw[2] = mevent.magnetic.z;
    mag_axes(mag_raw);
}
