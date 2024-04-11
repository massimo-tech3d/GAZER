/******
 * Initialization and reading
 * of PNI RM3100 magnetometer WitMotion breakout board
 *
 * The RM3100 includes both I2C and SPI interfaces, however
 * the WitMotion board exports the SPI interface only.
 * 
 * The SPI interface requires 5 wires, which makes the cabling a lot more complicated
 * therefore we use a SPI to I2C bridge, which make this module a lot more complex than it
 * should be.
 *
 * I'm considering to add a SPI interface to the main GAZER board and move to the 5 wires cable
 *
 * Created by Massimo Tasso, July, 13, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 * based on https://github.com/hnguy169/RM3100-Arduino
 *
 * 
 * Since GAZER does not use SPI interfaces, the RM3100 is bridged to I2C interface
 * via an NXP SC18IS602B I2C to SPI bridge
 * 
 * NOTE: the WitMotion BB leaves a lot to be desired and it's not reccommended
 *       later in the development phase, PNI has released their own breakout board
 *       which exports also the i2c interface. I'm sure this is worth using because the
 *       sensor is by far the smoothest and with a very low noise level, that much that
 *       the kalman smoothing is entirely optional (even if we use it)
 * 
 *       i2c module for this sensor can be found on github
 ******/

#include <Arduino.h>
#include <SimpleKalmanFilter.h>
//#include "SimpleKalmanFilter.h"

// SC18IS601B bridge values
#define BRIDGE_SPICLK_1843_kHz 0B00  // 1.8 MBit/s -- too fast for RM3100
#define BRIDGE_SPICLK_461_kHz  0B01  // 461 kbit/s
#define BRIDGE_SPICLK_115_kHz  0B10  // 115 kbit/s
#define BRIDGE_SPICLK_58_kHz   0B11  // 58 kbit/s
#define BRIDGE_SPIMODE_0       0B00  // CPOL: 0  CPHA: 0
#define BRIDGE_SPIMODE_1       0B01  // CPOL: 0  CPHA: 1
#define BRIDGE_SPIMODE_2       0B10  // CPOL: 1  CPHA: 0
#define BRIDGE_SPIMODE_3       0B11  // CPOL: 1  CPHA: 1
#define SLAVENUM 0                   // slave number of RM3100
#define SPI_SPEED BRIDGE_SPICLK_461_kHz
#define SPI_MODE BRIDGE_SPIMODE_0
#define BRIDGE_CONFIG_SPI_CMD  0xF0  // CONFIGURE SPI FUNCTION ON BRIDGE
#define I2CAddress 0x28              // Hexadecimal slave I2C address for bridge

// RM3100 internal register values without the R/W bit
#define RM3100_REVID_REG  0x36   // Hexadecimal address for the Revid internal register
#define RM3100_POLL_REG   0x00   // Hexadecimal address for the Poll internal register
#define RM3100_CMM_REG    0x01   // Hexadecimal address for the Continuous Measurement Mode internal register
#define RM3100_STATUS_REG 0x34   // Hexadecimal address for the Status internal register
#define RM3100_TMRC_REG   0x0B   // Hexadecimal address for the Status internal register
#define RM3100_CCX1_REG   0x04   // Hexadecimal address for the Cycle Count X1 internal register
#define RM3100_CCX0_REG   0x05   // Hexadecimal address for the Cycle Count X0 internal register
#define RM3100_CCY1_REG   0x06   // Hexadecimal address for the Cycle Count X1 internal register
#define RM3100_CCY0_REG   0x07   // Hexadecimal address for the Cycle Count X0 internal register
#define RM3100_CCZ1_REG   0x08   // Hexadecimal address for the Cycle Count X1 internal register
#define RM3100_CCZ0_REG   0x09   // Hexadecimal address for the Cycle Count X0 internal register
//options
#define initialCC 200  // Cycle count default = 200 (lower cycle count = higher data rates but lower resolution)
/*
 * CC    Sensitivity     Noise    Max sample rate
 *  50      50 nT        30 nT        1600/3
 * 100      26 nT        20 nT         850/3
 * 200      13 nT        15 nT         440/3
 * 400     (8 nT)?      (10 nT)?       200/3
 */

// this magnetometer is not very noisy. Hence a kalman smooth also on the raw readings.
#define K_ERR_M_R 0.08   // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
#define K_Q_M_R   0.01   // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed

SimpleKalmanFilter kf_r_mx = SimpleKalmanFilter(K_ERR_M_R, K_ERR_M_R, K_Q_M_R);
SimpleKalmanFilter kf_r_my = SimpleKalmanFilter(K_ERR_M_R, K_ERR_M_R, K_Q_M_R);
SimpleKalmanFilter kf_r_mz = SimpleKalmanFilter(K_ERR_M_R, K_ERR_M_R, K_Q_M_R);

uint8_t revid;
uint16_t cycleCount;
float gain;

void initSPI();
uint8_t readReg(uint8_t reg);
void changeCycleCount(uint16_t newCC);
void writeReg(uint8_t reg, uint8_t value);


bool init_magnetometer_R3100(void) {
  Serial.println("Starting RM3100");
  initSPI();
  
  revid = readReg(RM3100_REVID_REG);
  if(revid!=0x22) {
    return false;
  }
  changeCycleCount(initialCC); //set the cycle count;
  cycleCount = readReg(RM3100_CCX1_REG);
  cycleCount = (cycleCount << 8) | readReg(RM3100_CCX0_REG);
  
  gain = (0.3671 * (float)cycleCount) + 1.5; //linear equation to calculate the gain from cycle count
  Serial.print("Gain = "); //display gain; default gain should be around 75 for the default cycle count of 200
  Serial.println(gain);
  Serial.println();

  // Enable transmission to take continuous measurement with Alarm functions off
  writeReg(RM3100_CMM_REG, 0x79);

  return true;
}

/*
* RM3100 adopts the NED convention
* when X (the arrow) is pointing North, Y points at East and Z points Down
* so X pointing forward, Y aims at right and Z down
*
* x axis is inverted so that azimut direction is computed as required
*   E : 90°, S : 180°, W : 270°, N : 0/360°
*/
void mag_axes_RM3100(float raw[3]) {
  float north = -raw[0];  // right direction becomes front
  float east = -raw[1];  // front direction becomes left
  float down = -raw[2];  // up direction

  raw[0] = north; // -x;
  raw[1] = east;  // y;
  raw[2] = down;  // -z;
}

void mag_readings_RM3100(float mag_raw[3], bool smooth=true) {
  long x = 0;
  long y = 0;
  long z = 0;
  uint8_t x2,x1,x0,y2,y1,y0,z2,z1,z0;

  //wait until data is ready using polling method
  while((readReg(RM3100_STATUS_REG) & 0x80) != 0x80); //read internal status register

  //read measurements
  Wire.beginTransmission(I2CAddress);
  Wire.write(0x01);         // function to send to SS0 of bridge
  Wire.write(0xA4);         // data payload 1 byte --> address of register to be read  or 0x80 to set the read bit
  Wire.write(0xFF);
  Wire.write(0xFF);
  Wire.write(0xFF);
  Wire.write(0xFF);
  Wire.write(0xFF);
  Wire.write(0xFF);
  Wire.write(0xFF);
  Wire.write(0xFF);
  Wire.write(0xFF);
  Wire.endTransmission();
  delay(1);
  Wire.requestFrom(I2CAddress, 10);
  Wire.read(); // consume dummy data generated by the bridge
  x2 = Wire.read();  // receive a byte as character
  x1 = Wire.read();  // receive a byte as character
  x0 = Wire.read();  // receive a byte as character
  y2 = Wire.read();  // receive a byte as character
  y1 = Wire.read();  // receive a byte as character
  y0 = Wire.read();  // receive a byte as character
  z2 = Wire.read();  // receive a byte as character
  z1 = Wire.read();  // receive a byte as character
  z0 = Wire.read();  // receive a byte as character
  // Serial.print("READINGS Z  - z2: ");Serial.print(z2, HEX);Serial.print(" z1: ");Serial.print(z1, HEX);Serial.print(" z0: ");Serial.println(z0, HEX);

  //special bit manipulation since there is not a 24 bit signed int data type
  if (x2 & 0x80) x = 0xFF;
  if (y2 & 0x80) y = 0xFF;
  if (z2 & 0x80) z = 0xFF;

  //format results into single 32 bit signed value
  x = (x * 256 * 256 * 256) | (int32_t)(x2) * 256 * 256 | (uint16_t)(x1) * 256 | x0;
  y = (y * 256 * 256 * 256) | (int32_t)(y2) * 256 * 256 | (uint16_t)(y1) * 256 | y0;
  // z = (z * 256 * 256 * 256) | (int32_t)(z2) * 256 * 256 | (uint16_t)(z1) * 256 | z0;
  /************************
   * NOTE: for some f*cking reason the z-line, and ONLY the z-line, does NOT work on Teensy 4.0 - it DOES work as it should in ESP32
   *       on Teensy 4.0 it loses the first byte in case the value is negative. Negative z values are therefore completely screwed up.
   * 
   *       The workaround I found is processing the z axis separately as follows.
  *************************/
  z = z << 24;
  z += ((uint32_t)z2 << 16);
  z += ((uint16_t)z1 << 8);
  z += z0;
 
  // calculate magnitude of results -- hard iron screws it up
  // double uT = sqrt(pow(((float)(x)/gain),2) + pow(((float)(y)/gain),2)+ pow(((float)(z)/gain),2));
  mag_raw[0] = (float)(x)/gain;
  mag_raw[1] = (float)(y)/gain;
  mag_raw[2] = (float)(z)/gain;

  if(smooth) {
    mag_raw[0] = kf_r_mx.updateEstimate(mag_raw[0]);
    mag_raw[1] = kf_r_my.updateEstimate(mag_raw[1]);
    mag_raw[2] = kf_r_mz.updateEstimate(mag_raw[2]);
  }
  mag_axes_RM3100(mag_raw);
}

/*
 * Initialises the bridge SPI
 */
void initSPI(){
  Wire.beginTransmission(I2CAddress);
  Wire.write(BRIDGE_CONFIG_SPI_CMD);
  Wire.write(0x00 | SPI_SPEED); // order bit 0, mode bits 00  => 0x00 | SPI_SPEED
  Wire.endTransmission();
}

/*
 * reg is the 7 bit value of the RM3100 register's address (without the R/W bit)
 */
uint8_t readReg(uint8_t reg){
  uint8_t data=0;
  Wire.beginTransmission(I2CAddress);
  Wire.write(0x01);         // function to send to SS0 of bridge
  Wire.write(reg | 0x80);   // data payload 1 byte --> address of register to be read  or 0x80 to set the read bit
  Wire.write(0xFF);
  Wire.write(0xFF);
  Wire.endTransmission();
  delay(1);
  Wire.requestFrom(I2CAddress, 2);  // 2 reads, 1st is dummy, 2nd is value
  while (Wire.available()) {        // peripheral may send less than requested
    data = Wire.read();             // receive a byte as character
  }
  return data;
}

/*
 * reg is the 7 bit value of the RM3100 register's address (without the R/W bit)
 * value is the 8 bit data being written
 */
void writeReg(uint8_t reg, uint8_t value){
  Wire.beginTransmission(I2CAddress);
  Wire.write(0x01);       // function to send to SS0 of bridge
  Wire.write(reg & 0x7F); //AND with 0x7F to make first bit(read/write bit) low for write
  Wire.write(value);
  Wire.endTransmission();
}

/*
 * newCC is the new cycle count value (16 bits) to change the data acquisition
 * This is the first function being called that writes something to the RM3100
 * 
 * Since the first byte written (cold start) always fails, this function starts
 * by executing a dummy write.
 */
void changeCycleCount(uint16_t newCC){
  uint8_t CCMSB = (newCC & 0xFF00) >> 8; //get the most significant byte
  uint8_t CCLSB = newCC & 0xFF; //get the least significant byte

  // dummy write 1 byte because the first write after a cold start ALWAYS fails
  writeReg(RM3100_CCX1_REG, CCMSB);
  delay(1);

  Wire.beginTransmission(I2CAddress);
  Wire.write(0x01);  // function to send to SS0
  Wire.write(RM3100_CCX1_REG & 0x7F); // data payload 1 byte --> AND with 0x7F to make first bit(read/write bit) low for write
  Wire.write(CCMSB);  //write new cycle count to ccx1
  Wire.write(CCLSB);  //write new cycle count to ccx0
  Wire.write(CCMSB);  //write new cycle count to ccy1
  Wire.write(CCLSB);  //write new cycle count to ccy0
  Wire.write(CCMSB);  //write new cycle count to ccz1
  Wire.write(CCLSB);  //write new cycle count to ccz0
  Wire.endTransmission();
}

int dataRate_RM3100(void) {
  uint8_t tmrc = readReg(RM3100_TMRC_REG);
  Serial.print("TMRC = 0x"); //REVID ID should be 0x22
  Serial.println(tmrc, HEX);
  switch(tmrc) {
    case 0x94:
      return 150;
    case 0x95:
      return 75;
    case 0x96:
      return 37;
    case 0x97:
      return 18;
    default:
      return 9;
  }
}
